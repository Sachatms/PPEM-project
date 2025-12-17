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

/* Use exact coefficients from x86 version for MD5 compatibility */
#define RGB2GRAY_COEF_R 0.29893602129378f
#define RGB2GRAY_COEF_G 0.58704307445112f
#define RGB2GRAY_COEF_B 0.11402090425510f

/**
 * Convert RGB image to grayscale
 * Signature matches header: rgb2Gray(int size, unsigned char *rgb, float *gray)
 */
void rgb2Gray(int size, unsigned char *rgb, float *gray)
{
    int i;

    /* Parallelization over all pixels
     * Chunk size 512 for efficient memory bandwidth utilization */
    #pragma omp parallel for schedule(static, 512)
    for (i = 0; i < size; i++) {
        int rgbIdx = i * 3;

        /* Use exact x86 coefficients for MD5 match */
        gray[i] = RGB2GRAY_COEF_R * (float)rgb[rgbIdx] +
                  RGB2GRAY_COEF_G * (float)rgb[rgbIdx + 1] +
                  RGB2GRAY_COEF_B * (float)rgb[rgbIdx + 2];
    }
}
