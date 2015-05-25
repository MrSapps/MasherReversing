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

static void SetLoWord(DWORD& v, WORD lo)
{
    WORD hiWord = HIWORD(v);
    v = MAKELPARAM(lo, hiWord);
}
static void SetLoInt(int& v, WORD lo)
{
    WORD hiWord = HIWORD(v);
    v = MAKELPARAM(lo, hiWord);
}



DWORD& init_32_dword_62EEA8 = *(DWORD*)0x62EEA8;
DWORD& gFirstAudioFrameDWORD_dword_62EFB4 = *(DWORD*)0x62EFB4;
int& gAudioFrameSizeBytes = *(int*)0x0062EFC4;

WORD** gAudioFrameDataPtr = (WORD**)0x0062EFB0;

//unsigned char gSndTbl_byte_62EEB0[256] = {};
unsigned char* gSndTbl_byte_62EEB0 = (unsigned char*)0x62EEB0;

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
        gSndTbl_byte_62EEB0[index++] = tableValue;
    } while (index < 256);
}

int __cdecl GetSoundTableValue(__int16 tblIndex)
{
    int result; // eax@1
    signed __int16 positiveTblIdx; // ax@1

    positiveTblIdx = abs(tblIndex);
    result = (unsigned __int16)((signed __int16)gSndTbl_byte_62EEB0[positiveTblIdx >> 7] << 7) | (unsigned __int16)(positiveTblIdx >> gSndTbl_byte_62EEB0[positiveTblIdx >> 7]);
    if (tblIndex < 0)
        result = -result;
    return result;
}

int __cdecl sub_408F50(__int16 a1)
{
    int result; // eax@1
    __int16 v2; // ax@1

    v2 = abs(a1);
    result = (unsigned __int16)((v2 & 0x7F) << (v2 >> 7)) | (unsigned __int16)(1 << ((v2 >> 7) - 2));
    if (a1 < 0)
        result = -result;
    return result;
}

WORD *__cdecl SetupAudioDecodePtrs(WORD *rawFrameBuffer)
{
    WORD *result; // eax@1

    *gAudioFrameDataPtr = rawFrameBuffer;
    result = rawFrameBuffer + 2;
    gFirstAudioFrameDWORD_dword_62EFB4 = *(DWORD *)rawFrameBuffer;
    *gAudioFrameDataPtr = rawFrameBuffer + 2;
    init_32_dword_62EEA8 = 32;
    return result;
}

int __cdecl SndRelated_sub_409650()
{
    int result; // eax@1
    int v1; // ecx@1
    unsigned __int8 v2; // zf@1
    char v3; // sf@1
    unsigned __int8 v4; // of@1
    int v5; // ecx@2
    unsigned int v6; // ecx@2

    v1 = init_32_dword_62EEA8 & 7;
    result = init_32_dword_62EEA8 - v1;
    v4 = init_32_dword_62EEA8 - v1; //SET0 16
    v2 = init_32_dword_62EEA8 - v1 == 16;
    v3 = init_32_dword_62EEA8 - v1 - 16 < 0;
    init_32_dword_62EEA8 -= v1;
    gFirstAudioFrameDWORD_dword_62EFB4 >>= v1;
    if ((unsigned __int8)(v3 ^ v4) | v2)
    {
        v5 = **gAudioFrameDataPtr;
        ++*gAudioFrameDataPtr;
        v6 = (v5 << result) | gFirstAudioFrameDWORD_dword_62EFB4;
        result += 16;
        gFirstAudioFrameDWORD_dword_62EFB4 = v6;
        init_32_dword_62EEA8 = result;
    }
    return result;
}

