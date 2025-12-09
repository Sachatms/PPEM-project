/*
	============================================================================
	Name        : costConstruction.c
	Author      : kdesnos, JZHAHG
	Version     : 1.1 - OpenMP parallelized for C6678
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : Computation of the costs associated to the pixels of the
	              stereo pair for a given disparity
	============================================================================
*/

#include "costConstruction.h"
#include <math.h>
#include <ti/runtime/openmp/omp.h>

#define min(x,y) (((x)<(y))?(x):(y))

unsigned char hammingCost(unsigned char *a, unsigned char *b)
{
    int i;
    unsigned char res=0;

    /* Bitwise exclusive or to identify the differences
       between the two signatures */
    unsigned char diffBit = *a ^ *b;

    /* Count the 1 in the diffBit word */
    for(i=0; i<8; i++)
    {
        res += ((diffBit & (1<<i))? 1 : 0);
    }

    return res;
}

void costConstruction (int height, int width, float truncValue,
                       char *disparity,
                       float *grayL, float *grayR,
                       unsigned char *cenL, unsigned char *cenR,
                       float *disparityError)
{
    int idx;
    int totalPixels = height * width;
    char disp = *disparity;

    /* OpenMP parallelization: each thread processes a chunk of pixels
     * Using schedule(static) for deterministic results (MD5 validation) */
    #pragma omp parallel for schedule(static)
    for(idx = 0; idx < totalPixels; idx++)
    {
        unsigned char censusCost;
        int i = idx % width;
        int j = idx / width;
        int leftPxlIdx = idx;
        int rightPxlIdx = j * width + (((i - disp) > 0) ? i - disp : 0);

        /* Get the cost from the census signatures */
        censusCost = hammingCost(cenL + leftPxlIdx, cenR + rightPxlIdx);

        /* Combination method 3 -- weight addition */
        disparityError[leftPxlIdx] = min((float)fabs((float)(grayL[leftPxlIdx] - grayR[rightPxlIdx])), truncValue) + censusCost / 5.0f;
    }
}
