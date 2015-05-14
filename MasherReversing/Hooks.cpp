#include "Hooks.hpp"
#include "jmphookedfunction.hpp"
#include <algorithm>
#include <string>
#include <set>
#include <stdint.h>
#include <SDL.h>
#include <iostream>

#undef min
#undef max
#undef RGB

SDL_Window *win = nullptr;
SDL_Renderer *ren = nullptr;
SDL_Texture *sdlTexture = nullptr;

int StartSDL()
{
    if (win) { return 0; }

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    win = SDL_CreateWindow("Masher", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (win == nullptr)
    {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr)
    {
        SDL_DestroyWindow(win);
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
   
    sdlTexture = SDL_CreateTexture(ren,
        SDL_PIXELFORMAT_RGB565,
        SDL_TEXTUREACCESS_STREAMING,
        320, 240);

    return 0;
}

Uint16 pixels[240*320] = {};

void FlipSDL()
{
    SDL_UpdateTexture(sdlTexture, NULL, pixels, 320 * sizeof(Uint16));

    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, sdlTexture, NULL, NULL);
    SDL_RenderPresent(ren);
}

void StopSDL()
{
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}


#pragma pack(1)
#pragma pack(push)
struct ddv_class
{
    HANDLE mFileHandle;
    DWORD ddvVersion;
    DWORD contains;
    DWORD frameRate;
    DWORD mNumberOfFrames;
    DWORD field5;
    DWORD width;
    DWORD height;
    DWORD maxVideoFrameSize;
    DWORD field9;
    DWORD keyFrameRate;
    DWORD audioFormat;
    DWORD sampleRate;
    DWORD maxAudioFrameSize;
    DWORD fieldE;
    DWORD framesInterleave; 
    DWORD mRawFrameBitStreamData;
    DWORD mDecodedBitStream;
    DWORD mLastUsedFrameBuffer;
    DWORD mUnknownBuffer4;
    DWORD mAudioFrameSizeBytesQ;
    DWORD mAudioFrameSizeBitsQ;
    DWORD nNumMacroblocksX;
    DWORD nNumMacroblocksY;
    BYTE mHasAudio;
    BYTE mHasVideo;
    BYTE field_62;      // Padding?
    BYTE field_63;      // Padding?
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
};
#pragma pack(pop)

static_assert(sizeof(ddv_class) == 0x94, "Structure size must match exactly!");

// We can't compile the hook stubs as __thiscall, so __fastcall is used as a workaround/hack
char __fastcall decode_ddv_frame(void* hack, ddv_class *thisPtr, unsigned char* screenBuffer);
typedef decltype(&decode_ddv_frame) ddv__func5_block_decoder_q_type;

static ddv__func5_block_decoder_q_type real_ddv__func5_block_decoder_q = (ddv__func5_block_decoder_q_type)0x00409FE0;
static JmpHookedFunction<ddv__func5_block_decoder_q_type>* ddv_func6_decodes_block_q_hook;

void idct(int16_t* pSource, int32_t* pDestination);

static void do_blit_output_no_mmx(int macroBlockBuffer, int* decodedBitStream)
{
    idct((int16_t*)macroBlockBuffer, decodedBitStream);
}

static int To1d(int x, int y)
{
    // 8x8 index to x64 index
    return y * 8 + x;
}

void SetElement(int x, int y, unsigned short int* ptr, unsigned short int value);

// This can probably be made more generic still, but probably no point.
// We base stuff off of this:
template <int r_bits, int g_bits, int b_bits>
inline unsigned short RGB(unsigned char r, unsigned char g, unsigned char b) {
    return ((((r >> (8 - r_bits)) << g_bits) |
        (g >> (8 - g_bits))) << b_bits) |
        (b >> (8 - b_bits));
}

// We can make wrappers like this to handle other colour orders:
template <int r_bits, int g_bits, int b_bits>
inline unsigned short BGR(unsigned char r, unsigned char g, unsigned char b) {
    return RGB<b_bits, g_bits, r_bits>(b, g, r);
}

// And then more wrappers for syntactic sugar:
inline unsigned short RGB565(unsigned char r, unsigned char g, unsigned char b) {
    return RGB<5, 6, 5>(r, g, b);
}
unsigned char Clamp(float v)
{
    if (v < 0.0f) v = 0.0f;
    if (v > 255.0f) v = 255.0f;
    return (unsigned char)v;
};

static void ConvertYuvToRgbAndBlit(unsigned short int* pFrameBuffer, int xoff, int yoff);
int __cdecl decode_bitstream(WORD *pFrameData, unsigned short int *pOutput);

void half_idct(int32_t* pSource, int32_t* pDestination, int nPitch, int nIncrement, int nShift)
{
    int32_t pTemp[8];

    for (int i = 0; i < 8; i++)
    {
        pTemp[4] = pSource[0 * nPitch] * 8192 + pSource[2 * nPitch] * 10703 + pSource[4 * nPitch] * 8192 + pSource[6 * nPitch] * 4433;
        pTemp[5] = pSource[0 * nPitch] * 8192 + pSource[2 * nPitch] * 4433 - pSource[4 * nPitch] * 8192 - pSource[6 * nPitch] * 10704;
        pTemp[6] = pSource[0 * nPitch] * 8192 - pSource[2 * nPitch] * 4433 - pSource[4 * nPitch] * 8192 + pSource[6 * nPitch] * 10704;
        pTemp[7] = pSource[0 * nPitch] * 8192 - pSource[2 * nPitch] * 10703 + pSource[4 * nPitch] * 8192 - pSource[6 * nPitch] * 4433;

        pTemp[0] = pSource[1 * nPitch] * 11363 + pSource[3 * nPitch] * 9633 + pSource[5 * nPitch] * 6437 + pSource[7 * nPitch] * 2260;
        pTemp[1] = pSource[1 * nPitch] * 9633 - pSource[3 * nPitch] * 2259 - pSource[5 * nPitch] * 11362 - pSource[7 * nPitch] * 6436;
        pTemp[2] = pSource[1 * nPitch] * 6437 - pSource[3 * nPitch] * 11362 + pSource[5 * nPitch] * 2261 + pSource[7 * nPitch] * 9633;
        pTemp[3] = pSource[1 * nPitch] * 2260 - pSource[3 * nPitch] * 6436 + pSource[5 * nPitch] * 9633 - pSource[7 * nPitch] * 11363;

        pDestination[0 * nPitch] = (pTemp[4] + pTemp[0]) >> nShift;
        pDestination[1 * nPitch] = (pTemp[5] + pTemp[1]) >> nShift;
        pDestination[2 * nPitch] = (pTemp[6] + pTemp[2]) >> nShift;
        pDestination[3 * nPitch] = (pTemp[7] + pTemp[3]) >> nShift;
        pDestination[4 * nPitch] = (pTemp[7] - pTemp[3]) >> nShift;
        pDestination[5 * nPitch] = (pTemp[6] - pTemp[2]) >> nShift;
        pDestination[6 * nPitch] = (pTemp[5] - pTemp[1]) >> nShift;
        pDestination[7 * nPitch] = (pTemp[4] - pTemp[0]) >> nShift;

        pSource += nIncrement;
        pDestination += nIncrement;
    }
}

// 0x40ED90
void idct(int16_t* pSource, int32_t* pDestination)
{
    int32_t pTemp[64];
    int32_t pExtendedSource[64];

    // Source is passed as signed 16 bits stored every 32 bits
    // We sign extend it at the beginning like Masher does
    for (int i = 0; i < 64; i++)
    {
        pExtendedSource[i] = pSource[i * 2];
    }

    half_idct(pExtendedSource, pTemp, 8, 1, 11);
    half_idct(pTemp, pDestination, 1, 8, 18);
}

DWORD* gBlockWidthQ_before_635A0C = (DWORD*)0x635A08;
DWORD* gBlockHeightQ_before_63580C = (DWORD*)0x635808;

DWORD* gQuant1_dword_42AEC8 = (DWORD*)0x42AEC8;
DWORD* gQaunt2_dword_42AFC4 = (DWORD*)0x42AFC4;

DWORD *g_252_buffer_unk_635A0C = (DWORD*)0x635A0C;
DWORD *g_252_buffer_unk_63580C = (DWORD*)0x63580C;

/*
0x00635A0C  24 00 00 00, 24 00 00 00, 30 00 00 00, 2a 00 00 00, 30 00 00 00, 5e 00 00 00, 34 00 00 00, 34 00 00 00  $...$...0...*...0...^...4...4...
0x00635A2C  5e 00 00 00, c6 00 00 00, 84 00 00 00, 70 00 00 00, 84 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00  ^...Æ.......p.......Æ...Æ...Æ...
0x00635A4C  c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00  Æ...Æ...Æ...Æ...Æ...Æ...Æ...Æ...
0x00635A6C  c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00  Æ...Æ...Æ...Æ...Æ...Æ...Æ...Æ...
0x00635A8C  c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00  Æ...Æ...Æ...Æ...Æ...Æ...Æ...Æ...
0x00635AAC  c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00  Æ...Æ...Æ...Æ...Æ...Æ...Æ...Æ...
0x00635ACC  c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00  Æ...Æ...Æ...Æ...Æ...Æ...Æ...Æ...
0x00635AEC  c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, c6 00 00 00, 00 00 00 00
*/

/*
0x0063580C  18 00 00 00 16 00 00 00 14 00 00 00 18 00 00 00 1c 00 00 00 1c 00 00 00 1a 00 00 00 1c 00 00 00  ................................
0x0063582C  20 00 00 00 30 00 00 00 26 00 00 00 20 00 00 00 22 00 00 00 24 00 00 00 30 00 00 00 2c 00 00 00   ...0...&... ..."...$...0...,...
0x0063584C  2c 00 00 00 30 00 00 00 34 00 00 00 50 00 00 00 66 00 00 00 74 00 00 00 50 00 00 00 3a 00 00 00  ,...0...4...P...f...t...P...:...
0x0063586C  4a 00 00 00 46 00 00 00 62 00 00 00 90 00 00 00 80 00 00 00 6e 00 00 00 70 00 00 00 66 00 00 00  J...F...b.......€...n...p...f...
0x0063588C  72 00 00 00 78 00 00 00 7a 00 00 00 6e 00 00 00 8a 00 00 00 ae 00 00 00 88 00 00 00 80 00 00 00  r...x...z...n...Š...®...ˆ...€...
0x006358AC  9c 00 00 00 b8 00 00 00 be 00 00 00 ae 00 00 00 a2 00 00 00 da 00 00 00 a0 00 00 00 70 00 00 00  œ...¸.......®...¢...Ú... ...p...
0x006358CC  7c 00 00 00 ce 00 00 00 d0 00 00 00 ce 00 00 00 c4 00 00 00 e0 00 00 00 f2 00 00 00 e2 00 00 00  |...Î...Ð...Î...Ä...à...ò...â...
0x006358EC  9a 00 00 00 b8 00 00 00 f0 00 00 00 c8 00 00 00 ce 00 00 00 ca 00 00 00 c6 00 00 00 00 00 00 00  š...¸...ð...È...Î...Ê...Æ.......
*/
static void after_block_decode_no_effect_q_impl(int quantScale)
{
    //signed int result; // eax@1
    //int arrayVal; // edx@4

    // I believe the static var was just used as an optimization to avoid re-calcing where possible
    //result = sQuantScale;
    //if (quantScale != sQuantScale)
    {
        //result = 16;
       // sQuantScale = quantScale;
        *gBlockWidthQ_before_635A0C = 16;            // changes nothing
        *gBlockHeightQ_before_63580C = 16;
        if (quantScale)                           // don't do anything if no scale, maybe for all black images?
        {
            signed int result = 0;
            do
            {
                g_252_buffer_unk_63580C[result] = quantScale * gQuant1_dword_42AEC8[result];
                result++;
                g_252_buffer_unk_635A0C[result-1] = quantScale * gQaunt2_dword_42AFC4[result];
                

            } while (result < 63);                   // 252/4=63
        }
        else
        {
            memset(g_252_buffer_unk_635A0C, 16, 252  /*sizeof(g_252_buffer_unk_635A0C)*/); // DWORD[63]
            memset(g_252_buffer_unk_63580C, 16, 252 /*sizeof(g_252_buffer_unk_63580C)*/);
        }
    }
}


static char __fastcall decode_ddv_frame(void* hack, ddv_class *thisPtr, unsigned char* pScreenBuffer)
{
    StartSDL();

    if (!thisPtr->mHasVideo)
    {
        return 0;
    }

    // No effect if no incremented
    ++thisPtr->field_6C;

    int(__cdecl *decodeMacroBlockfPtr)(int, int *, int, DWORD, int, int *) = nullptr;
    if (thisPtr->keyFrameRate <= 1)
    {
        // Should never happen - at least with the test data
        abort();
        // forcing this to get called resulting in really blocky video frames for keyframes > 1
        //decodeMacroBlockfPtr = (int(__cdecl *)(int, int *, int, _DWORD, int, int *))DecodeMacroBlockReleated_Q; // TODO: Reimpl
    }
    else
    {
        // gending uses this one - this outputs macroblock coefficients?
        decodeMacroBlockfPtr = (int(__cdecl *)(int, int *, int, DWORD, int, int *))ddv_func7_DecodeMacroBlock_ptr; // TODO: Reimpl
    }
 
    // Done once for the whole 320x240 image
    const int quantScale = decode_bitstream((WORD*)thisPtr->mRawFrameBitStreamData, (unsigned short int*)thisPtr->mDecodedBitStream);

    // Each block only seems to have 1 colour if this isn't called, but then resizing the window seems to fix it sometimes (perhaps causes
    // this function to be called else where).
   // after_block_decode_no_effect_q_ptr(quantScale); // TODO: Reimpl
    after_block_decode_no_effect_q_impl(quantScale);

    // Sanity check
    if (thisPtr->nNumMacroblocksX <= 0 || thisPtr->nNumMacroblocksY <= 0)
    {
        return 0;
    }

    int bitstreamCurPos = (int)thisPtr->mDecodedBitStream;
    
    int xoff = 0;
    auto buf = (unsigned short int*)pScreenBuffer;
    DWORD block1Output = thisPtr->mMacroBlockBuffer_q;

    // For 320x240 image we have a 20x16 macro block grid (because 320/16 and 240/16)
    for (unsigned int xBlock = 0; xBlock < thisPtr->nNumMacroblocksX; xBlock++)
    {
        int yoff = 0;
        for (unsigned int yBlock = 0; yBlock < thisPtr->nNumMacroblocksY; yBlock++)
        {
            const int dataSizeBytes = thisPtr->mBlockDataSize_q * 4; // Convert to byte count 64*4=256

            const int afterBlock1Ptr = decodeMacroBlockfPtr(bitstreamCurPos, Cr_block, block1Output, 0, 0, 0);
            do_blit_output_no_mmx(block1Output, Cr_block); // TODO: Reimpl
            const int block2Output = dataSizeBytes + block1Output;

            const int afterBlock2Ptr = decodeMacroBlockfPtr(afterBlock1Ptr, Cb_block, block2Output, 0, 0, 0);
            do_blit_output_no_mmx(block2Output, Cb_block);
            const int block3Output = dataSizeBytes + block2Output;

            const int afterBlock3Ptr = decodeMacroBlockfPtr(afterBlock2Ptr, Y1_block, block3Output, 1, 0, 0);
            do_blit_output_no_mmx(block3Output, Y1_block);
            const int block4Output = dataSizeBytes + block3Output;
  
            const int afterBlock4Ptr = decodeMacroBlockfPtr(afterBlock3Ptr, Y2_block, block4Output, 1, 0, 0);
            do_blit_output_no_mmx(block4Output, Y2_block);
            const int block5Output = dataSizeBytes + block4Output;

            const int afterBlock5Ptr = decodeMacroBlockfPtr(afterBlock4Ptr, Y3_block, block5Output, 1, 0, 0);
            do_blit_output_no_mmx(block5Output, Y3_block);
            const int block6Output = dataSizeBytes + block5Output;

            bitstreamCurPos = decodeMacroBlockfPtr(afterBlock5Ptr, Y4_block, block6Output, 1, 0, 0);
            do_blit_output_no_mmx(block6Output, Y4_block);
            block1Output = dataSizeBytes + block6Output;

            ConvertYuvToRgbAndBlit(buf, xoff, yoff);

            yoff += 16;
        }
        xoff += 16;
    }

    FlipSDL();

    // The app doesn't seem to do anything with the return value
    return 0;
}

void SetElement2(int x, int y, unsigned short int* ptr, unsigned short int value)
{
    const int kWidth = 320;
    ptr[(kWidth * y) + x] = value;
}


static void ConvertYuvToRgbAndBlit(unsigned short int* pFrameBuffer, int xoff, int yoff)
{
    // convert the Y1 Y2 Y3 Y4 and Cb and Cr blocks into a 16x16 array of (Y, Cb, Cr) pixels
    struct Macroblock_YCbCr_Struct
    {
        float Y;
        float Cb;
        float Cr;
    };

    Macroblock_YCbCr_Struct Macroblock_YCbCr[16][16] = {};

    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            Macroblock_YCbCr[x][y].Y = static_cast<float>(Y1_block[To1d(x, y)]);
            Macroblock_YCbCr[x + 8][y].Y = static_cast<float>(Y2_block[To1d(x, y)]);
            Macroblock_YCbCr[x][y + 8].Y = static_cast<float>(Y3_block[To1d(x, y)]);
            Macroblock_YCbCr[x + 8][y + 8].Y = static_cast<float>(Y4_block[To1d(x, y)]);

            Macroblock_YCbCr[x * 2][y * 2].Cb = static_cast<float>(Cb_block[To1d(x, y)]);
            Macroblock_YCbCr[x * 2 + 1][y * 2].Cb = static_cast<float>(Cb_block[To1d(x, y)]);
            Macroblock_YCbCr[x * 2][y * 2 + 1].Cb = static_cast<float>(Cb_block[To1d(x, y)]);
            Macroblock_YCbCr[x * 2 + 1][y * 2 + 1].Cb = static_cast<float>(Cb_block[To1d(x, y)]);

            Macroblock_YCbCr[x * 2][y * 2].Cr = static_cast<float>(Cr_block[To1d(x, y)]);
            Macroblock_YCbCr[x * 2 + 1][y * 2].Cr = static_cast<float>(Cr_block[To1d(x, y)]);
            Macroblock_YCbCr[x * 2][y * 2 + 1].Cr = static_cast<float>(Cr_block[To1d(x, y)]);
            Macroblock_YCbCr[x * 2 + 1][y * 2 + 1].Cr = static_cast<float>(Cr_block[To1d(x, y)]);
        }
    }

    // Convert the (Y, Cb, Cr) pixels into RGB pixels
    struct Macroblock_RGB_Struct
    {
        unsigned char Red;
        unsigned char Green;
        unsigned char Blue;
    };
    
    Macroblock_RGB_Struct Macroblock_RGB[16][16] = {};

    for (int x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            // Not sure if Cb/Cr are the other way around in mem/names :S
            const float r = (Macroblock_YCbCr[x][y].Y) + 1.402f * Macroblock_YCbCr[x][y].Cb;
            const float g = (Macroblock_YCbCr[x][y].Y) - 0.3437f * Macroblock_YCbCr[x][y].Cr - 0.7143f * Macroblock_YCbCr[x][y].Cb;
            const float b = (Macroblock_YCbCr[x][y].Y) + 1.772f * Macroblock_YCbCr[x][y].Cr;

            Macroblock_RGB[x][y].Red = Clamp(r);
            Macroblock_RGB[x][y].Green = Clamp(g);
            Macroblock_RGB[x][y].Blue = Clamp(b);

            SetElement(x + xoff, y + yoff, pFrameBuffer,
                RGB565(
                Macroblock_RGB[x][y].Red,
                Macroblock_RGB[x][y].Green,
                Macroblock_RGB[x][y].Blue));

            SetElement2(x + xoff, y + yoff, pixels,
                RGB565(
                Macroblock_RGB[x][y].Red,
                Macroblock_RGB[x][y].Green,
                Macroblock_RGB[x][y].Blue));
        }
    }
}
static inline void CheckForEscapeCode(char& bitsToShiftBy, int& rawWord1, WORD*& rawBitStreamPtr, DWORD& rawWord4, DWORD& v25)
{
    // I think this is used as an escape code?
    if (bitsToShiftBy & 16)   // 0b10000 if bit 5 set
    {
        rawWord1 = *rawBitStreamPtr;
        ++rawBitStreamPtr;

        bitsToShiftBy &= 15;
        rawWord4 = rawWord1 << bitsToShiftBy;
        v25 |= rawWord4;
    }
}

