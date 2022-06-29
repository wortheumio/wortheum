#pragma once

#include <fc/container/flat.hpp>
#include <worth/protocol/operations.hpp>
#include <worth/protocol/transaction.hpp>

#include <fc/string.hpp>

namespace worth { namespace app {

using namespace fc;

void operation_get_impacted_accounts(
   const worth::protocol::operation& op,
   fc::flat_set<protocol::account_name_type>& result );

void transaction_get_impacted_accounts(
   const worth::protocol::transaction& tx,
   fc::flat_set<protocol::account_name_type>& result
   );

} } // worth::app
