#include "MediaType.h"
#include "MediaInfo.h"
#include "AACInfo.h"
#include "AIFFInfo.h"
#include "ASFInfo.h"
#include "AVIInfo.h"
#include "BitmapInfo.h"
#include "DTSInfo.h"
#include "FLACInfo.h"
#include "FlashVideoInfo.h"
#include "MatroskaInfo.h"
#include "MIDIInfo.h"
#include "MLPInfo.h"
#include "MPEGAudioInfo.h"
#include "MPEGProgramInfo.h"
#include "MPEGVideoInfo.h"
#include "PCMInfo.h"
#include "PESPacketInfo.h"
#include "PNGInfo.h"
#include "ProgramAssociationInfo.h"
#include "ProgramMapInfo.h"
#include "QuickTimeInfo.h"
#include "RealMediaInfo.h"
#include "TheoraInfo.h"
#include "VorbisInfo.h"
#include "WAVInfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static const char *pMediaType[] = {"Unknown", "Unknown audio", "Unknown video", "3GPP", "AAC", "AC-3", "AIFF", "AMR", "ASF", "AU", "AVI", "Bitmap", "DAT", "DD+", "DTS", "DVR", "FLAC", "Flash Video", "H.264", "Matroska", "MIDI", "MIDI event", "MJPEG", "MLP", "Monkey's Audio", "MPEG-4", "MPEG-4 Video", "MPEG Audio", "MPEG Program", "MPEG Transport", "MPEG Video", "Ogg", "PCM", "PES Packet", "PNG", "Program Association Table", "Program Map Table", "QuickTime", "RealAudio", "RealMedia", "RMID", "Subpicture", "Theora", "VC1", "Vorbis", "WAV", "WMA", "WMV"};
static const char *pPixelFormat[] = {"Unknown", "RGB555", "RGB565", "RGB24", "RGB32", "YUY2", "YV12"};
static const char *pAACHeaderType[] = {"none", "ADIF", "ADTS"};
static const char *pAACProfile[] = {"Main", "LC", "SSR", "LTP"};
static const char *pMPEGAudioChannelMode[] = {"stereo", "joint stereo", "dual channel", "single channel"};
static const char *pPCMFormat[] = {"Unknown", "Linear", "ALAW", "MULAW", "Float"};
static const char *pPNGColorType[] = {"Unknown", "Grey scale", "True color", "Indexed color", "Grey scale with alpha", "True color with alpha"};
static const char *pWAVFormat[] = {
	"", "LPCM", "ADPCM", "IEEE FLOAT", "VSELP", "IBM CVSD", "ALAW", "MULAW", "DTS", "DRM", "WMSP1", "", "", "", "", "",
	"OKI ADPCM", "DVI ADPCM", "MEDIASPACE ADPCM", "SIERRA ADPCM", "G723 ADPCM", "DIGISTD", "DIGIFIX", "DIALOGIC OKI ADPCM", "MEDIAVISION ADPCM", "CU CODEC", "HP DYNAMIC VOICE", "", "", "", "", "",
	"YAMAHA ADPCM", "SONARC", "DSPGROUP TRUESPEECH", "ECHOSC1", "AUDIOFILE AF36", "APTX", "AUDIOFILE AF10", "PROSODY 1612", "LRC", "", "", "", "", "", "", "",
	"DOLBY AC2", "GSM610", "MSNAUDIO", "ANTEX ADPCME", "CONTROL RES VQLPC", "DIGIREAL", "DIGIADPCM", "CONTROL RES CR10", "NMS VBXADPCM", "ROLAND RDAC", "ECHOSC3", "ROCKWELL ADPCM", "ROCKWELL DIGITALK", "XEBEC", "", "",
	"G721 ADPCM", "G728 CELP", "MSG723", "MSG723_1", "MSG729", "SPG726", "", "", "", "", "", "", "", "", "", "",
	"MPEG", "", "RT24", "PAC", "", "MPEGLAYER3"
};
static char pSpace[20];

static void printStringInfo(const char *pName, char *pValue)
{
	if (pValue)
	{
		printf("%s%s = ", pSpace, pName);
		printf("%s\n", pValue);
	}
}

