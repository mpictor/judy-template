
cmake_minimum_required(VERSION 2.8)
set( CMAKE_BUILD_TYPE "RelWithDebInfo" )

set( JUDYS_SOURCES judy64.c judy64.h )

add_library( judy_lib ${JUDYS_SOURCES} )

add_executable( pennysort judyPennySort.c sort.c ${JUDYS_SOURCES} )
add_executable( hexsort judyHexSort.c sort.c ${JUDYS_SOURCES} )
set_target_properties( pennysort hexsort PROPERTIES COMPILE_FLAGS "-DSTANDALONE -march=native" )


add_executable( judyLtest judyLtest.cc )
target_link_libraries( judyLtest judy_lib )

# judyArray.h
# int-int sources?