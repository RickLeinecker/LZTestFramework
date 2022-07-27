#include "find.h"

/*
	This method finds the longest match between windowData and lookAhead if one exists.
*/
MatchReturn findMatch(unsigned char *windowData, unsigned char *lookAhead, int lookAheadSize, int dataSize, int windowStart, int windowEnd)
{
	/*
		Return value is this data structure with offset, length, and next character.
	*/
	MatchReturn ret;
	ret.offset = 0;
	ret.length = 0;
	ret.nextChar = lookAhead[0];

	/*
		Theoretically we will always look at the entire length of the lookahead buffer.
		But initially the window will not be large enough.
	*/
	int _lookAheadSize = lookAheadSize;
	if (_lookAheadSize > dataSize - windowEnd)
	{
		_lookAheadSize = dataSize - windowEnd;
	}

	/*
		Look through the entire window (so far).
		This might be 0-2500 or 4000-8096 or similar.
	*/
	for (int i = windowStart; i < windowEnd; i++)
	{

		/*
			Default this comparison to length of 0 with an offset pointing to this iteration start in the window.
			windowStart + 0, windowStart + 1, windowStart + 2, etc.
		*/
		int length = 0;
		int offset = i - windowStart;

		/*
			We do a compare (really a string compare) of _lookAheadSize bytes.
		*/
		for (int j = 0; j < _lookAheadSize; j++)
		{
			if (windowData[i + j] != lookAhead[j])
			{
				if (length > ret.length)
				{
					ret.offset = offset;
					ret.length = length;
					ret.nextChar = lookAhead[j];
				}
				break;
			}
			else
			{
				length++;

				if (j == _lookAheadSize - 1 && length > ret.length)
				{
					ret.offset = offset;
					ret.length = length;
					ret.nextChar = lookAhead[j + 1];
				}
			}
		}
	}

	return ret;
}

