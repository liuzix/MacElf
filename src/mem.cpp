//
//  mem.cpp
//  MacElf
//
//  Created by Zixiong Liu on 7/19/18.
//  Copyright © 2018 Zixiong Liu. All rights reserved.
//

#include "mem.hpp"
#include <sys/mman.h>


void* pageAllocate(size_t nBytes) {
    void* ptr = mmap(nullptr, nBytes, PROT_READ | PROT_WRITE | PROT_EXEC,
         MAP_PRIVATE | MAP_ANON | MAP_ANONYMOUS, -1, 0);
    if (ptr) {
        return ptr;
    } else {
        throw "mmap failed";
    }
}
