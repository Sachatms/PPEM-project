/*
============================================================================
Name        : aggregateCost.c
Author      : kdesnos, JZHAHG
Version     : 1.2 - OpenMP parallelized + optimized for C6678
Copyright   : CeCILL-C, IETR, INSA Rennes
Description : Aggregate the horizontal and vertical disparity error for
                  several offsets.
============================================================================
*/

#include "aggregateCost.h"
#include <string.h>
#include <ti/runtime/openmp/omp.h>

#define min(x,y) (((x)<(y))?(x):(y))
#define max(x,y) (((x)<(y))?(y):(x))

void aggregateCost (int height, int width, int nbIterations,
                    float * restrict disparityError, int *offsets,
                    float * restrict hWeights, float * restrict vWeights,
                    float * restrict aggregatedDisparity)
{
    int offsetIdx;
    int j;

    /* For each of the offset, do the horizontal and vertical aggregation */
    for(offsetIdx = 0; offsetIdx < 2*nbIterations; offsetIdx++)
    {
        int offset = offsets[offsetIdx/2];

        /* Even iterations are vertical, Odd are horizontal */
        int hOffset = (offsetIdx % 2 == 1) ? offset : 0;
        int vOffset = (offsetIdx % 2 == 0) ? offset : 0;

        /* Select the weights corresponding to the current offset */
        float * restrict weights = (offsetIdx % 2 == 0) ? vWeights : hWeights;
        int weightIdx = (offsetIdx/2) * (3 * height * width);

        /* Select the computation destination and source */
        float * restrict src = (offsetIdx % 2 == 0) ? disparityError : aggregatedDisparity;
        float * restrict dest = (offsetIdx % 2 == 0) ? aggregatedDisparity : disparityError;

        /* OpenMP parallelization: process rows for better cache locality */
        #pragma omp parallel for schedule(static)
        for(j = 0; j < height; j++)
        {
            int i;
            int rowOffset = j * width;
            int jMinus = max(j - vOffset, 0);
            int jPlus = min(j + vOffset, height - 1);

            for(i = 0; i < width; i++)
            {
                int pixelIdx = rowOffset + i;
                int iMinus = max(i - hOffset, 0);
                int iPlus = min(i + hOffset, width - 1);

                /* Get the costs of the pixels */
                float costO = src[pixelIdx];
                float costM = src[jMinus * width + iMinus];
                float costP = src[jPlus * width + iPlus];

                /* Get the weights - use pre-computed weight index */
                int wIdx = weightIdx + 3 * pixelIdx;
                float weightO = weights[wIdx];
                float weightM = weights[wIdx + 1];
                float weightP = weights[wIdx + 2];

                dest[pixelIdx] = weightO * costO + weightM * costM + weightP * costP;
            }
        }
    }

    /* Copy the result in the output buffer */
    memcpy(aggregatedDisparity, disparityError, height * width * sizeof(float));
}
