#pragma once
#include <worth/protocol/block_header.hpp>
#include <worth/protocol/transaction.hpp>

namespace worth { namespace protocol {

   struct signed_block : public signed_block_header
   {
      checksum_type calculate_merkle_root()const;
      vector<signed_transaction> transactions;
   };

} } // worth::protocol

FC_REFLECT_DERIVED( worth::protocol::signed_block, (worth::protocol::signed_block_header), (transactions) )
