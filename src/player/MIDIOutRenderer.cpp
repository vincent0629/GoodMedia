#include "MIDIOutRenderer.h"

#define CHANNEL_NUM 16

CMIDIOutRenderer::CMIDIOutRenderer() : CMediaRenderer()
{
	m_hmo = NULL;
	m_pLyrics[0] = '\0';
}

CMIDIOutRenderer::~CMIDIOutRenderer()
{
	Close();
}

bool CMIDIOutRenderer::Open(MediaInfo *pInfo)
{
	int i;

	Close();

	if (midiOutGetNumDevs() == 0)  //there is no MIDI device
		return false;
	if (midiOutOpen(&m_hmo, MIDI_MAPPER, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)  //open device failed
		return false;

	for (i = 0; i < CHANNEL_NUM; i++)  //reset instrument for each channel
		midiOutShortMsg(m_hmo, 0xC0 | i);
	return true;
}

void CMIDIOutRenderer::Write(void *pBuffer, int nSize)
{
	unsigned char *pData;
	unsigned long nMsg;

	if (m_hmo == NULL || nSize < 2)
		return;

	pData = (unsigned char *)pBuffer;
	if (pData[0] >= 0x80 && pData[0] <= 0xEF)  //MIDI event
	{
		if (nSize == 2)
			nMsg = (pData[1] << 8) | pData[0];
		else
			nMsg = (pData[2] << 16) | (pData[1] << 8) | pData[0];
		midiOutShortMsg(m_hmo, nMsg);
	}
	else if (pData[0] == 0xFF && pData[1] == 0x01)  //text event
	{
		if (pData[2] != '@')
		{
			if (pData[2] == '/' || pData[2] == '\\')
			{
				memcpy(m_pLyrics, pData + 3, nSize - 3);
				m_pLyrics[nSize - 3] = '\0';
			}
			else
				strncat(m_pLyrics, (char *)pData + 2, nSize - 2);
			FireRenderedEvent(m_pLyrics, strlen(m_pLyrics));
		}
	}
}

void CMIDIOutRenderer::Flush(bool bWait)
{
	int i;

	if (m_hmo == NULL)
		return;

	for (i = 0; i < CHANNEL_NUM; i++)  //note off for each channel
		midiOutShortMsg(m_hmo, 0x78B0 | i);
	m_pLyrics[0] = '\0';
}

void CMIDIOutRenderer::Close(void)
{
	if (m_hmo == NULL)
		return;

	midiOutReset(m_hmo);
	midiOutClose(m_hmo);
	m_hmo = NULL;
	m_pLyrics[0] = '\0';
}
