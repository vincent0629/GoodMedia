#ifndef _CACHESOURCE_H_
#define _CACHESOURCE_H_

#include "DataSource.h"

class CCacheSource : public CDataSource
{
public:
	typedef struct
	{
		int nIndex;
		unsigned char *pData;
		int nSize;
		int nOrder;
	} CachePage;
public:
	CCacheSource();
	~CCacheSource();
	bool Open(const char *path);
	void Close(void);
	size_t GetSize(void);
	size_t GetPosition(void);
	int ReadData(void *pData, int nSize);
	bool SeekData(size_t nOffset, int nFrom);
private:
	CDataSource *m_pSource;
	bool m_bDelete;
	size_t m_nSize, m_nPosition;
	int m_nCacheNum;
	CachePage **m_ppCaches;
	CachePage m_pCacheTables[5];
};

#endif
