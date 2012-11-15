#include <iostream>
#include <stdint.h>
#include "judyLArray.h"

int main() {

    judyLArray< uint64_t, uint64_t > jl;
    jl.insert(5, 12);
    jl.insert(6, 2);
    jl.insert(7, 312);
    jl.insert(8, 412);
    judyLArray< uint64_t, uint64_t >::pair kv = jl.atOrAfter(4);
    std::cout << "k " << kv.key << " v " << kv.value << std::endl;

    long v = jl.find( 9 );
    if( v != 0 || jl.success() ) {
        std::cout << "find fails - v: " << v << " success: " << jl.success() << std::endl;
    }
    v = jl.find( 7 );
    if( v != 312 || !jl.success() ) {
        std::cout << "find fails - v: " << v << " success: " << jl.success() << std::endl;
    }


    //TODO test all of judyLArray
}