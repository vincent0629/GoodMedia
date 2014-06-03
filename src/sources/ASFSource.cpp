#include "ASFSource.h"
#include "MediaInfo.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef struct
{
	unsigned long Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char Data4[8];
} GUID;

static bool GUIDEqual(const GUID *guid1, const GUID *guid2)
{
	return memcmp(guid1, guid2, sizeof(GUID)) == 0;
}

CASFSource::CASFSource(CMediaSource *pSource, int nIndex) : CMediaSource(pSource, nIndex)
{
	const GUID //top-level ASF object GUIDS
		ASF_Header_Object = {0x75B22630, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C}},
		ASF_Data_Object = {0x75B22636, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C}},
		//header Object GUIDs
		ASF_File_Properties_Object = {0x8CABDCA1, 0xA947, 0x11CF, {0x8E, 0xE4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65}},
		ASF_Stream_Properties_Object = {0xB7DC0791, 0xA9B7, 0x11CF, {0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65}},
		ASF_Header_Extension_Object = {0x5FBF03B5, 0xA92E, 0x11CF, {0x8E, 0xE3, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65}},
		ASF_Codec_List_Object = {0x86D15240, 0x311D, 0x11D0, {0xA3, 0xA4, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6}},
		ASF_Script_Command_Object = {0x1EFB1A30, 0x0B62, 0x11D0, {0xA3, 0x9B, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6}},
		ASF_Marker_Object = {0xF487CD01, 0xA951, 0x11CF, {0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65}},
		ASF_Bitrate_Mutual_Exclusion_Object = {0xD6E229DC, 0x35DA, 0x11D1, {0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE}},
		ASF_Error_Correction_Object = {0x75B22635, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C}},
		ASF_Content_Description_Object = {0x75B22633, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C}},
		ASF_Extended_Content_Description_Object = {0xD2D0A440, 0xE307, 0x11D2, {0x97, 0xF0, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50}},
		ASF_Content_Branding_Object = {0x2211B3FA, 0xBD23, 0x11D2, {0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E}},
		ASF_Stream_Bitrate_Properties_Object = {0x7BF875CE, 0x468D, 0x11D1, {0x8D, 0x82, 0x00, 0x60, 0x97, 0xC9, 0xA2, 0xB2}},
		ASF_Content_Encryption_Object = {0x2211B3FB, 0xBD23, 0x11D2, {0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E}},
		ASF_Extended_Content_Encryption_Object = {0x298AE614, 0x2622, 0x4C17, {0xB9, 0x35, 0xDA, 0xE0, 0x7E, 0xE9, 0x28, 0x9C}},
		ASF_Digital_Signature_Object = {0x2211B3FC, 0xBD23, 0x11D2, {0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E}},
		ASF_Padding_Object = {0x1806D474, 0xCADF, 0x4509, {0xA4, 0xBA, 0x9A, 0xAB, 0xCB, 0x96, 0xAA, 0xE8}},
		//stream properties object stream type GUIDs
		ASF_Audio_Media = {0xF8699E40, 0x5B4D, 0x11CF, {0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B}},
		ASF_Video_Media = {0xBC19EFC0, 0x5B4D, 0x11CF, {0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B}},
		ASF_Command_Media = {0x59DACFC0, 0x59E6, 0x11D0, {0xA3, 0xAC, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6}},
		ASF_JFIF_Media = {0xB61BE100, 0x5B4E, 0x11CF, {0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B}},
		ASF_Degradable_JPEG_Media = {0x35907DE0, 0xE415, 0x11CF, {0xA9, 0x17, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B}},
		ASF_File_Transfer_Media = {0x91BD222C, 0xF21C, 0x497A, {0x8B, 0x6D, 0x5A, 0xA8, 0x6B, 0xFC, 0x01, 0x85}},
		ASF_Binary_Media = {0x3AFB65E2, 0x47EF, 0x40F2, {0xAC, 0x2C, 0x70, 0xA9, 0x0D, 0x71, 0xD3, 0x43}};

	GUID guid;
	int nValue;
	int i, j, nSize, nObjs;
	int nTitleLen, nAuthorLen, nCopyrightLen, nDescriptionLen, nRatingLen;
	size_t nPos;
	int64_t n64;
	wchar_t wstr[256];
	char str[256];
	StreamData *pStream;
	AudioInfo *pAudioInfo;
	VideoInfo *pVideoInfo;

	m_pStreamPos = NULL;
	m_nDuration = 0;
	memset(&m_ASFInfo, 0, sizeof(ASFInfo));
	if (SourceReadData(&guid, 16) < 16)  //object id
		return;
	if (!GUIDEqual(&guid, &ASF_Header_Object))
		return;
	SourceSeekData(8, SEEK_CUR);  //object size
	nObjs = SourceReadLittleEndian(4);
	SourceSeekData(1, SEEK_CUR);  //reserved1
	if (SourceReadData() != 2)  //reserved2
		return;

	memset(m_pStreamData, 0, sizeof(m_pStreamData));
	for (i = 0; i < nObjs; i++)
	{
		SourceReadData(&guid, 16);  //object id
		SourceReadData(&nPos, 8);  //object size
		nPos = nPos - 24 + SourceGetPosition();

		if (GUIDEqual(&guid, &ASF_Stream_Properties_Object))
		{
			pStream = m_pStreamData + m_nOutputNum;
			SourceReadData(&guid, 16);  //stream type
			SourceSeekData(28, SEEK_CUR);
			nSize = SourceReadLittleEndian(4);  //error correction data length
			nValue = SourceReadLittleEndian(2);  //flags
			pStream->nStreamNumber = nValue & 0x7F;  //stream number
			SourceSeekData(4, SEEK_CUR);
			if (GUIDEqual(&guid, &ASF_Audio_Media))
			{
				pAudioInfo = new AudioInfo;
				pAudioInfo->nSize = sizeof(AudioInfo);
				pStream->nFormat = SourceReadLittleEndian(2);  //format tag
				pStream->type = AudioType(pStream->nFormat);
				pAudioInfo->nChannel = SourceReadLittleEndian(2);  //number of channels
				pAudioInfo->nSampleRate = SourceReadLittleEndian(4);  //samples per second
				SourceSeekData(6, SEEK_CUR);
				nValue = SourceReadLittleEndian(2);  //bits per sample
				nValue = SourceReadLittleEndian(2);  //codec specific data size
				SourceSeekData(nValue, SEEK_CUR);  //codec specific data
				pStream->pInfo = pAudioInfo;
				++m_nOutputNum;
			}
			else if (GUIDEqual(&guid, &ASF_Video_Media))
			{
				pVideoInfo = new VideoInfo;
				pVideoInfo->nSize = sizeof(VideoInfo);
				pVideoInfo->nWidth = SourceReadLittleEndian(4);  //encoded image width
				pVideoInfo->nHeight = SourceReadLittleEndian(4);  //encoded image height
				SourceSeekData(1, SEEK_CUR);  //reserved flags
				nValue = SourceReadLittleEndian(2);  //format data size
				SourceSeekData(16, SEEK_CUR);
				pStream->nFormat = SourceReadLittleEndian(4);  //compression id
				pStream->type = VideoType(pStream->nFormat);
				SourceSeekData(nValue - 20, SEEK_CUR);  //rest format data
				pStream->pInfo = pVideoInfo;
				++m_nOutputNum;
			}
		}
		else if (GUIDEqual(&guid, &ASF_File_Properties_Object))
		{
			SourceSeekData(40, SEEK_CUR);
			SourceReadData(&n64, 8);  //play duration
			m_nDuration = (int)(n64 / 10000);
			SourceSeekData(8, SEEK_CUR);  //send duration
			SourceReadData(&n64, 8);  //preroll
			if (m_nDuration > 0)
				m_nDuration -= (int)n64;
			SourceSeekData(4, SEEK_CUR);  //flags
			m_nPacketSize = SourceReadLittleEndian(4);  //minimum data packet size
			SourceSeekData(8, SEEK_CUR);
		}
		else if (GUIDEqual(&guid, &ASF_Content_Description_Object))
		{
			nTitleLen = SourceReadLittleEndian(2);  //title length
			nAuthorLen = SourceReadLittleEndian(2);  //author length
			nCopyrightLen = SourceReadLittleEndian(2);  //copyright length
			nDescriptionLen = SourceReadLittleEndian(2);  //description length
			nRatingLen = SourceReadLittleEndian(2);  //rating length
			if (nTitleLen > 0)
			{
				m_ASFInfo.pTitle = (wchar_t *)new char[nTitleLen];
				SourceReadData(m_ASFInfo.pTitle, nTitleLen);  //title
			}
			if (nAuthorLen > 0)
			{
				m_ASFInfo.pAuthor = (wchar_t *)new char[nAuthorLen];
				SourceReadData(m_ASFInfo.pAuthor, nAuthorLen);  //author
			}
			if (nCopyrightLen > 0)
			{
				m_ASFInfo.pCopyright = (wchar_t *)new char[nCopyrightLen];
				SourceReadData(m_ASFInfo.pCopyright, nCopyrightLen);  //copyright
			}
			if (nDescriptionLen > 0)
			{
				m_ASFInfo.pDescription = (wchar_t *)new char[nDescriptionLen];
				SourceReadData(m_ASFInfo.pDescription, nDescriptionLen);  //description
			}
			if (nRatingLen > 0)
			{
				m_ASFInfo.pRating = (wchar_t *)new char[nRatingLen];
				SourceReadData(m_ASFInfo.pRating, nRatingLen);  //rating
			}
		}
		else if (GUIDEqual(&guid, &ASF_Extended_Content_Description_Object))
		{
			nValue = SourceReadLittleEndian(2);  //content descriptors count
			for (j = nValue; j > 0; j--)
			{
				nValue = SourceReadLittleEndian(2);  //descriptor name length
				SourceReadData(wstr, nValue);  //descriptor name
				nValue = SourceReadLittleEndian(2);  //descriptor value data type
				if (nValue == 0)  //wide string
				{
					nValue = SourceReadLittleEndian(2);  //descriptor value length
					if (wcscmp(wstr, L"WM/AlbumTitle") == 0)
					{
						m_ASFInfo.pAlbum = (wchar_t *)new char[nValue];
						SourceReadData(m_ASFInfo.pAlbum, nValue);
					}
					else if (wcscmp(wstr, L"WM/Year") == 0)
					{
						m_ASFInfo.pYear = (wchar_t *)new char[nValue];
						SourceReadData(m_ASFInfo.pYear, nValue);
					}
					else if (wcscmp(wstr, L"WM/Genre") == 0)
					{
						m_ASFInfo.pGenre = (wchar_t *)new char[nValue];
						SourceReadData(m_ASFInfo.pGenre, nValue);
					}
					else
						SourceSeekData(nValue, SEEK_CUR);  //descriptor value
				}
				else
				{
					nValue = SourceReadLittleEndian(2);  //descriptor value length
					SourceSeekData(nValue, SEEK_CUR);  //descriptor value
				}
			}
		}
		else if (GUIDEqual(&guid, &ASF_Content_Encryption_Object))
		{
			nValue = SourceReadLittleEndian(4);  //secret data length
			SourceSeekData(nValue, SEEK_CUR);  //secret data
			nValue = SourceReadLittleEndian(4);  //protection type length
			SourceReadData(str, nValue);  //protection type
			m_ASFInfo.bDRM = strcmp(str, "DRM") == 0;
			nValue = SourceReadLittleEndian(4);  //key ID length
			m_ASFInfo.pKeyID = new char[nValue];
			SourceReadData(m_ASFInfo.pKeyID, nValue);  //key ID
			nValue = SourceReadLittleEndian(4);  //license URL length
			m_ASFInfo.pLicenseURL = new char[nValue];
			SourceReadData(m_ASFInfo.pLicenseURL, nValue);  //license URL
		}

		SourceSeekData(nPos, SEEK_SET);  //skip rest of object
	}  //for

	SourceReadData(&guid, 16);  //object id
	if (!GUIDEqual(&guid, &ASF_Data_Object))
	{
		m_nOutputNum = 0;
		return;
	}
	SourceSeekData(24, SEEK_CUR);  //object size, file id
	SourceReadData(&n64, 8);  //total data packets
	SourceSeekData(2, SEEK_CUR);  //reserved
	m_nPacketNum = (int)n64;

	m_pStreamPos = new StreamPos[m_nOutputNum];
	memset(m_pStreamPos, 0, m_nOutputNum * sizeof(StreamPos));
	for (i = 0; i < m_nOutputNum; i++)
		m_pStreamPos[i].nHeadPos = SourceGetPosition();
	SeekTime(0);
}

