#include <iostream>
#include <stdint.h>
#include <stdlib.h>

#include "judyL2Array.h"
typedef judyL2Array< uint64_t, uint64_t > jl2a;

int main() {
    std::cout.setf(std::ios::boolalpha);
    jl2a jl;
    std::cout << "size of judyL2Array: " << sizeof( jl ) << std::endl;
    jl.insert(5, 12);
    jl.insert(6, 2);
    jl.insert(7, 312);
    jl.insert(11, 412);
    jl.insert(7, 313);
    jl2a::pair kv = jl.atOrAfter(4);
    std::cout << "k " << kv.key << " v " << kv.value << std::endl;

    jl2a::cvector * v = jl.find( 8 );
    if( v || jl.success() ) {
        std::cout << "find: false positive - v: " << v << " success: " << jl.success() << std::endl;
        exit( EXIT_FAILURE );
    }
    v = jl.find( 11 );
    if( !v || !jl.success() ) {
        std::cout << "find: false negative - v: " << v << " success: " << jl.success() << std::endl;
        exit( EXIT_FAILURE );
    }


    //TODO test all of judyL2Array
    exit( EXIT_SUCCESS );
}