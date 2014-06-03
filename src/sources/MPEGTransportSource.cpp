#include "MPEGTransportSource.h"
#include <string.h>
#include <assert.h>

#define SYNC_BYTE 0x47

CMPEGTransportSource::CMPEGTransportSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	int i;
	StreamPos *pPos;

	m_nFirstSyncPos = 0;
	m_pPASource = NULL;
	m_PAInfo.nProgramNum = 0;
	m_ppPMSource = NULL;
	m_pPMInfo = NULL;

	if (NextSync(true) == -1)  //sync byte
		return;
	m_nFirstSyncPos = SourceGetPosition() - 1;
	SourceSeekData(188 - 1, SEEK_CUR);
	if (NextSync(true) == -1)
		return;
	m_nPacketSize = SourceGetPosition() - 1 - m_nFirstSyncPos;
	if (m_nPacketSize != 188 && m_nPacketSize != 192)
		return;
	for (i = 0; i < 2; i++)
	{
		SourceSeekData(m_nPacketSize - 1, SEEK_CUR);
		if (SourceReadData() != SYNC_BYTE)
			return;
	}

	memset(m_pStreamPos, 0, sizeof(m_pStreamPos));
	m_nOutputNum = 1;
	SourceSeekData(m_nFirstSyncPos, SEEK_SET);
	pPos = m_pStreamPos;
	pPos->nAbsPos = pPos->nHeadPos = m_nFirstSyncPos;
	m_pPASource = new CProgramAssociationSource(this, 0);
	m_PAInfo.nSize = sizeof(ProgramAssociationInfo);
	m_pPASource->GetSourceInfo(&m_PAInfo);

	m_nOutputNum += m_PAInfo.nProgramNum;
	for (i = 1; i < m_nOutputNum; i++)
	{
		pPos = m_pStreamPos + i;
		pPos->nAbsPos = pPos->nHeadPos = m_nFirstSyncPos;
	}
	m_ppPMSource = new CProgramMapSource *[m_PAInfo.nProgramNum];
	m_pPMInfo = new ProgramMapInfo[m_PAInfo.nProgramNum];
	for (i = 0; i < m_PAInfo.nProgramNum; i++)
	{
		if (m_PAInfo.pInfo[i].nProgramNumber == 0)
		{
			m_ppPMSource[i] = NULL;
			memset(m_pPMInfo + i, 0, sizeof(ProgramMapInfo));
		}
		else
		{
			m_ppPMSource[i] = new CProgramMapSource(this, i + 1);
			m_pPMInfo[i].nSize = sizeof(ProgramMapInfo);
			m_ppPMSource[i]->GetSourceInfo(m_pPMInfo + i);
			m_nOutputNum += m_pPMInfo[i].nStreamNum;
		}
	}

	for (i = 1 + m_PAInfo.nProgramNum; i < m_nOutputNum; i++)
	{
		pPos = m_pStreamPos + i;
		pPos->nAbsPos = pPos->nHeadPos = m_nFirstSyncPos;
	}
	SeekTime(0);
}

CMPEGTransportSource::~CMPEGTransportSource()
{
	int i;

	delete m_pPASource;
	for (i = 0; i < m_PAInfo.nProgramNum; i++)
		delete m_ppPMSource[i];
	delete[] m_ppPMSource;
	delete[] m_pPMInfo;
}

MediaType CMPEGTransportSource::GetSourceType(void)
{
	return MEDIA_TYPE_MPEGTRANSPORT;
}

bool CMPEGTransportSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(MediaInfo))
		return false;

	pInfo->nSize = sizeof(MediaInfo);
	return true;
}

MediaType CMPEGTransportSource::GetOutputType(int nIndex)
{
	if (nIndex >= m_nOutputNum)
		return MEDIA_TYPE_UNKNOWN;

	if (nIndex == 0)
		return MEDIA_TYPE_PROGRAMASSOCIATION;
	else if (nIndex <= m_PAInfo.nProgramNum)
		return MEDIA_TYPE_PROGRAMMAP;
	else
		return MEDIA_TYPE_PESPACKET;
}

CMediaSource *CMPEGTransportSource::GetOutputSource(int nIndex)
{
	if (nIndex >= m_nOutputNum)
		return NULL;

	if (nIndex == 0)
		return m_pPASource;
	if (nIndex <= m_PAInfo.nProgramNum)
		return m_ppPMSource[nIndex - 1];
	return CMediaSource::GetOutputSource(nIndex);
}

size_t CMPEGTransportSource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_pStreamPos[nIndex].nRelPos;
}

