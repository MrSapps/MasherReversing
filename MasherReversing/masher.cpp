#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <SDL.h>

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

int StartSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Masher", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (win == nullptr)
    {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr)
    {
        SDL_DestroyWindow(win);
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Event e;
    bool quit = false;
    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            if (e.type == SDL_KEYDOWN)
            {
                quit = true;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                quit = true;
            }
        }

        SDL_RenderClear(ren);
        SDL_RenderPresent(ren);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}


int main(int, char**)
{

    StartSDL();

    FILE* fp = fopen("Testing.DDV", "rb");
    //FILE* fp = fopen("Masher/GDENDING.DDV", "rb");

    DDVHeaderPart1 headerP1 = {};
    fread(&headerP1, sizeof(headerP1), 1, fp);

    DDVHeaderPart2 headerP2 = {};
    fread(&headerP2, sizeof(headerP2), 1, fp);

    // Only read audio section if audio is present in the file
    DDVHeaderPart3 headerP3 = {};
    const bool bHasAudio = (headerP1.contains & 0x2) == 0x2;
    if (bHasAudio)
    {
        fread(&headerP3, sizeof(headerP3), 1, fp);
    }

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
        for (uint32_t frame = 0; frame < headerP1.numberOfFrames; frame++)
        {
            uint16_t someOffset = 0;
            fread(&someOffset, 2, 1, fp);

            ppVideoFrames[frame].resize(pVideoFrameSizes[frame] + 2);
            fread(ppVideoFrames[frame].data(), pVideoFrameSizes[frame] + 2, 1, fp);

            if (someOffset + 2 >= ppVideoFrames[frame].size())
            {
                std::cout << someOffset + 2 << " is out of bounds on frame " << frame << std::endl;
            }
            else
            {
                uint8_t* pAudio = &ppVideoFrames[frame][someOffset + 2];

                int nDebug = 0;
            }
        }
    }
    else
    {
        for (uint32_t frame = 0; frame < headerP1.numberOfFrames; frame++)
        {
          
            ppVideoFrames[frame].resize(pVideoFrameSizes[frame]);
            fread(ppVideoFrames[frame].data(), pVideoFrameSizes[frame], 1, fp);

        
        }
    }

    
    fclose(fp);

    return 0;

}
