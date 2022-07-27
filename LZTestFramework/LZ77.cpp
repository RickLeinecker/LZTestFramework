#include "LZ77.h"
#include "util.h"
#include "BitIO.h"
#include "Find.h"
#include <malloc.h>

CLZ77::CLZ77()
{
	resetValues();
}

void CLZ77::resetValues(void)
{
	windowLength = WINDOWK * 1024;
	lookAheadSize = 15;
	windowStart = windowEnd = 0;
	error = 0;
}

CLZ77::~CLZ77()
{
}

int CLZ77::compress(char *inFile, char *outFile, int *dataSize, int *compressedSize )
{
	resetValues();

	int _dataSize;
	unsigned char *inData = (unsigned char *)load(inFile, &_dataSize);
	if (inData == NULL)
	{
		return -1;
	}

	*dataSize = _dataSize;
	*compressedSize = compress(inData, _dataSize, outFile);

	free(inData);

	return *compressedSize;
}

int CLZ77::compress(unsigned char *inData, int _dataSize, char *outFile)
{
	resetValues();

	/*
		This object outputs bits to the destination media.
	*/
	CBitIO io(outFile, "wb");

	/*
		Save a class variable with the size of the data.
	*/
	dataSize = _dataSize;

	/*
		Save the data size.
	*/
	io.writeInteger(dataSize);

	/*
		Keep going until out of data.
	*/
	while (windowEnd < dataSize)
	{
		/*
			Call the method that finds the next match (or single character).
		*/
		MatchReturn m = findMatch(inData, &inData[windowEnd], lookAheadSize, dataSize, windowStart, windowEnd);

		/*
			We need at least one character for this to be economical.
		*/
		if (m.length > 0)
		{
			/*
				Write offset, length, and next character.
			*/
			io.writeBits(m.offset, DISTANCEBITS);
			io.writeBits(m.length, LENGTHBITS);
			io.writeBits(m.nextChar, 8);

			/*
				Move the sliding window end index and possibly the window start index.
			*/
			advanceWindowEnd(m.length + 1, windowLength, &windowEnd, &windowStart);
		}

		/*
			Output for a single byte.
		*/
		else
		{
			/*
				Write 0, 0, and next character.
			*/
			io.writeBits(0, DISTANCEBITS);
			io.writeBits(0, LENGTHBITS);
			io.writeBits(m.nextChar, 8);

			/*
				Move the sliding window end index and possibly the window start index.
			*/
			advanceWindowEnd(1, windowLength, &windowEnd, &windowStart);
		}

	}

	io.close();

	return io.getBytesWritten();
}

unsigned char *CLZ77::decompressToMemory(char *inFile, int *len)
{
	resetValues();

	unsigned char *returnData = NULL;
	CBitIO io(inFile, "rb");

	dataSize = io.readInteger();
	*len = dataSize;
	returnData = new unsigned char[dataSize + 5000];
	if (returnData == NULL)
	{
		error = MEMORYDIDNOTALLOCATE;
		return NULL;
	}

	while (windowEnd < dataSize)
	{
		unsigned short distance = io.readBits(DISTANCEBITS);
		unsigned short length = io.readBits(LENGTHBITS);
		unsigned short newByte = io.readBits(8);

		int advanceAmount;

		if (distance == 0 && length == 0)
		{
			returnData[windowEnd] = (unsigned char)newByte;
			advanceAmount = 1;
		}
		else
		{
			for (int i = 0; i < length; i++)
			{
				returnData[windowEnd + i] = returnData[windowStart + i + distance];
			}

			returnData[windowEnd + length] = (unsigned char)newByte;
			advanceAmount = length + 1;
		}

		advanceWindowEnd(advanceAmount, windowLength, &windowEnd, &windowStart);
	}

	return returnData;
}

unsigned char *CLZ77::decompressToMemory(FILE *inFile)
{
	resetValues();

	unsigned char *ret = NULL;


	return ret;
}

int CLZ77::decompressToFile(char *inFile, char *outFile)
{
	resetValues();


	return 0;
}

int CLZ77::decompressToFile(FILE *inFile, FILE *outFile)
{
	resetValues();


	return 0;
}
