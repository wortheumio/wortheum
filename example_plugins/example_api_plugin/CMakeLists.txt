file(GLOB HEADERS "include/worth/plugins/example_api_plugin/*.hpp")

add_library( example_api_plugin
             ${HEADERS}
             example_api_plugin.cpp
           )

target_link_libraries( example_api_plugin appbase worth_chain fc )
target_include_directories( example_api_plugin
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
