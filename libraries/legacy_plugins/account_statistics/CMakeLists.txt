file(GLOB HEADERS "include/worth/account_statistics/*.hpp")

add_library( worth_account_statistics
             account_statistics_plugin.cpp
             account_statistics_api.cpp
           )

target_link_libraries( worth_account_statistics worth_chain worth_protocol worth_app )
target_include_directories( worth_account_statistics
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   worth_account_statistics

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)