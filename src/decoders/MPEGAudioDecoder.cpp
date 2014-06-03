#include "MPEGAudioDecoder.h"
#include "MPEGAudioInfo.h"
#include "bitstream.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MULT_SS(x, y) ((x * y) >> 13)

typedef CMPEGAudioDecoder::DECODERPARAM DECODERPARAM;

typedef struct
{
	int version;
	int layer;
	int bit_rate_index;
	int sampling_frequency;
	int mode;
	int mode_extension;
} HEADER;

typedef struct
{
	int nbal;
	int nlevels[16];
} BITALLOCATION;

typedef struct
{
	int C;
	int D;
	int grouping;
	int bitsPerCodeword;
	int msb;
} CLASSESOFQUANTIZATION;

/*static const float scalefactorTable[63] = {
	2.00000000000000f, 1.58740105196820f, 1.25992104989487f, 1.00000000000000f,
	0.79370052598410f, 0.62996052494744f, 0.50000000000000f, 0.39685026299205f,
	0.31498026247372f, 0.25000000000000f, 0.19842513149602f, 0.15749013123686f,
	0.12500000000000f, 0.09921256574801f, 0.07874506561843f, 0.06250000000000f,
	0.04960628287401f, 0.03937253280921f, 0.03125000000000f, 0.02480314143700f,
	0.01968626640461f, 0.01562500000000f, 0.01240157071850f, 0.00984313320230f,
	0.00781250000000f, 0.00620078535925f, 0.00492156660115f, 0.00390625000000f,
	0.00310039267963f, 0.00246078330058f, 0.00195312500000f, 0.00155019633981f,
	0.00123039165029f, 0.00097656250000f, 0.00077509816991f, 0.00061519582514f,
	0.00048828125000f, 0.00038754908495f, 0.00030759791257f, 0.00024414062500f,
	0.00019377454248f, 0.00015379895629f, 0.00012207031250f, 0.00009688727124f,
	0.00007689947814f, 0.00006103515625f, 0.00004844363562f, 0.00003844973907f,
	0.00003051757813f, 0.00002422181781f, 0.00001922486954f, 0.00001525878906f,
	0.00001211090890f, 0.00000961243477f, 0.00000762939453f, 0.00000605545445f,
	0.00000480621738f, 0.00000381469727f, 0.00000302772723f, 0.00000240310869f,
	0.00000190734863f, 0.00000151386361f, 0.00000120155435f
};*/
static const short scalefactorTable[63] = {  //2^13
	0x4000, 0x32CB, 0x2851, 0x2000,
	0x1965, 0x1428, 0x1000, 0x0CB2,
	0x0A14, 0x0800, 0x0659, 0x050A,
	0x0400, 0x032C, 0x0285, 0x0200,
	0x0196, 0x0142, 0x0100, 0x00CB,
	0x00A1, 0x0080, 0x0065, 0x0050,
	0x0040, 0x0032, 0x0028, 0x0020,
	0x0019, 0x0014, 0x0010, 0x000C,
	0x000A, 0x0008, 0x0006, 0x0005,
	0x0004, 0x0003, 0x0002, 0x0002,
	0x0001, 0x0001, 0x0001, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000
};

