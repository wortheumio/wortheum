#pragma once
#include <worth/protocol/types.hpp>
#include <worth/protocol/base.hpp>

namespace worth { namespace protocol {

#ifdef IS_TEST_NET
   struct example_required_action : public base_operation
   {
      account_name_type account;

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert(account); }

      friend bool operator==( const example_required_action& lhs, const example_required_action& rhs );
   };
#endif

} } // worth::protocol

#ifdef IS_TEST_NET
FC_REFLECT( worth::protocol::example_required_action, (account) )
#endif
