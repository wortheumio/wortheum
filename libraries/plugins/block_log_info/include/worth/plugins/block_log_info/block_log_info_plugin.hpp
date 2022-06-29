#pragma once
#include <worth/chain/worth_fwd.hpp>
#include <appbase/application.hpp>

#include <worth/plugins/chain/chain_plugin.hpp>

namespace worth { namespace plugins { namespace block_log_info {

namespace detail { class block_log_info_plugin_impl; }

using namespace appbase;

#define WORTH_BLOCK_LOG_INFO_PLUGIN_NAME "block_log_info"

class block_log_info_plugin : public appbase::plugin< block_log_info_plugin >
{
   public:
      block_log_info_plugin();
      virtual ~block_log_info_plugin();

      APPBASE_PLUGIN_REQUIRES( (worth::plugins::chain::chain_plugin) )

      static const std::string& name() { static std::string name = WORTH_BLOCK_LOG_INFO_PLUGIN_NAME; return name; }

      virtual void set_program_options( options_description& cli, options_description& cfg ) override;
      virtual void plugin_initialize( const variables_map& options ) override;
      virtual void plugin_startup() override;
      virtual void plugin_shutdown() override;

   private:
      std::unique_ptr< detail::block_log_info_plugin_impl > my;
};

} } } // worth::plugins::block_log_info