/*static const float D[512] = {  //coefficients of the synthesis window
	 0.000000000f, -0.000015259f, -0.000015259f, -0.000015259f,
	-0.000015259f, -0.000015259f, -0.000015259f, -0.000030518f,
	-0.000030518f, -0.000030518f, -0.000030518f, -0.000045776f,
	-0.000045776f, -0.000061035f, -0.000061035f, -0.000076294f,
	-0.000076294f, -0.000091553f, -0.000106812f, -0.000106812f,
	-0.000122070f, -0.000137329f, -0.000152588f, -0.000167847f,
	-0.000198364f, -0.000213623f, -0.000244141f, -0.000259399f,
	-0.000289917f, -0.000320435f, -0.000366211f, -0.000396729f,
	-0.000442505f, -0.000473022f, -0.000534058f, -0.000579834f,
	-0.000625610f, -0.000686646f, -0.000747681f, -0.000808716f,
	-0.000885010f, -0.000961304f, -0.001037598f, -0.001113892f,
	-0.001205444f, -0.001296997f, -0.001388550f, -0.001480103f,
	-0.001586914f, -0.001693726f, -0.001785278f, -0.001907349f,
	-0.002014160f, -0.002120972f, -0.002243042f, -0.002349854f,
	-0.002456665f, -0.002578735f, -0.002685547f, -0.002792358f,
	-0.002899170f, -0.002990723f, -0.003082275f, -0.003173828f,
	 0.003250122f,  0.003326416f,  0.003387451f,  0.003433228f,
	 0.003463745f,  0.003479004f,  0.003479004f,  0.003463745f,
	 0.003417969f,  0.003372192f,  0.003280640f,  0.003173828f,
	 0.003051758f,  0.002883911f,  0.002700806f,  0.002487183f,
	 0.002227783f,  0.001937866f,  0.001617432f,  0.001266479f,
	 0.000869751f,  0.000442505f, -0.000030518f, -0.000549316f,
	-0.001098633f, -0.001693726f, -0.002334595f, -0.003005981f,
	-0.003723145f, -0.004486084f, -0.005294800f, -0.006118774f,
	-0.007003784f, -0.007919312f, -0.008865356f, -0.009841919f,
	-0.010848999f, -0.011886597f, -0.012939453f, -0.014022827f,
	-0.015121460f, -0.016235352f, -0.017349243f, -0.018463135f,
	-0.019577026f, -0.020690918f, -0.021789551f, -0.022857666f,
	-0.023910522f, -0.024932861f, -0.025909424f, -0.026840210f,
	-0.027725220f, -0.028533936f, -0.029281616f, -0.029937744f,
	-0.030532837f, -0.031005859f, -0.031387329f, -0.031661987f,
	-0.031814575f, -0.031845093f, -0.031738281f, -0.031478882f,
	 0.031082153f,  0.030517578f,  0.029785156f,  0.028884888f,
	 0.027801514f,  0.026535034f,  0.025085449f,  0.023422241f,
	 0.021575928f,  0.019531250f,  0.017257690f,  0.014801025f,
	 0.012115479f,  0.009231567f,  0.006134033f,  0.002822876f,
	-0.000686646f, -0.004394531f, -0.008316040f, -0.012420654f,
	-0.016708374f, -0.021179199f, -0.025817871f, -0.030609131f,
	-0.035552979f, -0.040634155f, -0.045837402f, -0.051132202f,
	-0.056533813f, -0.061996460f, -0.067520142f, -0.073059082f,
	-0.078628540f, -0.084182739f, -0.089706421f, -0.095169067f,
	-0.100540161f, -0.105819702f, -0.110946655f, -0.115921021f,
	-0.120697021f, -0.125259399f, -0.129562378f, -0.133590698f,
	-0.137298584f, -0.140670776f, -0.143676758f, -0.146255493f,
	-0.148422241f, -0.150115967f, -0.151306152f, -0.151962280f,
	-0.152069092f, -0.151596069f, -0.150497437f, -0.148773193f,
	-0.146362305f, -0.143264771f, -0.139450073f, -0.134887695f,
	-0.129577637f, -0.123474121f, -0.116577148f, -0.108856201f,
	 0.100311279f,  0.090927124f,  0.080688477f,  0.069595337f,
	 0.057617187f,  0.044784546f,  0.031082153f,  0.016510010f,
	 0.001068115f, -0.015228271f, -0.032379150f, -0.050354004f,
	-0.069168091f, -0.088775635f, -0.109161377f, -0.130310059f,
	-0.152206421f, -0.174789429f, -0.198059082f, -0.221984863f,
	-0.246505737f, -0.271591187f, -0.297210693f, -0.323318481f,
	-0.349868774f, -0.376800537f, -0.404083252f, -0.431655884f,
	-0.459472656f, -0.487472534f, -0.515609741f, -0.543823242f,
	-0.572036743f, -0.600219727f, -0.628295898f, -0.656219482f,
	-0.683914185f, -0.711318970f, -0.738372803f, -0.765029907f,
	-0.791213989f, -0.816864014f, -0.841949463f, -0.866363525f,
	-0.890090942f, -0.913055420f, -0.935195923f, -0.956481934f,
	-0.976852417f, -0.996246338f, -1.014617920f, -1.031936646f,
	-1.048156738f, -1.063217163f, -1.077117920f, -1.089782715f,
	-1.101211548f, -1.111373901f, -1.120223999f, -1.127746582f,
	-1.133926392f, -1.138763428f, -1.142211914f, -1.144287109f,
	 1.144989014f,  1.144287109f,  1.142211914f,  1.138763428f,
	 1.133926392f,  1.127746582f,  1.120223999f,  1.111373901f,
	 1.101211548f,  1.089782715f,  1.077117920f,  1.063217163f,
	 1.048156738f,  1.031936646f,  1.014617920f,  0.996246338f,
	 0.976852417f,  0.956481934f,  0.935195923f,  0.913055420f,
	 0.890090942f,  0.866363525f,  0.841949463f,  0.816864014f,
	 0.791213989f,  0.765029907f,  0.738372803f,  0.711318970f,
	 0.683914185f,  0.656219482f,  0.628295898f,  0.600219727f,
	 0.572036743f,  0.543823242f,  0.515609741f,  0.487472534f,
	 0.459472656f,  0.431655884f,  0.404083252f,  0.376800537f,
	 0.349868774f,  0.323318481f,  0.297210693f,  0.271591187f,
	 0.246505737f,  0.221984863f,  0.198059082f,  0.174789429f,
	 0.152206421f,  0.130310059f,  0.109161377f,  0.088775635f,
	 0.069168091f,  0.050354004f,  0.032379150f,  0.015228271f,
	-0.001068115f, -0.016510010f, -0.031082153f, -0.044784546f,
	-0.057617187f, -0.069595337f, -0.080688477f, -0.090927124f,
	 0.100311279f,  0.108856201f,  0.116577148f,  0.123474121f,
	 0.129577637f,  0.134887695f,  0.139450073f,  0.143264771f,
	 0.146362305f,  0.148773193f,  0.150497437f,  0.151596069f,
	 0.152069092f,  0.151962280f,  0.151306152f,  0.150115967f,
	 0.148422241f,  0.146255493f,  0.143676758f,  0.140670776f,
	 0.137298584f,  0.133590698f,  0.129562378f,  0.125259399f,
	 0.120697021f,  0.115921021f,  0.110946655f,  0.105819702f,
	 0.100540161f,  0.095169067f,  0.089706421f,  0.084182739f,
	 0.078628540f,  0.073059082f,  0.067520142f,  0.061996460f,
	 0.056533813f,  0.051132202f,  0.045837402f,  0.040634155f,
	 0.035552979f,  0.030609131f,  0.025817871f,  0.021179199f,
	 0.016708374f,  0.012420654f,  0.008316040f,  0.004394531f,
	 0.000686646f, -0.002822876f, -0.006134033f, -0.009231567f,
	-0.012115479f, -0.014801025f, -0.017257690f, -0.019531250f,
	-0.021575928f, -0.023422241f, -0.025085449f, -0.026535034f,
	-0.027801514f, -0.028884888f, -0.029785156f, -0.030517578f,
	 0.031082153f,  0.031478882f,  0.031738281f,  0.031845093f,
	 0.031814575f,  0.031661987f,  0.031387329f,  0.031005859f,
	 0.030532837f,  0.029937744f,  0.029281616f,  0.028533936f,
	 0.027725220f,  0.026840210f,  0.025909424f,  0.024932861f,
	 0.023910522f,  0.022857666f,  0.021789551f,  0.020690918f,
	 0.019577026f,  0.018463135f,  0.017349243f,  0.016235352f,
	 0.015121460f,  0.014022827f,  0.012939453f,  0.011886597f,
	 0.010848999f,  0.009841919f,  0.008865356f,  0.007919312f,
	 0.007003784f,  0.006118774f,  0.005294800f,  0.004486084f,
	 0.003723145f,  0.003005981f,  0.002334595f,  0.001693726f,
	 0.001098633f,  0.000549316f,  0.000030518f, -0.000442505f,
	-0.000869751f, -0.001266479f, -0.001617432f, -0.001937866f,
	-0.002227783f, -0.002487183f, -0.002700806f, -0.002883911f,
	-0.003051758f, -0.003173828f, -0.003280640f, -0.003372192f,
	-0.003417969f, -0.003463745f, -0.003479004f, -0.003479004f,
	-0.003463745f, -0.003433228f, -0.003387451f, -0.003326416f,
	 0.003250122f,  0.003173828f,  0.003082275f,  0.002990723f,
	 0.002899170f,  0.002792358f,  0.002685547f,  0.002578735f,
	 0.002456665f,  0.002349854f,  0.002243042f,  0.002120972f,
	 0.002014160f,  0.001907349f,  0.001785278f,  0.001693726f,
	 0.001586914f,  0.001480103f,  0.001388550f,  0.001296997f,
	 0.001205444f,  0.001113892f,  0.001037598f,  0.000961304f,
	 0.000885010f,  0.000808716f,  0.000747681f,  0.000686646f,
	 0.000625610f,  0.000579834f,  0.000534058f,  0.000473022f,
	 0.000442505f,  0.000396729f,  0.000366211f,  0.000320435f,
	 0.000289917f,  0.000259399f,  0.000244141f,  0.000213623f,
	 0.000198364f,  0.000167847f,  0.000152588f,  0.000137329f,
	 0.000122070f,  0.000106812f,  0.000106812f,  0.000091553f,
	 0.000076294f,  0.000076294f,  0.000061035f,  0.000061035f,
	 0.000045776f,  0.000045776f,  0.000030518f,  0.000030518f,
	 0.000030518f,  0.000030518f,  0.000015259f,  0.000015259f,
	 0.000015259f,  0.000015259f,  0.000015259f,  0.000015259f
};*/
static const short D[512] = {  //coefficients of the synthesis window  //2^13
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFE, 0xFFFE, 0xFFFE,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFC,
	0xFFFC, 0xFFFB, 0xFFFB, 0xFFFA,
	0xFFFA, 0xFFF9, 0xFFF8, 0xFFF8,
	0xFFF7, 0xFFF6, 0xFFF6, 0xFFF5,
	0xFFF4, 0xFFF3, 0xFFF2, 0xFFF1,
	0xFFF1, 0xFFF0, 0xFFEF, 0xFFEE,
	0xFFED, 0xFFEC, 0xFFEB, 0xFFEA,
	0xFFE9, 0xFFE8, 0xFFE8, 0xFFE7,
	0x001B, 0x001B, 0x001C, 0x001C,
	0x001C, 0x001D, 0x001D, 0x001C,
	0x001C, 0x001C, 0x001B, 0x001A,
	0x0019, 0x0018, 0x0016, 0x0014,
	0x0012, 0x0010, 0x000D, 0x000A,
	0x0007, 0x0004, 0x0000, 0xFFFD,
	0xFFF8, 0xFFF3, 0xFFEE, 0xFFE8,
	0xFFE2, 0xFFDC, 0xFFD6, 0xFFCF,
	0xFFC8, 0xFFC0, 0xFFB8, 0xFFB0,
	0xFFA8, 0xFFA0, 0xFF97, 0xFF8E,
	0xFF85, 0xFF7C, 0xFF73, 0xFF6A,
	0xFF61, 0xFF57, 0xFF4E, 0xFF46,
	0xFF3D, 0xFF35, 0xFF2D, 0xFF25,
	0xFF1E, 0xFF17, 0xFF11, 0xFF0C,
	0xFF07, 0xFF03, 0xFF00, 0xFEFE,
	0xFEFC, 0xFEFC, 0xFEFD, 0xFEFF,
	0x00FF, 0x00FA, 0x00F4, 0x00ED,
	0x00E4, 0x00D9, 0x00CE, 0x00C0,
	0x00B1, 0x00A0, 0x008D, 0x0079,
	0x0063, 0x004C, 0x0032, 0x0017,
	0xFFFB, 0xFFDD, 0xFFBD, 0xFF9B,
	0xFF78, 0xFF53, 0xFF2D, 0xFF06,
	0xFEDE, 0xFEB4, 0xFE89, 0xFE5E,
	0xFE32, 0xFE05, 0xFDD8, 0xFDAA,
	0xFD7D, 0xFD4F, 0xFD22, 0xFCF5,
	0xFCC9, 0xFC9E, 0xFC74, 0xFC4B,
	0xFC24, 0xFBFF, 0xFBDC, 0xFBBB,
	0xFB9C, 0xFB81, 0xFB68, 0xFB53,
	0xFB41, 0xFB33, 0xFB29, 0xFB24,
	0xFB23, 0xFB27, 0xFB30, 0xFB3E,
	0xFB52, 0xFB6B, 0xFB8B, 0xFBB0,
	0xFBDB, 0xFC0D, 0xFC46, 0xFC85,
	0x0336, 0x02E9, 0x0295, 0x023A,
	0x01D8, 0x016F, 0x00FF, 0x0087,
	0x0009, 0xFF84, 0xFEF8, 0xFE64,
	0xFDCA, 0xFD2A, 0xFC83, 0xFBD5,
	0xFB22, 0xFA69, 0xF9AA, 0xF8E6,
	0xF81E, 0xF750, 0xF67E, 0xF5A8,
	0xF4CF, 0xF3F2, 0xF313, 0xF231,
	0xF14D, 0xF068, 0xEF81, 0xEE9A,
	0xEDB3, 0xECCC, 0xEBE6, 0xEB01,
	0xEA1E, 0xE93E, 0xE860, 0xE786,
	0xE6AF, 0xE5DD, 0xE510, 0xE448,
	0xE385, 0xE2C9, 0xE214, 0xE165,
	0xE0BF, 0xE020, 0xDF89, 0xDEFB,
	0xDE76, 0xDDFB, 0xDD89, 0xDD21,
	0xDCC4, 0xDC71, 0xDC28, 0xDBEA,
	0xDBB8, 0xDB90, 0xDB74, 0xDB63,
	0x24A4, 0x249E, 0x248D, 0x2471,
	0x2449, 0x2417, 0x23D9, 0x2390,
	0x233D, 0x22E0, 0x2278, 0x2206,
	0x218B, 0x2106, 0x2078, 0x1FE1,
	0x1F42, 0x1E9C, 0x1DED, 0x1D38,
	0x1C7C, 0x1BB9, 0x1AF1, 0x1A24,
	0x1952, 0x187B, 0x17A1, 0x16C3,
	0x15E3, 0x1500, 0x141B, 0x1335,
	0x124E, 0x1167, 0x1080, 0x0F99,
	0x0EB4, 0x0DD0, 0x0CEE, 0x0C0F,
	0x0B32, 0x0A59, 0x0983, 0x08B1,
	0x07E3, 0x071B, 0x0657, 0x0598,
	0x04DF, 0x042C, 0x037E, 0x02D7,
	0x0237, 0x019D, 0x0109, 0x007D,
	0xFFF8, 0xFF7A, 0xFF02, 0xFE92,
	0xFE29, 0xFDC7, 0xFD6C, 0xFD18,
	0x0336, 0x037C, 0x03BB, 0x03F4,
	0x0426, 0x0451, 0x0476, 0x0496,
	0x04AF, 0x04C3, 0x04D1, 0x04DA,
	0x04DE, 0x04DD, 0x04D8, 0x04CE,
	0x04C0, 0x04AE, 0x0499, 0x0480,
	0x0465, 0x0446, 0x0425, 0x0402,
	0x03DD, 0x03B6, 0x038D, 0x0363,
	0x0338, 0x030C, 0x02DF, 0x02B2,
	0x0284, 0x0257, 0x0229, 0x01FC,
	0x01CF, 0x01A3, 0x0178, 0x014D,
	0x0123, 0x00FB, 0x00D4, 0x00AE,
	0x0089, 0x0066, 0x0044, 0x0024,
	0x0006, 0xFFEA, 0xFFCF, 0xFFB5,
	0xFF9E, 0xFF88, 0xFF74, 0xFF61,
	0xFF50, 0xFF41, 0xFF33, 0xFF28,
	0xFF1D, 0xFF14, 0xFF0D, 0xFF07,
	0x00FF, 0x0102, 0x0104, 0x0105,
	0x0105, 0x0103, 0x0101, 0x00FE,
	0x00FA, 0x00F5, 0x00F0, 0x00EA,
	0x00E3, 0x00DC, 0x00D4, 0x00CC,
	0x00C4, 0x00BB, 0x00B3, 0x00AA,
	0x00A0, 0x0097, 0x008E, 0x0085,
	0x007C, 0x0073, 0x006A, 0x0061,
	0x0059, 0x0051, 0x0049, 0x0041,
	0x0039, 0x0032, 0x002B, 0x0025,
	0x001F, 0x0019, 0x0013, 0x000E,
	0x0009, 0x0004, 0x0000, 0xFFFD,
	0xFFFA, 0xFFF7, 0xFFF4, 0xFFF1,
	0xFFEF, 0xFFED, 0xFFEB, 0xFFE9,
	0xFFE8, 0xFFE7, 0xFFE6, 0xFFE5,
	0xFFE5, 0xFFE5, 0xFFE4, 0xFFE4,
	0xFFE5, 0xFFE5, 0xFFE5, 0xFFE6,
	0x001B, 0x001A, 0x0019, 0x0019,
	0x0018, 0x0017, 0x0016, 0x0015,
	0x0014, 0x0013, 0x0012, 0x0011,
	0x0010, 0x0010, 0x000F, 0x000E,
	0x000D, 0x000C, 0x000B, 0x000B,
	0x000A, 0x0009, 0x0009, 0x0008,
	0x0007, 0x0007, 0x0006, 0x0006,
	0x0005, 0x0005, 0x0004, 0x0004,
	0x0004, 0x0003, 0x0003, 0x0003,
	0x0002, 0x0002, 0x0002, 0x0002,
	0x0002, 0x0001, 0x0001, 0x0001,
	0x0001, 0x0001, 0x0001, 0x0001,
	0x0001, 0x0001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000
};