int __cdecl sound16bitRelated_sub_4096B0(WORD *outPtr, int numSamplesPerFrame)
{
    int c1; // eax@1
    unsigned int v3; // edx@1
    __int16 v4; // di@1
    WORD *srcPtr; // esi@2
    int srcVal1; // ecx@2
    int c2; // eax@4
    unsigned int v8; // edx@4
    int v9; // ecx@4
    __int16 v10; // di@4
    int srcVal2; // ecx@5
    int c3; // eax@6
    unsigned int v13; // edx@6
    int v14; // ecx@6
    __int16 v15; // di@6
    int srcVal3; // ecx@7
    int c4; // eax@8
    unsigned int v18; // edx@8
    int v19; // ebx@8
    __int16 v20; // di@8
    int srcVal4; // ecx@9
    int c5; // eax@10
    unsigned int v23; // edx@10
    int v24; // ebp@10
    __int16 v25; // di@10
    int srcVal5; // ecx@11
    WORD *v27; // eax@12
    __int16 v28; // dx@12
    int v29; // ecx@12
    unsigned __int8 v30; // zf@12
    char v31; // sf@12
    unsigned __int8 v32; // of@12
    int v33; // edi@13
    WORD *v34; // eax@14
    __int16 v35; // dx@14
    int v36; // ecx@14
    unsigned __int8 v37; // zf@14
    char v38; // sf@14
    unsigned __int8 v39; // of@14
    int v40; // edi@15
    int v41; // ebx@16
    int v42; // ecx@17
    int v43; // eax@19
    unsigned int v44; // edx@19
    int v45; // esi@19
    unsigned __int8 v46; // zf@19
    char v47; // sf@19
    unsigned __int8 v48; // of@19
    WORD *srcPtr2; // edi@20
    int srcVal6; // ecx@20
    signed int v51; // ecx@22
    int c6; // eax@23
    unsigned int v53; // edx@23
    int srcVal7; // ecx@24
    int c7; // eax@26
    unsigned int v56; // edx@26
    int srcVal8; // ecx@27
    int v58; // eax@34
    int v59; // edi@34
    int v60; // eax@34
    __int16 v61; // ax@35
    char v62; // zf@37
    int v64; // [sp+10h] [bp-28h]@6
    int v65; // [sp+14h] [bp-24h]@8
    int v66; // [sp+18h] [bp-20h]@10
    int v67; // [sp+1Ch] [bp-1Ch]@12
    int v68; // [sp+20h] [bp-18h]@14
    int v69; // [sp+24h] [bp-14h]@17
    signed int v70; // [sp+28h] [bp-10h]@10
    signed int v71; // [sp+2Ch] [bp-Ch]@10
    signed int v72; // [sp+30h] [bp-8h]@10
    int v73; // [sp+34h] [bp-4h]@4
    WORD *v74; // [sp+3Ch] [bp+4h]@16
    int v75; // [sp+40h] [bp+8h]@17

    c1 = init_32_dword_62EEA8 - 16;
    init_32_dword_62EEA8 = c1;
    v4 = gFirstAudioFrameDWORD_dword_62EFB4;
    v3 = gFirstAudioFrameDWORD_dword_62EFB4 >> 16;
    gFirstAudioFrameDWORD_dword_62EFB4 >>= 16;
    if (c1 > 16)
    {
        srcPtr = *gAudioFrameDataPtr;
    }
    else
    {
        srcVal1 = **gAudioFrameDataPtr;
        srcPtr = *gAudioFrameDataPtr + 1;
        ++*gAudioFrameDataPtr;
        v3 |= srcVal1 << c1;
        c1 += 16;
    }
    v9 = v4;
    v10 = v3;
    c2 = c1 - 16;
    v8 = v3 >> 16;
    v73 = v9;
    gFirstAudioFrameDWORD_dword_62EFB4 = v8;
    init_32_dword_62EEA8 = c2;
    if (c2 <= 16)
    {
        srcVal2 = *srcPtr;
        ++srcPtr;
        *gAudioFrameDataPtr = srcPtr;
        v8 |= srcVal2 << c2;
        c2 += 16;
    }
    v14 = v10;
    v15 = v8;
    c3 = c2 - 16;
    v13 = v8 >> 16;
    v64 = v14;
    gFirstAudioFrameDWORD_dword_62EFB4 = v13;
    init_32_dword_62EEA8 = c3;
    if (c3 <= 16)
    {
        srcVal3 = *srcPtr;
        ++srcPtr;
        *gAudioFrameDataPtr = srcPtr;
        v13 |= srcVal3 << c3;
        c3 += 16;
    }
    v19 = v15;
    v20 = v13;
    c4 = c3 - 16;
    v18 = v13 >> 16;
    v65 = v19;
    gFirstAudioFrameDWORD_dword_62EFB4 = v18;
    init_32_dword_62EEA8 = c4;
    if (c4 <= 16)
    {
        srcVal4 = *srcPtr;
        ++srcPtr;
        *gAudioFrameDataPtr = srcPtr;
        v18 |= srcVal4 << c4;
        c4 += 16;
    }
    c5 = c4 - 16;
    v24 = v20;
    v66 = v20;
    init_32_dword_62EEA8 = c5;
    v70 = 1 << (v64 - 1);
    v71 = 1 << (v19 - 1);
    v72 = 1 << (v20 - 1);
    v25 = v18;
    v23 = v18 >> 16;
    gFirstAudioFrameDWORD_dword_62EFB4 = v23;
    if (c5 <= 16)
    {
        srcVal5 = *srcPtr;
        *gAudioFrameDataPtr = srcPtr + 1;
        gFirstAudioFrameDWORD_dword_62EFB4 = (srcVal5 << c5) | v23;
        init_32_dword_62EEA8 = c5 + 16;
    }
    *outPtr = v25;
    v67 = v25;
    v27 = &outPtr[gAudioFrameSizeBytes];
    v28 = gFirstAudioFrameDWORD_dword_62EFB4;
    gFirstAudioFrameDWORD_dword_62EFB4 >>= 16;
    v29 = init_32_dword_62EEA8 - 16;
    v32 = init_32_dword_62EEA8 - 16; // SETO 16
    v30 = init_32_dword_62EEA8 == 32;
    v31 = init_32_dword_62EEA8 - 32 < 0;
    init_32_dword_62EEA8 -= 16;
    if ((unsigned __int8)(v31 /*^ v32*/) | v30)
    {
        v33 = **gAudioFrameDataPtr;
        ++*gAudioFrameDataPtr;
        gFirstAudioFrameDWORD_dword_62EFB4 |= v33 << v29;
        init_32_dword_62EEA8 = v29 + 16;
    }
    v68 = v28;
    *v27 = v28;
    v34 = &v27[gAudioFrameSizeBytes];
    v35 = gFirstAudioFrameDWORD_dword_62EFB4;
    gFirstAudioFrameDWORD_dword_62EFB4 >>= 16;
    v36 = init_32_dword_62EEA8 - 16;
    v39 = init_32_dword_62EEA8 - 16; // SETO 16
    v37 = init_32_dword_62EEA8 == 32;
    v38 = init_32_dword_62EEA8 - 32 < 0;
    init_32_dword_62EEA8 -= 16;
    if ((unsigned __int8)(v38 /*^ v39*/) | v37)
    {
        v40 = **gAudioFrameDataPtr;
        ++*gAudioFrameDataPtr;
        gFirstAudioFrameDWORD_dword_62EFB4 |= v40 << v36;
        init_32_dword_62EEA8 = v36 + 16;
    }
    v41 = v35;
    *v34 = v35;
    v74 = &v34[gAudioFrameSizeBytes];
    if (numSamplesPerFrame > 3)
    {
        v42 = v64;
        v69 = (1 << v64) - 1;
        v75 = numSamplesPerFrame - 3;
        while (1)
        {
            SetLoInt(v45, gFirstAudioFrameDWORD_dword_62EFB4 & v69);
            v44 = gFirstAudioFrameDWORD_dword_62EFB4 >> v42;
            v43 = init_32_dword_62EEA8 - v42;
            v48 = init_32_dword_62EEA8 - v42; // SETO 16
            v46 = init_32_dword_62EEA8 - v42 == 16;
            v47 = init_32_dword_62EEA8 - v42 - 16 < 0;
            init_32_dword_62EEA8 -= v42;
            gFirstAudioFrameDWORD_dword_62EFB4 >>= v42;
            if ((unsigned __int8)(v47 /*^ v48*/) | v46)
            {
                srcVal6 = **gAudioFrameDataPtr;
                srcPtr2 = *gAudioFrameDataPtr + 1;
                ++*gAudioFrameDataPtr;
                v44 |= srcVal6 << v43;
                v24 = v66;
                v43 += 16;
                gFirstAudioFrameDWORD_dword_62EFB4 = v44;
                init_32_dword_62EEA8 = v43;
            }
            else
            {
                srcPtr2 = *gAudioFrameDataPtr;
            }
            v51 = 1 << (v64 - 1);
            v45 = (signed __int16)v45;
            if ((signed __int16)v45 != v70)
            {
                break;
            }
            c6 = v43 - v65;
            init_32_dword_62EEA8 = c6;
            v45 = v44 & ((1 << v65) - 1);
            v53 = v44 >> v65;
            gFirstAudioFrameDWORD_dword_62EFB4 = v53;
            if (c6 <= 16)
            {
                srcVal7 = *srcPtr2;
                ++srcPtr2;
                *gAudioFrameDataPtr = srcPtr2;
                v53 |= srcVal7 << c6;
                v24 = v66;
                c6 += 16;
                gFirstAudioFrameDWORD_dword_62EFB4 = v53;
                init_32_dword_62EEA8 = c6;
            }
            v51 = v71;
            v45 = (signed __int16)v45;
            if ((signed __int16)v45 != v71)
            {
                if (!(v45 & v71))
                    goto LABEL_34;
            LABEL_33:
                v45 = -(v45 & ~v51);
                goto LABEL_34;
            }
            c7 = c6 - v24;
            init_32_dword_62EEA8 = c7;
            v45 = v53 & ((1 << v24) - 1);
            v56 = v53 >> v24;
            gFirstAudioFrameDWORD_dword_62EFB4 = v56;
            if (c7 <= 16)
            {
                srcVal8 = *srcPtr2;
                *gAudioFrameDataPtr = srcPtr2 + 1;
                v24 = v66;
                gFirstAudioFrameDWORD_dword_62EFB4 = (srcVal8 << c7) | v56;
                init_32_dword_62EEA8 = c7 + 16;
            }
            v45 = (signed __int16)v45;
            if ((signed __int16)v45 & v72)
                v45 = -(v45 & ~v72);
        LABEL_34:
            v59 = v67;
            v67 = v68;
            v60 = 5 * v41 - 4 * v68;
            v68 = v41;
            v58 = (v59 + v60) >> 1;
            if (v73)
            {
                v61 = GetSoundTableValue(v58);
                v41 = (signed __int16)sub_408F50(v45 + v61);
            }
            else
            {
                v41 = (signed __int16)(v58 + (WORD)v45);
            }
            *v74 = v41;
            v62 = v75 == 1;
            v74 += gAudioFrameSizeBytes;
            --v75;
            if (v62)
                return SndRelated_sub_409650();
            v42 = v64;
        }
        if (!(v45 & v70))
            goto LABEL_34;
        goto LABEL_33;
    }
    return SndRelated_sub_409650();
}

