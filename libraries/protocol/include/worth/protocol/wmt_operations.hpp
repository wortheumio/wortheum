#pragma once

#include <worth/protocol/base.hpp>
#include <worth/protocol/asset.hpp>
#include <worth/protocol/misc_utilities.hpp>

#ifdef WORTHEUM_ENABLE_WMT

#define WMT_MAX_UNIT_ROUTES            10
#define WMT_MAX_UNIT_COUNT             20
#define WMT_MAX_DECIMAL_PLACES         8
#define WMT_MIN_HARD_CAP_WORTH_UNITS   10000
#define WMT_MIN_SATURATION_WORTH_UNITS 1000
#define WMT_MIN_SOFT_CAP_WORTH_UNITS   1000

namespace worth { namespace protocol {


/**
 * This operation introduces new WMT into blockchain as identified by
 * Numerical Asset Identifier (NAI). Also the WMT precision (decimal points)
 * is explicitly provided.
 */
struct wmt_create_operation : public base_operation
{
   account_name_type control_account;
   asset_symbol_type symbol;

   /// The amount to be transfered from @account to null account as elevation fee.
   asset             wmt_creation_fee;
   /// Separately provided precision for clarity and redundancy.
   uint8_t           precision;

   extensions_type   extensions;

   void validate()const;

   void get_required_active_authorities( flat_set<account_name_type>& a )const
   { a.insert( control_account ); }
};

struct wmt_generation_unit
{
   flat_map< account_name_type, uint16_t >        worth_unit;
   flat_map< account_name_type, uint16_t >        token_unit;

   uint32_t worth_unit_sum()const;
   uint32_t token_unit_sum()const;

   void validate()const;
};

struct wmt_capped_generation_policy
{
   wmt_generation_unit pre_soft_cap_unit;
   wmt_generation_unit post_soft_cap_unit;

   uint16_t            soft_cap_percent = 0;

   uint32_t            min_unit_ratio = 0;
   uint32_t            max_unit_ratio = 0;

   extensions_type     extensions;

   void validate()const;
};

typedef static_variant<
   wmt_capped_generation_policy
   > wmt_generation_policy;

struct wmt_setup_operation : public base_operation
{
   account_name_type control_account;
   asset_symbol_type symbol;

   int64_t                 max_supply = WORTH_MAX_SHARE_SUPPLY;

   wmt_generation_policy   initial_generation_policy;

   time_point_sec          contribution_begin_time;
   time_point_sec          contribution_end_time;
   time_point_sec          launch_time;

   share_type              worth_units_soft_cap;
   share_type              worth_units_hard_cap;

   extensions_type         extensions;

   void validate()const;

   void get_required_active_authorities( flat_set<account_name_type>& a )const
   { a.insert( control_account ); }
};

struct wmt_emissions_unit
{
   flat_map< account_name_type, uint16_t >        token_unit;
};

struct wmt_setup_emissions_operation : public base_operation
{
   account_name_type   control_account;
   asset_symbol_type   symbol;

   time_point_sec      schedule_time;
   wmt_emissions_unit  emissions_unit;

   uint32_t            interval_seconds = 0;
   uint32_t            interval_count = 0;

   time_point_sec      lep_time;
   time_point_sec      rep_time;

   asset               lep_abs_amount;
   asset               rep_abs_amount;
   uint32_t            lep_rel_amount_numerator = 0;
   uint32_t            rep_rel_amount_numerator = 0;

   uint8_t             rel_amount_denom_bits = 0;
   bool                remove = false;

   extensions_type     extensions;

   void validate()const;

   void get_required_active_authorities( flat_set<account_name_type>& a )const
   { a.insert( control_account ); }
};

struct wmt_param_allow_voting
{
   bool value = true;
};

typedef static_variant<
   wmt_param_allow_voting
   > wmt_setup_parameter;

struct wmt_param_windows_v1
{
   uint32_t cashout_window_seconds = 0;                // WORTH_CASHOUT_WINDOW_SECONDS
   uint32_t reverse_auction_window_seconds = 0;        // WORTH_REVERSE_AUCTION_WINDOW_SECONDS
};

struct wmt_param_vote_regeneration_period_seconds_v1
{
   uint32_t vote_regeneration_period_seconds = 0;      // WORTH_VOTING_MANA_REGENERATION_SECONDS
   uint32_t votes_per_regeneration_period = 0;
};

struct wmt_param_rewards_v1
{
   uint128_t               content_constant = 0;
   uint16_t                percent_curation_rewards = 0;
   protocol::curve_id      author_reward_curve;
   protocol::curve_id      curation_reward_curve;
};

struct wmt_param_allow_downvotes
{
   bool value = true;
};

typedef static_variant<
   wmt_param_windows_v1,
   wmt_param_vote_regeneration_period_seconds_v1,
   wmt_param_rewards_v1,
   wmt_param_allow_downvotes
   > wmt_runtime_parameter;

struct wmt_set_setup_parameters_operation : public base_operation
{
   account_name_type                control_account;
   asset_symbol_type                symbol;
   flat_set< wmt_setup_parameter >  setup_parameters;
   extensions_type                  extensions;

