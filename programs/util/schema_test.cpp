
namespace worth { namespace protocol {
struct votable_asset_info_v1;
} }

#include <worth/protocol/types_fwd.hpp>
#include <worth/chain/worth_fwd.hpp>

#include <worth/schema/schema.hpp>
#include <worth/schema/schema_impl.hpp>
#include <worth/schema/schema_types.hpp>

#include <worth/chain/schema_types/oid.hpp>
#include <worth/protocol/schema_types/account_name_type.hpp>
#include <worth/protocol/schema_types/asset_symbol_type.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <worth/chain/account_object.hpp>
#include <worth/chain/worth_objects.hpp>

using namespace worth::schema;

struct mystruct
{
   std::string alpha;
   uint64_t beta;
};

FC_REFLECT( mystruct,
   (alpha)
   (beta)
   );

void process( std::shared_ptr< abstract_schema > s )
{
   std::string name;
   s->get_name(name);
   std::cout << "   name = " << name;
   std::vector< std::shared_ptr< abstract_schema > > deps;
   s->get_deps(deps);
   std::vector< std::string > dep_names;
   for( const std::shared_ptr< abstract_schema >& s : deps )
   {
      std::string s_name;
      s->get_name( s_name );
      dep_names.push_back( s_name );
   }
   std::cout << "   deps = " << fc::json::to_string(dep_names);
   std::string str_schema;
   s->get_str_schema( str_schema );
   std::cout << "   s = " << str_schema;
   std::cout << std::endl;
}

int main( int argc, char** argv, char** envp )
{
   std::vector< std::shared_ptr< abstract_schema > > schemas;

   schemas.push_back( get_schema_for_type< mystruct >() );
   schemas.push_back( get_schema_for_type< worth::chain::account_object >() );
   schemas.push_back( get_schema_for_type< worth::chain::comment_object >() );
   add_dependent_schemas( schemas );

   for( const std::shared_ptr< abstract_schema >& s : schemas )
   {
      process( s );
   }

   return 0;
}
