# Set Project name and version
project (GDALAOI)

set (GDALAOI_VERSION_MAJOR 1)
set (GDALAOI_VERSION_MINOR 0)

set (PROJECT_SOURCE_DIR src)

option(BUILD_SHARED_LIBS "Build with shared library" ON)

set(GDALAOI_LIB_NAME ogr_AOI)

set(GDAL_INCLUDE_DIR /usr/local/include CACHE PATH "Include PATH for GDAL")
set(GDAL_LIB_PATH /usr/local/lib CACHE PATH "Library PATH for GDAL")
set(GDAL_SRC_PATH /usr/local/lib/src/gdal CACHE PATH "Source PATH for GDAL")
set(GDAL_HFA_SRC_PATH ${GDAL_SRC_PATH}/frmts/hfa)

###############################################################################
# CMake settings
cmake_minimum_required(VERSION 2.6.0)

IF(NOT CMAKE_BUILD_TYPE)
  #SET(CMAKE_BUILD_TYPE "DEBUG")
  SET(CMAKE_BUILD_TYPE "RELEASE")
  #SET(CMAKE_BUILD_TYPE "RELWITHDEBINFO")
  #SET(CMAKE_BUILD_TYPE "MINSIZEREL")
ENDIF()

set(CMAKE_COLOR_MAKEFILE ON)

# Allow advanced users to generate Makefiles printing detailed commands
mark_as_advanced(CMAKE_VERBOSE_MAKEFILE)

# Path to additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ./cmake/modules/)
###############################################################################

###############################################################################
# Platform and compiler specific settings

# Recommended C++ compilation flags
# -Weffc++
set(GDALUTILS_CXX_FLAGS "-pedantic -Wall -Wpointer-arith -Wcast-align -Wcast-qual -Wredundant-decls -Wno-long-long")

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC ${GDALUTILS_CXX_FLAGS}")
  if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98")
  endif()

elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR "${CMAKE_CXX_COMPILER}" MATCHES "clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GDALUTILS_CXX_FLAGS}")
endif()

if (APPLE)
  set(SO_EXT dylib)
 # set(CMAKE_FIND_FRAMEWORK "LAST")
elseif(WIN32)
  set(SO_EXT dll)
else()
  set(SO_EXT so)
endif(APPLE)
###############################################################################

include_directories(${GDAL_INCLUDE_DIR} ${GDAL_HFA_SRC_PATH})
set(GDAL_LIBRARIES -L${GDAL_LIB_PATH} -lgdal)

###############################################################################
# Build library

set(GDALAOI_SRCS ${PROJECT_SOURCE_DIR}/aoidatasource.cpp ${PROJECT_SOURCE_DIR}/aoidriver.cpp ${PROJECT_SOURCE_DIR}/aoilayer.cpp ${PROJECT_SOURCE_DIR}/aoiproj.cpp)

add_library( ${GDALAOI_LIB_NAME} ${GDALAOI_SRCS})

# remove the leading "lib" as GDAL won't look for files with this prefix
set_target_properties(${GDALAOI_LIB_NAME} PROPERTIES PREFIX "")

target_link_libraries(${GDALAOI_LIB_NAME} ${GDAL_LIBRARIES} )
install (TARGETS ${GDALAOI_LIB_NAME} DESTINATION gdalplugins)

