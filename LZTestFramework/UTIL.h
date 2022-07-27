#ifndef __UTIL_H__
#define __UTIL_H__

void advanceWindowEnd(int lengthToAdd, int windowLength, int *windowEnd, int *windowStart);
int compareBuffers(unsigned char *one, unsigned char *two, int len);
void *load(const char *filePath, int *len);

#endif