#pragma once

#include <worth/protocol/worth_required_actions.hpp>

#include <worth/chain/evaluator.hpp>

namespace worth { namespace chain {

using namespace worth::protocol;

#ifdef IS_TEST_NET
WORTH_DEFINE_ACTION_EVALUATOR( example_required, required_automated_action )
#endif

} } //worth::chain
