/*
	============================================================================
	Name        : yuvRead.h
	Author      : kdesnos & mpelcat
	Version     : 1.0
	Copyright   : CECILL-C
	Description : YUV data reading interface for C6678
	              Uses embedded data arrays loaded via CCS debugger
	============================================================================
*/

#ifndef READ_YUV_H
#define READ_YUV_H

#include "params.h"

/*
 * External references to embedded YUV data arrays.
 * These arrays are defined in yuvRead.c and loaded via CCS "Load Memory".
 */
extern unsigned char yuvDataLeft[];
extern unsigned char yuvDataRight[];

/**
 * Initialize the YUV data reader.
 * Prints the memory addresses for CCS Load Memory feature.
 *
 * @param id    Source identifier (0=left, 1=right)
 * @param xSize Width of the video frames
 * @param ySize Height of the video frames
 */
void initReadYUV(int id, int xSize, int ySize);

/**
 * Read a frame from the embedded YUV data.
 * Copies Y, U, V components from the embedded arrays.
 *
 * @param id    Source identifier (0=left, 1=right)
 * @param xSize Width of the frame
 * @param ySize Height of the frame
 * @param y     Output buffer for Y component (xSize * ySize bytes)
 * @param u     Output buffer for U component (xSize * ySize / 4 bytes)
 * @param v     Output buffer for V component (xSize * ySize / 4 bytes)
 */
void readYUV(int id, int xSize, int ySize, unsigned char *y, unsigned char *u, unsigned char *v);

#endif
