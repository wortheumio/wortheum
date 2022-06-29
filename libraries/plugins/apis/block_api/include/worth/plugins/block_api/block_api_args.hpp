#pragma once
#include <worth/plugins/block_api/block_api_objects.hpp>

#include <worth/protocol/types.hpp>
#include <worth/protocol/transaction.hpp>
#include <worth/protocol/block_header.hpp>

#include <worth/plugins/json_rpc/utility.hpp>

namespace worth { namespace plugins { namespace block_api {

/* get_block_header */

struct get_block_header_args
{
   uint32_t block_num;
};

struct get_block_header_return
{
   optional< block_header > header;
};

/* get_block */
struct get_block_args
{
   uint32_t block_num;
};

struct get_block_return
{
   optional< api_signed_block_object > block;
};

} } } // worth::block_api

FC_REFLECT( worth::plugins::block_api::get_block_header_args,
   (block_num) )

FC_REFLECT( worth::plugins::block_api::get_block_header_return,
   (header) )

FC_REFLECT( worth::plugins::block_api::get_block_args,
   (block_num) )

FC_REFLECT( worth::plugins::block_api::get_block_return,
   (block) )

