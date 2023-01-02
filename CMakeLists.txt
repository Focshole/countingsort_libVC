cmake_minimum_required( VERSION 3.20 )
project(
  libVC_countsort
  VERSION 2.0
  DESCRIPTION
    "Counting sort test for libvc"
  LANGUAGES CXX C)
# target
set ( EXE_NAME "run_countsort" )

# look for dependencies
list ( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )

find_package ( LibVersioningCompiler REQUIRED )
include_directories ( ${LIBVC_INCLUDE_DIRS} )
link_directories( ${LIVC_LIB_DIR} )

# finalize dependencies
set ( DEP_LIBS ${DEP_LIBS} ${LIBVC_LIBRARIES} )

# sources
set ( SRC_CNTSRT ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/kernel.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/TimeMonitor.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
                )

# compiler options
if(NOT CMAKE_BUILD_TYPE)
  message("No build type detected. Build type set to Release by default")
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)

set_source_files_properties( ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp PROPERTIES COMPILE_FLAGS
"-D_PATH_TO_KERNEL=\"\\\"${CMAKE_CURRENT_SOURCE_DIR}/\\\"\"")
# finalize target
add_executable( ${EXE_NAME} ${SRC_CNTSRT} )
target_link_libraries( ${EXE_NAME} ${DEP_LIBS} )

# install path
if ( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
  set (CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
endif ()

install ( TARGETS ${EXE_NAME} DESTINATION bin )
