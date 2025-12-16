/*
	============================================================================
	Name        : main.c
	Author      : Original x86 version, adapted for TI C6678
	Version     : 1.1
	Description : Stereo matching application for C6678 EVM
	============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "params.h"
#include "yuvRead.h"
#include "yuv2RGB.h"
#include "rgb2Gray.h"
#include "census.h"
#include "costConstruction.h"
#include "offsetGen.h"
#include "computeWeights.h"
#include "aggregateCost.h"
#include "disparitySelect.h"
#include "medianFilter.h"
#include "md5.h"

#include <ti/runtime/openmp/omp.h>

#ifndef NUM_THREADS
#define NUM_THREADS 1
#endif

int main(void) {
	int nthreads;
	int actualThreads = 0;

	/* Configure OpenMP thread count */
	omp_set_num_threads(NUM_THREADS);

	/* Force OpenMP runtime initialization and verify threads */
	#pragma omp parallel
	{
		#pragma omp atomic
		actualThreads++;

		#pragma omp master
		{
			nthreads = omp_get_num_threads();
		}
	}

	printf("==============================================\n");
	printf("Stereo Matching App - C6678 OpenMP Version\n");
	printf("==============================================\n");
	printf("Image size: %d x %d\n", WIDTH, HEIGHT);
	printf("Disparity range: [%d, %d]\n", MIN_DISPARITY, MAX_DISPARITY);
	printf("Number of frames: %d\n", NB_FRAME);
	printf("OpenMP threads: %d (requested: %d, actual: %d)\n", nthreads, NUM_THREADS, actualThreads);
	printf("==============================================\n\n");

	// Open YUV Files (left & right)
	initReadYUV(0, WIDTH, HEIGHT);
	initReadYUV(1, WIDTH, HEIGHT);

	while (1) {

		// Read images
		static unsigned char yL[HEIGHT * WIDTH], uL[HEIGHT * WIDTH / 4], vL[HEIGHT * WIDTH / 4];
		static unsigned char yR[HEIGHT * WIDTH], uR[HEIGHT * WIDTH / 4], vR[HEIGHT * WIDTH / 4];
		readYUV(0, WIDTH, HEIGHT, yL, uL, vL);
		readYUV(1, WIDTH, HEIGHT, yR, uR, vR);

		// Convert images to RGB
		static unsigned char rgbL[HEIGHT * WIDTH * 3], rgbR[HEIGHT * WIDTH * 3];
		yuv2rgb(WIDTH, HEIGHT, yL, uL, vL, rgbL);
		yuv2rgb(WIDTH, HEIGHT, yR, uR, vR, rgbR);

		// Convert to gray
		static float grayL[HEIGHT * WIDTH], grayR[HEIGHT * WIDTH];
		rgb2Gray(HEIGHT * WIDTH, rgbL, grayL);
		rgb2Gray(HEIGHT * WIDTH, rgbR, grayR);

		// Census
		static unsigned char cenL[HEIGHT * WIDTH], cenR[HEIGHT * WIDTH];
		census(HEIGHT, WIDTH, grayL, cenL);
		census(HEIGHT, WIDTH, grayR, cenR);

	// Pre-compute weights for offset aggregation
	int offsets[NB_ITERATIONS];
	static float weightsHor[NB_ITERATIONS * HEIGHT * WIDTH * 3], weightsVert[NB_ITERATIONS * HEIGHT * WIDTH * 3];
	offsetGen(NB_ITERATIONS, offsets);
	unsigned int idx;
	for (idx = 0; idx < NB_ITERATIONS; idx++) {
		computeWeights(HEIGHT, WIDTH, 0, offsets + idx, rgbL, weightsHor + idx * (3 * HEIGHT * WIDTH));
		computeWeights(HEIGHT, WIDTH, 1, offsets + idx, rgbL, weightsVert + idx * (3 * HEIGHT * WIDTH));
	}

	// Find for each pixel, the disparity level minimizing the aggregated costs.
	static unsigned char depthMap[HEIGHT * WIDTH];
	memset(depthMap, 0, HEIGHT * WIDTH*sizeof(char));
	static float bestCost[HEIGHT * WIDTH];

	// For each degree of disparity
	char disp;
	for (disp = MIN_DISPARITY; disp <= MAX_DISPARITY; disp++) {			// Cost construction
			static float dispError[HEIGHT * WIDTH];
			costConstruction(HEIGHT, WIDTH, 12 /*Magic number*/, &disp, grayL, grayR, cenL, cenR, dispError);

			static float aggregatedDisparityCost[HEIGHT * WIDTH];
			aggregateCost(HEIGHT, WIDTH, NB_ITERATIONS, dispError, offsets, weightsHor, weightsVert, aggregatedDisparityCost);

			if (disp == MIN_DISPARITY) {
				memcpy(bestCost, aggregatedDisparityCost, HEIGHT * WIDTH * sizeof(float));
			}
			else {
				// Compare the current disparity cost to previous ones
				disparitySelect(HEIGHT, WIDTH, 12, MIN_DISPARITY, &disp, aggregatedDisparityCost, bestCost, depthMap);
			}
		}

		// Apply median filter on result
		static unsigned char filteredDepthMap[HEIGHT * WIDTH];
		medianFilter(HEIGHT, WIDTH, 1, depthMap, filteredDepthMap);

		// MD5
		MD5_Update(HEIGHT * WIDTH * sizeof(char), filteredDepthMap);
	}

	return 0;
}
