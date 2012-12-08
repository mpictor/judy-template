
cmake_minimum_required(VERSION 2.8)
if( NOT DEFINED CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE "RelWithDebInfo" ) #optimize, but include debug info
endif( NOT DEFINED CMAKE_BUILD_TYPE )

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
    else()
        message( "Unrecognized compiler - no optimization flags set. Edit CMakeLists.txt or set JUDY_OPTIMIZE_FLAGS." )
        set( JUDY_OPTIMIZE_FLAGS "" )
    endif( CMAKE_C_COMPILER MATCHES "gcc" )
endif( NOT DEFINED JUDY_OPTIMIZE_FLAGS )

set( JUDYS_SOURCES judy64.c judy64.h )

add_library( judy_lib ${JUDYS_SOURCES} )
set_target_properties( judy_lib PROPERTIES COMPILE_FLAGS "${JUDY_OPTIMIZE_FLAGS}" )

add_executable( pennysort judyPennySort.c sort.c ${JUDYS_SOURCES} )
add_executable( hexsort judyHexSort.c sort.c ${JUDYS_SOURCES} )
set_target_properties( pennysort hexsort PROPERTIES COMPILE_FLAGS "-DSTANDALONE ${JUDY_OPTIMIZE_FLAGS}" )


add_executable( judyLtest judyLtest.cc )
target_link_libraries( judyLtest judy_lib )

# judyArray.h
# int-int sources?