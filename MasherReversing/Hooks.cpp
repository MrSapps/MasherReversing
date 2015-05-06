#include "Hooks.hpp"
#include "jmphookedfunction.hpp"
#include <algorithm>

#undef min
#undef max
#undef RGB

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
char __fastcall ddv__func5_block_decoder_q(void* hack, ddv_class *thisPtr, unsigned char* screenBuffer);
typedef decltype(&ddv__func5_block_decoder_q) ddv__func5_block_decoder_q_type;

static ddv__func5_block_decoder_q_type real_ddv__func5_block_decoder_q = (ddv__func5_block_decoder_q_type)0x00409FE0;
static JmpHookedFunction<ddv__func5_block_decoder_q_type>* ddv_func6_decodes_block_q_hook;

// Wrappers for hand made calling conventions
static void do_write_block_bit1_no_mmx(int param) // TODO: Reimpl
{
    __asm
    {
        mov ecx, param
        push ecx
        call write_block_bit1_no_mmx_ptr
        pop ecx
    }
}

static void do_write_block_other_bits_no_mmx(int param) // TODO: Reimpl
{
    __asm
    {
        mov ecx, param
            push ecx
            call write_block_other_bits_no_mmx_ptr
            pop ecx
    }
}

static void do_blit_output_no_mmx(int macroBlockBuffer, int* decodedBitStream) // TODO: Reimpl
{
    __asm
    {
        push decodedBitStream
        push macroBlockBuffer
        call blit_output_no_mmx_ptr
        add esp, 8
    }
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

static char __fastcall ddv__func5_block_decoder_q(void* hack, ddv_class *thisPtr, unsigned char* pScreenBuffer)
{
    OutputDebugString("ddv__func5_block_decoder_q\n");

    // Take the non MMX path - this function replaces the code that was gurded by this, probably don't matter what its set to now :)
    *p_gCpuSupportsMMX = false;
    // Force no dithering or scaling
    //dword_62EFE0 = 0;
   // gMacroBlockStripWidthInBytes = 32;


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
        //decodeMacroBlockfPtr = (int(__cdecl *)(int, int *, int, _DWORD, int, int *))DecodeMacroBlockReleated_Q;
    }
    else
    {
        // gending uses this one - this outputs macroblock coefficients?
        decodeMacroBlockfPtr = (int(__cdecl *)(int, int *, int, DWORD, int, int *))ddv_func7_DecodeMacroBlock_ptr;
    }

    DWORD block1Output = thisPtr->mMacroBlockBuffer_q;
    
    // Done once for the whole 320x240 image
    const int firstWordOfRawBitStreamData = decode_bitstream_q_ptr((WORD*)thisPtr->mRawFrameBitStreamData, (unsigned int*)thisPtr->mDecodedBitStream); // TODO: Reimpl

    // Each block only seems to have 1 colour if this isn't called, but then resizing the window seems to fix it sometimes (perhaps causes
    // this function to be called else where).
    after_block_decode_no_effect_q_ptr(firstWordOfRawBitStreamData); // TODO: Reimpl

    // Sanity check
    if (thisPtr->nNumMacroblocksX <= 0 || thisPtr->nNumMacroblocksY <= 0)
    {
        return 0;
    }

    int bitstreamCurPos = (int)thisPtr->mDecodedBitStream;
    
    int xoff = 0;
    auto buf = (unsigned short int*)pScreenBuffer;

    // For 320x240 image we have a 20x16 macro block grid (because 320/16 and 240/16)
    for (unsigned int xBlock = 0; xBlock < thisPtr->nNumMacroblocksX; xBlock++)
    {
        int yoff = 0;
        for (unsigned int yBlock = 0; yBlock < thisPtr->nNumMacroblocksY; yBlock++)
        {
            // 2 chroma blocks and 4 Luma in YUV 4:2:0

            // B1
            const int afterBlock1Ptr = decodeMacroBlockfPtr(bitstreamCurPos, Cr_block, block1Output, 0, 0, 0);
            do_blit_output_no_mmx(block1Output, Cr_block); // Inverse DCT?

            const int dataSizeBytes = 4 * thisPtr->mBlockDataSize_q; // Convert to byte count 64*4=256
            int block2Output = dataSizeBytes + block1Output;

            // B2
            const int afterBlock2Ptr = decodeMacroBlockfPtr(afterBlock1Ptr, Cb_block, block2Output, 0, 0, 0);
            do_blit_output_no_mmx(block2Output, Cb_block);
            const int block3Output = dataSizeBytes + block2Output;

            // B3
            const int afterBlock3Ptr = decodeMacroBlockfPtr(afterBlock2Ptr, Y1_block, block3Output, 1, 0, 0);
            do_blit_output_no_mmx(block3Output, Y1_block);
            const int block4Output = dataSizeBytes + block3Output;
  
            // B4
            const int afterBlock4Ptr = decodeMacroBlockfPtr(afterBlock3Ptr, Y2_block, block4Output, 1, 0, 0);
            do_blit_output_no_mmx(block4Output, Y2_block);
            const int block5Output = dataSizeBytes + block4Output;

            // B5
            const int afterBlock5Ptr = decodeMacroBlockfPtr(afterBlock4Ptr, Y3_block, block5Output, 1, 0, 0);
            do_blit_output_no_mmx(block5Output, Y3_block);
            const int block6Output = dataSizeBytes + block5Output;

            // B6
            bitstreamCurPos = decodeMacroBlockfPtr(afterBlock5Ptr, Y4_block, block6Output, 1, 0, 0);
            do_blit_output_no_mmx(block6Output, Y4_block);
            block1Output = dataSizeBytes + block6Output;
            

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

                    SetElement(x + xoff, y + yoff, buf,
                        RGB565(
                        Macroblock_RGB[x][y].Red,
                        Macroblock_RGB[x][y].Green,
                        Macroblock_RGB[x][y].Blue));
                }
            }
            yoff += 16;
        }
        xoff += 16;
    }


    // The app doesn't seem to do anything with the return value
    return 0;
}

