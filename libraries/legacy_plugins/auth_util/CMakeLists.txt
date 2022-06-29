file(GLOB HEADERS "include/worth/plugins/auth_util/*.hpp")

add_library( worth_auth_util
             ${HEADERS}
             auth_util_plugin.cpp
             auth_util_api.cpp
           )

target_link_libraries( worth_auth_util worth_app worth_chain worth_protocol fc )
target_include_directories( worth_auth_util
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
