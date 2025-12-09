/*
	============================================================================
	Name        : medianFilter.h
	Author      : kdesnos
	Version     : 1.0
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : Application of a 3x3 median filter to an image.
	============================================================================
*/

#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H

/**
 * Apply a 3x3 median filter to an image.
 *
 * @param height             Height of the image
 * @param width              Width of the image
 * @param topDownBorderSize  Number of border rows to skip
 * @param rawDisparity       Input image
 * @param filteredDisparity  Output filtered image
 */
void medianFilter (int height , int width, int topDownBorderSize,
                   unsigned char *rawDisparity,
				   unsigned char *filteredDisparity);

/**
 * Quicksort implementation for median computation.
 */
void quickSort(int startIdx, int endIdx, unsigned char *values);

#endif