/*
int __cdecl decode_bitstream_q(_WORD *pFrameData, unsigned int *pOutput)
{
    int v2; // eax@1
    int v3; // edx@1
    char v4; // cl@1
    int v5; // edi@1
    int v6; // esi@1
    int firstFrameWord; // eax@1
    int v8; // edx@1
    __int64 v9; // qt0@1
    unsigned int v10; // ebx@2
    char v11; // ch@3
    char v12; // cl@3
    int v13; // eax@4
    __int64 v14; // qt0@6
    int v15; // eax@8
    int v16; // eax@10
    __int64 v17; // qt0@13
    int v18; // eax@13
    int v19; // eax@15
    int v20; // eax@17
    __int64 v21; // qt0@20
    int v22; // eax@20
    int v23; // eax@22
    int v24; // eax@24
    int v25; // edx@26
    char v26; // cl@26
    int v27; // ebx@26
    int v28; // eax@27
    char v29; // ch@28
    char v30; // cl@28
    int v31; // eax@29
    int data_size_q; // [sp+Ch] [bp-4h]@1
    unsigned int v34; // [sp+18h] [bp+8h]@1

    firstFrameWord = *pFrameData;
    data_size_q = firstFrameWord;
    v34 = (unsigned int)(pFrameData + 1);
    v8 = __ROL__(*(_DWORD *)(2 * (v34 >> 1)), 16);
    v6 = (v34 >> 1) + 2;
    HIDWORD(v9) = firstFrameWord;
    LODWORD(v9) = v8;
    v3 = v8 << 11;
    v4 = 11;
    v2 = ((unsigned __int64)(v9 << 11) >> 32) & 0x7FF;
    *(_WORD *)(2 * ((unsigned int)pOutput >> 1)) = v2;// store in output
    v5 = ((unsigned int)pOutput >> 1) + 1;
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
                                    v10 = (unsigned __int64)(unsigned int)v3 << 13 >> 32;
                                    if (v10 >= 0x20)
                                        break;
                                    v27 = (unsigned __int64)(unsigned int)v3 << 17 >> 32;
                                    v26 = v4 + 8;
                                    v25 = v3 << 8;
                                    if (v26 & 0x10)
                                    {
                                        v28 = *(_WORD *)(2 * v6);
                                        v26 &= 0xFu;
                                        ++v6;
                                        v2 = v28 << v26;
                                        v25 |= v2;
                                    }
                                    v29 = v26;
                                    v30 = byte_42A5C0[4 * v27];
                                    v3 = v25 << v30;
                                    v4 = v29 + v30;
                                    if (v4 & 0x10)
                                    {
                                        v31 = *(_WORD *)(2 * v6);
                                        v4 &= 0xFu;
                                        ++v6;
                                        v2 = v31 << v4;
                                        v3 |= v2;
                                    }
                                    *(_WORD *)(2 * v5++) = word_42A5C2[2 * v27];
                                }
                                v11 = v4;
                                v12 = byte_41A5C0[8 * v10];
                                v3 <<= v12;
                                v4 = v11 + v12;
                                if (v4 & 0x10)
                                {
                                    v13 = *(_WORD *)(2 * v6);
                                    v4 &= 0xFu;
                                    ++v6;
                                    v2 = v13 << v4;
                                    v3 |= v2;
                                }
                                LOWORD(v2) = word_41A5C2[4 * v10];
                                if ((_WORD)v2 != 31775)
                                    break;
                                HIDWORD(v14) = v2;
                                LODWORD(v14) = v3;
                                *(_WORD *)(2 * v5) = (unsigned __int64)(v14 << 16) >> 32;
                                v2 = *(_WORD *)(2 * v6++) << v4;
                                v3 = v2 | (v3 << 16);
                                ++v5;
                            }
                            *(_WORD *)(2 * v5++) = v2;
                            if ((_WORD)v2 == 0xFE00u)
                            {
                                v15 = (unsigned __int64)(unsigned int)v3 << 11 >> 32;
                                if (v15 == 0x3FF)
                                    return data_size_q;
                                v2 = v15 & 0x7FF;
                                v3 <<= 11;
                                *(_WORD *)(2 * v5) = v2;
                                v4 += 11;
                                ++v5;
                                if (v4 & 0x10)
                                {
                                    v16 = *(_WORD *)(2 * v6);
                                    v4 &= 0xFu;
                                    ++v6;
                                    v2 = v16 << v4;
                                    v3 |= v2;
                                }
                            }
                            LOWORD(v2) = word_41A5C4[4 * v10];
                        } while (!(_WORD)v2);
                        if ((_WORD)v2 != 31775)
                            break;
                        HIDWORD(v17) = v2;
                        LODWORD(v17) = v3;
                        *(_WORD *)(2 * v5) = (unsigned __int64)(v17 << 16) >> 32;
                        v18 = *(_WORD *)(2 * v6++);
                        v2 = v18 << v4;
                        ++v5;
                        v3 = v2 | (v3 << 16);
                    }
                    *(_WORD *)(2 * v5++) = v2;
                    if ((_WORD)v2 == 0xFE00u)
                    {
                        v19 = (unsigned __int64)(unsigned int)v3 << 11 >> 32;
                        if (v19 == 1023)
                            return data_size_q;
                        v2 = v19 & 0x7FF;
                        *(_WORD *)(2 * v5++) = v2;
                        v4 += 11;
                        v3 <<= 11;
                        if (v4 & 0x10)
                        {
                            v20 = *(_WORD *)(2 * v6);
                            v4 &= 0xFu;
                            ++v6;
                            v2 = v20 << v4;
                            v3 |= v2;
                        }
                    }
                    LOWORD(v2) = word_41A5C6[4 * v10];
                } while (!(_WORD)v2);
                if ((_WORD)v2 != 31775)
                    break;
                HIDWORD(v21) = v2;
                LODWORD(v21) = v3;
                *(_WORD *)(2 * v5) = (unsigned __int64)(v21 << 16) >> 32;
                v22 = *(_WORD *)(2 * v6++);
                v2 = v22 << v4;
                ++v5;
                v3 = v2 | (v3 << 16);
            }
            *(_WORD *)(2 * v5++) = v2;
        } while ((_WORD)v2 != 0xFE00u);
        v23 = (unsigned __int64)(unsigned int)v3 << 11 >> 32;
        if (v23 == 0x3FF)
            break;
        v2 = v23 & 0x7FF;
        *(_WORD *)(2 * v5++) = v2;
        v4 += 11;
        v3 <<= 11;
        if (v4 & 0x10)
        {
            v24 = *(_WORD *)(2 * v6);
            v4 &= 0xFu;
            ++v6;
            v2 = v24 << v4;
            v3 |= v2;
        }
    }
    return data_size_q;
}
*/


void SetElement(int x, int y, unsigned short int* ptr, unsigned short int value)
{
    const int kWidth = 640;
    ptr[(kWidth * y) + x] = value;
}

static JmpHookedFunction<ddv_func7_DecodeMacroBlock>* ddv_func7_DecodeMacroBlock_hook;

void InstallHooks()
{
    ddv_func6_decodes_block_q_hook = new JmpHookedFunction<ddv__func5_block_decoder_q_type>(real_ddv__func5_block_decoder_q, &ddv__func5_block_decoder_q);
}
