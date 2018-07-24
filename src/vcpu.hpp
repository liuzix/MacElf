#ifndef vcpu_hpp
#define vcpu_hpp

#include <Hypervisor/hv.h>
#include <Hypervisor/hv_vmx.h>

#include "gdt.hpp"
#include "debug.hpp"
#include "mem.hpp"

class VCPU {
private:
    hv_vcpuid_t id;
    GuestVirtualMemoryManager* virtualMem;
public:
    VCPU(GuestVirtualMemoryManager* virtualMem);
    void loadGDT(uint64_t gdtGuestPhys, size_t limit);
    void run(uint64_t rip);
    void setRSP(uint64_t rsp)
    { hvCheck(hv_vcpu_write_register(id, HV_X86_RSP, rsp)); }
};

#endif /* vcpu_hpp */
