#pragma once

#include <stdio.h>

#define WINDOWK 4
#define DISTANCEBITS 12
#define LENGTHBITS 4

#define FILEDIDNOTOPEN 1
#define MEMORYDIDNOTALLOCATE 2

class CLZ77
{
public:
	CLZ77();
	~CLZ77();

	int windowLength;
	int windowStart, windowEnd, lookAheadSize;
	int dataSize;
	int error;

	int compress(char *inFile, char *outFile, int *dataSize, int *compressedSize);
	int compress(unsigned char *inFile, int _dataSize, char *outFile);

	unsigned char *decompressToMemory(char *inFile, int *len);
	unsigned char *decompressToMemory(FILE *inFile);
	int decompressToFile(char *inFile, char *outFile);
	int decompressToFile(FILE *inFile, FILE *outFile);

private:
	void resetValues(void);

};

