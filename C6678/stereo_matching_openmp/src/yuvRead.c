/*
	============================================================================
	Name        : yuvRead.c
	Author      : kdesnos & mpelcat, adapted for C6678
	Version     : 1.0
	Copyright   : CECILL-C
	Description : YUV file reading for C6678
	============================================================================
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "yuvRead.h"
#include "clock.h"

/*========================================================================
   Global Variables
   ======================================================================*/
#define NB_PATH 2

static char* path[] = { PATH_LEFT, PATH_RIGHT };
static FILE* ptfile[NB_PATH] = { NULL, NULL };
static int frame[NB_PATH] = { 0, 0 };

/*========================================================================
   initReadYUV DEFINITION
   ======================================================================*/
void initReadYUV(int id, int xSize, int ySize) {
    long fsize;
    long expectedSize;

    if (id < 0 || id >= NB_PATH) {
        printf("ERROR: Invalid YUV file id: %d\n", id);
        return;
    }

    if ((ptfile[id] = fopen(path[id], "rb")) == NULL) {
        printf("ERROR: Cannot open YUV file '%s'\n", path[id]);
        printf("Make sure the dat/ folder with YUV files is in the CCS project directory.\n");
        return;
    }

    printf("Opened file '%s'\n", path[id]);

    /* Obtain file size */
    fseek(ptfile[id], 0, SEEK_END);
    fsize = ftell(ptfile[id]);
    rewind(ptfile[id]);

    /* Check file size (YUV 4:2:0 format: Y + U/4 + V/4 = 1.5 bytes per pixel) */
    expectedSize = (long)NB_FRAME * (xSize * ySize + xSize * ySize / 2);

    if (fsize < expectedSize) {
        printf("WARNING: YUV file size (%ld) smaller than expected (%ld)\n", fsize, expectedSize);
        printf("File may contain fewer than %d frames.\n", NB_FRAME);
    }

    printf("YUV file '%s' ready (%ld bytes)\n", path[id], fsize);

    /* Reset frame counter */
    frame[id] = 0;

    /* Initialize timing for first file only */
    if (id == 0) {
        startTiming(0);
    }
}

/*========================================================================
   readYUV DEFINITION
   ======================================================================*/
void readYUV(int id, int xSize, int ySize, unsigned char *y, unsigned char *u, unsigned char *v) {
    size_t res;

    if (id < 0 || id >= NB_PATH || ptfile[id] == NULL) {
        printf("ERROR: Invalid file handle for id %d\n", id);
        return;
    }

    /* Loop back to beginning if we've read all frames */
    if (frame[id] >= NB_FRAME) {
        rewind(ptfile[id]);
        frame[id] = 0;
    }
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

    /* Read Y component (full resolution) */
    res = fread(y, sizeof(char), xSize * ySize, ptfile[id]);

    /* Read U component (quarter resolution for 4:2:0) */
    res += fread(u, sizeof(char), xSize * ySize / 4, ptfile[id]);

    /* Read V component (quarter resolution for 4:2:0) */
    res += fread(v, sizeof(char), xSize * ySize / 4, ptfile[id]);

    if (res != (size_t)(xSize * ySize + xSize * ySize / 2)) {
        printf("WARNING: Read %zu bytes, expected %d\n", res, xSize * ySize + xSize * ySize / 2);
    }
}
