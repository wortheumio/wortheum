#pragma once
#include <worth/chain/worth_fwd.hpp>

#include <worth/protocol/required_automated_actions.hpp>

#include <worth/chain/worth_object_types.hpp>

namespace worth { namespace chain {

using worth::protocol::optional_automated_action;

class pending_optional_action_object : public object< pending_optional_action_object_type, pending_optional_action_object >
{
   WORTH_STD_ALLOCATOR_CONSTRUCTOR( pending_optional_action_object )

   public:
      template< typename Constructor, typename Allocator >
      pending_optional_action_object( Constructor&& c, allocator< Allocator > a )
      {
         c( *this );
      }

      id_type                    id;

      time_point_sec             execution_time;
      optional_automated_action  action;
};

typedef multi_index_container<
   pending_optional_action_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< pending_optional_action_object, pending_optional_action_id_type, &pending_optional_action_object::id > >,
      ordered_unique< tag< by_execution >,
         composite_key< pending_optional_action_object,
            member< pending_optional_action_object, time_point_sec, &pending_optional_action_object::execution_time >,
            member< pending_optional_action_object, pending_optional_action_id_type, &pending_optional_action_object::id >
         >
      >
   >,
   allocator< pending_optional_action_object >
> pending_optional_action_index;

} } //worth::chain

FC_REFLECT( worth::chain::pending_optional_action_object,
            (id)(execution_time)(action) )
CHAINBASE_SET_INDEX_TYPE( worth::chain::pending_optional_action_object, worth::chain::pending_optional_action_index )
