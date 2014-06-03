#include "FileSource.h"
#include "SourceGraph.h"
#include "DefaultMediaSourceFactory.h"
#include "FileRenderer.h"
#include "MPEGVideoInfo.h"
#include "MPEGAudioInfo.h"
#include "platform.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

bool demux(char *path)
{
	static const char *pMediaType[] = {"Unknown", "Unknown audio", "Unknown video", "3GPP", "AAC", "AC-3", "AIFF", "AMR", "ASF", "AU", "AVI", "Bitmap", "DAT", "DD+", "DTS", "DVR", "FLAC", "Flash Video", "H.264", "Matroska", "MIDI", "MIDI event", "MJPEG", "MLP", "Monkey's Audio", "MPEG-4", "MPEG-4 Video", "MPEG Audio", "MPEG Program", "MPEG Transport", "MPEG Video", "Ogg", "PCM", "PES Packet", "PNG", "Program Association Table", "Program Map Table", "QuickTime", "RealAudio", "RealMedia", "RMID", "Subpicture", "Theora", "VC1", "Vorbis", "WAV", "WMA", "WMV"};
	const char *pVideoVersion[] = {"dump", "m1v", "m2v", "m4v"};
	char *pStr, fileName[50], pInput[10] = {0};
	CFileSource *pDataSource;
	CSourceGraph *pGraph;
	MediaType mediaType;
	CMediaSource *pSource, *pOutput;
	int i, nIndex, nFirstIndex, nLevel;
	MPEGVideoInfo videoInfo;
	MPEGAudioInfo audioInfo;
	CFileRenderer *pRenderer;
	FileRenderInfo fileInfo;
	bool bResult;
	unsigned char *pFrame;
	int nFrameSize, nSize, nTotalSize, nUnit;
	clock_t clk;

	assert(sizeof(pMediaType) / sizeof(pMediaType[0]) == MEDIA_TYPE_NUM);

#ifdef _WIN32
	pStr = strrchr(path, '\\');
#else
	pStr = strrchr(path, '/');
#endif
	strcpy(fileName, pStr == NULL? path : pStr + 1);
	pStr = strrchr(fileName, '.');
	bResult = false;

	CDefaultMediaSourceFactory::Register();
	pDataSource = new CFileSource();
	pDataSource->Open(path);
	pGraph = new CSourceGraph();
	mediaType = pGraph->Open(pDataSource);
	if (mediaType != MEDIA_TYPE_UNKNOWN)
	{
		pSource = pGraph->GetSource();
		nFirstIndex = 0;
		nLevel = 0;
		while (!bResult)
		{
			printf("%2d: %s\n", 0, pMediaType[pSource->GetSourceType()]);
			for (i = 0; i < pSource->GetOutputNum(); i++)
				printf("%2d: %s\n", i + 1, pMediaType[pSource->GetOutputType(i)]);

			printf("\nWhich stream do you want? [0-%d]: ", pSource->GetOutputNum());
			gets(pInput);
			if (*pInput != '\0')
			{
				nIndex = _ttoi(pInput);
				if (nIndex == 0)
				{
					nIndex = pSource->GetIndex();
					pSource = pSource->GetParent();
					bResult = true;
				}
				else if (nIndex >= 1 && nIndex <= pSource->GetOutputNum())
				{
					--nIndex;
					if (pSource->GetSourceType() == pSource->GetOutputType(nIndex))
						bResult = true;
					else
						pSource = pSource->GetOutputSource(nIndex);
					if (nFirstIndex == 0)
						nFirstIndex = nIndex + 1;
					++nLevel;
				}
			}
			else
			{
				pSource = pSource->GetParent();
				if (--nLevel < 0)
					break;
			}
		}
	}

	if (bResult)
	{
		sprintf(pStr, "_%d.", nFirstIndex);
		pStr = pStr + strlen(pStr);
		switch (pSource->GetOutputType(nIndex))
		{
			case MEDIA_TYPE_AAC:
				sprintf(pStr, "aac");
				break;
			case MEDIA_TYPE_AC3:
				sprintf(pStr, "ac3");
				break;
			case MEDIA_TYPE_DDPLUS:
				sprintf(pStr, "ec3");
				break;
			case MEDIA_TYPE_DTS:
				sprintf(pStr, "dts");
				break;
			case MEDIA_TYPE_H264:
				sprintf(pStr, "264");
				break;
			case MEDIA_TYPE_MIDI:
				sprintf(pStr, "mid");
				break;
			case MEDIA_TYPE_MLP:
				sprintf(pStr, "mlp");
				break;
			case MEDIA_TYPE_MPEGAUDIO:
				sprintf(pStr, "mpa");
				audioInfo.nSize = sizeof(MPEGAudioInfo);
				pOutput = pSource->GetOutputSource(nIndex);
				if (pOutput)
				{
					if (pOutput->GetSourceInfo(&audioInfo))
						sprintf(pStr, "mp%d", audioInfo.nLayer);
				}
				else if (pSource->GetOutputInfo(nIndex, &audioInfo))
					sprintf(pStr, "mp%d", audioInfo.nLayer);
				break;
			case MEDIA_TYPE_MPEGPROGRAM:
				sprintf(pStr, "mpg");
				break;
			case MEDIA_TYPE_MPEGVIDEO:
				sprintf(pStr, "mpv");
				videoInfo.nSize = sizeof(MPEGVideoInfo);
				pOutput = pSource->GetOutputSource(nIndex);
				if (pOutput)
				{
					if (pOutput->GetSourceInfo(&videoInfo))
						sprintf(pStr, pVideoVersion[videoInfo.nVersion]);
				}
				else if (pSource->GetOutputInfo(nIndex, &videoInfo))
					sprintf(pStr, pVideoVersion[videoInfo.nVersion]);
				break;
			case MEDIA_TYPE_PCM:
				sprintf(pStr, "pcm");
				break;
			default:
				sprintf(pStr, "dump");
				break;
		}  //switch

		pRenderer = new CFileRenderer();
		fileInfo.nSize = sizeof(FileRenderInfo);
		fileInfo.path = fileName;
		pRenderer->Open(&fileInfo);
		nFrameSize = 1024 * 1024;
		pFrame = new unsigned char[nFrameSize];
		nTotalSize = 0;
		nUnit = 1;
		clk = clock();
		for (;;)
		{
			nSize = pSource->ReadData(nIndex, pFrame, nFrameSize);
			if (nSize == 0)
				break;
			pRenderer->Write(pFrame, nSize);
			printf(".");
			nTotalSize += nSize;
			if (nTotalSize >= nUnit * 1024 * 1024)
			{
				printf("%dMB", nUnit);
				++nUnit;
			}
		}
		clk = clock() - clk;
		delete[] pFrame;
		delete pRenderer;
		pGraph->Close();
		printf("\nGenerate %s\n", fileName);
		printf("Spent %d ms\n", (int)(clk * 1000 / CLK_TCK));
	}

	delete pGraph;
	delete pDataSource;
	return bResult;
}

int main(int argc, char *argv[])
{
	char *param;

	if (argc < 2)
	{
#ifdef _WIN32
		param = strrchr(argv[0], '\\');
#else
		param = strrchr(argv[0], '/');
#endif
		printf("%s <file>\n", param == NULL? argv[0] : param + 1);
		return 0;
	}

	param = argv[argc - 1];
	if (param[0] == '"')
	{
		//remove double quote
		++param;
		param[strlen(param) - 1] = '\0';
	}

	if (demux(param))
		printf("success\n");
	else
		printf("failed\n");
	return 0;
}
