#include "WrapSource.h"

CWrapSource::CWrapSource(CDataSource *pSource, MediaType type)
{
	m_pSource = pSource;
	m_nOutputNum = 1;
	m_OutputType = type == MEDIA_TYPE_UNKNOWN? DetectOutputType(0, 0) : type;
}

CWrapSource::~CWrapSource()
{
}

size_t CWrapSource::GetSize(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_pSource->GetSize();
}

size_t CWrapSource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_pSource->GetPosition();
}

int CWrapSource::ReadData(int nIndex)
{
	return nIndex >= m_nOutputNum? -1 : m_pSource->ReadData();
}

int CWrapSource::ReadData(int nIndex, void *pData, int nSize)
{
	char pBuffer[1024];
	int n, nCount;

	if (nIndex >= m_nOutputNum)
		return 0;
	if (pData != NULL)
		return m_pSource->ReadData(pData, nSize);

	nCount = 0;
	while (nCount < nSize)
	{
		n = nSize - nCount;
		if (n > sizeof(pBuffer))
			n = sizeof(pBuffer);
		n = m_pSource->ReadData(pBuffer, n);
		if (n == 0)
			break;
		nCount += n;
	}
	return nCount;
}

int CWrapSource::ReadBigEndian(int nIndex, int nSize)
{
	return nIndex >= m_nOutputNum? 0 : m_pSource->ReadBigEndian(nSize);
}

bool CWrapSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return nIndex >= m_nOutputNum? false : m_pSource->SeekData(nOffset, nFrom);
}

MediaType CWrapSource::GetSourceType(void)
{
	return MEDIA_TYPE_UNKNOWN;
}

bool CWrapSource::GetSourceInfo(MediaInfo *pInfo)
{
	return false;
}

MediaType CWrapSource::GetOutputType(int nIndex)
{
	return m_OutputType;
}

int CWrapSource::GetDuration(void)
{
	return 0;
}

int CWrapSource::SeekTime(int nTime)
{
	return 0;
}
