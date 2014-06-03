#include "DataSource.h"
#include <assert.h>

CDataSource::~CDataSource()
{
}

size_t CDataSource::GetSize(void)
{
	return 0;
}

size_t CDataSource::GetPosition(void)
{
	return 0;
}

int CDataSource::ReadData(void)
{
	unsigned char nResult;

	return ReadData(&nResult, 1) == 0? -1 : nResult;
}

int CDataSource::ReadLittleEndian(int nSize)
{
	int nResult;

	assert(nSize <= sizeof(int));
	nResult = 0;
	ReadData(&nResult, nSize);
	return nResult;
}

int CDataSource::ReadBigEndian(int nSize)
{
	int i;
	int nResult;

	assert(nSize <= sizeof(int));
	nResult = 0;
	for (i = nSize; i > 0; i--)
		nResult = (nResult << 8) | ReadData();
	return nResult;
}

bool CDataSource::SeekData(size_t nOffset, int nFrom)
{
	return false;
}
