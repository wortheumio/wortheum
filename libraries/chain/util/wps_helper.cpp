#include <worth/chain/worth_fwd.hpp>

#include <worth/chain/util/wps_helper.hpp>

namespace worth { namespace chain {

void wps_helper::remove_proposals( database& db, const flat_set<int64_t>& proposal_ids, const account_name_type& proposal_owner )
{
   if( proposal_ids.empty() )
      return;

   auto& proposalIndex = db.get_mutable_index< proposal_index >();
   auto& byIdIdx = proposalIndex.indices().get< by_proposal_id >();

   auto& votesIndex = db.get_mutable_index< proposal_vote_index >();
   auto& byVoterIdx = votesIndex.indices().get< by_proposal_voter >();

   wps_removing_reducer obj_perf( db.get_wps_remove_threshold() );

   for(auto id : proposal_ids)
   {
      auto foundPosI = byIdIdx.find(id);

      if(foundPosI == byIdIdx.end())
         continue;

      FC_ASSERT(foundPosI->creator == proposal_owner, "Only proposal owner can remove it...");

      remove_proposal< by_proposal_id >( foundPosI, proposalIndex, votesIndex, byVoterIdx, obj_perf );
      if( obj_perf.done )
         break;
   }

}

} } // worth::chain
