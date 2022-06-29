#include <worth/chain/required_action_evaluator.hpp>

namespace worth { namespace chain {

#ifdef IS_TEST_NET

void example_required_evaluator::do_apply( const example_required_action& a ) {}

#endif

} } //worth::chain