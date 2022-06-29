
#include <worth/chain/worth_object_types.hpp>

#include <worth/chain/index.hpp>

#include <worth/chain/block_summary_object.hpp>
#include <worth/chain/history_object.hpp>
#include <worth/chain/pending_required_action_object.hpp>
#include <worth/chain/pending_optional_action_object.hpp>
#include <worth/chain/wmt_objects.hpp>
#include <worth/chain/worth_objects.hpp>
#include <worth/chain/wps_objects.hpp>
#include <worth/chain/transaction_object.hpp>
#include <worth/chain/witness_schedule.hpp>

namespace worth { namespace chain {

void initialize_core_indexes( database& db )
{
   WORTH_ADD_CORE_INDEX(db, dynamic_global_property_index);
   WORTH_ADD_CORE_INDEX(db, account_index);
   WORTH_ADD_CORE_INDEX(db, account_metadata_index);
   WORTH_ADD_CORE_INDEX(db, account_authority_index);
   WORTH_ADD_CORE_INDEX(db, witness_index);
   WORTH_ADD_CORE_INDEX(db, transaction_index);
   WORTH_ADD_CORE_INDEX(db, block_summary_index);
   WORTH_ADD_CORE_INDEX(db, witness_schedule_index);
   WORTH_ADD_CORE_INDEX(db, comment_index);
   WORTH_ADD_CORE_INDEX(db, comment_content_index);
   WORTH_ADD_CORE_INDEX(db, comment_vote_index);
   WORTH_ADD_CORE_INDEX(db, witness_vote_index);
   WORTH_ADD_CORE_INDEX(db, limit_order_index);
   WORTH_ADD_CORE_INDEX(db, feed_history_index);
   WORTH_ADD_CORE_INDEX(db, convert_request_index);
   WORTH_ADD_CORE_INDEX(db, liquidity_reward_balance_index);
   WORTH_ADD_CORE_INDEX(db, operation_index);
   WORTH_ADD_CORE_INDEX(db, account_history_index);
   WORTH_ADD_CORE_INDEX(db, hardfork_property_index);
   WORTH_ADD_CORE_INDEX(db, withdraw_vesting_route_index);
   WORTH_ADD_CORE_INDEX(db, owner_authority_history_index);
   WORTH_ADD_CORE_INDEX(db, account_recovery_request_index);
   WORTH_ADD_CORE_INDEX(db, change_recovery_account_request_index);
   WORTH_ADD_CORE_INDEX(db, escrow_index);
   WORTH_ADD_CORE_INDEX(db, savings_withdraw_index);
   WORTH_ADD_CORE_INDEX(db, decline_voting_rights_request_index);
   WORTH_ADD_CORE_INDEX(db, reward_fund_index);
   WORTH_ADD_CORE_INDEX(db, vesting_delegation_index);
   WORTH_ADD_CORE_INDEX(db, vesting_delegation_expiration_index);
   WORTH_ADD_CORE_INDEX(db, pending_required_action_index);
   WORTH_ADD_CORE_INDEX(db, pending_optional_action_index);
#ifdef WORTHEUM_ENABLE_WMT
   WORTH_ADD_CORE_INDEX(db, wmt_token_index);
   WORTH_ADD_CORE_INDEX(db, account_regular_balance_index);
   WORTH_ADD_CORE_INDEX(db, account_rewards_balance_index);
   WORTH_ADD_CORE_INDEX(db, nai_pool_index);
   WORTH_ADD_CORE_INDEX(db, wmt_token_emissions_index);
   WORTH_ADD_CORE_INDEX(db, wmt_contribution_index);
   WORTH_ADD_CORE_INDEX(db, wmt_ico_index);
#endif
   WORTH_ADD_CORE_INDEX(db, proposal_index);
   WORTH_ADD_CORE_INDEX(db, proposal_vote_index);
}

index_info::index_info() {}
index_info::~index_info() {}

} }
