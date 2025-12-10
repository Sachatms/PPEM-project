/**
 * rgb2Gray.c - RGB to Grayscale Conversion (DSP Optimized)
 * 
 * Optimizations applied:
 * - Row-based parallelization for better cache locality
 * - Restrict pointers for compiler optimization
 * - Integer multiply/shift instead of float for faster computation
 * - Avoided division by using pre-computed weights
 */

#include <params.h>

#ifdef _TMS320C6X
    #include <ti/runtime/openmp/omp.h>
#else
    #include <omp.h>
#endif

/**
 * Convert RGB image to grayscale using standard luminosity method
 * Y = 0.299*R + 0.587*G + 0.114*B (ITU-R BT.601)
 * 
 * @param rgb   Input RGB image (HEIGHT x WIDTH x 3)
 * @param gray  Output grayscale image (HEIGHT x WIDTH)
 */
void rgb2Gray(const unsigned char * restrict rgb, 
              unsigned char * restrict gray)
{
    /* 
     * Use fixed-point approximation for speed:
     * Y = (77*R + 150*G + 29*B) >> 8
     * 
     * 77/256  = 0.30078  (approx 0.299)
     * 150/256 = 0.58594  (approx 0.587)
     * 29/256  = 0.11328  (approx 0.114)
     * Sum = 256/256 = 1.0
     */
    
    /* Row-based parallelization for better cache locality */
    #pragma omp parallel for schedule(static)
    for (int y = 0; y < HEIGHT; y++) {
        const int rowOffset = y * WIDTH;
        
        for (int x = 0; x < WIDTH; x++) {
            const int idx = rowOffset + x;
            const int rgbIdx = idx * 3;
            
            const unsigned int r = rgb[rgbIdx];      /* Red */
            const unsigned int g = rgb[rgbIdx + 1];  /* Green */
            const unsigned int b = rgb[rgbIdx + 2];  /* Blue */
            
            /* Fixed-point calculation: (77*R + 150*G + 29*B) >> 8 */
            gray[idx] = (unsigned char)((77 * r + 150 * g + 29 * b) >> 8);
        }
    }
}
