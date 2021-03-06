#pragma once
#include <worth/chain/worth_object_types.hpp>

namespace worth { namespace chain {
struct by_account;
} }

namespace worth { namespace plugins { namespace reputation {

using namespace std;
using namespace worth::chain;

using chainbase::t_vector;

#ifndef WORTH_REPUTATION_SPACE_ID
#define WORTH_REPUTATION_SPACE_ID 17
#endif

enum reputation_plugin_object_type
{
   reputation_object_type        = ( WORTH_REPUTATION_SPACE_ID << 8 )
};


class reputation_object : public object< reputation_object_type, reputation_object >
{
   public:
      template< typename Constructor, typename Allocator >
      reputation_object( Constructor&& c, allocator< Allocator > a )
      {
         c( *this );
      }

      reputation_object() {}

      id_type           id;

      account_name_type account;
      share_type        reputation;
};

typedef oid< reputation_object > reputation_id_type;

typedef multi_index_container<
   reputation_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< reputation_object, reputation_id_type, &reputation_object::id > >,
      ordered_unique< tag< by_account >, member< reputation_object, account_name_type, &reputation_object::account > >
   >,
   allocator< reputation_object >
> reputation_index;

} } } // worth::plugins::reputation


FC_REFLECT( worth::plugins::reputation::reputation_object, (id)(account)(reputation) )
CHAINBASE_SET_INDEX_TYPE( worth::plugins::reputation::reputation_object, worth::plugins::reputation::reputation_index )
