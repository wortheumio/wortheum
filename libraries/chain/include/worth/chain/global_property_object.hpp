#pragma once
#include <worth/chain/worth_fwd.hpp>

#include <fc/uint128.hpp>

#include <worth/chain/worth_object_types.hpp>

#include <worth/protocol/asset.hpp>

namespace worth { namespace chain {

   using worth::protocol::asset;
   using worth::protocol::price;

   /**
    * @class dynamic_global_property_object
    * @brief Maintains global state information
    * @ingroup object
    * @ingroup implementation
    *
    * This is an implementation detail. The values here are calculated during normal chain operations and reflect the
    * current values of global blockchain properties.
    */
   class dynamic_global_property_object : public object< dynamic_global_property_object_type, dynamic_global_property_object>
   {
      public:
         template< typename Constructor, typename Allocator >
         dynamic_global_property_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         dynamic_global_property_object(){}

         id_type           id;

         uint32_t          head_block_number = 0;
         block_id_type     head_block_id;
         time_point_sec    time;
         account_name_type current_witness;


         /**
          *  The total POW accumulated, aka the sum of num_pow_witness at the time new POW is added
          */
         uint64_t total_pow = -1;

         /**
          * The current count of how many pending POW witnesses there are, determines the difficulty
          * of doing pow
          */
         uint32_t num_pow_witnesses = 0;

         asset       virtual_supply             = asset( 0, WORTH_SYMBOL );
         asset       current_supply             = asset( 0, WORTH_SYMBOL );
         asset       confidential_supply        = asset( 0, WORTH_SYMBOL ); ///< total asset held in confidential balances
         asset       init_wbd_supply            = asset( 0, WBD_SYMBOL );
         asset       current_wbd_supply         = asset( 0, WBD_SYMBOL );
         asset       confidential_wbd_supply    = asset( 0, WBD_SYMBOL ); ///< total asset held in confidential balances
         asset       total_vesting_fund_worth   = asset( 0, WORTH_SYMBOL );
         asset       total_vesting_shares       = asset( 0, VESTS_SYMBOL );
         asset       total_reward_fund_worth    = asset( 0, WORTH_SYMBOL );
         fc::uint128 total_reward_shares2; ///< the running total of REWARD^2
         asset       pending_rewarded_vesting_shares = asset( 0, VESTS_SYMBOL );
         asset       pending_rewarded_vesting_worth  = asset( 0, WORTH_SYMBOL );

         price       get_vesting_share_price() const
         {
            if ( total_vesting_fund_worth.amount == 0 || total_vesting_shares.amount == 0 )
               return price ( asset( 1, WORTH_SYMBOL ), asset( 10000000, VESTS_SYMBOL ) );

            return price( total_vesting_shares, total_vesting_fund_worth );
         }

         price get_reward_vesting_share_price() const
         {
            return price( total_vesting_shares + pending_rewarded_vesting_shares,
               total_vesting_fund_worth + pending_rewarded_vesting_worth );
         }

         /**
          *  This property defines the interest rate that WBD deposits receive.
          */
         uint16_t wbd_interest_rate = 0;

         uint16_t wbd_print_rate = WORTH_100_PERCENT;

         /**
          *  Maximum block size is decided by the set of active witnesses which change every round.
          *  Each witness posts what they think the maximum size should be as part of their witness
          *  properties, the median size is chosen to be the maximum block size for the round.
          *
          *  @note the minimum value for maximum_block_size is defined by the protocol to prevent the
          *  network from getting stuck by witnesses attempting to set this too low.
          */
         uint32_t     maximum_block_size = 0;

         /**
          * The size of the block that is partitioned for actions.
          * Required actions can only be delayed if they take up more than this amount. More can be
          * included, but are not required. Block generation should only include transactions up
          * to maximum_block_size - required_actions_parition_size to ensure required actions are
          * not delayed when they should not be.
          */
         uint16_t     required_actions_partition_percent = 0;

         /**
          * The current absolute slot number.  Equal to the total
          * number of slots since genesis.  Also equal to the total
          * number of missed slots plus head_block_number.
          */
         uint64_t      current_aslot = 0;

         /**
          * used to compute witness participation.
          */
         fc::uint128_t recent_slots_filled;
         uint8_t       participation_count = 0; ///< Divide by 128 to compute participation percentage

         uint32_t last_irreversible_block_num = 0;

         /**
          * The number of votes regenerated per day.  Any user voting slower than this rate will be
          * "wasting" voting power through spillover; any user voting faster than this rate will have
          * their votes reduced.
          */
         uint32_t vote_power_reserve_rate = WORTH_INITIAL_VOTE_POWER_RATE;

         uint32_t delegation_return_period = WORTH_DELEGATION_RETURN_PERIOD_HF0;

         uint64_t reverse_auction_seconds = 0;

         int64_t available_account_subsidies = 0;

         uint16_t wbd_stop_percent = 0;
         uint16_t wbd_start_percent = 0;

         //settings used to compute payments for every proposal
         time_point_sec next_maintenance_time;
         time_point_sec last_budget_time;

         uint16_t content_reward_percent = WORTH_CONTENT_REWARD_PERCENT_HF16;
         uint16_t vesting_reward_percent = WORTH_VESTING_FUND_PERCENT_HF16;
         uint16_t wps_fund_percent = WORTH_PROPOSAL_FUND_PERCENT_HF0;

         asset wps_interval_ledger = asset( 0, WBD_SYMBOL );

         uint16_t downvote_pool_percent = 0;

#ifdef WORTHEUM_ENABLE_WMT
         asset wmt_creation_fee = asset( 1000, WBD_SYMBOL );
#endif
   };

   typedef multi_index_container<
      dynamic_global_property_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< dynamic_global_property_object, dynamic_global_property_object::id_type, &dynamic_global_property_object::id > >
      >,
      allocator< dynamic_global_property_object >
   > dynamic_global_property_index;

} } // worth::chain

#ifdef ENABLE_MIRA
namespace mira {

template<> struct is_static_length< worth::chain::dynamic_global_property_object > : public boost::true_type {};

} // mira
#endif

FC_REFLECT( worth::chain::dynamic_global_property_object,
             (id)
             (head_block_number)
             (head_block_id)
             (time)
             (current_witness)
             (total_pow)
             (num_pow_witnesses)
             (virtual_supply)
             (current_supply)
             (confidential_supply)
             (init_wbd_supply)
             (current_wbd_supply)
             (confidential_wbd_supply)
             (total_vesting_fund_worth)
             (total_vesting_shares)
             (total_reward_fund_worth)
             (total_reward_shares2)
             (pending_rewarded_vesting_shares)
             (pending_rewarded_vesting_worth)
             (wbd_interest_rate)
             (wbd_print_rate)
             (maximum_block_size)
             (required_actions_partition_percent)
             (current_aslot)
             (recent_slots_filled)
             (participation_count)
             (last_irreversible_block_num)
             (vote_power_reserve_rate)
             (delegation_return_period)
             (reverse_auction_seconds)
             (available_account_subsidies)
             (wbd_stop_percent)
             (wbd_start_percent)
             (next_maintenance_time)
             (last_budget_time)
             (content_reward_percent)
             (vesting_reward_percent)
             (wps_fund_percent)
             (wps_interval_ledger)
             (downvote_pool_percent)
#ifdef WORTHEUM_ENABLE_WMT
             (wmt_creation_fee)
#endif
          )
CHAINBASE_SET_INDEX_TYPE( worth::chain::dynamic_global_property_object, worth::chain::dynamic_global_property_index )
