#pragma once

#include <worth/protocol/block.hpp>

namespace worth { namespace chain {

struct transaction_notification
{
   transaction_notification( const worth::protocol::signed_transaction& tx ) : transaction(tx)
   {
      transaction_id = tx.id();
   }

   worth::protocol::transaction_id_type          transaction_id;
   const worth::protocol::signed_transaction&    transaction;
};

} }
