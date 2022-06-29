
#include <worth/protocol/wmt_operations.hpp>
#include <worth/protocol/validation.hpp>
#ifdef WORTHEUM_ENABLE_WMT

#define WMT_DESTINATION_FROM          account_name_type( "$from" )
#define WMT_DESTINATION_FROM_VESTING  account_name_type( "$from.vesting" )
#define WMT_DESTINATION_MARKET_MAKER  account_name_type( "$market_maker" )
#define WMT_DESTINATION_REWARDS       account_name_type( "$rewards" )
#define WMT_DESTINATION_VESTING       account_name_type( "$vesting" )

namespace worth { namespace protocol {

void common_symbol_validation( const asset_symbol_type& symbol )
{
   symbol.validate();
   FC_ASSERT( symbol.space() == asset_symbol_type::wmt_nai_space, "legacy symbol used instead of NAI" );
   FC_ASSERT( symbol.is_vesting() == false, "liquid variant of NAI expected");
}

template < class Operation >
void wmt_admin_operation_validate( const Operation& o )
{
   validate_account_name( o.control_account );
   common_symbol_validation( o.symbol );
}

void wmt_create_operation::validate()const
{
   wmt_admin_operation_validate( *this );
   FC_ASSERT( wmt_creation_fee.amount >= 0, "fee cannot be negative" );
   FC_ASSERT( wmt_creation_fee.amount <= WORTH_MAX_SHARE_SUPPLY, "Fee must be smaller than WORTH_MAX_SHARE_SUPPLY" );
   FC_ASSERT( is_asset_type( wmt_creation_fee, WORTH_SYMBOL ) || is_asset_type( wmt_creation_fee, WBD_SYMBOL ), "Fee must be WORTH or WBD" );
   FC_ASSERT( symbol.decimals() == precision, "Mismatch between redundantly provided precision ${prec1} vs ${prec2}",
      ("prec1",symbol.decimals())("prec2",precision) );
}

bool is_valid_unit_target( const account_name_type& name )
{
   if( is_valid_account_name(name) )
      return true;
   if( name == account_name_type("$from") )
      return true;
   if( name == account_name_type("$from.vesting") )
      return true;
   return false;
}

bool is_valid_wmt_emissions_unit_destination( const account_name_type& name )
{
   if ( is_valid_account_name( name ) )
      return true;
   if ( name == WMT_DESTINATION_REWARDS )
      return true;
   if ( name == WMT_DESTINATION_VESTING )
      return true;
   if ( name == WMT_DESTINATION_MARKET_MAKER )
      return true;
   return false;
}

uint32_t wmt_generation_unit::worth_unit_sum()const
{
   uint32_t result = 0;
   for(const std::pair< account_name_type, uint16_t >& e : worth_unit )
      result += e.second;
   return result;
}

uint32_t wmt_generation_unit::token_unit_sum()const
{
   uint32_t result = 0;
   for(const std::pair< account_name_type, uint16_t >& e : token_unit )
      result += e.second;
   return result;
}

void wmt_generation_unit::validate()const
{
   FC_ASSERT( worth_unit.size() <= WMT_MAX_UNIT_ROUTES );
   for(const std::pair< account_name_type, uint16_t >& e : worth_unit )
   {
      FC_ASSERT( is_valid_unit_target( e.first ) );
      FC_ASSERT( e.second > 0 );
   }
   FC_ASSERT( token_unit.size() <= WMT_MAX_UNIT_ROUTES );
   for(const std::pair< account_name_type, uint16_t >& e : token_unit )
   {
      FC_ASSERT( is_valid_unit_target( e.first ) );
      FC_ASSERT( e.second > 0 );
   }
}

void wmt_capped_generation_policy::validate()const
{
   pre_soft_cap_unit.validate();
   post_soft_cap_unit.validate();

   FC_ASSERT( soft_cap_percent > 0 );
   FC_ASSERT( soft_cap_percent <= WORTH_100_PERCENT );

   FC_ASSERT( pre_soft_cap_unit.worth_unit.size() > 0 );
   FC_ASSERT( pre_soft_cap_unit.token_unit.size() > 0 );

   FC_ASSERT( pre_soft_cap_unit.worth_unit.size() <= WMT_MAX_UNIT_COUNT );
   FC_ASSERT( pre_soft_cap_unit.token_unit.size() <= WMT_MAX_UNIT_COUNT );
   FC_ASSERT( post_soft_cap_unit.worth_unit.size() <= WMT_MAX_UNIT_COUNT );
   FC_ASSERT( post_soft_cap_unit.token_unit.size() <= WMT_MAX_UNIT_COUNT );

   // TODO : Check account name

   if( soft_cap_percent == WORTH_100_PERCENT )
   {
      FC_ASSERT( post_soft_cap_unit.worth_unit.size() == 0 );
      FC_ASSERT( post_soft_cap_unit.token_unit.size() == 0 );
   }
   else
   {
      FC_ASSERT( post_soft_cap_unit.worth_unit.size() > 0 );
   }
}

struct validate_visitor
{
   typedef void result_type;

