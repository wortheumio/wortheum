#include <worth/protocol/wps_operations.hpp>

#include <worth/protocol/validation.hpp>

namespace worth { namespace protocol {

void create_proposal_operation::validate()const
{
   validate_account_name( creator );
   validate_account_name( receiver );

   FC_ASSERT( end_date > start_date, "end date must be greater than start date" );

   FC_ASSERT( daily_pay.amount >= 0, "Daily pay can't be negative value" );
   FC_ASSERT( daily_pay.symbol.asset_num == WORTH_ASSET_NUM_WBD, "Daily pay should be expressed in WBD");

   FC_ASSERT( !subject.empty(), "subject is required" );
   FC_ASSERT( subject.size() <= WORTH_PROPOSAL_SUBJECT_MAX_LENGTH, "Subject is too long");
   FC_ASSERT( fc::is_utf8( subject ), "Subject is not valid UTF8" );

   FC_ASSERT( !permlink.empty(), "permlink is required" );
   validate_permlink(permlink);
}

void update_proposal_votes_operation::validate()const
{
   validate_account_name( voter );
   FC_ASSERT(proposal_ids.empty() == false);
   FC_ASSERT(proposal_ids.size() <= WORTH_PROPOSAL_MAX_IDS_NUMBER);
}

void remove_proposal_operation::validate() const
{
   validate_account_name(proposal_owner);
   FC_ASSERT(proposal_ids.empty() == false);
   FC_ASSERT(proposal_ids.size() <= WORTH_PROPOSAL_MAX_IDS_NUMBER);
}

} } //worth::protocol