typedef decltype(&sound16bitRelated_sub_4096B0) sound16bitRelated_sub_4096B0_type;
sound16bitRelated_sub_4096B0_type sound16bitRelated_sub_4096B0_ptr = (sound16bitRelated_sub_4096B0_type)0x4096B0;

int __cdecl decode_audio_frame(WORD *rawFrameBuffer, WORD *outPtr, signed int numSamplesPerFrame)
{
    int result; // eax@2

    SetupAudioDecodePtrs(rawFrameBuffer);
    if (false /*gAudioFrameSizeBits == 8*/)               // if mono
    {
        abort();
        /*
        Sound8BitRelated_sub_409200(outPtr, numSamplesPerFrame);
        result = gAudioFrameSizeBytes;
        if (gAudioFrameSizeBytes == 2)
        result = Sound8BitRelated_sub_409200((_BYTE *)outPtr + 1, numSamplesPerFrame);
        */
    }
    else
    {
        memset(outPtr, 0, numSamplesPerFrame * 4);

        sound16bitRelated_sub_4096B0(outPtr, numSamplesPerFrame);

        /*
        0x024960A0  fc ff 00 00 f2 ff 00 00 fe ff 00 00 09 00 00 00  üÿ..òÿ..þÿ......
        0x024960B0  0a 00 00 00 09 00 00 00 07 00 00 00 07 00 00 00  ................
        0x024960C0  06 00 00 00 03 00 00 00 05 00 00 00 05 00 00 00  ................
        0x024960D0  04 00 00 00 03 00 00 00 02 00 00 00 02 00 00 00  ................
        0x024960E0  01 00 00 00 fe ff 00 00 fe ff 00 00 ff ff 00 00  ....þÿ..þÿ..ÿÿ..
        0x024960F0  fc ff 00 00 fc ff 00 00 fb ff 00 00 fd ff 00 00  üÿ..üÿ..ûÿ..ýÿ..
        0x02496100  fb ff 00 00 fb ff 00 00 fa ff 00 00 f8 ff 00 00  ûÿ..ûÿ..úÿ..øÿ..
        0x02496110  f6 ff 00 00 f7 ff 00 00 f7 ff 00 00 f5 ff 00 00  öÿ..÷ÿ..÷ÿ..õÿ..
        0x02496120  f3 ff 00 00 f3 ff 00 00 f5 ff 00 00 f2 ff 00 00  óÿ..óÿ..õÿ..òÿ..
        0x02496130  f2 ff 00 00 f1 ff 00 00 ef ff 00 00 ef ff 00 00  òÿ..ñÿ..ïÿ..ïÿ..

        0x002A8E60  5b 00 00 00 38 00 00 00 fc ff 00 00 a4 ff 00 00  [...8...üÿ..¤ÿ..
        0x002A8E70  48 ff 00 00 e3 fe 00 00 8b fe 00 00 47 fe 00 00  Hÿ..ãþ...þ..Gþ..
        0x002A8E80  37 fe 00 00 4b fe 00 00 7b fe 00 00 b7 fe 00 00  7þ..Kþ..{þ..·þ..
        0x002A8E90  e4 8c 94 7f 12 1f 00 00 c4 00 2a 00 78 5f ff 04  äŒ”.....Ä.*.x_ÿ.
        0x002A8EA0  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
        */

       // sound16bitRelated_sub_4096B0_ptr(outPtr, numSamplesPerFrame);

        result = gAudioFrameSizeBytes;
        if (gAudioFrameSizeBytes == 2)
        {
        //    result = sound16bitRelated_sub_4096B0(outPtr + 1, numSamplesPerFrame);
         //   result = sound16bitRelated_sub_4096B0_ptr(outPtr + 1, numSamplesPerFrame);
        }
    }
    return result;
}


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

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED );
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
		SDL_PIXELFORMAT_ABGR8888,
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

