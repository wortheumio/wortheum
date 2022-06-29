#pragma once
#include <worth/chain/database.hpp>
#include <worth/protocol/asset_symbol.hpp>

#ifdef WORTHEUM_ENABLE_WMT

namespace worth { namespace chain {

   void replenish_nai_pool( database& db );
   void remove_from_nai_pool( database &db, const asset_symbol_type& a );

} } // worth::chain

#endif
