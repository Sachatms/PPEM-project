/*
	============================================================================
	Name        : disparitySelect.c
	Author      : kdesnos
	Version     : 1.2 - OpenMP parallelized + optimized for C6678
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : Iterative selection of the disparity with the lowest cost for
				  each pixel in order to construct the depth map.
	============================================================================
*/

#include "disparitySelect.h"
#include <string.h>
#include <ti/runtime/openmp/omp.h>

#define min(x,y) (((x)<(y))?(x):(y))

void disparitySelect(int height, int width, int scale,
	int minDisparity,
	char* disparity,
	float * restrict aggregatedDisparity,
	float * restrict bestCost,
	unsigned char * restrict result)
{
	int j;
	char disp = *disparity;
	unsigned char newDispVal = scale * (disp - minDisparity);

	/* OpenMP parallelization: process rows for better cache locality */
	#pragma omp parallel for schedule(static)
	for (j = 0; j < height; j++)
	{
		int i;
		int rowOffset = j * width;
		
		for (i = 0; i < width; i++)
		{
			int idx = rowOffset + i;
			float aggCost = aggregatedDisparity[idx];
			float best = bestCost[idx];
			
			/* If the cost of the aggregated disparity is lower, keep the new
			   disparity as the best, else, keep the current. */
			if (aggCost < best) {
				result[idx] = newDispVal;
				bestCost[idx] = aggCost;
			}
		}
	}
}
