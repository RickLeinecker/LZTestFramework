#include "lz77.h"
#include "lzss.h"
#include <malloc.h>
#include <stdio.h>
#include "Util.h"
#include <Windows.h>

static char inFile[] = "D:\\work\\Data Compression Dev\\TestData\\Data%d.unc";
static char outFile[] = "D:\\work\\Data Compression Dev\\TestData\\Data%d.LZ77";
static char outFile2[] = "D:\\work\\Data Compression Dev\\TestData\\Data%d.LZSS";

static char in[300], out[300];

void testLZ77(void)
{

	for (int i = 0; i <= 36; i++)
	{
		sprintf_s(in, sizeof(in), inFile, i);
		sprintf_s(out, sizeof(out), outFile, i);

		printf("Compressing %s to %s\n", in, out);

		CLZ77 lz77;
		int dataSize = 0, compressedSize = 0;
		int result = lz77.compress(in, out, &dataSize, &compressedSize);

		printf("Size:%d Compressed:%d %d%%\n", dataSize, compressedSize, compressedSize * 100 / dataSize);

		int len = 0;
		unsigned long start = GetTickCount();
		unsigned char *data = lz77.decompressToMemory(out, &len);
		unsigned long ellapsed = GetTickCount() - start;

		// Now do the comparison.
		int len2 = 0;
		unsigned char *comparison = (unsigned char *)load(in, &len2);
		int res = compareBuffers(comparison, data, len);

		free(comparison);
		free(data);

		printf("Decompressed: %d Comparison:%d Ellapsed:%u\n", len, res, ellapsed);

		printf("\n");
	}

}

void testLZSS(void)
{

	for (int i = 0; i <= 36; i++)
	{

		static char inFile[] = "D:\\work\\Data Compression Dev\\TestData\\Data%d.unc";
		static char outFile[] = "D:\\work\\Data Compression Dev\\TestData\\Data%d.lzss";

		sprintf_s(in, sizeof(in), inFile, i);
		sprintf_s(out, sizeof(out), outFile2, i);

		printf("Compressing %s to %s\n", in, out);

		CLZSS lzss;
		int dataSize = 0, compressedSize = 0;
		int result = lzss.compress(in, out, &dataSize, &compressedSize);

		printf("Size:%d Compressed:%d %d%%\n", dataSize, compressedSize, compressedSize * 100 / dataSize);

		int len = 0;
		unsigned long start = GetTickCount();
		unsigned char *data = lzss.decompressToMemory(out, &len);
		unsigned long ellapsed = GetTickCount() - start;

		// Now do the comparison.
		int len2 = 0;
		unsigned char *comparison = (unsigned char *)load(in, &len2);
		int res = compareBuffers(comparison, data, len);

		// VERY TEMP
//		FILE *debug;
//		fopen_s(&debug, out2, "wb");
//		fwrite(data, sizeof(unsigned char), len, debug);
//		fclose(debug);

		free(comparison);
		free(data);

		printf("Decompressed: %d Comparison:%d Ellapsed:%u\n", len, res, ellapsed);

		printf("\n");
	}

}