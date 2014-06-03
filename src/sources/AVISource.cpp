#include "AVISource.h"
#include "PCMInfo.h"
#include <string.h>
#include <stdint.h>
#include <assert.h>

CAVISource::CAVISource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	char pBuffer[5];
	size_t nOffset, nMovieBegin;
	int nSize, nValue;
	int i;

	m_BitmapInfo.pColorTable = NULL;
	memset(&m_pStreamPos, 0, sizeof(m_pStreamPos));
	m_nDuration = 0;
	memset(&m_AVIInfo, 0, sizeof(AVIInfo));
	memset(m_pTypes, 0, sizeof(m_pTypes));
	memset(m_pStreamTypes, 0, sizeof(m_pStreamTypes));

	pBuffer[4] = '\0';
	SourceReadData(pBuffer, 4);
	if (strcmp(pBuffer, "RIFF") != 0)  //RIFF
		return;
	m_nFileSize = SourceReadLittleEndian(4);  //RIFF size
	SourceReadData(pBuffer, 4);  //chunk id
	if (strcmp(pBuffer, "AVI ") != 0)  //not AVI chunk
		return;

	while (SourceReadData(pBuffer, 4) == 4)  //chunk id
	{
		nSize = SourceReadLittleEndian(4);  //chunk size
		nOffset = SourceGetPosition();  //remember current position

		if (strcmp(pBuffer, "LIST") == 0)  //LIST chunk
		{
			SourceReadData(pBuffer, 4);  //subchunk id
			if (strcmp(pBuffer, "hdrl") == 0)  //header chunk
			{
				SourceReadData(pBuffer, 4);  //header id
				if (strcmp(pBuffer, "avih") != 0)  //not AVI header
					return;
				SourceSeekData(4, SEEK_CUR);  //size
				m_nMicrosecondsPerFrame = SourceReadLittleEndian(4);  //microseconds per frame
				m_AVIInfo.fFrameRate = m_nMicrosecondsPerFrame == 0? 0.0f : 1000000.0f / m_nMicrosecondsPerFrame;  //frames per second
				SourceSeekData(4, SEEK_CUR);  //max bytes per second
				SourceSeekData(4, SEEK_CUR);  //reserved
				SourceSeekData(4, SEEK_CUR);  //flag
				m_nFrameNum = SourceReadLittleEndian(4);  //number of frames
				SourceSeekData(4, SEEK_CUR);  //initial frames
				nValue = SourceReadLittleEndian(4);  //number of streams
				//m_nOutputNum = nValue;
				SourceSeekData(4, SEEK_CUR);  //suggested buffer size
				m_BitmapInfo.nWidth = SourceReadLittleEndian(4);  //video width
				m_BitmapInfo.nHeight = SourceReadLittleEndian(4);  //video height
				SourceSeekData(4, SEEK_CUR);  //scale
				SourceSeekData(4, SEEK_CUR);  //rate
				SourceSeekData(4, SEEK_CUR);  //start
				SourceSeekData(4, SEEK_CUR);  //length
				continue;
			}  //hdrl
			else if (strcmp(pBuffer, "strl") == 0)  //stream chunk
			{
				SourceReadData(pBuffer, 4);
				if (strcmp(pBuffer, "strh") == 0)  //stream header
				{
					SourceSeekData(4, SEEK_CUR);
					SourceReadData(pBuffer, 4);
					if (strcmp(pBuffer, "vids") == 0)  //fccType: video
					{
						//if (m_AVIInfo.pVideoFCC[0] == '\0' && m_AVIInfo.nCompression == 0)  //first video
						{
							SourceReadData(m_AVIInfo.pVideoFCC, 4);  //fccHandler
							SourceSeekData(48, SEEK_CUR);
							do
							{
								SourceReadData(pBuffer, 4);
							} while(strcmp(pBuffer, "strf") != 0);  //stream format
							SourceSeekData(4, SEEK_CUR);  //size
							SourceSeekData(4, SEEK_CUR);  //size
							SourceSeekData(4, SEEK_CUR);  //width
							SourceSeekData(4, SEEK_CUR);  //height
							SourceSeekData(2, SEEK_CUR);  //planes
							nValue = SourceReadLittleEndian(2);
							m_BitmapInfo.nBitsPerPixel = nValue;  //bits per pixel
							m_BitmapInfo.nCompression = SourceReadLittleEndian(4);  //compression
							SourceSeekData(4, SEEK_CUR);  //size image
							SourceSeekData(4, SEEK_CUR);  //x pels per meter
							SourceSeekData(4, SEEK_CUR);  //y pels per meter
							m_BitmapInfo.nColorUsed = SourceReadLittleEndian(4);  //color used
							SourceSeekData(4, SEEK_CUR);
							if (m_BitmapInfo.nColorUsed != 0)
							{
								delete[] m_BitmapInfo.pColorTable;
								m_BitmapInfo.pColorTable = new unsigned char[m_BitmapInfo.nColorUsed * 4];
								SourceReadData(m_BitmapInfo.pColorTable, m_BitmapInfo.nColorUsed * 4);  //color table
							}
							m_pTypes[m_nOutputNum] = VideoType(m_BitmapInfo.nCompression);
							m_pStreamTypes[m_nOutputNum] = STREAM_VIDEO;
							++m_nOutputNum;
						}
					}
					else if (strcmp(pBuffer, "auds") == 0)  //fccType: audio
					{
						//if (m_AVIInfo.pAudioFCC[0] == '\0' && m_AVIInfo.nFormatTag == 0)  //first audio
						{
							SourceReadData(m_AVIInfo.pAudioFCC, 4);  //fccHandler
							SourceSeekData(48, SEEK_CUR);
							do
							{
								SourceReadData(pBuffer, 4);
							} while(strcmp(pBuffer, "strf") != 0);  //stream format
							SourceSeekData(4, SEEK_CUR);  //size
							m_WAVInfo.nFormatTag = SourceReadLittleEndian(2);  //format tag
							m_WAVInfo.nChannel = SourceReadLittleEndian(2);  //channels
							m_WAVInfo.nSampleRate = SourceReadLittleEndian(4);  //samples per second
							SourceSeekData(4, SEEK_CUR);  //average bytes per second
							SourceSeekData(2, SEEK_CUR);  //block align
							m_WAVInfo.nBitsPerSample = SourceReadLittleEndian(2);  //bits per sample
							m_nBytesPerSecond = m_WAVInfo.nSampleRate * (m_WAVInfo.nBitsPerSample >> 3) * m_WAVInfo.nChannel;
							m_pTypes[m_nOutputNum] = AudioType(m_WAVInfo.nFormatTag);
							m_pStreamTypes[m_nOutputNum] = STREAM_AUDIO;
							++m_nOutputNum;
						}
					}
					else if (strcmp(pBuffer, "iavs") == 0)  //fccType: interleaved audio and video stream
					{
						SourceReadData(m_AVIInfo.pVideoFCC, 4);  //fccHandler
						SourceSeekData(4, SEEK_CUR);  //flags
						SourceSeekData(2, SEEK_CUR);  //priority
						SourceSeekData(2, SEEK_CUR);  //language
						SourceSeekData(4, SEEK_CUR);  //initial frames
						SourceSeekData(4, SEEK_CUR);  //scale
						SourceSeekData(4, SEEK_CUR);  //rate
						SourceSeekData(4, SEEK_CUR);  //start
						SourceSeekData(4, SEEK_CUR);  //length
						SourceSeekData(4, SEEK_CUR);  //suggested buffer size
						SourceSeekData(4, SEEK_CUR);  //quality
						SourceSeekData(4, SEEK_CUR);  //sample size
						SourceSeekData(8, SEEK_CUR);  //rcFrame
						SourceReadData(pBuffer, 4);
						if (strcmp(pBuffer, "strf") == 0)
						{
							nValue = SourceReadLittleEndian(4);  //length
							SourceSeekData(nValue, SEEK_CUR);
						}
						if (m_BitmapInfo.nWidth == 0 && m_BitmapInfo.nHeight == 0)
						{
							m_BitmapInfo.nWidth = 720;
							m_BitmapInfo.nHeight = m_AVIInfo.fFrameRate == 25.0f? 576 : 480;
						}
						m_pTypes[m_nOutputNum++] = MEDIA_TYPE_UNKNOWN;
					}
				}  //strh
			}  //strl
			else if (strcmp(pBuffer, "movi") == 0)  //movie chunk
				nMovieBegin = SourceGetPosition();
			else if (strcmp(pBuffer, "idx1") == 0)  //index chunk
				;
		}  //LIST

		SourceSeekData(nSize - (SourceGetPosition() - nOffset), SEEK_CUR);  //skip rest of the chunk
	}  //while

	if (m_nOutputNum > 0)
	{
		m_nDuration = m_AVIInfo.fFrameRate == 0.0f? 0 : (int)(1000.0f * m_nFrameNum / m_AVIInfo.fFrameRate);
		for (i = 0; i < m_nOutputNum; i++)
			m_pStreamPos[i].nHeadPos = nMovieBegin;
		SeekTime(0);
	}
}

