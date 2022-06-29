#pragma once

#include <worth/protocol/asset.hpp>

namespace worth { namespace chain { namespace util {

using worth::protocol::asset;
using worth::protocol::price;

inline asset to_wbd( const price& p, const asset& worth )
{
   FC_ASSERT( worth.symbol == WORTH_SYMBOL );
   if( p.is_null() )
      return asset( 0, WBD_SYMBOL );
   return worth * p;
}

inline asset to_worth( const price& p, const asset& wbd )
{
   FC_ASSERT( wbd.symbol == WBD_SYMBOL );
   if( p.is_null() )
      return asset( 0, WORTH_SYMBOL );
   return wbd * p;
}

} } }
