#include "tcp_receiver.hh"
#include<iostream>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if(message.RST) {
    RST = true; reader().set_error(); return;
  }
  if(message.SYN) {
    isn = message.seqno; if_set_isn = true;
    next_abs = max(next_abs, message.seqno.unwrap(isn, checkpoint));
  }
  if(! if_set_isn) return;

  uint64_t abs_seq = message.seqno.unwrap(isn, checkpoint);
  checkpoint = abs_seq;
  uint64_t str_index = message.SYN ? 0 : abs_seq - 1;
  reassembler_.insert(str_index, message.payload, message.FIN);
  if(writer().has_error()) RST = true;

  next_abs = static_cast<uint64_t>(writer().bytes_pushed()) + 1;
  if(writer().is_closed()) next_abs += 1;
}

TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage msg;
  if(if_set_isn) msg.ackno = Wrap32(0).wrap(next_abs, isn);
  msg.window_size = 
    writer().available_capacity() > UINT16_MAX ? UINT16_MAX : writer().available_capacity();

  msg.RST = RST;
  if(writer().has_error()) msg.RST = true;

  return msg;
}
