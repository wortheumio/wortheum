#pragma once

#include <worth/protocol/asset.hpp>

#define WORTH_SYMBOL_LEGACY_SER_1   (uint64_t(1) | (WORTH_SYMBOL_U64 << 8))
#define WORTH_SYMBOL_LEGACY_SER_2   (uint64_t(2) | (WORTH_SYMBOL_U64 << 8))
#define WORTH_SYMBOL_LEGACY_SER_3   (uint64_t(5) | (WORTH_SYMBOL_U64 << 8))
#define WORTH_SYMBOL_LEGACY_SER_4   (uint64_t(3) | (uint64_t('0') << 8) | (uint64_t('.') << 16) | (uint64_t('0') << 24) | (uint64_t('0') << 32) | (uint64_t('1') << 40))
#define WORTH_SYMBOL_LEGACY_SER_5   (uint64_t(3) | (uint64_t('6') << 8) | (uint64_t('.') << 16) | (uint64_t('0') << 24) | (uint64_t('0') << 32) | (uint64_t('0') << 40))

namespace worth { namespace protocol {

class legacy_worth_asset_symbol_type
{
   public:
      legacy_worth_asset_symbol_type() {}

      bool is_canon()const
      {   return ( ser == WORTH_SYMBOL_SER );    }

      uint64_t ser = WORTH_SYMBOL_SER;
};

struct legacy_worth_asset
{
   public:
      legacy_worth_asset() {}

      template< bool force_canon >
      asset to_asset()const
      {
         if( force_canon )
         {
            FC_ASSERT( symbol.is_canon(), "Must use canonical WORTH symbol serialization" );
         }
         return asset( amount, WORTH_SYMBOL );
      }

      static legacy_worth_asset from_amount( share_type amount )
      {
         legacy_worth_asset leg;
         leg.amount = amount;
         return leg;
      }

      static legacy_worth_asset from_asset( const asset& a )
      {
         FC_ASSERT( a.symbol == WORTH_SYMBOL );
         return from_amount( a.amount );
      }

      share_type                       amount;
      legacy_worth_asset_symbol_type   symbol;
};

} }

namespace fc { namespace raw {

template< typename Stream >
inline void pack( Stream& s, const worth::protocol::legacy_worth_asset_symbol_type& sym )
{
   switch( sym.ser )
   {
      case WORTH_SYMBOL_LEGACY_SER_1:
      case WORTH_SYMBOL_LEGACY_SER_2:
      case WORTH_SYMBOL_LEGACY_SER_3:
      case WORTH_SYMBOL_LEGACY_SER_4:
      case WORTH_SYMBOL_LEGACY_SER_5:
         wlog( "pack legacy serialization ${s}", ("s", sym.ser) );
      case WORTH_SYMBOL_SER:
         pack( s, sym.ser );
         break;
      default:
         FC_ASSERT( false, "Cannot serialize legacy symbol ${s}", ("s", sym.ser) );
   }
}

template< typename Stream >
inline void unpack( Stream& s, worth::protocol::legacy_worth_asset_symbol_type& sym, uint32_t depth )
{
   //  994240:        "account_creation_fee": "0.1 WORTH"
   // 1021529:        "account_creation_fee": "10.0 WORTH"
   // 3143833:        "account_creation_fee": "3.00000 WORTH"
   // 3208405:        "account_creation_fee": "2.00000 WORTH"
   // 3695672:        "account_creation_fee": "3.00 WORTH"
   // 4338089:        "account_creation_fee": "0.001 0.001"
   // 4626205:        "account_creation_fee": "6.000 6.000"
   // 4632595:        "account_creation_fee": "6.000 6.000"
   depth++;
   uint64_t ser = 0;

   fc::raw::unpack( s, ser, depth );
   switch( ser )
   {
      case WORTH_SYMBOL_LEGACY_SER_1:
      case WORTH_SYMBOL_LEGACY_SER_2:
      case WORTH_SYMBOL_LEGACY_SER_3:
      case WORTH_SYMBOL_LEGACY_SER_4:
      case WORTH_SYMBOL_LEGACY_SER_5:
         wlog( "unpack legacy serialization ${s}", ("s", ser) );
      case WORTH_SYMBOL_SER:
         sym.ser = ser;
         break;
      default:
         FC_ASSERT( false, "Cannot deserialize legacy symbol ${s}", ("s", ser) );
   }
}

} // fc::raw

inline void to_variant( const worth::protocol::legacy_worth_asset& leg, fc::variant& v )
{
   to_variant( leg.to_asset<false>(), v );
}

inline void from_variant( const fc::variant& v, worth::protocol::legacy_worth_asset& leg )
{
   worth::protocol::asset a;
   from_variant( v, a );
   leg = worth::protocol::legacy_worth_asset::from_asset( a );
}

template<>
struct get_typename< worth::protocol::legacy_worth_asset_symbol_type >
{
   static const char* name()
   {
      return "worth::protocol::legacy_worth_asset_symbol_type";
   }
};

} // fc

FC_REFLECT( worth::protocol::legacy_worth_asset,
   (amount)
   (symbol)
   )