static short N[64][32];

static void flushDecoder(DECODERPARAM *param)
{
	int i, k;

	for (i = 0; i < 2; i++)
		for (k = 0; k < 1024; k++)
			param->V[i][k] = 0;
}

static void initDecoder(DECODERPARAM *param)
{
	static int bInitialized = 0;
	int i, k;
	double PI64;

	param->V[0] = (short *)malloc(1024 * sizeof(short));
	param->V[1] = (short *)malloc(1024 * sizeof(short));
	if (!bInitialized)
	{
		bInitialized = 1;
		PI64 = 3.14159265358979 / 64.0;
		for (i = 0; i < 64; i++)
			for (k = 0; k < 32; k++)
				N[i][k] = cos((16 + i) * (2 * k + 1) * PI64) * (1 << 13);
	}
	flushDecoder(param);
}

static void uninitDecoder(DECODERPARAM *param)
{
	free(param->V[0]);
	free(param->V[1]);
}

static void synthesis(short *sbsamples, short *v, int nChannel, short *pPCM)
{
	int i, j;
	long sum;
	const short *pd;

	for (i = 1023; i >= 64; --i)  //shifting
		v[i] = v[i - 64];
	for (i = 0; i < 64; ++i)  //matrixing
	{
		sum = 0;
		for (j = 0; j < 32; ++j)
			sum += MULT_SS(sbsamples[j], N[i][j]);
		v[i] = sum;
	}
	for (j = 0, pd = D; j < 32; ++j, ++pd, ++v)  //calculate 32 samples
	{
		sum = 0;
		for (i = 0; i < 8 * 64; i += 64)  //window by 512 coefficients
			sum += v[i << 1] * pd[i] + v[(i << 1) + 96] * pd[i + 32];
		sum >>= 11;
		if (sum > 0x7FFF)
			sum = 0x7FFF;
		else if (sum < -0x8000)
			sum = -0x8000;
		pPCM[j * nChannel] = (short)sum;
	}
}

