#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint64_t target = 0xdeadbeef;
    asm volatile("dc civac, %0\n\tdsb ish\n\tisb" : : "r"(&target) : "memory");
    printf("DC CIVAC succeeded in EL0!\n");
    return 0;
}
