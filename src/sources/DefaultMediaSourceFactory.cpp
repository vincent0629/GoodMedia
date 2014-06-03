#include "DefaultMediaSourceFactory.h"
#include "MediaSourceFactory.h"
#include "AACSource.h"
#include "AC3Source.h"
#include "AIFFSource.h"
#include "AMRSource.h"
#include "ASFSource.h"
#include "AUSource.h"
#include "AVISource.h"
#include "BitmapSource.h"
#include "DATSource.h"
#include "DTSSource.h"
#include "FLACSource.h"
#include "FlashVideoSource.h"
#include "H264Source.h"
#include "MatroskaSource.h"
#include "MIDISource.h"
#include "MLPSource.h"
#include "MonkeysAudioSource.h"
#include "MPEGAudioSource.h"
#include "MPEGProgramSource.h"
#include "MPEGTransportSource.h"
#include "MPEGVideoSource.h"
#include "OggSource.h"
#include "PCMSource.h"
#include "PESPacketSource.h"
#include "PNGSource.h"
#include "ProgramAssociationSource.h"
#include "ProgramMapSource.h"
#include "QuickTimeSource.h"
#include "RealAudioSource.h"
#include "RealMediaSource.h"
#include "RMIDSource.h"
#include "TheoraSource.h"
#include "VorbisSource.h"
#include "WAVSource.h"

static CMediaSource *createFunc(MediaType type, CMediaSource *pSource, int nIndex)
{
	CMediaSource *pResult;

	pResult = NULL;
	switch (type)
	{
		case MEDIA_TYPE_AAC:
			pResult = new CAACSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_AC3:
			pResult = new CAC3Source(pSource, nIndex);
			break;
		case MEDIA_TYPE_AIFF:
			pResult = new CAIFFSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_AMR:
			pResult = new CAMRSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_ASF:
			pResult = new CASFSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_AU:
			pResult = new CAUSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_AVI:
			pResult = new CAVISource(pSource, nIndex);
			break;
		case MEDIA_TYPE_BITMAP:
			pResult = new CBitmapSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_DAT:
			pResult = new CDATSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_DDPLUS:
			pResult = new CAC3Source(pSource, nIndex);
			break;
		case MEDIA_TYPE_DTS:
			pResult = new CDTSSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_FLAC:
			pResult = new CFLACSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_FLASHVIDEO:
			pResult = new CFlashVideoSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_H264:
			pResult = new CH264Source(pSource, nIndex);
			break;
		case MEDIA_TYPE_MATROSKA:
			pResult = new CMatroskaSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_MIDI:
			pResult = new CMIDISource(pSource, nIndex);
			break;
		case MEDIA_TYPE_MLP:
			pResult = new CMLPSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_MONKEYSAUDIO:
			pResult = new CMonkeysAudioSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_MPEG4:
			pResult = new CQuickTimeSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_MPEGAUDIO:
			pResult = new CMPEGAudioSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_MPEGPROGRAM:
			pResult = new CMPEGProgramSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_MPEGTRANSPORT:
			pResult = new CMPEGTransportSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_MPEGVIDEO:
			pResult = new CMPEGVideoSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_OGG:
			pResult = new COggSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_PCM:
			pResult = new CPCMSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_PESPACKET:
			pResult = new CPESPacketSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_PNG:
			pResult = new CPNGSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_PROGRAMASSOCIATION:
			pResult = new CProgramAssociationSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_PROGRAMMAP:
			pResult = new CProgramMapSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_QUICKTIME:
			pResult = new CQuickTimeSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_REALAUDIO:
			pResult = new CRealAudioSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_REALMEDIA:
			pResult = new CRealMediaSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_RMID:
			pResult = new CRMIDSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_THEORA:
			pResult = new CTheoraSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_VORBIS:
			pResult = new CVorbisSource(pSource, nIndex);
			break;
		case MEDIA_TYPE_WAV:
			pResult = new CWAVSource(pSource, nIndex);
			break;
		default:
			break;
	}

	if (pResult != NULL)
		if (pResult->GetOutputNum() == 0)
		{
			delete pResult;
			pResult = NULL;
		}
	return pResult;
}

void CDefaultMediaSourceFactory::Register(void)
{
	int i;

	for (i = 0; i < MEDIA_TYPE_NUM; i++)
		CMediaSourceFactory::Register((MediaType)i, createFunc);
}
