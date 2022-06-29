#pragma once

#include <fc/io/raw.hpp>
#include <worth/protocol/types_fwd.hpp>

#define WORTH_ASSET_SYMBOL_PRECISION_BITS    4
#define WORTH_ASSET_CONTROL_BITS             1
#define WORTH_NAI_SHIFT                      ( WORTH_ASSET_SYMBOL_PRECISION_BITS + WORTH_ASSET_CONTROL_BITS )
#define WMT_MAX_NAI                          99999999
#define WMT_MIN_NAI                          1
#define WMT_MIN_NON_RESERVED_NAI             10000000
#define WORTH_ASSET_SYMBOL_NAI_LENGTH        10
#define WORTH_ASSET_SYMBOL_NAI_STRING_LENGTH ( WORTH_ASSET_SYMBOL_NAI_LENGTH + 2 )
#define WMT_MAX_NAI_POOL_COUNT               10
#define WMT_MAX_NAI_GENERATION_TRIES         100

#define WORTH_PRECISION_WBD   (3)
#define WORTH_PRECISION_WORTH (3)
#define WORTH_PRECISION_VESTS (6)

// One's place is used for check digit, which means NAI 0-9 all have NAI data of 0 which is invalid
// This space is safe to use because it would alwasys result in failure to convert from NAI
#define WORTH_NAI_WBD   (1)
#define WORTH_NAI_WORTH (2)
#define WORTH_NAI_VESTS (3)

#define WORTH_ASSET_NUM_WBD \
  (uint32_t(((WMT_MAX_NAI + WORTH_NAI_WBD)   << WORTH_NAI_SHIFT) | WORTH_PRECISION_WBD))
#define WORTH_ASSET_NUM_WORTH \
  (uint32_t(((WMT_MAX_NAI + WORTH_NAI_WORTH) << WORTH_NAI_SHIFT) | WORTH_PRECISION_WORTH))
#define WORTH_ASSET_NUM_VESTS \
  (uint32_t(((WMT_MAX_NAI + WORTH_NAI_VESTS) << WORTH_NAI_SHIFT) | WORTH_PRECISION_VESTS))

#ifdef IS_TEST_NET

#define VESTS_SYMBOL_U64  (uint64_t('V') | (uint64_t('E') << 8) | (uint64_t('S') << 16) | (uint64_t('T') << 24) | (uint64_t('S') << 32))
#define WORTH_SYMBOL_U64  (uint64_t('T') | (uint64_t('E') << 8) | (uint64_t('S') << 16) | (uint64_t('T') << 24) | (uint64_t('S') << 32))
#define WBD_SYMBOL_U64    (uint64_t('T') | (uint64_t('B') << 8) | (uint64_t('D') << 16))

#else

#define VESTS_SYMBOL_U64  (uint64_t('V') | (uint64_t('E') << 8) | (uint64_t('S') << 16) | (uint64_t('T') << 24) | (uint64_t('S') << 32))
#define WORTH_SYMBOL_U64  (uint64_t('W') | (uint64_t('O') << 8) | (uint64_t('R') << 16) | (uint64_t('T') << 24) | (uint64_t('H') << 32))
#define WBD_SYMBOL_U64    (uint64_t('W') | (uint64_t('B') << 8) | (uint64_t('D') << 16))

#endif

#define VESTS_SYMBOL_SER  (uint64_t(6) | (VESTS_SYMBOL_U64 << 8)) ///< VESTS|VESTS with 6 digits of precision
#define WORTH_SYMBOL_SER  (uint64_t(3) | (WORTH_SYMBOL_U64 << 8)) ///< WORTH|TESTS with 3 digits of precision
#define WBD_SYMBOL_SER    (uint64_t(3) |   (WBD_SYMBOL_U64 << 8)) ///< WBD|TBD with 3 digits of precision

#define WORTH_ASSET_MAX_DECIMALS 12

#define WMT_ASSET_NUM_PRECISION_MASK   0xF
#define WMT_ASSET_NUM_CONTROL_MASK     0x10
#define WMT_ASSET_NUM_VESTING_MASK     0x20

