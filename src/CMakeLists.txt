add_subdirectory(C++)

if( WIN32 OR ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )

if (QUICKFIX_EXAMPLES)
  add_executable(at at.cpp getopt.c)
  
  target_include_directories(at PRIVATE ${PROJECT_SOURCE_DIR} 
    ${PROJECT_SOURCE_DIR}/src/C++)
    
  target_link_libraries(at ${PROJECT_NAME})
  
  if (NOT WIN32)
  ADD_CUSTOM_TARGET(at_target ALL
                    COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE:at> ${PROJECT_SOURCE_DIR}/test/at)
  else()
  set_target_properties(at PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/test/debug/at/
                        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/test/release/at/
                        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PROJECT_SOURCE_DIR}/test/release/at/)
  endif()  
endif()  

if (QUICKFIX_EXAMPLES)
  add_executable(pt pt.cpp
    ${PROJECT_SOURCE_DIR}/src/C++/test/catch_amalgamated.cpp
    ${PROJECT_SOURCE_DIR}/src/C++/test/TestHelper.cpp)

  target_compile_definitions(pt PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN)

  target_include_directories(pt PRIVATE ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src/C++
    ${PROJECT_SOURCE_DIR}/src/C++/test)

  target_link_libraries(pt ${PROJECT_NAME})
  
  if (NOT WIN32)
  ADD_CUSTOM_TARGET(pt_target ALL
                    COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE:pt> ${PROJECT_SOURCE_DIR}/test/pt)
  else()
  set_target_properties(pt PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/test/debug/pt/
                        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/test/release/pt/
                        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PROJECT_SOURCE_DIR}/test/release/pt/)
  endif()  
endif()  

endif()

if (HAVE_PYTHON3)
add_subdirectory(python3)
endif ()

if (BUILD_TESTING AND QUICKFIX_TESTS)
  set(quickfix_test_working_dir "${PROJECT_SOURCE_DIR}/test")
  set(quickfix_spec_path "${PROJECT_SOURCE_DIR}/spec")

  if (TARGET ut)
    add_test(NAME ut
             COMMAND $<TARGET_FILE:ut>
                     --quickfix-config-file ${quickfix_test_working_dir}/cfg/ut.cfg
                     --quickfix-spec-path ${quickfix_spec_path})
    set_tests_properties(ut PROPERTIES
                         WORKING_DIRECTORY "${quickfix_test_working_dir}")
  endif()

  if (TARGET at)
    if (WIN32)
      add_test(NAME at
               COMMAND ${PROJECT_SOURCE_DIR}/test/runat.bat $<CONFIG> 6666)
    else()
      add_test(NAME at
               COMMAND ${PROJECT_SOURCE_DIR}/test/runat.sh 6666)
    endif()
    set_tests_properties(at PROPERTIES
                         WORKING_DIRECTORY "${quickfix_test_working_dir}")
  endif()

  if (TARGET pt)
    set(pt_ctest_script "${CMAKE_CURRENT_BINARY_DIR}/RunPtCTest.cmake")
    file(WRITE "${pt_ctest_script}" [=[
if(NOT DEFINED PT_EXECUTABLE)
  message(FATAL_ERROR "PT_EXECUTABLE is required")
endif()

if(NOT DEFINED SPEC_PATH)
  message(FATAL_ERROR "SPEC_PATH is required")
endif()

if(NOT DEFINED TEST_WORKING_DIRECTORY)
  message(FATAL_ERROR "TEST_WORKING_DIRECTORY is required")
endif()

if(NOT DEFINED NETWORK_PORT)
  message(FATAL_ERROR "NETWORK_PORT is required")
endif()

execute_process(
  COMMAND "${PT_EXECUTABLE}" "--quickfix-spec-path" "${SPEC_PATH}" "-#" "~[network]"
  WORKING_DIRECTORY "${TEST_WORKING_DIRECTORY}"
  RESULT_VARIABLE quickfix_pt_non_network_result)
if(NOT quickfix_pt_non_network_result EQUAL 0)
  message(FATAL_ERROR "pt non-network benchmark phase failed with exit code ${quickfix_pt_non_network_result}")
endif()

execute_process(
  COMMAND "${PT_EXECUTABLE}" "--quickfix-spec-path" "${SPEC_PATH}" "-#" "[network]" "--port" "${NETWORK_PORT}"
  WORKING_DIRECTORY "${TEST_WORKING_DIRECTORY}"
  RESULT_VARIABLE quickfix_pt_network_result)
if(NOT quickfix_pt_network_result EQUAL 0)
  message(FATAL_ERROR "pt network benchmark phase failed with exit code ${quickfix_pt_network_result}")
endif()
]=])

    add_test(NAME pt
             COMMAND ${CMAKE_COMMAND}
                     -DPT_EXECUTABLE=$<TARGET_FILE:pt>
                     -DSPEC_PATH=${quickfix_spec_path}
                     -DTEST_WORKING_DIRECTORY=${quickfix_test_working_dir}
                     -DNETWORK_PORT=6668
                     -P ${pt_ctest_script})
    set_tests_properties(pt PROPERTIES
                         WORKING_DIRECTORY "${quickfix_test_working_dir}"
                         CONFIGURATIONS Release)
  endif()
endif()
