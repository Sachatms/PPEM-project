/*
 * MD5 Message-Digest Algorithm header
 */

#ifndef _MD5_H
#define _MD5_H

/* Any 32-bit or wider unsigned integer data type will do */
typedef unsigned int MD5_u32plus;

typedef struct {
	MD5_u32plus lo, hi;
	MD5_u32plus a, b, c, d;
	unsigned char buffer[64];
	MD5_u32plus block[16];
} MD5_CTX;

/**
 * Initialize MD5 context
 */
void MD5_Init(void);

/**
 * Finalize MD5 computation and output hash
 * @param result 16-byte output buffer for MD5 hash
 */
void MD5_Final(unsigned char *result);

/**
 * Compute MD5 hash of a data block and print to console
 * @param size Size of data in bytes
 * @param data Pointer to data
 */
void MD5_Update(unsigned long size, const void *data);

#endif