#define ASSET_SYMBOL_NAI_KEY      "nai"
#define ASSET_SYMBOL_DECIMALS_KEY "decimals"

namespace worth { namespace protocol {

class asset_symbol_type
{
   public:
      enum asset_symbol_space
      {
         legacy_space = 1,
         wmt_nai_space = 2
      };

      explicit operator uint32_t() { return to_nai(); }

      // buf must have space for WORTH_ASSET_SYMBOL_MAX_LENGTH+1
      static asset_symbol_type from_string( const std::string& str );
      static asset_symbol_type from_nai_string( const char* buf, uint8_t decimal_places );
      static asset_symbol_type from_asset_num( uint32_t asset_num )
      {   asset_symbol_type result;   result.asset_num = asset_num;   return result;   }
      static uint32_t asset_num_from_nai( uint32_t nai, uint8_t decimal_places );
      static asset_symbol_type from_nai( uint32_t nai, uint8_t decimal_places )
      {   return from_asset_num( asset_num_from_nai( nai, decimal_places ) );          }
      static uint8_t damm_checksum_8digit(uint32_t value);

      std::string to_string()const;

      void to_nai_string( char* buf )const;
      std::string to_nai_string()const
      {
         char buf[ WORTH_ASSET_SYMBOL_NAI_STRING_LENGTH ];
         to_nai_string( buf );
         return std::string( buf );
      }

      uint32_t to_nai()const;

      /**Returns true when symbol represents vesting variant of the token,
       * false for liquid one.
       */
      bool is_vesting() const;
      /**Returns vesting symbol when called from liquid one
       * and liquid symbol when called from vesting one.
       * Returns back the WBD symbol if represents WBD.
       */
      asset_symbol_type get_paired_symbol() const;
      /**Returns asset_num stripped of precision holding bits.
       * \warning checking that it's WMT symbol is caller responsibility.
       */
      uint32_t get_stripped_precision_wmt_num() const
      { 
         return asset_num & ~( WMT_ASSET_NUM_PRECISION_MASK );
      }

      asset_symbol_space space()const;
      uint8_t decimals()const
      {  return uint8_t( asset_num & WMT_ASSET_NUM_PRECISION_MASK );    }
      void validate()const;

      friend bool operator == ( const asset_symbol_type& a, const asset_symbol_type& b )
      {  return (a.asset_num == b.asset_num);   }
      friend bool operator != ( const asset_symbol_type& a, const asset_symbol_type& b )
      {  return (a.asset_num != b.asset_num);   }
      friend bool operator <  ( const asset_symbol_type& a, const asset_symbol_type& b )
      {  return (a.asset_num <  b.asset_num);   }
      friend bool operator >  ( const asset_symbol_type& a, const asset_symbol_type& b )
      {  return (a.asset_num >  b.asset_num);   }
      friend bool operator <= ( const asset_symbol_type& a, const asset_symbol_type& b )
      {  return (a.asset_num <= b.asset_num);   }
      friend bool operator >= ( const asset_symbol_type& a, const asset_symbol_type& b )
      {  return (a.asset_num >= b.asset_num);   }

      uint32_t asset_num = 0;
};

} } // worth::protocol

FC_REFLECT(worth::protocol::asset_symbol_type, (asset_num))

