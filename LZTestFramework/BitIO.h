#pragma once

#define FORREAD 1
#define FORWRITE 2

class CBitIO
{
public:
	CBitIO(FILE *fp);
	CBitIO(char *filePath, char *mode);
	~CBitIO();

	void close(void);
	void flush(void);
	void write(unsigned char bit);
	void writeBits(unsigned short value, int bits);
	unsigned char read(void);
	unsigned short readBits(int bits);
	int getBytesWritten(void);
	void writeInteger(int value);
	int readInteger(void);

private:
	unsigned char accumulator, currentBit;
	FILE *_fp;
	int bytesWritten;
	int _mode;

};

