#include <worth/protocol/validation.hpp>
#include <worth/protocol/worth_optional_actions.hpp>

namespace worth { namespace protocol {

#ifdef IS_TEST_NET
void example_optional_action::validate()const
{
   validate_account_name( account );
}
#endif

} } //worth::protocol
