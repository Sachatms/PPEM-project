/*
	============================================================================
	Name        : yuvRead.h
	Author      : kdesnos & mpelcat
	Version     : 1.0
	Copyright   : CECILL-C
	Description : YUV file reading interface
	============================================================================
*/

#ifndef READ_YUV_H
#define READ_YUV_H

#include "params.h"

/**
 * Initialize the YUV file reader.
 * Opens the YUV file and verifies its size.
 *
 * @param id    File identifier (0=left, 1=right)
 * @param xSize Width of the video frames
 * @param ySize Height of the video frames
 */
void initReadYUV(int id, int xSize, int ySize);

/**
 * Read a frame from the YUV file.
 * Reads Y, U, V components in 4:2:0 format.
 *
 * @param id    File identifier (0=left, 1=right)
 * @param xSize Width of the frame
 * @param ySize Height of the frame
 * @param y     Output buffer for Y component (xSize * ySize bytes)
 * @param u     Output buffer for U component (xSize * ySize / 4 bytes)
 * @param v     Output buffer for V component (xSize * ySize / 4 bytes)
 */
void readYUV(int id, int xSize, int ySize, unsigned char *y, unsigned char *u, unsigned char *v);

#endif
