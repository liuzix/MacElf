#ifndef vcpu_hpp
#define vcpu_hpp

#include <Hypervisor/hv.h>
#include <Hypervisor/hv_vmx.h>

#include "debug.hpp"

class VCPU {
private:
    hv_vcpuid_t id;
    
public:
    VCPU();
};

#endif /* vcpu_hpp */
