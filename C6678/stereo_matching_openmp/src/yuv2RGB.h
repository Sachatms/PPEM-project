/*
	============================================================================
	Name        : yuv2RGB.h
	Author      : kdesnos
	Version     : 1.0
	Copyright   : CECILL-C, IETR, INSA Rennes
	Description : Transformation of an YUV image into an RGB image.
	============================================================================
*/

#ifndef YUV_2_RGB_H
#define YUV_2_RGB_H

#include "params.h"

/**
 * Convert a YUV image to RGB format.
 *
 * @param width  Width of the image
 * @param height Height of the image
 * @param y      Y component buffer (width * height bytes)
 * @param u      U component buffer (width * height / 4 bytes)
 * @param v      V component buffer (width * height / 4 bytes)
 * @param rgb    Output RGB buffer (width * height * 3 bytes)
 */
void yuv2rgb(int width, int height, unsigned char *y, unsigned char *u, unsigned char *v, unsigned char *rgb);

#endif
