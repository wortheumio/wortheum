file(GLOB HEADERS "include/worth/private_message/*.hpp")

add_library( worth_private_message
             private_message_plugin.cpp
           )

target_link_libraries( worth_private_message worth_chain worth_protocol worth_app )
target_include_directories( worth_private_message
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   worth_private_message

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
