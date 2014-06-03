#include "MIDISource.h"
#include <string.h>
#include <stdint.h>
#include <assert.h>

CMIDISource::CMIDISource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	char str[5];
	int i;
	StreamPos *pPos;
	MIDITrack *pTrack;
	TempoEvent *pEvent;
	unsigned char pData[1024];
	int nSize;

	m_pStreamPos = NULL;
	m_pTracks = NULL;
	m_pTempoEvents = NULL;
	m_nDuration = 0;

	str[4] = '\0';
	SourceReadData(str, 4);
	if (strcmp(str, "MThd") != 0)
		return;
	if (SourceReadBigEndian(4) != 6)  //header length
		return;

	m_MIDIInfo.nFormat = SourceReadBigEndian(2);
	m_nOutputNum = SourceReadBigEndian(2);  //number of track
	m_nTicksPerQuarterNote = SourceReadBigEndian(2);
	m_pStreamPos = new StreamPos[m_nOutputNum];
	m_pTracks = new MIDITrack[m_nOutputNum];
	for (i = 0; i < m_nOutputNum; i++)
	{
		pPos = m_pStreamPos + i;
		SourceReadData(str, 4);  //track id
		pPos->nSize = SourceReadBigEndian(4);  //track length
		if (strcmp(str, "MTrk") == 0)
			pPos->nHeadPos = SourceGetPosition();
		else
			pPos->nHeadPos = 0;  //invalid track
		SourceSeekData(pPos->nSize, SEEK_CUR);  //skip to next track
	}

	if (m_nOutputNum == 0)
		return;

	pTrack = m_pTracks;  //first track;
	m_pTempoEvents = new TempoEvent[m_pStreamPos[0].nSize / 6];  //minimum size of a change tempo event is 6
	m_nTempoEventNum = 1;
	pEvent = m_pTempoEvents;
	pEvent->nTempo = 500000;  //default tempo in microseconds per quarter note
	pEvent->nTicks = 0;
	pEvent->nTime = 0;

	SeekTime(0);
	for (;;)
	{
		nSize = ReadData(0, pData, sizeof(pData));
		if (nSize == 0)
			break;

		if (pData[0] == 0xFF && pData[1] == 0x51)  //set tempo
		{
			pEvent = m_pTempoEvents + m_nTempoEventNum;
			pEvent->nTempo = (pData[2] << 16) | (pData[3] << 8) | pData[4];  //microseconds per quarter note
			pEvent->nTicks = pTrack->nTicks;
			pEvent->nTime = TicksToTime(pTrack->nTicks);
			++m_nTempoEventNum;
		}
	}

	m_nDuration = m_pStreamPos[0].nTime;
	for (i = 1; i < m_nOutputNum; i++)  //for each track except the first one
	{
		do
		{
			nSize = ReadData(i, pData, sizeof(pData));
		} while (nSize > 0);  //until the last event of this track
		if (m_pStreamPos[i].nTime > m_nDuration)
			m_nDuration = m_pStreamPos[i].nTime;
	}
	SeekTime(0);
}

CMIDISource::~CMIDISource()
{
	delete[] m_pStreamPos;
	delete[] m_pTracks;
	delete[] m_pTempoEvents;
}

MediaType CMIDISource::GetSourceType(void)
{
	return MEDIA_TYPE_MIDI;
}

bool CMIDISource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(MIDIInfo))
		return false;

	m_MIDIInfo.nSize = sizeof(MIDIInfo);
	memcpy(pInfo, &m_MIDIInfo, m_MIDIInfo.nSize);
	return true;
}

MediaType CMIDISource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : MEDIA_TYPE_MIDIEVENT;
}

bool CMIDISource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	if (nIndex >= m_nOutputNum || pInfo->nSize < sizeof(MediaInfo))
		return false;

	pInfo->nSize = sizeof(MediaInfo);
	return true;
}

size_t CMIDISource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_pStreamPos[nIndex].nRelPos;
}

