#pragma once
#include <worth/chain/worth_fwd.hpp>
#include <worth/plugins/account_by_key/account_by_key_plugin.hpp>
#include <worth/plugins/json_rpc/json_rpc_plugin.hpp>

#include <appbase/application.hpp>

#define WORTH_ACCOUNT_BY_KEY_API_PLUGIN_NAME "account_by_key_api"


namespace worth { namespace plugins { namespace account_by_key {

using namespace appbase;

class account_by_key_api_plugin : public appbase::plugin< account_by_key_api_plugin >
{
public:
   APPBASE_PLUGIN_REQUIRES(
      (worth::plugins::account_by_key::account_by_key_plugin)
      (worth::plugins::json_rpc::json_rpc_plugin)
   )

   account_by_key_api_plugin();
   virtual ~account_by_key_api_plugin();

   static const std::string& name() { static std::string name = WORTH_ACCOUNT_BY_KEY_API_PLUGIN_NAME; return name; }

   virtual void set_program_options( options_description& cli, options_description& cfg ) override;

   virtual void plugin_initialize( const variables_map& options ) override;
   virtual void plugin_startup() override;
   virtual void plugin_shutdown() override;

   std::shared_ptr< class account_by_key_api > api;
};

} } } // worth::plugins::account_by_key
