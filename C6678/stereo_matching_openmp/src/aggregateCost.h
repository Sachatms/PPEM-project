/*
	============================================================================
	Name        : aggregateCost.h
	Author      : kdesnos, JZHAHG
	Version     : 1.0
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : Aggregate the horizontal and vertical disparity error for 
                  several offsets.
	============================================================================
*/

#ifndef AGGREGATE_COST_H
#define AGGREGATE_COST_H

/**
 * Perform cost aggregation using weighted neighbors.
 *
 * @param height              Height of the image
 * @param width               Width of the image
 * @param nbIterations        Number of aggregation iterations
 * @param disparityError      Input/temp cost map
 * @param offsets             Offset values for each iteration
 * @param hWeights            Horizontal aggregation weights
 * @param vWeights            Vertical aggregation weights
 * @param aggregatedDisparity Output aggregated cost map
 */
void aggregateCost (int height , int width, int nbIterations,
					float *disparityError, int *offsets,
					float *hWeights, float *vWeights,
					float *aggregatedDisparity);

#endif
