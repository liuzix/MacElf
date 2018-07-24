//
//  main.cpp
//  MacElf
//
//  Created by Zixiong Liu on 7/19/18.
//  Copyright © 2018 Zixiong Liu. All rights reserved.
//

#include <iostream>
#include <Hypervisor/hv.h>
#include <Hypervisor/hv_vmx.h>

#include "mem.hpp"
#include "debug.hpp"
#include "gdt.hpp"

#define INIT_MEM_SIZE (4096 * 16)
#define INIT_MEM_BASE 0x4000000
#define GDT_GUEST_MEM 0x1000


int main(int argc, const char * argv[]) {
    std::cout << "Creating VM instance..." << std::endl;
    hvCheck(hv_vm_create(HV_VM_DEFAULT));
    
    void* initMem = valloc(INIT_MEM_SIZE);
    
    hvCheck(hv_vm_map(initMem, INIT_MEM_BASE, INIT_MEM_SIZE,
        HV_MEMORY_READ | HV_MEMORY_WRITE | HV_MEMORY_EXEC));

    GDT gdt;
    gdt.addEntry(GDTEntry::getData(0));
    gdt.addEntry(GDTEntry::getCode(0));
    gdt.addEntry(GDTEntry::getData(3));
    gdt.addEntry(GDTEntry::getCode(3));
    gdt.writeToMemory();
    
    hvCheck(hv_vm_map(gdt.getMemRegion(), GDT_GUEST_MEM,
        gdt.getSize(), HV_MEMORY_READ | HV_MEMORY_WRITE));
    
    
    
    return 0;
}