CAVISource::~CAVISource()
{
	delete[] m_BitmapInfo.pColorTable;
}

MediaType CAVISource::GetSourceType(void)
{
	return MEDIA_TYPE_AVI;
}

bool CAVISource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(AVIInfo))
		return false;

	m_AVIInfo.nSize = sizeof(AVIInfo);
	memcpy(pInfo, &m_AVIInfo, m_AVIInfo.nSize);
	return true;
}

MediaType CAVISource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : m_pTypes[nIndex];
}

bool CAVISource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	PCMInfo *pPCMInfo;
	char str[50];
	int nLen;
	unsigned long nCompression;

	if (nIndex >= m_nOutputNum)
		return false;

	switch (m_pTypes[nIndex])
	{
		case MEDIA_TYPE_BITMAP:
			if (pInfo->nSize >= sizeof(BitmapInfo))
			{
				m_BitmapInfo.nSize = sizeof(BitmapInfo);
				memcpy(pInfo, &m_BitmapInfo, m_BitmapInfo.nSize);
				return true;
			}
			break;
		case MEDIA_TYPE_PCM:
			if (pInfo->nSize >= sizeof(PCMInfo))
			{
				memcpy(pInfo, &m_WAVInfo, sizeof(AudioInfo));
				pPCMInfo = (PCMInfo *)pInfo;
				pPCMInfo->nSize = sizeof(PCMInfo);
				switch (m_WAVInfo.nFormatTag)
				{
					case 0:
					case 1:
						pPCMInfo->nFormat = PCM_LINEAR;
						break;
					case 3:
						pPCMInfo->nFormat = PCM_FLOAT;
						break;
					case 6:
						pPCMInfo->nFormat = PCM_ALAW;
						break;
					case 7:
						pPCMInfo->nFormat = PCM_MULAW;
						break;
					default:
						pPCMInfo->nFormat = PCM_UNKNOWN;
						break;
				}
				pPCMInfo->nBitsPerSample = m_WAVInfo.nBitsPerSample;
				pPCMInfo->nByteOrder = PCM_LITTLE_ENDIAN;
				pPCMInfo->bSigned = m_WAVInfo.nBitsPerSample > 8;
				return true;
			}
			break;
		case MEDIA_TYPE_UNKNOWN_AUDIO:
			sprintf(str, "Audio format tag = %ld", m_WAVInfo.nFormatTag);
			nLen = strlen(str) + 1;
			if (pInfo->nSize >= sizeof(MediaInfo) + nLen)
			{
				pInfo->nSize = sizeof(MediaInfo) + nLen;
				strcpy(((TextInfo *)pInfo)->pText, str);
				return true;
			}
			break;
		case MEDIA_TYPE_UNKNOWN_VIDEO:
			nCompression = m_BitmapInfo.nCompression;
			if (nCompression < 0x20000000)
				sprintf(str, "Video compression = 0x%lX", nCompression);
			else
				sprintf(str, "Video compression = 0x%lX (%c%c%c%c)", nCompression, (char)(nCompression & 0xFF), (char)((nCompression >> 8) & 0xFF), (char)((nCompression >> 16) & 0xFF), (char)((nCompression >> 24) & 0xFF));
			nLen = strlen(str) + 1;
			if (pInfo->nSize >= sizeof(MediaInfo) + nLen)
			{
				pInfo->nSize = sizeof(MediaInfo) + nLen;
				strcpy(((TextInfo *)pInfo)->pText, str);
				return true;
			}
			break;
		default:
			break;
	}
	return false;
}

