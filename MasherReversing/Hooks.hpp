#pragma once

#include <windows.h>

// functions:
// ddv_func7_DecodeMacroBlock
// 0040E6B0 __usercall

// DecodeMacroBlockReleated_Q
// 0040E870, __usercall

// int __cdecl decode_bitstream_q(_WORD *pFrameData, unsigned int *pOutput)
// 00407B20

// signed int __cdecl after_block_decode_no_effect_q(int a1)
// 0040E360

// write_block_bit1_no_mmx
// 0040BB30 __usercall

// write_block_bit4_no_mmx
// 0040AB50 __usercall

// write_block_bit8_no_mmx
// 0040A2C0 __usercall

// write_block_other_bits_no_mmx
// 0040B330 __usercall


// vars:
static char* p_gCpuSupportsMMX = (char*)0x0062EFDC;

// TODO: Probably structs as there are 7 other DWORDS related after each one
// of these
static int* p_gMacroBlock1Buffer = (int*)0x006313E8;
static int* p_gMacroBlock2Buffer = (int*)0x006314E8;
static int* p_gMacroBlock3Buffer = (int*)0x00630FE8;
static int* p_gMacroBlock4Buffer = (int*)0x006311E8;
static int* p_gMacroBlock5Buffer = (int*)0x006310E8;
static int* p_gMacroBlock6Buffer = (int*)0x006312E8;

static DWORD* p_dword_62EFE0 = (DWORD*)0x0062EFE0;
static DWORD* p_dword_62EFD8 = (DWORD*)0x0062EFD8;
static DWORD* p_dword_62EFD4 = (DWORD*)0x0062EFD4;

void InstallHooks();
