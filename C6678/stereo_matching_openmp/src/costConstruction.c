/*
============================================================================
Name        : costConstruction.c
Author      : kdesnos, JZHAHG
Version     : 1.2 - OpenMP parallelized + optimized for C6678
Copyright   : CeCILL-C, IETR, INSA Rennes
Description : Computation of the costs associated to the pixels of the
              stereo pair for a given disparity
============================================================================
*/

#include "costConstruction.h"
#include <math.h>
#include <ti/runtime/openmp/omp.h>

#define min(x,y) (((x)<(y))?(x):(y))

/* Static inline version for internal use - avoids function call overhead */
static unsigned char hammingCostLocal(unsigned char a, unsigned char b)
{
    unsigned char diffBit = a ^ b;
    /* Fast popcount using bit manipulation */
    unsigned char res = 0;
    while (diffBit) {
        res += diffBit & 1;
        diffBit >>= 1;
    }
    return res;
}

/* External function matching header declaration */
unsigned char hammingCost(unsigned char *a, unsigned char *b)
{
    return hammingCostLocal(*a, *b);
}

void costConstruction (int height, int width, float truncValue,
                       char *disparity,
                       float * restrict grayL, float * restrict grayR,
                       unsigned char * restrict cenL, unsigned char * restrict cenR,
                       float * restrict disparityError)
{
    int j;
    char disp = *disparity;

    /* OpenMP parallelization: process rows for better cache locality
     * Each row is processed by one thread iteration */
    #pragma omp parallel for schedule(static)
    for(j = 0; j < height; j++)
    {
        int i;
        int rowOffset = j * width;

        for(i = 0; i < width; i++)
        {
            int leftPxlIdx = rowOffset + i;
            int rightX = (i - disp) > 0 ? i - disp : 0;
            int rightPxlIdx = rowOffset + rightX;

            /* Inline hamming cost calculation */
            unsigned char censusCost = hammingCostLocal(cenL[leftPxlIdx], cenR[rightPxlIdx]);

            /* Combination method 3 -- weight addition */
            float grayDiff = grayL[leftPxlIdx] - grayR[rightPxlIdx];
            float absDiff = grayDiff < 0 ? -grayDiff : grayDiff;  /* Faster than fabs */
            float truncDiff = absDiff < truncValue ? absDiff : truncValue;

            disparityError[leftPxlIdx] = truncDiff + censusCost * 0.2f;  /* Multiply faster than divide */
        }
    }
}
