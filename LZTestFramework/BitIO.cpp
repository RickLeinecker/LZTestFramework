#include "stdafx.h"
#include "BitIO.h"

CBitIO::CBitIO(FILE *fp)
{
	accumulator = 0;
	currentBit = 0x80;// 1;
	_fp = fp;
	bytesWritten = 0;
	_mode = FORREAD;
}

CBitIO::CBitIO(char *filePath, char *mode)
{
	accumulator = 0;
	currentBit = 0x80;// 1;
	_fp = fopen(filePath, mode);
	bytesWritten = 0;
	_mode = 0;
	if (_stricmp(mode, "rb") == 0)
	{
		_mode = FORREAD;
		currentBit = 0;
	}
	else if (_stricmp(mode, "wb") == 0)
	{
		_mode = FORWRITE;
	}
}

CBitIO::~CBitIO()
{
	if (_fp != NULL)
	{
		close();
	}
}

void CBitIO::close(void)
{
	flush();
	fclose(_fp);
	_fp = NULL;
}

unsigned char CBitIO::read(void)
{
	unsigned char ret = 0;

	if (currentBit == 0)
	{
		currentBit = 0x80;
		fread(&accumulator, sizeof(unsigned char), 1, _fp);
	}

	if ((accumulator & currentBit) != 0)
	{
		ret = 1;
	}

	currentBit >>= 1;

	return ret;
}

unsigned short CBitIO::readBits(int bits)
{
	unsigned short ret = 0;
	unsigned short bit = 1;// 1 << (bits - 1);// 0x80;

	for (int i = 0; i < bits; i++)
	{
		if (read() != 0)
		{
			ret |= bit;
		}
		bit <<= 1;// >>= 1;
	}

	return ret;
}

void CBitIO::write(unsigned char bit)
{
	if (bit)
	{
		accumulator |= currentBit;
	}
	//	currentBit <<= 1;
	currentBit >>= 1;
	if (currentBit == 0)
	{
		fwrite(&accumulator, sizeof(unsigned char), 1, _fp);
		bytesWritten++;
		accumulator = 0;
		currentBit = 0x80;// 1;
	}
}

void CBitIO::writeBits(unsigned short value, int bits)
{
	for (int i = 0; i < bits; i++)
	{
		write(value & 1);
		value >>= 1;
	}
}

void CBitIO::writeInteger(int value)
{
	fwrite(&value, sizeof(int), 1, _fp);
}

int CBitIO::readInteger(void)
{
	int ret;
	fread(&ret, sizeof(int), 1, _fp);
	return ret;
}

void CBitIO::flush(void)
{

	do
	{
		write(0);
	} 
	while( currentBit != 0x80 );

}

int CBitIO::getBytesWritten(void)
{
	return bytesWritten;
}
