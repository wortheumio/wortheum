#pragma once
#include <worth/chain/worth_fwd.hpp>

#include <worth/protocol/version.hpp>

#include <worth/chain/worth_object_types.hpp>

namespace worth { namespace chain {

   using chainbase::t_vector;

   class hardfork_property_object : public object< hardfork_property_object_type, hardfork_property_object >
   {
      WORTH_STD_ALLOCATOR_CONSTRUCTOR( hardfork_property_object )

      public:
         template< typename Constructor, typename Allocator >
         hardfork_property_object( Constructor&& c, allocator< Allocator > a )
            :processed_hardforks( a )
         {
            c( *this );
         }

         id_type                                                              id;

         using t_processed_hardforks = t_vector< fc::time_point_sec >;

         t_processed_hardforks                                                processed_hardforks;
         uint32_t                                                             last_hardfork = 0;
         protocol::hardfork_version                                           current_hardfork_version;
         protocol::hardfork_version                                           next_hardfork;
         fc::time_point_sec                                                   next_hardfork_time;
   };

   typedef multi_index_container<
      hardfork_property_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< hardfork_property_object, hardfork_property_object::id_type, &hardfork_property_object::id > >
      >,
      allocator< hardfork_property_object >
   > hardfork_property_index;

} } // worth::chain

FC_REFLECT( worth::chain::hardfork_property_object,
   (id)(processed_hardforks)(last_hardfork)(current_hardfork_version)
   (next_hardfork)(next_hardfork_time) )
CHAINBASE_SET_INDEX_TYPE( worth::chain::hardfork_property_object, worth::chain::hardfork_property_index )
