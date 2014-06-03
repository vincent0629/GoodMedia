#include "MPEGProgramSource.h"
#include <string.h>
#include <assert.h>

#define PACK_START_CODE 0xBA
#define PROGRAM_END_CODE 0xB9
#define SYSTEM_HEADER_START_CODE 0xBB
#define PRIVATE_1_ID 0xBD

CMPEGProgramSource::CMPEGProgramSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	unsigned char pBuffer[8];
	int i, nCode;
	int nValue, nLen;
	int nStreamNum;
	bool *pbCodeUsed;

	m_pStreamPos = NULL;
	m_nDuration = 0;
	m_pnCodes = NULL;

	if (NextSync() != PACK_START_CODE)  //pack
		return;

	nValue = SourceReadData();
	if ((nValue & 0xF1) == 0x21)
	{
		m_MPEGProgramInfo.nVersion = 1;
		SourceReadData(pBuffer, 7);
		if (!((pBuffer[1] & 0x01) && (pBuffer[3] & 0x01) && (pBuffer[4] & 0x80) && (pBuffer[6] & 0x01)))  //check pack header
			return;
	}
	else if ((nValue & 0xC4) == 0x44)
	{
		m_MPEGProgramInfo.nVersion = 2;
		SourceReadData(pBuffer, 8);
		if (!((pBuffer[1] & 0x04) && (pBuffer[3] & 0x04) && (pBuffer[4] & 0x01) && (pBuffer[7] & 0x03) == 0x03))  //check pack header
			return;
		nValue = SourceReadData();  //stuffing length
		SourceSeekData(nValue & 0x07, SEEK_CUR);  //stuffing bytes
	}
	else
		return;

	nStreamNum = 0;
	nCode = NextSync();
	if (nCode == SYSTEM_HEADER_START_CODE)  //system header
	{
		SourceSeekData(5, SEEK_CUR);
		nValue = SourceReadData();
		m_MPEGProgramInfo.bCBR = (nValue & 2) != 0;  //CBR or VBR
		nStreamNum = nValue >> 2;  //audio stream number
		nStreamNum += SourceReadData() & 0x1F;  //video stream number
	}

	m_pnCodes = new int[nStreamNum + 2];  //+2 for safety
	pbCodeUsed = new bool[0x200];
	memset(pbCodeUsed, 0, 0x200 * sizeof(bool));
	for (i = 0; i < 30 || (i < 1000 && nStreamNum > 0); i++)
	{
		nCode = NextPacket(&nLen);
		if (nCode == -1)
			break;
		else if (nCode >= 0xC0 && nCode <= 0xFE)  //elementary stream
		{
			if (!pbCodeUsed[nCode])
			{
				--nStreamNum;
				pbCodeUsed[nCode] = true;
				m_pnCodes[m_nOutputNum++] = nCode;
			}
		}
		else if ((nCode >> 8) == PRIVATE_1_ID)  //private 1
		{
			if (!pbCodeUsed[0x100 | (nCode & 0xFF)])
			{
				--nStreamNum;
				pbCodeUsed[0x100 | (nCode & 0xFF)] = true;
				m_pnCodes[m_nOutputNum++] = nCode;
			}
		}
		SourceSeekData(nLen, SEEK_CUR);  //skip packet
	}  //for
	delete[] pbCodeUsed;

	m_pStreamPos = new StreamPos[m_nOutputNum];
	if (m_nOutputNum > 0)
		SeekTime(0);
}

CMPEGProgramSource::~CMPEGProgramSource()
{
	delete[] m_pStreamPos;
	delete[] m_pnCodes;
}

MediaType CMPEGProgramSource::GetSourceType(void)
{
	return MEDIA_TYPE_MPEGPROGRAM;
}

bool CMPEGProgramSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(MPEGProgramInfo))
		return false;

	m_MPEGProgramInfo.nSize = sizeof(MPEGProgramInfo);
	memcpy(pInfo, &m_MPEGProgramInfo, m_MPEGProgramInfo.nSize);
	return true;
}

MediaType CMPEGProgramSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_PESPACKET;
}

bool CMPEGProgramSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return false;
}

