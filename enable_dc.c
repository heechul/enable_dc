#include <linux/module.h>
#include <linux/smp.h>
#include <asm/cputype.h>

#define APPLE_CPU_PART_M1_ICESTORM  0x022
#define APPLE_CPU_PART_M1_FIRESTORM 0x023

static void enable_el0_dc(void *info) {
    u64 val, midr;
    u32 part_num;

    // 1. Enable standard ARM64 EL0 cache maintenance (SCTLR_EL1.UCI = bit 26)
    asm volatile("mrs %0, sctlr_el1" : "=r"(val));
    val |= (1ULL << 26);
    asm volatile("msr sctlr_el1, %0\n\tisb" : : "r"(val));

    // 2. Read MIDR_EL1 to safely distinguish Icestorm (E-core) vs Firestorm (P-core)
    asm volatile("mrs %0, midr_el1" : "=r"(midr));
    part_num = (midr >> 4) & 0xFFF;

    if (part_num == APPLE_CPU_PART_M1_FIRESTORM) {
        // Firestorm P-core: clear bit 11 in HID4 (S3_0_C15_C4_0)
        asm volatile("mrs %0, S3_0_C15_C4_0" : "=r"(val));
        val &= ~(1ULL << 11);
        asm volatile("msr S3_0_C15_C4_0, %0\n\tisb" : : "r"(val));
    } else if (part_num == APPLE_CPU_PART_M1_ICESTORM) {
        // Icestorm E-core: clear bit 11 in EHID4 (S3_0_C15_C4_1)
        asm volatile("mrs %0, S3_0_C15_C4_1" : "=r"(val));
        val &= ~(1ULL << 11);
        asm volatile("msr S3_0_C15_C4_1, %0\n\tisb" : : "r"(val));
    }
}

static void disable_el0_dc(void *info) {
    u64 val, midr;
    u32 part_num;

    asm volatile("mrs %0, sctlr_el1" : "=r"(val));
    val &= ~(1ULL << 26);
    asm volatile("msr sctlr_el1, %0\n\tisb" : : "r"(val));

    asm volatile("mrs %0, midr_el1" : "=r"(midr));
    part_num = (midr >> 4) & 0xFFF;

    if (part_num == APPLE_CPU_PART_M1_FIRESTORM) {
        asm volatile("mrs %0, S3_0_C15_C4_0" : "=r"(val));
        val |= (1ULL << 11);
        asm volatile("msr S3_0_C15_C4_0, %0\n\tisb" : : "r"(val));
    } else if (part_num == APPLE_CPU_PART_M1_ICESTORM) {
        asm volatile("mrs %0, S3_0_C15_C4_1" : "=r"(val));
        val |= (1ULL << 11);
        asm volatile("msr S3_0_C15_C4_1, %0\n\tisb" : : "r"(val));
    }
}

static int __init dc_init(void) {
    on_each_cpu(enable_el0_dc, NULL, 1);
    pr_info("enable_dc: EL0 DC CIVAC enabled on all cores\n");
    return 0;
}

static void __exit dc_exit(void) {
    on_each_cpu(disable_el0_dc, NULL, 1);
    pr_info("enable_dc: EL0 DC CIVAC restored to default\n");
}

module_init(dc_init);
module_exit(dc_exit);
MODULE_LICENSE("GPL");
