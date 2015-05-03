#pragma once

#include <windows.h>

// functions:
// int __usercall ddv_func7_DecodeMacroBlock<eax>(int thisPtr<ebp>)
#pragma pack(push)
#pragma pack(1)
struct DecodeMacroBlock_Struct
{
    DWORD field_0;
    DWORD field_4;
    DWORD mOutput;
    DWORD field_C;
    DWORD mCoEffsBuffer;
    unsigned char ZeroOrOneConstant;
    unsigned char field_15;
};
#pragma pack(pop)

static_assert(sizeof(DecodeMacroBlock_Struct) == 0x16, "Wrong size");


typedef int(__cdecl *ddv_func7_DecodeMacroBlock)(DecodeMacroBlock_Struct* params);
static ddv_func7_DecodeMacroBlock ddv_func7_DecodeMacroBlock_ptr = (ddv_func7_DecodeMacroBlock)0x0040E6B0;

// Only for DDV's with a low number of key frames
// DecodeMacroBlockReleated_Q
// 0040E870, __usercall

typedef int(__cdecl *decode_bitstream_q)(WORD *pFrameData, unsigned int *pOutput);
static decode_bitstream_q decode_bitstream_q_ptr = (decode_bitstream_q)0x00407B20;

typedef signed int(__cdecl* after_block_decode_no_effect_q)(int a1);
static after_block_decode_no_effect_q after_block_decode_no_effect_q_ptr = (after_block_decode_no_effect_q)0x0040E360;


typedef int(__cdecl* write_block_bit1_no_mmx)(int a1);
static write_block_bit1_no_mmx write_block_bit1_no_mmx_ptr = (write_block_bit1_no_mmx)0x0040BB30;

// write_block_bit4_no_mmx
// 0040AB50 __usercall

// write_block_bit8_no_mmx
// 0040A2C0 __usercall

// write_block_other_bits_no_mmx
// 0040B330 __usercall


// vars:
static char* p_gCpuSupportsMMX = (char*)0x0062EFDC;
static char& gCpuSupportsMMX = *p_gCpuSupportsMMX;

// TODO: Probably structs as there are 7 other DWORDS related after each one
// of these
static int* p_gMacroBlock1Buffer = (int*)0x006313E8;
static int& gMacroBlock1Buffer = *p_gMacroBlock1Buffer;

static int* p_gMacroBlock2Buffer = (int*)0x006314E8;
static int& gMacroBlock2Buffer = *p_gMacroBlock2Buffer;

static int* p_gMacroBlock3Buffer = (int*)0x00630FE8;
static int& gMacroBlock3Buffer = *p_gMacroBlock3Buffer;

static int* p_gMacroBlock4Buffer = (int*)0x006311E8;
static int& gMacroBlock4Buffer = *p_gMacroBlock4Buffer;

static int* p_gMacroBlock5Buffer = (int*)0x006310E8;
static int& gMacroBlock5Buffer = *p_gMacroBlock5Buffer;

static int* p_gMacroBlock6Buffer = (int*)0x006312E8;
static int& gMacroBlock6Buffer = *p_gMacroBlock6Buffer;

static DWORD* p_dword_62EFE0 = (DWORD*)0x0062EFE0;
static DWORD& dword_62EFE0 = *p_dword_62EFE0;

static DWORD* p_dword_62EFD8 = (DWORD*)0x0062EFD8;
static DWORD& dword_62EFD8 = *p_dword_62EFD8;

static DWORD* p_dword_62EFD4 = (DWORD*)0x0062EFD4;
static DWORD& dword_62EFD4 = *p_dword_62EFD4;

void InstallHooks();
