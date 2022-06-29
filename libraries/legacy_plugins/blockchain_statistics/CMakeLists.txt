file(GLOB HEADERS "include/worth/blockchain_statistics/*.hpp")

add_library( worth_blockchain_statistics
             blockchain_statistics_plugin.cpp
             blockchain_statistics_api.cpp
           )

target_link_libraries( worth_blockchain_statistics worth_chain worth_protocol worth_app )
target_include_directories( worth_blockchain_statistics
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   worth_blockchain_statistics

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)