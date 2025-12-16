/*
	============================================================================
	Name        : readYUV.h
	Author      : kdesnos & mpelcat
	Version     : 1.1
	Copyright   : CECILL-C
	Description : C6x readYUV header
	============================================================================
*/

#ifndef READ_YUV_H
#define READ_YUV_H

#include "params.h"

#define NB_FRAMES 5

/**
* Initialize the readYUV actor.
*
* @param id
*        Source identifier (0=left, 1=right)
* @param xSize
*        The width of the opened YUV file
* @param ySize
*        The height of the opened YUV file
*/
void initReadYUV(int id, int xSize, int ySize);

/**
* Read a new frame from the YUV data.
*
* @param id
*        Source identifier (0=left, 1=right)
* @param xSize
*        The width of the opened YUV file
* @param ySize
*        The height of the opened YUV file
* @param y
*       Destination of the Y component read from the file
* @param u
*       Destination of the U component read from the file
* @param v
*       Destination of the V component read from the file
*/
void readYUV(int id, int xSize, int ySize, unsigned char *y, unsigned char *u, unsigned char *v);

#endif
