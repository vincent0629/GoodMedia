#ifndef _DATASOURCE_H_
#define _DATASOURCE_H_

#include <stdio.h>

class CDataSource
{
public:
	virtual ~CDataSource();
	virtual bool Open(const char *path) = 0;
	virtual void Close(void) = 0;
	//Get number of bytes
	virtual size_t GetSize(void);
	//Get current position
	virtual size_t GetPosition(void);
	//Read a byte
	//return the byte, -1 means end of data source
	virtual int ReadData(void);
	//Read specified bytes into a buffer
	//return number of bytes read actually
	virtual int ReadData(void *pData, int nSize) = 0;
	//Read specified bytes and convert to an integer
	//return data
	int ReadLittleEndian(int nSize);
	//Read specified bytes reversely and convert to an integer
	//return data
	int ReadBigEndian(int nSize);
	//Seek to specified position
	//nFrom is SEEK_SET, SEEK_CUR, or SEEK_END
	//return success or not
	virtual bool SeekData(size_t nOffset, int nFrom);
};

#endif
