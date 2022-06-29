#pragma once

#include <worth/protocol/types.hpp>

#include <fc/reflect/reflect.hpp>

namespace worth { namespace protocol {
struct signed_transaction;
} } // worth::protocol

namespace worth { namespace plugins { namespace rc {

using worth::protocol::account_name_type;
using worth::protocol::signed_transaction;

account_name_type get_resource_user( const signed_transaction& tx );

} } } // worth::plugins::rc
