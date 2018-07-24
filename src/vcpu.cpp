#include "vcpu.hpp"

static bool hostCapabilitiesChecked = false;

/** we need a helper function to check
 for the control fields that the cpu supports */
static void checkHostCapabilities();

static inline uint64_t capToControl(uint64_t cap, uint64_t ctrl);

VCPU::VCPU() {
    if (!hostCapabilitiesChecked) {
        checkHostCapabilities();
    }
    hvCheck(hv_vcpu_create(&id, HV_VCPU_DEFAULT));
    
    
}

static uint64_t vmx_cap_pinbased, vmx_cap_procbased, vmx_cap_procbased2, vmx_cap_entry;

static void checkHostCapabilities() {
    hvCheck(hv_vmx_read_capability(HV_VMX_CAP_PINBASED,
        &vmx_cap_pinbased));
    hvCheck(hv_vmx_read_capability(HV_VMX_CAP_PROCBASED,
        &vmx_cap_procbased));
    hvCheck(hv_vmx_read_capability(HV_VMX_CAP_PROCBASED2,
        &vmx_cap_procbased2));
    hvCheck(hv_vmx_read_capability(HV_VMX_CAP_ENTRY, &vmx_cap_entry));
    
    hostCapabilitiesChecked = true;
}

static inline uint64_t capToControl(uint64_t cap, uint64_t ctrl) {
    return (ctrl | (cap & 0xffffffff)) & (cap >> 32);
}



