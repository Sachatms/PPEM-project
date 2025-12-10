/*
	============================================================================
	Name        : census.c
	Author      : kdesnos
	Version     : 1.2 - OpenMP parallelized + optimized for C6678
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : Census Transform for stereo matching
	============================================================================
*/

#include "census.h"
#include <ti/runtime/openmp/omp.h>

/**
 * Census transform with 3x3 window - computes 8-bit census signature
 * Each bit represents comparison of center pixel with neighbor
 * 
 * Signature matches header: census(int height, int width, float *gray, unsigned char *cen)
 */
void census(int height, int width, float *gray, unsigned char *cen)
{
    int x, y;
    
    /* Clear borders (row 0, row HEIGHT-1, col 0, col WIDTH-1) */
    /* Top and bottom rows */
    #pragma omp parallel for schedule(static)
    for (x = 0; x < width; x++) {
        cen[x] = 0;                              /* Row 0 */
        cen[(height - 1) * width + x] = 0;       /* Row HEIGHT-1 */
    }
    
    /* Left and right columns */
    #pragma omp parallel for schedule(static)
    for (y = 0; y < height; y++) {
        cen[y * width] = 0;                      /* Column 0 */
        cen[y * width + (width - 1)] = 0;        /* Column WIDTH-1 */
    }
    
    /* Process interior pixels (rows 1 to height-2, cols 1 to width-2) */
    /* Row-based parallelization for better cache locality */
    #pragma omp parallel for schedule(static)
    for (y = 1; y < height - 1; y++) {
        int xi;
        int rowOffset = y * width;
        int rowAbove = (y - 1) * width;
        int rowBelow = (y + 1) * width;
        
        for (xi = 1; xi < width - 1; xi++) {
            /* Cast float to unsigned char for comparison */
            unsigned char centerVal = (unsigned char)gray[rowOffset + xi];
            unsigned char signature = 0;
            
            /* 3x3 window comparison, center pixel vs 8 neighbors */
            /* Bit layout:
             * bit7: top-left    bit6: top-center    bit5: top-right
             * bit4: mid-left                        bit3: mid-right
             * bit2: bot-left    bit1: bot-center    bit0: bot-right
             */

            /* Top row */
            if ((unsigned char)gray[rowAbove + xi - 1] >= centerVal) signature |= 0x80; /* bit 7 */
            if ((unsigned char)gray[rowAbove + xi    ] >= centerVal) signature |= 0x40; /* bit 6 */
            if ((unsigned char)gray[rowAbove + xi + 1] >= centerVal) signature |= 0x20; /* bit 5 */

            /* Middle row (skip center) */
            if ((unsigned char)gray[rowOffset + xi - 1] >= centerVal) signature |= 0x10; /* bit 4 */
            if ((unsigned char)gray[rowOffset + xi + 1] >= centerVal) signature |= 0x08; /* bit 3 */

            /* Bottom row */
            if ((unsigned char)gray[rowBelow + xi - 1] >= centerVal) signature |= 0x04; /* bit 2 */
            if ((unsigned char)gray[rowBelow + xi    ] >= centerVal) signature |= 0x02; /* bit 1 */
            if ((unsigned char)gray[rowBelow + xi + 1] >= centerVal) signature |= 0x01; /* bit 0 */

            cen[rowOffset + xi] = signature;
        }
    }
}
