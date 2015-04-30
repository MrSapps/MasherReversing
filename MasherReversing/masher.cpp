#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>

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

// Frame data for 16x8 1 frame DDV where one block of 8x8 is black and the other
// is white is:
// 01 00 10 00 A0 00 27 11 AB 2B 3F 9B 04 0A 40 01 
// 01 43 80 49 01 8E 7E A2 FF 13 00 00

// ddv_play_frame
// -> calls_decode_block_q
//    -> decode_bitstream_q < decodes the bitstream with a (FE F0) terminated buffer?
// -> ddv_func7_DecodeMacroBlock x6
// Maybe before or after
// -> j_ddv__func5_block_decoder_q
//   -> ddv__func5_block_decoder_q
//      -> Either x6 blit_output_mmx_q or x6 blit_output_no_asm_q
// depending on supported cpu features. Before each blitter  func
// ddv_func7_DecodeMacroBlock is called
// finally depending on some other global set of flags  sub_40A2C0 or
// the likes will be called, the others seem to skip blocks or change the overall
// video height.

// after bit stream decoding we seem to end up with:
// 00 00 00 FE 00 00 00 FE 02 04 FE 0F 01 00 01 08
// FF 03 01 08 FF 03 FF 13 00 FE F0 03 00 FE 08 04
// 07 00 05 0C 05 00 03 1C 02 00 00 FE F0 03 00 FE << I assume "FE F0" is the end, or perhaps the previous "FE F0" instance
// 00 00 00 00 00 00 00 00 41 00 00 00 31 00 00 00
// 80 B8 42 00 10 01 F4 01 1C 00 00 00 00 00 00 00
// 01 00 00 00 0C 04 00 00 10 04 00 00 88 05 00 00
// 00 00 00 00 00 00 00 00 31 00 00 00 A1 00 00 00
// A0 01 F4 01 01 00 00 00 01 00 00 00 0F 00 00 00
// 01 00 00 00 00 00 00 00 10 00 00 00 08 00 00 00
// 1A 00 00 00 18 00 00 00 0F 00 00 00 00 00 00 00
// 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
// 10 01 F4 01 60 01 F4 01 00 00 00 00 00 00 00 00
// 00 00 00 00 00 00 00 00 01 00 00 00 01 00 00 00
// 00 01 00 00 00 00 00 00 02 00 00 00 01 00 00 00
// 00 01 F4 01 04 01 F4 01 00 00 00 00 00 00 00 00
// 10 01 F4 01 22 00 00 00 00 00 00 00 E0 60 07 02
// 40 00 00 00 00 00 00 00 A1 00 00 00 91 00 00 00
// 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
// 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
// 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
// 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
// 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
// 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
// 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
// 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
// 00 00 00 00 00 00 00 00 91 00 00 00 41 00 00 00 << Static ascii strings after this data, so this is the max possible size


int main(int, char**)
{
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
