#include "Hooks.hpp"
#include "jmphookedfunction.hpp"

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

static char __fastcall ddv__func5_block_decoder_q(void* hack, ddv_class *thisPtr, unsigned char* pScreenBuffer)
{
    OutputDebugString("ddv__func5_block_decoder_q\n");

    // Take the non MMX path - this function replaces the code that was gurded by this, probably don't matter what its set to now :)
    *p_gCpuSupportsMMX = false;

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

    // For 320x240 image we have a 20x16 macro block grid (because 320/16 and 240/16)
    for (unsigned int xBlock = 0; xBlock < thisPtr->nNumMacroblocksX; xBlock++)
    {
        unsigned char* pScreenBufferCurrentPos = pScreenBuffer;
  
        for (unsigned int yBlock = 0; yBlock < thisPtr->nNumMacroblocksY; yBlock++)
        {
      
            // B1
            const int afterBlock1Ptr = decodeMacroBlockfPtr(bitstreamCurPos, p_gMacroBlock1Buffer, block1Output, 0, 0, 0);
            do_blit_output_no_mmx(block1Output, p_gMacroBlock1Buffer);

            const int dataSizeBytes = 4 * thisPtr->mBlockDataSize_q; // Convert to byte count 64*4=256
            int block2Output = dataSizeBytes + block1Output;

            // B2
            const int afterBlock2Ptr = decodeMacroBlockfPtr(afterBlock1Ptr, p_gMacroBlock2Buffer, block2Output, 0, 0, 0);
            do_blit_output_no_mmx(block2Output, p_gMacroBlock2Buffer);
            const int block3Output = dataSizeBytes + block2Output;

            // B3
            const int afterBlock3Ptr = decodeMacroBlockfPtr(afterBlock2Ptr, p_gMacroBlock3Buffer, block3Output, 1, 0, 0);
            do_blit_output_no_mmx(block3Output, p_gMacroBlock3Buffer);
            const int block4Output = dataSizeBytes + block3Output;

            // B4
            const int afterBlock4Ptr = decodeMacroBlockfPtr(afterBlock3Ptr, p_gMacroBlock4Buffer, block4Output, 1, 0, 0);
            do_blit_output_no_mmx(block4Output, p_gMacroBlock4Buffer);
            const int block5Output = dataSizeBytes + block4Output;

            // B5
            const int afterBlock5Ptr = decodeMacroBlockfPtr(afterBlock4Ptr, p_gMacroBlock5Buffer, block5Output, 1, 0, 0);
            do_blit_output_no_mmx(block5Output, p_gMacroBlock5Buffer);
            const int block6Output = dataSizeBytes + block5Output;

            // B6
            bitstreamCurPos = decodeMacroBlockfPtr(afterBlock5Ptr, p_gMacroBlock6Buffer, block6Output, 1, 0, 0);
            do_blit_output_no_mmx(block6Output, p_gMacroBlock6Buffer);
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

                // Not expected for the test video
                abort();
                //write_block_other_bits_no_mmx(dataSizeBytes);// half height, every other horizontal block is skipped?
            }

            // Moves the blit pos down by 16 pixels (1280 is from (320*2)*2 which is the frame width in bytes doubled)
            pScreenBufferCurrentPos += 16 * gMacroBlockHeightSpacing;

            if (dword_62EFE0 & 8)
            {
                // Moves by another 16, thus 32 pixels down, this is for when the output is scaled by x2
                pScreenBufferCurrentPos += 16 * gMacroBlockHeightSpacing;
            }
        }

        // Amount to move along the frame buffer to next to the next vertical "strip" of blocks to write


        // Seems to be the width of the blitted macro block strip in bytes
        // thus no scaling = 16 pixels
        // 2x1 scaling = 32 pixels
        // and 2x2 scaling = 32 pixels
        pScreenBuffer += gMacroBlockStripWidthInBytes; 
    }

    // The app doesn't seem to do anything with the return value
    return 0;
}

static JmpHookedFunction<ddv_func7_DecodeMacroBlock>* ddv_func7_DecodeMacroBlock_hook;

void InstallHooks()
{
    ddv_func6_decodes_block_q_hook = new JmpHookedFunction<ddv__func5_block_decoder_q_type>(real_ddv__func5_block_decoder_q, &ddv__func5_block_decoder_q);
}
