#pragma once

#include <worth/protocol/worth_optional_actions.hpp>

#include <worth/chain/evaluator.hpp>

namespace worth { namespace chain {

using namespace worth::protocol;

#ifdef IS_TEST_NET
WORTH_DEFINE_ACTION_EVALUATOR( example_optional, optional_automated_action )
#endif

} } //worth::chain
