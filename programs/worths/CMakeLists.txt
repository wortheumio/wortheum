add_executable( worths main.cpp )
if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling worths with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()

if( WORTHEUM_STATIC_BUILD )
   target_link_libraries( worths PRIVATE
      "-static-libstdc++ -static-libgcc"
      appbase
      worth_utilities
      worth_plugins
      ${CMAKE_DL_LIBS}
      ${PLATFORM_SPECIFIC_LIBS}
   )
else( WORTHEUM_STATIC_BUILD )
   target_link_libraries( worths PRIVATE
      appbase
      worth_utilities
      worth_plugins
      ${CMAKE_DL_LIBS}
      ${PLATFORM_SPECIFIC_LIBS}
   )
endif( WORTHEUM_STATIC_BUILD )

if( CLANG_TIDY_EXE )
   set_target_properties(
      worths PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
   )
endif( CLANG_TIDY_EXE )

install( TARGETS
   worths

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
