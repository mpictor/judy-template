# Judy Template

This uses Karl Malbrain's [implementation](http://code.google.com/p/judyarray/) of the Judy Array.

For more information, see that site, [Doug Baskins' original](http://judy.sourceforge.net/) on sourceforge, or [wikipedia](http://en.wikipedia.org/wiki/Judy_array).

# What's the difference between this and the above?

* Doug Baskins code is licenced under the LGPL. While more permissive than the GPL, it's still not always acceptable. His code is very fast but weighs in at ~20k lines.

* Karl Malbrain's code is a single file containing the judy array and the test code; use requires creating a header.

* Both of the above are written in C, so they don't fit neatly into object-oriented C++.

* Unlike Doug's code, this is ~1250 lines. Unlike Karl's, this is split into several files.

# Templates
* `judyLArray` - a C++ template wrapper for an int-int Judy Array. JudyKey and JudyValue must be integer types and the same size as a pointer (i.e. 32- or 64-bit)
* `judySArray` - Same as judyLArray, but with string-int mapping. The above restrictions on JudyValue apply here as well.
* **TODO** - single-key, multi-value versions of the above

# Files
* `CMakeLists.txt` - CMake build logic. If you don't have CMake, it should be quite easy to write a file for the build system of your choice.

## src/
* `judy.c`, `judy.h` - implementation of the Judy Array
* `judyLArray.h` - the judyLArray template
* `judySArray.h` - the judySArray template

## test/
* `hexSort.c` - Sorts a file where each line contains 32 hex chars. Compiles to `hexsort`, which is the same executable as compiling Karl's code with `-DHEXSORT -DSTANDALONE`
* `pennySort.c` - Sorts strings; compiles to `pennysort`. Same as compiling Karl's code with `-DSTANDALONE`.
* `sort.c`, `sort.h` - Karl's sorting functions. Only used by `hexsort` and `pennysort`.
* `judyLtest.cc` - an incomplete test of the judyLArray template.
* `judyStest.cc` - an incomplete test of the judySArray template.


# Compiling
* requires C and C++ compilers, CMake.
* from the command line:
**  `mkdir build; cd build`
**  `cmake .. -DENABLE_TESTING=TRUE`
**  `make`

# License

Karl Malbrain's code is public domain; what I've added is public domain as well.

# Contact
mpictor -a-t- gmail
