/*
	============================================================================
	Name        : offsetGen.h
	Author      : kdesnos
	Version     : 1.0
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : Generation of an array of the offsets used in the 
	              computation of the depth map.
	============================================================================
*/

#ifndef OFFSET_GEN_H
#define OFFSET_GEN_H

/**
 * Generate offset values for cost aggregation iterations.
 *
 * @param nbIterations Number of offsets to generate
 * @param offsets      Output array for offsets
 */
void offsetGen (int nbIterations, int *offsets);

#endif