static void printStringInfo(const char *pName, wchar_t *pValue)
{
	if (pValue)
	{
		printf("%s%s = ", pSpace, pName);
		wprintf(L"%s\n", pValue);
	}
}

void printAudioInfo(const char *pSpace, AudioInfo *pInfo)
{
	printf("%sSample rate = %d\n", pSpace, pInfo->nSampleRate);
	printf("%sChannel = %d\n", pSpace, pInfo->nChannel);
}

void printVideoInfo(const char *pSpace, VideoInfo *pInfo)
{
	printf("%sWidth = %d\n", pSpace, pInfo->nWidth);
	printf("%sHeight = %d\n", pSpace, pInfo->nHeight);
}

void printMediaInfo(MediaType type, MediaInfo *pInfo, int nLevel)
{
	int i;
	char pstr[5];

	assert(sizeof(pMediaType) / sizeof(pMediaType[0]) == MEDIA_TYPE_NUM);
	assert(sizeof(pPixelFormat) / sizeof(pPixelFormat[0]) == PIXEL_FORMAT_NUM);
	assert(sizeof(pPCMFormat) / sizeof(pPCMFormat[0]) == PCM_FORMAT_NUM);

	for (i = 0; i < nLevel * 2; i++)
		pSpace[i] = ' ';
	pSpace[i] = '\0';

	printf("%s[%s]\n", pSpace, pMediaType[type]);
	if (pInfo == NULL)
	{
		printf("%sfail\n", pSpace);
		return;
	}

	switch (type)
	{
		case MEDIA_TYPE_UNKNOWN:
		case MEDIA_TYPE_UNKNOWN_AUDIO:
		case MEDIA_TYPE_UNKNOWN_VIDEO:
			TextInfo *pTextInfo;
			pTextInfo = (TextInfo *)pInfo;
			printf("%s%s\n", pSpace, pTextInfo->pText);
			break;
		case MEDIA_TYPE_AC3:
		case MEDIA_TYPE_DDPLUS:
		case MEDIA_TYPE_WMA:
			printAudioInfo(pSpace, (AudioInfo *)pInfo);
			break;
		case MEDIA_TYPE_DVR:
		case MEDIA_TYPE_H264:
		case MEDIA_TYPE_MJPEG:
		case MEDIA_TYPE_MPEG4VIDEO:
		case MEDIA_TYPE_VC1:
		case MEDIA_TYPE_WMV:
			printVideoInfo(pSpace, (VideoInfo *)pInfo);
			break;
		case MEDIA_TYPE_3GPP:
		case MEDIA_TYPE_MPEG4:
		case MEDIA_TYPE_QUICKTIME:
			QuickTimeInfo *pQuickTimeInfo;
			pQuickTimeInfo = (QuickTimeInfo *)pInfo;
			printStringInfo("File type", pQuickTimeInfo->pFileType);
			break;
		case MEDIA_TYPE_AAC:
			AACInfo *pAACInfo;
			pAACInfo = (AACInfo *)pInfo;
			printf("%sHeader = %s\n", pSpace, pAACHeaderType[pAACInfo->nHeaderType]);
			printf("%sProfile = %s\n", pSpace, pAACProfile[pAACInfo->nProfile]);
			printAudioInfo(pSpace, (AudioInfo *)pInfo);
			break;
		case MEDIA_TYPE_AIFF:
			AIFFInfo *pAIFFInfo;
			pAIFFInfo = (AIFFInfo *)pInfo;
			printf("%sCompressed = %s\n", pSpace, pAIFFInfo->bCompressed? "true" : "false");
			if (pAIFFInfo->bCompressed)
			{
				printf("%sCompression type = %s\n", pSpace, pAIFFInfo->pCompressionType);
				printStringInfo("Compression type name", pAIFFInfo->pCompressionName);
			}
			break;
		case MEDIA_TYPE_ASF:
			ASFInfo *pASFInfo;
			pASFInfo = (ASFInfo *)pInfo;
			printStringInfo("Title", pASFInfo->pTitle);
			printStringInfo("Author", pASFInfo->pAuthor);
			printStringInfo("Copyright", pASFInfo->pCopyright);
			printStringInfo("Description", pASFInfo->pDescription);
			printStringInfo("Rating", pASFInfo->pRating);
			printStringInfo("Album", pASFInfo->pAlbum);
			printStringInfo("Year", pASFInfo->pYear);
			printStringInfo("Genre", pASFInfo->pGenre);
			printf("%sDRM = %s\n", pSpace, pASFInfo->bDRM? "true" : "false");
			if (pASFInfo->bDRM)
			{
				printf("%sKey ID = %s\n", pSpace, pASFInfo->pKeyID);
				printf("%sLicense URL = %s\n", pSpace, pASFInfo->pLicenseURL);
			}
			break;
		case MEDIA_TYPE_AVI:
			AVIInfo *pAVIInfo;
			pAVIInfo = (AVIInfo *)pInfo;
			printf("%sVideo FCC = %s\n", pSpace, pAVIInfo->pVideoFCC);
			printf("%sVideo frame rate = %.2f\n", pSpace, pAVIInfo->fFrameRate);
			printf("%sAudio FCC = %s\n", pSpace, pAVIInfo->pAudioFCC);
			break;
		case MEDIA_TYPE_BITMAP:
			BitmapInfo *pBitmapInfo;
			pBitmapInfo = (BitmapInfo *)pInfo;
			printVideoInfo(pSpace, (VideoInfo *)pInfo);
			printf("%sBits per pixel = %d\n", pSpace, pBitmapInfo->nBitsPerPixel);
			if (pBitmapInfo->nCompression == 0)
				strcpy(pstr, "RGB");
			else if (pBitmapInfo->nCompression == 1)
				strcpy(pstr, "RLE4");
			else if (pBitmapInfo->nCompression == 2)
				strcpy(pstr, "RLE8");
			else
			{
				memcpy(pstr, &pBitmapInfo->nCompression, 4);
				pstr[4] = '\0';
			}
			printf("%sCompression = 0x%lX (%s)\n", pSpace, pBitmapInfo->nCompression, pstr);
			printf("%sColor used = %d\n", pSpace, pBitmapInfo->nColorUsed);
			break;
		case MEDIA_TYPE_DTS:
			DTSInfo *pDTSInfo;
			pDTSInfo = (DTSInfo *)pInfo;
			printf("%sBits per sample = %d\n", pSpace, pDTSInfo->nBitsPerSample);
			printAudioInfo(pSpace, (AudioInfo *)pInfo);
			break;
		case MEDIA_TYPE_FLAC:
			FLACInfo *pFLACInfo;
			pFLACInfo = (FLACInfo *)pInfo;
			printf("%sBits per sample = %d\n", pSpace, pFLACInfo->nBitsPerSample);
			printAudioInfo(pSpace, (AudioInfo *)pInfo);
			break;
		case MEDIA_TYPE_FLASHVIDEO:
			FlashVideoInfo *pFlashVideoInfo;
			pFlashVideoInfo = (FlashVideoInfo *)pInfo;
			printf("%sVersion = %d\n", pSpace, pFlashVideoInfo->nVersion);
			break;
		case MEDIA_TYPE_MATROSKA:
			MatroskaInfo *pMatroskaInfo;
			pMatroskaInfo = (MatroskaInfo *)pInfo;
			printf("%sVersion = %d\n", pSpace, pMatroskaInfo->nVersion);
			printf("%sRead version = %d\n", pSpace, pMatroskaInfo->nReadVersion);
			printf("%sType version = %d\n", pSpace, pMatroskaInfo->nTypeVersion);
			printf("%sType read version = %d\n", pSpace, pMatroskaInfo->nTypeReadVersion);
			break;
		case MEDIA_TYPE_MIDI:
			MIDIInfo *pMIDIInfo;
			pMIDIInfo = (MIDIInfo *)pInfo;
			printf("%sFormat = %d\n", pSpace, pMIDIInfo->nFormat);
			break;
		case MEDIA_TYPE_MLP:
			MLPInfo *pMLPInfo;
			pMLPInfo = (MLPInfo *)pInfo;
			printf("%sMode = %s\n", pSpace, pMLPInfo->nStream == MLP_STREAM_FBA? "FBA" : "FBB");
			printAudioInfo(pSpace, (AudioInfo *)pInfo);
			break;
		case MEDIA_TYPE_MPEGAUDIO:
			MPEGAudioInfo *pMPEGAudioInfo;
			pMPEGAudioInfo = (MPEGAudioInfo *)pInfo;
			if (pMPEGAudioInfo->nVersion == 0)  //2.5
				printf("%sVersion = 2.5\n", pSpace);
			else
				printf("%sVersion = %d\n", pSpace, pMPEGAudioInfo->nVersion);
			printf("%sLayer = %d\n", pSpace, pMPEGAudioInfo->nLayer);
			printf("%sChannel mode = %s\n", pSpace, pMPEGAudioChannelMode[pMPEGAudioInfo->nChannelMode]);
			printAudioInfo(pSpace, (AudioInfo *)pInfo);
			break;
		case MEDIA_TYPE_MPEGPROGRAM:
			MPEGProgramInfo *pMPEGProgramInfo;
			pMPEGProgramInfo = (MPEGProgramInfo *)pInfo;
			printf("%sVersion = %d\n", pSpace, pMPEGProgramInfo->nVersion);
			printf("%sCBR = %d\n", pSpace, pMPEGProgramInfo->bCBR);
			break;
		case MEDIA_TYPE_MPEGVIDEO:
			MPEGVideoInfo *pMPEGVideoInfo;
			pMPEGVideoInfo = (MPEGVideoInfo *)pInfo;
			printf("%sVersion = %d\n", pSpace, pMPEGVideoInfo->nVersion);
			if (pMPEGVideoInfo->nVersion == 1)
				printf("%sPixel aspect ratio = %.4f\n", pSpace, pMPEGVideoInfo->fPelAspectRatio);
			else if (pMPEGVideoInfo->nVersion == 2)
				printf("%sDisplay aspect ratio = %d : %d\n", pSpace, pMPEGVideoInfo->nDispAspectRatio >> 16, pMPEGVideoInfo->nDispAspectRatio & 0xFFFF);
			printf("%sFrame rate = %.2f\n", pSpace, pMPEGVideoInfo->fFrameRate);
			printVideoInfo(pSpace, (VideoInfo *)pInfo);
			break;
		case MEDIA_TYPE_PCM:
			PCMInfo *pPCMInfo;
			pPCMInfo = (PCMInfo *)pInfo;
			printf("%sFormat = %s\n", pSpace, pPCMFormat[pPCMInfo->nFormat]);
			printf("%sBits per sample = %d\n", pSpace, pPCMInfo->nBitsPerSample);
			printAudioInfo(pSpace, (AudioInfo *)pInfo);
			break;
		case MEDIA_TYPE_PESPACKET:
			PESPacketInfo *pPESPacketInfo;
			pPESPacketInfo = (PESPacketInfo *)pInfo;
			printf("%sStream ID = 0x%02X\n", pSpace, pPESPacketInfo->nStreamID);
			if (pPESPacketInfo->nStreamID == 0xBD)  //private stream 1
				printf("%sStream sub ID = 0x%02X\n", pSpace, pPESPacketInfo->nStreamSubID);
			break;
		case MEDIA_TYPE_PNG:
			PNGInfo *pPNGInfo;
			pPNGInfo = (PNGInfo *)pInfo;
			printf("%sWidth = %d\n", pSpace, pPNGInfo->nWidth);
			printf("%sHeight = %d\n", pSpace, pPNGInfo->nHeight);
			printf("%sBits per pixel = %d\n", pSpace, pPNGInfo->nBitsPerPixel);
			printf("%sColor type = %s\n", pSpace, pPNGColorType[pPNGInfo->nColorType]);
			printf("%sColor used = %d\n", pSpace, pPNGInfo->nColorUsed);
			break;
		case MEDIA_TYPE_PROGRAMASSOCIATION:
			ProgramAssociationInfo *pProgramAssociationInfo;
			pProgramAssociationInfo = (ProgramAssociationInfo *)pInfo;
			printf("%sVersion = %d\n", pSpace, pProgramAssociationInfo->nVersion);
			for (i = 0; i < pProgramAssociationInfo->nProgramNum; i++)
				printf("%sProgram Number = %d PID = %d\n", pSpace, pProgramAssociationInfo->pInfo[i].nProgramNumber, pProgramAssociationInfo->pInfo[i].nPID);
			break;
		case MEDIA_TYPE_PROGRAMMAP:
			ProgramMapInfo *pProgramMapInfo;
			pProgramMapInfo = (ProgramMapInfo *)pInfo;
			printf("%sProgram Number = %d\n", pSpace, pProgramMapInfo->nProgramNumber);
			printf("%sVersion = %d\n", pSpace, pProgramMapInfo->nVersion);
			for (i = 0; i < pProgramMapInfo->nStreamNum; i++)
				printf("%sType = 0x%02X PID = %d\n", pSpace, pProgramMapInfo->pInfo[i].nType, pProgramMapInfo->pInfo[i].nPID);
			break;
		case MEDIA_TYPE_REALAUDIO:
		case MEDIA_TYPE_REALMEDIA:
			RealMediaInfo *pRealMediaInfo;
			pRealMediaInfo = (RealMediaInfo *)pInfo;
			printStringInfo("Title", pRealMediaInfo->pTitle);
			printStringInfo("Author", pRealMediaInfo->pAuthor);
			printStringInfo("Copyright", pRealMediaInfo->pCopyright);
			printStringInfo("Comment", pRealMediaInfo->pComment);
			break;
		case MEDIA_TYPE_THEORA:
			TheoraInfo *pTheoraInfo;
			pTheoraInfo = (TheoraInfo *)pInfo;
			printf("%sVersion = %d.%d.%d\n", pSpace, pTheoraInfo->nMajorVersion, pTheoraInfo->nMinorVersion, pTheoraInfo->nVersionRevision);
			printf("%sFrame width = %d\n", pSpace, pTheoraInfo->nFrameWidth);
			printf("%sFrame height = %d\n", pSpace, pTheoraInfo->nFrameHeight);
			printf("%sX offset = %d\n", pSpace, pTheoraInfo->nOffsetX);
			printf("%sY offset = %d\n", pSpace, pTheoraInfo->nOffsetY);
			printf("%sFrame rate = %.2f\n", pSpace, pTheoraInfo->fFrameRate);
			printVideoInfo(pSpace, (VideoInfo *)pInfo);
			break;
		case MEDIA_TYPE_VORBIS:
			VorbisInfo *pVorbisInfo;
			pVorbisInfo = (VorbisInfo *)pInfo;
			printStringInfo("Vendor", pVorbisInfo->pVendor);
			if (pVorbisInfo->nUserComment > 0)
			{
				printf("%sUser comment:\n", pSpace);
				for (i = 0; i < pVorbisInfo->nUserComment; i++)
				{
					printf("%s ", pSpace);
					wprintf(L"%s\n", pVorbisInfo->pUserComment[i]);
				}
			}
			printAudioInfo(pSpace, (AudioInfo *)pInfo);
			break;
		case MEDIA_TYPE_WAV:
			WAVInfo *pWAVInfo;
			pWAVInfo = (WAVInfo *)pInfo;
			printf("%sFormat = 0x%lX", pSpace, pWAVInfo->nFormatTag);
			if (pWAVInfo->nFormatTag >= sizeof(pWAVFormat) / sizeof(pWAVFormat[0]))
				printf(" (Unknown)\n");
			else if (*pWAVFormat[pWAVInfo->nFormatTag] == '\0')
				printf(" (Unknown)\n");
			else
				printf(" (%s)\n", pWAVFormat[pWAVInfo->nFormatTag]);
			printf("%sBits per sample = %d\n", pSpace, pWAVInfo->nBitsPerSample);
			printAudioInfo(pSpace, (AudioInfo *)pInfo);
			break;
		default:
			break;
	}
}
