
cmake_minimum_required(VERSION 2.8)
project( JudyTemplates )

include(CMakePushCheckState)
include(CheckCXXSourceRuns)

set( TEST_STD_ENABLEIF "
#include <iostream>
#include <type_traits>
template<class T>
typename std::enable_if<std::is_pointer<T>::value, T>::type foo2(T t) {return t;}
int main(int a, char ** b) {std::cout << foo2(b[0]) <<std::endl;}
" )
cmake_push_check_state()
  if( UNIX )
    set( CMAKE_REQUIRED_FLAGS "-std=c++11" )
  else( UNIX )
    # vars probably need set for MSVC11, embarcadero, etc
  endif( UNIX )
  CHECK_CXX_SOURCE_RUNS( "${TEST_STD_ENABLEIF}" HAVE_STD_ENABLEIF )   #quotes are *required*!
cmake_pop_check_state()
if( "${HAVE_STD_ENABLEIF}" )
  message(" .. Compiler supports std::is_pointer<T> and std::enable_if<B,T>")
  add_definitions( -DHAVE_STD_ENABLEIF )
  if(UNIX)
    add_definitions( "-std=c++11" )
  endif()
else()
  message(" !! Compiler does not support std::is_pointer<T> and/or std::enable_if<B,T>")
endif()

if( NOT DEFINED CMAKE_BUILD_TYPE )
  # set( CMAKE_BUILD_TYPE "RelWithDebInfo" ) #optimize, but include debug info
  set( CMAKE_BUILD_TYPE "Release" )
endif( NOT DEFINED CMAKE_BUILD_TYPE )

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

if( NOT DEFINED JUDY_OPTIMIZE_FLAGS )
  if( CMAKE_COMPILER_IS_GNUCC )
    set( JUDY_OPTIMIZE_FLAGS "-march=native" )

    #test for LTO; this uses an internal variable so it may break
    if( DEFINED CMAKE_C_COMPILER_VERSION )
      if( ${CMAKE_C_COMPILER_VERSION} VERSION_GREATER 4.7.0 )
        set( JUDY_OPTIMIZE_FLAGS "${JUDY_OPTIMIZE_FLAGS} -flto" )
        message( "  --  GCC version: ${CMAKE_C_COMPILER_VERSION}. Enabling link-time optimization." )
      endif( ${CMAKE_C_COMPILER_VERSION} VERSION_GREATER 4.7.0 )
    endif( DEFINED CMAKE_C_COMPILER_VERSION )
  #elseif( MSVC )
    #  set( JUDY_OPTIMIZE_FLAGS "..." ) # <--- set MSVC flags here <---
  else()
    message( "Unrecognized compiler - no optimization flags set. Edit CMakeLists.txt or set JUDY_OPTIMIZE_FLAGS." )
    set( JUDY_OPTIMIZE_FLAGS "" )
  endif( CMAKE_COMPILER_IS_GNUCC )
endif( NOT DEFINED JUDY_OPTIMIZE_FLAGS )
add_definitions( ${JUDY_OPTIMIZE_FLAGS} )

set( JUDYS_SOURCES src/judy.c src/judy.h )

if( CMAKE_COMPILER_IS_GNUCC )
  add_definitions( -pedantic -W -Wall -Wundef -Wfloat-equal -Wshadow -Winline -Wno-long-long )
endif( CMAKE_COMPILER_IS_GNUCC )

add_library( judy_lib STATIC ${JUDYS_SOURCES} )

include_directories( src )

if( ENABLE_TESTING )
  include( CTest )
  include_directories( test )
  enable_testing()

  add_executable( pennysort test/pennySort.c test/sort.c ${JUDYS_SOURCES} )
  add_executable( hexsort test/hexSort.c test/sort.c ${JUDYS_SOURCES} )
  set_target_properties( pennysort hexsort PROPERTIES COMPILE_FLAGS "-DSTANDALONE" )


  add_executable( judyLtest test/judyLtest.cc )
  target_link_libraries( judyLtest judy_lib )
  add_test( judyLtest judyLtest )

  add_executable( judyL2test test/judyL2test.cc )
  target_link_libraries( judyL2test judy_lib )
  add_test( judyL2test judyL2test )

  add_executable( judyStest test/judyStest.cc )
  target_link_libraries( judyStest judy_lib )
  add_test( judyStest judyStest )

  add_executable( judyS2test test/judyS2test.cc )
  target_link_libraries( judyS2test judy_lib )
  add_test( judyS2test judyS2test )

endif( ENABLE_TESTING )
