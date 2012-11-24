
cmake_minimum_required(VERSION 2.8)
set( CMAKE_BUILD_TYPE "Debug" )

set( JUDYS_SOURCES judy64.c judy64.h )

add_library( judy_lib ${JUDYS_SOURCES} )

add_executable( pennysort judyPennySort.c ${JUDYS_SOURCES} )
set_target_properties( pennysort PROPERTIES COMPILE_FLAGS "-DSTANDALONE" )
# target_link_libraries( pennysort judy_lib )

add_executable( hexsort judyPennySort.c ${JUDYS_SOURCES} )
set_target_properties( hexsort PROPERTIES COMPILE_FLAGS "-DSTANDALONE -DHEXKEYS" )
# target_link_libraries( hexsort judy_lib )

add_executable( judyLtest judyLtest.cc )
target_link_libraries( judyLtest judy_lib )

# judyArray.h
# int-int sources?