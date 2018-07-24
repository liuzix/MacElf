//
//  main.cpp
//  MacElf
//
//  Created by Zixiong Liu on 7/19/18.
//  Copyright © 2018 Zixiong Liu. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <Hypervisor/hv.h>
#include <Hypervisor/hv_vmx.h>

#include "mem.hpp"
#include "debug.hpp"
#include "gdt.hpp"
#include "vcpu.hpp"

#define INIT_MEM_SIZE (4096 * 16)
#define INIT_MEM_BASE 0x4000000
#define GDT_GUEST_MEM 0x1000

#define INIT_STACK_SIZE 4096
#define INIT_STACK_BOTTOM 0xf00000000

#define TOTAL_GUEST_MEM_BYTES ((size_t)4 * 1024 * 1024 * 1024)

void readKernel(const char* fileName, void* hostPtr, size_t size) {
    std::ifstream f(fileName, std::ios::in | std::ios::binary);
    f.read((char*)hostPtr, size);
}

int main(int argc, const char * argv[]) {
    std::cout << "Creating VM instance..." << std::endl;
    hvCheck(hv_vm_create(HV_VM_DEFAULT));
    
    size_t nPages = TOTAL_GUEST_MEM_BYTES / 4096;
    GuestPhysicalMemoryManager guestPhysicalMemoryManager(nPages);
    GuestVirtualMemoryManager guestVirtualMemoryManager(&guestPhysicalMemoryManager);

    
    void* initMem = guestVirtualMemoryManager.allocateGuestVirtual(INIT_MEM_BASE, INIT_MEM_SIZE, false);
    readKernel("kernel/kernel.bin", initMem, INIT_MEM_SIZE);

    GDT gdt;
    gdt.addEntry(GDTEntry::getCode(0));
    gdt.addEntry(GDTEntry::getData(0));
    gdt.addEntry(GDTEntry::getCode(3));
    gdt.addEntry(GDTEntry::getData(3));
    gdt.writeToMemory();
    
    uint64_t gdtPhys = guestPhysicalMemoryManager.allocate(gdt.getMemRegion());
    guestVirtualMemoryManager.mapGuestPhysicalToGuestVirtual(gdtPhys, gdtPhys, false);
    
    VCPU vcpu(&guestVirtualMemoryManager);
    vcpu.loadGDT(gdtPhys, gdt.getSize());
    
    guestVirtualMemoryManager.allocateGuestVirtual(INIT_STACK_BOTTOM, INIT_MEM_SIZE, true);
    
    uint64_t stackTop = INIT_STACK_BOTTOM + INIT_STACK_SIZE;
    vcpu.setRSP(stackTop);
    vcpu.run(INIT_MEM_BASE);
    
    return 0;
}
