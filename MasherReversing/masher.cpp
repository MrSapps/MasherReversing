#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

struct DDVHeader
{
    uint32_t ddvTag;
    uint32_t ddvVersion;
    uint32_t contains;          // 0x3 = audio and video, 0x1 = video 0x2 = audio (might be other way round)
    uint32_t frameRate;
    uint32_t frameNumber;
    uint32_t field5;
    uint32_t width;
    uint32_t height;
    uint32_t field8;
    uint32_t field9;
    uint32_t keyFrameRate;
    uint32_t audioFormat;
    uint32_t sampleRate;
    uint32_t fieldD;
    uint32_t fieldE;
    uint32_t framesInterleave;
};

int main(int, char**)
{
    FILE* fp = fopen("MIP01C03.DDV", "rb");
    //FILE* fp = fopen("Masher/GDENDING.DDV", "rb");

    DDVHeader header;
    fread(&header, 0x40, 1, fp);

    uint32_t audioArraySize = header.framesInterleave * 4;
    uint32_t videoArraySize = header.frameNumber * 4;

    uint32_t* pAudioFrameSizes = (uint32_t*)malloc(audioArraySize);
    uint32_t* pVideoFrameSizes = (uint32_t*)malloc(videoArraySize);

    fread(pAudioFrameSizes, audioArraySize, 1, fp);
    fread(pVideoFrameSizes, videoArraySize, 1, fp);

    uint8_t** ppAudioFrames = (uint8_t**)malloc(header.framesInterleave * sizeof(uint8_t*));
    uint8_t** ppVideoFrames = (uint8_t**)malloc(header.frameNumber * sizeof(uint8_t*));

    for (uint32_t frame = 0; frame < header.framesInterleave; frame++)
    {
        ppAudioFrames[frame] = (uint8_t*)malloc(pAudioFrameSizes[frame]);
        fread(ppAudioFrames[frame], pAudioFrameSizes[frame], 1, fp);
    }

    for (uint32_t frame = 0; frame < header.frameNumber; frame++)
    {
        uint16_t someOffset;
        fread(&someOffset, 2, 1, fp);

        ppVideoFrames[frame] = (uint8_t*)malloc(pVideoFrameSizes[frame] + 2);
        fread(ppVideoFrames[frame], pVideoFrameSizes[frame] + 2, 1, fp);

        uint8_t* pAudio = &ppVideoFrames[frame][someOffset + 2];

        int nDebug = 0;
    }





    // Free everything

    for (unsigned int frame = 0; frame < header.frameNumber; frame++)
    {
        free(ppVideoFrames[frame]);
    }
    for (unsigned int frame = 0; frame < header.framesInterleave; frame++)
    {
        free(ppAudioFrames[frame]);
    }

    free(ppVideoFrames);
    free(ppAudioFrames);
    free(pVideoFrameSizes);
    free(pAudioFrameSizes);
    fclose(fp);
    return 0;

}
