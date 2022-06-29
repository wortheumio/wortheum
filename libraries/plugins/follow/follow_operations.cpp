#include <worth/plugins/follow/follow_operations.hpp>

#include <worth/protocol/operation_util_impl.hpp>

namespace worth { namespace plugins{ namespace follow {

void follow_operation::validate()const
{
   FC_ASSERT( follower != following, "You cannot follow yourself" );
}

void reblog_operation::validate()const
{
   FC_ASSERT( account != author, "You cannot reblog your own content" );
}

} } } //worth::plugins::follow

WORTH_DEFINE_OPERATION_TYPE( worth::plugins::follow::follow_plugin_operation )
