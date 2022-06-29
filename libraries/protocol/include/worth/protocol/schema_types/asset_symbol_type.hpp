
#pragma once

#include <worth/schema/abstract_schema.hpp>
#include <worth/schema/schema_impl.hpp>

#include <worth/protocol/asset_symbol.hpp>

namespace worth { namespace schema { namespace detail {

//////////////////////////////////////////////
// asset_symbol_type                        //
//////////////////////////////////////////////

struct schema_asset_symbol_type_impl
   : public abstract_schema
{
   WORTH_SCHEMA_CLASS_BODY( schema_asset_symbol_type_impl )
};

}

template<>
struct schema_reflect< worth::protocol::asset_symbol_type >
{
   typedef detail::schema_asset_symbol_type_impl           schema_impl_type;
};

} }
