#pragma once

#ifdef WORTHEUM_ENABLE_WMT

#include <fc/optional.hpp>
#include <worth/chain/database.hpp>
#include <worth/chain/wmt_objects/wmt_token_object.hpp>
#include <worth/protocol/asset_symbol.hpp>

namespace worth { namespace chain { namespace util { namespace wmt {

const wmt_token_object* find_token( const database& db, uint32_t nai );
const wmt_token_object* find_token( const database& db, asset_symbol_type symbol, bool precision_agnostic = false );
const wmt_token_emissions_object* last_emission( const database& db, const asset_symbol_type& symbol );
fc::optional< time_point_sec > last_emission_time( const database& db, const asset_symbol_type& symbol );

} } } } // worth::chain::util::wmt

#endif
