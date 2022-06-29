
#pragma once

#include <worth/protocol/operation_util.hpp>

#include <fc/static_variant.hpp>

namespace fc
{
   std::string name_from_type( const std::string& type_name );

   struct from_operation
   {
      variant& var;
      from_operation( variant& dv )
         : var( dv ) {}

      typedef void result_type;
      template<typename T> void operator()( const T& v )const
      {
         auto name = name_from_type( fc::get_typename< T >::name() );
         var = variant( std::make_pair( name, v ) );
      }
   };

   struct get_operation_name
   {
      string& name;
      get_operation_name( string& dv )
         : name( dv ) {}

      typedef void result_type;
      template< typename T > void operator()( const T& v )const
      {
         name = name_from_type( fc::get_typename< T >::name() );
      }
   };
}

namespace worth { namespace protocol {

struct operation_validate_visitor
{
   typedef void result_type;
   template<typename T>
   void operator()( const T& v )const { v.validate(); }
};

} } // worth::protocol

//
// Place WORTH_DEFINE_OPERATION_TYPE in a .cpp file to define
// functions related to your operation type
//
#define WORTH_DEFINE_OPERATION_TYPE( OperationType )                       \
                                                                           \
namespace worth { namespace protocol {                                     \
                                                                           \
void operation_validate( const OperationType& op )                         \
{                                                                          \
   op.visit( worth::protocol::operation_validate_visitor() );              \
}                                                                          \
                                                                           \
void operation_get_required_authorities( const OperationType& op,          \
                                         flat_set< account_name_type >& active,         \
                                         flat_set< account_name_type >& owner,          \
                                         flat_set< account_name_type >& posting,        \
                                         std::vector< authority >& other ) \
{                                                                          \
   op.visit( worth::protocol::get_required_auth_visitor( active, owner, posting, other ) ); \
}                                                                          \
                                                                           \
} } /* worth::protocol */