// Video hook
char __fastcall decode_ddv_frame(void* hack, ddv_class *thisPtr, unsigned char* screenBuffer);
typedef decltype(&decode_ddv_frame) ddv__func5_block_decoder_q_type;

static ddv__func5_block_decoder_q_type real_ddv__func5_block_decoder_q = (ddv__func5_block_decoder_q_type)0x00409FE0;
static JmpHookedFunction<ddv__func5_block_decoder_q_type>* ddv_func6_decodes_block_q_hook;

// Sound hook
BYTE *__cdecl do_decode_audio_frame(ddv_class *thisPtr);
typedef decltype(&do_decode_audio_frame) do_decode_audio_frame_type;

static do_decode_audio_frame_type real_do_decode_audio_frame_type = (do_decode_audio_frame_type)0x0040DFE0;
static JmpHookedFunction<do_decode_audio_frame_type>* do_decode_audio_frame_type_hook;


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

/*
// One of these breaks prophcy DDV
typedef int(__cdecl *ddv_func7_DecodeMacroBlock)(void* params);
static ddv_func7_DecodeMacroBlock ddv_func7_DecodeMacroBlock_ptr = (ddv_func7_DecodeMacroBlock)0x0040E6B0;

typedef signed int(__cdecl* after_block_decode_no_effect_q)(int a1);
static after_block_decode_no_effect_q after_block_decode_no_effect_q_ptr = (after_block_decode_no_effect_q)0x0040E360;
*/

