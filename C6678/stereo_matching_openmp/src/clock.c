/*
	============================================================================
	Name        : clock.c
	Author      : Adapted for TI C6678
	Version     : 1.0
	Copyright   : CECILL-C
	Description : Timing primitives for C6678 using TSCL/TSCH registers
	============================================================================
*/

#include "clock.h"
#include <c6x.h>  /* For TSCL, TSCH access */

/* Storage for start times (in CPU cycles) */
static unsigned long long startTimes[MAX_STAMPS];

/* C6678 CPU frequency in Hz (1.0 GHz for TMDSEVM6678L) */
#define CPU_FREQ_HZ 1000000000ULL

/*
 * Initialize the Time Stamp Counter (TSC)
 * Must be called once before using timing functions
 */
static int tsc_initialized = 0;

static void initTSC(void) {
    if (!tsc_initialized) {
        /* Enable the Time Stamp Counter by writing to TSCL */
        TSCL = 0;
        tsc_initialized = 1;
    }
}

/*
 * Read the 64-bit Time Stamp Counter
 */
static unsigned long long readTSC(void) {
    unsigned int low, high;

    /* Read low first, then high (order matters for proper 64-bit read) */
    low = TSCL;
    high = TSCH;

    return ((unsigned long long)high << 32) | low;
}

/*
 * Start timing for a given stamp
 */
void startTiming(int stamp) {
    initTSC();

    if (stamp >= 0 && stamp < MAX_STAMPS) {
        startTimes[stamp] = readTSC();
    }
}

/*
 * Stop timing for a given stamp
 * Returns elapsed time in microseconds
 */
unsigned int stopTiming(int stamp) {
    unsigned long long endTime;
    unsigned long long elapsedCycles;
    unsigned int elapsedUs;

    if (stamp < 0 || stamp >= MAX_STAMPS) {
        return 0;
    }

    endTime = readTSC();
    elapsedCycles = endTime - startTimes[stamp];

    /* Convert cycles to microseconds */
    /* elapsedUs = (elapsedCycles * 1000000) / CPU_FREQ_HZ */
    /* Simplified: elapsedUs = elapsedCycles / (CPU_FREQ_HZ / 1000000) = elapsedCycles / 1000 */
    elapsedUs = (unsigned int)(elapsedCycles / (CPU_FREQ_HZ / 1000000ULL));

    return elapsedUs;
}
