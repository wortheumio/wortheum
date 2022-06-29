#pragma once

#include <worth/chain/wps_objects.hpp>
#include <worth/chain/notifications.hpp>
#include <worth/chain/database.hpp>
#include <worth/chain/index.hpp>
#include <worth/chain/account_object.hpp>

#include <worth/chain/util/wps_helper.hpp>

#include <worth/protocol/wps_operations.hpp>

namespace worth { namespace chain {

class wps_processor
{
   public:

      using t_proposals = std::vector< std::reference_wrapper< const proposal_object > >;

   private:

      const static std::string removing_name;
      const static std::string calculating_name;
      const static uint32_t total_amount_divider = 100;

      //Get number of microseconds for 1 day( daily_ms )
      const int64_t daily_seconds = fc::days(1).to_seconds();

      chain::database& db;

      bool is_maintenance_period( const time_point_sec& head_time ) const;

      void remove_proposals( const time_point_sec& head_time );

      void find_active_proposals( const time_point_sec& head_time, t_proposals& proposals );

      uint64_t calculate_votes( const proposal_id_type& id );
      void calculate_votes( const t_proposals& proposals );

      void sort_by_votes( t_proposals& proposals );

      asset get_treasury_fund();

      asset get_daily_inflation();

      asset calculate_maintenance_budget( const time_point_sec& head_time );

      void transfer_daily_inflation_to_treasury( const asset& daily_inflation );

      void transfer_payments( const time_point_sec& head_time, asset& maintenance_budget_limit, const t_proposals& proposals );

      void update_settings( const time_point_sec& head_time );

      void remove_old_proposals( const block_notification& note );
      void make_payments( const block_notification& note );

      void record_funding( const block_notification& note );

   public:

      wps_processor( chain::database& _db ) : db( _db ){}

      const static std::string& get_removing_name();
      const static std::string& get_calculating_name();

      void run( const block_notification& note );
};

} } // namespace worth::chain