   template< typename T >
   void operator()( const T& x )
   {
      x.validate();
   }
};

void wmt_setup_emissions_operation::validate()const
{
   wmt_admin_operation_validate( *this );

   FC_ASSERT( schedule_time > WORTH_GENESIS_TIME );
   FC_ASSERT( emissions_unit.token_unit.empty() == false, "Emissions token unit cannot be empty" );

   for ( const auto& e : emissions_unit.token_unit )
   {
      FC_ASSERT( is_valid_wmt_emissions_unit_destination( e.first ),
         "Emissions token unit destination ${n} is invalid", ("n", e.first) );
      FC_ASSERT( e.second > 0, "Emissions token unit must be greater than 0" );
   }

   FC_ASSERT( interval_seconds >= WMT_EMISSION_MIN_INTERVAL_SECONDS,
      "Interval seconds must be greater than or equal to ${seconds}", ("seconds", WMT_EMISSION_MIN_INTERVAL_SECONDS) );

   FC_ASSERT( interval_count > 0, "Interval count must be greater than 0" );

   FC_ASSERT( lep_time <= rep_time, "Left endpoint time must be less than or equal to right endpoint time" );

   // If time modulation is enabled
   if ( lep_time != rep_time )
   {
      FC_ASSERT( lep_time >= schedule_time, "Left endpoint time cannot be before the schedule time" );

      // If we don't emit indefinitely
      if ( interval_count != WMT_EMIT_INDEFINITELY )
      {
         FC_ASSERT(
            uint64_t( interval_seconds ) * uint64_t( interval_count ) + uint64_t( schedule_time.sec_since_epoch() ) <= std::numeric_limits< int32_t >::max(),
            "Schedule end time overflow" );
         FC_ASSERT( rep_time <= schedule_time + fc::seconds( uint64_t( interval_seconds ) * uint64_t( interval_count ) ),
            "Right endpoint time cannot be after the schedule end time" );
      }
   }

   FC_ASSERT( symbol.is_vesting() == false, "Use liquid variant of WMT symbol to specify emission amounts" );
   FC_ASSERT( symbol == lep_abs_amount.symbol, "Left endpoint symbol mismatch" );
   FC_ASSERT( symbol == rep_abs_amount.symbol, "Right endpoint symbol mismatch" );
   FC_ASSERT( lep_abs_amount.amount >= 0, "Left endpoint cannot have negative emission" );
   FC_ASSERT( rep_abs_amount.amount >= 0, "Right endpoint cannot have negative emission" );

   FC_ASSERT( lep_abs_amount.amount > 0 || lep_rel_amount_numerator > 0 || rep_abs_amount.amount > 0 || rep_rel_amount_numerator > 0,
      "An emission operation must have positive non-zero emission" );

   // rel_amount_denom_bits <- any value of unsigned int is OK
}

void wmt_setup_operation::validate()const
{
   wmt_admin_operation_validate( *this );

   FC_ASSERT( max_supply > 0, "Max supply must be greater than 0" );
   FC_ASSERT( max_supply <= WORTH_MAX_SHARE_SUPPLY, "Max supply must be less than ${n}", ("n", WORTH_MAX_SHARE_SUPPLY) );
   FC_ASSERT( contribution_begin_time > WORTH_GENESIS_TIME, "Contribution begin time must be greater than ${t}", ("t", WORTH_GENESIS_TIME) );
   FC_ASSERT( contribution_end_time > contribution_begin_time, "Contribution end time must be after contribution begin time" );
   FC_ASSERT( launch_time >= contribution_end_time, "WMT ICO launch time must be after the contribution end time" );
   FC_ASSERT( worth_units_soft_cap <= worth_units_hard_cap, "Worth units soft cap must less than or equal to worth units hard cap" );
   FC_ASSERT( worth_units_soft_cap >= WMT_MIN_SOFT_CAP_WORTH_UNITS, "Worth units soft cap must be greater than or equal to ${n}", ("n", WMT_MIN_SOFT_CAP_WORTH_UNITS) );
   FC_ASSERT( worth_units_hard_cap >= WMT_MIN_HARD_CAP_WORTH_UNITS, "Worth units hard cap must be greater than or equal to ${n}", ("n", WMT_MIN_HARD_CAP_WORTH_UNITS) );
   FC_ASSERT( worth_units_hard_cap <= WORTH_MAX_SHARE_SUPPLY, "Worth units hard cap must be less than or equal to ${n}", ("n", WORTH_MAX_SHARE_SUPPLY) );

   validate_visitor vtor;
   initial_generation_policy.visit( vtor );
}

struct wmt_set_runtime_parameters_operation_visitor
{
   wmt_set_runtime_parameters_operation_visitor(){}

