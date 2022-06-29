#pragma once

#include <worth/protocol/worth_operations.hpp>

#include <worth/chain/evaluator.hpp>

namespace worth { namespace chain {

using namespace worth::protocol;

WORTH_DEFINE_EVALUATOR( account_create )
WORTH_DEFINE_EVALUATOR( account_create_with_delegation )
WORTH_DEFINE_EVALUATOR( account_update )
WORTH_DEFINE_EVALUATOR( account_update2 )
WORTH_DEFINE_EVALUATOR( transfer )
WORTH_DEFINE_EVALUATOR( transfer_to_vesting )
WORTH_DEFINE_EVALUATOR( witness_update )
WORTH_DEFINE_EVALUATOR( account_witness_vote )
WORTH_DEFINE_EVALUATOR( account_witness_proxy )
WORTH_DEFINE_EVALUATOR( withdraw_vesting )
WORTH_DEFINE_EVALUATOR( set_withdraw_vesting_route )
WORTH_DEFINE_EVALUATOR( comment )
WORTH_DEFINE_EVALUATOR( comment_options )
WORTH_DEFINE_EVALUATOR( delete_comment )
WORTH_DEFINE_EVALUATOR( vote )
WORTH_DEFINE_EVALUATOR( custom )
WORTH_DEFINE_EVALUATOR( custom_json )
WORTH_DEFINE_EVALUATOR( custom_binary )
WORTH_DEFINE_EVALUATOR( pow )
WORTH_DEFINE_EVALUATOR( pow2 )
WORTH_DEFINE_EVALUATOR( feed_publish )
WORTH_DEFINE_EVALUATOR( convert )
WORTH_DEFINE_EVALUATOR( limit_order_create )
WORTH_DEFINE_EVALUATOR( limit_order_cancel )
WORTH_DEFINE_EVALUATOR( report_over_production )
WORTH_DEFINE_EVALUATOR( limit_order_create2 )
WORTH_DEFINE_EVALUATOR( escrow_transfer )
WORTH_DEFINE_EVALUATOR( escrow_approve )
WORTH_DEFINE_EVALUATOR( escrow_dispute )
WORTH_DEFINE_EVALUATOR( escrow_release )
WORTH_DEFINE_EVALUATOR( claim_account )
WORTH_DEFINE_EVALUATOR( create_claimed_account )
WORTH_DEFINE_EVALUATOR( request_account_recovery )
WORTH_DEFINE_EVALUATOR( recover_account )
WORTH_DEFINE_EVALUATOR( change_recovery_account )
WORTH_DEFINE_EVALUATOR( transfer_to_savings )
WORTH_DEFINE_EVALUATOR( transfer_from_savings )
WORTH_DEFINE_EVALUATOR( cancel_transfer_from_savings )
WORTH_DEFINE_EVALUATOR( decline_voting_rights )
WORTH_DEFINE_EVALUATOR( reset_account )
WORTH_DEFINE_EVALUATOR( set_reset_account )
WORTH_DEFINE_EVALUATOR( claim_reward_balance )
#ifdef WORTHEUM_ENABLE_WMT
WORTH_DEFINE_EVALUATOR( claim_reward_balance2 )
#endif
WORTH_DEFINE_EVALUATOR( delegate_vesting_shares )
WORTH_DEFINE_EVALUATOR( witness_set_properties )
#ifdef WORTHEUM_ENABLE_WMT
WORTH_DEFINE_EVALUATOR( wmt_setup )
WORTH_DEFINE_EVALUATOR( wmt_setup_emissions )
WORTH_DEFINE_EVALUATOR( wmt_set_setup_parameters )
WORTH_DEFINE_EVALUATOR( wmt_set_runtime_parameters )
WORTH_DEFINE_EVALUATOR( wmt_create )
WORTH_DEFINE_EVALUATOR( wmt_contribute )
#endif
WORTH_DEFINE_EVALUATOR( create_proposal )
WORTH_DEFINE_EVALUATOR( update_proposal_votes )
WORTH_DEFINE_EVALUATOR( remove_proposal )

} } // worth::chain
