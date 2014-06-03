#include "QuickTimeSource.h"
#include "PCMInfo.h"
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#define strncasecmp strnicmp
#define strcasecmp stricmp
#endif

CQuickTimeSource::CQuickTimeSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	char pType[5];
	int i, nLevel, nValue;
	size_t nSize, pnPos[10], nDataHead, nDataSize;
	int64_t n64;
	int *ppnChunkTable[10];
	int pnChunkTableSize[10];
	int pnIndex[10];
	bool bChunkOffsetExists;
	MediaType type;

	memset(&m_Info, 0, sizeof(QuickTimeInfo));
	m_nDuration = 0;
	m_SourceType = MEDIA_TYPE_QUICKTIME;
	m_pChunkTable = NULL;
	memset(ppnChunkTable, 0, sizeof(ppnChunkTable));
	memset(m_pTypes, 0, sizeof(m_pTypes));

	pType[4] = '\0';
	nLevel = 0;
	bChunkOffsetExists = false;
	for (;;)
	{
		while (nLevel > 0)
			if (SourceGetPosition() >= pnPos[nLevel - 1])
				--nLevel;
			else
				break;
		nSize = NextAtom(pType);
		if (nSize == -1)  //end of file
			break;
		pnPos[nLevel] = nSize + SourceGetPosition();

		switch (nLevel)
		{
			case 0:
				if (strcmp(pType, "ftyp") == 0)
				{
					SourceReadData(m_Info.pFileType, 4);
					if (strncasecmp(m_Info.pFileType, "3g", 2) == 0)
						m_SourceType = MEDIA_TYPE_3GPP;
					else //if (strcasecmp(m_Info.pFileType, "avc1") == 0 || strncasecmp(m_Info.pFileType, "iso", 3) == 0 || strncasecmp(m_Info.pFileType, "m4", 2) == 0 || strcasecmp(m_Info.pFileType, "mmp4") == 0 || strncasecmp(m_Info.pFileType, "mp4", 3) == 0 || strcasecmp(m_Info.pFileType, "msnv") == 0 || strncasecmp(m_Info.pFileType, "nd", 2) == 0)
						m_SourceType = MEDIA_TYPE_MPEG4;
				}
				else if (strcmp(pType, "moov") == 0)  //movie
				{
					++nLevel;
					continue;
				}
				else if (strcmp(pType, "mdat") == 0)  //movie data
				{
					nDataHead = SourceGetPosition();
					nDataSize = nSize;
				}
				else if (strcmp(pType, "") == 0)
					return;
				else if (strstr("free,junk,skip,wide,pnot,uuid,pict", pType) == NULL)
				{
					if (strstr("FREE,JUNK,SKIP,WIDE,PNOT,UUID,PICT", pType) == NULL)
						return;
				}
				break;
			case 1:
				if (strcmp(pType, "mvhd") == 0)  //movie header
				{
					SourceSeekData(12, SEEK_CUR);
					nValue = SourceReadBigEndian(4);  //time scale
					n64 = SourceReadBigEndian(4);  //duration
					n64 *= 1000;
					m_nDuration = (int)(n64 / nValue);
				}
				else if (strcmp(pType, "trak") == 0)  //track
				{
					++m_nOutputNum;
					++nLevel;
					continue;
				}
				else if (strcmp(pType, "cmov") == 0)  //compressed movie
				{
					++m_nOutputNum;
					++nLevel;
					continue;
				}
				break;
			case 2:
				if (strcmp(pType, "tkhd") == 0)  //track header
				{
					/*SourceSeekData(76, SEEK_CUR);
					m_VideoInfo.nWidth = SourceReadLittleEndian(4);
					m_VideoInfo.nHeight = SourceReadLittleEndian(4);*/
				}
				else if (strcmp(pType, "mdia") == 0)  //media
				{
					++nLevel;
					continue;
				}
				else if (strcmp(pType, "dcom") == 0)  //data compression
					SourceReadData(pType, 4);  //compression algorithm
				break;
			case 3:
				if (strcmp(pType, "hdlr") == 0)  //handler reference
				{
					SourceSeekData(4, SEEK_CUR);
					SourceReadData(pType, 4);  //component type
					SourceReadData(pType, 4);  //component subtype
					if (strcmp(pType, "vide") == 0)
						m_pTypes[m_nOutputNum - 1] = MEDIA_TYPE_UNKNOWN_VIDEO;
					else if (strcmp(pType, "soun") == 0)
						m_pTypes[m_nOutputNum - 1] = MEDIA_TYPE_UNKNOWN_AUDIO;
				}
				else if (strcmp(pType, "minf") == 0)  //media information
				{
					++nLevel;
					continue;
				}
				break;
			case 4:
				if (strcmp(pType, "stbl") == 0)  //sample table
				{
					++nLevel;
					continue;
				}
				break;
			case 5:
				if (strcmp(pType, "stsd") == 0)  //sample description
				{
					SourceSeekData(12, SEEK_CUR);
					SourceReadData(m_pFormats[m_nOutputNum - 1], 4);  //data format
					m_pFormats[m_nOutputNum - 1][4] = '\0';
					type = MediaTypeOf(m_pFormats[m_nOutputNum - 1]);
					if (type != MEDIA_TYPE_UNKNOWN)
						m_pTypes[m_nOutputNum - 1] = type;
				}
				else if (strcmp(pType, "stco") == 0)  //chunk offset
				{
					SourceSeekData(4, SEEK_CUR);
					nValue = SourceReadBigEndian(4);  //number of entries
					pnChunkTableSize[m_nOutputNum - 1] = nValue;
					ppnChunkTable[m_nOutputNum - 1] = new int[nValue];
					for (i = 0; i < nValue; i++)  //chunk offset table
						ppnChunkTable[m_nOutputNum - 1][i] = SourceReadBigEndian(4);
					bChunkOffsetExists = true;
				}
				break;
		}  //switch (nLevel)
		SourceSeekData(pnPos[nLevel], SEEK_SET);  //skip rest of atom
	}  //for (;;)

	if (m_nOutputNum == 0)
		return;

	if (bChunkOffsetExists)
	{
		nSize = 0;
		for (i = 0; i < m_nOutputNum; i++)
			nSize += pnChunkTableSize[i];
		m_pChunkTable = new ChunkStruct[nSize + 1];  //need an extra offset which point to the end of movie data

		memset(pnIndex, 0, m_nOutputNum * sizeof(pnIndex[0]));
		m_nChunkTableSize = 0;
		while (m_nChunkTableSize < nSize)
		{
			nValue = -1;
			for (i = 0; i < m_nOutputNum; i++)
				if (pnIndex[i] < pnChunkTableSize[i])
				{
					if (nValue == -1)
						nValue = i;
					else if (ppnChunkTable[i][pnIndex[i]] < ppnChunkTable[nValue][pnIndex[nValue]])
						nValue = i;
				}
			m_pChunkTable[m_nChunkTableSize].nIndex = nValue;
			m_pChunkTable[m_nChunkTableSize].nOffset = ppnChunkTable[nValue][pnIndex[nValue]];
			++m_nChunkTableSize;
			++pnIndex[nValue];
		}
		m_pChunkTable[m_nChunkTableSize].nIndex = -1;
		m_pChunkTable[m_nChunkTableSize].nOffset = nDataHead + nDataSize;  //end of movie data
		++m_nChunkTableSize;

		for (i = 0; i < m_nOutputNum; i++)
			delete[] ppnChunkTable[i];
	}
	else
	{
		m_nChunkTableSize = 1;
		m_pChunkTable = new ChunkStruct[2];
		m_pChunkTable[0].nIndex = 0;
		m_pChunkTable[0].nOffset = nDataHead;
		m_pChunkTable[1].nIndex = -1;
		m_pChunkTable[1].nOffset = nDataHead + nDataSize;
	}

	for (i = 0; i < m_nOutputNum; i++)
		m_pStreamPos[i].nHeadPos = nDataHead;
	SeekTime(0);
}

