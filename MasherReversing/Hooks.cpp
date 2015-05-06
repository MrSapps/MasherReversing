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
    dword_62EFE0 = 0;
    gMacroBlockStripWidthInBytes = 32;

    int(__cdecl *decodeMacroBlockfPtr)(int, int *, int, DWORD, int, int *) = nullptr;

    if (!thisPtr->mHasVideo)
    {
        return 0;
    }

    ++thisPtr->field_6C;
  
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
    int decodeRet = decode_bitstream_q_ptr((WORD*)thisPtr->mRawFrameBitStreamData, (unsigned int*)thisPtr->mDecodedBitStream); // TODO: Reimpl

    // Each block only seems to have 1 colour if this isn't called, but then resizing the window seems to fix it sometimes (perhaps causes
    // this function to be called else where).
    after_block_decode_no_effect_q_ptr(decodeRet); // TODO: Reimpl

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
        unsigned char* pScreenBufferCurrentPos = pScreenBuffer;
  
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

            if (dword_62EFE0 & 1) // Hit on 2x2 dithering
            {
                do_write_block_bit1_no_mmx((int)pScreenBufferCurrentPos);
                //  write_block_bit1_no_mmx_ptr(dataSizeBytes);// sets width too small and images look weird
            }
            else if (dword_62EFE0 & 4)
            {
                // Not expected for the test video
                abort();
                //write_block_bit4_no_mmx(dataSizeBytes);// plays at half height?
            }
            else if (dword_62EFE0 & 8) // Hit on "no dithering", 2x1 dithering
            {
                abort();
                // Not expected for the test video
                //write_block_bit8_no_mmx(dataSizeBytes);// seems to be the "normal" case
            }
            else
            {
                // When no dithering or scaling?

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
                        const float r = (Macroblock_YCbCr[x][y].Y ) + 1.402f * Macroblock_YCbCr[x][y].Cb;
                        const float g = (Macroblock_YCbCr[x][y].Y ) - 0.3437f * Macroblock_YCbCr[x][y].Cr - 0.7143f * Macroblock_YCbCr[x][y].Cb;
                        const float b = (Macroblock_YCbCr[x][y].Y ) + 1.772f * Macroblock_YCbCr[x][y].Cr;
                     


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

                // The code above replaces this
                //do_write_block_other_bits_no_mmx((int)pScreenBufferCurrentPos);// half height, every other horizontal block is skipped?
            }

            // Moves the blit pos down by 16 pixels (1280 is from (320*2)*2 which is the frame width in bytes doubled)
            pScreenBufferCurrentPos += 16 * gMacroBlockHeightSpacing;

            if (dword_62EFE0 & 8)
            {
                // Moves by another 16, thus 32 pixels down, this is for when the output is scaled by x2
                pScreenBufferCurrentPos += 16 * gMacroBlockHeightSpacing;
            }

            yoff += 16;
        }

        // Amount to move along the frame buffer to next to the next vertical "strip" of blocks to write

        xoff += 16;

        pScreenBuffer += gMacroBlockStripWidthInBytes; 
    }


    // The app doesn't seem to do anything with the return value
    return 0;
}

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
