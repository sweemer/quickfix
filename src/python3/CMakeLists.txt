set(quickfix_python_VERSION_MAJOR 16)
set(quickfix_python_VERSION_MINOR 0)
set(quickfix_python_VERSION_PATCH 1)
set(quickfix_python_VERSION ${quickfix_python_VERSION_MAJOR}.${quickfix_python_VERSION_MINOR}.${quickfix_python_VERSION_PATCH} )

find_program(SWIG_EXECUTABLE swig REQUIRED)

set(quickfix_python_INTERFACE_DIR ${PROJECT_SOURCE_DIR}/src/python)
set(quickfix_python_GENERATED_CPP ${CMAKE_CURRENT_BINARY_DIR}/QuickfixPython.cpp)
set(quickfix_python_SWIG_ARGS
    -I${PROJECT_SOURCE_DIR}/src/C++
    -threads
    -D__cpp_noexcept_function_type
    -DHAVE_SSL=1
    -DHAVE_MYSQL
    -DHAVE_POSTGRESQL
    -python
    -c++
    -o ${quickfix_python_GENERATED_CPP})

add_custom_command(
  OUTPUT ${quickfix_python_GENERATED_CPP}
  COMMAND ${SWIG_EXECUTABLE} ${quickfix_python_SWIG_ARGS} ${quickfix_python_INTERFACE_DIR}/quickfix.i
  DEPENDS
    ${quickfix_python_INTERFACE_DIR}/quickfix.i
    ${PROJECT_SOURCE_DIR}/src/quickfix.i
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND_EXPAND_LISTS
  VERBATIM)

set(quickfix_python_SOURCES ${quickfix_python_GENERATED_CPP})
set(quickfix_python_lib_name _${PROJECT_NAME})

add_library(${quickfix_python_lib_name} SHARED  ${quickfix_python_SOURCES})

target_include_directories(${quickfix_python_lib_name} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src/C++ ${PROJECT_SOURCE_DIR}/src/swig ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${quickfix_python_lib_name} ${PROJECT_NAME} Python3::Module)

if (HAVE_SSL)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_SSL=1")
  target_link_libraries(${quickfix_python_lib_name} ${PROJECT_NAME} ${OPENSSL_LIBRARIES})
endif()

set_target_properties(${quickfix_python_lib_name} PROPERTIES VERSION ${quickfix_python_VERSION} SOVERSION ${quickfix_python_VERSION_MAJOR} PREFIX "")

install(TARGETS ${quickfix_python_lib_name} DESTINATION lib/python3)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/python/ DESTINATION lib/python3
        FILES_MATCHING PATTERN "quickfix*.py"
        PATTERN test EXCLUDE)
