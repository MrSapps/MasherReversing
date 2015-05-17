#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <SDL.h>
#include "Hooks.hpp"

struct DDVHeaderPart1
{
    uint32_t ddvTag;

    // offset 4
    uint32_t ddvVersion;

    // offset 8
    uint32_t contains;          // 0x3 = audio and video, 0x1 = video 0x2 = audio (might be other way round)
    uint32_t frameRate;
    uint32_t numberOfFrames;
};

struct DDVHeaderPart2
{
    uint32_t field5; // Probably a reserved field, it has no effect and isn't read by masher lib
    uint32_t width;
    uint32_t height;


    uint32_t maxVideoFrameSize; // must add 8 to this for some reason
    uint32_t field9; // size of a buffer that contains shorts, i.e read field9 * 2
    uint32_t keyFrameRate;
};

struct DDVHeaderPart3
{
    uint32_t audioFormat;
    uint32_t sampleRate;
    uint32_t maxAudioFrameSize;
    uint32_t fieldE; // size of 1 audio channel?

    uint32_t framesInterleave;
};

extern Uint16 pixels[240 * 320];


int main(int, char**)
{
    StartSDL();

   // FILE* fp = fopen("Testing.DDV", "rb");
    FILE* fp = fopen("Masher/GDENDING.DDV", "rb");

    DDVHeaderPart1 headerP1 = {};
    fread(&headerP1, sizeof(headerP1), 1, fp);

    DDVHeaderPart2 headerP2 = {};
    fread(&headerP2, sizeof(headerP2), 1, fp);

    // Only read audio section if audio is present in the file
    DDVHeaderPart3 headerP3 = {};
    const bool bHasAudio = (headerP1.contains & 0x2) == 0x2;
    const bool bHasVideo = (headerP1.contains & 0x1) == 0x1;
    if (bHasAudio)
    {
        fread(&headerP3, sizeof(headerP3), 1, fp);
    }

    ddv_class ddv = {};
    ddv.ddvVersion = headerP1.ddvVersion;
    ddv.contains = headerP1.contains;
    ddv.frameRate = headerP1.frameRate;
    ddv.mNumberOfFrames = headerP1.numberOfFrames;

    ddv.field5 = headerP2.field5;
    ddv.width = headerP2.width;
    ddv.height = headerP2.height;
    ddv.maxAudioFrameSize = headerP2.maxVideoFrameSize;
    ddv.field9 = headerP2.field9;
    ddv.keyFrameRate = headerP2.keyFrameRate;
    
    if (bHasAudio)
    {
        ddv.audioFormat = headerP3.audioFormat;
        ddv.sampleRate = headerP3.sampleRate;
        ddv.maxAudioFrameSize = headerP3.maxAudioFrameSize;
        ddv.fieldE = headerP3.fieldE;
        ddv.framesInterleave = headerP3.framesInterleave;
    }

    ddv.mHasAudio = bHasAudio;
    ddv.mHasVideo = bHasVideo;

    ddv.nNumMacroblocksX = (ddv.width >> 4);
    ddv.nNumMacroblocksY = (ddv.height >> 4);

    ddv.mBlockDataSize_q = 64; // what should this be??

    ddv.mDecodedBitStream = (DWORD)malloc(40000*50);
    ddv.mMacroBlockBuffer_q = (DWORD)malloc(4000000*50);

    /*
    HANDLE mFileHandle;
    DWORD mRawFrameBitStreamData;
    DWORD mDecodedBitStream;
    DWORD mLastUsedFrameBuffer;
    DWORD mUnknownBuffer4;
    DWORD mAudioFrameSizeBytesQ;
    DWORD mAudioFrameSizeBitsQ;


    DWORD mCurrentFrameNumber;
    DWORD mCurrentFrameNumber2;
    DWORD field_6C; // Some sort of counter
    DWORD mUnknownBuffer1;
    DWORD mFrameSizesArray;
    DWORD field_78;
    DWORD field_7C;
    DWORD mCurrentFrameBuffer;
    DWORD field_84;
    DWORD mSizeOfWhatIsReadIntoMUnknownBuffer2;
    DWORD mMacroBlockBuffer_q;
    DWORD mBlockDataSize_q;
    */

    const uint32_t audioArraySize = headerP3.framesInterleave;
    const uint32_t videoArraySize = headerP1.numberOfFrames;

    std::vector<uint32_t> pAudioFrameSizes(audioArraySize);
    std::vector<uint32_t> pVideoFrameSizes(videoArraySize);

    fread(pAudioFrameSizes.data(), audioArraySize * sizeof(uint32_t), 1, fp);
    fread(pVideoFrameSizes.data(), videoArraySize * sizeof(uint32_t), 1, fp);

    std::vector<std::vector<uint8_t>> ppAudioFrames(headerP3.framesInterleave);
    std::vector<std::vector<uint8_t>> ppVideoFrames(headerP1.numberOfFrames);

    for (uint32_t frame = 0; frame < headerP3.framesInterleave; frame++)
    {
        ppAudioFrames[frame].resize(pAudioFrameSizes[frame]);
        fread(ppAudioFrames[frame].data(), pAudioFrameSizes[frame], 1, fp);
    }

    if (bHasAudio)
    {
        for (uint32_t frame = headerP3.framesInterleave; frame < headerP1.numberOfFrames; frame++)
        {
            ppVideoFrames[frame].resize(pVideoFrameSizes[frame] + 4);
            fread(ppVideoFrames[frame].data(), pVideoFrameSizes[frame] + 4, 1, fp);

            uint8_t* pAudio = &ppVideoFrames[frame][0];

            char buffer[90000] = {};
            fseek(fp, 0x609D, 0);
            fread(buffer, 1, 90000, fp);

            /*
            0x04DDD19C  02 00 10 00 a0 00 00 05 01 28 08 40 40 00 80 02  .... ....(.@@.€.
            0x04DDD1AC  00 14 05 a0 20 00 00 01 00 0a 02 50 14 80 80 00  ...  ......P.€€.
            */

            // First offset is frame 5 at 0x609D for GENDING
            ddv.mRawFrameBitStreamData = (DWORD)buffer;


            decode_ddv_frame(nullptr, &ddv, (unsigned char *)pixels);
           // FlipSDL();
            SDL_Delay(100);




        }
    }
    else
    {
        for (uint32_t frame = 0; frame < headerP1.numberOfFrames; frame++)
        {
          
            ppVideoFrames[frame].resize(pVideoFrameSizes[frame]);
            fread(ppVideoFrames[frame].data(), pVideoFrameSizes[frame], 1, fp);

            ddv.mRawFrameBitStreamData = (DWORD)ppVideoFrames[frame].data();


            decode_ddv_frame(nullptr, &ddv, (unsigned char *)pixels);
            FlipSDL();
            SDL_Delay(1000);
        }
    }

    
    fclose(fp);

    return 0;

}
