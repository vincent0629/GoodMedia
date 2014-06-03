#include "MPEGAudioSource.h"
#include <string.h>
#include <stdint.h>

#define LOOK_FRAME 20
static int versionTable[4] = {0, -1, 2, 1};  //2.5, reserved, 2, 1
static int layerTable[4] = {-1, 3, 2, 1};
static int bitRateIndexTable[3][3] = {{3, 4, 4}, {0, 1, 2}, {3, 4, 4}};  //MPEG-2.5, MPEG-1, MPEG-2
static int bitRateTable[5][16] = {
	{0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, -1}, //MPEG-1 LAYER 1
	{0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, -1},  //MPEG-1 LAYER 2
	{0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, -1},  //MPEG-1 LAYER 3
	{0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, -1},  //MPEG-2/2.5 LAYER 1
	{0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, -1}};  //MPEG-2/2.5 LAYER 2/3
static int sampleRateTable[3][4] = {
	{11025, 12000, 8000, -1},  //MPEG-2.5
	{44100, 48000, 32000, -1},  //MPEG-1
	{22050, 24000, 16000, -1}};  //MPEG-2
static MPEGAudioChannel channelModeTable[] = {MPEGAUDIO_STEREO, MPEGAUDIO_JOINT_STEREO, MPEGAUDIO_DUAL_CHANNEL, MPEGAUDIO_SINGLE_CHANNEL};

CMPEGAudioSource::CMPEGAudioSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	int i, j;
	int nFramePos[LOOK_FRAME], nFrameSize[LOOK_FRAME];
	unsigned char pHeader[LOOK_FRAME][4];
	bool bDone;

	m_StreamPos.nHeadPos = -1;
	m_nFrameIndex = 0;
	bDone = false;
	for (i = 0; i < LOOK_FRAME && !bDone; i++)
	{
		nFrameSize[i] = NextFrame(true);
		if (nFrameSize[i] == 0)
			break;
		nFramePos[i] = SourceGetPosition() - HEADER_SIZE;
		m_StreamPos.nHeadPos = 0;

		memcpy(pHeader[i], m_pHeader, 4);
		for (j = 0; j < i; j++)
			if (nFramePos[j] + nFrameSize[j] == nFramePos[i])
			{
				m_StreamPos.nHeadPos = nFramePos[j];
				m_MPEGAudioInfo.nVersion = versionTable[(pHeader[j][1] >> 3) & 3];
				m_MPEGAudioInfo.nLayer = layerTable[(pHeader[j][1] >> 1) & 3];
				m_MPEGAudioInfo.nSampleRate = sampleRateTable[m_MPEGAudioInfo.nVersion][(pHeader[j][2] >> 2) & 3];
				m_MPEGAudioInfo.nChannelMode = channelModeTable[pHeader[j][3] >> 6];
				bDone = true;
				break;
			}
	}
	if (!bDone)
		return;

	m_nOutputNum = 1;
	m_MPEGAudioInfo.nChannel = m_MPEGAudioInfo.nChannelMode == MPEGAUDIO_SINGLE_CHANNEL? 1 : 2;
	if (m_MPEGAudioInfo.nVersion == 2 && m_MPEGAudioInfo.nLayer == 3)
		m_nSamplesPerFrame = 576;
	else if (m_MPEGAudioInfo.nLayer == 1)
		m_nSamplesPerFrame = 384;
	else
		m_nSamplesPerFrame = 1152;
	m_nDuration = 0;

	SeekTime(0);
}

CMPEGAudioSource::~CMPEGAudioSource()
{
}

MediaType CMPEGAudioSource::GetSourceType(void)
{
	return MEDIA_TYPE_MPEGAUDIO;
}

bool CMPEGAudioSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(MPEGAudioInfo))
		return false;

	m_MPEGAudioInfo.nSize = sizeof(MPEGAudioInfo);
	memcpy(pInfo, &m_MPEGAudioInfo, m_MPEGAudioInfo.nSize);
	return true;
}

MediaType CMPEGAudioSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_MPEGAUDIO;
}

bool CMPEGAudioSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	return nIndex >= m_nOutputNum? false : GetSourceInfo(pInfo);
}

size_t CMPEGAudioSource::GetPosition(int nIndex)
{
	return 0;  //not implemented
}

int CMPEGAudioSource::ReadData(int nIndex, void *pData, int nSize)
{
	if (nIndex >= m_nOutputNum)
		return 0;

	nSize = NextFrame(false);
	if (nSize == 0)
		return 0;

	if (pData != NULL)
	{
		memcpy(pData, m_pHeader, HEADER_SIZE);
		SourceReadData((char *)pData + HEADER_SIZE, nSize - HEADER_SIZE);
	}
	else
		SourceSeekData(nSize - HEADER_SIZE, SEEK_CUR);  //skip this frame
	return nSize;
}

