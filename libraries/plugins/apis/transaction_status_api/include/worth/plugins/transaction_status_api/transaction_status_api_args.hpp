#pragma once

#include <worth/protocol/types.hpp>
#include <worth/protocol/transaction.hpp>
#include <worth/protocol/block_header.hpp>

#include <worth/plugins/json_rpc/utility.hpp>
#include <worth/plugins/transaction_status/transaction_status_objects.hpp>

namespace worth { namespace plugins { namespace transaction_status_api {

struct find_transaction_args
{
   chain::transaction_id_type transaction_id;
   fc::optional< fc::time_point_sec > expiration;
};

struct find_transaction_return
{
   transaction_status::transaction_status status;
   fc::optional< uint32_t > block_num;
};

} } } // worth::transaction_status_api

FC_REFLECT( worth::plugins::transaction_status_api::find_transaction_args, (transaction_id)(expiration) )
FC_REFLECT( worth::plugins::transaction_status_api::find_transaction_return, (status)(block_num) )