int CMIDISource::ReadData(int nIndex, void *pData, int nSize)
{
	StreamPos *pPos;
	unsigned char *pBuffer;
	MIDITrack *pTrack;
	int nStatus;
	int nValue;

	if (nIndex >= m_nOutputNum)
		return 0;

	pPos = m_pStreamPos + nIndex;
	if (pPos->nHeadPos == 0)  //invalid track
		return 0;
	if (pPos->nRelPos >= pPos->nSize)
		return 0;

	SourceSeekData(pPos->nAbsPos, SEEK_SET);
	pBuffer = (unsigned char *)pData;
	pTrack = m_pTracks + nIndex;

	pTrack->nTicks += ReadVarLen();  //delta time
	nStatus = SourceReadData();
	if (nStatus >= 0xF0 && nStatus < 0xFF)  //system exclusive event
	{
		pBuffer[0] = nStatus;
		nValue = ReadVarLen();
		SourceReadData(pBuffer + 1, nValue);
		assert(nSize >= 1 + nValue);
		if (nSize > 1 + nValue)
			nSize = 1 + nValue;
	}
	else
	{
		if ((nStatus & 0x80) == 0)  //running status
		{
			pBuffer[0] = pTrack->nStatus;
			pBuffer[1] = nStatus;
		}
		else  //MIDI event or meta event
		{
			pTrack->nStatus = nStatus;
			pBuffer[0] = nStatus;
			pBuffer[1] = SourceReadData();
		}
		switch (pBuffer[0] & 0xF0)
		{
			case 0x80:
			case 0x90:
			case 0xA0:
			case 0xB0:
			case 0xE0:
				pBuffer[2] = SourceReadData();
				nSize = 3;
				break;
			case 0xC0:
			case 0xD0:
				nSize = 2;
				break;
			case 0xF0:
				nValue = ReadVarLen();  //data length
				SourceReadData(pBuffer + 2, nValue);
				assert(nSize >= 2 + nValue);
				if (nSize > 2 + nValue)
					nSize = 2 + nValue;
				break;
			default:
				nSize = 0;
				break;
		}  //switch
	}  //else

	pPos->nAbsPos = SourceGetPosition();
	pPos->nRelPos = pPos->nAbsPos - pPos->nHeadPos;
	pPos->nTime = TicksToTime(pTrack->nTicks);
	if (pPos->nTime < m_nSeekTime)  //event before seektime
	{
		pPos->nTime = 0;  //render immediately
		if (pBuffer[0] >= 0x80 && pBuffer[0] <= 0x9F)  //note on or note off
			pBuffer[0] = 0x00;  //invalid event, should not be renderered
	}

	return nSize;
}

bool CMIDISource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;  //meaningless
}

int CMIDISource::GetOutputTime(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_pStreamPos[nIndex].nTime;
}

int CMIDISource::GetDuration(void)
{
	return m_nDuration;  //not implemented
}

int CMIDISource::SeekTime(int nTime)
{
	int i;
	StreamPos *pPos;
	MIDITrack *pTrack;

	if (nTime > m_nDuration)
		nTime = m_nDuration;
	for (i = 0; i < m_nOutputNum; i++)
	{
		pPos = m_pStreamPos + i;
		pPos->nAbsPos = pPos->nHeadPos;
		pPos->nRelPos = 0;
		pPos->nTime = nTime;
		pTrack = m_pTracks + i;
		pTrack->nTicks = 0;
		pTrack->nStatus = 0;
	}
	m_nSeekTime = nTime;
	return nTime;
}

int CMIDISource::ReadVarLen(void)
{
	int nValue;
	int nByte;

	nValue = 0;
	do
	{
		nByte = SourceReadData();
		nValue = (nValue << 7) | (nByte & 0x7F);
	} while (nByte & 0x80);
	return nValue;
}

int CMIDISource::TicksToTime(int nTick)
{
	int i;
	TempoEvent *pEvent;
	int64_t n64;

	for (i = 0; i < m_nTempoEventNum; i++)
		if (nTick < m_pTempoEvents[i].nTicks)
			break;
	pEvent = m_pTempoEvents + (i - 1);
	n64 = nTick - pEvent->nTicks;
	n64 *= pEvent->nTempo;
	n64 /= m_nTicksPerQuarterNote * 1000;
	n64 += pEvent->nTime;
	return (int)n64;
}
