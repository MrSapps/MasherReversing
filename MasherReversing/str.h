#pragma once

const int kSectorSize = 2336;
const int kNumSectorsPerSecond = 150;
// FPS is 15

struct SectorHeader
{
    // MOIR is video, VALE is audio
    DWORD mSectorType;
    DWORD mSectorNumber;
};

struct PsxStrVideoHeader
{
    /*
    0000  char   {4}     "AKIK"
    0004  uint16 {2}     Sector number within frame (zero-based)
    0006  uint16 {2}     Number of sectors in frame
    0008  uint32 {4}     Frame number within file (one-based)
    000C  uint32 {4}     Frame data length
    0010  uint16 {2}     Video Width
    0012  uint16 {2}     Video Height
    0014  uint16 {2}     Number of MDEC codes divided by two, and rounded up to a multiple of 32
    0016  uint16 {2}     Always 0x3800
    0018  uint16 {2}     Quantization level of the video frame
    001A  uint16 {2}     Version of the video frame (always 2)
    001C  uint32 {4}     Always 0x00000000
    */
};

struct MasherVideoHeaderWrapper
{
    DWORD mMagic; // AKAIK
    PsxStrVideoHeader mStrHeader;
};

/*
0000  uint16 {2}     Number of MDEC codes divided by two, and rounded up to a multiple of 32
0002  uint16 {2}     Always 0x3800
0004  uint16 {2}     Quantization level of the video frame
0006  uint16 {2}     Version of the video frame (always 2)
*/

// The video is encoded using the same encoding as used by the PSX STR format.

// Each VALE sector contains 18 groups of 128 bytes of audio data encoded using the same 37.8kHz 4-bit stereo CDXA ADPCM as the PSX STR format.

class Str
{
public:

};
