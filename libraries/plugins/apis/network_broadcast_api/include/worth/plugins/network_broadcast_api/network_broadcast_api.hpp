#pragma once
#include <worth/protocol/block.hpp>

#include <worth/plugins/p2p/p2p_plugin.hpp>
#include <worth/plugins/chain/chain_plugin.hpp>
#include <worth/plugins/json_rpc/utility.hpp>

#include <fc/optional.hpp>
#include <fc/variant.hpp>
#include <fc/vector.hpp>

#include <boost/thread/mutex.hpp>

namespace worth { namespace plugins { namespace network_broadcast_api {

using std::vector;
using fc::variant;
using fc::optional;
using worth::plugins::json_rpc::void_type;

using worth::protocol::signed_transaction;
using worth::protocol::transaction_id_type;
using worth::protocol::signed_block;

struct broadcast_transaction_args
{
   signed_transaction   trx;
   int32_t              max_block_age = -1;
};

typedef void_type broadcast_transaction_return;

struct broadcast_block_args
{
   signed_block   block;
};

typedef void_type broadcast_block_return;

namespace detail{ class network_broadcast_api_impl; }

class network_broadcast_api
{
   public:
      network_broadcast_api();
      ~network_broadcast_api();

      DECLARE_API(
         (broadcast_transaction)
         (broadcast_block)
      )

   private:
      std::unique_ptr< detail::network_broadcast_api_impl > my;
};

} } } // worth::plugins::network_broadcast_api

FC_REFLECT( worth::plugins::network_broadcast_api::broadcast_transaction_args,
   (trx)(max_block_age) )

FC_REFLECT( worth::plugins::network_broadcast_api::broadcast_block_args,
   (block) )
