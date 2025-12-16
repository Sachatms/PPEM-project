/*
	============================================================================
	Name        : medianFilter.c
	Author      : kdesnos
	Version     : 1.2 - OpenMP parallelized + optimized for C6678
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : Application of a 3x3 median filter to an image.
	============================================================================
*/

#include "medianFilter.h"
#include <ti/runtime/openmp/omp.h>

#define min(x,y) (((x)<(y))?(x):(y))
#define max(x,y) (((x)<(y))?(y):(x))

/* Optimized median of 9 using sorting network (fewer comparisons) */
static void sort2(unsigned char *a, unsigned char *b) {
	if (*a > *b) {
		unsigned char t = *a;
		*a = *b;
		*b = t;
	}
}

static unsigned char median9(unsigned char *p) {
	/* Sorting network for 9 elements to find median */
	sort2(&p[1], &p[2]); sort2(&p[4], &p[5]); sort2(&p[7], &p[8]);
	sort2(&p[0], &p[1]); sort2(&p[3], &p[4]); sort2(&p[6], &p[7]);
	sort2(&p[1], &p[2]); sort2(&p[4], &p[5]); sort2(&p[7], &p[8]);
	sort2(&p[0], &p[3]); sort2(&p[5], &p[8]); sort2(&p[4], &p[7]);
	sort2(&p[3], &p[6]); sort2(&p[1], &p[4]); sort2(&p[2], &p[5]);
	sort2(&p[4], &p[7]); sort2(&p[4], &p[2]); sort2(&p[6], &p[4]);
	sort2(&p[4], &p[2]);
	return p[4];
}

void medianFilter(int height, int width, int topDownBorderSize,
                  unsigned char * restrict rawDisparity,
                  unsigned char * restrict filteredDisparity)
{
	int j;
	int effectiveHeight = height - topDownBorderSize;

	/* OpenMP parallelization: process rows for better cache locality */
	#pragma omp parallel for schedule(static)
	for(j = topDownBorderSize; j < effectiveHeight; j++)
	{
		int i;
		int outRowOffset = (j - topDownBorderSize) * width;

		for(i = 0; i < width; i++)
		{
			unsigned char pixels[9];
			int k, l, idx = 0;

			/* Get the 9 pixels in the 3x3 window */
			for(l = -1; l <= 1; l++)
			{
				int y = min(max(j + l, 0), height - 1);
				int rowOff = y * width;

				for(k = -1; k <= 1; k++)
				{
					int x = min(max(i + k, 0), width - 1);
					pixels[idx++] = rawDisparity[rowOff + x];
				}
			}

		/* Find median using optimized sorting network */
		filteredDisparity[outRowOffset + i] = median9(pixels);
		}
	}
}

/* quickSort implementation for header compatibility (not used internally) */
void quickSort(int startIdx, int endIdx, unsigned char *values)
{
	int i, j;
	unsigned char pivot, temp;

	if (startIdx >= endIdx) return;

	pivot = values[(startIdx + endIdx) / 2];
	i = startIdx;
	j = endIdx;

	while (i <= j) {
		while (values[i] < pivot) i++;
		while (values[j] > pivot) j--;
		if (i <= j) {
			temp = values[i];
			values[i] = values[j];
			values[j] = temp;
			i++;
			j--;
		}
	}

	if (startIdx < j) quickSort(startIdx, j, values);
	if (i < endIdx) quickSort(i, endIdx, values);
}
