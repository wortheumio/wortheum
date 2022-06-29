#include <worth/chain/worth_fwd.hpp>

#include <worth/chain/worth_evaluator.hpp>
#include <worth/chain/database.hpp>
#include <worth/chain/worth_objects.hpp>
#include <worth/chain/wmt_objects.hpp>
#include <worth/chain/util/reward.hpp>
#include <worth/chain/util/wmt_token.hpp>

#include <worth/protocol/wmt_operations.hpp>

#ifdef WORTHEUM_ENABLE_WMT
namespace worth { namespace chain {

namespace {

/// Return WMT token object controlled by this account identified by its symbol. Throws assert exception when not found!
inline const wmt_token_object& get_controlled_wmt( const database& db, const account_name_type& control_account, const asset_symbol_type& wmt_symbol )
{
   const wmt_token_object* wmt = db.find< wmt_token_object, by_symbol >( wmt_symbol );
   // The WMT is supposed to be found.
   FC_ASSERT( wmt != nullptr, "WMT numerical asset identifier ${wmt} not found", ("wmt", wmt_symbol.to_nai()) );
   // Check against unotherized account trying to access (and alter) WMT. Unotherized means "other than the one that created the WMT".
   FC_ASSERT( wmt->control_account == control_account, "The account ${account} does NOT control the WMT ${wmt}",
      ("account", control_account)("wmt", wmt_symbol.to_nai()) );

   return *wmt;
}

}

namespace {

class wmt_setup_parameters_visitor : public fc::visitor<bool>
{
public:
   wmt_setup_parameters_visitor( wmt_token_object& wmt_token ) : _wmt_token( wmt_token ) {}

   bool operator () ( const wmt_param_allow_voting& allow_voting )
   {
      _wmt_token.allow_voting = allow_voting.value;
      return true;
   }

private:
   wmt_token_object& _wmt_token;
};

const wmt_token_object& common_pre_setup_evaluation(
   const database& _db, const asset_symbol_type& symbol, const account_name_type& control_account )
{
   const wmt_token_object& wmt = get_controlled_wmt( _db, control_account, symbol );

   // Check whether it's not too late to setup emissions operation.
   FC_ASSERT( wmt.phase < wmt_phase::setup_completed, "WMT pre-setup operation no longer allowed after setup phase is over" );

   return wmt;
}

} // namespace

void wmt_create_evaluator::do_apply( const wmt_create_operation& o )
{
   FC_ASSERT( _db.has_hardfork( WORTH_WMT_HARDFORK ), "WMT functionality not enabled until hardfork ${hf}", ("hf", WORTH_WMT_HARDFORK) );
   const dynamic_global_property_object& dgpo = _db.get_dynamic_global_properties();

   auto token_ptr = util::wmt::find_token( _db, o.symbol, true );

   if ( o.wmt_creation_fee.amount > 0 ) // Creation case
   {
      FC_ASSERT( token_ptr == nullptr, "WMT ${nai} has already been created.", ("nai", o.symbol.to_nai() ) );
      FC_ASSERT( _db.get< nai_pool_object >().contains( o.symbol ), "Cannot create an WMT that didn't come from the NAI pool." );

      asset creation_fee;

      if( o.wmt_creation_fee.symbol == dgpo.wmt_creation_fee.symbol )
      {
         creation_fee = o.wmt_creation_fee;
      }
      else
      {
         const auto& fhistory = _db.get_feed_history();
         FC_ASSERT( !fhistory.current_median_history.is_null(), "Cannot pay the fee using different asset symbol because there is no price feed." );

         if( dgpo.wmt_creation_fee.symbol == WORTH_SYMBOL )
            creation_fee = _db.to_worth( o.wmt_creation_fee );
         else
            creation_fee = _db.to_wbd( o.wmt_creation_fee );
      }

      FC_ASSERT( creation_fee == dgpo.wmt_creation_fee,
         "Fee of ${ef} does not match the creation fee of ${sf}", ("ef", creation_fee)("sf", dgpo.wmt_creation_fee) );

      _db.adjust_balance( o.control_account , -o.wmt_creation_fee );
      _db.adjust_balance( WORTH_NULL_ACCOUNT,  o.wmt_creation_fee );
   }
   else // Reset case
   {
      FC_ASSERT( token_ptr != nullptr, "Cannot reset a non-existent WMT. Did you forget to specify the creation fee?" );
      FC_ASSERT( token_ptr->control_account == o.control_account, "You do not control this WMT. Control Account: ${a}", ("a", token_ptr->control_account) );
      FC_ASSERT( token_ptr->phase == wmt_phase::account_elevated, "WMT cannot be reset if setup is completed. Phase: ${p}", ("p", token_ptr->phase) );
      FC_ASSERT( !util::wmt::last_emission_time( _db, token_ptr->liquid_symbol ), "Cannot reset an WMT that has existing token emissions." );

      _db.remove( *token_ptr );
   }

   // Create WMT object common to both liquid and vesting variants of WMT.
   _db.create< wmt_token_object >( [&]( wmt_token_object& token )
   {
      token.liquid_symbol = o.symbol;
      token.control_account = o.control_account;
      token.market_maker.token_balance = asset( 0, token.liquid_symbol );
   });

   remove_from_nai_pool( _db, o.symbol );

   if ( !_db.is_pending_tx() )
      replenish_nai_pool( _db );
}

struct wmt_setup_evaluator_visitor
{
   const wmt_ico_object& _ico;
   database& _db;