CASFSource::~CASFSource()
{
	int i;

	delete[] m_ASFInfo.pTitle;
	delete[] m_ASFInfo.pAuthor;
	delete[] m_ASFInfo.pCopyright;
	delete[] m_ASFInfo.pDescription;
	delete[] m_ASFInfo.pRating;
	delete[] m_ASFInfo.pAlbum;
	delete[] m_ASFInfo.pYear;
	delete[] m_ASFInfo.pGenre;
	delete[] m_ASFInfo.pKeyID;
	delete[] m_ASFInfo.pLicenseURL;

	delete[] m_pStreamPos;
	for (i = 0; i < m_nOutputNum; i++)
		delete[] m_pStreamData[i].pInfo;
}

MediaType CASFSource::GetSourceType(void)
{
	return MEDIA_TYPE_ASF;
}

bool CASFSource::GetSourceInfo(MediaInfo *pInfo)
{
	if (m_nOutputNum == 0 || pInfo->nSize < sizeof(ASFInfo))
		return false;

	m_ASFInfo.nSize = sizeof(ASFInfo);
	memcpy(pInfo, &m_ASFInfo, m_ASFInfo.nSize);
	return true;
}

MediaType CASFSource::GetOutputType(int nIndex)
{
	return nIndex >= m_nOutputNum? MEDIA_TYPE_UNKNOWN : m_pStreamData[nIndex].type;
}

