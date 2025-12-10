/*
	============================================================================
	Name        : rgb2Gray.c
	Author      : kdesnos
	Version     : 1.2 - OpenMP parallelized + optimized for C6678
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : RGB to Grayscale conversion
	============================================================================
*/

#include "rgb2Gray.h"
#include <ti/runtime/openmp/omp.h>

/**
 * Convert RGB image to grayscale using standard luminosity method
 * Y = 0.299*R + 0.587*G + 0.114*B (ITU-R BT.601)
 * 
 * Signature matches header: rgb2Gray(int size, unsigned char *rgb, float *gray)
 */
void rgb2Gray(int size, unsigned char *rgb, float *gray)
{
    int i;
    
    /* Parallelization over all pixels */
    #pragma omp parallel for schedule(static)
    for (i = 0; i < size; i++) {
        int rgbIdx = i * 3;
        
        unsigned int r = rgb[rgbIdx];      /* Red */
        unsigned int g = rgb[rgbIdx + 1];  /* Green */
        unsigned int b = rgb[rgbIdx + 2];  /* Blue */
        
        /* Standard ITU-R BT.601 coefficients */
        gray[i] = 0.299f * r + 0.587f * g + 0.114f * b;
    }
}