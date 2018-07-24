//
//  mem.cpp
//  MacElf
//
//  Created by Zixiong Liu on 7/19/18.
//  Copyright © 2018 Zixiong Liu. All rights reserved.
//

#include "mem.hpp"

#include <iostream>
#include <Hypervisor/hv.h>
#include "debug.hpp"

uint64_t GuestPhysicalMemoryManager::allocate(void* hostVirt) {
    std::lock_guard<std::mutex> guard(mutex);
    size_t startClock = clock;
    do {
        if (!freeMap[clock]) {
            freeMap[clock] = true;
            uint64_t ret = baseAddr + 4096 * clock;
            //std::cout << "Mapping 0x" << std::hex << hostVirt
            //    << " To 0x" << std::hex << ret << std::endl;
            hvCheck(hv_vm_map(hostVirt, ret, 4096,
                HV_MEMORY_EXEC | HV_MEMORY_READ | HV_MEMORY_WRITE));
            guestToHostMapping[ret] = hostVirt;
            return ret;
        }
        clock = (clock + 1) % freeMap.size();
    } while (clock != startClock);
    throw "out of memory";
}

void GuestPhysicalMemoryManager::free(uint64_t guestPhysAddr) {
    std::lock_guard<std::mutex> guard(mutex);
    
    // make sure the page is mapped
    assert(guestToHostMapping.count(guestPhysAddr) > 0);
    size_t index = (guestPhysAddr - baseAddr) / 4096;
    assert(freeMap[index]);
    
    freeMap[index] = false;
    guestToHostMapping.erase(guestPhysAddr);
    hvCheck(hv_vm_unmap(guestPhysAddr, 4096));
}

void* GuestPhysicalMemoryManager::getHost(uint64_t guestPhys) {
    guestPhys &= ~(uint64_t)0xFFF;
    if (guestToHostMapping.count(guestPhys) == 0) {
        throw "Non existent guest physical address";
    }
    return guestToHostMapping[guestPhys];
}

GuestVirtualMemoryManager::GuestVirtualMemoryManager
    (GuestPhysicalMemoryManager* physicalManager) : physicalManager(physicalManager)
{
    pml4eGuest = physicalManager->allocate();
    pml4eHost = (pageTableEntry*)physicalManager->getHost(pml4eGuest);
    memset(pml4eHost, 0, 4096);
}

inline pageTableEntry getDefaultEntry() {
    pageTableEntry new_pe;
    memset(&new_pe, 0, sizeof(new_pe));
    new_pe.address = 0;
    new_pe.present = 1;
    new_pe.global = 0;
    new_pe.hugePage = 0;
    new_pe.user = 1;
    new_pe.writable = 1;
    return new_pe;
}

void GuestVirtualMemoryManager::mapGuestPhysicalToGuestVirtual
    (uint64_t paddr, uint64_t vaddr, bool user)
{
    std::lock_guard<std::mutex> guard(mutex);
    
    uint64_t PLM4_index = (vaddr >> 39) & 0b111111111;
    uint64_t PDP_index = (vaddr >> 30) & 0b111111111;
    uint64_t PD_index = (vaddr >> 21) & 0b111111111;
    uint64_t PT_index = (vaddr >> 12) & 0b111111111;

    pageTableEntry* PDP = nullptr;
    if (!pml4eHost[PLM4_index].present) {
        if (!paddr) return;
        uint64_t PDPGuest = physicalManager->allocate();
        //std::cout << "New PDP: 0x" << std::hex << PDPGuest << std::endl;
        PDP = (pageTableEntry*)physicalManager->getHost(PDPGuest);
        memset(PDP, 0, 4096);
        pml4eHost[PLM4_index] = getDefaultEntry();
        pml4eHost[PLM4_index].address = PDPGuest / 4096;
    } else {
        PDP = (pageTableEntry*)physicalManager->
            getHost(pml4eHost[PLM4_index].address * 4096);
    }
    
    pageTableEntry* PD = nullptr;
    if (!PDP[PDP_index].present) {
        if (!paddr) return;
        uint64_t PDGuest = physicalManager->allocate();
        //std::cout << "New PD: 0x" << std::hex << PDGuest << std::endl;
        PD = (pageTableEntry*)physicalManager->getHost(PDGuest);
        memset(PD, 0, 4096);
        PDP[PDP_index] = getDefaultEntry();
        PDP[PDP_index].address = PDGuest / 4096;
    } else {
        PD = (pageTableEntry*)physicalManager->
            getHost(PDP[PDP_index].address * 4096);
    }
    
    pageTableEntry* PT = nullptr;
    if (!PD[PD_index].present) {
        if (!paddr) return;
        uint64_t PTGuest = physicalManager->allocate();
        //std::cout << "New PT: 0x" << std::hex << PTGuest << std::endl;
        PT = (pageTableEntry*)physicalManager->getHost(PTGuest);
        memset(PT, 0, 4096);
        PD[PD_index] = getDefaultEntry();
        PD[PD_index].address = PTGuest / 4096;
    } else {
        PT = (pageTableEntry*)physicalManager->
            getHost(PD[PD_index].address * 4096);
    }
    
    if (PT[PT_index].present) {
        if (!paddr) {
            PT[PT_index].present = false;
        } else {
            std::cout << "Virtual Address 0x"
                      << std::hex << vaddr << " has already been mapped";
            throw "duplicated mapping";
        }
    } else {
        if (!paddr) return;
        PT[PT_index] = getDefaultEntry();
        PT[PT_index].address = paddr / 4096;
        PT[PT_index].user = user;
    }
}

void* GuestVirtualMemoryManager::allocateGuestVirtual
    (uint64_t guest, size_t len, bool user)
{
    if ((guest & 0xFFF) != 0) throw "non-aligned guest virtual addr";
    
    if (len == 0) {
        std::cout << "Warning: zero size allocation!" << std::endl;
        return nullptr;
    }

    void* ret = nullptr;
    for(size_t n = 0; n < len; n += 4096) {
        uint64_t guestPhys = physicalManager->allocate();
        if (!ret) ret = physicalManager->getHost(guestPhys);
        mapGuestPhysicalToGuestVirtual(guestPhys, guest + n, user);
    }
    
    return ret;
}


