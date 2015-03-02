cmake_minimum_required(VERSION 2.6.0)

# pull in the pods macros. See cmake/pods.cmake for documentation
set(POD_NAME erf-couchdb)
include(cmake/pods.cmake)


add_definitions( -Wall -fdiagnostics-show-option -Wno-unused-local-typedefs -fPIC )
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")

option ( USE_PEDANTIC "Turn on -pendantic mode in gcc. This will spit out *lots* of warnings from lcm :-(, but hopefully none from the rest of the code" OFF)
if( USE_PEDANTIC )
    add_definitions( -pedantic )
endif (USE_PEDANTIC)


add_library( erf-couchdb SHARED
  src/exceptions.cpp
  src/couchdb.cpp
  src/utils.cpp
  )
pods_install_headers( 
  src/exceptions.hpp
  src/couchdb.hpp
  src/utils.hpp
  DESTINATION
  erf-couchdb
)
pods_use_pkg_config_packages(erf-couchdb 
  boost-1.54.0
  boost-1.54.0-system
  boost-1.54.0-chrono
  libcurl)
pods_install_libraries( erf-couchdb )
pods_install_pkg_config_file(erf-couchdb
    CFLAGS
    LIBS -lerf-couchdb -lpthread
    REQUIRES boost-1.54.0 boost-1.54.0-chrono boost-1.54.0-system libcurl
    VERSION 1.0.0)


# add the test subdirectory
add_subdirectory( test )
