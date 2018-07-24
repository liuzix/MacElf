//
//  arch.h
//  MacElf
//
//  Created by Zixiong Liu on 7/22/18.
//  Copyright © 2018 Zixiong Liu. All rights reserved.
//

#ifndef arch_h
#define arch_h

/*
 * Basic CPU control in CR0
 */
#define CR0_PE    0x00000001 /* Protection Enable */
#define CR0_MP    0x00000002 /* Monitor Coprocessor */
#define CR0_EM    0x00000004 /* Emulation */
#define CR0_TS    0x00000008 /* Task Switched */
#define CR0_ET    0x00000010 /* Extension Type */
#define CR0_NE    0x00000020 /* Numeric Error */
#define CR0_WP    0x00010000 /* Write Protect */
#define CR0_AM    0x00040000 /* Alignment Mask */
#define CR0_NW    0x20000000 /* Not Write-through */
#define CR0_CD    0x40000000 /* Cache Disable */
#define CR0_PG    0x80000000 /* Paging */

#define CR4_PSE         0x00000010      // Page size extension
#define CR4_PAE         0x00000020
#define CR4_OSFXSR      0x00000200
#define CR4_OSXMMEXCPT  0x00000400
#define CR4_OSXSAVE     (1 << 18)
#define CR4_VMXE        0x00002000

#define EFER_SCE        0x00000001      // syscall Extension
#define EFER_LME        0x00000100      // Lond Mode Enable
#define EFER_LMA        0x00000400      // Lond Mode Active

#define MSR_IA32_SYSENTER_CS        0x00000174
#define MSR_IA32_SYSENTER_ESP        0x00000175
#define MSR_IA32_SYSENTER_EIP        0x00000176

#define MSR_STAR        0xc0000081 /* legacy mode SYSCALL target */
#define MSR_LSTAR        0xc0000082 /* long mode SYSCALL target */
#define MSR_CSTAR        0xc0000083 /* compat mode SYSCALL target */
#define MSR_SYSCALL_MASK    0xc0000084 /* EFLAGS mask for syscall */

#define MSR_FS_BASE        0xc0000100 /* 64bit FS base */
#define MSR_GS_BASE        0xc0000101 /* 64bit GS base */
#define MSR_KERNEL_GS_BASE    0xc0000102 /* SwapGS GS shadow */

#define MSR_TIME_STAMP_COUNTER 0x00000010
#define MSR_TSC_AUX            0xc0000103

#define XCR0_SSE_STATE  0x00000002
#define XCR0_AVX_STATE  0x00000004


#endif /* arch_h */
