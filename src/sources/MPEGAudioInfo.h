#ifndef _MPEGAUDIOINFO_H_
#define _MPEGAUDIOINFO_H_

#include "MediaInfo.h"

enum MPEGAudioChannel {MPEGAUDIO_STEREO, MPEGAUDIO_JOINT_STEREO, MPEGAUDIO_DUAL_CHANNEL, MPEGAUDIO_SINGLE_CHANNEL};  //channel mode

typedef struct _MPEGAudioInfo : AudioInfo
{
	int nVersion;
	int nLayer;
	MPEGAudioChannel nChannelMode;
} MPEGAudioInfo;

#endif
