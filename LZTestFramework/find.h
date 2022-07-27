#ifndef __FIND_H__
#define __FIND_H__

struct MatchReturn
{
	short offset, length;
	unsigned char nextChar;
};

MatchReturn findMatch(unsigned char *windowData, unsigned char *lookAhead, int lookAheadSize, 
	int dataSize, int windowStart, int windowEnd);

#endif