   void validate()const;

   void get_required_active_authorities( flat_set<account_name_type>& a )const
   { a.insert( control_account ); }
};

struct wmt_set_runtime_parameters_operation : public base_operation
{
   account_name_type                   control_account;
   asset_symbol_type                   symbol;
   flat_set< wmt_runtime_parameter >   runtime_parameters;
   extensions_type                     extensions;

   void validate()const;

   void get_required_active_authorities( flat_set<account_name_type>& a )const
   { a.insert( control_account ); }
};

struct wmt_contribute_operation : public base_operation
{
   account_name_type  contributor;
   asset_symbol_type  symbol;
   uint32_t           contribution_id;
   asset              contribution;
   extensions_type    extensions;

   void validate() const;
   void get_required_active_authorities( flat_set<account_name_type>& a )const
   { a.insert( contributor ); }
};

} }

FC_REFLECT(
   worth::protocol::wmt_create_operation,
   (control_account)
   (symbol)
   (wmt_creation_fee)
   (extensions)
)

FC_REFLECT(
   worth::protocol::wmt_setup_operation,
   (control_account)
   (symbol)
   (max_supply)
   (initial_generation_policy)
   (contribution_begin_time)
   (contribution_end_time)
   (launch_time)
   (worth_units_soft_cap)
   (worth_units_hard_cap)
   (extensions)
   )

FC_REFLECT(
   worth::protocol::wmt_generation_unit,
   (worth_unit)
   (token_unit)
   )


FC_REFLECT(
   worth::protocol::wmt_capped_generation_policy,
   (pre_soft_cap_unit)
   (post_soft_cap_unit)
   (soft_cap_percent)
   (min_unit_ratio)
   (max_unit_ratio)
   (extensions)
   )

FC_REFLECT(
   worth::protocol::wmt_emissions_unit,
   (token_unit)
   )

FC_REFLECT(
   worth::protocol::wmt_setup_emissions_operation,
   (control_account)
   (symbol)
   (schedule_time)
   (emissions_unit)
   (interval_seconds)
   (interval_count)
   (lep_time)
   (rep_time)
   (lep_abs_amount)
   (rep_abs_amount)
   (lep_rel_amount_numerator)
   (rep_rel_amount_numerator)
   (rel_amount_denom_bits)
   (remove)
   (extensions)
   )

FC_REFLECT(
   worth::protocol::wmt_param_allow_voting,
   (value)
   )

FC_REFLECT_TYPENAME( worth::protocol::wmt_setup_parameter )

FC_REFLECT(
   worth::protocol::wmt_param_windows_v1,
   (cashout_window_seconds)
   (reverse_auction_window_seconds)
   )

FC_REFLECT(
   worth::protocol::wmt_param_vote_regeneration_period_seconds_v1,
   (vote_regeneration_period_seconds)
   (votes_per_regeneration_period)
   )

FC_REFLECT(
   worth::protocol::wmt_param_rewards_v1,
   (content_constant)
   (percent_curation_rewards)
   (author_reward_curve)
   (curation_reward_curve)
   )

FC_REFLECT(
   worth::protocol::wmt_param_allow_downvotes,
   (value)
)

FC_REFLECT_TYPENAME(
   worth::protocol::wmt_runtime_parameter
   )

FC_REFLECT(
   worth::protocol::wmt_set_setup_parameters_operation,
   (control_account)
   (symbol)
   (setup_parameters)
   (extensions)
   )

FC_REFLECT(
   worth::protocol::wmt_set_runtime_parameters_operation,
   (control_account)
   (symbol)
   (runtime_parameters)
   (extensions)
   )

FC_REFLECT(
   worth::protocol::wmt_contribute_operation,
   (contributor)
   (symbol)
   (contribution_id)
   (contribution)
   (extensions)
   )

#endif
