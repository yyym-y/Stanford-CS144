#pragma once

#include <memory>
#include <optional>

#include "exception.hh"
#include "network_interface.hh"

class RouteItem {
public:
  uint32_t prefix_ {};
  int prefix_length_ {};
  std::optional<Address> next_hop_ {};
  size_t interface_num_ {};
  RouteItem( uint32_t route_prefix, int prefix_length, 
              std::optional<Address> next_hop, size_t interface_num ) 
    : prefix_( route_prefix ), prefix_length_( prefix_length ),
      next_hop_( next_hop ), interface_num_( interface_num ) {};
  RouteItem& operator=( const RouteItem& other );
  RouteItem() {};
  std::string to_string();
};

class RouteTable {
private:
  class TreeNode {
  public:
    bool is_item {};
    uint16_t _pos[2] {};
    std::optional<RouteItem> item {};
    bool is_leaf() { return !_pos[0] && !_pos[1]; }
    TreeNode() : is_item( false ) { _pos[0] = 0; _pos[1] = 0; }
  };
  std::vector<TreeNode> tree_ {};
  uint32_t pos = 0;
  void DFS_check(int pos_t, int i, uint32_t num);
public:
  RouteTable() { tree_.push_back( TreeNode() ); };
  void addItem( uint32_t route_prefix, int prefix_length,
                std::optional<Address> next_hop, size_t interface_num );
  bool queryItem( RouteItem& result, uint32_t des_ip );
  void _check();
};

// \brief A router that has multiple network interfaces and
// performs longest-prefix-match routing between them.
class Router
{
public:
  // Add an interface to the router
  // \param[in] interface an already-constructed network interface
  // \returns The index of the interface after it has been added to the router
  size_t add_interface( std::shared_ptr<NetworkInterface> interface )
  {
    _interfaces.push_back( notnull( "add_interface", std::move( interface ) ) );
    return _interfaces.size() - 1;
  }

  // Access an interface by index
  std::shared_ptr<NetworkInterface> interface( const size_t N ) { return _interfaces.at( N ); }

  // Add a route (a forwarding rule)
  void add_route( uint32_t route_prefix,
                  uint8_t prefix_length,
                  std::optional<Address> next_hop,
                  size_t interface_num );

  // Route packets between the interfaces
  void route();

private:
  // The router's collection of network interfaces
  std::vector<std::shared_ptr<NetworkInterface>> _interfaces {};

  RouteTable table_ {};
};
