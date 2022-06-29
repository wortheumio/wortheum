
#pragma once

#include <worth/schema/abstract_schema.hpp>
#include <worth/schema/schema_impl.hpp>

#include <worth/protocol/types.hpp>

namespace worth { namespace schema { namespace detail {

//////////////////////////////////////////////
// account_name_type                        //
//////////////////////////////////////////////

struct schema_account_name_type_impl
   : public abstract_schema
{
   WORTH_SCHEMA_CLASS_BODY( schema_account_name_type_impl )
};

}

template<>
struct schema_reflect< worth::protocol::account_name_type >
{
   typedef detail::schema_account_name_type_impl           schema_impl_type;
};

} }

namespace fc {

template<>
struct get_typename< worth::protocol::account_name_type >
{
   static const char* name()
   {
      return "worth::protocol::account_name_type";
   }
};

}
