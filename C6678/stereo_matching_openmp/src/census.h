/*
	============================================================================
	Name        : census.h
	Author      : kdesnos, JZHAHG
	Version     : 1.0
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : Computation of the census corresponding to an input gray image
	============================================================================
*/

#ifndef CENSUS_H
#define CENSUS_H

/**
 * Compute the census transform of a grayscale image.
 * Each pixel gets an 8-bit signature based on comparisons with its 8 neighbors.
 *
 * @param height Height of the image
 * @param width  Width of the image
 * @param gray   Input grayscale image (height * width floats)
 * @param cen    Output census image (height * width bytes)
 */
void census(int height, int width, float *gray, unsigned char *cen);

#endif
