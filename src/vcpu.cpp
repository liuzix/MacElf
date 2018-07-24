#include "vcpu.hpp"

#include <iostream>
#include "arch.h"

static bool hostCapabilitiesChecked = false;

/** we need a helper function to check
 for the control fields that the cpu supports */
static void checkHostCapabilities();

static inline uint64_t capToControl(uint64_t cap, uint64_t ctrl);

static uint64_t vmx_cap_pinbased, vmx_cap_procbased, vmx_cap_procbased2, vmx_cap_entry, vmx_cap_exit;

#define VMCS_CTRL_CPU_BASED_HLT    (1 << 7)
#define VMCS_CTRL_CPU_BASED_SECONDARY  (1 << 31)
#define VMCS_CTRL_CPU_BASED2_EPT (1 << 1)
#define VMCS_CTRL_CPU_BASED2_VPID (1 << 5)
#define VMCS_CTRL_CPU_BASED2_UNRESTRICTED (1 << 7)
#define VMCS_GUEST_IA32_EFER_SYSCALL (1)
#define VMCS_GUEST_IA32_EFER_LONGENABLE (1 << 8)
#define VMCS_GUEST_IA32_EFER_LONGACTIVE (1 << 10)

static inline uint64_t vmcs_read(int vcpuid, uint32_t encoding);

static inline void vmcs_write(int vcpuid, uint32_t encoding, uint64_t val);

