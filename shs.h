
/* --------------------------------- SHS.H ------------------------------- */

/*
 * NIST proposed Secure Hash Standard. 
 *
 * Written 2 September 1992, Peter C. Gutmann.
 * This implementation placed in the public domain. 
 *
 * Comments to pgut1@cs.aukuni.ac.nz 
 */

/* Useful defines/typedefs */

#ifndef SHS_H
#define SHS_H

typedef unsigned char BYTE1;
typedef unsigned long LONG1;

/* The SHS block size and message digest sizes, in bytes */

#define SHS_BLOCKSIZE	64
#define SHS_DIGESTSIZE	20

/* The structure for storing SHS info */

typedef struct {
	LONG1 digest [5];	/* Message digest */
	LONG1 countLo, countHi;	/* 64-bit bit count */
	LONG1 data [16];		/* SHS data buffer */
} SHS_INFO;

#define	local	static

void shsInit (SHS_INFO *shsInfo);
void shsUpdate (SHS_INFO *shsInfo, BYTE1 *buffer, int count);
void shsFinal (SHS_INFO *shsInfo);

#endif
