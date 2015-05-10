#include "Hooks.hpp"
#include "jmphookedfunction.hpp"
#include <algorithm>
#include <string>
#include <set>

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
char __fastcall decode_ddv_frame(void* hack, ddv_class *thisPtr, unsigned char* screenBuffer);
typedef decltype(&decode_ddv_frame) ddv__func5_block_decoder_q_type;

static ddv__func5_block_decoder_q_type real_ddv__func5_block_decoder_q = (ddv__func5_block_decoder_q_type)0x00409FE0;
static JmpHookedFunction<ddv__func5_block_decoder_q_type>* ddv_func6_decodes_block_q_hook;


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

static void ConvertYuvToRgbAndBlit(unsigned short int* pFrameBuffer, int xoff, int yoff);
int __cdecl decode_bitstream(WORD *pFrameData, unsigned short int *pOutput);

static char __fastcall decode_ddv_frame(void* hack, ddv_class *thisPtr, unsigned char* pScreenBuffer)
{
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
    const int firstWordOfRawBitStreamData = decode_bitstream((WORD*)thisPtr->mRawFrameBitStreamData, (unsigned short int*)thisPtr->mDecodedBitStream);

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

    // The app doesn't seem to do anything with the return value
    return 0;
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
    *pOut = v3 >> (32 - 16);
    ++pOut;

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

/*
42A5C0 - indexing at * 4 returning 1 (bits to shift)
42A5C2 - indexing at * 2 returning 2 (output word)

41A5C0 - indexing * 8 returning 1 (bits to shift)
41A5C2 - indexing * 4 returning 2 - sets lo word
41A5C4 - indeding * 4 returning 2 - sets lo word
41A5C6 - indexing * 4 returning 2 - sets lo word

0x42A5C0-0x41A5C0
0x10000 = 65536, 64kb table x2

Unique elements of table 2:

Bits
0
6
7
8
9
10
11
12
13

W1
0
4
7
8
9
10
11
1013
1014
1015
1016
1017
1020
1026
1028
1029
2043
2044
2046
2050
2051
2052
3068
3069
3070
3075
4093
4098
4099
5117
5118
5121
5122
6142
6143
6145
6146
7166
7167
7169
7170
8190
8191
8193
8194
9214
9215
9217
10239
14337
15359
15361
16383
16385
17407
17409
18431
18433
19455
19457
20479
20481
21503
21505
22527
31775 0x7C1F

W2
0
1
2
3
1021
1022
1023
1025
2047
2049
3071
3073
4095
4097
5119
31775
65024

W3
0
1
1023
65024
*/

int __cdecl decode_bitstream(WORD *pFrameData, unsigned short int *pOutput)
{
    DWORD rawWord4; // eax@1
    DWORD v3; // edx@1
    char bitsToShiftBy; // cl@1
    unsigned short int* pOut; // edi@1
    WORD* rawBitStreamPtr; // esi@1
    int v8; // edx@1


    unsigned int table_index_2; // ebx@2
    char tblValueBits; // cl@3
    int rawWord3; // eax@4
    int v15; // eax@8
    int rawWord5; // eax@10
    int t11Bits; // eax@15
    int rawWord7; // eax@17
    int tmp11Bits2; // eax@22
    int rawWord9; // eax@24
    int table_index_1; // ebx@26
    int rawWord1; // eax@27

    int rawWord2; // eax@29
    int AC_Coefficient; // [sp+Ch] [bp-4h]@1
    DWORD* secondWordPtr; // [sp+18h] [bp+8h]@1


    AC_Coefficient = *pFrameData;

    secondWordPtr = (DWORD*)(pFrameData + 1);

    v8 = *secondWordPtr; // Last used

    __asm
    {
        rol v8, 16
    }

    rawBitStreamPtr = (pFrameData + 3);

    rawWord4 = (v8 >> (32 - 11)) & MASK_11_BITS; 

    v3 = v8 << 11;      //  number of zero-value AC Coefficients? End v8 use
    bitsToShiftBy = 11;
  
    *pOutput = rawWord4; // store in output
    pOut = pOutput + 1;

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
                                    table_index_2 = v3 >> (32-13); // 0x1FFF / 8191 table size? 8192/8=1024 entries?
                                    if (table_index_2 >= 32)
                                    {
                                        break;
                                    }
                                    table_index_1 = v3 >> (32-17); // 0x1FFFF / 131071, 131072/4=32768 entries?
                                  
                                    v3 = v3 << 8;
                                    bitsToShiftBy = bitsToShiftBy + 8;

                                    CheckForEscapeCode(bitsToShiftBy, rawWord1, rawBitStreamPtr, rawWord4, v3);

                                   
                                    const char bitsToShiftFromTbl = gTbl1[table_index_1].mBitsToShift;
                                    v3 = v3 << bitsToShiftFromTbl;
                                    bitsToShiftBy = bitsToShiftBy + bitsToShiftFromTbl;
                                    
                                    CheckForEscapeCode(bitsToShiftBy, rawWord2, rawBitStreamPtr, rawWord4, v3);

                                    // Everything in the table is 0's after 4266 bytes 4266/2=2133 to perhaps 2048/4096 is max?
                                    *pOut++ = gTbl1[table_index_1].mOutputWord;

                                } // End while


                                tblValueBits = gTbl2[table_index_2].mBitsToShift;
                                v3 <<= tblValueBits;
                                bitsToShiftBy = bitsToShiftBy + tblValueBits;

                                CheckForEscapeCode(bitsToShiftBy, rawWord3, rawBitStreamPtr, rawWord4, v3);

                                SetLoWord(rawWord4, gTbl2[table_index_2].mOutputWord1);

                                if ((WORD)rawWord4 != 0x7C1F) // 0b 11111 00000 11111
                                {
                                    break;
                                }

                                OutputWordAndAdvance(rawBitStreamPtr, rawWord4, pOut, bitsToShiftBy, v3);
                            } // End while

                            *pOut++ = rawWord4;

                            if ((WORD)rawWord4 == MDEC_END)
                            {
                                v15 = v3 >> (32 - 11);
                                if (v15 == MASK_10_BITS)
                                {
                                    return AC_Coefficient;
                                }
                                rawWord4 = v15 & MASK_11_BITS;
                                v3 <<= 11;
                                *pOut = rawWord4;
                                bitsToShiftBy += 11;

                                ++pOut;
                                
                                CheckForEscapeCode(bitsToShiftBy, rawWord5, rawBitStreamPtr, rawWord4, v3);

                            }

                            // Set low word
                            SetLoWord(rawWord4, gTbl2[table_index_2].mOutputWord2);
                        } while (!(WORD)rawWord4);
                        
                       
                        if ((WORD)rawWord4 != 0x7C1F)
                        {
                            break;
                        }

                        OutputWordAndAdvance(rawBitStreamPtr, rawWord4, pOut, bitsToShiftBy, v3);
                    } // End while
                    
                    *pOut++ = rawWord4;

                    if ((WORD)rawWord4 == MDEC_END)
                    {
                        t11Bits = v3 >> (32 - 11);
                        if (t11Bits == MASK_10_BITS)
                        {
                            return AC_Coefficient;
                        }
                        rawWord4 = t11Bits & MASK_11_BITS;
                        *pOut++ = rawWord4;
                        bitsToShiftBy += 11;
                        v3 <<= 11;

                        CheckForEscapeCode(bitsToShiftBy, rawWord7, rawBitStreamPtr, rawWord4, v3);
                    }

                    SetLoWord(rawWord4, gTbl2[table_index_2].mOutputWord3);

                } while (!(WORD)rawWord4);

              
                if ((WORD)rawWord4 != 0x7C1F)
                {
                    break;
                }


                OutputWordAndAdvance(rawBitStreamPtr, rawWord4, pOut, bitsToShiftBy, v3);
            } // End while

            *pOut++ = rawWord4;

        } while ((WORD)rawWord4 != MDEC_END);       // FE 00 is in the raw data
        
        tmp11Bits2 = v3 >> (32-11);
        if (tmp11Bits2 == MASK_10_BITS) // 10 bits
        {
            break;
        }
        rawWord4 = tmp11Bits2 & MASK_11_BITS; // Mask 11 bits
        *pOut++ = rawWord4; // Add to output
        bitsToShiftBy += 11; // Consumed 11 bits
        v3 <<= 11; // Shift v4 by consumed bits - now  number of zero-value AC Coefficients?

        CheckForEscapeCode(bitsToShiftBy, rawWord9, rawBitStreamPtr, rawWord4, v3);

    }
    return AC_Coefficient;
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
