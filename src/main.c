#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <errno.h>
#include <limits.h>
#include <omp.h>

#include "params.h"
#include "yuvRead.h"
#include "displayRGB.h"
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


int stopThreads = 0;
int CONFIG_THREADS = 0; /* nouvelle variable globale exposée via params.h */

/* Parse a positive integer from string. Returns -1 on error. */
static int parse_positive_int(const char* s) {
	if (!s) return -1;
	char* end = NULL;
	errno = 0;
	long v = strtol(s, &end, 10);
	if (errno != 0 || end == s || *end != '\0' || v <= 0) return -1;
	if (v > INT_MAX) return -1;
	return (int)v;
}



int main(int argc, char** argv) {
	printf("Stereo Matching App\n");

	/* --- OpenMP thread configuration & bench/batch options --- */
	int cli_threads = -1;
	int bench_frames = 0; // legacy bench mode: 0 = no bench
	int batches = 0;      // number of batches (if >0 batch mode enabled)
	int batch_size = 70;  // default batch size
	for (int i = 1; i < argc; ++i) {
		const char* arg = argv[i];
		if (strncmp(arg, "--threads=", 10) == 0) {
			cli_threads = parse_positive_int(arg + 10);
			if (cli_threads < 0) {
				fprintf(stderr, "Invalid value for %s\n", arg);
				cli_threads = -1;
			}
		}
		else if (strcmp(arg, "--threads") == 0) {
			if (i + 1 < argc) {
				cli_threads = parse_positive_int(argv[++i]);
				if (cli_threads < 0) {
					fprintf(stderr, "Invalid value for --threads %s\n", argv[i]);
					cli_threads = -1;
				}
			}
			else {
				fprintf(stderr, "--threads requires a numeric argument\n");
			}
		}
		else if (strncmp(arg, "--bench=", 8) == 0) {
			bench_frames = parse_positive_int(arg + 8);
			if (bench_frames < 0) bench_frames = 0;
		}
		else if (strcmp(arg, "--bench") == 0) {
			if (i + 1 < argc) {
				bench_frames = parse_positive_int(argv[++i]);
				if (bench_frames < 0) bench_frames = 0;
			}
			else {
				fprintf(stderr, "--bench requires a numeric argument\n");
			}
		}
		else if (strncmp(arg, "--batches=", 10) == 0) {
			batches = parse_positive_int(arg + 10);
			if (batches < 0) batches = 0;
		}
		else if (strcmp(arg, "--batches") == 0) {
			if (i + 1 < argc) {
				batches = parse_positive_int(argv[++i]);
				if (batches < 0) batches = 0;
			}
			else {
				fprintf(stderr, "--batches requires a numeric argument\n");
			}
		}
		else if (strncmp(arg, "--batch-size=", 13) == 0) {
			batch_size = parse_positive_int(arg + 13);
			if (batch_size <= 0) batch_size = 70;
		}
		else if (strcmp(arg, "--batch-size") == 0 || strcmp(arg, "--batchsize") == 0) {
			if (i + 1 < argc) {
				batch_size = parse_positive_int(argv[++i]);
				if (batch_size <= 0) batch_size = 70;
			}
			else {
				fprintf(stderr, "--batch-size requires a numeric argument\n");
			}
		}
	}

	int env_threads = -1;
	const char* env = getenv("OMP_NUM_THREADS");
	if (env) {
		env_threads = parse_positive_int(env);
		if (env_threads < 0) {
			fprintf(stderr, "Ignoring invalid OMP_NUM_THREADS=\"%s\"\n", env);
			env_threads = -1;
		}
	}

	int selected_threads;
	if (cli_threads > 0) {
		selected_threads = cli_threads;
	}
	else if (env_threads > 0) {
		selected_threads = env_threads;
	}
	else {
		selected_threads = omp_get_max_threads();
	}

	int nprocs = omp_get_num_procs();
	if (selected_threads > nprocs) {
		fprintf(stderr, "Requested %d threads > available processors %d, capping to %d\n",
			selected_threads, nprocs, nprocs);
		selected_threads = nprocs;
	}
	if (selected_threads <= 0) {
		fprintf(stderr, "Invalid thread count %d, defaulting to 1\n", selected_threads);
		selected_threads = 1;
	}

	omp_set_num_threads(selected_threads);
	CONFIG_THREADS = selected_threads;

	/* Print configuration and OpenMP info */
#ifdef _OPENMP
	printf("OpenMP: _OPENMP=%d\n", _OPENMP);
#else
	printf("OpenMP: _OPENMP macro not defined\n");
#endif
	printf("OpenMP config: requested=%d, omp_get_max_threads()=%d, omp_get_num_procs()=%d\n",
		selected_threads, omp_get_max_threads(), nprocs);
	if (env) printf("Environment OMP_NUM_THREADS=\"%s\"\n", env);

	/* Verification test: run a simple parallel region and count actual threads */
#pragma omp parallel
	{
#pragma omp single
		{
			int runtime_threads = omp_get_num_threads();
			printf("OpenMP verification: parallel region observed %d threads\n", runtime_threads);
			if (runtime_threads != selected_threads) {
				fprintf(stderr, "Warning: requested %d threads but observed %d threads\n",
					selected_threads, runtime_threads);
			}
		}
	}

	/* --- End OpenMP infra --- */

	// Open YUV Files (left & right)
	initReadYUV(0, WIDTH, HEIGHT);
	initReadYUV(1, WIDTH, HEIGHT);

	// Init display
	displayRGBInit(0, HEIGHT, WIDTH);
	displayRGBInit(1, HEIGHT, WIDTH);

	// If batch mode requested, run batches; otherwise fall back to bench or interactive
	if (batches > 0) {
		for (int b = 0; b < batches && !stopThreads; ++b) {
			int frames_processed = 0;
			double batch_t0 = omp_get_wtime();

			while (!stopThreads && frames_processed < batch_size) {

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
				for (unsigned idx = 0; idx < NB_ITERATIONS; idx++) {
					computeWeights(HEIGHT, WIDTH, 0, offsets + idx, rgbL, weightsHor + idx * (3 * HEIGHT * WIDTH));
					computeWeights(HEIGHT, WIDTH, 1, offsets + idx, rgbL, weightsVert + idx * (3 * HEIGHT * WIDTH));
				}

				// Find for each pixel, the disparity level minimizing the aggregated costs.
				static unsigned char depthMap[HEIGHT * WIDTH];
				memset(depthMap, 0, HEIGHT * WIDTH*sizeof(char));
				static float bestCost[HEIGHT * WIDTH];

				// For each degree of disparity
				for (char disp = MIN_DISPARITY; disp <= MAX_DISPARITY; disp++) {

					// Cost construction
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

				// Display
				displayRGB(0, HEIGHT, WIDTH, rgbL);
				displayLum(1, filteredDepthMap);

				// MD5
				MD5_Update(HEIGHT * WIDTH * sizeof(char), filteredDepthMap);
				frames_processed++;
			}

			double batch_t1 = omp_get_wtime();
			double total = batch_t1 - batch_t0;
			printf("Batch %d/%d: frames=%d total_time=%.6f s avg_ms_per_frame=%.3f FPS=%.2f\n",
				b+1, batches, frames_processed, total, (total * 1000.0) / frames_processed, frames_processed / total);
		}
	}
	else {
		// legacy behavior: bench_frames or interactive loop
		int frames_processed = 0;
		double bench_t0 = 0.0;
		if (bench_frames > 0) bench_t0 = omp_get_wtime();

		while (!stopThreads && (bench_frames == 0 || frames_processed < bench_frames)) {

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
			for (unsigned idx = 0; idx < NB_ITERATIONS; idx++) {
				computeWeights(HEIGHT, WIDTH, 0, offsets + idx, rgbL, weightsHor + idx * (3 * HEIGHT * WIDTH));
				computeWeights(HEIGHT, WIDTH, 1, offsets + idx, rgbL, weightsVert + idx * (3 * HEIGHT * WIDTH));
			}

			// Find for each pixel, the disparity level minimizing the aggregated costs.
			static unsigned char depthMap[HEIGHT * WIDTH];
			memset(depthMap, 0, HEIGHT * WIDTH*sizeof(char));
			static float bestCost[HEIGHT * WIDTH];

			// For each degree of disparity
			for (char disp = MIN_DISPARITY; disp <= MAX_DISPARITY; disp++) {

				// Cost construction
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

			// Display
			displayRGB(0, HEIGHT, WIDTH, rgbL);
			displayLum(1, filteredDepthMap);

			// MD5
			MD5_Update(HEIGHT * WIDTH * sizeof(char), filteredDepthMap);
			frames_processed++;
		}
		if (bench_frames > 0) {
			double bench_t1 = omp_get_wtime();
			double total = bench_t1 - bench_t0;
			printf("Benchmark: frames=%d total_time=%.6f s avg_ms_per_frame=%.3f FPS=%.2f\n",
				frames_processed, total, (total * 1000.0) / frames_processed, frames_processed / total);
		}
	}

	return 0;
}