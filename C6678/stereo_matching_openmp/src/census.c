/**
 * census.c - Census Transform (DSP Optimized)
 * 
 * Optimizations applied:
 * - Row-based parallelization for better cache locality
 * - Restrict pointers for compiler optimization
 * - Inlined comparison logic
 * - Minimized boundary checks
 */

#include <params.h>

#ifdef _TMS320C6X
    #include <ti/runtime/openmp/omp.h>
#else
    #include <omp.h>
#endif

/**
 * Census transform with 3x3 window - computes 8-bit census signature
 * Each bit represents comparison of center pixel with neighbor
 * 
 * @param gray  Input grayscale image (HEIGHT x WIDTH)
 * @param cen   Output census signature (HEIGHT x WIDTH)
 */
void census(const unsigned char * restrict gray, 
            unsigned char * restrict cen)
{
    /* Clear borders (row 0, row HEIGHT-1, col 0, col WIDTH-1) */
    /* Top and bottom rows */
    #pragma omp parallel for schedule(static)
    for (int x = 0; x < WIDTH; x++) {
        cen[x] = 0;                              /* Row 0 */
        cen[(HEIGHT - 1) * WIDTH + x] = 0;       /* Row HEIGHT-1 */
    }
    
    /* Left and right columns */
    #pragma omp parallel for schedule(static)
    for (int y = 0; y < HEIGHT; y++) {
        cen[y * WIDTH] = 0;                      /* Column 0 */
        cen[y * WIDTH + (WIDTH - 1)] = 0;        /* Column WIDTH-1 */
    }
    
    /* Process interior pixels (rows 1 to HEIGHT-2, cols 1 to WIDTH-2) */
    /* Row-based parallelization for better cache locality */
    #pragma omp parallel for schedule(static)
    for (int y = 1; y < HEIGHT - 1; y++) {
        const int rowOffset = y * WIDTH;
        const int rowAbove = (y - 1) * WIDTH;
        const int rowBelow = (y + 1) * WIDTH;
        
        for (int x = 1; x < WIDTH - 1; x++) {
            const unsigned char centerVal = gray[rowOffset + x];
            unsigned char signature = 0;
            
            /* 3x3 window comparison, center pixel vs 8 neighbors */
            /* Bit layout: 
             * bit7: top-left    bit6: top-center    bit5: top-right
             * bit4: mid-left                        bit3: mid-right
             * bit2: bot-left    bit1: bot-center    bit0: bot-right
             */
            
            /* Top row */
            if (gray[rowAbove + x - 1] >= centerVal) signature |= 0x80; /* bit 7 */
            if (gray[rowAbove + x    ] >= centerVal) signature |= 0x40; /* bit 6 */
            if (gray[rowAbove + x + 1] >= centerVal) signature |= 0x20; /* bit 5 */
            
            /* Middle row (skip center) */
            if (gray[rowOffset + x - 1] >= centerVal) signature |= 0x10; /* bit 4 */
            if (gray[rowOffset + x + 1] >= centerVal) signature |= 0x08; /* bit 3 */
            
            /* Bottom row */
            if (gray[rowBelow + x - 1] >= centerVal) signature |= 0x04; /* bit 2 */
            if (gray[rowBelow + x    ] >= centerVal) signature |= 0x02; /* bit 1 */
            if (gray[rowBelow + x + 1] >= centerVal) signature |= 0x01; /* bit 0 */
            
            cen[rowOffset + x] = signature;
        }
    }
}
