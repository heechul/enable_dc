#include <linux/module.h>
#include <linux/smp.h>

static void enable_el0_dc(void *info) {
    u64 val;

    // 1. Set SCTLR_EL1.UCI (bit 26) to allow EL0 DC CVAU/CIVAC/CVAC/IVAC
    asm volatile("mrs %0, sctlr_el1" : "=r"(val));
    val |= (1ULL << 26);
    asm volatile("msr sctlr_el1, %0\n\tisb" : : "r"(val));

    // 2. Clear bit 11 in Apple HID4 (P-cores) and EHID4 (E-cores)
    // Note: Check MIDR_EL1 if distinguishing E-core (Icestorm) vs P-core (Firestorm)
    asm volatile("mrs %0, S3_0_C15_C4_0" : "=r"(val)); // HID4
    val &= ~(1ULL << 11);
    asm volatile("msr S3_0_C15_C4_0, %0\n\tisb" : : "r"(val));

    asm volatile("mrs %0, S3_0_C15_C4_1" : "=r"(val)); // EHID4
    val &= ~(1ULL << 11);
    asm volatile("msr S3_0_C15_C4_1, %0\n\tisb" : : "r"(val));
}

static int __init dc_init(void) {
    on_each_cpu(enable_el0_dc, NULL, 1);
    pr_info("Enabled EL0 DC cache maintenance instructions\n");
    return 0;
}

static void __exit dc_exit(void) {}

module_init(dc_init);
module_exit(dc_exit);
MODULE_LICENSE("GPL");