VCPU::VCPU(GuestVirtualMemoryManager* virtualMem) : virtualMem(virtualMem) {
    if (!hostCapabilitiesChecked) {
        checkHostCapabilities();
    }
    hvCheck(hv_vcpu_create(&id, HV_VCPU_DEFAULT));
    
    vmcs_write(id, VMCS_CTRL_PIN_BASED,
        capToControl(vmx_cap_pinbased, 0));

    vmcs_write(id, VMCS_CTRL_CPU_BASED,
        capToControl(vmx_cap_procbased, VMCS_CTRL_CPU_BASED_HLT | (1 << 19) | (1 << 20)
        | VMCS_CTRL_CPU_BASED_SECONDARY));
    
    vmcs_write(id, VMCS_CTRL_CPU_BASED2,
        capToControl(vmx_cap_procbased2, VMCS_CTRL_CPU_BASED2_UNRESTRICTED  | VMCS_CTRL_CPU_BASED2_VPID | VMCS_CTRL_CPU_BASED2_EPT));

    vmcs_write(id, VMCS_CTRL_VMENTRY_CONTROLS,
        capToControl(vmx_cap_entry, VMENTRY_GUEST_IA32E | VMENTRY_LOAD_EFER));


    uint64_t exitCtrl = vmcs_read(id, VMCS_CTRL_VMEXIT_CONTROLS);
    vmcs_write(id, VMCS_CTRL_VMEXIT_CONTROLS,
        capToControl(vmx_cap_exit, exitCtrl | VMEXIT_LOAD_EFER));
    
    vmcs_write(id, VMCS_CTRL_EXC_BITMAP, 0xffffffff);
    vmcs_write(id, VMCS_CTRL_PF_ERROR_MASK, 0x0);
    vmcs_write(id, VMCS_CTRL_PF_ERROR_MATCH, 0x0);
    
    vmcs_write(id, VMCS_CTRL_CR0_MASK, 0x0);
    vmcs_write(id, VMCS_CTRL_CR0_SHADOW, 0);
    vmcs_write(id, VMCS_CTRL_CR4_MASK, 0);
    vmcs_write(id, VMCS_CTRL_CR4_SHADOW, 0);
    
    vmcs_write(id, VMCS_GUEST_CR0, CR0_MP | CR0_PE | CR0_NE | CR0_PG);
    vmcs_write(id, VMCS_GUEST_CR3, virtualMem->getGuestCR3());
    uint64_t cr4 = vmcs_read(id, VMCS_GUEST_CR4);
    vmcs_write(id, VMCS_GUEST_CR4,  cr4 | CR4_OSFXSR | CR4_OSXMMEXCPT | CR4_VMXE | CR4_OSXSAVE | CR4_PAE);
    
    uint64_t efer = vmcs_read(id, VMCS_GUEST_IA32_EFER);
    vmcs_write(id, VMCS_GUEST_IA32_EFER,
        efer | EFER_SCE | EFER_LME | EFER_LMA);

    
    vmcs_write(id, VMCS_GUEST_CS, 8);
    vmcs_write(id, VMCS_GUEST_CS_LIMIT, 0x0);
    vmcs_write(id, VMCS_GUEST_CS_AR, 0x209b);
    vmcs_write(id, VMCS_GUEST_CS_BASE, 0);
    
    vmcs_write(id, VMCS_GUEST_DS, 16);
    vmcs_write(id, VMCS_GUEST_DS_LIMIT, 0x0);
    vmcs_write(id, VMCS_GUEST_DS_AR, 0x93);
    vmcs_write(id, VMCS_GUEST_DS_BASE, 0);

    vmcs_write(id, VMCS_GUEST_ES, 16);
    vmcs_write(id, VMCS_GUEST_ES_LIMIT, 0x0);
    vmcs_write(id, VMCS_GUEST_ES_AR, 0x93);
    vmcs_write(id, VMCS_GUEST_ES_BASE, 0);
    
    vmcs_write(id, VMCS_GUEST_FS, 16);
    vmcs_write(id, VMCS_GUEST_FS_LIMIT, 0x0);
    vmcs_write(id, VMCS_GUEST_FS_AR, 0x93);
    vmcs_write(id, VMCS_GUEST_FS_BASE, 0);
    
    vmcs_write(id, VMCS_GUEST_GS, 16);
    vmcs_write(id, VMCS_GUEST_GS_LIMIT, 0x0);
    vmcs_write(id, VMCS_GUEST_GS_AR, 0x93);
    vmcs_write(id, VMCS_GUEST_GS_BASE, 0);
    
    vmcs_write(id, VMCS_GUEST_SS, 16);
    vmcs_write(id, VMCS_GUEST_SS_LIMIT, 0x0);
    vmcs_write(id, VMCS_GUEST_SS_AR, 0x93);
    vmcs_write(id, VMCS_GUEST_SS_BASE, 0);
    
    vmcs_write(id, VMCS_GUEST_LDTR, 0);
    vmcs_write(id, VMCS_GUEST_LDTR_BASE, 0);
    vmcs_write(id, VMCS_GUEST_LDTR_LIMIT, 0);
    vmcs_write(id, VMCS_GUEST_LDTR_AR, 0x10000);
    
    vmcs_write(id, VMCS_GUEST_TR, 0);
    vmcs_write(id, VMCS_GUEST_TR_BASE, 0);
    vmcs_write(id, VMCS_GUEST_TR_LIMIT, 0);
    vmcs_write(id, VMCS_GUEST_TR_AR, 0x8B);
    
    vmcs_write(id, VMCS_GUEST_IDTR_BASE, 0);
    vmcs_write(id, VMCS_GUEST_IDTR_LIMIT, 0xffff);
    
    hvCheck(hv_vcpu_write_register(id, HV_X86_CS, 8));
    hvCheck(hv_vcpu_write_register(id, HV_X86_DS, 16));
    hvCheck(hv_vcpu_write_register(id, HV_X86_ES, 16));
    hvCheck(hv_vcpu_write_register(id, HV_X86_FS, 16));
    hvCheck(hv_vcpu_write_register(id, HV_X86_GS, 16));
    hvCheck(hv_vcpu_write_register(id, HV_X86_SS, 16));

    
    hv_vmx_vcpu_write_vmcs(id, VMCS_GUEST_RFLAGS, 0x2);
    
    uint64_t dbgCtl = vmcs_read(id, VMCS_GUEST_IA32_DEBUGCTL);
    vmcs_write(id, VMCS_GUEST_IA32_DEBUGCTL, dbgCtl & 0b1101111111000011);
    
    hv_vcpu_enable_native_msr(id, MSR_TIME_STAMP_COUNTER, 1);
    hv_vcpu_enable_native_msr(id, MSR_TSC_AUX, 1);
    hv_vcpu_enable_native_msr(id, MSR_KERNEL_GS_BASE, 1);
    hv_vcpu_enable_native_msr(id, MSR_GS_BASE, 1);
    hv_vcpu_enable_native_msr(id, MSR_FS_BASE, 1);
    hv_vcpu_enable_native_msr(id, MSR_STAR, 1);
    hv_vcpu_enable_native_msr(id, MSR_LSTAR, 1);
    hv_vcpu_enable_native_msr(id, MSR_CSTAR, 1);
    hv_vcpu_enable_native_msr(id, MSR_SYSCALL_MASK, 1);
}

