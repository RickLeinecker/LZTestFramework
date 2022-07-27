#include "util.h"

#include <stdio.h>
#include <io.h>
#include <malloc.h>

/*
	This method advances the window end index according to what was found.
	It also adjusts the window start index if necessary.
*/
void advanceWindowEnd(int lengthToAdd, int windowLength, int *windowEnd, int *windowStart)
{
	*windowEnd += lengthToAdd;
	*windowStart = *windowEnd - windowLength;
	if (*windowStart < 0)
	{
		*windowStart = 0;
	}
}

int compareBuffers(unsigned char *one, unsigned char *two, int len)
{
	for (int i = 0; i < len; i++)
	{
		if (one[i] != two[i])
		{
			return i + 1;
		}
	}

	return 0;
}

/*
	This function simply allocates memory and loads a file into it.
*/
void *load(const char *filePath, int *len)
{
	/* Assume 0. */
	*len = 0;

	/* Attempt to open the file. */
	FILE *fp;
	errno_t err = fopen_s( &fp, filePath, "rb");

	/* If file did not open return NULL indicating error. */
	if (err != 0)
	{
		return NULL;
	}

	/* Populate *len with the file length. This line will not port to gcc Linux. */
	*len = (int)_filelength(_fileno(fp));

	/* Allocate the membory. */
	void *ret = malloc(*len);
	/* Check to see if memory allocated. */
	if (ret == NULL)
	{
		/* Set len to 0. */
		*len = 0;
		/* Close the file. */
		fclose(fp);
		/* Bail out return NULL to indicate error. */
		return NULL;
	}

	/* Read the data. */
	fread(ret, *len, sizeof(char), fp);
	/* Close the file. */
	fclose(fp);

	/* Return the allocated/populated buffer. */
	return ret;
}
