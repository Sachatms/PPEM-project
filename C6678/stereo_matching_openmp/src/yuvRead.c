/*
	============================================================================
	Name        : yuvRead.c
	Author      : kdesnos & mpelcat
	Version     : 1.1
	Copyright   : CECILL-C
	Description : C6x readYUV using embedded data arrays
	============================================================================
*/

#include "yuvRead.h"
#include <xdc/runtime/System.h>
#include <string.h>
#include <xdc/runtime/Timestamp.h>
#include <xdc/runtime/Types.h>

#define PICSIZE (WIDTH * HEIGHT * 3 / 2)

/* Reserving memory for the input sequences (left and right cameras)
 * This memory should be loaded by the Load function of CCS
 * Forcing in DDR3 using .myInputVideoMem section
 */
#pragma DATA_SECTION(input_sequence_left, ".myInputVideoMem");
static unsigned char input_sequence_left[PICSIZE * NB_FRAMES];

#pragma DATA_SECTION(input_sequence_right, ".myInputVideoMem");
static unsigned char input_sequence_right[PICSIZE * NB_FRAMES];

static int currentFrameIndex[2] = {0, 0};

/*========================================================================
   initReadYUV DEFINITION
   ======================================================================*/
void initReadYUV(int id, int xSize, int ySize) {
	if (id < 0 || id > 1) {
		System_printf("ERROR: Invalid YUV id: %d\n", id);
		return;
	}
	currentFrameIndex[id] = 0;
}

/*========================================================================
   readYUV DEFINITION
   ======================================================================*/
void readYUV(int id, int xSize, int ySize, unsigned char *y, unsigned char *u, unsigned char *v) {
	static int frameCounter = 0;
	static unsigned int time = 0;
	unsigned int now;

	unsigned char* input_sequence = (id == 0) ? input_sequence_left : input_sequence_right;
	unsigned char* input_y = input_sequence + currentFrameIndex[id] * PICSIZE;
	unsigned char* input_u = input_y + ySize * xSize;
	unsigned char* input_v = input_u + (ySize * xSize / 4);

	/* FPS measurement every 10 frames (on right camera to avoid double counting) */
	if (id == 1) {
		if (frameCounter == 0) {
			now = Timestamp_get32();
			unsigned int delta = (now - time) / 10;
			float fps = 1000000000.0 / (float)delta;
			System_printf("fps: %f\n", fps);
			time = Timestamp_get32();
		}
		frameCounter = (frameCounter + 1) % 10;
	}

	memcpy(y, input_y, ySize * xSize * sizeof(char));
	memcpy(u, input_u, ySize * xSize * sizeof(char) / 4);
	memcpy(v, input_v, ySize * xSize * sizeof(char) / 4);

	currentFrameIndex[id]++;
	currentFrameIndex[id] = currentFrameIndex[id] % NB_FRAMES;
}
