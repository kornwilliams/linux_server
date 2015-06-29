MESSAGE( STATUS "DEBUG: ${${PROJECT_NAME}_SOURCE_DIR}" )

GET_FILENAME_COMPONENT( BW_BUILD_DIR "${${PROJECT_NAME}_SOURCE_DIR}" ABSOLUTE )
GET_FILENAME_COMPONENT( BW_ROOT_DIR "${BW_BUILD_DIR}/../" ABSOLUTE )
GET_FILENAME_COMPONENT( BW_CMAKE_DIR "${BW_ROOT_DIR}/cmake/" ABSOLUTE )
GET_FILENAME_COMPONENT( BW_TESTS_DIR "${BW_ROOT_DIR}/tests/" ABSOLUTE )

MESSAGE( STATUS "DEBUG: ${BW_BUILD_DIR}" )
MESSAGE( STATUS "DEBUG: ${BW_ROOT_DIR}" )
MESSAGE( STATUS "DEBUG: ${BW_CMAKE_DIR}" )

SET(RUNTIME_OUTPUT_PATH ${BW_BUILD_DIR})
SET(ARCHIVE_OUTPUT_PATH ${BW_BUILD_DIR})

INCLUDE_DIRECTORIES(
  ${BW_ROOT_DIR}
  /usr/include/mysql++
  /usr/local/mysql/include
  /usr/include
  /usr/local/include
  /usr/local/include/zookeeper
  /opt/curl/include
)

LINK_DIRECTORIES(
  /usr/lib
  /usr/local/lib
  /usr/lib64/nptl
  /opt/curl/lib
  /usr/lib64/mysql
)

# LINK_LIBRARIES(pthread rt log4cplus mysqlpp crypto ssl)
LINK_LIBRARIES(pthread rt)

MACRO (BW_ADD_STATIC_LIBRARY directory library)
  MESSAGE(STATUS "DEBUG: ${ARGN}")
  AUX_SOURCE_DIRECTORY("${BW_ROOT_DIR}/${directory}" ${directory}_SOURCE_FILES)

  MESSAGE(STATUS "DEBUG: ${BW_ROOT_DIR}/${directory} source files = ${${directory}_SOURCE_FILES}")
  ADD_LIBRARY(${library} STATIC ${${directory}_SOURCE_FILES})
  TARGET_LINK_LIBRARIES( ${library} ${ARGN} )
ENDMACRO (BW_ADD_STATIC_LIBRARY)

MACRO (BW_ADD_SHARED_LIBRARY directory library)
  MESSAGE(STATUS "DEBUG: ${ARGN}")
  AUX_SOURCE_DIRECTORY("${BW_ROOT_DIR}/${directory}" ${directory}_SOURCE_FILES)

  MESSAGE(STATUS "DEBUG: ${BW_ROOT_DIR}/${directory} source files = ${${directory}_SOURCE_FILES}")
  ADD_LIBRARY(${library} SHARED ${${directory}_SOURCE_FILES})
  TARGET_LINK_LIBRARIES( ${library} ${ARGN} )
ENDMACRO (BW_ADD_SHARED_LIBRARY)

MACRO (BW_ADD_EXECUTABLE directory executable)
  MESSAGE(STATUS "DEBUG: ${ARGN}")
  AUX_SOURCE_DIRECTORY("${BW_ROOT_DIR}/${directory}" ${directory}_SOURCE_FILES)

  MESSAGE(STATUS "DEBUG: ${BW_ROOT_DIR}/${directory} source files = ${${directory}_SOURCE_FILES}")
  ADD_EXECUTABLE(${executable} EXCLUDE_FROM_ALL ${${directory}_SOURCE_FILES})
  TARGET_LINK_LIBRARIES( ${executable} ${ARGN} )
ENDMACRO (BW_ADD_EXECUTABLE)

