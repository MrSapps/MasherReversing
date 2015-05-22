#include "Hooks.hpp"
#include "jmphookedfunction.hpp"
#include <algorithm>
#include <string>
#include <set>
#include <stdint.h>
#include <SDL.h>
#include <iostream>
#include <vector>

#undef min
#undef max
#undef RGB

static SDL_Window *win = nullptr;
static SDL_Renderer *ren = nullptr;
static SDL_Texture *sdlTexture = nullptr;

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
   

    return 0;
}

static int w = 0;
std::vector<Uint32> pixels;

void SetSurfaceSize(int w, int h)
{
    ::w = w;

    if (sdlTexture)
    {
        SDL_DestroyTexture(sdlTexture);
        sdlTexture = 0;
    }

    sdlTexture = SDL_CreateTexture(ren,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        w, h);

    pixels.resize(w*h);

}

void FlipSDL()
{
    SDL_UpdateTexture(sdlTexture, NULL, pixels.data(), w * sizeof(Uint32));

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

// We can't compile the hook stubs as __thiscall, so __fastcall is used as a workaround/hack
char __fastcall decode_ddv_frame(void* hack, ddv_class *thisPtr, unsigned char* screenBuffer);
typedef decltype(&decode_ddv_frame) ddv__func5_block_decoder_q_type;

static ddv__func5_block_decoder_q_type real_ddv__func5_block_decoder_q = (ddv__func5_block_decoder_q_type)0x00409FE0;
static JmpHookedFunction<ddv__func5_block_decoder_q_type>* ddv_func6_decodes_block_q_hook;

void idct(int16_t* pSource, int32_t* pDestination);

static void do_blit_output_no_mmx(WORD* macroBlockBuffer, int* decodedBitStream)
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
inline unsigned int RGB(unsigned char r, unsigned char g, unsigned char b) {
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

inline unsigned int RGB888(unsigned char r, unsigned char g, unsigned char b) {
    return RGB<8, 8, 8>(r, g, b);
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

const DWORD gQuant1_dword_42AEC8[64] =
{
    0x0000000C, 0x0000000B, 0x0000000A, 0x0000000C, 0x0000000E, 0x0000000E, 0x0000000D, 0x0000000E,
    0x00000010, 0x00000018, 0x00000013, 0x00000010, 0x00000011, 0x00000012, 0x00000018, 0x00000016,
    0x00000016, 0x00000018, 0x0000001A, 0x00000028, 0x00000033, 0x0000003A, 0x00000028, 0x0000001D,
    0x00000025, 0x00000023, 0x00000031, 0x00000048, 0x00000040, 0x00000037, 0x00000038, 0x00000033,
    0x00000039, 0x0000003C, 0x0000003D, 0x00000037, 0x00000045, 0x00000057, 0x00000044, 0x00000040,
    0x0000004E, 0x0000005C, 0x0000005F, 0x00000057, 0x00000051, 0x0000006D, 0x00000050, 0x00000038,
    0x0000003E, 0x00000067, 0x00000068, 0x00000067, 0x00000062, 0x00000070, 0x00000079, 0x00000071,
    0x0000004D, 0x0000005C, 0x00000078, 0x00000064, 0x00000067, 0x00000065, 0x00000063, 0x00000010
};

const DWORD gQaunt2_dword_42AFC4[64] =
{
    0x00000010, 0x00000012, 0x00000012, 0x00000018, 0x00000015, 0x00000018, 0x0000002F, 0x0000001A,
    0x0000001A, 0x0000002F, 0x00000063, 0x00000042, 0x00000038, 0x00000042, 0x00000063, 0x00000063,
    0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063,
    0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063,
    0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063,
    0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063,
    0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063,
    0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063, 0x00000063
};

const DWORD g_block_related_1_dword_42B0C8[64] =
{
    0x00000001, 0x00000008, 0x00000010, 0x00000009, 0x00000002, 0x00000003, 0x0000000A, 0x00000011,
    0x00000018, 0x00000020, 0x00000019, 0x00000012, 0x0000000B, 0x00000004, 0x00000005, 0x0000000C,
    0x00000013, 0x0000001A, 0x00000021, 0x00000028, 0x00000030, 0x00000029, 0x00000022, 0x0000001B,
    0x00000014, 0x0000000D, 0x00000006, 0x00000007, 0x0000000E, 0x00000015, 0x0000001C, 0x00000023,
    0x0000002A, 0x00000031, 0x00000038, 0x00000039, 0x00000032, 0x0000002B, 0x00000024, 0x0000001D,
    0x00000016, 0x0000000F, 0x00000017, 0x0000001E, 0x00000025, 0x0000002C, 0x00000033, 0x0000003A,
    0x0000003B, 0x00000034, 0x0000002D, 0x00000026, 0x0000001F, 0x00000027, 0x0000002E, 0x00000035,
    0x0000003C, 0x0000003D, 0x00000036, 0x0000002F, 0x00000037, 0x0000003E, 0x0000003F, 0x0000098E // TODO: Last value too large?
};

const DWORD g_block_related_unknown_dword_42B0C4[64] = 
{
    0x00000000, 0x00000001, 0x00000008, 0x00000010, 0x00000009, 0x00000002, 0x00000003, 0x0000000A,
    0x00000011, 0x00000018, 0x00000020, 0x00000019, 0x00000012, 0x0000000B, 0x00000004, 0x00000005,
    0x0000000C, 0x00000013, 0x0000001A, 0x00000021, 0x00000028, 0x00000030, 0x00000029, 0x00000022,
    0x0000001B, 0x00000014, 0x0000000D, 0x00000006, 0x00000007, 0x0000000E, 0x00000015, 0x0000001C,
    0x00000023, 0x0000002A, 0x00000031, 0x00000038, 0x00000039, 0x00000032, 0x0000002B, 0x00000024,
    0x0000001D, 0x00000016, 0x0000000F, 0x00000017, 0x0000001E, 0x00000025, 0x0000002C, 0x00000033,
    0x0000003A, 0x0000003B, 0x00000034, 0x0000002D, 0x00000026, 0x0000001F, 0x00000027, 0x0000002E,
    0x00000035, 0x0000003C, 0x0000003D, 0x00000036, 0x0000002F, 0x00000037, 0x0000003E, 0x0000003F
};

const DWORD g_block_related_2_dword_42B0CC[64] =
{
    0x00000008, 0x00000010, 0x00000009, 0x00000002, 0x00000003, 0x0000000A, 0x00000011, 0x00000018,
    0x00000020, 0x00000019, 0x00000012, 0x0000000B, 0x00000004, 0x00000005, 0x0000000C, 0x00000013,
    0x0000001A, 0x00000021, 0x00000028, 0x00000030, 0x00000029, 0x00000022, 0x0000001B, 0x00000014,
    0x0000000D, 0x00000006, 0x00000007, 0x0000000E, 0x00000015, 0x0000001C, 0x00000023, 0x0000002A,
    0x00000031, 0x00000038, 0x00000039, 0x00000032, 0x0000002B, 0x00000024, 0x0000001D, 0x00000016,
    0x0000000F, 0x00000017, 0x0000001E, 0x00000025, 0x0000002C, 0x00000033, 0x0000003A, 0x0000003B,
    0x00000034, 0x0000002D, 0x00000026, 0x0000001F, 0x00000027, 0x0000002E, 0x00000035, 0x0000003C,
    0x0000003D, 0x00000036, 0x0000002F, 0x00000037, 0x0000003E, 0x0000003F, 0x0000098E, 0x0000098E
};

const DWORD g_block_related_3_dword_42B0D0[64] =
{
    0x00000010, 0x00000009, 0x00000002, 0x00000003, 0x0000000A, 0x00000011, 0x00000018, 0x00000020,
    0x00000019, 0x00000012, 0x0000000B, 0x00000004, 0x00000005, 0x0000000C, 0x00000013, 0x0000001A,
    0x00000021, 0x00000028, 0x00000030, 0x00000029, 0x00000022, 0x0000001B, 0x00000014, 0x0000000D,
    0x00000006, 0x00000007, 0x0000000E, 0x00000015, 0x0000001C, 0x00000023, 0x0000002A, 0x00000031,
    0x00000038, 0x00000039, 0x00000032, 0x0000002B, 0x00000024, 0x0000001D, 0x00000016, 0x0000000F,
    0x00000017, 0x0000001E, 0x00000025, 0x0000002C, 0x00000033, 0x0000003A, 0x0000003B, 0x00000034,
    0x0000002D, 0x00000026, 0x0000001F, 0x00000027, 0x0000002E, 0x00000035, 0x0000003C, 0x0000003D,
    0x00000036, 0x0000002F, 0x00000037, 0x0000003E, 0x0000003F, 0x0000098E, 0x0000098E, 0x0000F384
};

//DWORD* g_252_buffer_unk_635A0C = (DWORD*)0x635A08;
//DWORD* g_252_buffer_unk_63580C = (DWORD*)0x635808;

DWORD g_252_buffer_unk_635A0C[64] = {};
DWORD g_252_buffer_unk_63580C[64] = {};

static void after_block_decode_no_effect_q_impl(int quantScale)
{
    /*
    for (int i = 0; i < 8; i++)
    {
        int pos = i * 8;
        char buffer[8192] = {};
        sprintf(buffer, "0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X,\n", 
            g_block_related_3_dword_42B0D0[pos + 0],
            g_block_related_3_dword_42B0D0[pos + 1],
            g_block_related_3_dword_42B0D0[pos + 2],
            g_block_related_3_dword_42B0D0[pos + 3],
            g_block_related_3_dword_42B0D0[pos + 4],
            g_block_related_3_dword_42B0D0[pos + 5],
            g_block_related_3_dword_42B0D0[pos + 6],
            g_block_related_3_dword_42B0D0[pos + 7]
            );
        OutputDebugString(buffer);
    }*/
    

    g_252_buffer_unk_63580C[0] = 16;
    g_252_buffer_unk_635A0C[0] = 16;
    if (quantScale > 0)
    {
        signed int result = 0;
        do
        {
            auto val = gQuant1_dword_42AEC8[result];
            result++;
            g_252_buffer_unk_63580C[result] = quantScale * val;
            g_252_buffer_unk_635A0C[result] = quantScale * gQaunt2_dword_42AFC4[result];


        } while (result < 63);                   // 252/4=63
    }
    else
    {
        // These are simply null buffers to start with
        for (int i = 0; i < 64; i++)
        {
            g_252_buffer_unk_635A0C[i] = 16;
            g_252_buffer_unk_63580C[i] = 16;
        }
       // memset(&g_252_buffer_unk_635A0C[1], 16, 252  /*sizeof(g_252_buffer_unk_635A0C)*/); // DWORD[63]
       // memset(&g_252_buffer_unk_63580C[1], 16, 252 /*sizeof(g_252_buffer_unk_63580C)*/);
    }

}

static void SetLoWord(DWORD& v, WORD lo)
{
    WORD hiWord = HIWORD(v);
    v = MAKELPARAM(lo, hiWord);
}

static void SetHiWord(DWORD& v, WORD hi)
{
    WORD loWord = LOWORD(v);
    v = MAKEWPARAM(loWord, hi);
}

// Return val becomes param 1
WORD* __cdecl ddv_func7_DecodeMacroBlock_impl(WORD* bitstreamPtr, int * blockPtr, WORD* outputBlockPtr, DWORD isYBlock, int unused1, int* unused2)
{
    int v1; // ebx@1
    DWORD *v2; // esi@1
    WORD* endPtr; // edx@3
    DWORD *output_q; // ebp@3
    unsigned int counter; // edi@3
    DWORD* v6; // esi@3
    WORD* outptr; // edx@3
    WORD* dataPtr; // edx@5
    unsigned int macroBlockWord; // eax@6
    unsigned int blockNumberQ; // edi@9
    int index1; // eax@14
    int index2; // esi@14
    int index3; // ecx@14
    signed int v14; // eax@15
    int cnt; // ecx@15
    unsigned int v16; // ecx@15
   // int v17; // esi@15
    int idx; // ebx@16
    DWORD outVal; // ecx@18
    unsigned int macroBlockWord1; // eax@20
  //  int v21; // esi@21
    unsigned int v22; // edi@21
    int v23; // ebx@21
 //   signed int v24; // eax@21
    DWORD v25; // ecx@21
   // DecodeMacroBlock_Struct *thisPtr; // [sp-4h] [bp-10h]@3

    v1 = isYBlock /*this->ZeroOrOneConstant*/;                 // off 14
    v2 = &g_252_buffer_unk_63580C[1];
    
    if (!isYBlock /*this->ZeroOrOneConstant*/)
    {
        v2 = &g_252_buffer_unk_635A0C[1];
    }

    v6 = v2;
    counter = 0;
    outptr = (WORD*)bitstreamPtr /*this->mOutput >> 1*/;
    //thisPtr = this;
    output_q = (DWORD*)outputBlockPtr /*this->mCoEffsBuffer*/;               // off 10 quantised coefficients
    endPtr = outptr - 1;
    
    do
    {
        ++endPtr;
    }
    while (*endPtr == 0xFE00u);  // 0xFE00 == END_OF_BLOCK, hence this loop moves past the EOB
    
    *output_q = (v1 << 10) + 2 * (*endPtr << 21 >> 22);
    dataPtr = endPtr + 1; // last use of endPtr


    if (*(BYTE *)(dataPtr - 1) & 1)        // has video flag?
    {
        
        do
        {
            macroBlockWord1 = *dataPtr++;// bail if end
            if (macroBlockWord1 == 0xFE00)
            {
                break;
            }
            DWORD* v21 = (macroBlockWord1 >> 10) + v6;
            v22 = (macroBlockWord1 >> 10) + counter;
            v23 = g_block_related_1_dword_42B0C8[v22];
            signed int v24 = output_q[v23] + (macroBlockWord1 << 22);
            SetHiWord(v25, HIWORD(v24));
            counter = v22 + 1;
            SetLoWord(v25, ( *(v21) * (v24 >> 22) + 4) >> 3);
            v6 = v21 + 1;
            output_q[v23] = v25;
        } while (counter < 63);                     // 63 AC values?
        
    }
    else
    {
        
        while (1)
        {
            macroBlockWord = *dataPtr++;// bail if end
            if (macroBlockWord == 0xFE00)
            {
                break;
            }
            v16 = macroBlockWord;
            v14 = macroBlockWord << 22;
            v16 >>= 10;
            DWORD* v17 = v16 + v6;
            cnt = v16 + 1;
            while (1)
            {
                --cnt;
                idx = g_block_related_1_dword_42B0C8[counter];
                if (!cnt)
                {
                    break;
                }
                output_q[idx] = 0;
                ++counter;
            }
            SetHiWord(outVal, HIWORD(v14));
            ++counter;
            SetLoWord(outVal, (*(v17) * (v14 >> 22) + 4) >> 3);
            v6 = v17 + 1;
            output_q[idx] = outVal;
            if (counter >= 63)                      // 63 AC values?
            {
                return dataPtr;
            }
        }
        if (counter)
        {
            blockNumberQ = counter + 1;
            if (blockNumberQ & 3)
            {
                output_q[g_block_related_unknown_dword_42B0C4[blockNumberQ++]] = 0;
                if (blockNumberQ & 3)
                {
                    output_q[g_block_related_unknown_dword_42B0C4[blockNumberQ++]] = 0;
                    if (blockNumberQ & 3)
                    {
                        output_q[g_block_related_unknown_dword_42B0C4[blockNumberQ++]] = 0;
                    }
                }
            }
            
            while (blockNumberQ != 64)              // 63 AC values?
            {
                index1 = g_block_related_1_dword_42B0C8[blockNumberQ];
                index2 = g_block_related_2_dword_42B0CC[blockNumberQ];
                index3 = g_block_related_3_dword_42B0D0[blockNumberQ];
                output_q[g_block_related_unknown_dword_42B0C4[blockNumberQ]] = 0;
                output_q[index1] = 0;
                output_q[index2] = 0;
                output_q[index3] = 0;
               // blockNumberQ += 4;
                blockNumberQ++;
            }
        }
        else
        {
            memset(output_q + 1, 0, 252u);            // 63 dwords buffer
        }
        
    }
    return dataPtr;
}

unsigned char byte_62EEB0[256] = {};

void init_Snd_tbl()
{
    int index = 0;
    do
    {
        int tableValue = 0;
        for (int i = index; i > 0; ++tableValue)
        {
            i >>= 1;
        }
        byte_62EEB0[index++] = tableValue;
    } while (index < 256);
}

/*
// One of these breaks prophcy DDV
typedef int(__cdecl *ddv_func7_DecodeMacroBlock)(void* params);
static ddv_func7_DecodeMacroBlock ddv_func7_DecodeMacroBlock_ptr = (ddv_func7_DecodeMacroBlock)0x0040E6B0;

typedef signed int(__cdecl* after_block_decode_no_effect_q)(int a1);
static after_block_decode_no_effect_q after_block_decode_no_effect_q_ptr = (after_block_decode_no_effect_q)0x0040E360;
*/

#include "PSXMDECDecoder.h"

char __fastcall decode_ddv_frame(void* hack, ddv_class *thisPtr, unsigned char* pScreenBuffer)
{
    //StartSDL();

    if (!thisPtr->mHasVideo)
    {
        return 0;
    }

    // No effect if no incremented
    ++thisPtr->field_6C;

   // DWORD*(__cdecl *decodeMacroBlockfPtr)(DWORD*, int *, DWORD*, DWORD, int, int *) = nullptr;
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
        //decodeMacroBlockfPtr = (int(__cdecl *)(int, int *, int, DWORD, int, int *))ddv_func7_DecodeMacroBlock_ptr; // TODO: Reimpl
       // decodeMacroBlockfPtr = ddv_func7_DecodeMacroBlock_impl;
    }
 
    // Done once for the whole 320x240 image
    const int quantScale = decode_bitstream(thisPtr->mRawFrameBitStreamData, thisPtr->mDecodedBitStream);

    // Each block only seems to have 1 colour if this isn't called, but then resizing the window seems to fix it sometimes (perhaps causes
    // this function to be called else where).
  //  after_block_decode_no_effect_q_ptr(quantScale); // TODO: Reimpl
    after_block_decode_no_effect_q_impl(quantScale);

    // Sanity check
    if (thisPtr->nNumMacroblocksX <= 0 || thisPtr->nNumMacroblocksY <= 0)
    {
        return 0;
    }


    WORD* bitstreamCurPos = thisPtr->mDecodedBitStream;
    
    int xoff = 0;
    auto buf = (unsigned short int*)pScreenBuffer;
    WORD* block1Output = thisPtr->mMacroBlockBuffer_q;

    // For 320x240 image we have a 20x16 macro block grid (because 320/16 and 240/16)
    for (unsigned int xBlock = 0; xBlock < thisPtr->nNumMacroblocksX; xBlock++)
    {
        int yoff = 0;
        for (unsigned int yBlock = 0; yBlock < thisPtr->nNumMacroblocksY; yBlock++)
        {
            const int dataSizeBytes = thisPtr->mBlockDataSize_q * 4; // Convert to byte count 64*4=256

            WORD* afterBlock1Ptr = ddv_func7_DecodeMacroBlock_impl(bitstreamCurPos, Cr_block, block1Output, 0, 0, 0);
            do_blit_output_no_mmx(block1Output, Cr_block); // TODO: Reimpl
            WORD* block2Output = dataSizeBytes + block1Output;

            WORD* afterBlock2Ptr = ddv_func7_DecodeMacroBlock_impl(afterBlock1Ptr, Cb_block, block2Output, 0, 0, 0);
            do_blit_output_no_mmx(block2Output, Cb_block);
            WORD* block3Output = dataSizeBytes + block2Output;

            WORD* afterBlock3Ptr = ddv_func7_DecodeMacroBlock_impl(afterBlock2Ptr, Y1_block, block3Output, 1, 0, 0);
            do_blit_output_no_mmx(block3Output, Y1_block);
            WORD* block4Output = dataSizeBytes + block3Output;
  
            WORD* afterBlock4Ptr = ddv_func7_DecodeMacroBlock_impl(afterBlock3Ptr, Y2_block, block4Output, 1, 0, 0);
            do_blit_output_no_mmx(block4Output, Y2_block);
            WORD* block5Output = dataSizeBytes + block4Output;

            WORD* afterBlock5Ptr = ddv_func7_DecodeMacroBlock_impl(afterBlock4Ptr, Y3_block, block5Output, 1, 0, 0);
            do_blit_output_no_mmx(block5Output, Y3_block);
            WORD* block6Output = dataSizeBytes + block5Output;

            bitstreamCurPos = ddv_func7_DecodeMacroBlock_impl(afterBlock5Ptr, Y4_block, block6Output, 1, 0, 0);
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

void do_hack(int quantScale, unsigned short int* pBitStream, unsigned char* pScreenBuffer)
{

    // Each block only seems to have 1 colour if this isn't called, but then resizing the window seems to fix it sometimes (perhaps causes
    // this function to be called else where).
    //  after_block_decode_no_effect_q_ptr(quantScale); // TODO: Reimpl
    after_block_decode_no_effect_q_impl(quantScale);

    std::vector<WORD> block(7000000);

    WORD* bitstreamCurPos = (WORD*)pBitStream;

    int xoff = 0;
    auto buf = (unsigned short int*)pScreenBuffer;
    WORD* block1Output = block.data();

    // For 320x240 image we have a 20x16 macro block grid (because 320/16 and 240/16)
    for (unsigned int xBlock = 0; xBlock < 320/16; xBlock++)
    {
        int yoff = 0;
        for (unsigned int yBlock = 0; yBlock < 240/16; yBlock++)
        {
            const int dataSizeBytes = 64 * 4; // Convert to byte count 64*4=256

            WORD* afterBlock1Ptr = ddv_func7_DecodeMacroBlock_impl(bitstreamCurPos, Cr_block, block1Output, 0, 0, 0);
            do_blit_output_no_mmx(block1Output, Cr_block); // TODO: Reimpl
            WORD* block2Output = dataSizeBytes + block1Output;

            WORD* afterBlock2Ptr = ddv_func7_DecodeMacroBlock_impl(afterBlock1Ptr, Cb_block, block2Output, 0, 0, 0);
            do_blit_output_no_mmx(block2Output, Cb_block);
            WORD* block3Output = dataSizeBytes + block2Output;

            WORD* afterBlock3Ptr = ddv_func7_DecodeMacroBlock_impl(afterBlock2Ptr, Y1_block, block3Output, 1, 0, 0);
            do_blit_output_no_mmx(block3Output, Y1_block);
            WORD* block4Output = dataSizeBytes + block3Output;

            WORD* afterBlock4Ptr = ddv_func7_DecodeMacroBlock_impl(afterBlock3Ptr, Y2_block, block4Output, 1, 0, 0);
            do_blit_output_no_mmx(block4Output, Y2_block);
            WORD* block5Output = dataSizeBytes + block4Output;

            WORD* afterBlock5Ptr = ddv_func7_DecodeMacroBlock_impl(afterBlock4Ptr, Y3_block, block5Output, 1, 0, 0);
            do_blit_output_no_mmx(block5Output, Y3_block);
            WORD* block6Output = dataSizeBytes + block5Output;

            bitstreamCurPos = ddv_func7_DecodeMacroBlock_impl(afterBlock5Ptr, Y4_block, block6Output, 1, 0, 0);
            do_blit_output_no_mmx(block6Output, Y4_block);
            block1Output = dataSizeBytes + block6Output;

            ConvertYuvToRgbAndBlit(buf, xoff, yoff);

            yoff += 16;
        }
        xoff += 16;
    }

    FlipSDL();

}


void SetElement2(int x, int y, unsigned int* ptr, unsigned int value)
{
    int kWidth = ::w;
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

            /*
            SetElement(x + xoff, y + yoff, pFrameBuffer,
            RGB565(
            Macroblock_RGB[x][y].Red,
            Macroblock_RGB[x][y].Green,
            Macroblock_RGB[x][y].Blue));
            */

            if (!pixels.size())
            {
                SetSurfaceSize(320, 240);  // Guess when running as a hook;
            }

            
            SetElement2(x + xoff, y + yoff, pixels.data(),
                RGB888(
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


void InstallHooks()
{
    ddv_func6_decodes_block_q_hook = new JmpHookedFunction<ddv__func5_block_decoder_q_type>(real_ddv__func5_block_decoder_q, &decode_ddv_frame);
}