bool CASFSource::GetOutputInfo(int nIndex, MediaInfo *pInfo)
{
	char str[50];
	int nLen;
	unsigned long nFormat;

	if (nIndex >= m_nOutputNum)
		return false;

	if (m_pStreamData[nIndex].type == MEDIA_TYPE_UNKNOWN_AUDIO)
	{
		sprintf(str, "Audio format tag = %ld", m_pStreamData[nIndex].nFormat);
		nLen = strlen(str) + 1;
		if (pInfo->nSize < sizeof(MediaInfo) + nLen)
			return false;
		pInfo->nSize = sizeof(MediaInfo) + nLen;
		strcpy(((TextInfo *)pInfo)->pText, str);
	}
	else if (m_pStreamData[nIndex].type == MEDIA_TYPE_UNKNOWN_VIDEO)
	{
		nFormat = m_pStreamData[nIndex].nFormat;
		if (nFormat < 0x20000000)
			sprintf(str, "Video compression = 0x%lX", nFormat);
		else
			sprintf(str, "Video compression = 0x%lX (%c%c%c%c)", nFormat, (char)(nFormat & 0xFF), (char)((nFormat >> 8) & 0xFF), (char)((nFormat >> 16) & 0xFF), (char)((nFormat >> 24) & 0xFF));
		nLen = strlen(str) + 1;
		if (pInfo->nSize < sizeof(MediaInfo) + nLen)
			return false;
		pInfo->nSize = sizeof(MediaInfo) + nLen;
		strcpy(((TextInfo *)pInfo)->pText, str);
	}
	else
	{
		if (m_pStreamData[nIndex].pInfo == NULL)
			return false;
		if (pInfo->nSize < m_pStreamData[nIndex].pInfo->nSize)
			return false;
		memcpy(pInfo, m_pStreamData[nIndex].pInfo, m_pStreamData[nIndex].pInfo->nSize);
	}
	return true;
}