size_t CMPEGProgramSource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_pStreamPos[nIndex].nRelPos;
}

int CMPEGProgramSource::ReadData(int nIndex, void *pData, int nSize)
{
	StreamPos *pPos;
	int nCode, n;
	int nPacketLen;

	if (nIndex >= m_nOutputNum)
		return 0;

	pPos = m_pStreamPos + nIndex;
	SourceSeekData(pPos->nAbsPos, SEEK_SET);
	if (pPos->nRelPos >= pPos->nSize)
	{
		nCode = m_pnCodes[nIndex];
		while ((n = NextPacket(&nPacketLen)) != nCode)
		{
			if (n == -1)
				return 0;
			SourceSeekData(nPacketLen, SEEK_CUR);  //skip packet
		}

 		SourceSeekData(-6, SEEK_CUR);
		pPos->nSize += 6 + nPacketLen;
	}

	if (pPos->nRelPos < pPos->nSize)
	{
		if (nSize > pPos->nSize - pPos->nRelPos)
			nSize = (int)(pPos->nSize - pPos->nRelPos);
		SourceReadData(pData, nSize);
		pPos->nRelPos += nSize;
	}
	else
		nSize = 0;
	pPos->nAbsPos = SourceGetPosition();

	return nSize;
}

bool CMPEGProgramSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	StreamPos *pPos;
	size_t nPos;
	int n;

	if (nIndex >= m_nOutputNum)
		return false;

	pPos = m_pStreamPos + nIndex;
	nPos = pPos->nRelPos;
	if (nFrom == SEEK_CUR)
		nOffset += nPos;
	else if (nFrom != SEEK_SET)
		return false;

	if (nOffset < nPos)
	{
		nPos = pPos->nAbsPos = pPos->nRelPos = pPos->nSize = 0;  //seek to begin of source
		pPos->nTime = 0;
	}
	else
		nOffset -= nPos;
	while (nOffset >= 3000)
		if ((n = ReadData(nIndex, NULL, 3000)) == 0)
			break;
		else
			nOffset -= n;
	if (nOffset > 0)
		ReadData(nIndex, NULL, (int)nOffset);
	return true;
}

int CMPEGProgramSource::GetDuration(void)
{
	size_t nPos, nNextPos;
	int nPacketLen;
	int nStreamID;
	size_t pts, minPTS, maxPTS;

	if (m_nOutputNum == 0)
		return 0;
	if (m_nDuration != 0)
		return m_nDuration;
	if (SourceGetSize() == 0)  //unknown size
		return 0;

	nPos = SourceGetPosition();  //remember current position

	SourceSeekData(0, SEEK_SET);
	minPTS = -1;
	for (;;)
	{
		nStreamID = NextPacket(&nPacketLen);
		if (nStreamID == -1)
			return 0;
		nNextPos = SourceGetPosition() + nPacketLen;
		minPTS = ParsePTS(nStreamID);
		if (minPTS >= 0)
			break;
		SourceSeekData(nNextPos, SEEK_SET);  //skip packet
	}

	if (SourceGetSize() > 1024 * 1024)
		SourceSeekData(SourceGetSize() - 1024 * 1024, SEEK_SET);
	maxPTS = pts = 0;
	for (;;)
	{
		nStreamID = NextPacket(&nPacketLen);
		if (nStreamID == -1)
			break;
		nNextPos = SourceGetPosition() + nPacketLen;
		pts = ParsePTS(nStreamID);
		if (pts > maxPTS)
			maxPTS = pts;
		SourceSeekData(nNextPos, SEEK_SET);  //skip packet
	}

	SourceSeekData(nPos, SEEK_SET);  //restore position
	m_nDuration = maxPTS < minPTS? 0 : (int)((maxPTS - minPTS) / 90);
	return m_nDuration;
}

int CMPEGProgramSource::SeekTime(int nTime)
{
	int i;
	CMediaSource *pSource;

	for (i = 0; i < m_nOutputNum; i++)
	{
		if (m_ppOutputSources != NULL)
		{
			pSource = GetOutputSource(i);
			if (pSource != NULL)
				pSource->SeekTime(0);
		}

		memset(m_pStreamPos + i, 0, sizeof(StreamPos));
	}
	SourceSeekData(0, SEEK_SET);
	return 0;
}