void VCPU::loadGDT(uint64_t gdtGuestPhys, size_t limit) {
    vmcs_write(id, VMCS_GUEST_GDTR_BASE, gdtGuestPhys);
    vmcs_write(id, VMCS_GUEST_GDTR_LIMIT, limit - 1);
}

void VCPU::run(uint64_t rip) {
    vmcs_write(id, VMCS_GUEST_RIP, rip);
    hvCheck(hv_vcpu_write_register(id, HV_X86_RIP, rip));
    hvCheck(hv_vcpu_write_register(id, HV_X86_RFLAGS, 0x2));
    for (;;) {
        hvCheck(hv_vcpu_run(id));
        uint64_t exit_reason = vmcs_read(id, VMCS_RO_EXIT_REASON);
        std::cout << "VM exit!" << std::endl;
        std::cout << "Exit reason: 0x" << std::hex << exit_reason << std::endl;
        
        uint64_t exit_qualification = vmcs_read(id, VMCS_RO_EXIT_QUALIFIC);
        std::cout << "Exit qualification: 0x" << std::hex << exit_qualification << std::endl;
        
        uint64_t guestLin = vmcs_read(id, VMCS_RO_GUEST_LIN_ADDR);
        std::cout << "Guest Linear address: 0x" << std::hex << guestLin << std::endl;
        
        uint64_t guestPhys = vmcs_read(id, VMCS_GUEST_PHYSICAL_ADDRESS);
        std::cout << "Guest Physical address: 0x" << std::hex << guestPhys << std::endl;
        
        uint64_t curRip;
        hvCheck(hv_vcpu_read_register(id, HV_X86_RIP, &curRip));
        
        std::cout << "Guest RIP=0x" << std::hex << curRip << std::endl;
        
        switch (exit_reason) {
            case 0x0: {
                uint64_t vector = vmcs_read(id, VMCS_RO_VMEXIT_IRQ_INFO);
                std::cout << "Guest exception vector: " << vector << std::endl;
                
                uint64_t error = vmcs_read(id, VMCS_RO_VMEXIT_IRQ_ERROR);
                std::cout << "Guest exception error: " << error << std::endl;
            }
        }
    }
}

static void checkHostCapabilities() {
    hvCheck(hv_vmx_read_capability(HV_VMX_CAP_PINBASED,
        &vmx_cap_pinbased));
    hvCheck(hv_vmx_read_capability(HV_VMX_CAP_PROCBASED,
        &vmx_cap_procbased));
    hvCheck(hv_vmx_read_capability(HV_VMX_CAP_PROCBASED2,
        &vmx_cap_procbased2));
    hvCheck(hv_vmx_read_capability(HV_VMX_CAP_ENTRY, &vmx_cap_entry));
    hvCheck(hv_vmx_read_capability(HV_VMX_CAP_EXIT, &vmx_cap_exit));
    hostCapabilitiesChecked = true;
}

static inline uint64_t capToControl(uint64_t cap, uint64_t ctrl) {
    return (ctrl | (cap & 0xffffffff)) & (cap >> (uint64_t)32);
}

static inline uint64_t
vmcs_read(int vcpuid, uint32_t encoding) {
    uint64_t val = 0;
    hvCheck(hv_vmx_vcpu_read_vmcs(((hv_vcpuid_t) vcpuid), encoding, &val));
    return (val);
}

static inline void
vmcs_write(int vcpuid, uint32_t encoding, uint64_t val) {
    if (encoding == 0x00004002) {
        if (val == 0x0000000000000004) {
            abort();
        }
    }
    hvCheck(hv_vmx_vcpu_write_vmcs((hv_vcpuid_t)vcpuid, encoding, val));
}