/*CMediaSource *CASFSource::GetOutputSource(int nIndex)
{
	CProxySource *pProxy;
	UnknownInfo unknownInfo;
	char str[8];
	int i;

	if (nIndex >= m_nOutputNum)
		return NULL;
	if (m_ppOutputSources == NULL)
	{
		m_ppOutputSources = new CMediaSource *[m_nOutputNum];
		memset(m_ppOutputSources, 0, m_nOutputNum * sizeof(CMediaSource *));
	}

	if (m_ppOutputSources[nIndex] == NULL)
		switch (m_pStreamData[nIndex].type)
		{
			case MEDIA_TYPE_WMA:
				pProxy = new CProxySource(this, MEDIA_TYPE_WMA, 1);
				pProxy->SetSourceIndex(0, nIndex);
				pProxy->SetSourceInfo(m_pStreamData[nIndex].pInfo);
				m_ppOutputSources[nIndex] = pProxy;
				break;
			case MEDIA_TYPE_WMV:
			case MEDIA_TYPE_DVR:
			case MEDIA_TYPE_VC1:
				pProxy = new CProxySource(this, m_pStreamData[nIndex].type, 1);
				pProxy->SetSourceIndex(0, nIndex);
				pProxy->SetSourceInfo(m_pStreamData[nIndex].pInfo);
				m_ppOutputSources[nIndex] = pProxy;
				break;
			case MEDIA_TYPE_UNKNOWN_AUDIO:
				unknownInfo.nSize = sizeof(UnknownInfo);
				sprintf(unknownInfo.pData, "Audio format tag = %ld", m_pStreamData[nIndex].nFormat);
				pProxy = new CProxySource(this, MEDIA_TYPE_UNKNOWN_AUDIO, 1);
				pProxy->SetSourceInfo(&unknownInfo);
				pProxy->SetSourceIndex(0, nIndex);
				m_ppOutputSources[nIndex] = pProxy;
				break;
			case MEDIA_TYPE_UNKNOWN_VIDEO:
				unknownInfo.nSize = sizeof(UnknownInfo);
				sprintf(unknownInfo.pData, "Video compression = 0x%X", m_pStreamData[nIndex].nFormat);
				strcpy(str, " (    )");
				memcpy(str + 2, &m_pStreamData[nIndex].nFormat, 4);
				for (i = 2; i < 6; i++)
					if (str[i] < ' ')
						break;
				if (i == 6)
					strcat(unknownInfo.pData, str);
				pProxy = new CProxySource(this, MEDIA_TYPE_UNKNOWN_VIDEO, 1);
				pProxy->SetSourceInfo(&unknownInfo);
				pProxy->SetSourceIndex(0, nIndex);
				m_ppOutputSources[nIndex] = pProxy;
				break;
			default:
				return CMediaSource::GetOutputSource(nIndex);
		}
	return m_ppOutputSources[nIndex];
}*/

