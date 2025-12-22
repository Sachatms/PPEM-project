/*
	============================================================================
	Name        : rgb2Gray.h
	Author      : kdesnos
	Version     : 1.0
	Copyright   : CECILL-C, IETR, INSA Rennes
	Description : Transformation of an RGB image into a gray-level image.
	============================================================================
*/

#ifndef RGB_2_GRAY_H
#define RGB_2_GRAY_H

/**
 * Convert an RGB image to grayscale.
 *
 * @param size Total number of pixels
 * @param rgb  Input RGB buffer (size * 3 bytes)
 * @param gray Output grayscale buffer (size floats)
 */
void rgb2Gray(int size, unsigned char *rgb, float *gray);

#endif
