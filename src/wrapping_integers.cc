#include "wrapping_integers.hh"
#include <iostream>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  uint32_t num = static_cast<uint32_t>(n);
  return zero_point + num;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint64_t top_32 = checkpoint >> 32;
  uint64_t low_32 = checkpoint << 32 >> 32;
  uint64_t num = static_cast<uint64_t>(this->raw_value_ - zero_point.get_raw_value());
  int bias_r = num >= low_32 ? 0 : 1;
  int bias_l = num <= low_32 ? 0 : 1;
  uint64_t larger =  static_cast<uint64_t>(num + (static_cast<uint64_t>(top_32 + bias_r) << 32));
  uint64_t less = static_cast<uint64_t>(num + (static_cast<uint64_t>(top_32 - bias_l) << 32));
  if(top_32 != 0)
    return larger - checkpoint > checkpoint - less ? less : larger;
  if(num <= checkpoint)
    return larger - checkpoint > checkpoint - num ? num : larger;
  return larger;
}