int CAVISource::GetOutputTime(int nIndex)
{
	CMediaSource *pSource;

	if (nIndex >= m_nOutputNum)
		return 0;

	pSource = GetOutputSource(nIndex);
	return pSource == NULL? m_pStreamPos[nIndex].nTime : pSource->GetOutputTime(0);
}

size_t CAVISource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_pStreamPos[nIndex].nRelPos;
}

int CAVISource::ReadData(int nIndex, void *pData, int nSize)
{
	int nResult;
	StreamPos *pStreamPos;
	int nTotal;
	char chunkID[5];
	int64_t n64;

	if (nIndex >= m_nOutputNum)
		return 0;

	nResult = 0;
	pStreamPos = m_pStreamPos + nIndex;
	SourceSeekData(pStreamPos->nAbsPos, SEEK_SET);  //seek to previous position
	if (pStreamPos->nRelPos >= pStreamPos->nSize)
		for (;;)
		{
			nTotal = NextChunk(chunkID);  //find next chunk
			if (chunkID[0] == '\0')  //there is no chunk
				break;
			if (chunkID[1] - '0' == nIndex)  //found chunk
			{
				pStreamPos->nSize += nTotal;
				if (m_pStreamTypes[nIndex] == STREAM_VIDEO)  //video chunk
				{
					n64 = m_nFrameIndex;
					n64 *= m_nMicrosecondsPerFrame;
					n64 /= 1000;
					pStreamPos->nTime = (int)n64;  //milliseconds
					++m_nFrameIndex;
				}
				break;
			}
			else
				SourceSeekData(nTotal, SEEK_CUR);  //skip chunk
		}

	if (pStreamPos->nRelPos < pStreamPos->nSize)  //there are available data
	{
		if (nSize > pStreamPos->nSize - pStreamPos->nRelPos)
			nSize = (int)(pStreamPos->nSize - pStreamPos->nRelPos);
		nSize = SourceReadData(pData, nSize);  //read whole chunk
		pStreamPos->nRelPos += nSize;
		nResult = nSize;
	}
	pStreamPos->nAbsPos = SourceGetPosition();  //remember current position

	return nResult;
}

