//
//  mem.hpp
//  MacElf
//
//  Created by Zixiong Liu on 7/19/18.
//  Copyright © 2018 Zixiong Liu. All rights reserved.
//

#ifndef mem_hpp
#define mem_hpp

#include <mutex>
#include <vector>
#include <unordered_map>

class GuestPhysicalMemoryManager {
    std::mutex mutex;
    size_t clock = 0;
    size_t baseAddr = 0;
    std::vector<bool> freeMap;
    std::unordered_map<uint64_t, void*> guestToHostMapping;
public:
    GuestPhysicalMemoryManager(size_t nPages) : freeMap(nPages) {
        freeMap[0] = true;
    };
    uint64_t allocate()
    { return allocate(valloc(4096)); }
    uint64_t allocate(void* hostVirt);
    
    void free(uint64_t guestPhysAddr);
    void* getHost(uint64_t guestPhys);
};

struct pageTableEntry {
    bool present : 1;
    bool writable : 1;
    bool user : 1;
    bool writeThrough : 1;
    bool cacheDisabled : 1;
    bool accessed : 1;
    bool dirty : 1;
    bool hugePage : 1;
    bool global : 1;
    int software : 3;
    uint64_t address : 40;
    int reserved : 12;
} __attribute__((packed));

class GuestVirtualMemoryManager {
    std::mutex mutex;
    pageTableEntry* pml4eHost;
    uint64_t pml4eGuest;
    GuestPhysicalMemoryManager* physicalManager;
    
public:
    GuestVirtualMemoryManager(GuestPhysicalMemoryManager* physicalManager);
    /** set paddr to zero to unmap */
    void mapGuestPhysicalToGuestVirtual(uint64_t paddr, uint64_t vaddr, bool user);
    void* allocateGuestVirtual(uint64_t guest, size_t len, bool user);
    
    uint64_t getGuestCR3() const
    { return pml4eGuest; }
};




#endif /* mem_hpp */
