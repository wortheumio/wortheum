#pragma once
#include <worth/chain/worth_fwd.hpp>
#include <worth/plugins/chain/chain_plugin.hpp>

#include <worth/chain/generic_custom_operation_interpreter.hpp>


#define WORTH_REPUTATION_PLUGIN_NAME "reputation"


namespace worth { namespace plugins{ namespace reputation {

namespace detail { class reputation_plugin_impl; }

using namespace appbase;
using worth::chain::generic_custom_operation_interpreter;

class reputation_plugin : public appbase::plugin< reputation_plugin >
{
   public:
      reputation_plugin();
      virtual ~reputation_plugin();

      APPBASE_PLUGIN_REQUIRES( (worth::plugins::chain::chain_plugin) )

      static const std::string& name() { static std::string name = WORTH_REPUTATION_PLUGIN_NAME; return name; }

      virtual void set_program_options(
         options_description& cli,
         options_description& cfg ) override;
      virtual void plugin_initialize( const variables_map& options ) override;
      virtual void plugin_startup() override;
      virtual void plugin_shutdown() override;

   private:
      std::unique_ptr< detail::reputation_plugin_impl > my;
};

} } } //worth::reputation