bool CMPEGAudioSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;  //not implemented
}

int CMPEGAudioSource::GetOutputTime(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_StreamPos.nTime;
}

int CMPEGAudioSource::GetDuration(void)
{
	size_t nPos;
	int i, nFrameSize, nSumSize;

	if (m_nOutputNum == 0)
		return 0;

	if (m_nDuration == 0)
	{
		nPos = SourceGetPosition();
		SourceSeekData(m_StreamPos.nHeadPos, SEEK_SET);
		nSumSize = 0;
		for (i = 0; i < 20; i++)  //estimate using 20 frames
			if ((nFrameSize = NextFrame(false)) != 0)
			{
				SourceSeekData(nFrameSize, SEEK_CUR);  //skip a frame
				nSumSize += nFrameSize;
			}
			else
				break;
		SourceSeekData(nPos, SEEK_SET);
		m_nDuration = nSumSize == 0? 0 : (int)(SourceGetSize() * i / nSumSize * m_nSamplesPerFrame * 1000 / m_MPEGAudioInfo.nSampleRate);
	}
	return m_nDuration;
}

int CMPEGAudioSource::SeekTime(int nTime)
{
	int nIndex, nFrameSize;

	if (m_nOutputNum == 0)
		return 0;

	nIndex = (int)((int64_t)nTime * m_MPEGAudioInfo.nSampleRate / (m_nSamplesPerFrame * 1000));
	if (nIndex < m_nFrameIndex)
	{
		m_nFrameIndex = 0;
		SourceSeekData(m_StreamPos.nHeadPos, SEEK_SET);
		m_StreamPos.nRelPos = 0;
		m_StreamPos.nTime = 0;
	}
	while (m_nFrameIndex < nIndex)
		if ((nFrameSize = NextFrame(false)) != 0)
		{
			SourceSeekData(nFrameSize - HEADER_SIZE, SEEK_CUR);  //skip a frame
			m_StreamPos.nRelPos += nFrameSize;
		}
		else
			break;
	return m_StreamPos.nTime;
}

int CMPEGAudioSource::NextFrame(bool bSearch)
{
	int i, nValue, nSize;
	int nFlags, nVersion, nLayer, nProtection, nBitRate, nSampleRate, nPadding;

	if (SourceReadData(m_pHeader, 3) < 3)
		return 0;

	if (m_StreamPos.nHeadPos == -1)
	{
		if (memcmp(m_pHeader, "ID3", 3) == 0)  //ID3v2
		{
			SourceSeekData(2, SEEK_CUR);  //version
			nFlags = SourceReadData();  //flags
			nValue = 0;
			for (i = 0; i < 4; i++)
				nValue = (nValue << 7) | (SourceReadData() & 0x7F);  //size
			SourceSeekData(nValue, SEEK_CUR);  //ID3v2
			if (SourceReadData(m_pHeader, 3) < 3)
				return 0;
		}
	}

	for (i = bSearch? 4096 : 1; i != 0; i--)  //4096 is max frame size?
	{
		if (m_pHeader[0] == 0xFF && (m_pHeader[1] & 0xE0) == 0xE0)  //sync word
		{
			nVersion = versionTable[(m_pHeader[1] >> 3) & 3];
			nLayer = layerTable[(m_pHeader[1] >> 1) & 3];
			if (nVersion >= 0 && nLayer > 0)
			{
				nProtection = m_pHeader[1] & 1;
				nValue = bitRateIndexTable[nVersion][nLayer - 1];
				nBitRate = bitRateTable[nValue][m_pHeader[2] >> 4] * 1000;
				nSampleRate = sampleRateTable[nVersion][(m_pHeader[2] >> 2) & 3];
				nPadding = (m_pHeader[2] >> 1) & 1;
				if (nBitRate > 0 && nSampleRate > 0)
					if (bSearch || (nVersion == m_MPEGAudioInfo.nVersion && nLayer == m_MPEGAudioInfo.nLayer && nSampleRate == m_MPEGAudioInfo.nSampleRate))
					{
						SourceReadData(m_pHeader + 3, 1);
						break;
					}
			}
		}
		m_pHeader[0] = m_pHeader[1];
		m_pHeader[1] = m_pHeader[2];
		if (SourceReadData(m_pHeader + 2, 1) < 1)
			return 0;
	}
	if (i == 0)
		return 0;

	if (nVersion == 2 && nLayer == 3)
		nSize = 72 * nBitRate / nSampleRate + nPadding;
	else if (nLayer == 1)
		nSize = (12 * nBitRate / nSampleRate + nPadding) * 4;
	else
		nSize = 144 * nBitRate / nSampleRate + nPadding;

	m_StreamPos.nTime = (int)((int64_t)m_nFrameIndex * m_nSamplesPerFrame * 1000 / nSampleRate);
	++m_nFrameIndex;
	return nSize;
}
