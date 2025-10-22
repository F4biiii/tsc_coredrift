/*
 * timingServices.h
 *
 *  Created on: Sep 19, 2025
 *      Author: root
 */

#ifndef TIMINGSERVICES_H_
#define TIMINGSERVICES_H_

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <err.h>


// Inline assembly to use rdtsc instruction
static __inline__ uint64_t read_tsc(void) {
    unsigned int lo, hi;
    // Serialize with cpuid to ensure accurate measurement
    __asm__ __volatile__ (
        "cpuid\n\t"            // Serialize before rdtsc
        "rdtsc\n\t"            // Read TSC
        : "=a" (lo), "=d" (hi) // Output
        : "a" (0)              // Input: eax = 0
        : "%ebx", "%ecx");     // Clobbered registers
    return ((uint64_t)hi << 32) | lo;
}

static __inline__ uint64_t read_tscp(void) 
{
    unsigned int lo,hi;

    __asm__ __volatile__ (
                    //"mfence\n\t" If software requires RDTSCP to be executed only after all previous stores are globally visible, it can execute MFENCE immediately before RDTSCP
                    "RDTSCP\n\t"
                    "mov %%edx, %0\n\t"
                    "mov %%eax, %1\n\t"
                    "CPUID\n\t": "=r" (hi), "=r" (lo)::
                    "%rax", "%rbx", "%rcx", "%rdx");
    return ((uint64_t)hi << 32) | lo;
}


#endif /* TIMINGSERVICES_H_ */
