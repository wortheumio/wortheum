#pragma once

#include <worth/chain/util/asset.hpp>

#include <worth/protocol/asset.hpp>
#include <worth/protocol/config.hpp>
#include <worth/protocol/types.hpp>
#include <worth/protocol/misc_utilities.hpp>

#include <fc/reflect/reflect.hpp>

#include <fc/uint128.hpp>

namespace worth { namespace chain { namespace util {

using worth::protocol::asset;
using worth::protocol::price;
using worth::protocol::share_type;

using fc::uint128_t;

struct comment_reward_context
{
   share_type rshares;
   uint16_t   reward_weight = 0;
   asset      max_wbd;
   uint128_t  total_reward_shares2;
   asset      total_reward_fund_worth;
   price      current_worth_price;
   protocol::curve_id   reward_curve = protocol::quadratic;
   uint128_t  content_constant = WORTH_CONTENT_CONSTANT_HF0;
};

uint64_t get_rshare_reward( const comment_reward_context& ctx );

inline uint128_t get_content_constant_s()
{
   return WORTH_CONTENT_CONSTANT_HF0; // looking good for posters
}

uint128_t evaluate_reward_curve( const uint128_t& rshares, const protocol::curve_id& curve = protocol::quadratic, const uint128_t& var1 = WORTH_CONTENT_CONSTANT_HF0 );

inline bool is_comment_payout_dust( const price& p, uint64_t worth_payout )
{
   return to_wbd( p, asset( worth_payout, WORTH_SYMBOL ) ) < WORTH_MIN_PAYOUT_WBD;
}

} } } // worth::chain::util

FC_REFLECT( worth::chain::util::comment_reward_context,
   (rshares)
   (reward_weight)
   (max_wbd)
   (total_reward_shares2)
   (total_reward_fund_worth)
   (current_worth_price)
   (reward_curve)
   (content_constant)
   )
