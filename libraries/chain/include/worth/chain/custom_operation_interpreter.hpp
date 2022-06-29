
#pragma once

#include <memory>

#include <worth/protocol/types.hpp>

namespace worth { namespace schema {
   struct abstract_schema;
} }

namespace worth { namespace protocol {
   struct custom_json_operation;
} }

namespace worth { namespace chain {

class custom_operation_interpreter
{
   public:
      virtual void apply( const protocol::custom_json_operation& op ) = 0;
      virtual void apply( const protocol::custom_binary_operation & op ) = 0;
      virtual worth::protocol::custom_id_type get_custom_id() = 0;
      virtual std::shared_ptr< worth::schema::abstract_schema > get_operation_schema() = 0;
};

} } // worth::chain
