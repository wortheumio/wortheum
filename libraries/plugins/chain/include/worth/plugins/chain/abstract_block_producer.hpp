#pragma once

#include <fc/time.hpp>

#include <worth/chain/database.hpp>

namespace worth { namespace plugins { namespace chain {
   
class abstract_block_producer {
public:
   virtual ~abstract_block_producer() = default;

   virtual worth::chain::signed_block generate_block(
      fc::time_point_sec when,
      const worth::chain::account_name_type& witness_owner,
      const fc::ecc::private_key& block_signing_private_key,
      uint32_t skip = worth::chain::database::skip_nothing) = 0;
};

} } } // worth::plugins::chain
