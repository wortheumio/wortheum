#pragma once
#include <worth/plugins/json_rpc/utility.hpp>
#include <worth/plugins/reputation/reputation_objects.hpp>
#include <worth/plugins/database_api/database_api_objects.hpp>

#include <worth/protocol/types.hpp>

#include <fc/optional.hpp>
#include <fc/variant.hpp>
#include <fc/vector.hpp>

namespace worth { namespace plugins { namespace reputation {

using worth::protocol::account_name_type;

namespace detail
{
   class reputation_api_impl;
}

struct account_reputation
{
   account_name_type             account;
   worth::protocol::share_type   reputation;
};

struct get_account_reputations_args
{
   account_name_type account_lower_bound;
   uint32_t          limit = 1000;
};

struct get_account_reputations_return
{
   vector< account_reputation > reputations;
};

class reputation_api
{
   public:
      reputation_api();
      ~reputation_api();

      DECLARE_API(
         (get_account_reputations)
      )

   private:
      std::unique_ptr< detail::reputation_api_impl > my;
};

} } } // worth::plugins::reputation

FC_REFLECT( worth::plugins::reputation::account_reputation,
            (account)(reputation) );

FC_REFLECT( worth::plugins::reputation::get_account_reputations_args,
            (account_lower_bound)(limit) );

FC_REFLECT( worth::plugins::reputation::get_account_reputations_return,
            (reputations) );
