#include "router.hh"

#include <iostream>
#include <limits>
#include <ranges>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  table_.addItem( route_prefix, prefix_length, next_hop, interface_num );
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  cout << "ROUTE ----\n";
  ranges::for_each( _interfaces, [this]( auto& intfc ) {
    auto& incoming_dgrams = intfc->datagrams_received();
    while ( !incoming_dgrams.empty() ) {
      InternetDatagram& datagram = incoming_dgrams.front();
      incoming_dgrams.pop();
      uint32_t dst = datagram.header.dst;
      cout << "route dst = " << dst << "--\n";
      RouteItem result;
      if ( !table_.queryItem( result, dst ) )
        continue;
      if ( datagram.header.ttl <= 1 )
        continue;
      datagram.header.ttl --;
      datagram.header.compute_checksum();
      interface( result.interface_num_ )->send_datagram( datagram,
        result.next_hop_.has_value() ?
          *result.next_hop_ : Address::from_ipv4_numeric( datagram.header.dst )
      );
    }
  });
}

RouteItem& RouteItem::operator=( const RouteItem& other ) {
  this->prefix_ = other.prefix_;
  this->prefix_length_ = other.prefix_length_;
  this->next_hop_ = other.next_hop_;
  this->interface_num_ = other.interface_num_;
  return *this;
}

void RouteTable::addItem( uint32_t route_prefix, uint8_t prefix_length, 
                          std::optional<Address> next_hop, size_t interface_num ) {
  uint32_t mask = 0xFFFFFFFF; int shift_len = 32 - prefix_length;
  if( shift_len >= 32)
    mask = 0;
  else mask = mask >> shift_len << shift_len;
  route_prefix &= mask;
  RouteItem item = RouteItem( route_prefix, prefix_length, next_hop, interface_num );

  uint32_t tem_pos = 0;
  for( int i = 0, j = 31 ; i < prefix_length ; i ++, j -- ) {
    int bit = route_prefix >> j & 1;
    if( !tree_[tem_pos]._pos[bit] ) {
      tree_[tem_pos]._pos[bit] = ++ pos;
      tree_.push_back( TreeNode() );
    }
    tem_pos = tree_[tem_pos]._pos[bit];
  }
  tree_[tem_pos].is_item = true;
  tree_[tem_pos].item = item;
}

bool RouteTable::queryItem( RouteItem& result, uint32_t des_ip ) {
  cout << "des_ip" << des_ip << "----\n";
  bool matched = false;
  uint32_t tem_pos = 0;
  for( int i = 0, j = 31 ; i < 32 && !tree_[tem_pos].is_leaf() ; i ++, j -- ) {
    if ( tree_[tem_pos].is_item ) {
      matched = true;
      if ( !tree_[tem_pos].item.has_value() )
        throw std::runtime_error("RouteTable[" + to_string(tem_pos) + "] is_item is true but item is null");
      result = tree_[tem_pos].item.value();
    }

    int bit = des_ip >> j & 1;
    if ( tree_[tem_pos]._pos[bit] == 0 )
      break;
    tem_pos = tree_[tem_pos]._pos[bit];
  }
  return matched;
}