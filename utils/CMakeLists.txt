###############################################################################
#
# CMake build scripts for GDAL Utilities
# 
# CMake build script created 2012/08/08 by Peter Bunting
#
# These scripts were initial based on those used for libLAS (http://liblas.org/)
# and then edited for SPDLib (http://www.spdlib.org) before being edited again
# for RSGISLib and then being used for GDALUtils.
#
# History
# 2010/12/14 - Created by Peter Bunting for SPDLib
# 2012/02/07 - Edited by Peter Bunting for RSGISLib
# 2012/08/08 - Edited by Peter Bunting for GDAL Utils
#
###############################################################################

###############################################################################
# Set Project name and version
project (GDALUTILS)

set (GDALUTILS_VERSION_MAJOR 1)
set (GDALUTILS_VERSION_MINOR 1)

set (PROJECT_SOURCE_DIR src)

option(BUILD_SHARED_LIBS "Build with shared library" ON)

set(GDALUTILS_COMMON_LIB_NAME gdalcommon)
set(LIBIMGF90_BIN_NAME libimgf90)
set(OGRDISSOLVE_BIN_NAME ogrdissolve)

set(LIBIMGF90 FALSE CACHE BOOL "Build libimgf90")
set(OGRDISSOLVE TRUE CACHE BOOL "Build ogrdissolve")

set(GDAL_INCLUDE_DIR /usr/local/include CACHE PATH "Include PATH for GDAL")
set(GDAL_LIB_PATH /usr/local/lib CACHE PATH "Library PATH for GDAL")

set(GEOS_INCLUDE_DIR /usr/local/include CACHE PATH "Include PATH for GEOS")
set(GEOS_LIB_PATH /usr/local/lib CACHE PATH "Library PATH for GEOS")
###############################################################################

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

###############################################################################
# Check the required libraries are present

include_directories(${GEOS_INCLUDE_DIR})
set(GEOS_LIBRARIES -L${GEOS_LIB_PATH} -lgeos -lgeos_c )

include_directories(${GDAL_INCLUDE_DIR})
set(GDAL_LIBRARIES -L${GDAL_LIB_PATH} -lgdal)

###############################################################################

###############################################################################
# Build executables

add_library( ${GDALUTILS_COMMON_LIB_NAME} ${PROJECT_SOURCE_DIR}/common/glayer.c ${PROJECT_SOURCE_DIR}/common/gutils.c ${PROJECT_SOURCE_DIR}/common/ginfo.h ${PROJECT_SOURCE_DIR}/common/fparse.c )
target_link_libraries(${GDALUTILS_COMMON_LIB_NAME} ${GDAL_LIBRARIES} )
install (TARGETS ${GDALUTILS_COMMON_LIB_NAME} DESTINATION lib)
set(GDALCOMMON_INCLUDE_FILES ${PROJECT_SOURCE_DIR}/common/fparse.h ${PROJECT_SOURCE_DIR}/common/ginfo.h ${PROJECT_SOURCE_DIR}/common/glayer.h ${PROJECT_SOURCE_DIR}/common/gutils.h)
install (FILES ${GDALCOMMON_INCLUDE_FILES} DESTINATION include)
include_directories ("${PROJECT_SOURCE_DIR}/common")

if(LIBIMGF90)
	add_executable(${LIBIMGF90_BIN_NAME} ${PROJECT_SOURCE_DIR}/libimgf90/libimgf90.c ${PROJECT_SOURCE_DIR}/libimgf90/override-specs.h )
	target_link_libraries (${LIBIMGF90_BIN_NAME} ${GDALUTILS_COMMON_LIB_NAME} ${GDAL_LIBRARIES} )
	install (TARGETS ${LIBIMGF90_BIN_NAME} DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif(LIBIMGF90)

if(OGRDISSOLVE)
	add_executable(${OGRDISSOLVE_BIN_NAME} ${PROJECT_SOURCE_DIR}/ogrdissolve/ogrdissolve.cpp )
	target_link_libraries (${OGRDISSOLVE_BIN_NAME} ${GDALUTILS_COMMON_LIB_NAME} ${GDAL_LIBRARIES} ${GEOS_LIBRARIES} )
	install (TARGETS ${OGRDISSOLVE_BIN_NAME} DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif(OGRDISSOLVE)

###############################################################################


