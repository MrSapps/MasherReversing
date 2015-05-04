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
    BYTE mHasAudioQ;
    BYTE field_61;
    BYTE field_62;
    BYTE field_63;
    DWORD mCurrentFrameNumber;
    DWORD mCurrentFrameNumber2;
    DWORD field_6C;
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

static void blit_output_no_mmx(int)
{
    // TODO
}

static void write_block_bit4_no_mmx(int)
{
    // TODO
}

static void write_block_bit8_no_mmx(int)
{
    // TODO
}

static void write_block_other_bits_no_mmx(int)
{
    // TODO
}

int __SETO__(int, int)
{
    // TODO
    return 0;
}


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


char __fastcall ddv__func5_block_decoder_q(void* hack, ddv_class *thisPtr, unsigned char* pScreenBuffer)
{
    OutputDebugString("ddv__func5_block_decoder_q\n");

    // Take the non MMX path
    *p_gCpuSupportsMMX = false;

    /*
    ddv_func6_decodes_block_q_hook->UnHook();
    auto ret = ddv_func6_decodes_block_q_hook->OrignalFunction()(hack, thisPtr, screenBuffer);
    ddv_func6_decodes_block_q_hook->Hook();
    */

//    ddv_class *pThis3; // eax@1
    ddv_class *pThis1 = 0; // ebp@1
    int(__cdecl *decodeMacroBlockfPtr)(int, int *, int, DWORD, int, int *); // edi@2
    int keyFrameRate = 0; // eax@2
    int decode6_mmx_ret = 0; // ebx@4
    int dataSizeDWords = 0; // ebp@4
    int buffer = 0; // esi@4
   // int v9; // eax@4
    int dataSizeBytes = 0; // ebp@7
//    int decode1_mmx_ret; // ebx@7
//    int v12; // esi@7
//    int decode2_mmx_ret; // ebx@7
//    int v14; // esi@7
//    int decode3_mmx_ret; // ebx@7
//    int v16; // esi@7
//    int decode4_mmx_ret; // ebx@7
//    int v18; // esi@7
//    int decode5_mmx_ret; // ebx@7
 //   int v20; // esi@7
    int block1PtrCopy = 0; // ST30_4@14
    int decode1_ret = 0; // ebx@14
    int block2Ptr = 0; // esi@14
    int block2PtrCopy = 0; // ST18_4@14
    int decode2_ret = 0; // ebx@14
    int block3Ptr = 0; // esi@14
    int decode3_ret = 0; // ebx@14
    int block3PtrCopy = 0; // ST40_4@14
    int block4Ptr = 0; // esi@14
    int block4PtrCopy = 0; // ST28_4@14
    int decode4_ret = 0; // ebx@14
    int block5Ptr = 0; // esi@14
    int block5PtrCopy = 0; // ST10_4@14
    int decode5_ret = 0; // ebx@14
    int block6Ptr = 0; // esi@14
    int v36 = 0; // edx@21
    int numXBlocks = 0; // edx@25
    unsigned __int8 v38 = 0; // sf@25
    unsigned __int8 v39 = 0; // of@25
    unsigned __int8 *pScreenBufferCurrentPos = 0; // [sp+10h] [bp-14h]@5
    ddv_class *pThis2 = 0; // [sp+14h] [bp-10h]@1
    unsigned int blockYCount = 0; // [sp+18h] [bp-Ch]@5
    int dataSizeDWordsCopy = 0; // [sp+1Ch] [bp-8h]@4
    int xBlockCnt = 0; // [sp+20h] [bp-4h]@4

    pThis1 = thisPtr;
    pThis2 = thisPtr;
    if (!(thisPtr->field_61 & 0xFF))
    {
        return thisPtr->field_61;
    }
    keyFrameRate = thisPtr->keyFrameRate;
    ++thisPtr->field_6C;
    decodeMacroBlockfPtr = (int(__cdecl *)(int, int *, int, DWORD, int, int *))ddv_func7_DecodeMacroBlock_ptr;// gending uses this one - this outputs macroblock coefficients?
    if (keyFrameRate <= 1)
    {
        // Should never happen - at least with the test data
        abort();
        //decodeMacroBlockfPtr = (int(__cdecl *)(int, int *, int, _DWORD, int, int *))DecodeMacroBlockReleated_Q;// forcing this to get called resulting in really blocky video frames for keyframes > 1
    }
    buffer = thisPtr->mMacroBlockBuffer_q;
    
    // Done once for the whole 320x240 image
    int v9 = decode_bitstream_q_ptr((WORD*)thisPtr->mRawFrameBitStreamData, (unsigned int*)thisPtr->mDecodedBitStream);
  //  after_block_decode_no_effect_q(v9);           // has no effect if call noped
    decode6_mmx_ret = (int)pThis1->mDecodedBitStream;

    dataSizeDWords = pThis1->mBlockDataSize_q; // 64
    dataSizeDWordsCopy = dataSizeDWords;
    xBlockCnt = 0;

    if (pThis2->nNumMacroblocksX <= 0) // 20
    {
        return (char)pThis2;
    }

    // Now loop over the 16x16 macro blocks that make up the image, so we have 320/16=xblocks (20), 240/16=yblocks (16)
    do
    {
        pScreenBufferCurrentPos = pScreenBuffer;
        blockYCount = 0;
        if (pThis2->nNumMacroblocksY <= 0) // 320/16=15
        {
            goto skip_block_blts; // aka break - wonder why this is within the loop?
        }
        do
        {
            if (gCpuSupportsMMX)
            {
                OutputDebugString("Should not be hitting this code path\n");
            }
            else
            {
                // B1
                block1PtrCopy = buffer;
                decode1_ret = decodeMacroBlockfPtr(decode6_mmx_ret, &gMacroBlock1Buffer, buffer, 0, 0, 0);
                blit_output_no_mmx(dataSizeDWords);     // none ASM version of above - again blank if not called
                
                dataSizeBytes = 4 * dataSizeDWords;
                block2Ptr = dataSizeBytes + buffer;
                block2PtrCopy = block2Ptr;

                // B2
                decode2_ret = decodeMacroBlockfPtr(decode1_ret, &gMacroBlock2Buffer, block2Ptr, 0, block1PtrCopy, &gMacroBlock1Buffer);
                blit_output_no_mmx(dataSizeBytes);
                block3Ptr = dataSizeBytes + block2Ptr;

                // B3
                decode3_ret = decodeMacroBlockfPtr(decode2_ret, &gMacroBlock3Buffer, block3Ptr, 1, block2PtrCopy, &gMacroBlock2Buffer);
                block3PtrCopy = block3Ptr;
                blit_output_no_mmx(dataSizeBytes);
                block4Ptr = dataSizeBytes + block3Ptr;
                block4PtrCopy = block4Ptr;

                // B4
                decode4_ret = decodeMacroBlockfPtr(decode3_ret, &gMacroBlock4Buffer, block4Ptr, 1, block3PtrCopy, &gMacroBlock3Buffer);
                blit_output_no_mmx(dataSizeBytes);
                block5Ptr = dataSizeBytes + block4Ptr;
                block5PtrCopy = block5Ptr;

                // B5
                decode5_ret = decodeMacroBlockfPtr(decode4_ret, &gMacroBlock5Buffer, block5Ptr, 1, block4PtrCopy, &gMacroBlock4Buffer);
                blit_output_no_mmx(dataSizeBytes);
                block6Ptr = dataSizeBytes + block5Ptr;

                // B6
                decode6_mmx_ret = decodeMacroBlockfPtr(decode5_ret, &gMacroBlock6Buffer, block6Ptr, 1, block5PtrCopy, &gMacroBlock5Buffer);
                blit_output_no_mmx(dataSizeBytes);      // a missing macro block/square if not called

                buffer = dataSizeBytes + block6Ptr;

                if (dword_62EFE0 & 1) // Hit on 2x2 dithering
                {
                    calling_conv_hack((int)pScreenBufferCurrentPos);
                    //                    write_block_bit1_no_mmx_ptr(dataSizeBytes);// sets width too small and images look weird
                    goto after_block_write;
                }
                if (dword_62EFE0 & 4)
                {
                    // Not expected for the test video
                    abort();
                    //write_block_bit4_no_mmx(dataSizeBytes);// plays at half height?
                    //goto after_block_write;
                }
                if (dword_62EFE0 & 8) // Hit on "no dithering", 2x1 dithering
                {
                    // Not expected for the test video
                    //write_block_bit8_no_mmx(dataSizeBytes);// seems to be the "normal" case
                    //goto after_block_write;
                }
            }
            // Not expected for the test video
            abort();
            //write_block_other_bits_no_mmx(dataSizeBytes);// half height, every other horizontal block is skipped?
        after_block_write:

            v36 = (int)&pScreenBufferCurrentPos[16 * dword_62EFD8];// dword_62EFD8 is probably the buffer width? 1280 - always 1280?

            pScreenBufferCurrentPos += 16 * dword_62EFD8;

            if (dword_62EFE0 & 8)
            {
                pScreenBufferCurrentPos = (unsigned __int8 *)(16 * dword_62EFD8 + v36);
            }

            dataSizeDWords = dataSizeDWordsCopy;
            ++blockYCount;
        } while (blockYCount < pThis2->nNumMacroblocksY); // 16 lines for the block?

    skip_block_blts:
        pScreenBuffer += dword_62EFD4;              // not sure what this is, height maybe? No scaling = 32, 2x1 = 64, 2x2=64
        numXBlocks = pThis2->nNumMacroblocksX; // 240/16=20
        v39 = __SETO__(xBlockCnt + 1, numXBlocks);
        v38 = xBlockCnt++ + 1 - numXBlocks < 0;
        //} while (v38 ^ v39);
    } while (numXBlocks < pThis2->nNumMacroblocksX);
    //return (char)pThis2;

  //  memset(pScreenBuffer, 0xff, 256 * 390);

    // The app doesn't seem to do anything with the return value
    return 0;
}

JmpHookedFunction<ddv_func7_DecodeMacroBlock>* ddv_func7_DecodeMacroBlock_hook;





void InstallHooks()
{
    ddv_func6_decodes_block_q_hook = new JmpHookedFunction<ddv__func5_block_decoder_q_type>(real_ddv__func5_block_decoder_q, &ddv__func5_block_decoder_q);
}
