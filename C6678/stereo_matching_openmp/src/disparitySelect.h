/*
	============================================================================
	Name        : disparitySelect.h
	Author      : kdesnos
	Version     : 1.0
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : Iterative selection of the disparity with the lowest cost for
	              each pixel in order to construct the depth map.
	============================================================================
*/

#ifndef DISPARITY_SELECT_H
#define DISPARITY_SELECT_H

/**
 * Select the best disparity for each pixel based on cost comparison.
 *
 * @param height              Height of the image
 * @param width               Width of the image
 * @param scale               Scale factor for disparity values
 * @param minDisparity        Minimum disparity value
 * @param disparity           Current disparity being tested
 * @param aggregatedDisparity Current aggregated costs
 * @param bestCost            Best costs so far (updated in-place)
 * @param result              Best disparity map (updated in-place)
 */
void disparitySelect (int height, int width, int scale,
                      int minDisparity,
                      char *disparity,
					  float *aggregatedDisparity,
                      float *bestCost, unsigned char *result);

#endif
