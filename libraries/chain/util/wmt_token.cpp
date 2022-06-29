#include <worth/chain/worth_fwd.hpp>
#include <worth/chain/util/wmt_token.hpp>
#include <worth/chain/worth_object_types.hpp>
#ifdef WORTHEUM_ENABLE_WMT

namespace worth { namespace chain { namespace util { namespace wmt {

const wmt_token_object* find_token( const database& db, uint32_t nai )
{
   const auto& idx = db.get_index< wmt_token_index >().indices().get< by_symbol >();

   auto itr = idx.lower_bound( asset_symbol_type::from_nai( nai, 0 ) );
   for (; itr != idx.end(); ++itr )
   {
      if (itr->liquid_symbol.to_nai() != nai )
         break;
      return &*itr;
   }

   return nullptr;
}

const wmt_token_object* find_token( const database& db, asset_symbol_type symbol, bool precision_agnostic )
{
   // Only liquid symbols are stored in the wmt token index
   auto s = symbol.is_vesting() ? symbol.get_paired_symbol() : symbol;

   if ( precision_agnostic )
      return find_token( db, s.to_nai() );
   else
      return db.find< wmt_token_object, by_symbol >( s );
}

const wmt_token_emissions_object* last_emission( const database& db, const asset_symbol_type& symbol )
{
   const auto& idx = db.get_index< wmt_token_emissions_index, by_symbol_time >();

   const auto range = idx.equal_range( symbol );

   /*
    * The last element in this range is our highest value 'schedule_time', so
    * we reverse the iterator range and take the first one.
    */
   auto itr = range.second;
   while ( itr != range.first )
   {
      --itr;
      return &*itr;
   }

   return nullptr;
}

fc::optional< time_point_sec > last_emission_time( const database& db, const asset_symbol_type& symbol )
{
   auto _last_emission = last_emission( db, symbol );

   if ( _last_emission != nullptr )
   {
      // A maximum interval count indicates we should emit indefinitely
      if ( _last_emission->interval_count == WMT_EMIT_INDEFINITELY )
         return time_point_sec::maximum();
      else
         // This potential time_point overflow is protected by wmt_setup_emissions_operation::validate
         return _last_emission->schedule_time + fc::seconds( uint64_t( _last_emission->interval_seconds ) * uint64_t( _last_emission->interval_count ) );
   }

   return {};
}

} } } } // worth::chain::util::wmt

#endif
