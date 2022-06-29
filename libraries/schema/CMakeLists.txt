file(GLOB HEADERS "include/worth/schema/*.hpp")
add_library( worth_schema schema.cpp ${HEADERS} )
target_link_libraries( worth_schema fc )
target_include_directories( worth_schema PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   worth_schema

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
install( FILES ${HEADERS} DESTINATION "include/worth/schema" )
