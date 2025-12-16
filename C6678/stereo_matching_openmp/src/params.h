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
#define NB_FRAME 5

/* FPS measurement interval - should be a divisor of NB_FRAME */
#define FPS 5

typedef unsigned char uchar;

#endif
