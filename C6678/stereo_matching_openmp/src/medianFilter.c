/*
	============================================================================
	Name        : medianFilter.c
	Author      : kdesnos
	Version     : 1.1 - OpenMP parallelized for C6678
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : Application of a 3x3 median filter to an image.
	============================================================================
*/

#include "medianFilter.h"
#include <ti/runtime/openmp/omp.h>

#define min(x,y) (((x)<(y))?(x):(y))
#define max(x,y) (((x)<(y))?(y):(x))

void swap(unsigned char *a, unsigned char *b){
	unsigned char buf = *a;
	*a=*b;
	*b=buf;
}

void quickSortPartition(int startIdx, int endIdx, int *pivotIdx, unsigned char *values){
	int idx;
	int swapIdx = startIdx;
	swap(values+*pivotIdx,values+endIdx);
	for(idx = startIdx; idx < endIdx; idx++){
		if(values[idx]<=values[endIdx]){
			swap(values+swapIdx,values+idx);
			swapIdx++;
		}
	}
	swap(values+swapIdx, values+endIdx);
	*pivotIdx = swapIdx;
}

void quickSort(int startIdx, int endIdx, unsigned char *values){
	if(startIdx<endIdx){
		int pivotIdx = startIdx;
		quickSortPartition(startIdx, endIdx, &pivotIdx, values);
		quickSort(startIdx,pivotIdx-1,values);
		quickSort(pivotIdx+1,endIdx,values);
	}
}

void medianFilter (int height , int width, int topDownBorderSize,
                   unsigned char *rawDisparity,
				   unsigned char *filteredDisparity)
{
	int idx;
	int effectiveHeight = height - 2 * topDownBorderSize;
	int totalPixels = effectiveHeight * width;

	/* OpenMP parallelization: each output pixel is independent
	 * Using schedule(static) for deterministic results
	 * Note: pixels array is private to each thread */
	#pragma omp parallel for schedule(static)
	for(idx = 0; idx < totalPixels; idx++){
		int i = idx % width;
		int j = topDownBorderSize + (idx / width);
		int k, l;
		unsigned char pixels[9];

		/* Output pixel is the median of a 3x3 window
		   Get the 9 pixels */
		for(l=-1;l<=1;l++){
			int y = min(max(j+l,0),height-1);
			for(k=-1;k<=1;k++){
				int x = min(max(i+k,0),width-1);
				pixels[(l+1)*3+k+1] = rawDisparity[y*width+x];
			}
		}

		/* Sort the 9 values */
		quickSort(0, 8, pixels);
		filteredDisparity[(j-topDownBorderSize)*width+i] = pixels[9/2];
	}
}
