#include "MatroskaSource.h"
#include <string.h>
#include <stdint.h>

CMatroskaSource::CMatroskaSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	int nLevel;
	unsigned int nID, nSize, nTimecodeScale;
	size_t pnPos[10];
	int64_t n64;
	char pData[8];

	m_MatroskaInfo.nVersion = 1;
	m_MatroskaInfo.nReadVersion = 1;
	m_MatroskaInfo.nTypeVersion = 1;
	m_MatroskaInfo.nTypeReadVersion = 1;
	nLevel = 0;
	nTimecodeScale = 1000000;
	m_nDuration = 0;

	nID = ReadID();
	if (nID != 0x1A45DFA3)  //EBML
		return;
	nSize = ReadUint();
	pnPos[nLevel++] = nSize + SourceGetPosition();

	for (;;)
	{
		while (nLevel > 0)
			if (SourceGetPosition() >= pnPos[nLevel - 1])
				--nLevel;
			else
				break;
		nID = ReadID();
		if (nID == 0)  //end of file
			break;
		nSize = ReadUint();
		pnPos[nLevel] = nSize + SourceGetPosition();

		switch (nLevel)
		{
			case 0:
				switch (nID)
				{
					case 0x1A45DFA3:  //EBML
						++nLevel;
						continue;
					case 0x18538067:  //Segment
						++nLevel;
						continue;
				}  //switch
				break;
			case 1:
				switch (nID)
				{
					case 0x4286:  //EBMLVersion
						m_MatroskaInfo.nVersion = SourceReadBigEndian(nSize);
						continue;
					case 0x42F7:  //EBMLReadVersion
						m_MatroskaInfo.nReadVersion = SourceReadBigEndian(nSize);
						continue;
					case 0x4282:  //DocType
						SourceReadData(pData, nSize);
						if (strncmp(pData, "matroska", 8) != 0)
							return;
						continue;
					case 0x4287:  //DocTypeVersion
						m_MatroskaInfo.nTypeVersion = SourceReadBigEndian(nSize);
						continue;
					case 0x4285:  //DocTypeReadVersion
						m_MatroskaInfo.nTypeReadVersion = SourceReadBigEndian(nSize);
						continue;
					case 0x1549A966:  //Info
						++nLevel;
						continue;
					case 0x1654AE6B:  //Track
						++nLevel;
						++m_nOutputNum;
						continue;
				}  //switch
				break;
			case 2:
				switch (nID)
				{
					case 0x2AD7B1:  //TimeCodeScale
						nTimecodeScale = SourceReadBigEndian(nSize);
						continue;
					case 0x4489:  //Duration
						for (; nSize > 0; nSize--)
							pData[nSize - 1] = SourceReadData();
						n64 = nSize == 4? *(float *)pData : *(double *)pData;
						n64 *= nTimecodeScale;
						n64 /= 1000000;
						m_nDuration = (int)n64;
						continue;
				}  //switch
				break;
		}  //switch (nLevel)

		SourceSeekData(nSize, SEEK_CUR);  //skip element
	}
}

CMatroskaSource::~CMatroskaSource()
{
}

MediaType CMatroskaSource::GetSourceType(void)
{
	return MEDIA_TYPE_MATROSKA;
}

bool CMatroskaSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(MatroskaInfo))
		return false;

	m_MatroskaInfo.nSize = sizeof(MatroskaInfo);
	memcpy(pInfo, &m_MatroskaInfo, m_MatroskaInfo.nSize);
	return true;
}

MediaType CMatroskaSource::GetOutputType(int nIndex)
{
	return MEDIA_TYPE_UNKNOWN;
}

bool CMatroskaSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return false;
}

size_t CMatroskaSource::GetPosition(int nIndex)
{
	return 0;  //not implemented
}

int CMatroskaSource::ReadData(int nIndex, void *pData, int nSize)
{
	return 0;  //not implemented
}

bool CMatroskaSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;  //note implemented
}

int CMatroskaSource::GetDuration(void)
{
	return m_nDuration;
}

int CMatroskaSource::SeekTime(int nTime)
{
	return 0;  //not implemented
}

unsigned int CMatroskaSource::ReadID(void)
{
	int n;
	unsigned int nValue;
	
	n = SourceReadData();
	if (n <= 0)  //end of file or invalid ID
		return 0;

	for (nValue = n; (n & 0x80) == 0; n <<= 1)
		nValue = (nValue << 8) | SourceReadData();
	return nValue;
}

unsigned int CMatroskaSource::ReadUint(void)
{
	int i;
	unsigned int n, nValue;

	i = 0;
	nValue = SourceReadData();
	for (n = nValue; (n & 0x80) == 0 && i < 8; n <<= 1)
	{
		nValue = (nValue << 8) | SourceReadData();
		++i;
	}
	nValue &= ~(1 << (i + 1) * 7);
	return nValue;
}
