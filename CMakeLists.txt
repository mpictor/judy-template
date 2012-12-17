
cmake_minimum_required(VERSION 2.8)

if( NOT DEFINED CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE "RelWithDebInfo" ) #optimize, but include debug info
endif( NOT DEFINED CMAKE_BUILD_TYPE )

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

if( NOT DEFINED JUDY_OPTIMIZE_FLAGS )
    if( CMAKE_C_COMPILER MATCHES "gcc" )
        set( JUDY_OPTIMIZE_FLAGS "-march=native" )

        #test for LTO; this uses an internal variable so it may break
        if( DEFINED CMAKE_C_COMPILER_VERSION )
#             message( "  --  GCC version: ${CMAKE_C_COMPILER_VERSION}" )
            if( ${CMAKE_C_COMPILER_VERSION} VERSION_GREATER 4.7.0 )
                set( JUDY_OPTIMIZE_FLAGS "${JUDY_OPTIMIZE_FLAGS} -flto" )
            message( "  --  GCC version: ${CMAKE_C_COMPILER_VERSION}. Enabling link-time optimization." )
            endif( ${CMAKE_C_COMPILER_VERSION} VERSION_GREATER 4.7.0 )
        endif( DEFINED CMAKE_C_COMPILER_VERSION )
    #elseif( CMAKE_C_COMPILER MATCHES "Visual" )
    #  set( JUDY_OPTIMIZE_FLAGS "..." ) # <--- set MSVC flags here <---
    else()
        message( "Unrecognized compiler - no optimization flags set. Edit CMakeLists.txt or set JUDY_OPTIMIZE_FLAGS." )
        set( JUDY_OPTIMIZE_FLAGS "" )
    endif( CMAKE_C_COMPILER MATCHES "gcc" )
endif( NOT DEFINED JUDY_OPTIMIZE_FLAGS )

set( JUDYS_SOURCES src/judy.c src/judy.h )

add_library( judy_lib ${JUDYS_SOURCES} )
set_target_properties( judy_lib PROPERTIES COMPILE_FLAGS "${JUDY_OPTIMIZE_FLAGS}" )

include_directories( src )

if( ENABLE_TESTING )
  include( CTest )
  include_directories( test )
  enable_testing()

  add_executable( pennysort test/pennySort.c test/sort.c ${JUDYS_SOURCES} )
  add_executable( hexsort test/hexSort.c test/sort.c ${JUDYS_SOURCES} )
  set_target_properties( pennysort hexsort PROPERTIES COMPILE_FLAGS "-DSTANDALONE ${JUDY_OPTIMIZE_FLAGS}" )


  add_executable( judyLtest test/judyLtest.cc )
  target_link_libraries( judyLtest judy_lib )

  add_executable( judyStest test/judyStest.cc )
  target_link_libraries( judyStest judy_lib )

  set_target_properties( judyLtest judyStest PROPERTIES COMPILE_FLAGS "${JUDY_OPTIMIZE_FLAGS}" )
endif( ENABLE_TESTING )