int CMPEGTransportSource::ReadData(int nIndex, void *pData, int nSize)
{
	StreamPos *pPos;
	int nPID, nTargetPID;
	int nPacketLen;
	StreamInfo *pInfo;

	if (nIndex >= m_nOutputNum)
		return 0;

	pPos = m_pStreamPos + nIndex;
	SourceSeekData(pPos->nAbsPos, SEEK_SET);
	if (pPos->nRelPos >= pPos->nSize)
	{
		if (nIndex == 0)  //program association section
			nTargetPID = 0;
		else if (nIndex < 1 + m_PAInfo.nProgramNum)  //program map section
			nTargetPID = m_PAInfo.pInfo[nIndex - 1].nPID;
		else
		{
			pInfo = GetStreamInfo(nIndex);
			nTargetPID = pInfo->nPID;
		}

		do
		{
			nPID = NextPacket(&nPacketLen);
			if (nPID == -1)
			{
				pPos->nAbsPos = SourceGetPosition();
				return 0;
			}
		} while (nPID != nTargetPID);
		pPos->nSize += nPacketLen;
	}

	if (nSize > pPos->nSize - pPos->nRelPos)
		nSize = pPos->nSize - pPos->nRelPos;
	nSize = SourceReadData(pData, nSize);
	pPos->nRelPos += nSize;
	pPos->nAbsPos = SourceGetPosition();
	return nSize;
}

bool CMPEGTransportSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	int nSize, n;
	StreamPos *pPos;

	if (nFrom == SEEK_CUR)
	{
		nSize = 0;
		while (nSize < nOffset)
		{
			n = ReadData(nIndex, NULL, nOffset - nSize);
			if (n == 0)
				return false;
			nSize += n;
		}
		return true;
	}
	else if (nFrom == SEEK_SET && nOffset == 0)
	{
		pPos = m_pStreamPos + nIndex;
		pPos->nAbsPos = pPos->nHeadPos;
		pPos->nRelPos = 0;
		pPos->nSize = 0;
		SourceSeekData(m_nFirstSyncPos, SEEK_SET);
		return true;
	}
	return false;
}

int CMPEGTransportSource::GetDuration(void)
{
	return 0;
}

int CMPEGTransportSource::SeekTime(int nTime)
{
	int i;
	StreamPos *pPos;

	for (i = 1 + m_PAInfo.nProgramNum; i < m_nOutputNum; i++)
	{
		pPos = m_pStreamPos + i;
		pPos->nAbsPos = pPos->nHeadPos;
		pPos->nRelPos = 0;
		pPos->nSize = 0;
	}
	return 0;
}

int CMPEGTransportSource::NextSync(bool bSearch)
{
	int i, nCode;
	size_t nLen;

	if (bSearch)
		for (i = 192; i > 0; i--)
		{
			nCode = SourceReadData();
			if (nCode == -1 || nCode == SYNC_BYTE)
				return nCode;
		}
	else
	{
		nLen = (SourceGetPosition() - m_nFirstSyncPos) % m_nPacketSize;
		if (nLen != 0)
			SourceSeekData(m_nPacketSize - nLen, SEEK_CUR);
		if (SourceReadData() == SYNC_BYTE)
			return SYNC_BYTE;
	}
	return -1;
}

int CMPEGTransportSource::NextPacket(int *pnLen)
{
	int nPID;
	int i, nValue;
	size_t nPos;

	while (NextSync(false) == SYNC_BYTE)
	{
		nValue = SourceReadBigEndian(3);
		if ((nValue & 0x800000) == 0 && (nValue & 0x10))
		{
			nPos = SourceGetPosition() + 188 - 4;
			nPID = (nValue >> 8) & 0x1FFF;
			if (nValue & 0x20)
				SourceSeekData(SourceReadData(), SEEK_CUR);  //adaptation field
			if (nValue & 0x400000)  //payload unit start indicator
			{
				if (nPID == 0)
					SourceSeekData(SourceReadData(), SEEK_CUR);  //pointer field
				else
					for (i = 0; i < m_PAInfo.nProgramNum; i++)
						if (m_PAInfo.pInfo[i].nPID == nPID)
						{
							SourceSeekData(SourceReadData(), SEEK_CUR);  //pointer field
							break;
						}
			}

			assert(nPos >= SourceGetPosition());
			if (pnLen != NULL)
				*pnLen = (unsigned int)(nPos - SourceGetPosition());
			return nPID;
		}  //if
	}  //while
	return -1;  //end of stream
}

StreamInfo *CMPEGTransportSource::GetStreamInfo(int nIndex)
{
	int i, j;

	nIndex -= 1 + m_PAInfo.nProgramNum;
	i = j = 0;
	while (j + m_pPMInfo[i].nStreamNum <= nIndex)
		j += m_pPMInfo[i++].nStreamNum;
	return m_pPMInfo[i].nProgramNumber == 0? NULL : m_pPMInfo[i].pInfo + nIndex - j;
}