bool CAVISource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	StreamPos *pPos;
	int nSize;

	if (nIndex >= m_nOutputNum || nFrom == SEEK_END)
		return false;

	pPos = m_pStreamPos + nIndex;
	if (nFrom == SEEK_SET)
		if (nOffset >= pPos->nRelPos)
			nOffset -= pPos->nRelPos;
		else
		{
			pPos->nAbsPos = pPos->nHeadPos;
			pPos->nRelPos = 0;
			pPos->nSize = 0;
			pPos->nTime = 0;
		}
	while (nOffset > 0)
	{
		nSize = ReadData(nIndex, NULL, nOffset);
		if (nSize == 0)
			break;
		nOffset -= nSize;
	}
	return nOffset == 0;
}

int CAVISource::GetDuration(void)
{
	return m_nDuration;
}

int CAVISource::SeekTime(int nTime)
{
	int i;
	CMediaSource *pSource;
	StreamPos *pPos;

	for (i = 0; i < m_nOutputNum; i++)
	{
		if (m_ppOutputSources != NULL)
		{
			pSource = GetOutputSource(i);
			if (pSource != NULL)
			{
				pSource->SeekTime(nTime);
				continue;
			}
		}

		pPos = m_pStreamPos + i;
		pPos->nAbsPos = pPos->nHeadPos;
		pPos->nRelPos = 0;
		pPos->nSize = 0;
		pPos->nTime = 0;
	}
	m_nFrameIndex = 0;
	return 0;
}

int CAVISource::NextChunk(char *chunkID)
{
	int nSize;

	chunkID[4] = '\0';
	if (SourceGetPosition() >= m_nFileSize || SourceReadData(chunkID, 4) < 4)  //chunk id
	{
		chunkID[0] = '\0';
		return 0;
	}
	assert(!(chunkID[0] == '\0' || strcmp(chunkID + 1, "JUN") == 0 || strcmp(chunkID + 1, "01w") == 0 || strcmp(chunkID + 1, "00d") == 0));
	/*if (chunkID[0] == '\0' || strcmp(chunkID + 1, "JUN") == 0 || strcmp(chunkID + 1, "01w") == 0 || strcmp(chunkID + 1, "00d") == 0)  //some files have error?
	{
		chunkID[0] = chunkID[1];
		chunkID[1] = chunkID[2];
		chunkID[2] = chunkID[3];
		chunkID[3] = SourceReadData();
		if (chunkID[3] == '\0')
		{
			chunkID[0] = '\0';
			return 0;
		}
	}*/
	nSize = SourceReadLittleEndian(4);  //size
	if (chunkID[0] == 'i' && chunkID[1] == 'x')
	{
		SourceSeekData(8, SEEK_CUR);
		SourceReadData(chunkID, 4);
		nSize -= 12;
	}
	return nSize;
}
