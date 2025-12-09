/*
	============================================================================
	Name        : yuvRead.c
	Author      : kdesnos & mpelcat, adapted for C6678
	Version     : 1.0
	Copyright   : CECILL-C
	Description : YUV data reading for C6678 using embedded data arrays

	NOTE: This version uses pre-allocated arrays that are loaded via CCS
	      debugger using "Tools -> Load Memory" with the .dat files.
	============================================================================
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "yuvRead.h"
#include "clock.h"

/*========================================================================
   Embedded YUV Data Arrays

   These arrays will be populated via CCS "Load Memory" feature:
   1. Build and load the program
   2. Pause execution at the beginning of main()
   3. Use Tools -> Load Memory to load:
      - BBB_3D_L.dat into yuvDataLeft  (address printed at startup)
      - BBB_3D_R.dat into yuvDataRight (address printed at startup)
   4. Resume execution

   Array size calculation for NB_FRAME frames at 480x270 YUV 4:2:0:
   Per frame: 480*270 + 480*270/4 + 480*270/4 = 129600 + 32400 + 32400 = 194400 bytes
   ======================================================================*/

/* Frame size in bytes (YUV 4:2:0) */
#define FRAME_SIZE_Y    (WIDTH * HEIGHT)
#define FRAME_SIZE_UV   (WIDTH * HEIGHT / 4)
#define FRAME_SIZE      (FRAME_SIZE_Y + 2 * FRAME_SIZE_UV)

/* Total data size for NB_FRAME frames */
#define TOTAL_YUV_SIZE  (FRAME_SIZE * NB_FRAME)

/*
 * Embedded YUV data arrays - placed in external memory (DDR3)
 * These are loaded via CCS debugger before running
 */
#pragma DATA_SECTION(yuvDataLeft, ".far:dataBufsLeft")
#pragma DATA_ALIGN(yuvDataLeft, 8)
unsigned char yuvDataLeft[TOTAL_YUV_SIZE];

#pragma DATA_SECTION(yuvDataRight, ".far:dataBufsRight")
#pragma DATA_ALIGN(yuvDataRight, 8)
unsigned char yuvDataRight[TOTAL_YUV_SIZE];

/* Frame counters */
static int frame[2] = { 0, 0 };
static int initialized[2] = { 0, 0 };

/*========================================================================
   initReadYUV DEFINITION
   ======================================================================*/
void initReadYUV(int id, int xSize, int ySize) {
    if (id < 0 || id > 1) {
        printf("ERROR: Invalid YUV id: %d\n", id);
        return;
    }

    /* Print address for CCS Load Memory */
    if (id == 0) {
        printf("\n=== LOAD DATA INSTRUCTIONS ===\n");
        printf("Use CCS: Tools -> Load Memory\n");
        printf("Load BBB_3D_L.dat to address: 0x%08X\n", (unsigned int)yuvDataLeft);
        printf("Load BBB_3D_R.dat to address: 0x%08X\n", (unsigned int)yuvDataRight);
        printf("Data size per file: %d bytes (%d frames)\n", TOTAL_YUV_SIZE, NB_FRAME);
        printf("==============================\n\n");
    }

    /* Reset frame counter */
    frame[id] = 0;
    initialized[id] = 1;

    /* Initialize timing for first file only */
    if (id == 0) {
        startTiming(0);
    }

    printf("YUV source %d ready: %d frames at %dx%d\n", id, NB_FRAME, xSize, ySize);
}

/*========================================================================
   readYUV DEFINITION
   ======================================================================*/
void readYUV(int id, int xSize, int ySize, unsigned char *y, unsigned char *u, unsigned char *v) {
    unsigned char* framePtr;

    if (id < 0 || id > 1 || !initialized[id]) {
        printf("ERROR: YUV source %d not initialized\n", id);
        return;
    }

    /* Loop back to beginning if we've read all frames */
    if (frame[id] >= NB_FRAME) {
        frame[id] = 0;
    }

    /* Calculate pointer to current frame in the embedded data */
    framePtr = (id == 0) ? yuvDataLeft : yuvDataRight;
    framePtr += frame[id] * FRAME_SIZE;

    /* Copy Y component (full resolution) */
    memcpy(y, framePtr, FRAME_SIZE_Y);
    framePtr += FRAME_SIZE_Y;

    /* Copy U component (quarter resolution) */
    memcpy(u, framePtr, FRAME_SIZE_UV);
    framePtr += FRAME_SIZE_UV;

    /* Copy V component (quarter resolution) */
    memcpy(v, framePtr, FRAME_SIZE_UV);

    frame[id]++;

    /* FPS measurement (on right image only to avoid double counting) */
    if (id == 1 && (frame[id] % FPS) == 0 && frame[id] > 0) {
        static int firstMeasurement = 1;
        if (!firstMeasurement) {
            unsigned int time = stopTiming(0);
            printf("\nPerformance: %d frames in %u us - %.2f fps\n",
                   FPS, time, ((float)FPS) / (float)time * 1000000.0f);
        }
        firstMeasurement = 0;
        startTiming(0);
    }
}