   typedef void result_type;

   void operator()( const wmt_param_windows_v1& param_windows )const
   {
      // 0 <= reverse_auction_window_seconds + WMT_UPVOTE_LOCKOUT < cashout_window_seconds < WMT_VESTING_WITHDRAW_INTERVAL_SECONDS
      uint64_t sum = ( param_windows.reverse_auction_window_seconds + WMT_UPVOTE_LOCKOUT );

      FC_ASSERT( sum < param_windows.cashout_window_seconds,
         "'reverse auction window + upvote lockout' interval must be less than cashout window (${c}). Was ${sum} seconds.",
         ("c", param_windows.cashout_window_seconds)
         ( "sum", sum ) );

      FC_ASSERT( param_windows.cashout_window_seconds <= WMT_VESTING_WITHDRAW_INTERVAL_SECONDS,
         "Cashout window second must be less than 'vesting withdraw' interval (${v}). Was ${val} seconds.",
         ("v", WMT_VESTING_WITHDRAW_INTERVAL_SECONDS)
         ("val", param_windows.cashout_window_seconds) );
   }

   void operator()( const wmt_param_vote_regeneration_period_seconds_v1& vote_regeneration )const
   {
      // 0 < vote_regeneration_seconds < WMT_VESTING_WITHDRAW_INTERVAL_SECONDS
      FC_ASSERT( vote_regeneration.vote_regeneration_period_seconds > 0 &&
         vote_regeneration.vote_regeneration_period_seconds <= WMT_VESTING_WITHDRAW_INTERVAL_SECONDS,
         "Vote regeneration period must be greater than 0 and less than 'vesting withdraw' (${v}) interval. Was ${val} seconds.",
         ("v", WMT_VESTING_WITHDRAW_INTERVAL_SECONDS )
         ("val", vote_regeneration.vote_regeneration_period_seconds) );

      FC_ASSERT( vote_regeneration.votes_per_regeneration_period > 0 &&
         vote_regeneration.votes_per_regeneration_period <= WMT_MAX_VOTES_PER_REGENERATION,
         "Votes per regeneration period exceeds maximum (${max}). Was ${v}",
         ("max", WMT_MAX_VOTES_PER_REGENERATION)
         ("v", vote_regeneration.vote_regeneration_period_seconds) );

      // With previous assertion, this value will not overflow a 32 bit integer
      // This calculation is designed to round up
      uint32_t nominal_votes_per_day = ( vote_regeneration.votes_per_regeneration_period * 86400 + vote_regeneration.vote_regeneration_period_seconds - 1 )
         / vote_regeneration.vote_regeneration_period_seconds;

      FC_ASSERT( nominal_votes_per_day <= WMT_MAX_NOMINAL_VOTES_PER_DAY,
         "Nominal votes per day exceeds maximum (${max}). Was ${v}",
         ("max", WMT_MAX_NOMINAL_VOTES_PER_DAY)
         ("v", nominal_votes_per_day) );
   }

   void operator()( const wmt_param_rewards_v1& param_rewards )const
   {
      FC_ASSERT( param_rewards.percent_curation_rewards <= WORTH_100_PERCENT,
         "Percent Curation Rewards must not exceed 10000. Was ${n}",
         ("n", param_rewards.percent_curation_rewards) );

      switch( param_rewards.author_reward_curve )
      {
         case linear:
         case quadratic:
            break;
         default:
            FC_ASSERT( false, "Author Reward Curve must be linear or quadratic" );
      }

      switch( param_rewards.curation_reward_curve )
      {
         case linear:
         case square_root:
         case bounded_curation:
            break;
         default:
            FC_ASSERT( false, "Curation Reward Curve must be linear, square_root, or bounded_curation." );
      }
   }

   void operator()( const wmt_param_allow_downvotes& )const
   {
      //Nothing to do
   }
};

void wmt_set_runtime_parameters_operation::validate()const
{
   wmt_admin_operation_validate( *this );
   FC_ASSERT( !runtime_parameters.empty() );

   wmt_set_runtime_parameters_operation_visitor visitor;
   for( auto& param: runtime_parameters )
      param.visit( visitor );
}

void wmt_set_setup_parameters_operation::validate() const
{
   wmt_admin_operation_validate( *this );
   FC_ASSERT( setup_parameters.empty() == false );
}

void wmt_contribute_operation::validate() const
{
   validate_account_name( contributor );
   common_symbol_validation( symbol );
   FC_ASSERT( contribution.symbol == WORTH_SYMBOL, "Contributions must be made in WORTH" );
   FC_ASSERT( contribution.amount > 0, "Contribution amount must be greater than 0" );
}

} }
#endif
