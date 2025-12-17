/*
	============================================================================
	Name        : census.c
	Author      : kdesnos, JZHAHG
	Version     : 1.1 - OpenMP parallelized for C6678
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : Computation of the census corresponding to an input gray image
	============================================================================
*/

#include "census.h"
#include <string.h>
#include <ti/runtime/openmp/omp.h>

void census(int height, int width, float *gray, unsigned char *cen){
    int idx;
    int innerHeight = height - 2;
    int innerWidth = width - 2;
    int totalInnerPixels = innerHeight * innerWidth;

    /* Fill the 1st and last lines with 0 */
    memset(cen, 0, width*sizeof(char));
    memset(cen+(height-1)*width, 0, width*sizeof(char));

    /* Fill first and last column of each row with 0 */
    int j;
    for(j=1; j<height-1; j++){
        cen[j*width] = 0;
        cen[(j+1)*width-1] = 0;
    }

    /* OpenMP parallelization: inner pixels are independent
     * Static schedule with chunk size for better cache locality on DSP
     * Chunk size 384 balances workload across 8 cores */
    #pragma omp parallel for schedule(static, 384)
    for(idx = 0; idx < totalInnerPixels; idx++){
        int i = 1 + (idx % innerWidth);
        int jj = 1 + (idx / innerWidth);
        int k, l;
        unsigned char signature = 0x00;
        int bit = 7;

        /* For each pixel, compute its census signature with
           a 3x3 pixels window around it. */
        for(l = -1; l <= 1; l++){
            for(k=-1 ; k<=1; k++){
                /* In the 8 bit signature, a bit is set
                   to 1 if the compared pixel is inferior to the current. */
                if(k!=0 || l!=0){
                    if(gray[jj*width+i] > gray[(jj+l)*width+(i+k)]){
                        signature |= 1 << bit;
                    }
                    bit--;
                }
            }
        }
        cen[jj*width+i] = signature;
    }
}
