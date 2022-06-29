file(GLOB HEADERS "include/worth/plugins/block_info/*.hpp")

add_library( worth_block_info
             ${HEADERS}
             block_info_plugin.cpp
             block_info_api.cpp
           )

target_link_libraries( worth_block_info worth_app worth_chain worth_protocol fc )
target_include_directories( worth_block_info
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
