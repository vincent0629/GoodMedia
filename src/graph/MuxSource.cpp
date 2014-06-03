#include "MuxSource.h"
#include <string.h>
#include <assert.h>

CMuxSource::CMuxSource()
{
	m_nSourceNum = 0;
	m_MediaType = MEDIA_TYPE_UNKNOWN;
	m_nDuration = 0;
	m_nOutputTime = 0;
}

CMuxSource::~CMuxSource()
{
	int i;

	for (i = 0; i < m_nSourceNum; i++)
		delete[] m_pSourceParams[i].pBuffer;
}

MediaType CMuxSource::GetSourceType(void)
{
	return m_MediaType;
}

bool CMuxSource::GetSourceInfo(MediaInfo *pInfo)
{
	return false;
}

MediaType CMuxSource::GetOutputType(int nIndex)
{
	return m_MediaType;
}

bool CMuxSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	if (nIndex >= m_nOutputNum || pInfo->nSize < sizeof(MediaInfo))
		return false;

	pInfo->nSize = sizeof(MediaInfo);
	return true;
}

int CMuxSource::GetOutputTime(int nIndex)
{
	return m_nOutputTime;
}

int CMuxSource::GetDuration(void)
{
	return m_nDuration;
}

int CMuxSource::SeekTime(int nTime)
{
	int i;
	SourceParam *pParam;

	for (i = 0; i < m_nSourceNum; i++)
	{
		pParam = m_pSourceParams + i;
		delete[] pParam->pBuffer;
		pParam->pBuffer = NULL;
		pParam->nTime = 0;
	}
	return 0;
}

size_t CMuxSource::GetPosition(int nIndex)
{
	return 0;
}

int CMuxSource::ReadData(int nIndex, void *pData, int nSize)
{
	int i;
	SourceParam *pParam;
	int nTime;

	if (nIndex >= m_nOutputNum)
		return 0;

	nTime = -1;
	nIndex = 0;
	for (i = 0; i < m_nSourceNum; i++)
	{
		pParam = m_pSourceParams + i;
		if (pParam->pBuffer == NULL || pParam->nBufferSize < nSize)
		{
			delete[] pParam->pBuffer;
			pParam->pBuffer = new unsigned char[nSize];
			pParam->nBufferSize = nSize;
			pParam->nBufferBytes = pParam->pSource->ReadData(pParam->nIndex, pParam->pBuffer, pParam->nBufferSize);
			pParam->nTime = pParam->pSource->GetOutputTime(pParam->nIndex);
		}
		if ((pParam->nTime < nTime || nTime == -1) && pParam->nBufferBytes > 0)  //find out the source with the minimum output time
		{
			nTime = pParam->nTime;
			nIndex = i;
		}
	}

	pParam = m_pSourceParams + nIndex;
	if (nSize > pParam->nBufferBytes)
		nSize = pParam->nBufferBytes;
	if (nSize > 0)
	{
		if (pData != NULL)
			memcpy(pData, pParam->pBuffer, nSize);
		m_nOutputTime = pParam->nTime;

		pParam->nBufferBytes = pParam->pSource->ReadData(pParam->nIndex, pParam->pBuffer, pParam->nBufferSize);
		pParam->nTime = pParam->pSource->GetOutputTime(pParam->nIndex);
	}
	return nSize;
}

bool CMuxSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;
}

void CMuxSource::AddSource(CMediaSource *pSource, int nIndex)
{
	SourceParam *pParam;

	if (m_nSourceNum > 0)
		assert(pSource->GetOutputType(nIndex) == m_MediaType);

	pParam = m_pSourceParams + m_nSourceNum;
	pParam->pSource = pSource;
	pParam->nIndex = nIndex;
	pParam->pBuffer = NULL;
	pParam->nTime = 0;
	++m_nSourceNum;

	if (pSource->GetDuration() > m_nDuration)
		m_nDuration = pSource->GetDuration();
	if (m_nOutputNum == 0)
	{
		m_nOutputNum = 1;
		m_MediaType = pSource->GetOutputType(nIndex);
	}
}
