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

ddv__func5_block_decoder_q_type real_ddv__func5_block_decoder_q = (ddv__func5_block_decoder_q_type)0x00409FE0;
JmpHookedFunction<ddv__func5_block_decoder_q_type>* ddv_func6_decodes_block_q_hook;


/*int __declspec (naked)*/ void  calling_conv_hack(int param)
{
    __asm
    {
        mov ecx, param
        push ecx
        call write_block_bit1_no_mmx_ptr
        pop ecx
    }
}

//B2, D6 07
/*int __declspec (naked)*/ void  calling_conv_hack2(int macroBlockBuffer, int* decodedBitStream)
{
    __asm
    {
        push decodedBitStream
        push macroBlockBuffer
        call blit_output_no_mmx_ptr
        add esp, 8
    }
}

char __fastcall ddv__func5_block_decoder_q(void* hack, ddv_class *thisPtr, unsigned char* pScreenBuffer)
{
    OutputDebugString("ddv__func5_block_decoder_q\n");

    // Take the non MMX path
    *p_gCpuSupportsMMX = false;

    int(__cdecl *decodeMacroBlockfPtr)(int, int *, int, DWORD, int, int *); // edi@2

    int decode6_mmx_ret = 0; // ebx@4
    int dataSizeDWords = 0; // ebp@4
    int buffer = 0; // esi@4
    int dataSizeBytes = 0; // ebp@7
    int decode1_ret = 0; // ebx@14
    int block2Ptr = 0; // esi@14
    int decode2_ret = 0; // ebx@14
    int block3Ptr = 0; // esi@14
    int decode3_ret = 0; // ebx@14
    int block4Ptr = 0; // esi@14
    int decode4_ret = 0; // ebx@14
    int block5Ptr = 0; // esi@14
    int decode5_ret = 0; // ebx@14
    int block6Ptr = 0; // esi@14
    unsigned __int8 *pScreenBufferCurrentPos = 0; // [sp+10h] [bp-14h]@5

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

    buffer = thisPtr->mMacroBlockBuffer_q;
    
    // Done once for the whole 320x240 image
    int v9 = decode_bitstream_q_ptr((WORD*)thisPtr->mRawFrameBitStreamData, (unsigned int*)thisPtr->mDecodedBitStream);
    after_block_decode_no_effect_q_ptr(v9); // actually does have an effect
    decode6_mmx_ret = (int)thisPtr->mDecodedBitStream;

    dataSizeDWords = thisPtr->mBlockDataSize_q; // 64 - because 64 coefficients?

    // Sanity check
    if (thisPtr->nNumMacroblocksX <= 0 || thisPtr->nNumMacroblocksY <= 0)
    {
        return 0;
    }

    // For 320x240 image we have a 20x16 macro block grid (because 320/16 and 240/16)
    for (unsigned int xBlock = 0; xBlock < thisPtr->nNumMacroblocksX; xBlock++)
    {
        pScreenBufferCurrentPos = pScreenBuffer;

        for (unsigned int yBlock = 0; yBlock < thisPtr->nNumMacroblocksY; yBlock++)
        {
            // B1
            decode1_ret = decodeMacroBlockfPtr(decode6_mmx_ret, p_gMacroBlock1Buffer, buffer, 0 /*14h*/, 0, 0);
            calling_conv_hack2(buffer, p_gMacroBlock1Buffer); // << didn't change gMacroBlock1Buffer!

            dataSizeBytes = 4 * dataSizeDWords;
            block2Ptr = dataSizeBytes + buffer;

            // B2
            decode2_ret = decodeMacroBlockfPtr(decode1_ret, p_gMacroBlock2Buffer, block2Ptr, 0, buffer, p_gMacroBlock1Buffer); // last 2 args are unused?
            calling_conv_hack2(block2Ptr, p_gMacroBlock2Buffer);
            block3Ptr = dataSizeBytes + block2Ptr;

            // B3
            decode3_ret = decodeMacroBlockfPtr(decode2_ret, p_gMacroBlock3Buffer, block3Ptr, 1, block2Ptr, p_gMacroBlock2Buffer);
            calling_conv_hack2(block3Ptr, p_gMacroBlock3Buffer);
            block4Ptr = dataSizeBytes + block3Ptr;

            // B4
            decode4_ret = decodeMacroBlockfPtr(decode3_ret, p_gMacroBlock4Buffer, block4Ptr, 1, block3Ptr, p_gMacroBlock3Buffer);
            calling_conv_hack2(block4Ptr, p_gMacroBlock4Buffer);
            block5Ptr = dataSizeBytes + block4Ptr;

            // B5
            decode5_ret = decodeMacroBlockfPtr(decode4_ret, p_gMacroBlock5Buffer, block5Ptr, 1, block4Ptr, p_gMacroBlock4Buffer);
            calling_conv_hack2(block5Ptr, p_gMacroBlock5Buffer);
            block6Ptr = dataSizeBytes + block5Ptr;

            // B6
            decode6_mmx_ret = decodeMacroBlockfPtr(decode5_ret, p_gMacroBlock6Buffer, block6Ptr, 1, block5Ptr, p_gMacroBlock5Buffer);
            calling_conv_hack2(block6Ptr, p_gMacroBlock6Buffer);
            buffer = dataSizeBytes + block6Ptr;

            if (dword_62EFE0 & 1) // Hit on 2x2 dithering
            {
                calling_conv_hack((int)pScreenBufferCurrentPos);
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
                // Not expected for the test video
                abort();
                //write_block_other_bits_no_mmx(dataSizeBytes);// half height, every other horizontal block is skipped?
            }

            const int v36 = (int)&pScreenBufferCurrentPos[16 * dword_62EFD8];// dword_62EFD8 is probably the buffer width? 1280 - always 1280?

            pScreenBufferCurrentPos += 16 * dword_62EFD8;

            if (dword_62EFE0 & 8)
            {
                pScreenBufferCurrentPos = (unsigned __int8 *)(16 * dword_62EFD8 + v36);
            }

            dataSizeDWords = dataSizeDWords;
        }
        pScreenBuffer += dword_62EFD4;              // not sure what this is, height maybe? No scaling = 32, 2x1 = 64, 2x2=64
    }

    // The app doesn't seem to do anything with the return value
    return 0;
}

JmpHookedFunction<ddv_func7_DecodeMacroBlock>* ddv_func7_DecodeMacroBlock_hook;





void InstallHooks()
{
    ddv_func6_decodes_block_q_hook = new JmpHookedFunction<ddv__func5_block_decoder_q_type>(real_ddv__func5_block_decoder_q, &ddv__func5_block_decoder_q);
}
