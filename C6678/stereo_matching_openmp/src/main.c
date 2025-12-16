/*
	============================================================================
	Name        : main.c
	Author      : Original x86 version, adapted for TI C6678
	Version     : 1.0
	Description : Stereo matching application for C6678 EVM
	============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

/* TI OpenMP Runtime */
#include <ti/runtime/openmp/omp.h>

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
#include "clock.h"

/* Number of OpenMP threads (C6678 has 8 cores)
 * Note: Memory-bound workload may not scale with all 8 cores
 * Test with 1, 2, 4, 8 to find optimal */
#ifndef NUM_THREADS
#define NUM_THREADS 2  /* Try 2 or 4 for memory-bound workloads */
#endif

int main(void) {
	int frameCount = 0;
	unsigned int totalTime = 0;
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

	/* Open YUV Files (left & right) */
	initReadYUV(0, WIDTH, HEIGHT);
	initReadYUV(1, WIDTH, HEIGHT);

	/* Start global timing */
	startTiming(0);

	/* Process NB_FRAME frames */
	while (frameCount < 3 * NB_FRAME) {

		/* Start frame timing */
		startTiming(1);

		/* Read images */
		static unsigned char yL[HEIGHT * WIDTH], uL[HEIGHT * WIDTH / 4], vL[HEIGHT * WIDTH / 4];
		static unsigned char yR[HEIGHT * WIDTH], uR[HEIGHT * WIDTH / 4], vR[HEIGHT * WIDTH / 4];
		readYUV(0, WIDTH, HEIGHT, yL, uL, vL);
		readYUV(1, WIDTH, HEIGHT, yR, uR, vR);

		/* Convert images to RGB */
		static unsigned char rgbL[HEIGHT * WIDTH * 3], rgbR[HEIGHT * WIDTH * 3];
		yuv2rgb(WIDTH, HEIGHT, yL, uL, vL, rgbL);
		yuv2rgb(WIDTH, HEIGHT, yR, uR, vR, rgbR);

		/* Convert to gray */
		static float grayL[HEIGHT * WIDTH], grayR[HEIGHT * WIDTH];
		rgb2Gray(HEIGHT * WIDTH, rgbL, grayL);
		rgb2Gray(HEIGHT * WIDTH, rgbR, grayR);

		/* Census transform */
		static unsigned char cenL[HEIGHT * WIDTH], cenR[HEIGHT * WIDTH];
		census(HEIGHT, WIDTH, grayL, cenL);
		census(HEIGHT, WIDTH, grayR, cenR);

		/* Pre-compute weights for offset aggregation */
		int offsets[NB_ITERATIONS];
		static float weightsHor[NB_ITERATIONS * HEIGHT * WIDTH * 3];
		static float weightsVert[NB_ITERATIONS * HEIGHT * WIDTH * 3];
		offsetGen(NB_ITERATIONS, offsets);

		unsigned int idx;
		for (idx = 0; idx < NB_ITERATIONS; idx++) {
			computeWeights(HEIGHT, WIDTH, 0, offsets + idx, rgbL,
			               weightsHor + idx * (3 * HEIGHT * WIDTH));
			computeWeights(HEIGHT, WIDTH, 1, offsets + idx, rgbL,
			               weightsVert + idx * (3 * HEIGHT * WIDTH));
		}

		/* Find for each pixel, the disparity level minimizing the aggregated costs */
		static unsigned char depthMap[HEIGHT * WIDTH];
		memset(depthMap, 0, HEIGHT * WIDTH * sizeof(char));
		static float bestCost[HEIGHT * WIDTH];

		/* Initialize bestCost to max float for each frame */
		{
			int i;
			for (i = 0; i < HEIGHT * WIDTH; i++) {
				bestCost[i] = FLT_MAX;
			}
		}

		/* For each degree of disparity */
		char disp;
		for (disp = MIN_DISPARITY; disp <= MAX_DISPARITY; disp++) {

			/* Cost construction */
			static float dispError[HEIGHT * WIDTH];
			costConstruction(HEIGHT, WIDTH, 12.0f /*truncValue*/, &disp,
			                 grayL, grayR, cenL, cenR, dispError);

			/* Aggregate costs */
			static float aggregatedDisparityCost[HEIGHT * WIDTH];
			aggregateCost(HEIGHT, WIDTH, NB_ITERATIONS, dispError, offsets,
			              weightsHor, weightsVert, aggregatedDisparityCost);

			/* Compare the current disparity cost to best so far */
			disparitySelect(HEIGHT, WIDTH, 12, MIN_DISPARITY, &disp,
			                aggregatedDisparityCost, bestCost, depthMap);
		}

		/* Apply median filter on result */
		static unsigned char filteredDepthMap[HEIGHT * WIDTH];
		medianFilter(HEIGHT, WIDTH, 1, depthMap, filteredDepthMap);

		/* MD5 hash computation */
		MD5_Update(HEIGHT * WIDTH * sizeof(char), filteredDepthMap);

		/* Frame timing */
		unsigned int frameTime = stopTiming(1);

		frameCount++;
		printf("Frame %3d processed in %u us\n", frameCount % NB_FRAME, frameTime);
	}

	/* Total timing */
	totalTime = stopTiming(0);

	printf("\n==============================================\n");
	printf("Processing complete!\n");
	printf("Total frames: %d\n", frameCount);
	printf("Total time: %u us\n", totalTime);
	printf("Average time per frame: %u us\n", totalTime / frameCount);
	printf("Average FPS: %.2f\n", (float)frameCount / ((float)totalTime / 1000000.0f));
	printf("==============================================\n");

	return 0;
}