namespace fc { namespace raw {

// Legacy serialization of assets
// 0000pppp aaaaaaaa bbbbbbbb cccccccc dddddddd eeeeeeee ffffffff 00000000
// Symbol = abcdef
//
// NAI serialization of assets
// aaa1pppp bbbbbbbb cccccccc dddddddd
// NAI = (MSB to LSB) dddddddd cccccccc bbbbbbbb aaa
//
// NAI internal storage of legacy assets

template< typename Stream >
inline void pack( Stream& s, const worth::protocol::asset_symbol_type& sym )
{
   switch( sym.space() )
   {
      case worth::protocol::asset_symbol_type::legacy_space:
      {
         uint64_t ser = 0;
         switch( sym.asset_num )
         {
            case WORTH_ASSET_NUM_WORTH:
               ser = WORTH_SYMBOL_SER;
               break;
            case WORTH_ASSET_NUM_WBD:
               ser = WBD_SYMBOL_SER;
               break;
            case WORTH_ASSET_NUM_VESTS:
               ser = VESTS_SYMBOL_SER;
               break;
            default:
               FC_ASSERT( false, "Cannot serialize unknown asset symbol" );
         }
         pack( s, ser );
         break;
      }
      case worth::protocol::asset_symbol_type::wmt_nai_space:
         pack( s, sym.asset_num );
         break;
      default:
         FC_ASSERT( false, "Cannot serialize unknown asset symbol" );
   }
}

template< typename Stream >
inline void unpack( Stream& s, worth::protocol::asset_symbol_type& sym, uint32_t )
{
   uint64_t ser = 0;
   s.read( (char*) &ser, 4 );

   switch( ser )
   {
      case WORTH_SYMBOL_SER & 0xFFFFFFFF:
         s.read( ((char*) &ser)+4, 4 );
         FC_ASSERT( ser == WORTH_SYMBOL_SER, "invalid asset bits" );
         sym.asset_num = WORTH_ASSET_NUM_WORTH;
         break;
      case WBD_SYMBOL_SER & 0xFFFFFFFF:
         s.read( ((char*) &ser)+4, 4 );
         FC_ASSERT( ser == WBD_SYMBOL_SER, "invalid asset bits" );
         sym.asset_num = WORTH_ASSET_NUM_WBD;
         break;
      case VESTS_SYMBOL_SER & 0xFFFFFFFF:
         s.read( ((char*) &ser)+4, 4 );
         FC_ASSERT( ser == VESTS_SYMBOL_SER, "invalid asset bits" );
         sym.asset_num = WORTH_ASSET_NUM_VESTS;
         break;
      default:
         sym.asset_num = uint32_t( ser );
   }
   sym.validate();
}

} // fc::raw

inline void to_variant( const worth::protocol::asset_symbol_type& sym, fc::variant& var )
{
   try
   {
      mutable_variant_object o;
         o( ASSET_SYMBOL_NAI_KEY, sym.to_nai_string() )
          ( ASSET_SYMBOL_DECIMALS_KEY, sym.decimals() );
      var = std::move( o );
   } FC_CAPTURE_AND_RETHROW()
}

inline void from_variant( const fc::variant& var, worth::protocol::asset_symbol_type& sym )
{
   using worth::protocol::asset_symbol_type;

   try
   {
      FC_ASSERT( var.is_object(), "Asset symbol is expected to be an object." );

      auto& o = var.get_object();

      auto nai = o.find( ASSET_SYMBOL_NAI_KEY );
      FC_ASSERT( nai != o.end(), "Expected key '${key}'.", ("key", ASSET_SYMBOL_NAI_KEY) );
      FC_ASSERT( nai->value().is_string(), "Expected a string type for value '${key}'.", ("key", ASSET_SYMBOL_NAI_KEY) );

      auto decimals = o.find( ASSET_SYMBOL_DECIMALS_KEY );
      FC_ASSERT( decimals != o.end(), "Expected key '${key}'.", ("key", ASSET_SYMBOL_DECIMALS_KEY) );
      FC_ASSERT( decimals->value().is_uint64(), "Expected an unsigned integer type for value '${key}'.", ("key", ASSET_SYMBOL_DECIMALS_KEY) );
      FC_ASSERT( decimals->value().as_uint64() <= WORTH_ASSET_MAX_DECIMALS,
         "Expected decimals to be less than or equal to ${num}", ("num", WORTH_ASSET_MAX_DECIMALS) );

      sym = asset_symbol_type::from_nai_string( nai->value().as_string().c_str(), decimals->value().as< uint8_t >() );
   } FC_CAPTURE_AND_RETHROW()
}

} // fc
