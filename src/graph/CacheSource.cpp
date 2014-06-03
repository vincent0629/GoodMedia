#include "CacheSource.h"
#include "FileSource.h"
#include <string.h>

#define PAGE_SIZE 8192
#define PAGE_SIZE_SHIFT 13

CCacheSource::CCacheSource()
{
	int i;

	m_pSource = new CFileSource();
	for (i = 0; i < 5; i++)
		m_pCacheTables[i].pData = NULL;
	m_ppCaches = NULL;
	m_bDelete = false;
	m_nSize = 0;
	m_nPosition = 0;
}

CCacheSource::~CCacheSource()
{
	Close();
	delete m_pSource;
}

bool CCacheSource::Open(const char *path)
{
	int i;
	CachePage *pCache;

	Close();

	if (!m_pSource->Open(path))
		return false;

	m_nSize = m_pSource->GetSize();
	m_nCacheNum = (m_nSize + PAGE_SIZE - 1) >> PAGE_SIZE_SHIFT;
	m_ppCaches = new CachePage *[m_nCacheNum];
	for (i = 0; i < m_nCacheNum; i++)
		m_ppCaches[i] = NULL;
	for (i = 0; i < 5; i++)
	{
		pCache = m_pCacheTables + i;
		pCache->nIndex = -1;
		pCache->nSize = 0;
		pCache->nOrder = i;
	}
	m_nPosition = 0;

	return true;
}

void CCacheSource::Close(void)
{
	int i;

	for (i = 0; i < 5; i++)
	{
		delete[] m_pCacheTables[i].pData;
		m_pCacheTables[i].pData = NULL;
	}
	delete[] m_ppCaches;
	m_ppCaches = NULL;
	m_pSource->Close();
	m_nSize = 0;
	m_nPosition = 0;
}

size_t CCacheSource::GetSize(void)
{
	return m_nSize;
}

size_t CCacheSource::GetPosition(void)
{
	return m_nPosition;
}

int CCacheSource::ReadData(void *pData, int nSize)
{
	int i, nIndex, nOffset, nResult, n;
	CachePage *pCache;

	if (m_pSource == NULL || m_nPosition >= m_nSize)
		return 0;

	nIndex = m_nPosition >> PAGE_SIZE_SHIFT;
	nOffset = m_nPosition - (nIndex << PAGE_SIZE_SHIFT);
	nResult = 0;
	for (;;)
	{
		if (m_ppCaches[nIndex] == NULL)
		{
			for (i = 0; i < 5; i++)
				if (m_pCacheTables[i].nOrder == 4)
					break;
			pCache = m_pCacheTables + i;
			if (pCache->nIndex != -1)
				m_ppCaches[pCache->nIndex] = NULL;
			m_ppCaches[nIndex] = pCache;
			pCache->nIndex = nIndex;

			if (pCache->pData == NULL)
				pCache->pData = new unsigned char[PAGE_SIZE];
			m_pSource->SeekData(nIndex << PAGE_SIZE_SHIFT, SEEK_SET);
			pCache->nSize = m_pSource->ReadData(pCache->pData, PAGE_SIZE);
		}
		else
			pCache = m_ppCaches[nIndex];
		if (pCache->nOrder != 0)
		{
			for (i = 0; i < 5; i++)
				if (m_pCacheTables[i].nOrder < pCache->nOrder)
					++m_pCacheTables[i].nOrder;
			pCache->nOrder = 0;
		}

		if (nOffset + nSize - nResult <= pCache->nSize)
		{
			memcpy(pData, pCache->pData + nOffset, nSize - nResult);
			nResult = nSize;
			break;
		}
		else
		{
			n = pCache->nSize - nOffset;
			memcpy(pData, pCache->pData + nOffset, n);
			pData = (char *)pData + n;
			nResult += n;
			if (++nIndex == m_nCacheNum)
				break;
			nOffset = 0;
		}
	}
	m_nPosition += nResult;
	return nResult;
}

bool CCacheSource::SeekData(size_t nOffset, int nFrom)
{
	bool bResult;

	if (nFrom == SEEK_CUR)
		nOffset += m_nPosition;
	else if (nFrom == SEEK_END)
		nOffset += m_nSize;
	if (nOffset < 0)
	{
		m_nPosition = 0;
		bResult = false;
	}
	else if (nOffset > m_nSize)
	{
		m_nPosition = m_nSize;
		bResult = false;
	}
	else
	{
		m_nPosition = nOffset;
		bResult = true;
	}
	return bResult;
}
