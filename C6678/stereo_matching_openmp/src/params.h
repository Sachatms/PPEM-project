/*
	============================================================================
	Name        : params.h
	Author      : kdesnos (original), adapted for C6678
	Version     : 1.0
	Copyright   : CECILL-C, IETR, INSA Rennes
	Description : Parameters for C6678 stereo matching application
	============================================================================
*/

#ifndef PARAM_H
#define PARAM_H

/* Image dimensions */
#define HEIGHT 270
#define WIDTH 480

/* Disparity range */
#define MIN_DISPARITY -8
#define MAX_DISPARITY 8

/* Algorithm parameters */
#define NB_ITERATIONS 3

/* Number of frames to process (reduced for C6678 testing) */
#define NB_FRAME 10

/* FPS measurement interval - should be a divisor of NB_FRAME */
#define FPS 10

typedef unsigned char uchar;

/*
 * File paths for C6678
 *
 * IMPORTANT: On C6678, you have two options to load YUV data:
 *
 * Option 1: Use CCS "Load Data" feature (recommended for testing)
 *   - Comment out the PATH definitions below
 *   - Load data directly into memory arrays via CCS debugger
 *
 * Option 2: Use file I/O via CCS console (current setup)
 *   - The files should be in the CCS project's Debug folder
 *   - Or use absolute paths on the host PC (CCS I/O proxy)
 */
#define PATH_LEFT  "../dat/BBB_3D_L.yuv"
#define PATH_RIGHT "../dat/BBB_3D_R.yuv"

#endif
