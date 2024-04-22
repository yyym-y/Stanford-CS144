#include "tcp_sender.hh"
#include "tcp_config.hh"
#include <iostream>

using namespace std;

RetransmissionTimer& RetransmissionTimer::active() noexcept {
  is_active_ = true;
  return *this;
}

RetransmissionTimer& RetransmissionTimer::timeout() noexcept {
  RTO_ <<= 1;
  return *this;
}

RetransmissionTimer& RetransmissionTimer::reset() noexcept {
  time_passed_ = 0;
  return *this;
}

RetransmissionTimer& RetransmissionTimer::tick( uint64_t ms_since_last_tick ) noexcept {
  time_passed_ += is_active_ ? ms_since_last_tick : 0;
  return *this;
}

TCPSenderMessage TCPSender::make_message( uint64_t seqno, std::string payload,
                               bool SYN, bool FIN ) const {
  return {
    .seqno = Wrap32::wrap( seqno, isn_ ),
    .SYN = SYN,
    .payload = std::move(payload),
    .FIN = FIN,
    .RST = input_.reader().has_error()
  };
}


uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return num_bytes_in_flight_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return retransmission_cnt_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  Reader& byte_reader = input_.reader();
  const size_t window_size = wnd_size_ == 0 ? 1 : wnd_size_;
  /*
  * 在这个for循环中不断构建数据
  * @param("payload") 要发送的数据
  * @check("num_bytes_in_flight_ < window_size && !sent_fin_")
  *   * 已经发送的数据数量要小于接收方的窗口大小, 同时,如果发送了 FIN 信号也就不需要再发送了
  * @after("payload.clear()") 清空载荷准备下一次发送
  */
  for ( string payload {} ; num_bytes_in_flight_ < window_size && !sent_fin_; payload.clear() ) {
    // 如果已经发送了 SYN 信号并且此时没有实质性需要发送的数据(payload)且发送了FIN信号, 那么直接结束循环
    // @check("sent_fin_") : 这个条件确保了我们只发送FIN时可以正常的发送
    if (sent_syn_ && byte_reader.bytes_buffered() == 0 && sent_fin_ )
      break;
    // 检查是否要发送 SYN 并更新 sent_syn_ 值
    syn_flag_ = sent_syn_ ? false : true;
    if( syn_flag_ ) sent_syn_ = true;
    
    /*
    * 因为 `byte_reader.peek()` 是只弹出一个字符, 所以需要 while 循环来不断构建
    * @check("payload.size() + num_bytes_in_flight_ + syn_flag_ < window_size")
    *   * 要时刻保持 payload 的大小 + 已经发送的数据大小小于接收方窗口大小
    *   * 注意, syn_flag_ 也要考虑, 毕竟也占用窗口大小
    * @check("payload.size() < TCPConfig::MAX_PAYLOAD_SIZE") : 数据大小不能超过这个阈值
    */
    while ( payload.size() + num_bytes_in_flight_ + syn_flag_ < window_size &&
            payload.size() < TCPConfig::MAX_PAYLOAD_SIZE ) {
      // 没有内容可以读了, 或者流结束了, 直接返回
      if( byte_reader.bytes_buffered() == 0 || fin_flag_ )
        break;
      // 读取数据并更新
      payload += byte_reader.peek();
      byte_reader.pop(1);
      // 时刻判断数据流是否结束
      fin_flag_ |= byte_reader.is_finished();
    }

    // 是否要发送 FIN 信号
    fin_flag_ |= byte_reader.is_finished();
    if( fin_flag_ ) sent_fin_ = true;

    // 构建 msg
    TCPSenderMessage msg = make_message(next_seqno_, move( payload ), syn_flag_, fin_flag_);
    size_t correct_length = msg.sequence_length();
    // 注意, 我们在构建msg的时候没有考虑, 当加上FIN信号后, 整个数据报的长度大于窗口大小的情况
    // 也就是说 FIN + payload.size() == window_size 的情况, 这个时候再发送 FIN 信号就会超出窗口大小
    // 所以我们要分割数据报, 也就是说, 这次不发送 FIN , 下次发送只发送 FIN
    if( correct_length > window_size ) {
      msg.FIN = false; sent_fin_ = false;
      correct_length --;
    }
    // 如果没有啥有效的数据, 那就别发
    if( correct_length == 0 ) break;
    // 更新统计数据并发送
    outstanding_bytes_.emplace(msg);
    next_seqno_ += correct_length;
    num_bytes_in_flight_ += correct_length;
    transmit( msg );
    // 启动计时器
    if ( correct_length != 0 )
      timer_.active();
    // 没有可读的数据自然要结束循环
    if( byte_reader.empty() ) break;
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  return make_message( next_seqno_, {}, false );
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // 更新接收方的窗口大小
  wnd_size_ = msg.window_size;
  // 当 msg.ackno 没有值得时候, 一定是出现了某种错误, 那么久直接 set_error 重建流
  if( !msg.ackno.has_value() ) {
    if( msg.window_size == 0 )
      input_.set_error();
    return;
  }

  // 获取接收方期望的序列
  const uint64_t expecting_seqno = msg.ackno->unwrap( isn_, next_seqno_ );
  if( expecting_seqno > next_seqno_ ) return; // 如果确认的序号是我还没发的, 直接结束

  // 是否有确认的发送数据, 用于更新计数器
  bool is_acknowledged = false;
  while ( !outstanding_bytes_.empty() ) {
    auto& buffered_msg = outstanding_bytes_.front();
    const uint64_t can_pop_seqno = acked_seqno_ + buffered_msg.sequence_length();

    if( expecting_seqno <= acked_seqno_ || expecting_seqno < can_pop_seqno )
      break;
    
    is_acknowledged = true;
    num_bytes_in_flight_ -= buffered_msg.sequence_length();
    acked_seqno_ += buffered_msg.sequence_length();

    outstanding_bytes_.pop();
  }

  // 如果有确认的数据
  if( is_acknowledged ) {
    if( outstanding_bytes_.empty() ) // 全部发送的数据确认后, 那么直接构建一个新的计数器
      timer_ = RetransmissionTimer( initial_RTO_ms_ );
    else
      timer_ = move( RetransmissionTimer( initial_RTO_ms_ ).active() ); // 否则继续使用这个计数器
    retransmission_cnt_ = 0;
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  if( timer_.tick( ms_since_last_tick ).is_expired() ) {
    transmit(outstanding_bytes_.front());
    if( wnd_size_ == 0 )
      timer_.reset();
    else timer_.timeout().reset();
    ++ retransmission_cnt_;
  }
}
