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
    std::cout << "p1 " << kv.key << " p2 " << kv.value << std::endl;
}