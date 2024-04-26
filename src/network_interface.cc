#include <iostream>

#include "arp_message.hh"
#include "exception.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  uint32_t next_ip_addr = next_hop.ipv4_numeric();
  cout << ARP_table_.count( next_ip_addr ) << "---\n";
  if(time_control_.count( next_ip_addr ))
    cout << time_control_[next_ip_addr].if_MAC_useful( now_tick_ ) << "---~~~\n";
  if( ARP_table_.count( next_ip_addr ) && 
      time_control_[next_ip_addr].if_MAC_useful( now_tick_ ) ) {

    send_data_msg( dgram, ARP_table_[next_ip_addr] );
    return ;
  }
  if( time_control_.count( next_ip_addr ) && time_control_[next_ip_addr].if_can_send_ARP( now_tick_ ) )
    return;
  time_control_[next_ip_addr] = ARPTimer();
  send_ARP_msg( build_ARP_msg( ARPMessage::OPCODE_REQUEST, next_ip_addr ), ETHERNET_BROADCAST);
  buffer_[next_ip_addr].push( dgram );
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  EthernetHeader header = frame.header;
  if( header.dst != ETHERNET_BROADCAST && header.dst != ethernet_address_ )
    return ;
  switch ( header.type ) {
    case EthernetHeader::TYPE_ARP: {
      ARPMessage arp_msg;
      if( ! parse( arp_msg, frame.payload ) )
        break;
      switch ( arp_msg.opcode ) {
        case ARPMessage::OPCODE_REPLY: {
          time_control_[arp_msg.sender_ip_address].confirm().setTick( now_tick_ );
          ARP_table_[arp_msg.sender_ip_address] = arp_msg.sender_ethernet_address;
          send_buf_datagram( arp_msg.sender_ip_address, arp_msg.sender_ethernet_address );
          break;
        }
        
        case ARPMessage::OPCODE_REQUEST: {
          if( arp_msg.target_ip_address != ip_address_.ipv4_numeric() )
            break;
          if(! time_control_.count(arp_msg.sender_ip_address))
            time_control_[arp_msg.sender_ip_address] = ARPTimer();
          time_control_[arp_msg.sender_ip_address].confirm().setTick( now_tick_ );
          ARP_table_[arp_msg.sender_ip_address] = arp_msg.sender_ethernet_address;
          send_ARP_msg( build_ARP_msg( 
            ARPMessage::OPCODE_REPLY, arp_msg.sender_ip_address, arp_msg.sender_ethernet_address ), 
          arp_msg.sender_ethernet_address);
          break;
        }
        
        default:
          throw std::runtime_error("unexpect ARP.opcode = " + arp_msg.opcode);
      }
      break;
    }
    
    case EthernetHeader::TYPE_IPv4: {
      InternetDatagram ip_data;
      if( ! parse( ip_data, frame.payload ) )
        break;
      datagrams_received_.emplace( move( ip_data ) );
      break;
    }

    default:
      throw std::runtime_error("unexpect Header.TYPE = " + header.type);
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  now_tick_ += ms_since_last_tick;
}

void NetworkInterface::send_data_msg( const InternetDatagram& dgram, EthernetAddress mac_address) {
  EthernetHeader header = {
    .dst = mac_address,
    .src = ethernet_address_,
    .type = EthernetHeader::TYPE_IPv4
  };
  EthernetFrame frame = {
    .header = header,
    .payload = serialize( dgram )
  };
  transmit( frame );
}


void NetworkInterface::send_ARP_msg( ARPMessage msg, EthernetAddress dst ) {
  EthernetHeader header = {
    .dst = dst,
    .src = ethernet_address_,
    .type = EthernetHeader::TYPE_ARP
  };
  EthernetFrame frame = {
    .header = header,
    .payload = serialize( msg )
  };
  transmit( frame );
}

ARPMessage NetworkInterface::build_ARP_msg( uint16_t opcode, uint32_t target_ip ) {
  if ( opcode != ARPMessage::OPCODE_REQUEST )
    throw std::runtime_error("build_ARP_msg: opcode = ARPMessage::OPCODE_REPLY but in request method");
  ARPMessage msg;
  msg.opcode = ARPMessage::OPCODE_REQUEST;
  msg.sender_ethernet_address = ethernet_address_;
  msg.sender_ip_address = ip_address_.ipv4_numeric();
  msg.target_ip_address = target_ip;
  return msg;
}

ARPMessage NetworkInterface::build_ARP_msg( uint16_t opcode, uint32_t ori_ip, EthernetAddress ori_mac ) {
  if( opcode != ARPMessage::OPCODE_REPLY )
    throw std::runtime_error("build_ARP_msg: opcode = ARPMessage::OPCODE_REQUEST but in reply method");
  ARPMessage msg;
  msg.opcode = ARPMessage::OPCODE_REPLY;
  msg.sender_ethernet_address = ethernet_address_;
  msg.sender_ip_address = ip_address_.ipv4_numeric();
  msg.target_ip_address = ori_ip;
  msg.target_ethernet_address = ori_mac;
  return msg;
}

void NetworkInterface::send_buf_datagram( uint32_t ip_address, EthernetAddress mac_address ) {
  if( !buffer_.count( ip_address ) )
    return;
  while ( !buffer_[ip_address].empty() ) {
    send_data_msg( buffer_[ip_address].front(), mac_address );
    buffer_[ip_address].pop();
  }
}

ARPTimer& ARPTimer::confirm() { 
  if( status == ARPTimer::WAITING )
    status = ARPTimer::CONFIRM; 
  return *this;
}

bool ARPTimer::if_MAC_useful( uint64_t now_tick ) {
  return status == ARPTimer::CONFIRM && now_tick >= last_tick && (now_tick - last_tick <= 30000);
}

bool ARPTimer::if_can_send_ARP( uint64_t now_tick ) {
  return status == ARPTimer::WAITING && now_tick >= last_tick && (now_tick - last_tick <= 5000);
}

ARPTimer& ARPTimer::setTick( uint64_t now_tick ) { 
  this->last_tick = now_tick;
  return *this;
}