CQuickTimeSource::~CQuickTimeSource()
{
	delete[] m_pChunkTable;
}

MediaType CQuickTimeSource::GetSourceType(void)
{
	return m_SourceType;
}

bool CQuickTimeSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(QuickTimeInfo))
		return false;

	m_Info.nSize = sizeof(QuickTimeInfo);
	memcpy(pInfo, &m_Info, m_Info.nSize);
	return true;
}

MediaType CQuickTimeSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : m_pTypes[nIndex];
}

bool CQuickTimeSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	PCMInfo *pPCMInfo;
	char str[20];
	int nLen;

	if (nIndex >= m_nOutputNum)
		return false;

	switch (m_pTypes[nIndex])
	{
		case MEDIA_TYPE_PCM:
			if (pInfo->nSize < sizeof(PCMInfo))
				return false;
			pPCMInfo = (PCMInfo *)pInfo;
			pPCMInfo->nSize = sizeof(PCMInfo);
			pPCMInfo->bSigned = false;
			pPCMInfo->nBitsPerSample = 8;
			pPCMInfo->nByteOrder = PCM_LITTLE_ENDIAN;
			pPCMInfo->nChannel = 1;
			pPCMInfo->nFormat = PCM_MULAW;
			pPCMInfo->nSampleRate = 16000;
			return true;
		case MEDIA_TYPE_UNKNOWN_VIDEO:
			sprintf(str, "Video format = %s", m_pFormats[nIndex]);
			nLen = strlen(str) + 1;
			if (pInfo->nSize < sizeof(MediaInfo) + nLen)
				return false;
			pInfo->nSize = sizeof(MediaInfo) + nLen;
			strcpy(((TextInfo *)pInfo)->pText, str);
			return true;
		case MEDIA_TYPE_UNKNOWN_AUDIO:
			sprintf(str, "Audio format = %s", m_pFormats[nIndex]);
			nLen = strlen(str) + 1;
			if (pInfo->nSize < sizeof(MediaInfo) + nLen)
				return false;
			pInfo->nSize = sizeof(MediaInfo) + nLen;
			strcpy(((TextInfo *)pInfo)->pText, str);
			return true;
		default:
			return false;
	}
	return false;
}