size_t CASFSource::GetPosition(int nIndex)
{
	return nIndex >= m_nOutputNum? 0 : m_pStreamPos[nIndex].nRelPos;
}

int CASFSource::ReadData(int nIndex, void *pData, int nSize)
{
	static int nLengthTypeTable[] = {0, 1, 2, 4};
	StreamPos *pPos;
	StreamData *pStream;
	int nFlags, nLen, nStreamNumber;

	if (nIndex >= m_nOutputNum || m_nPacketIndex >= m_nPacketNum)
		return 0;

	pPos = m_pStreamPos + nIndex;
	pStream = m_pStreamData + nIndex;
	SourceSeekData(pPos->nAbsPos, SEEK_SET);
	while (pPos->nRelPos >= pPos->nSize && m_nPacketIndex < m_nPacketNum)
	{
		if (pStream->nNumOfPayloads == 0)
		{
			SourceSeekData(pStream->nPadding, SEEK_CUR);  //skip padding data
			++m_nPacketIndex;

			pPos->nAbsPos = SourceGetPosition();
			nFlags = SourceReadData();  //error correction flags
			if (nFlags == -1)
				return 0;
			if ((nFlags & 0x80) != 0)  //error correction present
			{
				assert((nFlags & 0x10) == 0);  //opaque data present
				SourceSeekData(nFlags & 0x0F, SEEK_CUR);  //error correction data length
				nFlags = SourceReadData();  //length type flags
				pStream->nPropertyFlags = SourceReadData();  //property flags
				assert(pStream->nPropertyFlags == 0x5D);
				nLen = nLengthTypeTable[(nFlags >> 5) & 0x03] + nLengthTypeTable[(nFlags >> 1) & 0x03];
				SourceSeekData(nLen, SEEK_CUR);  //packet length, sequence
				pStream->nPadding = SourceReadLittleEndian(nLengthTypeTable[(nFlags >> 3) & 0x03]);  //padding length
				SourceSeekData(6, SEEK_CUR);  //send time, duration
				pStream->bMultiplePayloads = (nFlags & 1) != 0;
				if (pStream->bMultiplePayloads)  //multiple payload present
				{
					pStream->nPayloadFlags = SourceReadData();  //payload flags
					pStream->nNumOfPayloads = pStream->nPayloadFlags & 0x3F;  //number of payloads
				}
				else  //single payload
					pStream->nNumOfPayloads = 1;
			}
			else
			{
				assert(false);  //will this happen?
			}
		}  //if

		while (pStream->nNumOfPayloads > 0)
		{
			--pStream->nNumOfPayloads;

			nStreamNumber = SourceReadData();
			if (nStreamNumber == -1)
				return 0;
			nStreamNumber &= 0x7F;  //stream number
			nLen = nLengthTypeTable[(pStream->nPropertyFlags >> 4) & 0x03] + nLengthTypeTable[(pStream->nPropertyFlags >> 2) & 0x03];
			SourceSeekData(nLen, SEEK_CUR);  //media object number, offset into media object
			nLen = SourceReadLittleEndian(nLengthTypeTable[pStream->nPropertyFlags & 0x03]);  //replicated data length
			if (nLen == 1)  //compressed payload
			{
				assert(false);  //not implemented
			}
			else
			{
				SourceSeekData(nLen, SEEK_CUR);  //replicated data
				if (pStream->bMultiplePayloads)
				{
					nLen = nLengthTypeTable[pStream->nPayloadFlags >> 6];  //payload length
					nLen = SourceReadLittleEndian(nLen);
					if (pStream->nStreamNumber != nStreamNumber)
						SourceSeekData(nLen, SEEK_CUR);
					else
					{
						pPos->nSize += nLen;
						break;
					}
				}
				else
				{
					if (pStream->nStreamNumber != nStreamNumber)
					{
						SourceSeekData(pPos->nAbsPos + m_nPacketSize, SEEK_SET);  //skip packet
						pStream->nPadding = 0;
					}
					else
					{
						pPos->nSize += m_nPacketSize - (SourceGetPosition() - pPos->nAbsPos) - pStream->nPadding;
						break;
					}
				}
			}
		}  //if
	}  //while

	if (nSize > pPos->nSize - pPos->nRelPos)
		nSize = pPos->nSize - pPos->nRelPos;
	nSize = SourceReadData(pData, nSize);
	if (nSize > 0)
	{
		pPos->nRelPos += nSize;
		pPos->nAbsPos = SourceGetPosition();
	}
	return nSize;
}

bool CASFSource::SeekData(int nIndex, size_t nOffset, int nFrom)
{
	return false;  //note implemented
}

int CASFSource::GetDuration(void)
{
	return m_nDuration;
}

int CASFSource::SeekTime(int nTime)
{
	int i;
	StreamPos *pPos;
	StreamData *pStream;

	for (i = 0; i < m_nOutputNum; i++)
	{
		pPos = m_pStreamPos + i;
		pPos->nAbsPos = pPos->nHeadPos;
		pPos->nRelPos = 0;
		pPos->nSize = 0;
		pPos->nTime = 0;
		pStream = m_pStreamData + i;
		pStream->nPadding = 0;
		pStream->nNumOfPayloads = 0;
	}
	m_nPacketIndex = 0;
	return 0;
}
