/*
	============================================================================
	Name        : disparitySelect.c
	Author      : kdesnos
	Version     : 1.1 - OpenMP parallelized for C6678
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : Iterative selection of the disparity with the lowest cost for
				  each pixel in order to construct the depth map.
	============================================================================
*/

#include "disparitySelect.h"
#include <string.h>
#include <ti/runtime/openmp/omp.h>

#define min(x,y) (((x)<(y))?(x):(y))
#define max(x,y) (((x)<(y))?(y):(x))

void disparitySelect(int height, int width, int scale,
	int minDisparity,
	char* disparity,
	float* aggregatedDisparity,
	float* bestCost,
	unsigned char* result)
{
	int idx;
	int totalPixels = height * width;
	char disp = *disparity;

	/* OpenMP parallelization: each pixel is independent
	 * Using schedule(static) for deterministic results */
	#pragma omp parallel for schedule(static)
	for (idx = 0; idx < totalPixels; idx++)
	{
		/* If the cost of the aggregated disparity is lower, keep the new
		   disparity as the best, else, keep the current. */
		result[idx] =
			(aggregatedDisparity[idx] < bestCost[idx]) ?
			scale * (disp - minDisparity) : result[idx];

		bestCost[idx] = min(aggregatedDisparity[idx], bestCost[idx]);
	}
}
