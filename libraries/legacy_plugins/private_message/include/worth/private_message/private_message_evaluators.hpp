#pragma once

#include <worth/chain/evaluator.hpp>

#include <worth/private_message/private_message_operations.hpp>
#include <worth/private_message/private_message_plugin.hpp>

namespace worth { namespace private_message {

WORTH_DEFINE_PLUGIN_EVALUATOR( private_message_plugin, worth::private_message::private_message_plugin_operation, private_message )

} }
