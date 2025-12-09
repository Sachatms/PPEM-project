/*
	============================================================================
	Name        : costConstruction.h
	Author      : kdesnos, JZHAHG
	Version     : 1.0
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : Computation of the costs associated to the pixels of the
	              stereo pair for a given disparity
	============================================================================
*/

#ifndef COST_CONSTRUCTION_H
#define COST_CONSTRUCTION_H

/**
 * Compute matching costs for a given disparity level.
 *
 * @param height         Height of the stereo pair
 * @param width          Width of the stereo pair
 * @param truncValue     Maximum cost value (truncation threshold)
 * @param disparity      Disparity offset to test
 * @param grayL          Left grayscale image
 * @param grayR          Right grayscale image
 * @param cenL           Left census image
 * @param cenR           Right census image
 * @param disparityError Output cost map (height * width floats)
 */
void costConstruction (int height, int width, float truncValue,
                       char *disparity,
                       float *grayL, float *grayR,
                       unsigned char *cenL, unsigned char *cenR,
                       float *disparityError);

/**
 * Compute Hamming distance between two 8-bit census signatures.
 *
 * @param a First signature
 * @param b Second signature
 * @return Number of differing bits (0-8)
 */
unsigned char hammingCost(unsigned char *a, unsigned char *b);

#endif
