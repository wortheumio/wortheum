#pragma once
#include <worth/chain/worth_fwd.hpp>
#include <worth/plugins/chain/chain_plugin.hpp>
#include <worth/plugins/p2p/p2p_plugin.hpp>
#include <worth/plugins/rc/rc_plugin.hpp>
#include <worth/plugins/witness/block_producer.hpp>

#include <appbase/application.hpp>

#define WORTH_WITNESS_PLUGIN_NAME "witness"

#define RESERVE_RATIO_PRECISION ((int64_t)10000)
#define RESERVE_RATIO_MIN_INCREMENT ((int64_t)5000)

#define WITNESS_CUSTOM_OP_BLOCK_LIMIT 5

namespace worth { namespace plugins { namespace witness {

namespace detail { class witness_plugin_impl; }

namespace block_production_condition
{
   enum block_production_condition_enum
   {
      produced = 0,
      not_synced = 1,
      not_my_turn = 2,
      not_time_yet = 3,
      no_private_key = 4,
      low_participation = 5,
      lag = 6,
      consecutive = 7,
      wait_for_genesis = 8,
      exception_producing_block = 9
   };
}

class witness_plugin : public appbase::plugin< witness_plugin >
{
public:
   APPBASE_PLUGIN_REQUIRES(
      (worth::plugins::chain::chain_plugin)
      (worth::plugins::p2p::p2p_plugin)
      (worth::plugins::rc::rc_plugin)
   )

   witness_plugin();
   virtual ~witness_plugin();

   static const std::string& name() { static std::string name = WORTH_WITNESS_PLUGIN_NAME; return name; }

   virtual void set_program_options(
      boost::program_options::options_description &command_line_options,
      boost::program_options::options_description &config_file_options
      ) override;

   virtual void plugin_initialize(const boost::program_options::variables_map& options) override;
   virtual void plugin_startup() override;
   virtual void plugin_shutdown() override;

private:
   std::unique_ptr< detail::witness_plugin_impl > my;
};

} } } // worth::plugins::witness
