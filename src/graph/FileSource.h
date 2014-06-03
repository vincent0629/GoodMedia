#ifndef _FILESOURCE_H_
#define _FILESOURCE_H_

#include "DataSource.h"

class CFileSource : public CDataSource
{
public:
	CFileSource();
	~CFileSource();
	bool Open(const char *path);
	void Close(void);
	size_t GetSize(void);
	size_t GetPosition(void);
	int ReadData(void);
	int ReadData(void *pData, int nSize);
	bool SeekData(size_t nOffset, int nFrom);
private:
	FILE *m_pFile;
	size_t m_nSize;
};

#endif