static inline void OutputWordAndAdvance(WORD*& rawBitStreamPtr, DWORD& rawWord4, unsigned short int*& pOut, char& numBitsToShiftBy, DWORD& v3)
{
    *pOut++ = v3 >> (32 - 16);

    rawWord4 = *rawBitStreamPtr++ << numBitsToShiftBy;
    v3 = rawWord4 | (v3 << 16);
}

#define MASK_11_BITS 0x7FF
#define MASK_10_BITS 0x3FF
#define MASK_13_BITS 0x1FFF
#define MDEC_END 0xFE00u

static void SetLoWord(DWORD& v, WORD lo)
{
    WORD hiWord = HIWORD(v);
    v = MAKELPARAM(lo, hiWord);
}

static DWORD GetBits(DWORD value, DWORD numBits)
{
    return value >> (32 - numBits);
}

static void SkipBits(DWORD& value, char numBits, char& bitPosCounter)
{
    value = value << numBits;
    bitPosCounter += numBits;
}

int decode_bitstream(WORD *pFrameData, unsigned short int *pOutput)
{
    unsigned int table_index_2 = 0;
    int ret = *pFrameData;
    DWORD v8 = *(DWORD*)(pFrameData + 1);
    WORD* rawBitStreamPtr = (pFrameData + 3);
    
    v8 = (v8 << 16) | (v8 >> 16); // Swap words

    DWORD rawWord4 = GetBits(v8, 11);

    char bitsShiftedCounter = 0;
    SkipBits(v8, 11, bitsShiftedCounter);
    DWORD v3 = v8;

    *pOutput++ = rawWord4; // store in output

    while (1)
    {
        do
        {
            while (1)
            {
                do
                {
                    while (1)
                    {
                        do
                        {
                            while (1)
                            {
                                while (1)
                                {
                                    table_index_2 = GetBits(v3, 13); // 0x1FFF / 8191 table size? 8192/8=1024 entries?
                                    if (table_index_2 >= 32)
                                    {
                                        break;
                                    }
                                    const int table_index_1 = GetBits(v3, 17); // 0x1FFFF / 131072, 131072/4=32768 entries?
                                  
                                    SkipBits(v3, 8, bitsShiftedCounter);

                                    int rawWord1;
                                    CheckForEscapeCode(bitsShiftedCounter, rawWord1, rawBitStreamPtr, rawWord4, v3);

                                   
                                    const char bitsToShiftFromTbl = gTbl1[table_index_1].mBitsToShift;

                                    SkipBits(v3, bitsToShiftFromTbl, bitsShiftedCounter);

                                    int rawWord2;
                                    CheckForEscapeCode(bitsShiftedCounter, rawWord2, rawBitStreamPtr, rawWord4, v3);

                                    // Everything in the table is 0's after 4266 bytes 4266/2=2133 to perhaps 2048/4096 is max?
                                    *pOutput++ = gTbl1[table_index_1].mOutputWord;

                                } // End while


                                const char tblValueBits = gTbl2[table_index_2].mBitsToShift;
                                
                                SkipBits(v3, tblValueBits, bitsShiftedCounter);

                                int rawWord3;
                                CheckForEscapeCode(bitsShiftedCounter, rawWord3, rawBitStreamPtr, rawWord4, v3);

                                SetLoWord(rawWord4, gTbl2[table_index_2].mOutputWord1);

                                if ((WORD)rawWord4 != 0x7C1F) // 0b 11111 00000 11111
                                {
                                    break;
                                }

                                OutputWordAndAdvance(rawBitStreamPtr, rawWord4, pOutput, bitsShiftedCounter, v3);
                            } // End while

                            *pOutput++ = rawWord4;

                            if ((WORD)rawWord4 == MDEC_END)
                            {
                                const int v15 = GetBits(v3, 11);
                                SkipBits(v3, 11, bitsShiftedCounter);

                                if (v15 == MASK_10_BITS)
                                {
                                    return ret;
                                }

                                rawWord4 = v15 & MASK_11_BITS;
                                *pOutput++ = rawWord4;

                                int rawWord5;
                                CheckForEscapeCode(bitsShiftedCounter, rawWord5, rawBitStreamPtr, rawWord4, v3);

                            }

                            SetLoWord(rawWord4, gTbl2[table_index_2].mOutputWord2);
                        } while (!(WORD)rawWord4);
                        
                       
                        if ((WORD)rawWord4 != 0x7C1F)
                        {
                            break;
                        }

                        OutputWordAndAdvance(rawBitStreamPtr, rawWord4, pOutput, bitsShiftedCounter, v3);
                    } // End while
                    
                    *pOutput++ = rawWord4;

                    if ((WORD)rawWord4 == MDEC_END)
                    {
                        const int t11Bits = GetBits(v3, 11);
                        SkipBits(v3, 11, bitsShiftedCounter);

                        if (t11Bits == MASK_10_BITS)
                        {
                            return ret;
                        }

                        rawWord4 = t11Bits & MASK_11_BITS;
                        *pOutput++ = rawWord4;

                        int rawWord7;
                        CheckForEscapeCode(bitsShiftedCounter, rawWord7, rawBitStreamPtr, rawWord4, v3);
                    }

                    SetLoWord(rawWord4, gTbl2[table_index_2].mOutputWord3);

                } while (!(WORD)rawWord4);

              
                if ((WORD)rawWord4 != 0x7C1F)
                {
                    break;
                }


                OutputWordAndAdvance(rawBitStreamPtr, rawWord4, pOutput, bitsShiftedCounter, v3);
            } // End while

            *pOutput++ = rawWord4;

        } while ((WORD)rawWord4 != MDEC_END);
        
        const int tmp11Bits2 = GetBits(v3, 11);
        SkipBits(v3, 11, bitsShiftedCounter);

        if (tmp11Bits2 == MASK_10_BITS)
        {
            return ret;
        }

        rawWord4 = tmp11Bits2;
        *pOutput++ = rawWord4;
      
        int rawWord9;
        CheckForEscapeCode(bitsShiftedCounter, rawWord9, rawBitStreamPtr, rawWord4, v3);

    }
    return ret;
}


void SetElement(int x, int y, unsigned short int* ptr, unsigned short int value)
{
    const int kWidth = 640;
    ptr[(kWidth * y) + x] = value;
}

static JmpHookedFunction<ddv_func7_DecodeMacroBlock>* ddv_func7_DecodeMacroBlock_hook;

void InstallHooks()
{
    ddv_func6_decodes_block_q_hook = new JmpHookedFunction<ddv__func5_block_decoder_q_type>(real_ddv__func5_block_decoder_q, &decode_ddv_frame);
}
