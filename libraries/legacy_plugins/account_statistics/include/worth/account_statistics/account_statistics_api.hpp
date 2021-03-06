#pragma once

#include <worth/account_statistics/account_statistics_plugin.hpp>

#include <fc/api.hpp>

namespace worth { namespace app {
   struct api_context;
} }

namespace worth { namespace account_statistics {

namespace detail
{
   class account_statistics_api_impl;
}

class account_statistics_api
{
   public:
      account_statistics_api( const worth::app::api_context& ctx );

      void on_api_startup();

   private:
      std::shared_ptr< detail::account_statistics_api_impl > _my;
};

} } // worth::account_statistics

FC_API( worth::account_statistics::account_statistics_api, )