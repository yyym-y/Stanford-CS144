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

  router_table_.emplace( prefix_info( prefix_length, route_prefix ), make_pair( interface_num, next_hop ) );
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  ranges::for_each( _interfaces, [this]( auto& intfc ) {
    auto& incoming_dgrams = intfc->datagrams_received();
    while ( !incoming_dgrams.empty() ) {
      auto& dgram = incoming_dgrams.front();
      const auto table_iter = find_export( dgram.header.dst );
      if ( table_iter == router_table_.cend() || dgram.header.ttl <= 1 ) {
        incoming_dgrams.pop(); // TTL == 0 || (TTL - 1) == 0
        continue;              // 无法路由或 ttl 为 0 的数据报直接抛弃
      }
      --dgram.header.ttl;
      dgram.header.compute_checksum();
      const auto& [interface_num, network_addr] = table_iter->second;

      interface( interface_num )
        ->send_datagram(
          dgram,
          network_addr.has_value()
            ? *network_addr
            : Address::from_ipv4_numeric( dgram.header.dst ) ); // 没有下一跳时，表示位于同一个网络中，直接交付
      incoming_dgrams.pop();                                    // 已转发数据报，从缓冲队列中弹出
    }
  } );
}

// 按最长前缀匹配找出最合适的路由表
Router::routerT::const_iterator Router::find_export( const uint32_t target_dst ) const
{
  return ranges::find_if( router_table_, [&target_dst]( const auto& item ) -> bool {
    return ( target_dst & item.first.mask_ ) == item.first.netID_;
  } );
}