MACRO (BW_ADD_TEST_CASE test_case)
  ADD_EXECUTABLE(${test_case} EXCLUDE_FROM_ALL ${BW_TESTS_DIR}/${test_case}.cc)
  TARGET_LINK_LIBRARIES( ${test_case} ${ARGN} gtest)
ENDMACRO (BW_ADD_TEST_CASE)

MACRO (GENERATE_THRIFT_RULES directory)
  FILE(GLOB THRIFT_SOURCES
    RELATIVE "${BW_ROOT_DIR}/${directory}" 
    "${BW_ROOT_DIR}/${directory}/*.thrift")
  MESSAGE("THRIFT_SOURCES : ${THRIFT_SOURCES}")
  SET(output_dir "${BW_BUILD_DIR}/thrift-gen")

  INCLUDE_DIRECTORIES("/usr/local/include/thrift/" 
    "${output_dir}/gen-cpp/")

  FOREACH(thrift_source ${THRIFT_SOURCES})
    STRING(REGEX REPLACE "\\.thrift" "_constants.cpp" constants_cpp "${thrift_source}" )
    MESSAGE(${constants_cpp})
    SET(out_file "${output_dir}/gen-cpp/${constants_cpp}")

    STRING(REGEX REPLACE "\\.thrift" "_types.cpp" types_cpp "${thrift_source}" )
    MESSAGE(${types_cpp})
    SET(out_file "${out_file}" "${output_dir}/gen-cpp/${types_cpp}")

    STRING(REGEX REPLACE "\\.thrift" ".cpp" stub_cpp "${thrift_source}" )

    if(thrift_source MATCHES ".*Service.thrift")
      MESSAGE(${stub_cpp})
      SET(out_file "${out_file}" "${output_dir}/gen-cpp/${stub_cpp}")
    endif()

    # add_dependencies(${stub_cpp} ${thrift_source})

    # skeleton代码，是否链接进来?
    # STRING(REGEX REPLACE "\\.thrift" "_server.skeleton.cpp" skeleton_cpp "${thrift_source}" )
    # MESSAGE(${skeleton_cpp})
    # SET(out_file "${out_file}" "${output_dir}/gen-cpp/${skeleton_cpp}")

    MESSAGE(${out_file})

    add_custom_command(OUTPUT ${out_file}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${output_dir}
      # COMMAND ${CMAKE_COMMAND} -E make_directory "${output_dir}" # 不能带引号!
      COMMAND thrift -o "${output_dir}" -gen cpp "${BW_ROOT_DIR}/${directory}/${thrift_source}"
        	  DEPENDS ${BW_ROOT_DIR}/${directory}/${thrift_source}
      ) 

    STRING(REGEX REPLACE "\\.thrift" "" libname "${thrift_source}" )
    MESSAGE("add library thrift_${libname} ...")
    add_library("thrift_${libname}" STATIC ${out_file}) 
    TARGET_LINK_LIBRARIES( "thrift_${libname}" thrift )
  ENDFOREACH(thrift_source ${THRIFT_SOURCES})
ENDMACRO (GENERATE_THRIFT_RULES)

GENERATE_THRIFT_RULES("melon/idl")

BW_ADD_STATIC_LIBRARY(base base log4cplus boost_system)
BW_ADD_STATIC_LIBRARY(fcgi_service fcgi_service fcgi boost_thread base)
BW_ADD_STATIC_LIBRARY(redis_executor redis_executor base hiredis)
BW_ADD_STATIC_LIBRARY(database database mysqlpp mysqlclient_r rt)
BW_ADD_STATIC_LIBRARY(melon/client melon_client base boost_thread zookeeper_mt boost_regex)
BW_ADD_STATIC_LIBRARY(melon/service melon_service thrift_BaseService thriftnb event zookeeper_mt)
BW_ADD_EXECUTABLE(service_runner service_runner dl)
BW_ADD_EXECUTABLE(fcgi_demo fcgi_demo fcgi_service)