   wmt_setup_evaluator_visitor( const wmt_ico_object& ico, database& db ): _ico( ico ), _db( db ){}

   typedef void result_type;

   void operator()( const wmt_capped_generation_policy& capped_generation_policy ) const
   {
      _db.modify( _ico, [&]( wmt_ico_object& ico )
      {
         ico.capped_generation_policy = capped_generation_policy;
      });
   }
};

void wmt_setup_evaluator::do_apply( const wmt_setup_operation& o )
{
   FC_ASSERT( _db.has_hardfork( WORTH_WMT_HARDFORK ), "WMT functionality not enabled until hardfork ${hf}", ("hf", WORTH_WMT_HARDFORK) );
#pragma message ("TODO: Adjust assertion below and add/modify negative tests appropriately.")
   const auto* _token = _db.find< wmt_token_object, by_symbol >( o.symbol );
   FC_ASSERT( _token, "WMT ${ac} not elevated yet.",("ac", o.control_account) );

   _db.modify(  *_token, [&]( wmt_token_object& token )
   {
#pragma message ("TODO: Add/modify test to check the token phase correctly set.")
      token.phase = wmt_phase::setup_completed;
      token.control_account = o.control_account;
      token.max_supply = o.max_supply;
   } );

   auto token_ico = _db.create< wmt_ico_object >( [&] ( wmt_ico_object& token_ico_obj )
   {
      token_ico_obj.symbol = _token->liquid_symbol;
      token_ico_obj.contribution_begin_time = o.contribution_begin_time;
      token_ico_obj.contribution_end_time = o.contribution_end_time;
      token_ico_obj.launch_time = o.launch_time;
      token_ico_obj.worth_units_soft_cap = o.worth_units_soft_cap;
      token_ico_obj.worth_units_hard_cap = o.worth_units_hard_cap;
   } );

   wmt_setup_evaluator_visitor visitor( token_ico, _db );
   o.initial_generation_policy.visit( visitor );
}

void wmt_setup_emissions_evaluator::do_apply( const wmt_setup_emissions_operation& o )
{
   FC_ASSERT( _db.has_hardfork( WORTH_WMT_HARDFORK ), "WMT functionality not enabled until hardfork ${hf}", ("hf", WORTH_WMT_HARDFORK) );

   const wmt_token_object& wmt = common_pre_setup_evaluation( _db, o.symbol, o.control_account );

   if ( o.remove )
   {
      auto last_emission = util::wmt::last_emission( _db, o.symbol );
      FC_ASSERT( last_emission != nullptr, "Could not find token emission for the given WMT: ${wmt}", ("wmt", o.symbol) );

      FC_ASSERT(
         last_emission->symbol == o.symbol &&
         last_emission->schedule_time == o.schedule_time &&
         last_emission->emissions_unit.token_unit == o.emissions_unit.token_unit &&
         last_emission->interval_seconds == o.interval_seconds &&
         last_emission->interval_count == o.interval_count &&
         last_emission->lep_time == o.lep_time &&
         last_emission->rep_time == o.rep_time &&
         last_emission->lep_abs_amount == o.lep_abs_amount &&
         last_emission->rep_abs_amount == o.rep_abs_amount &&
         last_emission->lep_rel_amount_numerator == o.lep_rel_amount_numerator &&
         last_emission->rep_rel_amount_numerator == o.rep_rel_amount_numerator &&
         last_emission->rel_amount_denom_bits == o.rel_amount_denom_bits,
         "WMT emissions must be removed from latest to earliest, last emission: ${le}. Current: ${c}", ("le", *last_emission)("c", o)
      );

      _db.remove( *last_emission );
   }
   else
   {
      FC_ASSERT( o.schedule_time > _db.head_block_time(), "Emissions schedule time must be in the future" );

      auto end_time = util::wmt::last_emission_time( _db, wmt.liquid_symbol );

      if ( end_time.valid() )
      {
         FC_ASSERT( o.schedule_time > *end_time,
            "WMT emissions cannot overlap with existing ranges and must be in chronological order, last emission time: ${end}",
            ("end", *end_time) );
      }

      _db.create< wmt_token_emissions_object >( [&]( wmt_token_emissions_object& eo )
      {
         eo.symbol = wmt.liquid_symbol;
         eo.schedule_time = o.schedule_time;
         eo.emissions_unit = o.emissions_unit;
         eo.interval_seconds = o.interval_seconds;
         eo.interval_count = o.interval_count;
         eo.lep_time = o.lep_time;
         eo.rep_time = o.rep_time;
         eo.lep_abs_amount = o.lep_abs_amount;
         eo.rep_abs_amount = o.rep_abs_amount;
         eo.lep_rel_amount_numerator = o.lep_rel_amount_numerator;
         eo.rep_rel_amount_numerator = o.rep_rel_amount_numerator;
         eo.rel_amount_denom_bits = o.rel_amount_denom_bits;
      });
   }
}

void wmt_set_setup_parameters_evaluator::do_apply( const wmt_set_setup_parameters_operation& o )
{
   FC_ASSERT( _db.has_hardfork( WORTH_WMT_HARDFORK ), "WMT functionality not enabled until hardfork ${hf}", ("hf", WORTH_WMT_HARDFORK) );

   const wmt_token_object& wmt_token = common_pre_setup_evaluation( _db, o.symbol, o.control_account );

   _db.modify( wmt_token, [&]( wmt_token_object& token )
   {
      wmt_setup_parameters_visitor visitor( token );

      for ( auto& param : o.setup_parameters )
         param.visit( visitor );
   });
}

struct wmt_set_runtime_parameters_evaluator_visitor
{
   wmt_token_object& _token;