int CMPEGProgramSource::GetOutputTime(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_pStreamPos[nIndex].nTime;
}

int CMPEGProgramSource::NextSync(void)
{
	int nBuffer;
	int i, nCode;

	nBuffer = SourceReadBigEndian(2);
	i = 20240;
	do
	{
		if (--i <= 0)
			return -1;
		if ((nCode = SourceReadData()) == -1)
			return -1;

		nBuffer = ((nBuffer & 0xFFFF) << 8) | nCode;
	} while (nBuffer != 0x000001);
	return SourceReadData();
}

int CMPEGProgramSource::NextPacket(int *pnPacketLen)
{
	int nCode;
	int nLen;

	for (;;)
	{
		nCode = NextSync();
		if (nCode == -1 || nCode == PROGRAM_END_CODE)  //end of program
			return -1;

		//pack
		if (nCode == PACK_START_CODE)
		{
			if (m_MPEGProgramInfo.nVersion == 1)
				SourceSeekData(8, SEEK_CUR);
			else
			{
				SourceSeekData(9, SEEK_CUR);
				nLen = SourceReadData();  //stuffing length
				SourceSeekData(nLen & 0x07, SEEK_CUR);  //stuffing bytes
			}
			nCode = NextSync();
			if (nCode == SYSTEM_HEADER_START_CODE)  //system header
			{
				nLen = SourceReadBigEndian(2);  //header length
				SourceSeekData(nLen, SEEK_CUR);  //system header
				nCode = NextSync();
			}
		}

		if (nCode >= 0xBC)  //packet
		{
			if (pnPacketLen)
				*pnPacketLen = SourceReadBigEndian(2);  //packet length
			if (nCode == PRIVATE_1_ID)  //private stream 1
			{
				SourceSeekData(2, SEEK_CUR);
				nLen = SourceReadData();  //header length
				SourceSeekData(nLen, SEEK_CUR);
				nCode = (PRIVATE_1_ID << 8) | SourceReadData();
				SourceSeekData(-1 - nLen - 3, SEEK_CUR);
			}
			break;
		}  //if
	}  //for

	return nCode;
}

size_t CMPEGProgramSource::ParsePTS(int nStreamID)
{
	size_t nPTS;
	int nFlag, nValue;

	nPTS = -1;
	if (m_MPEGProgramInfo.nVersion == 1)  //MPEG 1
	{
		if (nStreamID != 0xBF)
		{
			do
			{
				nFlag = SourceReadData();
			} while (nFlag & 0x80);
			if ((nFlag & 0xC0) == 0x40)  //STD buffer
			{
				SourceReadData();
				nFlag = SourceReadData();
			}
			if ((nFlag & 0xE0) == 0x20)  //PTS
			{
				nPTS = nFlag;
				nPTS = (nPTS & 0x0E) << 29;
				nValue = SourceReadBigEndian(4);
				nPTS |= (nValue & 0xFFFE0000) >> 2;
				nPTS |= (nValue & 0xFFFE) >> 1;
			}
		}
	}
	else  //MPEG 2
	{
		if (nStreamID != 0xBC && nStreamID != 0xBE && nStreamID != 0xBF && nStreamID != 0xF0 && nStreamID != 0xF1 && nStreamID != 0xFF && nStreamID != 0xF2 && nStreamID != 0xF8)
		{
			SourceReadData();
			nFlag = SourceReadData();
			SourceReadData();
			if (nFlag & 0x80)  //PTS
			{
				nValue = SourceReadData();
				if (((nValue >> 4) & 0x03) == (nFlag >> 6))
				{
					nPTS = nValue;
					nPTS = (nPTS & 0x0E) << 29;
					nValue = SourceReadBigEndian(4);
					nPTS |= (nValue & 0xFFFE0000) >> 2;
					nPTS |= (nValue & 0xFFFE) >> 1;
				}
			}
		}
	}
	return nPTS;
}