static void decode1(BITSTREAM *stream, HEADER *header, short **V, short *pPCM)
{
	int nChannel;
	int gr, sb, ch, nb;
	int allocation[2][32], scalefactor[2][32], sample;
	short fraction, sbsamples[2][32];
	int bound;

	nChannel = header->mode == 3? 1 : 2;
	if (header->mode != 1)  //stereo/dual_channel/single_channel
	{
		for (sb = 0; sb < 32; ++sb)  //bit allocation decoding
			for (ch = 0; ch < nChannel; ++ch)
				allocation[ch][sb] = bsread(stream, 4);
	}
	else  //intensity_stereo
	{
		bound = (header->mode_extension + 1) * 4;  //bound
		for (sb = 0; sb < bound; ++sb)  //bit allocation decoding
			for (ch = 0; ch < 2; ++ch)
				allocation[ch][sb] = bsread(stream, 4);
		for (sb = bound; sb < 32; ++sb)  //bit allocation decoding
			allocation[0][sb] = allocation[1][sb] = bsread(stream, 4);
	}
	for (sb = 0; sb < 32; ++sb)  //scalefactor decoding
		for (ch = 0; ch < nChannel; ++ch)
			if (allocation[ch][sb] != 0)
				scalefactor[ch][sb] = bsread(stream, 6);
	for (gr = 0; gr < 12; ++gr)
	{
		for (sb = 0; sb < 32; ++sb)  //requantization of subband samples
			for (ch = 0; ch < nChannel; ++ch)
			{
				nb = allocation[ch][sb];
				if (nb != 0)
				{
					++nb;
					sample = bsread(stream, nb);
					fraction = ((sample >> (nb - 1)) & 1) == 1? 0 : -1 << (nb - 1);
					fraction += sample & ((1 << (nb - 1)) - 1);
					fraction = ((fraction + 1) << nb) / ((1 << nb) - 1);
					fraction <<= 13 - (nb - 1);
					sbsamples[ch][sb] = MULT_SS(fraction, scalefactorTable[scalefactor[ch][sb]]);
				}
				else
					sbsamples[ch][sb] = 0;
			}

		for (ch = 0; ch < nChannel; ch++)
			synthesis(sbsamples[ch], V[ch], nChannel, pPCM + ch);  //synthesis subband filter
		pPCM += nChannel << 5;
	}  //for
}