#include "PSXMDECDecoder.h"

int __cdecl SetAudioFrameSizeBytesAndBits(int audioFrameSizeBytes, int audioFrameSizeBits)
{
    int result; // eax@1

    result = audioFrameSizeBytes;
    gAudioFrameSizeBytes = audioFrameSizeBytes;
   // gAudioFrameSizeBits = audioFrameSizeBits;
    return result;
}


// 0040DBB0
BYTE *__cdecl do_decode_audio_frame(ddv_class *thisPtr)
{
    BYTE *result; // eax@3

    if (thisPtr->mHasAudio && thisPtr->mAudioFrameNumber < thisPtr->mNumberOfFrames)
    {
        SetAudioFrameSizeBytesAndBits(thisPtr->mAudioFrameSizeBytesQ, thisPtr->mAudioFrameSizeBitsQ);
        decode_audio_frame(thisPtr->mAudioFrameBuffer, (WORD *)thisPtr->mDecodedSoundBuffer, thisPtr->mSingleAudioFrameSize);
        ++thisPtr->mAudioFrameNumber;
        result = (BYTE*)thisPtr->mDecodedSoundBuffer;
    }
    else
    {
        ++thisPtr->mAudioFrameNumber;
        result = 0;
    }
    return result;

}

char __fastcall decode_ddv_frame(void* hack, ddv_class *thisPtr, unsigned char* pScreenBuffer)
{
    //StartSDL();

    return 0;

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
                Macroblock_RGB[x][y].Blue,
                Macroblock_RGB[x][y].Green,
                Macroblock_RGB[x][y].Red));
                

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
    do_decode_audio_frame_type_hook = new JmpHookedFunction<do_decode_audio_frame_type>(real_do_decode_audio_frame_type, &do_decode_audio_frame);

}
