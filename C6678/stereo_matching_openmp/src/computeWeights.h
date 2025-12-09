/*
	============================================================================
	Name        : computeWeights.h
	Author      : kdesnos, JZHAHG
	Version     : 1.0
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : Computation of the weights associated to the pixel of an rgb 
	              image.
	============================================================================
*/

#ifndef COMPUTE_WEIGHTS_H
#define COMPUTE_WEIGHTS_H

/**
 * Compute aggregation weights for each pixel.
 *
 * @param height    Height of the image
 * @param width     Width of the image
 * @param horOrVert 0 for horizontal, 1 for vertical
 * @param offset    Neighbor distance for weight computation
 * @param rgbL      Input RGB image
 * @param weights   Output weights (3 * height * width floats)
 */
void computeWeights (int height , int width, int horOrVert,
					 int *offset, unsigned char *rgbL, float *weights);

#endif