   wmt_set_runtime_parameters_evaluator_visitor( wmt_token_object& token ) : _token( token ) {}

   typedef void result_type;

   void operator()( const wmt_param_windows_v1& param_windows )const
   {
      _token.cashout_window_seconds = param_windows.cashout_window_seconds;
      _token.reverse_auction_window_seconds = param_windows.reverse_auction_window_seconds;
   }

   void operator()( const wmt_param_vote_regeneration_period_seconds_v1& vote_regeneration )const
   {
      _token.vote_regeneration_period_seconds = vote_regeneration.vote_regeneration_period_seconds;
      _token.votes_per_regeneration_period = vote_regeneration.votes_per_regeneration_period;
   }

   void operator()( const wmt_param_rewards_v1& param_rewards )const
   {
      _token.content_constant = param_rewards.content_constant;
      _token.percent_curation_rewards = param_rewards.percent_curation_rewards;
      _token.author_reward_curve = param_rewards.author_reward_curve;
      _token.curation_reward_curve = param_rewards.curation_reward_curve;
   }

   void operator()( const wmt_param_allow_downvotes& param_allow_downvotes )const
   {
      _token.allow_downvotes = param_allow_downvotes.value;
   }
};

void wmt_set_runtime_parameters_evaluator::do_apply( const wmt_set_runtime_parameters_operation& o )
{
   FC_ASSERT( _db.has_hardfork( WORTH_WMT_HARDFORK ), "WMT functionality not enabled until hardfork ${hf}", ("hf", WORTH_WMT_HARDFORK) );

   const wmt_token_object& token = common_pre_setup_evaluation(_db, o.symbol, o.control_account);

   _db.modify( token, [&]( wmt_token_object& t )
   {
      wmt_set_runtime_parameters_evaluator_visitor visitor( t );

      for( auto& param: o.runtime_parameters )
         param.visit( visitor );
   });
}

void wmt_contribute_evaluator::do_apply( const wmt_contribute_operation& o )
{
   try
   {
      FC_ASSERT( _db.has_hardfork( WORTH_WMT_HARDFORK ), "WMT functionality not enabled until hardfork ${hf}", ("hf", WORTH_WMT_HARDFORK) );

      const wmt_token_object* token = util::wmt::find_token( _db, o.symbol );
      FC_ASSERT( token != nullptr, "Cannot contribute to an unknown WMT" );
      FC_ASSERT( token->phase >= wmt_phase::contribution_begin_time_completed, "WMT has yet to enter the contribution phase" );
      FC_ASSERT( token->phase < wmt_phase::contribution_end_time_completed, "WMT is no longer in the contribution phase" );

      const wmt_ico_object* token_ico = _db.find< wmt_ico_object, by_symbol >( token->liquid_symbol );
      FC_ASSERT( token_ico != nullptr, "Unable to find ICO data for symbol: ${sym}", ("sym", token->liquid_symbol) );
      FC_ASSERT( token_ico->contributed.amount < token_ico->worth_units_hard_cap, "WMT ICO has reached its hard cap and no longer accepts contributions" );
      FC_ASSERT( token_ico->contributed.amount + o.contribution.amount <= token_ico->worth_units_hard_cap,
         "The proposed contribution would exceed the ICO hard cap, maximum possible contribution: ${c}",
         ("c", asset( token_ico->worth_units_hard_cap - token_ico->contributed.amount, WORTH_SYMBOL )) );

      auto key = boost::tuple< asset_symbol_type, account_name_type, uint32_t >( o.contribution.symbol, o.contributor, o.contribution_id );
      auto contrib_ptr = _db.find< wmt_contribution_object, by_symbol_contributor >( key );
      FC_ASSERT( contrib_ptr == nullptr, "The provided contribution ID must be unique. Current: ${id}", ("id", o.contribution_id) );

      _db.adjust_balance( o.contributor, -o.contribution );

      _db.create< wmt_contribution_object >( [&] ( wmt_contribution_object& obj )
      {
         obj.contributor = o.contributor;
         obj.symbol = o.symbol;
         obj.contribution_id = o.contribution_id;
         obj.contribution = o.contribution;
      } );

      _db.modify( *token_ico, [&]( wmt_ico_object& ico )
      {
         ico.contributed += o.contribution;
      } );
   }
   FC_CAPTURE_AND_RETHROW( (o) )
}

} }
#endif