int CQuickTimeSource::GetDuration(void)
{
	return m_nDuration;
}

int CQuickTimeSource::SeekTime(int nTime)
{
	int i;

	for (i = 0; i < m_nOutputNum; i++)
	{
		m_pStreamPos[i].nAbsPos = m_pStreamPos[i].nHeadPos;
		m_pStreamPos[i].nRelPos = 0;
		m_pStreamPos[i].nSize = 0;
		m_pStreamPos[i].nTime = 0;
		m_pnChunkIndex[i] = -1;
	}
	return 0;
}

size_t CQuickTimeSource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_pStreamPos[nIndex].nRelPos;
}

int CQuickTimeSource::ReadData(int nIndex, void *pData, int nSize)
{
	StreamPos *pPos;
	int *pnChunkIndex;

	if (nIndex >= m_nOutputNum)
		return 0;

	pPos = m_pStreamPos + nIndex;
	pnChunkIndex = m_pnChunkIndex + nIndex;
	if (pPos->nRelPos >= pPos->nSize)
		while (*pnChunkIndex < m_nChunkTableSize - 1)
		{
			++(*pnChunkIndex);
			if (m_pChunkTable[*pnChunkIndex].nIndex == nIndex)
			{
				pPos->nSize += m_pChunkTable[*pnChunkIndex + 1].nOffset - m_pChunkTable[*pnChunkIndex].nOffset;
				pPos->nAbsPos = m_pChunkTable[*pnChunkIndex].nOffset;
				break;
			}
		}

	if (pPos->nRelPos < pPos->nSize)
	{
		if (nSize > pPos->nSize - pPos->nRelPos)
			nSize = pPos->nSize - pPos->nRelPos;
		SourceSeekData(pPos->nAbsPos, SEEK_SET);
		nSize = SourceReadData(pData, nSize);
		pPos->nAbsPos += nSize;
		pPos->nRelPos += nSize;
		return nSize;
	}

	return 0;
}

bool CQuickTimeSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;  //note implemented
}

int64_t CQuickTimeSource::NextAtom(char *pType)
{
	size_t nSize;

	nSize = SourceReadBigEndian(4);
	if (SourceReadData(pType, 4) < 4)
		return -1;

	if (nSize == 1)
	{
		nSize = SourceReadBigEndian(4);
		nSize = nSize << 32 | SourceReadBigEndian(4);
		nSize -= 16;
	}
	else if (nSize == 0)
		nSize = SourceGetSize() - SourceGetPosition();
	else
		nSize -= 8;
	return nSize;
}

MediaType CQuickTimeSource::MediaTypeOf(char *str)
{
	MediaType type;

	type = MEDIA_TYPE_UNKNOWN;
	if (strcmp(str, ".mp3") == 0)
		type = MEDIA_TYPE_MPEGAUDIO;
	else if (strcmp(str, "alaw") == 0)
		type = MEDIA_TYPE_PCM;
	else if (strcmp(str, "avc1") == 0)
		type = MEDIA_TYPE_H264;
	else if (strcmp(str, "mp4a") == 0)
		type = MEDIA_TYPE_AAC;
	else if (strcmp(str, "mp4v") == 0)
		type = MEDIA_TYPE_MPEG4VIDEO;
	else if (strcmp(str, "mpeg") == 0)
		type = MEDIA_TYPE_MPEGVIDEO;
	//else if (strcmp(str, "samr") == 0)
	//	type = MEDIA_TYPE_AMR;
	else if (strcmp(str, "ulaw") == 0)
		type = MEDIA_TYPE_PCM;
	return type;
}
