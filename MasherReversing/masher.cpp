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

static void PlayDDV(const char* fileName)
{
    FILE* fp = fopen(fileName, "rb");

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

    ddv.mDecodedBitStream = (DWORD)malloc(40000 * 50); // TODO: correct size
    ddv.mMacroBlockBuffer_q = (DWORD)malloc(4000000 * 50); // TODO: Correct size

    const uint32_t audioArraySize = headerP3.framesInterleave;
    const uint32_t videoArraySize = headerP1.numberOfFrames;

    std::vector<uint32_t> pAudioFrameSizes(audioArraySize);
    std::vector<uint32_t> pVideoFrameSizes(videoArraySize);

    fread(pAudioFrameSizes.data(), audioArraySize, sizeof(uint32_t), fp);
    fread(pVideoFrameSizes.data(), videoArraySize, sizeof(uint32_t), fp);

    std::vector<std::vector<uint8_t>> ppAudioFrames(headerP3.framesInterleave);
    std::vector<std::vector<uint8_t>> ppVideoFrames(headerP1.numberOfFrames);

    for (uint32_t frame = 0; frame < headerP3.framesInterleave; frame++)
    {
        ppAudioFrames[frame].resize(pAudioFrameSizes[frame]);
        fread(ppAudioFrames[frame].data(), pAudioFrameSizes[frame], 1, fp);
    }

    if (bHasAudio)
    {
        int c = 0;
        for (uint32_t frame = 0; frame < headerP1.numberOfFrames; frame++)
        {
            SDL_Event event = {};
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                case SDL_WINDOWEVENT:
                    switch (event.window.event)
                    {
                    case SDL_WINDOWEVENT_ENTER:
                        break;

                    case SDL_WINDOWEVENT_LEAVE:
                        break;
                    }
                    break;

                case SDL_KEYDOWN:
                    break;
                }
            }

            // IF audio then what is first DWORD? Offset within frame to audio data?

            unsigned int frameSize = pVideoFrameSizes[frame] + 4; // Always +4 if audio
            ppVideoFrames[frame].resize(frameSize);
            fread(ppVideoFrames[frame].data(), frameSize, 1, fp);

            uint8_t* pVideo = &ppVideoFrames[frame][4];
            ddv.mRawFrameBitStreamData = (DWORD)pVideo;

            decode_ddv_frame(nullptr, &ddv, (unsigned char *)pixels);
            //SDL_Delay(16*2);
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

    free((void*)ddv.mDecodedBitStream);
    free((void*)ddv.mMacroBlockBuffer_q);

}

int main(int, char**)
{
    StartSDL();

    // All public DDVs known to man
    std::vector<std::string> ddvs = 
    {
        "ASLIKEXP.DDV",
        "ASLIKINF.DDV",
        "BA1114.DDV",
        "BACKSTRY.DDV",
        "BDENDING.DDV",
        "BREW.DDV",
        "BREWCAMT.DDV",
        "BRP01C01.DDV",
        "BRP01C02.DDV",
        "BRP01C03.DDV",
        "BRP01C04.DDV",
        "BRP01C05.DDV",
        "BRP01C06.DDV",
        "BRP01C1R.DDV",
        "BRP01C2R.DDV",
        "BRP01C3R.DDV",
        "BRP01C4R.DDV",
        "BRP01C5R.DDV",
        "BRP01C6R.DDV",
        "BWP3P4.DDV",
        "BWP3P4R.DDV",
        "CONFRNCE.DDV",
        "Ddlogo.ddv",
        "DRIPEXP.DDV",
        "DRIPINF.DDV",
        "ESCAPE.DDV",
        "FEECO.DDV",
        "GDENDING.DDV",
        "gtilogo.ddv",
        "INFBOOTH.DDV",
        "INGRDNT.DDV",
        "INTRO.DDV",
        "logo.ddv",
        "M1018310.DDV",
        "M1018517.DDV",
        "M5171018.DDV",
        "MI122711.DDV",
        "MI218227.DDV",
        "MI221222.DDV",
        "MI221227.DDV",
        "MI222221.DDV",
        "MI227218.DDV",
        "MI227221.DDV",
        "MI313401.DDV",
        "Mi404430.ddv",
        "MI410313.DDV",
        "MI419431.DDV",
        "MI420432.DDV",
        "MI422423.DDV",
        "MI422437.DDV",
        "MI426427.DDV",
        "Mi430044.ddv",
        "MI431419.DDV",
        "Mi437422.ddv",
        "MI711122.DDV",
        "MIP01C03.DDV",
        "MP1C1C2.DDV",
        "MP1C2C1.DDV",
        "NE140601.DDV",
        "NE140604.DDV",
        "NE310033.DDV",
        "NE330309.DDV",
        "NE410404.DDV",
        "NE440401.DDV",
        "NE610104.DDV",
        "NE610602.DDV",
        "NE620603.DDV",
        "NE630604.DDV",
        "NE640401.DDV",
        "NE640601.DDV",
        "NEBLK201.DDV",
        "NEP01C08.DDV",
        "NEP01C09.DDV",
        "NEP01C10.DDV",
        "NEP01C8R.DDV",
        "NEP02C01.DDV",
        "NEP02C02.DDV",
        "NEP1C10R.DDV",
        "option.ddv",
        "P1391314.DDV",
        "PHLEGEXP.DDV",
        "PHLEGINF.DDV",
        "prophecy.ddv",
        "PV107071.DDV",
        "PV114081.DDV",
        "PV12181R.DDV",
        "PV131452.DDV",
        "PV370801.DDV",
        "PV410801.DDV",
        "PV430402.DDV",
        "PV470801.DDV",
        "PV550801.DDV",
        "PV712101.DDV",
        "Pv801901.ddv",
        "PV810301.DDV",
        "PV810401.DDV",
        "PV810501.DDV",
        "PV810901.DDV",
        "PV811101.DDV",
        "PV811201.DDV",
        "Pv821301.ddv",
        "PV910801.DDV",
        "PVP01C5R.DDV",
        "REWARD.DDV",
        "S1102N58.DDV",
        "SBP01C01.DDV",
        "SBP01C1R.DDV",
        "SSINFO.DDV",
        "Sv101112.ddv",
        "SV110703.DDV",
        "SV140106.DDV",
        "SV160703.ddv",
        "SV22073L.DDV",
        "SV36073R.DDV",
        "SV420405.DDV",
        "SV430703.DDV",
        "SV520504.DDV",
        "SV550507.DDV",
        "SV590703.DDV",
        "SV616618.DDV",
        "SV619622.DDV",
        "SV624063.DDV",
        "SV630609.DDV",
        "SV690613.DDV",
        "SV730101.DDV",
        "SV730202.DDV",
        "SV730306.DDV",
        "SV730401.DDV",
        "SV730502.DDV",
        "SV730801.DDV",
        "SV740904.DDV",
        "SV830703.DDV",
        "SV940704.DDV",
        "SVP01C05.DDV",
        "TRAIN1.DDV",
        "TRAIN2.DDV",
        "train3.ddv",
        "v1a4s01.ddv",
        "vision.ddv"
    };

    // TODO: gtilogo.ddv and prophecy.ddv do not render correctly
    std::string abesExoddusDir = "C:\\Program Files (x86)\\Steam\\SteamApps\\common\\Oddworld Abes Exoddus\\";
    for (auto& file : ddvs)
    {
        std::cout << "Playing: " << file.c_str() << std::endl;
        PlayDDV((abesExoddusDir+file).c_str());
    }

    StopSDL();

    return 0;

}
