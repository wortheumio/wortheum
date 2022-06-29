file(GLOB HEADERS "include/worth/chain/*.hpp" "include/worth/chain/util/*.hpp" "include/worth/chain/wmt_objects/*.hpp" "include/worth/chain/wps_objects/*.hpp")

## SORT .cpp by most likely to change / break compile
add_library( worth_chain

             # As database takes the longest to compile, start it first
             database.cpp
             index.cpp

             wmt_evaluator.cpp
             wps_evaluator.cpp

             wmt_objects/wmt_market_maker.cpp
             wmt_objects/nai_pool.cpp
             worth_evaluator.cpp
             worth_objects.cpp
             required_action_evaluator.cpp
             optional_action_evaluator.cpp

             witness_schedule.cpp
             fork_database.cpp

             shared_authority.cpp
             block_log.cpp

             generic_custom_operation_interpreter.cpp

             util/rd.cpp
             util/reward.cpp
             util/impacted.cpp
             util/advanced_benchmark_dumper.cpp
             util/wmt_token.cpp
             util/wps_processor.cpp
             util/wps_helper.cpp

             ${HEADERS}
           )

target_link_libraries( worth_chain worth_jsonball worth_protocol fc chainbase worth_schema appbase mira
                       ${PATCH_MERGE_LIB} )
target_include_directories( worth_chain
                            PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include" )

if( CLANG_TIDY_EXE )
   set_target_properties(
      worth_chain PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
   )
endif( CLANG_TIDY_EXE )

if(MSVC)
  set_source_files_properties( database.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

INSTALL( TARGETS
   worth_chain

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/worth/chain" )
