#include "LZSS.h"
#include "util.h"
#include "BitIO.h"
#include "Find.h"
#include <malloc.h>

CLZSS::CLZSS()
{
	resetValues();
}

void CLZSS::resetValues(void)
{
	windowLength = WINDOWK * 1024;
	lookAheadSize = 17;
	windowStart = windowEnd = 0;
	error = 0;
}

CLZSS::~CLZSS()
{
}

int CLZSS::compress(char *inFile, char *outFile, int *dataSize, int *compressedSize)
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

int CLZSS::compress(unsigned char *inData, int _dataSize, char *outFile)
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
		if (m.length >= 2)
		{
			/*
				Write offset, length, and next character.
			*/
			io.writeBits(0, 1);
			io.writeBits(m.offset, DISTANCEBITS);
			io.writeBits(m.length - 2, LENGTHBITS);

			/*
				Move the sliding window end index and possibly the window start index.
			*/
			advanceWindowEnd(m.length, windowLength, &windowEnd, &windowStart);
		}

		/*
			Output for a single byte.
		*/
		else
		{
			if (m.length == 1)
			{
				m.nextChar = inData[windowStart + m.offset];
			}
			io.writeBits(1, 1);
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

unsigned char *CLZSS::decompressToMemory(char *inFile, int *len)
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
		int advanceAmount;

		if (io.readBits(1) == 0)
		{
			unsigned short distance = io.readBits(DISTANCEBITS);
			unsigned short length = io.readBits(LENGTHBITS) + 2;

			for (int i = 0; i < length; i++)
			{
				returnData[windowEnd + i] = returnData[windowStart + i + distance];
			}

			advanceAmount = length;
		}
		else
		{
			unsigned short newByte = io.readBits(8);
			returnData[windowEnd] = (unsigned char)newByte;
			advanceAmount = 1;
		}

		advanceWindowEnd(advanceAmount, windowLength, &windowEnd, &windowStart);
	}

	return returnData;
}

unsigned char *CLZSS::decompressToMemory(FILE *inFile)
{
	resetValues();

	unsigned char *ret = NULL;


	return ret;
}

int CLZSS::decompressToFile(char *inFile, char *outFile)
{
	resetValues();


	return 0;
}

int CLZSS::decompressToFile(FILE *inFile, FILE *outFile)
{
	resetValues();


	return 0;
}
