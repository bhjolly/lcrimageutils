# Set Project name and version
project (LIBIMGF90)
enable_language (Fortran)

set (LIBIMGF90_VERSION_MAJOR 1)
set (LIBIMGF90_VERSION_MINOR 0)

set (PROJECT_SOURCE_DIR src)

option(BUILD_SHARED_LIBS "Build with shared library" ON)

set(LIBIMGF90_LIB_NAME imgf90)

set(GDAL_INCLUDE_PATH /usr/local/include CACHE PATH "Include PATH for GDAL")
set(GDAL_LIB_PATH /usr/local/lib CACHE PATH "Library PATH for GDAL")
set(GDALCOMMON_INCLUDE_PATH /usr/local/include CACHE PATH "Include PATH for gdalutils libgdalcommon")
set(GDALCOMMON_LIB_PATH /usr/local/include CACHE PATH "Library PATH for gdalutils libgdalcommon")

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

if (APPLE)
  set(SO_EXT dylib)
 # set(CMAKE_FIND_FRAMEWORK "LAST")
elseif(WIN32)
  set(SO_EXT dll)
else()
  set(SO_EXT so)
endif(APPLE)
###############################################################################

include_directories(${GDAL_INCLUDE_PATH} ${GDALCOMMON_INCLUDE_PATH})
if (WIN32)
  set(GDAL_LIBRARIES -LIBPATH:"${GDAL_LIB_PATH}" gdal_i.lib -LIBPATH:"${GDALCOMMON_LIB_PATH}" gdalcommon_i.lib )
else()
  set(GDAL_LIBRARIES -L${GDAL_LIB_PATH} -lgdal -L${GDALCOMMON_LIB_PATH} -lgdalcommon)
endif(WIN32)

###############################################################################
# Build library

set(LIBIMGF90_SRCS ${PROJECT_SOURCE_DIR}/libimgf90.c ${PROJECT_SOURCE_DIR}/libimgf90mod.f90)

add_library( ${LIBIMGF90_LIB_NAME} ${LIBIMGF90_SRCS})

target_link_libraries(${LIBIMGF90_LIB_NAME} ${GDAL_LIBRARIES} )
install (TARGETS ${LIBIMGF90_LIB_NAME} DESTINATION lib)
install (FILES libimgf90.mod DESTINATION mod)

