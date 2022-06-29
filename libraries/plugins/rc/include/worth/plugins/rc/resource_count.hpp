#pragma once

#include <worth/protocol/transaction.hpp>
#include <worth/protocol/optional_automated_actions.hpp>

#include <fc/int_array.hpp>
#include <fc/reflect/reflect.hpp>
#include <vector>

#define WORTH_NUM_RESOURCE_TYPES     5

namespace worth { namespace plugins { namespace rc {

enum rc_resource_types
{
   resource_history_bytes,
   resource_new_accounts,
   resource_market_bytes,
   resource_state_bytes,
   resource_execution_time
};

typedef fc::int_array< int64_t, WORTH_NUM_RESOURCE_TYPES > resource_count_type;

struct count_resources_result
{
   resource_count_type                            resource_count;
};

void count_resources(
   const worth::protocol::signed_transaction& tx,
   count_resources_result& result );

void count_resources(
   const worth::protocol::optional_automated_action&,
   count_resources_result& result );

} } } // worth::plugins::rc

FC_REFLECT_ENUM( worth::plugins::rc::rc_resource_types,
    (resource_history_bytes)
    (resource_new_accounts)
    (resource_market_bytes)
    (resource_state_bytes)
    (resource_execution_time)
   )

FC_REFLECT( worth::plugins::rc::count_resources_result,
   (resource_count)
)

FC_REFLECT_TYPENAME( worth::plugins::rc::resource_count_type )
