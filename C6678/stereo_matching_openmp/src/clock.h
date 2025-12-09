/*
	============================================================================
	Name        : clock.h
	Author      : Adapted for TI C6678
	Version     : 1.0
	Copyright   : CECILL-C
	Description : Timing primitives for C6678
	============================================================================
*/

#ifndef CLOCK_H
#define CLOCK_H

/* Maximum number of timing stamps */
#define MAX_STAMPS 50
#define CLOCK_STAMP_GENERAL 0

/*
 * Start timing for a given stamp
 * @param stamp - Index of the timing stamp (0 to MAX_STAMPS-1)
 */
void startTiming(int stamp);

/*
 * Stop timing for a given stamp
 * @param stamp - Index of the timing stamp
 * @return Elapsed time in microseconds
 */
unsigned int stopTiming(int stamp);

#endif
