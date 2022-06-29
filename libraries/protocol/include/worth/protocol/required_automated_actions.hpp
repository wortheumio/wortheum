#pragma once

#include <worth/protocol/worth_required_actions.hpp>

#include <worth/protocol/operation_util.hpp>

namespace worth { namespace protocol {

   /** NOTE: do not change the order of any actions or it will trigger a hardfork.
    */
   typedef fc::static_variant<
#ifdef IS_TEST_NET
            example_required_action
#endif
         > required_automated_action;

} } // worth::protocol

WORTH_DECLARE_OPERATION_TYPE( worth::protocol::required_automated_action );

FC_TODO( "Remove ifdef when first required automated action is added" )
#ifdef IS_TEST_NET
FC_REFLECT_TYPENAME( worth::protocol::required_automated_action );
#endif
