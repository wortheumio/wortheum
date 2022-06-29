file(GLOB HEADERS "include/worth/plugins/raw_block/*.hpp")

add_library( worth_raw_block
             ${HEADERS}
             raw_block_plugin.cpp
             raw_block_api.cpp
           )

target_link_libraries( worth_raw_block worth_app worth_chain worth_protocol fc )
target_include_directories( worth_raw_block
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