static void decode2(BITSTREAM *stream, HEADER *header, short **V, short *pPCM)
{
	static int bitRateTable[16] = {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384};
	static BITALLOCATION bitAllocationTable[8] = {
		{4, {-1, 0, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
		{4, {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 16}},
		{3, {-1, 0, 1, 2, 3, 4, 5, 16, -1, -1, -1, -1, -1, -1, -1, -1}},
		{2, {-1, 0, 1, 16, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
		{4, {-1, 0, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}},
		{3, {-1, 0, 1, 3, 4, 5, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1}},
		{4, {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14}},
		{2, {-1, 0, 1, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}}
	};
	static int bitAllocationIndex[72] = {
		0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3,
		4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
	};
	static CLASSESOFQUANTIZATION classesOfQuantizationTable[17] = {
		{    3, 1, 1,  5,  2},
		{    5, 2, 1,  7,  3},
		{    7, 1, 0,  3,  3},
		{    9, 4, 1, 10,  4},
		{   15, 1, 0,  4,  4},
		{   31, 1, 0,  5,  5},
		{   63, 1, 0,  6,  6},
		{  127, 1, 0,  7,  7},
		{  255, 1, 0,  8,  8},
		{  511, 1, 0,  9,  9},
		{ 1023, 1, 0, 10, 10},
		{ 2047, 1, 0, 11, 11},
		{ 4095, 1, 0, 12, 12},
		{ 8191, 1, 0, 13, 13},
		{16383, 1, 0, 14, 14},
		{32767, 1, 0, 15, 15},
		{65535, 1, 0, 16, 16}
	};
	int nChannel, nBitRatePerChannel;
	int sblimit, sb, ch, gr, s, nlevel, nb, samplecode;
	int allocation[2][32], scfsi[2][32], scalefactor[2][32][3], sample[3];
	int *baIndex;
	CLASSESOFQUANTIZATION *quantization;
	short fraction, sbsamples[3][2][32];
	int bound;

	nChannel = header->mode == 3? 1 : 2;
	sblimit = 0;
	baIndex = NULL;
	if (header->version == 1)
	{
		nBitRatePerChannel = bitRateTable[header->bit_rate_index] / nChannel;
		if (header->sampling_frequency == 1)  //48 kHz
		{
			if (nBitRatePerChannel >= 56 && nBitRatePerChannel <= 192)
			{
				sblimit = 27;
				baIndex = bitAllocationIndex;
			}
			else if (nBitRatePerChannel >= 32 && nBitRatePerChannel <= 48)
			{
				sblimit = 8;
				baIndex = bitAllocationIndex + 30;
			}
		}
		else  //44.1 kHz or 32 kHz
		{
			if (nBitRatePerChannel >= 56 && nBitRatePerChannel <= 80)
			{
				sblimit = 27;
				baIndex = bitAllocationIndex;
			}
			else if (nBitRatePerChannel >= 96 && nBitRatePerChannel <= 192)
			{
				sblimit = 30;
				baIndex = bitAllocationIndex;
			}
			else if (nBitRatePerChannel >= 32 && nBitRatePerChannel <= 48)
			{
				sblimit = header->sampling_frequency == 0? 8 : 12;
				baIndex = bitAllocationIndex + 30;
			}
		}
	}
	else  //version 2
	{
		sblimit = 30;
		baIndex = bitAllocationIndex + 42;
	}

	if (header->mode != 1)  //stereo/dual_channel/single_channel
	{
		for (sb = 0; sb < sblimit; ++sb)  //bit allocation decoding
		{
			nb = bitAllocationTable[baIndex[sb]].nbal;
			for (ch = 0; ch < nChannel; ++ch)
				allocation[ch][sb] = bsread(stream, nb);
		}
	}
	else  //intensity_stereo
	{
		bound = (header->mode_extension + 1) << 2;  //bound
		for (sb = 0; sb < bound; ++sb)  //bit allocation decoding
		{
			nb = bitAllocationTable[baIndex[sb]].nbal;
			for (ch = 0; ch < 2; ++ch)
				allocation[ch][sb] = bsread(stream, nb);
		}
		for (sb = bound; sb < sblimit; ++sb)  //bit allocation decoding
			allocation[0][sb] = allocation[1][sb] = bsread(stream, bitAllocationTable[baIndex[sb]].nbal);
	}
	for (sb = sblimit; sb < 32; ++sb)
		for (ch = 0; ch < nChannel; ++ch)
			allocation[ch][sb] = 0;
	for (sb = 0; sb < sblimit; ++sb)  //scalefactor selection information decoding
		for (ch = 0; ch < nChannel; ++ch)
			if (allocation[ch][sb] != 0)
				scfsi[ch][sb] = bsread(stream, 2);
	for (sb = 0; sb < sblimit; ++sb)  //scalefactor decoding
		for (ch = 0; ch < nChannel; ++ch)
			if (allocation[ch][sb] != 0)
			{
				scalefactor[ch][sb][0] = bsread(stream, 6);
				if (scfsi[ch][sb] == 0)
				{
					scalefactor[ch][sb][1] = bsread(stream, 6);
					scalefactor[ch][sb][2] = bsread(stream, 6);
				}
				else if (scfsi[ch][sb] == 1)
				{
					scalefactor[ch][sb][1] = scalefactor[ch][sb][0];
					scalefactor[ch][sb][2] = bsread(stream, 6);
				}
				else if (scfsi[ch][sb] == 2)
				{
					scalefactor[ch][sb][1] = scalefactor[ch][sb][0];
					scalefactor[ch][sb][2] = scalefactor[ch][sb][0];
				}
				else
				{
					scalefactor[ch][sb][1] = bsread(stream, 6);
					scalefactor[ch][sb][2] = scalefactor[ch][sb][1];
				}
			}
	for (gr = 0; gr < 12; ++gr)
	{
		for (sb = 0; sb < 32; ++sb)  //requantization of subband samples
			for (ch = 0; ch < nChannel; ++ch)
				if (allocation[ch][sb] != 0)
				{
					nlevel = bitAllocationTable[baIndex[sb]].nlevels[allocation[ch][sb]];
					quantization = classesOfQuantizationTable + nlevel;
					nb = quantization->bitsPerCodeword;
					if (quantization->grouping == 1)
					{
						samplecode = bsread(stream, nb);
						for (s = 0; s < 3; ++s)
						{
							sample[s] = samplecode % quantization->C;
							samplecode /= quantization->C;
						}
					}
					else
						for (s = 0; s < 3; ++s)
							sample[s] = bsread(stream, nb);
					nb = quantization->msb;
					for (s = 0; s < 3; ++s)
					{
						fraction = ((sample[s] >> (nb - 1)) & 1) == 1? 0 : -1 << (nb - 1);
						fraction += sample[s] & ((1 << (nb - 1)) - 1);
						fraction = ((fraction + quantization->D) << nb) / quantization->C;  //requantize
						fraction <<= 13 - (nb - 1);
						sbsamples[s][ch][sb] = MULT_SS(fraction, scalefactorTable[scalefactor[ch][sb][gr >> 2]]);  //rescale
					}
				}
				else
					for (s = 0; s < 3; ++s)
						sbsamples[s][ch][sb] = 0;

		for (s = 0; s < 3; ++s)
		{
			for (ch = 0; ch < nChannel; ++ch)
				synthesis(sbsamples[s][ch], V[ch], nChannel, pPCM + ch);  //synthesis subband filter
			pPCM += nChannel << 5;
		}
	}  //for
}

static void decode3(BITSTREAM *stream, HEADER *header, short **V, short *pPCM)
{
	memset(pPCM, 0, (header->version == 2? 576 : 1152) * (header->mode == 3? 1 : 2) * sizeof(short));
	/*
	nChannel = mode == 3? 1 : 2;
	nValue = bsread(stream, 9);  //main data end
	nValue = bsread(stream, 3);  //private bits
	for (ch = 0; ch < nChannel; ch++)
		for (scfsi_band = 0; scfsi_band < 4; scfsi_band++)
			scfsi[scfsi_band][ch] = bsread(stream, 1);
	for (gr = 0; gr < 2; gr++)
		for (ch = 0; ch < nChannel; ch++)
		{
			part2_3_length[gr][ch] = bsread(stream, 12);
			big_values[gr][ch] = bsread(stream, 9);
			global_gain[gr][ch] = bsread(stream, 8);
			scalefac_compress[gr][ch] = bsread(stream, 4);
			blocksplit_flag[gr][ch] = bsread(stream, 1);
			if (blocksplit_flag[gr][ch])
			{
				block_type[gr][ch] = bsread(stream, 2);
				switch_point[gr][ch] = bsread(stream, 1);
				for (region = 0; region < 2; region++)
					table_select[region][gr][ch] = bsread(stream, 5);
				for (window = 0; window < 3; window++)
					subblock_gain[window][gr][ch] = bsread(stream, 3);
			}
			else
			{
				for (region = 0; region < 3; region++)
					table_select[region][gr][ch] = bsread(stream, 5);
				region_address1[gr][ch] = bsread(stream, 4);
				region_address2[gr][ch] = bsread(stream, 3);
			}
			preflag[gr][ch] = bsread(stream, 1);
			scalefac_scale[gr][ch] = bsread(stream, 1);
			count1table_select[gr][ch] = bsread(stream, 1);
		}

	for (gr = 0; gr < 2; gr++)
		for (ch = 0; ch < nChannel; ch++)
		{
			if (blocksplit_flag[gr][ch] == 1 && block_type[gr][ch] == 2)
			{
				for (cb = 0; cb < switch_point_l[gr][ch]; cb++)
					if (scfsi[cb] == 0 || gr == 0)
						scalefac[cb][gr][ch] = bsread(stream, 
				for (cb = switch_point_s[gr][ch]; cb < cblimit_short; cb++)
					for (window = 0; window < 3; window++)
						if (scfsi[cb] == 0 || gr == 0)
							scalefac[cb][window][gr][ch] = bsread(stream, 
			}
			else
				for (cb = 0; cb < cblimit; cb++)
					if (scfsi[cb] == 0 || gr == 0)
						scalefac[cb][gr][ch] = bsread(stream, 
			Huffman_code_bits = bsread(stream, part2_3_length[gr][ch] - part2_length);
		}
	*/
}

CMPEGAudioDecoder::CMPEGAudioDecoder()
{
	initDecoder(&m_Param);
}

CMPEGAudioDecoder::~CMPEGAudioDecoder()
{
	uninitDecoder(&m_Param);
}

bool CMPEGAudioDecoder::SetInputInfo(MediaType type, MediaInfo *pInfo)
{
	MPEGAudioInfo *pMPEGAudioInfo;

	if (pInfo->nSize < sizeof(MPEGAudioInfo))
		return false;

	pMPEGAudioInfo = (MPEGAudioInfo *)pInfo;
	if (pMPEGAudioInfo->nVersion == 0 || pMPEGAudioInfo->nLayer == 3)  //not support version 2.5 and layer 3
		return false;

	m_nSampleRate = pMPEGAudioInfo->nSampleRate;
	m_nChannel = pMPEGAudioInfo->nChannel;
	return true;
}

bool CMPEGAudioDecoder::GetOutputInfo(MediaInfo *pInfo)
{
	AudioRenderInfo *pAudioInfo;

	if (pInfo->nSize < sizeof(AudioRenderInfo))
		return false;

	pAudioInfo = (AudioRenderInfo *)pInfo;
	pAudioInfo->nSize = sizeof(AudioRenderInfo);
	pAudioInfo->nSampleRate = m_nSampleRate;
	pAudioInfo->nBitsPerSample = sizeof(short) << 3;
	pAudioInfo->nChannel = m_nChannel;
	return true;
}

int CMPEGAudioDecoder::GetInputSize(void)
{
	return 4096;
}

int CMPEGAudioDecoder::GetOutputSize(void)
{
	return 1152 * sizeof(short) * 2;
}

int CMPEGAudioDecoder::Decode(unsigned char *pInData, int nInSize, void *pOutData)
{
	static int versionTable[] = {0, -1, 2, 1};  //2.5, reserved, 2, 1
	BITSTREAM *stream;
	HEADER header;
	int layer, protection_bit, padding_bit;
	int nValue, nSamples;

	m_Param.pInData = pInData;
	m_Param.nInSize = nInSize;
	stream = bsopen(ReadByte, &m_Param);
	//header
	nValue = bsread(stream, 11);  //syncword
	header.version = versionTable[bsread(stream, 2)];
	layer = 4 - bsread(stream, 2);
	protection_bit = bsread(stream, 1);
	header.bit_rate_index = bsread(stream, 4);
	header.sampling_frequency = bsread(stream, 2);
	padding_bit = bsread(stream, 1);
	nValue = bsread(stream, 1);  //private bit
	header.mode = bsread(stream, 2);
	header.mode_extension = bsread(stream, 2);
	nValue = bsread(stream, 1);  //copyright
	nValue = bsread(stream, 1);  //original/home
	nValue = bsread(stream, 2);  //emphasis
	//error_check
	if (protection_bit == 0)
		nValue = bsread(stream, 16);  //crc check
	//audio_data
	if (layer == 1)
	{
		decode1(stream, &header, m_Param.V, (short *)pOutData);
		nSamples = 384;
	}
	else if (layer == 2)
	{
		decode2(stream, &header, m_Param.V, (short *)pOutData);
		nSamples = 1152;
	}
	else if (layer == 3)
	{
		decode3(stream, &header, m_Param.V, (short *)pOutData);
		nSamples = header.version == 2? 576 : 1152;
	}
	bsclose(stream);

	return nSamples * (header.mode == 3? 1 : 2) * sizeof(short);
}

void CMPEGAudioDecoder::Flush(void)
{
	flushDecoder(&m_Param);
}

int CMPEGAudioDecoder::ReadByte(void *pData)
{
	DECODERPARAM *pParam;

	pParam = (DECODERPARAM *)pData;
	if (pParam->nInSize == 0)
		return -1;

	--pParam->nInSize;
	return *(pParam->pInData)++;
}
