/*
	Copyright (C) 2012-2020

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "R5900_Recompiler.h"
#include "ps2_system.h"
#include "R5900_Print.h"
#include "PS2DataBus.h"

using namespace Playstation2;
using namespace R5900;


#define ENABLE_MULTIPLY_LATENCY
#define ENABLE_DIVIDE_LATENCY


#define USE_NEW_STORE_CODE
#define USE_NEW_LOAD_CODE

#define USE_NEW_ADD_CODE
#define USE_NEW_ADDI_CODE
#define USE_NEW_SUB_CODE

#define USE_NEW_SYSCALL_CODE

#define USE_NEW_BEQ_CODE
#define USE_NEW_BNE_CODE
#define USE_NEW_BLTZ_CODE
#define USE_NEW_BGTZ_CODE
#define USE_NEW_BLEZ_CODE
#define USE_NEW_BGEZ_CODE

#define USE_NEW_BLTZAL_CODE
#define USE_NEW_BGEZAL_CODE

#define USE_NEW_J_CODE
#define USE_NEW_JR_CODE
#define USE_NEW_JAL_CODE
#define USE_NEW_JALR_CODE

#define ALLOW_ENCODING_DELAYSLOT
#define ENCODE_ALL_POSSIBLE_DELAYSLOTS

#define USE_NEW_DADD_CODE
#define USE_NEW_DADDI_CODE
#define USE_NEW_DSUB_CODE

#define USE_NEW_DADDU_CODE
#define USE_NEW_DADDIU_CODE
#define USE_NEW_DSUBU_CODE


#define USE_NEW_MFHI_CODE
#define USE_NEW_MFLO_CODE

#define USE_NEW_MULT_CODE
#define USE_NEW_MULTU_CODE
#define USE_NEW_DIV_CODE
#define USE_NEW_DIVU_CODE

#define USE_NEW_MULT1_CODE
#define USE_NEW_MULTU1_CODE
#define USE_NEW_DIV1_CODE
#define USE_NEW_DIVU1_CODE

#define USE_NEW_MFHI1_CODE
#define USE_NEW_MFLO1_CODE

#define USE_NEW_MADD_CODE
#define USE_NEW_MADD1_CODE
#define USE_NEW_MADDU_CODE
#define USE_NEW_MADDU1_CODE


#define USE_NEW_BEQL_CODE
#define USE_NEW_BNEL_CODE
#define USE_NEW_BLTZL_CODE
#define USE_NEW_BGTZL_CODE
#define USE_NEW_BLEZL_CODE
#define USE_NEW_BGEZL_CODE

#define USE_NEW_BLTZALL_CODE
#define USE_NEW_BGEZALL_CODE


#define USE_NEW_PAND_CODE
#define USE_NEW_POR_CODE
#define USE_NEW_PXOR_CODE
#define USE_NEW_PNOR_CODE

#define USE_NEW_PCEQB_CODE
#define USE_NEW_PCEQH_CODE
#define USE_NEW_PCEQW_CODE

#define USE_NEW_PCGTB_CODE
#define USE_NEW_PCGTH_CODE
#define USE_NEW_PCGTW_CODE

#define USE_NEW_PMINH_CODE
#define USE_NEW_PMINW_CODE
#define USE_NEW_PMAXH_CODE
#define USE_NEW_PMAXW_CODE

#define USE_NEW_PADDB_CODE
#define USE_NEW_PADDH_CODE
#define USE_NEW_PADDW_CODE

#define USE_NEW_PSUBB_CODE
#define USE_NEW_PSUBH_CODE
#define USE_NEW_PSUBW_CODE

#define USE_NEW_PABSH_CODE
#define USE_NEW_PABSW_CODE

#define USE_NEW_PADDSB_CODE
#define USE_NEW_PADDSH_CODE

#define USE_NEW_PADDUB_CODE
#define USE_NEW_PADDUH_CODE

#define USE_NEW_PSUBSB_CODE
#define USE_NEW_PSUBSH_CODE

#define USE_NEW_PSUBUB_CODE
#define USE_NEW_PSUBUH_CODE

#define USE_NEW_PSLLH_CODE
#define USE_NEW_PSLLW_CODE
#define USE_NEW_PSRAH_CODE
#define USE_NEW_PSRAW_CODE
#define USE_NEW_PSRLH_CODE
#define USE_NEW_PSRLW_CODE

#define USE_NEW_PSLLVW_CODE
#define USE_NEW_PSRAVW_CODE
#define USE_NEW_PSRLVW_CODE

#define USE_NEW_PEXTLB_CODE
#define USE_NEW_PEXTLH_CODE
#define USE_NEW_PEXTLW_CODE
#define USE_NEW_PEXTUB_CODE
#define USE_NEW_PEXTUH_CODE
#define USE_NEW_PEXTUW_CODE
#define USE_NEW_PINTH_CODE
#define USE_NEW_PINTEH_CODE

#define USE_NEW_PEXCH_CODE
#define USE_NEW_PEXCW_CODE
#define USE_NEW_PEXEH_CODE
#define USE_NEW_PEXEW_CODE

#define USE_NEW_PCPYLD_CODE
#define USE_NEW_PCPYUD_CODE
#define USE_NEW_PCPYH_CODE

#define USE_NEW_PPACB_CODE
#define USE_NEW_PPACH_CODE
#define USE_NEW_PPACW_CODE

#define USE_NEW_QFSRV_CODE

#define USE_NEW_PADSBH_CODE
#define USE_NEW_PLZCW_CODE

#define USE_NEW_PMULTH_CODE
#define USE_NEW_PMULTW_CODE
#define USE_NEW_PMULTUW_CODE

#define USE_NEW_PDIVBW_CODE
#define USE_NEW_PDIVW_CODE
#define USE_NEW_PDIVUW_CODE

#define USE_NEW_PMADDH_CODE
#define USE_NEW_PMADDW_CODE
#define USE_NEW_PMADDUW_CODE
#define USE_NEW_PMSUBH_CODE
#define USE_NEW_PMSUBW_CODE

#define USE_NEW_PHMADH_CODE
#define USE_NEW_PHMSBH_CODE

#define USE_NEW_PMFLO_CODE
#define USE_NEW_PMFHI_CODE


#define USE_NEW_PMTLO_CODE
#define USE_NEW_PMTHI_CODE

#define USE_NEW_PMFHL_LH_CODE
#define USE_NEW_PMFHL_LW_CODE
#define USE_NEW_PMFHL_SH_CODE
#define USE_NEW_PMFHL_SLW_CODE
#define USE_NEW_PMFHL_UW_CODE

#define USE_NEW_PMTHL_LW_CODE

#define USE_NEW_PEXT5_CODE
#define USE_NEW_PPAC5_CODE


/*

#define CHECK_EVENT_AFTER_START
*/

//#define ENABLE_SINGLE_STEP



#define CACHE_NOT_IMPLEMENTED


// test pc arg pass, new methodology etc
//#define TEST_NEW_CODE


// check that instructions in cached-region were not modified since last recompile
//#define CHECK_CACHED_INSTRUCTIONS


//#define USE_MEMORYPTR_FOR_CACHED_REGION


//#define USE_GETPTR_FOR_CACHED_REGION


// theoretically, anything in BIOS is read-only
//#define DONT_CHECK_BIOS_INSTRUCTIONS



//#define INCLUDE_ICACHE_RELOAD




//#define ALWAYS_USE_MEMORYPTR_FOR_ENCODING


#define ENCODE_SINGLE_RUN_PER_BLOCK


#define UPDATE_BEFORE_RETURN


// crashes unless you do this ?? Compiler dependent?
#define RESERVE_STACK_FRAME_FOR_CALL


//#define ENABLE_AUTONOMOUS_BRANCH_U
//#define ENABLE_AUTONOMOUS_BRANCH_C


//#define VERBOSE_RECOMPILE


x64Encoder *Recompiler::e;
//ICache_Device *Recompiler::ICache;
R5900::Cpu *R5900::Recompiler::r;
s32 R5900::Recompiler::OpLevel;
u32 R5900::Recompiler::LocalPC;
u32 R5900::Recompiler::Local_LastModifiedReg;
u32 R5900::Recompiler::Local_NextPCModified;

u32 R5900::Recompiler::CurrentCount;

u32 R5900::Recompiler::isBranchDelaySlot;
u32 R5900::Recompiler::isLoadDelaySlot;

u32 R5900::Recompiler::bStopEncodingAfter;
u32 R5900::Recompiler::bStopEncodingBefore;


//u32 Recompiler::Local_DelaySlot;
//u32 Recompiler::Local_DelayType;
//u32 Recompiler::Local_DelayCount;
//u32 Recompiler::Local_DelayCond;
//u32 Recompiler::Local_Condition;
R5900::Instruction::Format Recompiler::NextInst;

//Recompiler::RDelaySlot Recompiler::RDelaySlots [ 2 ];
//u32 Recompiler::DSIndex;
//u32 Recompiler::RDelaySlots_Valid;

static u64 Recompiler::MemCycles;

static u64 Recompiler::LocalCycleCount;
static u64 Recompiler::CacheBlock_CycleCount;

static bool Recompiler::bIsBlockInICache;

static u32 Recompiler::bResetCycleCount;


static u32 Recompiler::CurrentBlock_StartAddress;
static u32 Recompiler::NextBlock_StartAddress;

static u32* Recompiler::pForwardBranchTargets;
static u32 Recompiler::ForwardBranchIndex;

static u8** Recompiler::pPrefix_CodeStart;
static u8** Recompiler::pCodeStart;
static u32* Recompiler::CycleCount;

static u32 Recompiler::ulIndex_Mask;
static u32 Recompiler::MaxStep;
static u32 Recompiler::MaxStep_Shift;
static u32 Recompiler::MaxStep_Mask;

static u32 Recompiler::StartBlockIndex;
static u32 Recompiler::BlockIndex;


static const u8 recompiler_qfsrv_shift_table [ 16 * 16 ] __attribute__ ((aligned (16))) = {
	0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0,
	0x0, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1,
	0x1, 0x0, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2,
	0x2, 0x1, 0x0, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3,
	0x3, 0x2, 0x1, 0x0, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4,
	0x4, 0x3, 0x2, 0x1, 0x0, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5,
	0x5, 0x4, 0x3, 0x2, 0x1, 0x0, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6,
	0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7,
	0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8,
	0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9,
	0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa,
	0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0, 0xf, 0xe, 0xd, 0xc, 0xb,
	0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0, 0xf, 0xe, 0xd, 0xc,
	0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0, 0xf, 0xe, 0xd,
	0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0, 0xf, 0xe,
	0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0, 0xf
};

// with the right as dest and left as src
static const u8 recompiler_qfsrv_blend_table [ 16 * 16 ] __attribute__ ((aligned (16))) = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00
};


static const u8 recompiler_qfsrv_shift_table_rev [ 16 * 16 ] __attribute__ ((aligned (16))) = {
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
	0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x0,
	0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x0, 0x1,
	0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x0, 0x1, 0x2,
	0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x0, 0x1, 0x2, 0x3,
	0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x0, 0x1, 0x2, 0x3, 0x4,
	0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5,
	0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6,
	0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
	0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
	0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9,
	0xb, 0xc, 0xd, 0xe, 0xf, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa,
	0xc, 0xd, 0xe, 0xf, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb,
	0xd, 0xe, 0xf, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc,
	0xe, 0xf, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd,
	0xf, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe
};

// with the right as dest and left as src
static const u8 recompiler_qfsrv_blend_table_rev [ 16 * 16 ] __attribute__ ((aligned (16))) = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
};


// constructor
// NumberOfBlocks MUST be a power of 2, so 1 would mean 2, 2 would mean 4
Recompiler::Recompiler ( Cpu* R5900Cpu, u32 NumberOfBlocks, u32 BlockSize_PowerOfTwo, u32 MaxIStep_Shift )
{
	
	BlockSize = 1 << BlockSize_PowerOfTwo;
	
	MaxStep_Shift = MaxIStep_Shift;
	MaxStep = 1 << MaxIStep_Shift;
	MaxStep_Mask = MaxStep - 1;
	
	NumBlocks = 1 << NumberOfBlocks;
	NumBlocks_Mask = NumBlocks - 1;
	
	// need a mask for referencing each encoded instruction
	ulIndex_Mask = 1 << ( NumberOfBlocks + MaxIStep_Shift );
	ulIndex_Mask -= 1;
	
	// allocate variables
	//StartAddress = new u32 [ NumBlocks ];
	//RunCount = new u8 [ NumBlocks ];
	//MaxCycles = new u64 [ NumBlocks ];
	//Instructions = new u32 [ NumBlocks * MaxStep ];
	
	// only need to compare the starting address of the entire block
	StartAddress = new u32 [ NumBlocks ];
	pCodeStart = new u8* [ NumBlocks * MaxStep ];
	CycleCount = new u32 [ NumBlocks * MaxStep ];
	//EndAddress = new u32 [ NumBlocks * MaxStep ];
	
	pForwardBranchTargets = new u32 [ MaxStep ];
	
	// used internally by recompiler (in case it branches to a load/store or another branch, etc, then need to go to prefix instead)
	pPrefix_CodeStart = new u8* [ MaxStep ];
	
	
	// create the encoder
	//e = new x64Encoder ( BlockSize_PowerOfTwo, NumBlocks );
	InstanceEncoder = new x64Encoder ( BlockSize_PowerOfTwo, NumBlocks );
	
	e = InstanceEncoder;
	
	/*
	// set the "alternate stream" with the code to clear a block
	// which should just simply return 1 (meaning to recompile the block)
	e->SwitchToAlternateStream ();
	e->MovReg32ImmX ( RAX, 1 );
	e->Ret ();
	
	// we're done in the "alternate stream"
	e->SwitchToLiveStream ();
	
	// I'd like to know the size of the code
	cout << "\nSize of alternate stream in bytes: " << dec << e->lAlternateStreamSize << " Alt Stream=" << hex << e->ullAlternateStream;
	
	// reset all the blocks
	//for ( int i = 0; i < NumBlocks; i++ ) InitBlock ( i );
	for ( int i = 0; i < NumBlocks; i++ ) e->Emit_AltStreamToBlock8 ( i );
	*/
	
	//cout << "\nAfter clear, live code stream=" << hex << (((u64*)e->LiveCodeArea) [ 0 ]);
	//cout << "\n#2=" << hex << ((u64*)e->LiveCodeArea) [ ( ( (NumBlocks-1) & e->lCodeBlockSize_Mask ) << e->lCodeBlockSize_PowerOfTwo ) >> 3 ];
	//cout << " offset=" << hex << ( ( ( 2 & e->lCodeBlockSize_Mask ) << e->lCodeBlockSize_PowerOfTwo ) >> 3 );
	
	// testing
	//pCodeStart [ 0x27e4 >> 2 ] = 0x5373b36;
	
	//ICache = IC;
	r = R5900Cpu;
	
	Reset ();
}


// destructor
Recompiler::~Recompiler ()
{
	delete e;
	
	delete StartAddress;
	
	delete pPrefix_CodeStart;
	delete pCodeStart;
	delete CycleCount;
	delete pForwardBranchTargets;
	
	// delete the variables that were allocated
	/*
	delete StartAddress;
	delete RunCount;
	delete MaxCycles;
	delete Instructions;
	*/
}


void Recompiler::Reset ()
{
	//memset ( this, 0, sizeof( Recompiler ) );	
	// initialize the address and instruction so it is known that it does not refer to anything
	memset ( pForwardBranchTargets, 0x00, sizeof( u32 ) * MaxStep );
	memset ( pPrefix_CodeStart, 0x00, sizeof( u8* ) * MaxStep );
	memset ( StartAddress, 0xff, sizeof( u32 ) * NumBlocks );
	memset ( pCodeStart, 0x00, sizeof( u8* ) * NumBlocks * MaxStep );
	memset ( CycleCount, 0x00, sizeof( u32 ) * NumBlocks * MaxStep );
	
#ifdef ENABLE_ICACHE
	// reset invalidate arrays
	r->Bus->Reset_Invalidate ();
#endif
}

// returns 1 if it is ok to have instruction in branch delay slot when recompiling, zero otherwise
static bool Recompiler::isBranchDelayOk ( u32 ulInstruction, u32 Address )
{
#ifdef ENCODE_ALL_POSSIBLE_DELAYSLOTS

	u32 ulOpcode, ulFunction;
	
	ulOpcode = ulInstruction >> 26;
	
	
	// second row starting at second column is ok
	if ( ulOpcode >= 0x9 && ulOpcode <= 0xf )
	{

		// constant instructions that will never interrupt //
		//cout << "\nAddress=" << hex << Address << " Opcode=" << ulOpcode;
		return true;
	}
	
	
	
	// for R5900, DADDIU is ok //
	if ( ulOpcode == 0x19 )
	{
		return true;
	}
	
	
	
	// check special instructions
	if ( !ulOpcode )
	{
		ulFunction = ulInstruction & 0x3f;
		
		// first row is mostly ok
		if ( ( ( ulFunction >> 3 ) == 0 ) && ( ulFunction != 1 ) && ( ulFunction != 5 ) )
		{
			return true;
		}
		
		// row 4 is ok except for column 0 and column 2
		if ( ( ulFunction >> 3 ) == 4 && ulFunction != 0x20 && ulFunction != 0x22 )
		{
			return true;
		}
		
		// in row 5 for R3000A columns 2 and 3 are ok
		if ( ulFunction == 0x2a || ulFunction == 0x2b )
		{
			return true;
		}
		
		
		// in row 5 for R5900 columns 5 and 7 are ok //
		if ( ulFunction == 0x2d || ulFunction == 0x2f )
		{
			return true;
		}
		
		
		// for R5900, on row 2 MOVZ and MOVN are ok //
		if ( ulFunction == 0xa || ulFunction == 0xb )
		{
			return true;
		}
		
		
		// for R5900, on row 3 DSLLV, DSRLV, DSRAV or ok //
		if ( ulFunction == 0x14 || ulFunction == 0x16 || ulFunction == 0x17 )
		{
			return true;
		}
		
		// in last row for R5900, all is ok except columns 1 and 5 //
		if ( ( ulFunction >> 3 ) == 7 && ulFunction != 0x39 && ulFunction != 0x3d )
		{
			return true;
		}
		
	}
	
	// will leave out all store instructions for now to play it safe //
	
#else
	if ( !ulInstruction ) return true;
#endif
	
	return false;
}


u32 Recompiler::CloseOpLevel ( u32 OptLevel, u32 Address )
{
	switch  ( OptLevel )
	{
		case 0:
			break;
			
		case 1:
			// write back last modified register if in load delay slot
			//if ( isLoadDelaySlot )
			//{
				e->MovMemImm32 ( &r->LastModifiedRegister, Local_LastModifiedReg );
			//}
			
			// write back "NextPC" if there was no SYSCALL
			if ( !Local_NextPCModified )
			{
				e->MovMemImm32 ( &r->NextPC, Address );
			}
			break;
			
		case 2:
			// write back last modified register if in load delay slot
			//if ( isLoadDelaySlot )
			//{
				e->MovMemImm32 ( &r->LastModifiedRegister, Local_LastModifiedReg );
			//}
			
			// write back "NextPC" if there was no SYSCALL
			if ( !Local_NextPCModified )
			{
				e->MovMemImm32 ( &r->NextPC, Address );
			}
			break;
	}
}


// if block has no recompiled code, then it should return 1 (meaning to recompile the instruction(s))
u32 Recompiler::InitBlock ( u32 Block )
{
	// set the encoder to use
	e = InstanceEncoder;

	// start encoding in block
	e->StartCodeBlock ( Block );
	
	// set return value 1 (return value for X64 goes in register A)
	e->LoadImm32 ( RAX, 1 );
	
	// return
	e->x64EncodeReturn ();
	
	// done encoding in block
	e->EndCodeBlock ();
}



u32* Recompiler::RGetPointer ( u32 Address )
{

#ifdef ENABLE_ICACHE
	if ( ICache_Device::isCached ( Address ) )
	{
		// address is cached //
		
		// check if cache line has address
		if ( r->ICache.ICacheBlockSource [ ( Address >> 4 ) & 0xff ] == ( Address & 0x1ffffff0 ) )
		{
#ifdef VERBOSE_RECOMPILE
cout << "\nRGetPointer: CACHED";
#endif

			// address is already cached, so get pointer from cache
			return & r->ICache.ICacheData [ ( Address >> 2 ) & 0x3ff ];
		}
	}
#endif
	
#ifdef VERBOSE_RECOMPILE
cout << "\nRGetPointer: NON-CACHED";
#endif

	// address is NOT cache-able //
	
	if ( ( Address & 0x1fc00000 ) == 0x1fc00000 )
	{
		// cached bios region //
		return & r->Bus->BIOS.b32 [ ( Address & r->Bus->BIOS_Mask ) >> 2 ];
	}
	
	// cached ram region //
	return & r->Bus->MainMemory.b32 [ ( Address & r->Bus->MainMemory_Mask ) >> 2 ];
}


// returns the bitmap for the source registers for instruction
// if the instruction is not supported, then it will return -1ULL
static u64 Recompiler::GetSourceRegs ( R5900::Instruction::Format i, u32 Address )
{
	/*
	if ( !i.Value )
	{
		return 0;
	}
	*/
	
	// "special"
	if ( !i.Opcode )
	{
		return ( ( 1ULL << i.Rs ) | ( 1ULL << i.Rt ) );
	}
	
	// regimm
	if ( i.Opcode == 1 )
	{
		// rs is source reg //
		
		return ( 1ULL << i.Rs );
	}
	
	// j, jal
	if ( i.Opcode <= 3 )
	{
		return 0;
	}
	
	// beq, bne, blez, bgtz
	if ( ( i.Opcode >> 3 ) == 0 )
	{
		return ( ( 1ULL << i.Rs ) | ( 1ULL << i.Rt ) );
	}
	
	// immediates
	if ( ( i.Opcode >> 3 ) == 1 )
	{
		return ( 1ULL << i.Rs );
	}
	
	// stores
	if ( ( i.Opcode >> 3 ) == 5 )
	{
		return 0;
	}
	
	// loads
	if ( ( i.Opcode >> 3 ) == 4 )
	{
		return ( 1ULL << i.Rs );
	}
	
	return -1ULL;
	
	/*
	// check for "special"
	if ( !i.Opcode )
	{
		// rs,rt are source regs //
		
		// not including syscall or break, but these shouldn't cause problems
		if ( ( i.Funct == 12 ) || ( i.Funct == 13 ) )
		{
			return 0;
		}
		
		//if ( ( i.Funct >> 3 ) == 0 )
		//{
		//	return -1;
		//}
		
		return ( ( 1ULL << i.Rs ) | ( 1ULL << i.Rt ) );
	}
	
	// stores are cleared to go (runs load delay before the store)
	if ( ( i.Opcode >> 3 ) == 5 )
	{
		return 0;
	}
	
	// check for regimm, immediates, loads
	if ( ( i.Opcode == 1 ) || ( ( i.Opcode >> 3 ) == 1 ) || ( ( i.Opcode >> 3 ) == 4 ) )
	{
		// rs is source reg //
		
		return ( 1ULL << i.Rs );
	}
	
	// any other instructions are not cleared to go
	return -1ULL;
	*/
}


// returns the bitmap for the destination registers for instruction
// if the instruction is not supported, then it will return -1ULL
static u64 Recompiler::Get_DelaySlot_DestRegs ( R5900::Instruction::Format i )
{
	
	/*
	// check for "special"
	if ( !i.Opcode )
	{
		// rd is dest reg //
		
		// not including syscall or break, but these shouldn't cause problems
		if ( ( i.Funct == 12 ) || ( i.Funct == 13 ) )
		{
			return 0;
		}
		
		
		return ( 1ULL << i.Rd );
	}
	
	// check for regimm
	if ( ( i.Opcode == 1 ) )
	{
		// rd is dest reg //
		
		if ( i.Rt >= 16 )
		{
			return ( 1ULL << 31 );
		}
	}
	
	// check for jal
	if ( i.Opcode == 3 )
	{
		return ( 1ULL << 31 );
	}
	
	// immediates
	if ( ( i.Opcode >> 3 ) == 1 )
	{
		return ( 1ULL << i.Rt );
	}
	*/
	
	// loads
	/*
	if ( ( i.Opcode >> 3 ) == 4 )
	{
		// rt is dest reg //
		
		return ( 1ULL << i.Rt );
	}
	*/
	
	
	
	// any other instructions are not cleared to go
	return 0;
}


/*
static u64 Recompiler::ReturnZero ( void )
{
	return 0;
}


static u64 Recompiler::ReturnOne ( void )
{
	return 1;
}


static u64 Recompiler::ReturnTwo ( void )
{
	return 2;
}
*/




// returns number of instructions that were recompiled
u32 Recompiler::Recompile ( u32 BeginAddress )
{
	u32 Address, Block;
	s32 ret, Cycles;
	R5900::Instruction::Format inst;
	s32 reti;
	
	//u32 StartBlockIndex, BlockIndex, SaveBlockIndex;
	
	// number of instructions in current run
	u32 RunCount;
	
	u32 RecompileCount;
	u32 MaxCount;
	
	//u32 ProjectedMaxCount;
	
	//u32 Snapshot_Address [ 4 ];
	//u32 Snapshot_RecompileCount [ 4 ];
	
	//static u64 MemCycles;
	u32 SetCycles;
	
	//u32* pInstrPtr;
	
	u32* pSrcCodePtr;
	u32* pNextCode;
	
	//u32* pCmpCodePtr;
	//u32* pSaveCodePtr;
	//u32* pSaveCmpPtr;
	
	u32 SaveReg0;
	u32 ulCacheLineCount;
	
	//u64 LocalCycleCount, CacheBlock_CycleCount;
	
	int RetJumpCounter;
	
	//char* ReturnFromCacheReload;
	
	int i;
	
	unsigned long First_LastModifiedReg;
	
	s32 MaxBlocks;
	
	u32 NextAddress;
	
#ifdef VERBOSE_RECOMPILE
cout << "\nrecompile: starting recompile.";
#endif


	// need to first clear forward branch targets for the block
	memset ( pForwardBranchTargets, 0x00, sizeof( u32 ) * MaxStep );
	
	// initialize forward branch index
	// note: Will need a larger branch index table in the encoder object for larger code blocks than 128 instructions
	ForwardBranchIndex = c_ulForwardBranchIndex_Start;


	// mask address
	// don't do this
	//StartAddress &= c_iAddress_Mask;
	
	// haven't crossed any cache lines yet
	ulCacheLineCount = 0;
	
	// set the encoder to use
	e = InstanceEncoder;
	
	// the starting address needs to be on a block boundary
	BeginAddress = ( BeginAddress >> ( 2 + MaxStep_Shift ) ) << ( 2 + MaxStep_Shift );
	
	// save the address?
	Address = BeginAddress;
	
	// set the start address for the current block so recompiler can access it
	CurrentBlock_StartAddress = BeginAddress;
	
	// set the start address for the next block also
	NextBlock_StartAddress = CurrentBlock_StartAddress + ( 1 << ( 2 + MaxStep_Shift ) );
	
	// set the current optimization level
	OpLevel = OptimizeLevel;
	
	// get the block to encode in
	// new formula
	//Block = ( BeginAddress >> 2 ) & NumBlocks_Mask;
	Block = ( BeginAddress >> ( 2 + MaxStep_Shift ) ) & NumBlocks_Mask;
	
	
	// set block initially to cache
	//DoNotCache [ Block ] = 0;
	
	// start in code block
	e->StartCodeBlock ( Block );
	
	// set the start address for code block
	// address must actually match exactly. No mask
	StartAddress [ Block ] = BeginAddress;
	
	// set the instruction
	//Instructions [ Block ] = *((u32*) SrcCode);
	//pInstrPtr = & ( Instructions [ Block << MaxStep_Shift ] );
	
	
	// start cycles at zero
	Cycles = 0;
	
	// start PC
	//LocalPC = r->PC;
	
	
	// init count of recompiled instructions
	RecompileCount = 0;
	
	
	// want to stop at cache boundaries (would need extra code there anyways)
	// this is handled in loop now
	//MaxCount = MaxStep - ( ( Address >> 2 ) & MaxStep_Mask );
	//if ( MaxCount <= 0 ) MaxCount = 1;
	// set the maximum number of instructions to encode
	MaxCount = MaxStep;
	
	
	// NextPC has not been modified yet
	Local_NextPCModified = false;
	
	// some instructions need to stop encoding either before or after the instruction, at least for now
	// if stopping before, it keeps the instruction if there is nothing before it in the run
	bStopEncodingAfter = false;
	bStopEncodingBefore = false;
	
	// don't reset the cycle count yet
	bResetCycleCount = false;


	
	// should set local last modified register to 255
	Local_LastModifiedReg = 255;
	
	reti = 1;
	
	
	

	// clear delay slot
	//RDelaySlots [ 0 ].Value = 0;
	//RDelaySlots [ 1 ].Value = 0;

	// clear delay slot valid bits
	//RDelaySlots_Valid = 0;
	

	
	/////////////////////////////////////////////////////
	// note: multiply and divide require cycle count to be updated first
	// since they take more than one cycle to complete
	// same for mfhi and mflo, because they are interlocked
	// same for COP2 instructions
	// same for load and store
	// do the same for jumps and branches
	//////////////////////////////////////////////////////

	
	
	// get the starting block to store instruction addresses and cycle counts
	StartBlockIndex = ( Address >> 2 ) & ulIndex_Mask;
	BlockIndex = StartBlockIndex;

	// instruction count for current run
	RunCount = 0;
	
	// current delay slot index
	//DSIndex = 0;
	
	// each instruction takes at least one cycle
	//if ( !MemCycles ) MemCycles = 1;
	
	
	
	// this should get pointer to the instruction
	pSrcCodePtr = RGetPointer ( Address );
	
	// get the cycles per instruction
	// *** TODO FOR PS2 *** 
	/*
	if ( ICache_Device::isCached ( Address ) )
	{
		// address is cached //
		bIsBlockInICache = true;
		
		// one cycle to execute each instruction (unless reloading cache block)
		MemCycles = 1;
	}
	else
	{
		// address is NOT cache-able //
		bIsBlockInICache = false;
		
		// time to execute the instruction starts with the time to read it from memory
		if ( ( Address & 0x1fc00000 ) == 0x1fc00000 )
		{
			// bios region //
			MemCycles = Playstation2::DataBus::c_iBIOS_Read_Latency;
		}
		else
		{
			// ram region //
			MemCycles = Playstation2::DataBus::c_iRAM_Read_Latency;
		}
		
		// should be plus 1 like in the interpreter
		MemCycles += 1;
	}
	*/
	
	// *** todo ***
	// looks like cycles are set to 1 in interpreter, need to fix
	MemCycles = 1;


	// need to keep track of cycles for run
	//LocalCycleCount = MemCycles - 1;
	//CacheBlock_CycleCount = 0;
	LocalCycleCount = 0;

	// need to know of any other jumps to return
	RetJumpCounter = 0;
	
	
	
#ifdef VERBOSE_RECOMPILE
cout << "\nRecompiler: Starting loop";
#endif

	// for loads
	// 1. check that there are no events. If so, update Cycles,NextPC, then return
	// 2. check for synchronous interrupt
	// 3. check that there are no conflicts. If so, put load in delay slot, update Cycles,NextPC, then return
	// 4. encode load, then encode load delay slot
	// 5. if going across cache line and next line is not loaded, then put load in delay slot and return
	// 6. if it is a store in the delay slot, then can just process normally as if there is no delay slot and immediately load
	
	// for stores
	// 1. check that there are no events. If so, update Cycles,NextPC, then return
	// 2. check for synchronous interrupt
	// 3. encode store
	
	// for jumps/branches
	// 1. check that there are no events. If so, update Cycles,NextPC, then return
	// 2. check for synchronous interrupt (for jumps that might have them)
	// 3. check that there are no loads,stores,branches,delay slots, in the delay slot. If so, put branch/jump in delay slot, update Cycles,NextPC, then return
	// 4. encode jump/branch then encode delay slot
	// 5. if branching backwards within same block, if cached then make sure cache-block is loaded and then jump, implement forward jumps later?
	// 6. if not branching within same block or forward jumping before implementation, then update Cycles,NextPC, then return
	// 7. if going across cache blocks and next block not loaded, then put in delay slot and return
	
	// other delay slot instructions
	// 1. check that there are no conflicts with delay slot. If so, update Cycles,NextPC, then return
	// 2. encode instruction then encode delay slot
	// 3. if going across cache blocks and next block not loaded, then put in delay slot and return
	
	// finding source registers
	// special instructions can use rs,rt as source registers
	// stores use rs,rt as source registers
	// immediates and loads use only rs as source register

	//for ( int i = 0; i < MaxStep; i++, Address += 4 )
	for ( i = 0; i < MaxCount; i++ )
	{
#ifdef VERBOSE_RECOMPILE
cout << "\nRecompiling: ADDR=" << hex << Address;
#endif
		
		// start encoding a MIPS instruction
		e->StartInstructionBlock ();
		


#ifdef ENABLE_ICACHE
		// check if we are in a new icache block //
		// if region is cached and we are transitioning from previous block then need to check:
		// 1. check that block is in i-cache, and if so, continue
		// 2. if block is NOT in i-cache, then update Cycles,NextPC, and return
		// 3. don't put at beginning of block, so need RunCount >= 1 (will not be jumping between blocks)
		if ( bIsBlockInICache && ( ! ( Address & 0xf ) ) )
		{
#ifdef VERBOSE_RECOMPILE
cout << " END-CB";
#endif

			// new icache block //
			
			// for now just call it quits
			// later will add support for another block
			

			// check if address is cached in i-cache
			//if ( ICache_Device::isCached ( Address ) )
			//{
				// Address is cached //
				
				// update PC here for now
				
				// check if next cache-line is cached before executing it //
				
			if ( RunCount )
			{
					// Not the 1st intruction being encoded //
					
					// if address is cached, then check that address is in cache
					reti &= e->CmpMem32ImmX ( & r->ICache.ICacheBlockSource [ ( Address >> 4 ) & 0xff ], Address & 0x1ffffff0 );
				
#ifdef ENCODE_SINGLE_RUN_PER_BLOCK
					//reti &= e->Jmp_NE ( 0, 64 + ( i >> 2 ) );
					reti &= e->Jmp8_E ( 0, 0 );
					
#ifdef VERBOSE_RECOMPILE
cout << " Offset=" << hex << e->BranchOffset [ 64 + ( i >> 2 ) ];
#endif

#else
					// if not, then reload cache
					reti &= e->Jmp_NE ( 0, 12 + ulCacheLineCount );
#endif
				//}
				
				// cache-block is not loaded //
				
#ifdef UPDATE_BEFORE_RETURN
				// update NextPC
				// it should have already returned if NextPC was modified through other means so this should be ok
				e->MovMemImm32 ( & r->NextPC, Address );
				
				// update CycleCount
				//e->AddMem64ImmX ( & r->CycleCount, CacheBlock_CycleCount - 1 );
				e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount );
#endif
				
				// done - return
				e->Ret ();
				
				// starting a new cache block
				//LocalCycleCount = MemCycles - 1;
				//CacheBlock_CycleCount = 0;
				
				// cache-block is loaded and can continue //
				ret &= e->SetJmpTarget8 ( 0 );

				// update the number cache or block lines crossed
				ulCacheLineCount++;
				
			} // end if ( RunCount )
				
			
			// update pointer based on whether data is in cache currently or not
			// this has to run whenever cache-line changes and instructions are cached, could even run every instruction
			pSrcCodePtr = RGetPointer ( Address );

		} // end if ( bIsBlockInICache && ( ! ( Address & 0xf ) ) )
#endif
			
		
#ifdef VERBOSE_RECOMPILE
cout << " INSTR#" << dec << i;
//cout << " LOC=" << hex << ((u64) e->Get_CodeBlock_CurrentPtr ());
cout << " CycleDiff=" << dec << LocalCycleCount;
#endif


		// set r0 to zero for now
		//e->MovMemImm32 ( &r->GPR [ 0 ].u, 0 );
		
		// in front of the instruction, set NextPC to the next instruction
		// do this at beginning of code block
		// NextPC = PC + 4
		//e->MovMemImm32 ( &r->NextPC, Address + 4 );
		
		// get the instruction
		//inst.Value = *((u32*) SrcCode);
		inst.Value = *pSrcCodePtr;
		
		
		// get the next instruction
		// note: this does not work if the next address is in a new cache block and the region is cached
		NextInst.Value = *(pSrcCodePtr + 1);

		
#ifdef ENABLE_ICACHE
		// check if the address is cached
		// actually will need to check in the delay slot instruction if the delay slot is in the next cache block and if it is loaded or not
		//if ( ICache_Device::isCached ( Address ) )
		if ( bIsBlockInICache )
		{
			// address is in cached region //
			
			// check if next instruction is in same cache block or not
			if ( ! ( ( Address + 4 ) & 0xf ) )
			{
				// instruction is in another cache block
				
				pNextCode = RGetPointer ( Address + 4 );
				NextInst.Value = *pNextCode;
			}

			// make sure instruction is not at end of code block
			if ( ! ( ( Address + 4 ) & ( MaxStep_Mask << 2 ) ) )
			{
				NextInst.Value = -1;
			}
			
		}
		else
#endif
		{
			// not in cached region //
			
			// still need to check against edge of block
			if ( ! ( ( Address + 4 ) & ( MaxStep_Mask << 2 ) ) )
			{
				// this can actually happen, so need to prevent optimizations there
				NextInst.Value = -1;
			}
		}
		
		


#ifdef VERBOSE_RECOMPILE
cout << " OL=" << OpLevel;
#endif

		// check if a forward branch target needs to be set
		if ( pForwardBranchTargets [ BlockIndex & MaxStep_Mask ] )
		{
			// set the branch target
			e->SetJmpTarget ( pForwardBranchTargets [ BlockIndex & MaxStep_Mask ] );
		}
		
		// this is internal to recompiler and says where heading for instruction starts at
		pPrefix_CodeStart [ BlockIndex & MaxStep_Mask ] = e->Get_CodeBlock_CurrentPtr ();
		
		// this can be changed by the instruction being recompiled to point to where the starting entry point should be for instruction instead of prefix
		pCodeStart [ BlockIndex ] = e->Get_CodeBlock_CurrentPtr ();
		
	
			// must add one to the cycle offset for starting point because the interpreter adds an extra cycle at the end of run
			//CycleCount [ BlockIndex ] = LocalCycleCount + 1;
			CycleCount [ BlockIndex ] = LocalCycleCount;
		
		
		//EndAddress [ BlockIndex ] = -1;
		
		// recompile the instruction
		ret = R5900::Recompiler::Recompile ( inst, Address );

		
#ifdef VERBOSE_RECOMPILE
cout << " ret=" << ret;
//cout << " ENC0=" << hex << (((u64*) (pCodeStart [ BlockIndex ])) [ 0 ]);
//cout << " ENC1=" << hex << (((u64*) (pCodeStart [ BlockIndex ])) [ 1 ]);
cout << " ASM: " << R5900::Instruction::Print::PrintInstruction ( inst.Value ).c_str ();
cout << " IDX: " << dec << R5900::Instruction::Lookup::FindByInstruction ( inst.Value );
#endif

		if ( ret <= 0 )
		{
			// there was a problem, and recompiling is done
			
			// need to undo whatever we did for this instruction
			e->UndoInstructionBlock ();
			Local_NextPCModified = false;
			
//cout << "\nUndo: Address=" << hex << Address;
			
			// TODO: if no instructions have been encoded yet, then just try again with a lower optimization level
			if ( OpLevel > 0 )
			{
//cout << "\nNext Op Level down";

				// could not encode the instruction at optimization level, so go down a level and try again
				OpLevel--;
				
				//Address -= 4;
				
				// at this point, this should be the last instruction since we had to go down an op level
				// this shouldn't be so, actually
				//MaxCount = 1;
				
				// here we need to reset and redo the instruction
				bStopEncodingBefore = false;
				bStopEncodingAfter = false;
				Local_NextPCModified = false;
				
				bResetCycleCount = false;
				
				// redo the instruction
				i--;
				continue;
			}
			else
			{
			
				cout << "\nhps2x64: R5900: Recompiler: Error: Unable to encode instruction.";
				
				// mark block as unable to recompile if there were no instructions recompiled at all
				//if ( !Cycles ) DoNotCache [ Block ] = 1;
				
				// done
				break;
			}
		}
		
		
		
			// if this is not the first instruction, then it can halt encoding before it
			if ( RunCount && bStopEncodingBefore )
			{
#ifdef VERBOSE_RECOMPILE
cout << " bStopEncodingBefore";
#endif

#ifdef ENCODE_SINGLE_RUN_PER_BLOCK
				// first need to take back the instruction just encoded
				e->UndoInstructionBlock ();

				// check if we are in a new icache block //
				//if ( ! ( Address & 0xf ) )
				//{
				//	// in a new cache block, so must also clear branch to take back the instruction completely
				//	e->BranchOffset [ 64 + ( i >> 2 ) ] = -1;
				//}
				

				
#ifdef UPDATE_BEFORE_RETURN
				// check that NextPC was not modified
				// doesn't matter here except that RunCount>=1 so it is not first instruction in run, which is handled above
				// next pc was not modified because that will be handled differently now
				//if ( RunCount > 1 && !Local_NextPCModified )
				//{
					// update NextPC
					e->MovMemImm32 ( & r->NextPC, Address );
					
				//}
				
				// update CPU CycleCount
				//e->AddMem64ImmX ( & r->CycleCount, CacheBlock_CycleCount - 1 );
				e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount - MemCycles );
#endif

#ifdef VERBOSE_RECOMPILE
cout << " RETURN";
#endif

				// return;
				reti &= e->x64EncodeReturn ();


				
				// set the current optimization level
				// note: don't do this here, because the optimization level might have been changed by current instruction
				//OpLevel = OptimizeLevel;
				
				// reset flags
				bStopEncodingBefore = false;
				bStopEncodingAfter = false;
				Local_NextPCModified = false;
				
				bResetCycleCount = false;
				
				// starting a new run
				RunCount = 0;
				
				// restart cycle count back to zero
				LocalCycleCount = 0;
				
				// clear delay slots
				//RDelaySlots [ 0 ].Value = 0;
				//RDelaySlots [ 1 ].Value = 0;
				
				//LocalCycleCount = MemCycles - 1;
				//CacheBlock_CycleCount = 0;
				
				// need to redo this instruction at this address
				// since I needed to insert the code to stop the block at this point
				i--;
				continue;
#else

				// do not encode instruction and done encoding
				e->UndoInstructionBlock ();
				Local_NextPCModified = false;
				break;
#endif
			} // end if ( RunCount && bStopEncodingBefore )

			
			
			
			// instruction successfully encoded from MIPS into x64
			e->EndInstructionBlock ();
			
//cout << "\nCool: Address=" << hex << Address << " ret=" << dec << ret << " inst=" << hex << *pSrcCodePtr << " i=" << dec << i;

			// update number of instructions that have been recompiled
			RecompileCount++;
			
			// update to next instruction
			//SrcCode += 4;
			// *pInstrPtr++ = *pSrcCodePtr++;
			pSrcCodePtr++;
			
			// add number of cycles encoded
			Cycles += ret;
			
			// update address
			Address += 4;

			// update instruction count for run
			RunCount++;
			
			// go to next block index
			BlockIndex++;
			
			// update the cycles for run
			LocalCycleCount += MemCycles;
			//CacheBlock_CycleCount += MemCycles;

			// reset the optimization level for next instruction
			OpLevel = OptimizeLevel;
			
			
		// this has to happen here ?
		/*
		if ( bResetCycleCount )
		{
			// this isn't actually to reset the cycle count to zero, but rather to the cycles for the last instruction minus one
			// this is for instructions that wait for MulDiv unit to be ready or wait for COP2 to be ready
			LocalCycleCount = MemCycles - 1;
		}
		*/


#ifdef ENABLE_SINGLE_STEP
			bStopEncodingAfter = true;
#endif

		
		// if directed to stop encoding after the instruction, then do so
		if ( bStopEncodingAfter )
		{
#ifdef VERBOSE_RECOMPILE
cout << " bStopEncodingAfter";
#endif

#ifdef ENCODE_SINGLE_RUN_PER_BLOCK


#ifdef UPDATE_BEFORE_RETURN
			// check that NextPC was not modified and that this is not an isolated instruction
			// actually just need to check if NextPC was modified by the encoded instruction
			if ( !Local_NextPCModified )
			{
				// update NextPC
				e->MovMemImm32 ( & r->NextPC, Address );
			}
			
			// update CycleCount
			e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount - MemCycles );
#endif

				
#ifdef VERBOSE_RECOMPILE
cout << " RETURN";
#endif

			// return;
			reti &= e->x64EncodeReturn ();


			// set the current optimization level
			OpLevel = OptimizeLevel;
			
			// reset flags
			bStopEncodingBefore = false;
			bStopEncodingAfter = false;
			Local_NextPCModified = false;
			
			bResetCycleCount = false;
			
			// clear delay slots
			//RDelaySlots [ 0 ].Value = 0;
			//RDelaySlots [ 1 ].Value = 0;
			
			// starting a new run
			RunCount = 0;
			
			// restart cycle count to zero
			LocalCycleCount = 0;
			
			// cycle counts should start over
			//LocalCycleCount = MemCycles - 1;
			//CacheBlock_CycleCount = 0;
			
#else

				break;
#endif
		} // if ( bStopEncodingAfter )
			
			
			
		// reset flags
		bStopEncodingBefore = false;
		bStopEncodingAfter = false;
		Local_NextPCModified = false;
		
		bResetCycleCount = false;
		

	} // end for ( int i = 0; i < MaxStep; i++, Address += 4 )

#ifdef VERBOSE_RECOMPILE
cout << "\nRecompiler: Done with loop";
#endif




#ifdef ENCODE_SINGLE_RUN_PER_BLOCK
	// at end of block need to return ok //
	
	// encode return if it has not already been encoded at end of block
	if ( RunCount )
	{
	
#ifdef UPDATE_BEFORE_RETURN
		// check that NextPC was not modified and that this is not an isolated instruction
		if ( !Local_NextPCModified )
		{
			// update NextPC
			e->MovMemImm32 ( & r->NextPC, Address );
		}
		
		// update CycleCount
		// after update need to put in the minus MemCycles
		e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount - MemCycles );
#endif

	} // end if ( RunCount )

#ifdef VERBOSE_RECOMPILE
cout << "\nulCacheLineCount=" << dec << ulCacheLineCount;
#endif


#ifdef VERBOSE_RECOMPILE
cout << "\nRecompiler: Encoding RETURN";
#endif


	// return;
	reti &= e->x64EncodeReturn ();
	
#endif


	
	// done encoding block
	e->EndCodeBlock ();
	
	// address is now encoded
	
	
	if ( !reti )
	{
		cout << "\nRecompiler: Out of space in code block.";
	}

#ifdef VERBOSE_RECOMPILE
//cout << "\n(when all done)TEST0=" << hex << (((u64*) (pCodeStart [ 0x27e4 >> 2 ])) [ 0 ]);
//cout << " TEST1=" << hex << (((u64*) (pCodeStart [ 0x27e4 >> 2 ])) [ 1 ]);
#endif

	
	return reti;
	//return RecompileCount;
}

/*
void Recompiler::Invalidate ( u32 Address, u32 Count )
{
	s32 StartBlock, StopBlock;
	
	u32 iBlock;
	
	
	// get the address to actually start checking from
	Address -= ( MaxStep - 1 );
	
	// update the count
	Count += ( MaxStep - 1 );
	
	// get the starting block
	iBlock = ( Address >> 2 ) & NumBlocks_Mask;
	
	while ( Count > 0 )
	{
		if ( ( StartAddress [ iBlock ] == Address ) && ( RunCount [ iBlock ] >= Count ) )
		{
			// invalidate the block
			StartAddress [ iBlock ] = 0xffffffff;
		}
		
		// update vars
		Address += 4;
		iBlock++;
		Count--;
	}
	
}
*/


// code generation //

// generate the instruction prefix to check for any pending events
// will also update NextPC,CycleCount (CPU) and return if there is an event
// will also update CycleCount (System) on a load or store
long Recompiler::Generate_Prefix_EventCheck ( u32 Address, bool bIsBranchOrJump )
{
	long ret;
	
	// get updated CycleCount value for CPU
	e->MovRegMem64 ( RAX, & r->CycleCount );
	e->AddReg64ImmX ( RAX, LocalCycleCount );
	
	
	// want check that there are no events pending //
	
	// get the current cycle count and compare with next event cycle
	// note: actually need to either offset the next event cycle and correct when done or
	// or need to offset the next even cycle into another variable and check against that one
	e->CmpRegMem64 ( RAX, & Playstation1::System::_SYSTEM->NextEvent_Cycle );
	
	// branch if current cycle is greater (or equal?) than next event cycle
	// changing this so that it branches if not returning
	//e->Jmp_A ( 0, 100 + RetJumpCounter++ );
	e->Jmp8_B ( 0, 0 );
	
	// update NextPC
	e->MovMemImm32 ( & r->NextPC, Address );
	
	// update CPU CycleCount
	e->MovMemReg64 ( & r->CycleCount, RAX );
	
	// done for now - return
	ret = e->Ret ();
	
	// jump to here to continue execution in code block
	e->SetJmpTarget8 ( 0 );
	
	// if it is a branch or a jump, then no need to update the System CycleCount
	if ( !bIsBranchOrJump )
	{
		// since we have not reached the next event cycle, should write back the current system cycle
		// so that the correct cycle# gets seen when the store is executed
		// no need to update the CPU cycle count until either a branch/jump is encountered or returning
		// this way, there is no need to reset the current cycle number tally unless a branch/jump is encountered
		ret = e->MovMemReg64 ( & Playstation1::System::_SYSTEM->CycleCount, RAX );
	}

	return ret;
}


// BitTest should be 1 for SH, 3 for SW, 0 for SB, etc
static long R5900::Recompiler::Generate_Normal_Store ( R5900::Instruction::Format i, u32 Address, u32 BitTest, void* StoreFunctionToCall )
{
	long ret;
	
	// part 1: first check for event //
	
#ifdef CHECK_EVENT_AFTER_START
	// get updated CycleCount value for CPU (the value as it would be after instruction executed)
	e->MovRegMem64 ( RAX, & r->CycleCount );
	e->AddReg64ImmX ( RAX, LocalCycleCount - ( MemCycles - 1 ) );
	
	
	// want check that there are no events pending //
	
	// get the current cycle count and compare with next event cycle
	// note: actually need to either offset the next event cycle and correct when done or
	// or need to offset the next even cycle into another variable and check against that one
	e->CmpRegMem64 ( RAX, & Playstation2::System::_SYSTEM->NextEvent_Cycle );
	
	// branch if current cycle is greater (or equal?) than next event cycle
	// changing this so that it branches if not returning
	// note: should probably be below or equal then jump, since the interpreter adds one to cycle
	e->Jmp8_B ( 0, 0 );
	
	// update NextPC
	e->MovMemImm32 ( & r->NextPC, Address );
	
	// update CPU CycleCount
	//e->MovMemReg64 ( & r->CycleCount, RAX );
	e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount - MemCycles );
	
	// done for now - return
	e->Ret ();
	
	// jump to here to continue execution in code block
	e->SetJmpTarget8 ( 0 );
	
	// since we have not reached the next event cycle, should write back the current system cycle
	// so that the correct cycle# gets seen when the store is executed
	// no need to update the CPU cycle count until either a branch/jump is encountered or returning
	// this way, there is no need to reset the current cycle number tally unless a branch/jump is encountered
	e->DecReg64 ( RAX );
	e->MovMemReg64 ( & Playstation1::System::_SYSTEM->CycleCount, RAX );
	
	// part 2: check for synchronous interrupt //
	
	// this is where the entry point should be if this is the first instruction in the run
	pCodeStart [ BlockIndex ] = e->Get_CodeBlock_CurrentPtr ();
#endif
	
	// get the store address
	//u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	e->MovRegFromMem32 ( RCX, &r->GPR [ i.Base ].sw0 );
	e->AddReg32ImmX ( RCX, i.sOffset );

	// check for synchronous interrupt
	
	// if there is a synchronous interrupt possible, then check for it
	if ( BitTest )
	{
		// if ( StoreAddress & 0x1 )
		e->TestReg32ImmX ( RCX, BitTest );

		// branch if zero
		e->Jmp8_E ( 0, 0 );
		
		// update CycleCount, set PC, then jump to synchronous interrupt
		//e->MovMemReg64 ( & r->CycleCount, RAX );
		e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount );
		
		// set pc
		e->MovMemImm32 ( & r->PC, Address );
		
		//r->ProcessSynchronousInterrupt ( Cpu::EXC_ADES );
		e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_ADES> );
		
		// continue processing store from here //
		e->SetJmpTarget8 ( 0 );
	}
	
	// part 3: execute the store //
			
	switch ( i.Opcode )
	{
		case OPSQ:
			// get address of value to store
			e->MovRegImm64 ( RDX, &r->GPR [ i.Rt ].s );
			
			// clear bottom four bits of address
			e->AndReg32ImmX ( RCX, ~0xf );
			break;
		
		default:
			// get the value to store
			e->MovRegMem64 ( RDX, &r->GPR [ i.Rt ].s );
			
			break;
			
	}
	
	// call the function to store value //
	
#ifdef RESERVE_STACK_FRAME_FOR_CALL
	e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

	ret = e->Call ( StoreFunctionToCall );

#ifdef RESERVE_STACK_FRAME_FOR_CALL
	ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

	return ret;
}



// BitTest should be 1 for LH, 3 for LW, 0 for LB, etc
static long Recompiler::Generate_Normal_Load ( R5900::Instruction::Format i, u32 Address, u32 BitTest, void* LoadFunctionToCall )
{
	long ret;
	
#ifdef CHECK_EVENT_AFTER_START
	// part 1: first check for event //
	
	// get updated CycleCount value for CPU
	e->MovRegMem64 ( RAX, & r->CycleCount );
	e->AddReg64ImmX ( RAX, LocalCycleCount - ( MemCycles - 1 ) );
	
	
	// want check that there are no events pending //
	
	// get the current cycle count and compare with next event cycle
	// note: actually need to either offset the next event cycle and correct when done or
	// or need to offset the next even cycle into another variable and check against that one
	e->CmpRegMem64 ( RAX, & Playstation1::System::_SYSTEM->NextEvent_Cycle );
	
	// branch if current cycle is greater (or equal?) than next event cycle
	// changing this so that it branches if not returning
	// note: should probably be below or equal then jump, since the interpreter adds one to cycle
	e->Jmp8_B ( 0, 0 );
	
	// update NextPC
	e->MovMemImm32 ( & r->NextPC, Address );
	
	// update CPU CycleCount
	//e->MovMemReg64 ( & r->CycleCount, RAX );
	e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount - MemCycles );
	
	// done for now - return
	e->Ret ();
	
	// jump to here to continue execution in code block
	e->SetJmpTarget8 ( 0 );
	
	// since we have not reached the next event cycle, should write back the current system cycle
	// so that the correct cycle# gets seen when the store is executed
	// no need to update the CPU cycle count until either a branch/jump is encountered or returning
	// this way, there is no need to reset the current cycle number tally unless a branch/jump is encountered
	e->MovMemReg64 ( & Playstation1::System::_SYSTEM->CycleCount, RAX );
	
	// part 2: check for synchronous interrupt //
	
	// this is where the entry point should be if this is the first instruction in the run
	pCodeStart [ BlockIndex ] = e->Get_CodeBlock_CurrentPtr ();
#endif
	
	// get the store address
	//u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	e->MovRegFromMem32 ( RCX, &r->GPR [ i.Base ].sw0 );
	e->AddReg32ImmX ( RCX, i.sOffset );

	// check for synchronous interrupt
	
	// if there is a synchronous interrupt possible, then check for it
	if ( BitTest )
	{
		// if ( StoreAddress & 0x1 )
		e->TestReg32ImmX ( RCX, BitTest );

		// branch if zero
		e->Jmp8_E ( 0, 0 );
		
		// update CycleCount, set PC, then jump to synchronous interrupt
		//e->MovMemReg64 ( & r->CycleCount, RAX );
		e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount );
		
		// set pc
		e->MovMemImm32 ( & r->PC, Address );
		
		//r->ProcessSynchronousInterrupt ( Cpu::EXC_ADES );
		e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_ADEL> );
		
		// continue processing store from here //
		e->SetJmpTarget8 ( 0 );
	}
	
	// if 128-bit load, then clear bottom four bits of address
	if ( i.Opcode == OPLQ )
	{
		e->AndReg32ImmX ( RCX, ~0xf );
	}
	
	// part 3: execute the load //
	
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

	ret = e->Call ( LoadFunctionToCall );

#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif


	// part 4: store the result //
	
	/*
	// make sure that destination registers don't match
	// if so, cancel the load
	//if ( GetDestRegs ( NextInst ) & ( 1ULL << i.Rt ) )
	//{
	//	// cancelling load //
	//}
	//else
	{
		
		// for loads
		// 1. check that there are no events. If so, update Cycles,NextPC, then return
		// 2. check for synchronous interrupt
		// 3. check that there are no conflicts. If so, put load in delay slot, update Cycles,NextPC, then return
		// 4. encode load, then encode load delay slot
		// 5. if going across cache line and next line is not loaded, then put load in delay slot and return
		// 6. if it is a store in the delay slot, then can just process normally as if there is no delay slot and immediately load
		// 7. if at edge of code block, then put load in delay slot and return
		
		e->MovRegToMem32 ( & r->DelaySlots [ 1 ].Data, RDX );
		e->MovMemImm32 ( & r->DelaySlots [ 1 ].Instruction.Value, i.Value );
		
#ifdef EXECUTE_LOADS_IMMEDIATELY
		// check for conflicts
		// if there is a conflict, then put in delay slot and return
		// no need to check for r0 as it is fixed
		if ( ( ( 1 << i.Rt ) & GetSourceRegs ( NextInst, Address + 4 ) & ~1 ) || ( NextInst.Value == -1 ) || ( ( 1 << i.Rt ) & Get_DelaySlot_DestRegs ( NextInst ) ) )
		{
#endif

			// conflict //
			
			// put the load in delay slot and return
			//e->MovRegImm32 ( 0, i.sOffset );
			//e->AddRegMem32 ( 0, &r->GPR [ i.Base ].u );
			//e->OrMem64ImmX ( &r->Status.Value, 1 );
			
			e->MovReg64ImmX ( RAX, (u64) LoadFunctionToCall );
			e->MovMemReg64 ( (long long*) & r->DelaySlots [ 1 ].cb, RAX );
			
			e->MovMemImm32 ( & r->NextDelaySlotIndex, 0 );
			e->OrMem64ImmX ( &r->Status.Value, 2 );
			ret = e->MovMemImm32 ( & r->LastModifiedRegister, 255 );
			
			// return after putting the load into delay slot
			bStopEncodingAfter = true;
			
#ifdef EXECUTE_LOADS_IMMEDIATELY
		}
		else
		{
			// NO conflict //
			
			// execute the load immediately
			e->MovMemImm32 ( & r->NextDelaySlotIndex, 1 );
			ret = e->MovMemImm32 ( & r->LastModifiedRegister, 255 );
			
			// encode the load
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			ret = e->Call ( LoadFunctionToCall );

#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
		}
#endif

	} // end if ( GetDestRegs ( NextInst ) & ( 1ULL << i.Rt ) )
	*/

	return ret;
}


static long Recompiler::Generate_Normal_Branch ( R5900::Instruction::Format i, u32 Address, void* BranchFunctionToCall )
{
	u32 TargetAddress;
	long ret;
	
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nStart";
#endif
	
	// get the target address
	switch ( i.Opcode )
	{
		case OPJR:
		//case OPJALR:
		
			// don't know what the address is since it is variable
			TargetAddress = 0;
			break;
			
		case OPJ:
		case OPJAL:
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nAddress=" << hex << Address << " JumpAddress=" << i.JumpAddress;
#endif
			TargetAddress = ( 0xf0000000 & Address ) | ( i.JumpAddress << 2 );
			break;
			
		// must be a branch
		default:
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nAddress=" << hex << Address << " sOffset=" << i.sImmediate;
#endif
			TargetAddress = 4 + Address + ( i.sImmediate << 2 );
			break;
	}
	
	
#ifdef CHECK_EVENT_AFTER_START
	// part 1: first check for event //
	
	// get updated CycleCount value for CPU
	e->MovRegMem64 ( RAX, & r->CycleCount );
	e->AddReg64ImmX ( RAX, LocalCycleCount - ( MemCycles - 1 ) );
	
	
	// want check that there are no events pending //
	
	// get the current cycle count and compare with next event cycle
	// note: actually need to either offset the next event cycle and correct when done or
	// or need to offset the next even cycle into another variable and check against that one
	e->CmpRegMem64 ( RAX, & Playstation1::System::_SYSTEM->NextEvent_Cycle );
	
	// branch if current cycle is greater (or equal?) than next event cycle
	// changing this so that it branches if not returning
	// note: should probably be below or equal then jump, since the interpreter adds one to cycle
	e->Jmp8_B ( 0, 0 );
	
	// update NextPC
	e->MovMemImm32 ( & r->NextPC, Address );
	
	// update CPU CycleCount
	//e->MovMemReg64 ( & r->CycleCount, RAX );
	e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount - MemCycles );
	
	// done for now - return
	e->Ret ();

	
	// jump to here to continue execution in code block
	e->SetJmpTarget8 ( 0 );

	// this is where the entry point should be if this is the first instruction in the run
	pCodeStart [ BlockIndex ] = e->Get_CodeBlock_CurrentPtr ();
#endif

	// check for sychronous interrupt if applicable
	if ( i.Opcode == OPJR )
	{
		// get the address being jumped to
		e->MovRegMem32 ( RDX, & r->GPR [ i.Rs ].uw0 );
		
		// if ( StoreAddress & 0x1 )
		//e->TestReg32ImmX ( RDX, BitTest );
		e->TestReg32ImmX ( RDX, 0x3 );

		// branch if zero
		e->Jmp8_E ( 0, 0 );
		
		// update CycleCount, set PC, then jump to synchronous interrupt
		//e->MovMemReg64 ( & r->CycleCount, RAX );
		e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount );
		
		// set pc
		e->MovMemImm32 ( & r->PC, Address );
		
		//r->ProcessSynchronousInterrupt ( Cpu::EXC_ADEL );
		e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_ADEL> );
		
		// continue processing branch/jump from here //
		e->SetJmpTarget8 ( 0 );
	}


	// check if branching or not
	switch ( i.Opcode )
	{
		case OPBEQ:
		case OPBEQL:
			e->MovRegMem64 ( RCX, & r->GPR [ i.Rs ].u );
			e->CmpMemReg64 ( & r->GPR [ i.Rt ].u, RCX );
			//e->Jmp8_NE ( 0, 0 );
			e->Jmp_NE ( 0, 0 );
			break;
			
		case OPBNE:
		case OPBNEL:
			e->MovRegMem64 ( RCX, & r->GPR [ i.Rs ].u );
			e->CmpMemReg64 ( & r->GPR [ i.Rt ].u, RCX );
			//e->Jmp8_E ( 0, 0 );
			e->Jmp_E ( 0, 0 );
			break;
			
		case OPBLEZ:
		case OPBLEZL:
			e->CmpMem64ImmX ( & r->GPR [ i.Rs ].s, 0 );
			//e->Jmp8_G ( 0, 0 );
			e->Jmp_G ( 0, 0 );
			break;
			
		case OPBGTZ:
		case OPBGTZL:
			e->CmpMem64ImmX ( & r->GPR [ i.Rs ].s, 0 );
			//e->Jmp8_LE ( 0, 0 );
			e->Jmp_LE ( 0, 0 );
			break;
			
		case OPBLTZ:
		//case OPBGEZ:
		//case OPBLTZAL:
		//case OPBGEZAL:
			switch ( i.Rt )
			{
				case RTBLTZ:
				case RTBLTZL:
					e->CmpMem64ImmX ( & r->GPR [ i.Rs ].s, 0 );
					//e->Jmp8_GE ( 0, 0 );
					e->Jmp_GE ( 0, 0 );
					break;
					
				case RTBGEZ:
				case RTBGEZL:
					e->CmpMem64ImmX ( & r->GPR [ i.Rs ].s, 0 );
					//e->Jmp8_L ( 0, 0 );
					e->Jmp_L ( 0, 0 );
					break;
			
				case RTBLTZAL:
				case RTBLTZALL:
					e->CmpMem64ImmX ( & r->GPR [ i.Rs ].s, 0 );
					e->MovMemImm64 ( & r->GPR [ 31 ].u, Address + 8 );
					//e->Jmp8_GE ( 0, 0 );
					e->Jmp_GE ( 0, 0 );
					break;
			
				case RTBGEZAL:
				case RTBGEZALL:
					e->CmpMem64ImmX ( & r->GPR [ i.Rs ].s, 0 );
					e->MovMemImm64 ( & r->GPR [ 31 ].u, Address + 8 );
					//e->Jmp8_L ( 0, 0 );
					e->Jmp_L ( 0, 0 );
					break;
			}
			
			break;
			
		case OPJAL:
			e->MovMemImm64 ( & r->GPR [ 31 ].u, Address + 8 );
			break;
			
		case OPJALR:
		
			if ( i.Funct == 9 )
			{
				// JALR //
				
				// make sure Rd is not r0
				if ( i.Rd )
				{
					// save return address in Rd
					e->MovMemImm64 ( & r->GPR [ i.Rd ].u, Address + 8 );
				}
			}
			
			break;
	}
	
	
	// branching //
	

	
	
#ifdef ALLOW_ENCODING_DELAYSLOT
	// check if target address is inside current block or not
	// for now, only check for only jumping backwards
	//if ( TargetAddress && TargetAddress >= CurrentBlock_StartAddress && TargetAddress < NextBlock_StartAddress && isBranchDelayOk ( NextInst.Value ) )
	if (
#ifdef ENABLE_AUTO_BRANCH
		( TargetAddress && TargetAddress >= CurrentBlock_StartAddress && TargetAddress <= Address && isBranchDelayOk ( NextInst.Value, Address + 4 ) ) ||
#endif
		isBranchDelayOk ( NextInst.Value, Address + 4 )
		)
	{
		// target can be reached with jump in same block //
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nisBranchDelayOk";
#endif

//if ( i.Opcode == OPBNEL && NextInst.Opcode == OPADDIU )
//{
//	cout << "\nAddress=" << hex << Address << dec << " Opcode=BNEL Next= ADDIU " << R5900::Instruction::Print::PrintInstruction ( NextInst.Value );
//}
		
#ifndef CACHE_NOT_IMPLEMENTED
		// check if this is a cached region
		if ( bIsBlockInICache )
		{
			// region is cached //
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nbIsBlockInICache";
#endif
			
			// check if instruction in delay slot is in another cache block
			if ( ! ( ( Address + 4 ) & 0xf ) )
			{
				// instruction in delay slot is in next cache block //
				
				// check if cache block is loaded
				e->CmpMem32ImmX ( & r->ICache.ICacheBlockSource [ ( ( Address + 4 ) >> 4 ) & 0xff ], ( Address + 4 ) & 0x1ffffff0 );
				
				// jump if the cache line is not loaded
				e->Jmp8_NE ( 0, 1 );
				
				// the cache-line is loaded //
				
			}
		
#ifdef ENABLE_AUTO_BRANCH
			// check if that cache line is loaded we are jumping to
			e->CmpMem32ImmX ( & r->ICache.ICacheBlockSource [ ( TargetAddress >> 4 ) & 0xff ], TargetAddress & 0x1ffffff0 );
			
			e->Jmp8_E ( 0, 2 );
#endif
		}
#endif

#ifndef ENABLE_AUTO_BRANCH
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nbRecompile";
#endif
		// no auto branch, but cache line is loaded, so execute delay slot //
		// execute the instruction in delay slot
		// if it is in the same cache block or its cache block is loaded
		// note: if this is a mult/div then need to update the CPU cycles both before and after ?
		ret = Recompile ( NextInst, Address + 4 );
		
		if ( ret <= 0 )
		{
			cout << "\nR5900: Recompiler: Error encoding branch in delay slot.";
		}
		
		// update CPU CycleCount
		// note: must include both branch and delay slot ? And also must subtract the cyclecount at address, then add one since CycleCount[] is minus one
		e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount + MemCycles );
		
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nTargetAddress=" << hex << TargetAddress;
#endif
		
		if ( TargetAddress )
		{
			// update NextPC
			e->MovMemImm32 ( & r->NextPC, TargetAddress );
		}
		else
		{
			if ( i.Opcode == OPJR )
			{
				//e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].u );
				e->MovMemReg32 ( & r->NextPC, RDX );
			}
			else
			{
				// ???
				cout << "\nR5900: Recompiler: Problem setting NextPC for branch after delay slot.";
			}
		}
		
		// done - return
		ret = e->Ret ();
#endif
		
		// the cache-line is not loaded //
		
		// also jump here from above if needed
		if ( !e->SetJmpTarget8 ( 1 ) )
		{
			cout << "\nR5900: Recompiler: Short branch1 too far.";
		}
		
		
		// put branch into delay slot
		
		// first put in the target address
		switch ( i.Opcode )
		{
			case OPJR:
			//case OPJALR:
				e->MovMemReg32 ( & r->DelaySlots [ 1 ].Data, RDX );
				break;
				
			default:
				e->MovMemImm32 ( & r->DelaySlots [ 1 ].Data, TargetAddress );
				break;
		}
		
		// put in the instruction
		e->MovMemImm32 ( & r->DelaySlots [ 1 ].Instruction.Value, i.Value );
		
		e->MovReg64ImmX ( RAX, (u64) BranchFunctionToCall );
		e->MovMemReg64 ( (long long*) & r->DelaySlots [ 1 ].cb, RAX );
		
		e->MovMemImm32 ( & r->NextDelaySlotIndex, 0 );
		e->OrMem64ImmX ( &r->Status.Value, 2 << 8 );
		
		// important? - must update LastPC? PC? when releasing to a branch delay slot?
		e->MovMemImm32 ( & r->PC, Address );

		// update NextPC,CycleCount
#ifdef UPDATE_BEFORE_RETURN
		// update NextPC
		// it should have already returned if NextPC was modified through other means so this should be ok
		// NextPC is Address+4 here because the current instruction was already executed
		e->MovMemImm32 ( & r->NextPC, Address + 4 );
				
		// update CycleCount
		// ***todo*** should be the cycles plus one instruction since branch into delay slot has already been executed
		e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount );
#endif
				
		// done - return
		ret = e->Ret ();
				
#ifdef ENABLE_AUTO_BRANCH
		// cache-block is loaded and can continue //
		e->SetJmpTarget8 ( 2 );
		
		
		// execute the instruction in delay slot
		// if it is in the same cache block or its cache block is loaded
		// note: if this is a mult/div then need to update the CPU cycles both before and after ?
		ret = Recompile ( NextInst, Address + 4 );
		
		// update CPU CycleCount
		// note: must include both branch and delay slot ? And also must subtract the cyclecount at address, then add one since CycleCount[] is minus one
		e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount + ( MemCycles << 1 ) - CycleCount [ ( TargetAddress >> 2 ) & ulIndex_Mask ] + 1 );
		
		// check if jumping backwards or forwards
		if ( TargetAddress <= Address )
		{
			// backward branch //
			ret = e->JMP ( pCodeStart [ ( TargetAddress >> 2 ) & ulIndex_Mask ] );
		}
		else
		{
			// forward branch //
			e->Jmp ( 0, ForwardBranchIndex );
			
			// set the label for that line in code
			pForwardBranchTargets [ ( TargetAddress >> 2 ) & MaxStep_Mask ] = ForwardBranchIndex;
			
			// next time we'll use the next index
			ForwardBranchIndex++;
		}
#endif
	}
	else
#endif
	{
		// not directly branching to target address right now //
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nIntoDelaySlot";
#endif
		
		// put branch into delay slot
		//e->MovMemImm32 ( & r->DelaySlots [ 1 ].Data, TargetAddress );
		//e->MovMemImm32 ( & r->DelaySlots [ 1 ].Instruction.Value, i.Value );

		// first put in the target address
		switch ( i.Opcode )
		{
			case OPJR:
			//case OPJALR:
				e->MovMemReg32 ( & r->DelaySlots [ 1 ].Data, RDX );
				break;
				
			default:
				e->MovMemImm32 ( & r->DelaySlots [ 1 ].Data, TargetAddress );
				break;
		}
		
		// put in the instruction
		e->MovMemImm32 ( & r->DelaySlots [ 1 ].Instruction.Value, i.Value );

		
		e->MovReg64ImmX ( RAX, (u64) BranchFunctionToCall );
		e->MovMemReg64 ( (long long*) & r->DelaySlots [ 1 ].cb, RAX );
		
		e->MovMemImm32 ( & r->NextDelaySlotIndex, 0 );
		e->OrMem64ImmX ( &r->Status.Value, 2 << 8 );
		
		// important? - must update LastPC? PC? when releasing to a branch delay slot?
		e->MovMemImm32 ( & r->PC, Address );

		// update NextPC,CycleCount
#ifdef UPDATE_BEFORE_RETURN
		// update NextPC
		// it should have already returned if NextPC was modified through other means so this should be ok
		// NextPC is Address+4 here because the current instruction was already executed
		e->MovMemImm32 ( & r->NextPC, Address + 4 );
				
		// update CycleCount
		// ***todo*** should be the cycles plus one instruction since branch into delay slot has already been executed
		e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount );
#endif
				
		// done - return
		ret = e->Ret ();
	}
	
	// if inside current block, then check if cache line is loaded
	// if cache line is loaded, then jump to it, otherwise put branch in delay slot and return
	
	// not branching //
	/*
	if ( !e->SetJmpTarget8 ( 0 ) )
	{
		cout << "\nR5900: Recompiler: Short branch0 too far.";
	}
	*/
	
	if ( !e->SetJmpTarget ( 0 ) )
	{
		cout << "\nR5900: Recompiler: Short branch0 too far.";
	}

	// check for likely branch
	if ( i.Opcode >= 0x14 || ( i.Opcode == 1 && ( i.Rt & 0x7 ) >= 2 ) )
	{
		// likely branch //
		
		// update NextPC,CycleCount
#ifdef UPDATE_BEFORE_RETURN
		// update NextPC
		// it should have already returned if NextPC was modified through other means so this should be ok
		// NextPC is Address+4 here because the current instruction was already executed
		//e->MovMemImm32 ( & r->NextPC, Address + 4 );
		e->MovMemImm32 ( & r->NextPC, Address + 8 );

		// update CycleCount
		// ***todo*** should be the cycles plus one instruction since branch into delay slot has already been executed
		e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount );
#endif

		// done - return
		ret = e->Ret ();
	}


#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nEND";
#endif

	// done
	return ret;
}


static long Recompiler::Generate_Normal_Trap ( R5900::Instruction::Format i, u32 Address )
{
	// step 1: check for trap condition //
	
	switch ( i.Opcode )
	{
		case OPSPECIAL:
		
			switch ( i.Funct )
			{
				case SPTGE:
					e->MovRegMem64 ( RAX, & r->GPR [ i.Rs ].u );
					e->CmpRegMem64 ( RAX, & r->GPR [ i.Rt ].u );
					e->Jmp8_L ( 0, 0 );
					break;
					
				case SPTGEU:
					e->MovRegMem64 ( RAX, & r->GPR [ i.Rs ].u );
					e->CmpRegMem64 ( RAX, & r->GPR [ i.Rt ].u );
					e->Jmp8_B ( 0, 0 );
					break;
					
				case SPTLT:
					e->MovRegMem64 ( RAX, & r->GPR [ i.Rs ].u );
					e->CmpRegMem64 ( RAX, & r->GPR [ i.Rt ].u );
					e->Jmp8_GE ( 0, 0 );
					break;
					
				case SPTLTU:
					e->MovRegMem64 ( RAX, & r->GPR [ i.Rs ].u );
					e->CmpRegMem64 ( RAX, & r->GPR [ i.Rt ].u );
					e->Jmp8_AE ( 0, 0 );
					break;
					
				case SPTEQ:
					e->MovRegMem64 ( RAX, & r->GPR [ i.Rs ].u );
					e->CmpRegMem64 ( RAX, & r->GPR [ i.Rt ].u );
					e->Jmp8_NE ( 0, 0 );
					break;
					
				case SPTNE:
					e->MovRegMem64 ( RAX, & r->GPR [ i.Rs ].u );
					e->CmpRegMem64 ( RAX, & r->GPR [ i.Rt ].u );
					e->Jmp8_E ( 0, 0 );
					break;
			}
			
			break;
			
		case OPREGIMM:
			
			switch ( i.Rt )
			{
				case RTTGEI:
					e->CmpMem64ImmX ( & r->GPR [ i.Rs ].u, i.sImmediate );
					e->Jmp8_L ( 0, 0 );
					break;
					
				case RTTGEIU:
					e->CmpMem64ImmX ( & r->GPR [ i.Rs ].u, i.sImmediate );
					e->Jmp8_B ( 0, 0 );
					break;
					
				case RTTLTI:
					e->CmpMem64ImmX ( & r->GPR [ i.Rs ].u, i.sImmediate );
					e->Jmp8_GE ( 0, 0 );
					break;
					
				case RTTLTIU:
					e->CmpMem64ImmX ( & r->GPR [ i.Rs ].u, i.sImmediate );
					e->Jmp8_AE ( 0, 0 );
					break;
					
				case RTTEQI:
					e->CmpMem64ImmX ( & r->GPR [ i.Rs ].u, i.sImmediate );
					e->Jmp8_NE ( 0, 0 );
					break;
					
				case RTTNEI:
					e->CmpMem64ImmX ( & r->GPR [ i.Rs ].u, i.sImmediate );
					e->Jmp8_E ( 0, 0 );
					break;
			}
			
			break;
	}
	
	// step 2: trap //
	
#ifdef UPDATE_BEFORE_RETURN
	// update NextPC
	// it should have already returned if NextPC was modified through other means so this should be ok
	// NextPC is Address+4 here because the current instruction was already executed
	//e->MovMemImm32 ( & r->NextPC, Address + 4 );
	e->MovMemImm32 ( & r->PC, Address );

	// update CycleCount
	// ***todo*** should be the cycles plus one instruction since branch into delay slot has already been executed
	e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount );
#endif

	e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_TRAP> );
	
	// no trap taken
	e->SetJmpTarget8 ( 0 );
	
}



// regular arithemetic //

// *** todo *** no need to save LastModifiedRegister unless instruction is KNOWN to be in a delay slot on run
long Recompiler::ADDU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "ADDU";
	static const void *c_vFunction = R5900::Instruction::Execute::ADDU;
	
	//r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u + r->GPR [ i.Rt ].u;
	//CHECK_DELAYSLOT ( i.Rd );
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //ADDU );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rs )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					e->MovsxdReg64Mem32 ( RAX, &r->GPR [ i.Rt ].sw0 );
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( !i.Rt )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->MovsxdReg64Mem32 ( RAX, &r->GPR [ i.Rs ].sw0 );
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				/*
				else if ( i.Rd == i.Rs )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					e->MovsxdReg64Mem32 ( RAX, &r->GPR [ i.Rt ].s );
					//ret = e->AddMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->AddMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rt )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->MovsxdReg64Mem32 ( RAX, &r->GPR [ i.Rs ].s );
					//ret = e->AddMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->AddMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				*/
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].sw0 );
					e->AddRegMem32 ( RAX, &r->GPR [ i.Rt ].uw0 );
					e->Cdqe ();
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	
	return 1;
}

long Recompiler::SUBU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SUBU";
	static const void *c_vFunction = R5900::Instruction::Execute::SUBU;
	
	//r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u - r->GPR [ i.Rt ].u;
	//CHECK_DELAYSLOT ( i.Rd );
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //SUBU );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->MovsxdReg64Mem32 ( RAX, &r->GPR [ i.Rs ].sw0 );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				/*
				else if ( i.Rd == i.Rs )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->SubMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				*/
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].uw0 );
					e->SubRegMem32 ( RAX, &r->GPR [ i.Rt ].uw0 );
					e->Cdqe ();
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].u, RAX );
				}
				
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	
	return 1;
}

long Recompiler::AND ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "AND";
	static const void *c_vFunction = R5900::Instruction::Execute::AND;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //AND );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rs )
				{
					//ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
					ret = e->MovMemImm64 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( !i.Rt )
				{
					//ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
					ret = e->MovMemImm64 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( i.Rd == i.Rs )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].s );
					//ret = e->AndMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->AndMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rt )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
					//ret = e->AndMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->AndMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].u );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].u );
					//e->AndRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
					e->AndRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].u, RAX );
				}
				
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	
	return 1;
}

long Recompiler::OR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "OR";
	static const void *c_vFunction = R5900::Instruction::Execute::OR;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //OR );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rs )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].s );
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( !i.Rt )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rs )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].s );
					//ret = e->OrMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->OrMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rt )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
					//ret = e->OrMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->OrMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].u );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].u );
					//e->OrRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
					e->OrRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].u, RAX );
				}
				
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	
	return 1;
}

long Recompiler::XOR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "XOR";
	static const void *c_vFunction = R5900::Instruction::Execute::XOR;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //XOR );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rs )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].s );
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( !i.Rt )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rs )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].s );
					//ret = e->XorMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->XorMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rt )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
					//ret = e->XorMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->XorMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].u );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].u );
					//e->XorRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
					e->XorRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].u, RAX );
				}
				
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NOR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "NOR";
	static const void *c_vFunction = R5900::Instruction::Execute::NOR;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //NOR );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rs )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].s );
					//e->NotReg32 ( RAX );
					e->NotReg64 ( RAX );
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( !i.Rt )
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
					//e->NotReg32 ( RAX );
					e->NotReg64 ( RAX );
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].u );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].u );
					//e->OrRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
					e->OrRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
					//e->NotReg32 ( RAX );
					e->NotReg64 ( RAX );
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].u, RAX );
				}
				
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SLT ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SLT";
	static const void *c_vFunction = R5900::Instruction::Execute::SLT;
	
	//r->GPR [ i.Rd ].s = r->GPR [ i.Rs ].s < r->GPR [ i.Rt ].s ? 1 : 0;
	//CHECK_DELAYSLOT ( i.Rd );
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //SLT );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				// this should zero-extend to 64 bits
				e->XorRegReg32 ( RCX, RCX );
				//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].u );
				e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].u );
				//e->CmpRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
				e->CmpRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
				e->Set_L ( RCX );
				//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RCX );
				ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].u, RCX );
				
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SLTU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SLTU";
	static const void *c_vFunction = R5900::Instruction::Execute::SLTU;
	
	//r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u < r->GPR [ i.Rt ].u ? 1 : 0;
	//CHECK_DELAYSLOT ( i.Rd );
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //SLTU );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				// this should zero-extend to 64 bits
				e->XorRegReg32 ( RCX, RCX );
				//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].u );
				e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].u );
				//e->CmpRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
				e->CmpRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
				e->Set_B ( RCX );
				//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RCX );
				ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].u, RCX );
				
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}


////////////////////////////////////////////
// I-Type Instructions (non-interrupt)



long Recompiler::ADDIU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "ADDIU";
	static const void *c_vFunction = R5900::Instruction::Execute::ADDIU;
	
	//r->GPR [ i.Rt ].s = r->GPR [ i.Rs ].s + i.sImmediate;
	//CHECK_DELAYSLOT ( i.Rt );
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rt )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //ADDIU );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				if ( !i.Rs )
				{
					//e->MovMemImm32 ( &r->GPR [ i.Rt ].s, i.sImmediate );
					ret = e->MovMemImm64 ( &r->GPR [ i.Rt ].s, i.sImmediate );
				}
				/*
				else if ( i.Rt == i.Rs )
				{
					e->AddMem64ImmX ( &r->GPR [ i.Rt ].s, i.sImmediate );
				}
				*/
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].sw0 );
					//e->MovsxdReg64Mem32 ( RAX, &r->GPR [ i.Rs ].sw0 );
					e->AddReg32ImmX ( RAX, i.sImmediate );
					//e->AddReg64ImmX ( RAX, i.sImmediate );
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
					e->Cdqe ();
					ret = e->MovMemReg64 ( &r->GPR [ i.Rt ].s, RAX );
				}
				
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nError encoding ADDIU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::ANDI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "ANDI";
	static const void *c_vFunction = R5900::Instruction::Execute::ANDI;
	
	//r->GPR [ i.Rt ].u = r->GPR [ i.Rs ].u & i.uImmediate;
	//CHECK_DELAYSLOT ( i.Rt );
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rt )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //ANDI );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				if ( !i.Rs )
				{
					//e->MovMemImm32 ( &r->GPR [ i.Rt ].s, 0 );
					e->MovMemImm64 ( &r->GPR [ i.Rt ].s, 0 );
				}
				else if ( i.Rt == i.Rs )
				{
					//e->AndMem32ImmX ( &r->GPR [ i.Rt ].s, i.uImmediate );
					e->AndMem64ImmX ( &r->GPR [ i.Rt ].s, i.uImmediate );
				}
				else
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
					//e->AndReg32ImmX ( RAX, i.uImmediate );
					e->AndReg64ImmX ( RAX, i.uImmediate );
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rt ].s, RAX );
				}
				
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nError encoding ADDIU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::ORI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "ORI";
	static const void *c_vFunction = R5900::Instruction::Execute::ORI;
	
	//r->GPR [ i.Rt ].u = r->GPR [ i.Rs ].u | i.uImmediate;
	//CHECK_DELAYSLOT ( i.Rt );
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rt )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //ORI );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				if ( !i.Rs )
				{
					//e->MovMemImm32 ( &r->GPR [ i.Rt ].s, i.uImmediate );
					e->MovMemImm64 ( &r->GPR [ i.Rt ].s, i.uImmediate );
				}
				else if ( i.Rt == i.Rs )
				{
					//e->OrMem32ImmX ( &r->GPR [ i.Rt ].s, i.uImmediate );
					e->OrMem64ImmX ( &r->GPR [ i.Rt ].s, i.uImmediate );
				}
				else
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
					//e->OrReg32ImmX ( RAX, i.uImmediate );
					e->OrReg64ImmX ( RAX, i.uImmediate );
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rt ].s, RAX );
				}
				
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nError encoding ADDIU instruction.\n";
		return -1;
	}
	
	return 1;
}

long Recompiler::XORI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "XORI";
	static const void *c_vFunction = R5900::Instruction::Execute::XORI;
	
	//r->GPR [ i.Rt ].u = r->GPR [ i.Rs ].u ^ i.uImmediate;
	//CHECK_DELAYSLOT ( i.Rt );
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rt )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //XORI );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				if ( !i.Rs )
				{
					//e->MovMemImm32 ( &r->GPR [ i.Rt ].s, i.uImmediate );
					e->MovMemImm64 ( &r->GPR [ i.Rt ].s, i.uImmediate );
				}
				else if ( i.Rt == i.Rs )
				{
					//e->XorMem32ImmX ( &r->GPR [ i.Rt ].s, i.uImmediate );
					e->XorMem64ImmX ( &r->GPR [ i.Rt ].s, i.uImmediate );
				}
				else
				{
					//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
					//e->XorReg32ImmX ( RAX, i.uImmediate );
					e->XorReg64ImmX ( RAX, i.uImmediate );
					//ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rt ].s, RAX );
				}
				
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nError encoding ADDIU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SLTI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SLTI";
	static const void *c_vFunction = R5900::Instruction::Execute::SLTI;
	
	//r->GPR [ i.Rt ].s = r->GPR [ i.Rs ].s < i.sImmediate ? 1 : 0;
	//CHECK_DELAYSLOT ( i.Rt );
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rt )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //SLTI );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				// this should zero-extend to 64-bits
				e->XorRegReg32 ( RAX, RAX );
				//e->CmpMemImm32 ( &r->GPR [ i.Rs ].s, i.sImmediate );
				e->CmpMemImm64 ( &r->GPR [ i.Rs ].s, i.sImmediate );
				e->Set_L ( RAX );
				//ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].u, RAX );
				ret = e->MovMemReg64 ( &r->GPR [ i.Rt ].u, RAX );
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SLTIU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SLTIU";
	static const void *c_vFunction = R5900::Instruction::Execute::SLTIU;
	
	//r->GPR [ i.Rt ].u = r->GPR [ i.Rs ].u < ((u32) ((s32) i.sImmediate)) ? 1 : 0;
	//CHECK_DELAYSLOT ( i.Rt );
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rt )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //SLTIU );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				// this should zero-extend to 64-bits
				e->XorRegReg32 ( RAX, RAX );
				//e->CmpMemImm32 ( &r->GPR [ i.Rs ].s, i.sImmediate );
				e->CmpMemImm64 ( &r->GPR [ i.Rs ].s, i.sImmediate );
				e->Set_B ( RAX );
				//ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].u, RAX );
				ret = e->MovMemReg64 ( &r->GPR [ i.Rt ].u, RAX );
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::LUI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "LUI";
	static const void *c_vFunction = R5900::Instruction::Execute::LUI;
	
	//r->GPR [ i.Rt ].u = ( i.uImmediate << 16 );
	//CHECK_DELAYSLOT ( i.Rt );
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rt )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //LUI );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				//ret = e->MovMemImm32 ( &r->GPR [ i.Rt ].u, ( i.uImmediate << 16 ) );
				ret = e->MovMemImm64 ( &r->GPR [ i.Rt ].u, ( i.uImmediate << 16 ) );
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LUI instruction.\n";
		return -1;
	}
	return 1;
}







//////////////////////////////////////////////////////////
// Shift instructions



long Recompiler::SLL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SLL";
	static const void *c_vFunction = R5900::Instruction::Execute::SLL;
	
	//r->GPR [ i.Rd ].u = ( r->GPR [ i.Rt ].u << i.Shift );
	//CHECK_DELAYSLOT ( i.Rd );
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //SLL );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].uw0 );
				e->ShlRegImm32 ( RAX, (u32) i.Shift );
				e->Cdqe ();
				//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SLL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SRL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SRL";
	static const void *c_vFunction = R5900::Instruction::Execute::SRL;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //SRL );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].uw0 );
				e->ShrRegImm32 ( RAX, (u32) i.Shift );
				e->Cdqe ();
				//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SRA ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SRA";
	static const void *c_vFunction = R5900::Instruction::Execute::SRA;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //SRA );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].u );
				e->MovsxdReg64Mem32 ( RAX, &r->GPR [ i.Rt ].uw0 );
				//e->SarRegImm32 ( RAX, (u32) i.Shift );
				e->SarRegImm64 ( RAX, (u32) i.Shift );
				//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SLLV ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SLLV";
	static const void *c_vFunction = R5900::Instruction::Execute::SLLV;
	
	//r->GPR [ i.Rd ].u = ( r->GPR [ i.Rt ].u << ( r->GPR [ i.Rs ].u & 0x1f ) );
	//CHECK_DELAYSLOT ( i.Rd );
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //SLLV );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].u );
				e->MovRegMem32 ( RAX, &r->GPR [ i.Rt ].uw0 );
				//e->MovRegFromMem32 ( RCX, &r->GPR [ i.Rs ].u );
				e->MovRegMem32 ( RCX, &r->GPR [ i.Rs ].uw0 );
				e->ShlRegReg32 ( RAX );
				e->Cdqe ();
				//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SRLV ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SRLV";
	static const void *c_vFunction = R5900::Instruction::Execute::SRLV;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //SRLV );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].uw0 );
				e->MovRegFromMem32 ( RCX, &r->GPR [ i.Rs ].uw0 );
				e->ShrRegReg32 ( RAX );
				e->Cdqe ();
				//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SRAV ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SRAV";
	static const void *c_vFunction = R5900::Instruction::Execute::SRAV;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( c_vFunction ); //SRAV );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].u );
				e->MovsxdReg64Mem32 ( RAX, &r->GPR [ i.Rt ].uw0 );
				e->MovRegFromMem32 ( RCX, &r->GPR [ i.Rs ].uw0 );
				//e->SarRegReg32 ( RAX );
				e->SarRegReg64 ( RAX );
				//e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}


//----------------------------------------------------------------------------


////////////////////////////////////////////
// Jump/Branch Instructions



long Recompiler::J ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "J";
	static const void *c_vFunction = R5900::Instruction::Execute::J;
	
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.cb = r->_cb_Jump;
	//r->Status.DelaySlot_Valid |= 0x1;
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			//bStopEncodingBefore = true;

			// *MUST* update PC if branch will return to delay slot (at least for now)
			e->MovMemImm32 ( & r->PC, Address );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //J );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_J_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPJ> );
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //J instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::JR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "JR";
	static const void *c_vFunction = R5900::Instruction::Execute::JR;
	
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.cb = r->_cb_JumpRegister;
	//r->DelaySlot0.Data = r->GPR [ i.Rs ].u & ~3;
	//r->Status.DelaySlot_Valid |= 0x1;
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// has to stop encoding before AND after here due to the posible synchronous interrupt
			// so it has to have PC,LastPC updated (or could set it and move up the entry point)
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //JR );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_JR_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPJR> );
#else
			return -1;
#endif

			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//e->MovRegFromMem32 ( 0, &r->GPR [ i.Rs ].u );
			//e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			//ret = e->OrMemImm64 ( &r->Status.Value, 1 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //JR instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::JAL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "JAL";
	static const void *c_vFunction = R5900::Instruction::Execute::JAL;
	
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.cb = r->_cb_Jump;
	//r->Status.DelaySlot_Valid |= 0x1;
	//r->GPR [ 31 ].u = r->PC + 8;
	//CHECK_DELAYSLOT ( 31 );
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// there is no synchronous interrupt possible here, so only needs to stop encoding after
			bStopEncodingAfter = true;
			
			// also have to stop before because need an updated NextPC so you get the correct link value
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //JAL );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			break;
			
		case 1:
#ifdef USE_NEW_JAL_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPJAL> );
#else
			return -1;
#endif

			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->MovMemImm32 ( &r->GPR [ 31 ].u, Address + 8 );
			//ret = e->MovMemImm32 ( &r->LastModifiedRegister, 31 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //JAL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::JALR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "JALR";
	static const void *c_vFunction = R5900::Instruction::Execute::JALR;
	
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.Data = r->GPR [ i.Rs ].u & ~3;
	//r->DelaySlot0.cb = r->_cb_JumpRegister;
	//r->Status.DelaySlot_Valid |= 0x1;
	//r->GPR [ i.Rd ].u = r->PC + 8;
	//CHECK_DELAYSLOT ( i.Rd );
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// has to stop encoding before AND after here due to the posible synchronous interrupt
			// so it has to have PC,LastPC updated (or could set it and move up the entry point)
			bStopEncodingBefore = true;
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //JALR );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// make sure Rd is not r0
			if ( !i.Rd )
			{
				// make sure r0 stays at zero
				e->MovMemImm64 ( &r->GPR [ 0 ].u, 0 );
			}
			break;
			
		case 1:
#ifdef USE_NEW_JALR_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPJALR> );
#else
			return -1;
#endif
			
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//e->MovRegFromMem32 ( 0, &r->GPR [ i.Rs ].u );
			//e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			//ret = e->OrMemImm64 ( &r->Status.Value, 1 );
			//if ( i.Rd )
			//{
			//	e->MovMemImm32 ( &r->GPR [ i.Rd ].u, Address + 8 );
			//	ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			//}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //JALR instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BEQ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BEQ";
	static const void *c_vFunction = R5900::Instruction::Execute::BEQ;
	
	//if ( r->GPR [ i.Rs ].u == r->GPR [ i.Rt ].u )
	//{
	//	r->DelaySlot0.Instruction = i;
	//	r->DelaySlot0.cb = r->_cb_Branch;
	//	r->Status.DelaySlot_Valid |= 0x1;
	//}
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			bStopEncodingAfter = true;

			// *MUST* update PC if branch will return to delay slot (at least for now)
			e->MovMemImm32 ( & r->PC, Address );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //BEQ );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BEQ_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBEQ> );
#else
			return -1;
#endif
			
			//e->MovRegFromMem32 ( 0, &r->GPR [ i.Rs ].u );
			//e->CmpRegMem32 ( 0, &r->GPR [ i.Rt ].u );
			//e->Jmp8_NE ( 0, 0 );
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//ret = e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->SetJmpTarget8 ( 0 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BEQ instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BNE ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BNE";
	static const void *c_vFunction = R5900::Instruction::Execute::BNE;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			bStopEncodingAfter = true;
			
			// *MUST* update PC if branch will return to delay slot (at least for now)
			e->MovMemImm32 ( & r->PC, Address );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //BNE );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BNE_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBNE> );
#else
			return -1;
#endif
			
			//e->MovRegFromMem32 ( 0, &r->GPR [ i.Rs ].u );
			//e->CmpRegMem32 ( 0, &r->GPR [ i.Rt ].u );
			//e->Jmp8_E ( 0, 0 );
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//ret = e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->SetJmpTarget8 ( 0 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BNE instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BLEZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BLEZ";
	static const void *c_vFunction = R5900::Instruction::Execute::BLEZ;
	
	//if ( r->GPR [ i.Rs ].s <= 0 )
	//{
	//	// next instruction is in the branch delay slot
	//	r->DelaySlot0.Instruction = i;
	//	r->DelaySlot0.cb = r->_cb_Branch;
	//	r->Status.DelaySlot_Valid |= 0x1;
	//}
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			bStopEncodingAfter = true;
			
			// *MUST* update PC if branch will return to delay slot (at least for now)
			e->MovMemImm32 ( & r->PC, Address );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //BLEZ );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BLEZ_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBLEZ> );
#else
			return -1;
#endif
			
			//e->CmpMemImm32 ( &r->GPR [ i.Rs ].u, 0 );
			//e->Jmp8_G ( 0, 0 );
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//ret = e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->SetJmpTarget8 ( 0 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BLEZ instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BGTZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BGTZ";
	static const void *c_vFunction = R5900::Instruction::Execute::BGTZ;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			bStopEncodingAfter = true;
			
			// *MUST* update PC if branch will return to delay slot (at least for now)
			e->MovMemImm32 ( & r->PC, Address );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //BGTZ );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BGTZ_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBGTZ> );
#else
			return -1;
#endif
			
			//e->CmpMemImm32 ( &r->GPR [ i.Rs ].u, 0 );
			//e->Jmp8_LE ( 0, 0 );
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//ret = e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->SetJmpTarget8 ( 0 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BGTZ instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BLTZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BLTZ";
	static const void *c_vFunction = R5900::Instruction::Execute::BLTZ;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	
	// *testing*
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			bStopEncodingAfter = true;
			
			// *MUST* update PC if branch will return to delay slot (at least for now)
			e->MovMemImm32 ( & r->PC, Address );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //BLTZ );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BLTZ_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBLTZ> );
#else
			return -1;
#endif
			
			//e->CmpMemImm32 ( &r->GPR [ i.Rs ].u, 0 );
			//e->Jmp8_GE ( 0, 0 );
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//ret = e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->SetJmpTarget8 ( 0 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BLTZ instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BGEZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BGEZ";
	static const void *c_vFunction = R5900::Instruction::Execute::BGEZ;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			bStopEncodingAfter = true;
			
			// *MUST* update PC if branch will return to delay slot (at least for now)
			e->MovMemImm32 ( & r->PC, Address );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //BGEZ );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BGEZ_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBGEZ> );
#else
			return -1;
#endif
			
			//e->CmpMemImm32 ( &r->GPR [ i.Rs ].u, 0 );
			//e->Jmp8_L ( 0, 0 );
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//ret = e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->SetJmpTarget8 ( 0 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BGEZ instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BLTZAL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BLTZAL";
	static const void *c_vFunction = R5900::Instruction::Execute::BLTZAL;
	
	//if ( r->GPR [ i.Rs ].s < 0 )
	//{
	//	r->DelaySlot0.Instruction = i;
	//	r->DelaySlot0.cb = r->_cb_Branch;
	//	r->Status.DelaySlot_Valid |= 0x1;
	//}
	//r->GPR [ 31 ].u = r->PC + 8;
	//CHECK_DELAYSLOT ( 31 );
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// needs to also stop encoding before at level 0, because it requires an updated PC to link
			bStopEncodingBefore = true;
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //BLTZAL );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BLTZAL_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBLTZAL> );
#else
			return -1;
#endif
			
			//e->CmpMemImm32 ( &r->GPR [ i.Rs ].u, 0 );
			//e->Jmp8_GE ( 0, 0 );
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->SetJmpTarget8 ( 0 );
			//e->MovMemImm32 ( &r->GPR [ 31 ].u, Address + 8 );
			//ret = e->MovMemImm32 ( &r->LastModifiedRegister, 31 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BLTZAL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BGEZAL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BGEZAL";
	static const void *c_vFunction = R5900::Instruction::Execute::BGEZAL;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// needs to also stop encoding before at level 0, because it requires an updated PC to link
			bStopEncodingBefore = true;
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //BGEZAL );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BGEZAL_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBGEZAL> );
#else
			return -1;
#endif
			
			//e->CmpMemImm32 ( &r->GPR [ i.Rs ].u, 0 );
			//e->Jmp8_L ( 0, 0 );
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->SetJmpTarget8 ( 0 );
			//e->MovMemImm32 ( &r->GPR [ 31 ].u, Address + 8 );
			//ret = e->MovMemImm32 ( &r->LastModifiedRegister, 31 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BGEZAL instruction.\n";
		return -1;
	}
	return 1;
}


/////////////////////////////////////////////////////////////
// Multiply/Divide Instructions

long Recompiler::MULT ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MULT";
	static const void *c_vFunction = R5900::Instruction::Execute::MULT;
	
	/*
	// if rs is between -0x800 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff or between -0x7ff and -0x100000, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	*/
	
	// constant multiply cycles for PS2
	static const int c_iMultiplyCycles = 4 / 2;
	
	//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	//{
	//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	//}
	//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Slow;
	//if ( r->GPR [ i.Rs ].s < 0x800 && r->GPR [ i.Rs ].s >= -0x800 )
	//{
	//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Fast;
	//}
	//else if ( r->GPR [ i.Rs ].s < 0x100000 && r->GPR [ i.Rs ].s >= -0x100000 )
	//{
	//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Med;
	//}
	// multiply signed Lo,Hi = rs * rt
	//r->HiLo.sValue = ((s64) (r->GPR [ i.Rs ].s)) * ((s64) (r->GPR [ i.Rt ].s));
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// need to stop encoding before at level 0 to get an updated CycleCount
			// need to stop encoding after at level 0 because it updates CycleCount
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //MULT );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_MULT_CODE
			//bResetCycleCount = true;
			
			// calculate cycles mul/div unit will be busy for
			//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Slow;
			//if ( r->GPR [ i.Rs ].s < 0x800 && r->GPR [ i.Rs ].s >= -0x800 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Fast;
			//}
			//else if ( r->GPR [ i.Rs ].s < 0x100000 && r->GPR [ i.Rs ].s >= -0x100000 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Med;
			//}
			/*
			e->MovRegMem32 ( RCX, & r->GPR [ i.Rs ].sw0 );
			e->MovRegReg32 ( RAX, RCX );
			e->XorReg32ImmX ( RAX, -1 );
			e->CmovSRegReg32 ( RAX, RCX );
			e->MovReg32ImmX ( RDX, c_iMultiplyCycles_Slow );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Med );
			e->CmpReg32ImmX ( RAX, 0x100000 );
			e->CmovBRegReg32 ( RDX, RCX );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Fast );
			e->CmpReg32ImmX ( RAX, 0x800 );
			e->CmovBRegReg32 ( RDX, RCX );
			*/
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			e->MovRegReg64 ( RCX, RAX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			//e->SubRegReg64 ( RAX, RCX );
			//e->CmovBRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddRegReg64 ( RDX, RAX );
			e->SubRegReg64 ( RCX, RDX );
			
#ifdef ENABLE_MULTIPLY_LATENCY
			// add in the latency for multiply
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
#endif

			// write back the new busy until cycle
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			
			
			// do the multiply
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].sw0 );
			e->ImulRegMem32 ( & r->GPR [ i.Rt ].sw0 );
			// *** TODO FOR PS2 *** //
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->Cdqe ();
			e->MovMemReg64 ( & r->LO.s, RAX );
			//ret = e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->MovsxdReg64Reg32 ( RDX, RDX );
			ret = e->MovMemReg64 ( & r->HI.s, RDX );
			
			// *note*: the R5900 additionally writes to a destination register
			if ( i.Rd )
			{
				e->MovMemReg64 ( & r->GPR [ i.Rd ].s, RAX );
			}
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MULT instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::MULTU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MULTU";
	static const void *c_vFunction = R5900::Instruction::Execute::MULTU;
	
	// if rs is between 0 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
	// constant multiply cycles for PS2
	static const int c_iMultiplyCycles = 4 / 2;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// need to stop encoding before at level 0 to get an updated CycleCount
			// need to stop encoding after at level 0 because it updates CycleCount
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //MULTU );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_MULTU_CODE
			//bResetCycleCount = true;
			
			//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Slow;
			//if ( r->GPR [ i.Rs ].u < 0x800 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Fast;
			//}
			//else if ( r->GPR [ i.Rs ].u < 0x100000 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Med;
			//}
			/*
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].sw0 );
			e->MovReg32ImmX ( RDX, c_iMultiplyCycles_Slow );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Med );
			e->CmpReg32ImmX ( RAX, 0x100000 );
			e->CmovBRegReg32 ( RDX, RCX );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Fast );
			e->CmpReg32ImmX ( RAX, 0x800 );
			e->CmovBRegReg32 ( RDX, RCX );
			*/
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			e->MovRegReg64 ( RCX, RAX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			//e->SubRegReg64 ( RAX, RCX );
			//e->CmovBRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddRegReg64 ( RDX, RAX );
			e->SubRegReg64 ( RCX, RDX );
			
#ifdef ENABLE_MULTIPLY_LATENCY
			// add in the latency for multiply
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
#endif

			// write back the new busy until cycle
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			
			// do the multiply
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].sw0 );
			e->MulRegMem32 ( & r->GPR [ i.Rt ].sw0 );
			// *** todo for ps2 *** //
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->Cdqe ();
			e->MovMemReg64 ( & r->LO.s, RAX );
			//e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->MovsxdReg64Reg32 ( RDX, RDX );
			e->MovMemReg64 ( & r->HI.s, RDX );
			
			// *note*: the R5900 additionally writes to a destination register
			if ( i.Rd )
			{
				e->MovMemReg64 ( & r->GPR [ i.Rd ].s, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MULTU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::DIV ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DIV";
	static const void *c_vFunction = R5900::Instruction::Execute::DIV;
	
	static const int c_iDivideCycles = 36 / 2;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //DIV );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
			
		case 1:
#ifdef USE_NEW_DIV_CODE
			//bResetCycleCount = true;
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			e->MovRegReg64 ( RCX, RAX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			//e->SubRegReg64 ( RAX, RCX );
			//e->CmovBRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddRegReg64 ( RDX, RAX );
			e->SubRegReg64 ( RCX, RDX );
			
#ifdef ENABLE_DIVIDE_LATENCY
			// add in the latency for multiply
			e->AddReg64ImmX ( RCX, c_iDivideCycles );
#endif

			// write back the new busy until cycle
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			
			
			// mult/div unit is busy now
			//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iDivideCycles;
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddReg64ImmX ( RAX, c_iDivideCycles );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RAX );
			
			// divide signed: Lo = rs / rt; Hi = rs % rt
			//if ( r->GPR [ i.Rt ].u != 0 )
			//{
			//	// if rs = 0x80000000 and rt = -1 then hi = 0 and lo = 0x80000000
			//	if ( r->GPR [ i.Rs ].u == 0x80000000 && r->GPR [ i.Rt ].s == -1 )
			//	{
			//		r->HiLo.uHi = 0;
			//		r->HiLo.uLo = 0x80000000;
			//	}
			//	else
			//	{
			//		r->HiLo.sLo = r->GPR [ i.Rs ].s / r->GPR [ i.Rt ].s;
			//		r->HiLo.sHi = r->GPR [ i.Rs ].s % r->GPR [ i.Rt ].s;
			//	}
			//}
			//else
			//{
			//	if ( r->GPR [ i.Rs ].s < 0 )
			//	{
			//		r->HiLo.sLo = 1;
			//	}
			//	else
			//	{
			//		r->HiLo.sLo = -1;
			//	}
			//	r->HiLo.uHi = r->GPR [ i.Rs ].u;
			//}
			//e->MovRegMem32 ( RCX, & r->GPR [ i.Rt ].u );
			//e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].u );
			e->MovsxdReg64Mem32 ( RCX, & r->GPR [ i.Rt ].sw0 );
			e->MovsxdReg64Mem32 ( RAX, & r->GPR [ i.Rs ].sw0 );
			//e->OrRegReg64 ( RCX, RCX );
			//e->Jmp8_E ( 0, 0 );
			e->Jmp8_ECXZ ( 0, 0 );
			
			//e->MovRegReg64 ( RDX, RAX );
			//e->SarRegImm64 ( RDX, 63 );
			e->Cqo ();
			e->IdivRegReg64 ( RCX );
			// *** todo for ps2 *** //
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->Cdqe ();
			e->MovMemReg64 ( & r->LO.s, RAX );
			//e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->MovsxdReg64Reg32 ( RDX, RDX );
			e->MovMemReg64 ( & r->HI.s, RDX );
			e->Jmp8 ( 0, 1 );
			
			e->SetJmpTarget8 ( 0 );
			
			//e->MovReg64ImmX ( RCX, -1 );
			//e->MovReg64ImmX ( RDX, 1 );
			//e->OrRegReg32 ( RAX, RAX );
			//e->CmovSRegReg64 ( RCX, RDX );
			e->Cqo ();
			e->NotReg64 ( RDX );
			e->OrReg64ImmX ( RDX, 1 );
			
			// *** todo for ps2 *** //
			//e->MovMemReg32 ( & r->HiLo.uHi, RAX );
			e->MovMemReg64 ( & r->HI.u, RAX );
			//e->MovMemReg32 ( & r->HiLo.uLo, RCX );
			e->MovMemReg64 ( & r->LO.u, RDX );
			
			e->SetJmpTarget8 ( 1 );
			
			// done //
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //DIV instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::DIVU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DIVU";
	static const void *c_vFunction = R5900::Instruction::Execute::DIVU;
	
	static const int c_iDivideCycles = 36 / 2;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			// actually needs to stop encoding before for now, because the correct current cycle count is needed
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //DIVU );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_DIVU_CODE
			//bResetCycleCount = true;
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			e->MovRegReg64 ( RCX, RAX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			//e->SubRegReg64 ( RAX, RCX );
			//e->CmovBRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddRegReg64 ( RDX, RAX );
			e->SubRegReg64 ( RCX, RDX );
			
#ifdef ENABLE_DIVIDE_LATENCY
			// add in the latency for multiply
			e->AddReg64ImmX ( RCX, c_iDivideCycles );
#endif

			// write back the new busy until cycle
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			
			// mult/div unit is busy now
			//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iDivideCycles;
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddReg64ImmX ( RAX, c_iDivideCycles );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RAX );
			
			// divide unsigned: Lo = rs / rt; Hi = rs % rt
			//if ( r->GPR [ i.Rt ].u != 0 )
			//{
			//	r->HiLo.uLo = r->GPR [ i.Rs ].u / r->GPR [ i.Rt ].u;
			//	r->HiLo.uHi = r->GPR [ i.Rs ].u % r->GPR [ i.Rt ].u;
			//}
			//else
			//{
			//	r->HiLo.sLo = -1;
			//	r->HiLo.uHi = r->GPR [ i.Rs ].u;
			//}
			e->MovRegMem32 ( RCX, & r->GPR [ i.Rt ].uw0 );
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].uw0 );
			//e->OrRegReg64 ( RCX, RCX );
			//e->Jmp8_E ( 0, 0 );
			e->Jmp8_ECXZ ( 0, 0 );
			
			e->XorRegReg32 ( RDX, RDX );
			e->DivRegReg32 ( RCX );
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->Cdqe ();
			e->MovMemReg64 ( & r->LO.s, RAX );
			//e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->MovsxdReg64Reg32 ( RDX, RDX );
			e->MovMemReg64 ( & r->HI.s, RDX );
			e->Jmp8 ( 0, 1 );
			
			e->SetJmpTarget8 ( 0 );
			
			//e->MovMemImm32 ( & r->HiLo.sLo, -1 );
			//e->MovMemReg32 ( & r->HiLo.uHi, RAX );
			e->MovMemImm64 ( & r->LO.s, -1 );
			e->MovMemReg64 ( & r->HI.u, RAX );
			
			e->SetJmpTarget8 ( 1 );
			
			// done //
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //DIVU instruction.\n";
		return -1;
	}
	return 1;
}



long Recompiler::MFHI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MFHI";
	static const void *c_vFunction = R5900::Instruction::Execute::MFHI;
	
	//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	//{
	//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	//}
	//r->GPR [ i.Rd ].u = r->HiLo.uHi;
	//CHECK_DELAYSLOT ( i.Rd );
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction (can update CycleCount)
			bStopEncodingAfter = true;
			
			// for now, stop encoding before, because an updated CycleCount is needed to determine if Mul/Div is done
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //MFHI );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// make sure Rd is not r0
			if ( !i.Rd )
			{
				// make sure r0 stays at zero
				ret = e->MovMemImm64 ( &r->GPR [ 0 ].u, 0 );
			}
			break;
			
		case 1:
#ifdef USE_NEW_MFHI_CODE
			/*
			e->MovRegFromMem64 ( RAX, &r->CycleCount );
			e->CmpRegMem64 ( RAX, &r->MulDiv_BusyUntil_Cycle );
			e->CmovBRegMem64 ( RAX, &r->MulDiv_BusyUntil_Cycle );
			ret = e->MovRegToMem64 ( &r->CycleCount, RAX );
			if ( i.Rd )
			{
				e->MovRegFromMem32 ( RAX, &r->HiLo.uHi );
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RAX );
			}
			Local_LastModifiedReg = i.Rd;
			*/
			
			// this instruction interlocks if multiply/divide unit is busy
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			//e->XorRegReg64 ( RDX, RDX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle );
			//e->CmovBRegReg64 ( RDX, RAX );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			// move from Hi register
			//r->GPR [ i.Rd ].u = r->HiLo.uHi;
			//CHECK_DELAYSLOT ( i.Rd );
			if ( i.Rd )
			{
				//e->MovRegMem32 ( RAX, & r->HiLo.uHi );
				e->MovRegMem64 ( RAX, & r->HI.u );
				//e->MovMemReg32 ( & r->GPR [ i.Rd ].u, RAX );
				e->MovMemReg64 ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MFHI instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::MFLO ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MFLO";
	static const void *c_vFunction = R5900::Instruction::Execute::MFLO;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction (can update CycleCount)
			bStopEncodingAfter = true;
			
			// for now, stop encoding before, because an updated CycleCount is needed to determine if Mul/Div is done
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //MFLO );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// make sure Rd is not r0
			if ( !i.Rd )
			{
				// make sure r0 stays at zero
				ret = e->MovMemImm64 ( &r->GPR [ 0 ].u, 0 );
			}
			break;
			
		case 1:
#ifdef USE_NEW_MFLO_CODE
			/*
			e->MovRegFromMem64 ( RAX, &r->CycleCount );
			e->CmpRegMem64 ( RAX, &r->MulDiv_BusyUntil_Cycle );
			e->CmovBRegMem64 ( RAX, &r->MulDiv_BusyUntil_Cycle );
			ret = e->MovRegToMem64 ( &r->CycleCount, RAX );
			if ( i.Rd )
			{
				e->MovRegFromMem32 ( RAX, &r->HiLo.uLo );
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RAX );
			}
			Local_LastModifiedReg = i.Rd;
			*/
			
			// this instruction interlocks if multiply/divide unit is busy
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			//e->XorRegReg64 ( RDX, RDX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle );
			//e->CmovBRegReg64 ( RDX, RAX );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			// move from Lo register
			//r->GPR [ i.Rd ].u = r->HiLo.uLo;
			//CHECK_DELAYSLOT ( i.Rd );
			if ( i.Rd )
			{
				//e->MovRegMem32 ( RAX, & r->HiLo.uLo );
				e->MovRegMem64 ( RAX, & r->LO.u );
				//e->MovMemReg32 ( & r->GPR [ i.Rd ].u, RAX );
				e->MovMemReg64 ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MFLO instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::MTHI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MTHI";
	static const void *c_vFunction = R5900::Instruction::Execute::MTHI;
	
	//r->HiLo.uHi = r->GPR [ i.Rs ].u;
	
	// ***TODO*** should this sync with mul/div unit??
	
	int ret = 1;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //MTHI );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].uw0 );
			e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].u );
			//ret = e->MovRegToMem32 ( &r->HiLo.uHi, RAX );
			ret = e->MovMemReg64 ( &r->HI.u, RAX );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MTHI instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::MTLO ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MTLO";
	static const void *c_vFunction = R5900::Instruction::Execute::MTLO;
	
	//r->HiLo.uLo = r->GPR [ i.Rs ].u;
	
	// ***TODO*** should this sync with mul/div unit??
	
	int ret = 1;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //MTLO );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].uw0 );
			e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].u );
			//ret = e->MovRegToMem32 ( &r->HiLo.uLo, RAX );
			ret = e->MovMemReg64 ( &r->LO.u, RAX );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MTLO instruction.\n";
		return -1;
	}
	return 1;
}








////////////////////////////////////////////////////////
// Instructions that can cause Synchronous Interrupts //
////////////////////////////////////////////////////////


long Recompiler::ADD ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "ADD";
	static const void *c_vFunction = R5900::Instruction::Execute::ADD;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// go ahead and say that it modifies NextPC since it might, even if it does not this time around
			// but only if instruction is actually encoded
			Local_NextPCModified = true;
			
			// need to stop encoding before, because if it sync ints, that requires PC to be updated
			bStopEncodingBefore = true;
			
			// need to stop encoding after because if it sync ints, then it needs to "jump"
			bStopEncodingAfter = true;
			
			// update NextPC before executing instruction at level 0 since it may do a sync int
			e->MovMemImm32 ( &r->NextPC, Address + 4 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //ADD );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			if ( !i.Rd )
			{
				// make sure r0 stays at zero
				ret = e->MovMemImm64 ( &r->GPR [ 0 ].u, 0 );
			}
			
			break;
			
		case 1:
#ifdef USE_NEW_ADD_CODE
			e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].sw0 );
			e->AddRegMem32 ( RAX, &r->GPR [ i.Rt ].uw0 );
			
			// branch if not signed overflow
			e->Jmp8_NO ( 0, 0 );
			
			// update CycleCount, set PC, then jump to synchronous interrupt
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount + MemCycles );
			
			// set pc
			e->MovMemImm32 ( & r->PC, Address );
			
			//r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
			e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_OV> );
			
			// continue processing store from here //
			e->SetJmpTarget8 ( 0 );
			
			// check if destination is r0
			if ( i.Rd )
			{
				// store result if not signed overflow
				e->Cdqe ();
				ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
			}
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADD instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::ADDI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "ADDI";
	static const void *c_vFunction = R5900::Instruction::Execute::ADDI;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// go ahead and say that it modifies NextPC since it might, even if it does not this time around
			// but only if instruction is actually encoded
			Local_NextPCModified = true;
			
			// need to stop encoding before, because if it sync ints, that requires PC to be updated
			bStopEncodingBefore = true;
			
			// need to stop encoding after because if it sync ints, then it needs to "jump"
			bStopEncodingAfter = true;
			
			// update NextPC before executing instruction at level 0 since it may do a sync int
			e->MovMemImm32 ( &r->NextPC, Address + 4 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //ADDI );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			if ( !i.Rt )
			{
				// make sure r0 stays at zero
				ret = e->MovMemImm64 ( &r->GPR [ 0 ].u, 0 );
			}
			
			break;
			
		case 1:
#ifdef USE_NEW_ADDI_CODE
			e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].sw0 );
			//e->AddRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
			e->AddReg32ImmX ( RAX, i.sImmediate );
			
			// branch if not signed overflow
			e->Jmp8_NO ( 0, 0 );
			
			// update CycleCount, set PC, then jump to synchronous interrupt
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount + MemCycles );
			
			// set pc
			e->MovMemImm32 ( & r->PC, Address );
			
			//r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
			e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_OV> );
			
			// continue processing store from here //
			e->SetJmpTarget8 ( 0 );
			
			// check if destination is r0
			if ( i.Rt )
			{
				// store result if not signed overflow
				e->Cdqe ();
				ret = e->MovMemReg64 ( &r->GPR [ i.Rt ].s, RAX );
			}
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDI instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SUB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SUB";
	static const void *c_vFunction = R5900::Instruction::Execute::SUB;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// go ahead and say that it modifies NextPC since it might, even if it does not this time around
			// but only if instruction is actually encoded
			Local_NextPCModified = true;
			
			// need to stop encoding before, because if it sync ints, that requires PC to be updated
			bStopEncodingBefore = true;
			
			// need to stop encoding after because if it sync ints, then it needs to "jump"
			bStopEncodingAfter = true;
			
			// update NextPC before executing instruction at level 0 since it may do a sync int
			e->MovMemImm32 ( &r->NextPC, Address + 4 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //SUB );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			if ( !i.Rd )
			{
				// make sure r0 stays at zero
				ret = e->MovMemImm64 ( &r->GPR [ 0 ].u, 0 );
			}
			break;
			
		case 1:
#ifdef USE_NEW_SUB_CODE
			e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].sw0 );
			e->SubRegMem32 ( RAX, &r->GPR [ i.Rt ].uw0 );
			
			// branch if not signed overflow
			e->Jmp8_NO ( 0, 0 );
			
			// update CycleCount, set PC, then jump to synchronous interrupt
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount + MemCycles );
			
			// set pc
			e->MovMemImm32 ( & r->PC, Address );
			
			//r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
			e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_OV> );
			
			// continue processing store from here //
			e->SetJmpTarget8 ( 0 );
			
			// check if destination is r0
			if ( i.Rd )
			{
				// store result if not signed overflow
				e->Cdqe ();
				ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
			}
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SUB instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::SYSCALL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SYSCALL";
	static const void *c_vFunction = R5900::Instruction::Execute::SYSCALL;
	
	int ret = 1;
	
	// stop encoding after since it is an unconditional synchronous interrupt
	bStopEncodingAfter = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	
	switch ( OpLevel )
	{
		case 0:
			
			// stop encoding before due to synchronous interrupt (needs updated PC,LastPC)
			bStopEncodingBefore = true;
			
			// update NextPC before executing instruction at level 0 since it may do a sync int
			// note: no need for this since it is an unconditional sync int and stopped encoding before
			//e->MovMemImm32 ( &r->NextPC, Address + 4 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //SYSCALL );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			
#ifdef USE_NEW_SYSCALL_CODE
			// update CycleCount, set PC, then jump to synchronous interrupt
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount + MemCycles );
			
			// set pc
			e->MovMemImm32 ( & r->PC, Address );
			
			//r->ProcessSynchronousInterrupt ( Cpu::EXC_SYSCALL );
			e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_SYSCALL> );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SYSCALL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BREAK ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BREAK";
	static const void *c_vFunction = R5900::Instruction::Execute::BREAK;
	
	int ret = 1;
	
	// stop encoding after since it is an unconditional synchronous interrupt
	bStopEncodingAfter = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			// also need to stop encoding before, because it needs both PC and LastPC updated first
			bStopEncodingBefore = true;
			
			// update NextPC before executing instruction at level 0 since it may do a sync int
			// note: no need for this since it is an unconditional sync int
			//e->MovMemImm32 ( &r->NextPC, Address + 4 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //BREAK );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BREAK instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::Invalid ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "Invalid";
	static const void *c_vFunction = R5900::Instruction::Execute::Invalid;
	
	int ret = 1;
	
	// stop encoding after since it is an unconditional synchronous interrupt
	bStopEncodingAfter = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			// also need to stop encoding before, because it needs both PC and LastPC updated first
			bStopEncodingBefore = true;
			
			// update NextPC before executing instruction at level 0 since it may do a sync int
			// note: no need for this since it is an unconditional sync int
			//e->MovMemImm32 ( &r->NextPC, Address + 4 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //Invalid );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //Invalid instruction.\n";
		return -1;
	}
	return 1;
}





long Recompiler::MFC0 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MFC0";
	static const void *c_vFunction = R5900::Instruction::Execute::MFC0;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// should put something in delay slot, so return after this instruction at level 0
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //MFC0 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			e->MovRegImm32 ( RCX, i.Rd );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->Call ( R5900::Cpu::Read_MFC0 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->Cdqe ();
			e->MovMemReg64 ( & r->GPR [ i.Rt ].u, RAX );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MFC0 instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::MTC0 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MTC0";
	static const void *c_vFunction = R5900::Instruction::Execute::MTC0;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// should put something in delay slot, so return after this instruction at level 0
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //MTC0 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			break;
			
		case 1:
			e->MovRegImm32 ( RCX, i.Rd );
			e->MovRegMem32 ( RDX, & r->GPR [ i.Rt ].sw0 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->Call ( R5900::Cpu::Write_MTC0 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MTC0 instruction.\n";
		return -1;
	}
	return 1;
}








long Recompiler::CFC2_I ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "CFC2_I";
	static const void *c_vFunction = R5900::Instruction::Execute::CFC2_I;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	
	// probably needs an updated CycleCount, so need to stop encoding before too for now
	bStopEncodingBefore = true;
	
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //CFC2 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //CFC2 instruction.\n";
		return -1;
	}
	return 1;
}



long Recompiler::CTC2_I ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "CTC2_I";
	static const void *c_vFunction = R5900::Instruction::Execute::CTC2_I;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	
	// probably needs an updated CycleCount, so need to stop encoding before too for now
	bStopEncodingBefore = true;
	
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //CTC2 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //CTC2 instruction.\n";
		return -1;
	}
	return 1;
}



long Recompiler::CFC2_NI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "CFC2_NI";
	static const void *c_vFunction = R5900::Instruction::Execute::CFC2_NI;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	
	// probably needs an updated CycleCount, so need to stop encoding before too for now
	bStopEncodingBefore = true;
	
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //CFC2 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //CFC2 instruction.\n";
		return -1;
	}
	return 1;
}



long Recompiler::CTC2_NI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "CTC2_NI";
	static const void *c_vFunction = R5900::Instruction::Execute::CTC2_NI;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	
	// probably needs an updated CycleCount, so need to stop encoding before too for now
	bStopEncodingBefore = true;
	
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //CTC2 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //CTC2 instruction.\n";
		return -1;
	}
	return 1;
}




// Load/Store - will need to use address translation to get physical addresses when needed

//////////////////////////////////////////////////////////////////////////
// store instructions

// store instructions
long Recompiler::SB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SB";
	static const void *c_vFunction = R5900::Instruction::Execute::SB;
	
	int ret = 1;
	
/*
	// for now, stop encoding after this instruction
	// should be able to stop before, but continue after
#ifndef ENABLE_STORE_PREFIX
	bStopEncodingBefore = true;
#endif
#ifndef ENABLE_STORE_SUFFIX
	bStopEncodingAfter = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// can just stop encoding before and ignore the prefix requirement at level 0 for testing
			bStopEncodingBefore = true;
		
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //SB );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_STORE_CODE
			ret = Generate_Normal_Store ( i, Address, 0x0, (void*) Playstation2::DataBus::Write_t<0xffULL> );
#else
			return -1;
#endif

			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SB instruction.\n";
		return -1;
	}
	return 1;
}





long Recompiler::SH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SH";
	static const void *c_vFunction = R5900::Instruction::Execute::SH;
	
	int ret = 1;
	
/*
	// for now, stop encoding after this instruction
#ifndef ENABLE_STORE_PREFIX
	bStopEncodingBefore = true;
#endif
#ifndef ENABLE_STORE_SUFFIX
	bStopEncodingAfter = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// can just stop encoding before and ignore the prefix requirement at level 0 for testing
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //SH );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;

			
		case 1:
#ifdef USE_NEW_STORE_CODE
			ret = Generate_Normal_Store ( i, Address, 0x1, (void*) Playstation2::DataBus::Write_t<0xffffULL> );
#else
			return -1;
#endif

			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SH instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SW";
	static const void *c_vFunction = R5900::Instruction::Execute::SW;
	
	int ret = 1;
	
/*
	// for now, stop encoding after this instruction
#ifndef ENABLE_STORE_PREFIX
	bStopEncodingBefore = true;
#endif
#ifndef ENABLE_STORE_SUFFIX
	bStopEncodingAfter = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// can just stop encoding before and ignore the prefix requirement at level 0 for testing
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //SW );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		
		case 1:
#ifdef USE_NEW_STORE_CODE
			ret = R5900::Recompiler::Generate_Normal_Store ( i, Address, 0x3, (void*) Playstation2::DataBus::Write_t<0xffffffffULL> );
#else
			return -1;
#endif

			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SW instruction.\n";
		return -1;
	}
	return 1;
}





long Recompiler::SWL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SWL";
	static const void *c_vFunction = R5900::Instruction::Execute::SWL;
	
	int ret = 1;
	
/*
	// for now, stop encoding after this instruction
#ifndef ENABLE_STORE_PREFIX
	bStopEncodingBefore = true;
#endif
#ifndef ENABLE_STORE_SUFFIX
	bStopEncodingAfter = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// can just stop encoding before and ignore the prefix requirement at level 0 for testing
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //SWL );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		
		case 1:
#ifdef USE_NEW_STORE_CODE_SWL
			ret = Generate_Normal_Store ( i, Address, 0x0, R5900::Instruction::Execute::SWL_Recompiler );
#else
			return -1;
#endif

			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SWL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SWR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SWR";
	static const void *c_vFunction = R5900::Instruction::Execute::SWR;
	
	int ret = 1;
	
/*
	// for now, stop encoding after this instruction
#ifndef ENABLE_STORE_PREFIX
	bStopEncodingBefore = true;
#endif
#ifndef ENABLE_STORE_SUFFIX
	bStopEncodingAfter = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// can just stop encoding before and ignore the prefix requirement at level 0 for testing
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //SWR );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
			
		case 1:
#ifdef USE_NEW_STORE_CODE_SWR
			ret = Generate_Normal_Store ( i, Address, 0x0, R5900::Instruction::Execute::SWR_Recompiler );
#else
			return -1;
#endif

			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SWR instruction.\n";
		return -1;
	}
	return 1;
}



/////////////////////////////////////////////////
// load instructions

// load instructions with delay slot
// *** todo *** it is also possible to this and just process load after load delay slot has executed - would still need previous load address before delay slot
// *** todo *** could also skip delay slot zero and put straight into delay slot 1 after next instruction, or just process load delay slot after next instruction
long Recompiler::LB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "LB";
	static const void *c_vFunction = R5900::Instruction::Execute::LB;
	
	//LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.Data = LoadAddress;
	//r->DelaySlot0.cb = LB_DelaySlot_Callback_Bus;
	//r->Status.DelaySlot_Valid |= 0x1;
	//r->LastModifiedRegister = 255;
	int ret = 1;

/*
#ifndef ENABLE_LOAD_SUFFIX
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //LB );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			Generate_Normal_Load ( i, Address, 0x0, (void*) Playstation2::DataBus::Read_t<0xff> );
			
			// store result //
			
			if ( i.Rt )
			{
				// sign-extend from byte to 64-bits ??
				e->Cbw ();
				e->Cwde ();
				e->Cdqe ();
				
				// store
				e->MovMemReg64 ( & r->GPR [ i.Rt ].sq0, RAX );
			}
#else
			return -1;
#endif
			
			/*
			e->MovRegImm32 ( 0, i.sOffset );
			e->AddRegMem32 ( 0, &r->GPR [ i.Base ].u );
			e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			e->OrMemImm64 ( &r->Status.Value, 1 );
			ret = e->MovMemImm32 ( &r->LastModifiedRegister, 255 );
			*/
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LB instruction.\n";
		return -1;
	}
	return 1;
}






long Recompiler::LH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "LH";
	static const void *c_vFunction = R5900::Instruction::Execute::LH;
	
	int ret = 1;

/*
#ifndef ENABLE_LOAD_SUFFIX
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //LH );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			Generate_Normal_Load ( i, Address, 0x1, (void*) Playstation2::DataBus::Read_t<0xffff> );
			
			// store result //
			
			if ( i.Rt )
			{
				// sign-extend from byte to 64-bits ??
				//e->Cbw ();
				e->Cwde ();
				e->Cdqe ();
				
				// store
				e->MovMemReg64 ( & r->GPR [ i.Rt ].sq0, RAX );
			}
#else
			return -1;
#endif
			
			/*
			e->MovRegImm32 ( 0, i.sOffset );
			e->AddRegMem32 ( 0, &r->GPR [ i.Base ].u );
			e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			e->OrMemImm64 ( &r->Status.Value, 1 );
			ret = e->MovMemImm32 ( &r->LastModifiedRegister, 255 );
			*/
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LB instruction.\n";
		return -1;
	}
	return 1;
}








long Recompiler::LW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "LW";
	static const void *c_vFunction = R5900::Instruction::Execute::LW;
	
	int ret = 1;
	
/*
#ifndef ENABLE_LOAD_SUFFIX
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //LW );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			Generate_Normal_Load ( i, Address, 0x3, (void*) Playstation2::DataBus::Read_t<0xffffffff> );
			
			// store result //
			
			if ( i.Rt )
			{
				// sign-extend from byte to 64-bits ??
				//e->Cbw ();
				//e->Cwde ();
				e->Cdqe ();
				
				// store
				e->MovMemReg64 ( & r->GPR [ i.Rt ].sq0, RAX );
			}
#else
			return -1;
#endif
			
			/*
			e->MovRegImm32 ( 0, i.sOffset );
			e->AddRegMem32 ( 0, &r->GPR [ i.Base ].u );
			e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			e->OrMemImm64 ( &r->Status.Value, 1 );
			ret = e->MovMemImm32 ( &r->LastModifiedRegister, 255 );
			*/
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LB instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::LBU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "LBU";
	static const void *c_vFunction = R5900::Instruction::Execute::LBU;
	
	int ret = 1;
	
/*
#ifndef ENABLE_LOAD_SUFFIX
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //LBU );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			ret = Generate_Normal_Load ( i, Address, 0x0, (void*) Playstation2::DataBus::Read_t<0xff> );
			
			// store result //
			
			if ( i.Rt )
			{
				// zero-extend 16-bit value to 64-bits
				e->AndReg32ImmX ( RAX, 0xff );
				
				// store
				e->MovMemReg64 ( & r->GPR [ i.Rt ].sq0, RAX );
			}
#else
			return -1;
#endif
			
			/*
			e->MovRegImm32 ( 0, i.sOffset );
			e->AddRegMem32 ( 0, &r->GPR [ i.Base ].u );
			e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			e->OrMemImm64 ( &r->Status.Value, 1 );
			ret = e->MovMemImm32 ( &r->LastModifiedRegister, 255 );
			*/
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LBU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::LHU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "LHU";
	static const void *c_vFunction = R5900::Instruction::Execute::LHU;
	
	int ret = 1;
	
/*
#ifndef ENABLE_LOAD_SUFFIX
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //LHU );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			Generate_Normal_Load ( i, Address, 0x1, (void*) Playstation2::DataBus::Read_t<0xffff> );
			
			// store result //
			
			if ( i.Rt )
			{
				// zero-extend 16-bit value to 64-bits
				e->AndReg32ImmX ( RAX, 0xffff );
				
				// store
				e->MovMemReg64 ( & r->GPR [ i.Rt ].sq0, RAX );
			}
#else
			return -1;
#endif
			
			/*
			e->MovRegImm32 ( 0, i.sOffset );
			e->AddRegMem32 ( 0, &r->GPR [ i.Base ].u );
			e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			e->OrMemImm64 ( &r->Status.Value, 1 );
			ret = e->MovMemImm32 ( &r->LastModifiedRegister, 255 );
			*/
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LB instruction.\n";
		return -1;
	}
	return 1;
}





// load instructions without load-delay slot
long Recompiler::LWL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "LWL";
	static const void *c_vFunction = R5900::Instruction::Execute::LWL;
	
	int ret = 1;
	
/*
#ifndef ENABLE_LOAD_SUFFIX
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //LWL );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE_LWL
			ret = Generate_Normal_Load ( i, Address, 0x0, (void*) Cpu::ProcessLoadDelaySlot_t<OPLWL,0> );
#else
			return -1;
#endif
			
			/*
			e->MovRegImm32 ( 0, i.sOffset );
			e->AddRegMem32 ( 0, &r->GPR [ i.Base ].u );
			e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			e->OrMemImm64 ( &r->Status.Value, 1 );
			ret = e->MovMemImm32 ( &r->LastModifiedRegister, 255 );
			*/
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LB instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::LWR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "LWR";
	static const void *c_vFunction = R5900::Instruction::Execute::LWR;
	
	int ret = 1;
	
/*
#ifndef ENABLE_LOAD_SUFFIX
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //LWR );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE_LWR
			ret = Generate_Normal_Load ( i, Address, 0x0, (void*) Cpu::ProcessLoadDelaySlot_t<OPLWR,0> );
#else
			return -1;
#endif
			
			/*
			e->MovRegImm32 ( 0, i.sOffset );
			e->AddRegMem32 ( 0, &r->GPR [ i.Base ].u );
			e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			e->OrMemImm64 ( &r->Status.Value, 1 );
			ret = e->MovMemImm32 ( &r->LastModifiedRegister, 255 );
			*/
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LWR instruction.\n";
		return -1;
	}
	return 1;
}



// R3000A ONLY INSTRUCTIONS //

///////////////////////////
// GTE instructions

/*

long Recompiler::MFC2 ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	
	// probably needs an updated CycleCount, so need to stop encoding before too for now
	bStopEncodingBefore = true;
	
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //MFC2 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MFC2 instruction.\n";
		return -1;
	}
	return 1;
}


long Recompiler::MTC2 ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	
	// probably needs an updated CycleCount, so need to stop encoding before too for now
	bStopEncodingBefore = true;
	
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //MTC2 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MTC2 instruction.\n";
		return -1;
	}
	return 1;
}


long Recompiler::LWC2 ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingBefore = true;
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //LWC2 );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			return -1;
			e->MovRegImm32 ( 0, i.sOffset );
			e->AddRegMem32 ( 0, &r->GPR [ i.Base ].u );
			e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			e->OrMemImm64 ( &r->Status.Value, 1 );
			ret = e->MovMemImm32 ( &r->LastModifiedRegister, 255 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LWC2 instruction.\n";
		return -1;
	}
	return 1;
}


long Recompiler::SWC2 ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	
	// for now, stop encoding after this instruction
	// and before
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //SWC2 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SWC2 instruction.\n";
		return -1;
	}
	return 1;
}


long Recompiler::RFE ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	
	// *testing*
	bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //RFE );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //RFE instruction.\n";
		return -1;
	}
	return 1;
}


long Recompiler::RTPS ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //RTPS );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //RTPS instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCLIP ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //NCLIP );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCLIP instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::OP ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //OP );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //OP instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::DPCS ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //DPCS );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //DPCS instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::INTPL ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //INTPL );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //INTPL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::MVMVA ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //MVMVA );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MVMVA instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCDS ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //NCDS );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCDS instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::CDP ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //CDP );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //CDP instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCDT ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //NCDT );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCDT instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCCS ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //NCCS );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCCS instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::CC ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //CC );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //CC instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCS ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //NCS );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCS instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCT ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //NCT );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCT instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SQR ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //SQR );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SQR instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::DCPL ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //DCPL );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //DCPL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::DPCT ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //DPCT );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //DPCT instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::AVSZ3 ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //AVSZ3 );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //AVSZ3 instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::AVSZ4 ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //AVSZ4 );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //AVSZ4 instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::RTPT ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //RTPT );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //RTPT instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::GPF ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //GPF );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //GPF instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::GPL ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //GPL );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //GPL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCCT ( R5900::Instruction::Format i, u32 Address )
{
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction ); //NCCT );
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCCT instruction.\n";
		return -1;
	}
	return 1;
}
*/


long Recompiler::COP2 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "COP2";
	static const void *c_vFunction = R5900::Instruction::Execute::COP2;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



//// ***** R5900 INSTRUCTIONS ***** ////

// arithemetic instructions //

static long R5900::Recompiler::DADD ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DADD";
	static const void *c_vFunction = R5900::Instruction::Execute::DADD;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// go ahead and say that it modifies NextPC since it might, even if it does not this time around
			// but only if instruction is actually encoded
			Local_NextPCModified = true;
			
			// need to stop encoding before, because if it sync ints, that requires PC to be updated
			bStopEncodingBefore = true;
			
			// need to stop encoding after because if it sync ints, then it needs to "jump"
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_DADD_CODE
			e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
			e->AddRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
			
			// branch if not signed overflow
			e->Jmp8_NO ( 0, 0 );
			
			// update CycleCount, set PC, then jump to synchronous interrupt
			e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount );
			
			// set pc
			e->MovMemImm32 ( & r->PC, Address );
			
			//r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
			e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_OV> );
			
			// continue processing store from here //
			e->SetJmpTarget8 ( 0 );
			
			// check if destination is r0
			if ( i.Rd )
			{
				// store result if not signed overflow
				ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //" << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::DADDI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DADDI";
	static const void *c_vFunction = R5900::Instruction::Execute::DADDI;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// go ahead and say that it modifies NextPC since it might, even if it does not this time around
			// but only if instruction is actually encoded
			Local_NextPCModified = true;
			
			// need to stop encoding before, because if it sync ints, that requires PC to be updated
			bStopEncodingBefore = true;
			
			// need to stop encoding after because if it sync ints, then it needs to "jump"
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_DADDI_CODE
			e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
			e->AddReg64ImmX ( RAX, i.sImmediate );
			
			// branch if not signed overflow
			e->Jmp8_NO ( 0, 0 );
			
			// update CycleCount, set PC, then jump to synchronous interrupt
			e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount );
			
			// set pc
			e->MovMemImm32 ( & r->PC, Address );
			
			//r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
			e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_OV> );
			
			// continue processing store from here //
			e->SetJmpTarget8 ( 0 );
			
			// check if destination is r0
			if ( i.Rt )
			{
				// store result if not signed overflow
				ret = e->MovMemReg64 ( &r->GPR [ i.Rt ].s, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::DADDU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DADDU";
	static const void *c_vFunction = R5900::Instruction::Execute::DADDU;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_DADDU_CODE
			if ( i.Rd )
			{
				if ( !i.Rs )
				{
					if ( !i.Rt )
					{
						ret = e->MovMemImm64 ( &r->GPR [ i.Rd ].s, 0 );
					}
					else if ( i.Rd != i.Rt )
					{
						e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].s );
						ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
					}
				}
				else if ( !i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
						ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
					}
				}
				else if ( i.Rd == i.Rs )
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->AddMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rt )
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
					ret = e->AddMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rs == i.Rt )
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
					e->AddRegReg64 ( RAX, RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
					e->AddRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}

static long R5900::Recompiler::DADDIU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DADDIU";
	static const void *c_vFunction = R5900::Instruction::Execute::DADDIU;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_DADDIU_CODE
			if ( i.Rt )
			{
				if ( !i.Rs )
				{
					ret = e->MovMemImm64 ( &r->GPR [ i.Rt ].s, i.sImmediate );
				}
				else if ( i.Rt == i.Rs )
				{
					if ( i.sImmediate )
					{
						ret = e->AddMem64ImmX ( &r->GPR [ i.Rt ].s, i.sImmediate );
					}
				}
				else if ( !i.sImmediate )
				{
					if ( i.Rt != i.Rs )
					{
						e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
						ret = e->MovMemReg64 ( &r->GPR [ i.Rt ].s, RAX );
					}
				}
				else
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
					e->AddReg64ImmX ( RAX, i.sImmediate );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rt ].s, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::DSUB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DSUB";
	static const void *c_vFunction = R5900::Instruction::Execute::DSUB;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// go ahead and say that it modifies NextPC since it might, even if it does not this time around
			// but only if instruction is actually encoded
			Local_NextPCModified = true;
			
			// need to stop encoding before, because if it sync ints, that requires PC to be updated
			bStopEncodingBefore = true;
			
			// need to stop encoding after because if it sync ints, then it needs to "jump"
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_DSUB_CODE
			e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
			e->SubRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
			
			// branch if not signed overflow
			e->Jmp8_NO ( 0, 0 );
			
			// update CycleCount, set PC, then jump to synchronous interrupt
			e->AddMem64ImmX ( & r->CycleCount, LocalCycleCount );
			
			// set pc
			e->MovMemImm32 ( & r->PC, Address );
			
			//r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
			e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_OV> );
			
			// continue processing store from here //
			e->SetJmpTarget8 ( 0 );
			
			// check if destination is r0
			if ( i.Rd )
			{
				// store result if not signed overflow
				ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::DSUBU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DSUBU";
	static const void *c_vFunction = R5900::Instruction::Execute::DSUBU;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_DSUBU_CODE
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					ret = e->MovMemImm64 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( !i.Rs )
				{
					if ( i.Rd == i.Rt )
					{
						ret = e->NegMem64 ( &r->GPR [ i.Rd ].s );
					}
					else
					{
						e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].s );
						e->NegReg64 ( RAX );
						ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
					}
				}
				else if ( !i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
						ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
					}
				}
				else if ( i.Rd == i.Rs )
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->SubMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				/*
				else if ( i.Rd == i.Rt )
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
					ret = e->AddMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
				*/
				else
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].s );
					e->SubRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::DSLL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DSLL";
	static const void *c_vFunction = R5900::Instruction::Execute::DSLL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->MovMemImm64 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
					
					if ( i.Shift )
					{
						e->ShlRegImm64 ( RAX, (u32) i.Shift );
					}
					
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::DSLL32 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DSLL32";
	static const void *c_vFunction = R5900::Instruction::Execute::DSLL32;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->MovMemImm64 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
					e->ShlRegImm64 ( RAX, (u32) i.Shift + 32 );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::DSLLV ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DSLLV";
	static const void *c_vFunction = R5900::Instruction::Execute::DSLLV;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->MovMemImm64 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
					e->MovRegMem32 ( RCX, &r->GPR [ i.Rs ].uw0 );
					e->ShlRegReg64 ( RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::DSRA ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DSRA";
	static const void *c_vFunction = R5900::Instruction::Execute::DSRA;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->MovMemImm64 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
					
					if ( i.Shift )
					{
						e->SarRegImm64 ( RAX, (u32) i.Shift );
					}
					
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::DSRA32 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DSRA32";
	static const void *c_vFunction = R5900::Instruction::Execute::DSRA32;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->MovMemImm64 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
					e->SarRegImm64 ( RAX, (u32) i.Shift + 32 );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::DSRAV ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DSRAV";
	static const void *c_vFunction = R5900::Instruction::Execute::DSRAV;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->MovMemImm64 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
					e->MovRegMem32 ( RCX, &r->GPR [ i.Rs ].uw0 );
					e->SarRegReg64 ( RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::DSRL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DSRL";
	static const void *c_vFunction = R5900::Instruction::Execute::DSRL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->MovMemImm64 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
					
					if ( i.Shift )
					{
						e->ShrRegImm64 ( RAX, (u32) i.Shift );
					}
					
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::DSRL32 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DSRL32";
	static const void *c_vFunction = R5900::Instruction::Execute::DSRL32;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->MovMemImm64 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
					e->ShrRegImm64 ( RAX, (u32) i.Shift + 32 );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::DSRLV ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DSRLV";
	static const void *c_vFunction = R5900::Instruction::Execute::DSRLV;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->MovMemImm64 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else
				{
					e->MovRegMem64 ( RAX, &r->GPR [ i.Rt ].u );
					e->MovRegMem32 ( RCX, &r->GPR [ i.Rs ].uw0 );
					e->ShrRegReg64 ( RAX );
					ret = e->MovMemReg64 ( &r->GPR [ i.Rd ].s, RAX );
				}
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::MULT1 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MULT1";
	static const void *c_vFunction = R5900::Instruction::Execute::MULT1;
	
	// if rs is between -0x800 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff or between -0x7ff and -0x100000, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
	// constant multiply cycles for PS2
	static const int c_iMultiplyCycles = 4 / 2;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_MULT1_CODE
			//bResetCycleCount = true;
			
			// calculate cycles mul/div unit will be busy for
			//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Slow;
			//if ( r->GPR [ i.Rs ].s < 0x800 && r->GPR [ i.Rs ].s >= -0x800 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Fast;
			//}
			//else if ( r->GPR [ i.Rs ].s < 0x100000 && r->GPR [ i.Rs ].s >= -0x100000 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Med;
			//}
			/*
			e->MovRegMem32 ( RCX, & r->GPR [ i.Rs ].sw0 );
			e->MovRegReg32 ( RAX, RCX );
			e->XorReg32ImmX ( RAX, -1 );
			e->CmovSRegReg32 ( RAX, RCX );
			e->MovReg32ImmX ( RDX, c_iMultiplyCycles_Slow );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Med );
			e->CmpReg32ImmX ( RAX, 0x100000 );
			e->CmovBRegReg32 ( RDX, RCX );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Fast );
			e->CmpReg32ImmX ( RAX, 0x800 );
			e->CmovBRegReg32 ( RDX, RCX );
			*/
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			e->MovRegReg64 ( RCX, RAX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle1 );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			//e->SubRegReg64 ( RAX, RCX );
			//e->CmovBRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddRegReg64 ( RDX, RAX );
			e->SubRegReg64 ( RCX, RDX );
			
#ifdef ENABLE_MULTIPLY_LATENCY
			// add in the latency for multiply
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
#endif

			// write back the new busy until cycle
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// do the multiply
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].sw0 );
			e->ImulRegMem32 ( & r->GPR [ i.Rt ].sw0 );
			// *** TODO FOR PS2 *** //
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->Cdqe ();
			e->MovMemReg64 ( & r->LO.sq1, RAX );
			//ret = e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->MovsxdReg64Reg32 ( RDX, RDX );
			ret = e->MovMemReg64 ( & r->HI.sq1, RDX );
			
			// *note*: the R5900 additionally writes to a destination register
			if ( i.Rd )
			{
				e->MovMemReg64 ( & r->GPR [ i.Rd ].s, RAX );
			}
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MULTU1 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MULTU1";
	static const void *c_vFunction = R5900::Instruction::Execute::MULTU1;
	
	// if rs is between -0x800 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff or between -0x7ff and -0x100000, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
	// constant multiply cycles for PS2
	static const int c_iMultiplyCycles = 4 / 2;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_MULTU1_CODE
			//bResetCycleCount = true;
			
			//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Slow;
			//if ( r->GPR [ i.Rs ].u < 0x800 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Fast;
			//}
			//else if ( r->GPR [ i.Rs ].u < 0x100000 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Med;
			//}
			/*
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].sw0 );
			e->MovReg32ImmX ( RDX, c_iMultiplyCycles_Slow );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Med );
			e->CmpReg32ImmX ( RAX, 0x100000 );
			e->CmovBRegReg32 ( RDX, RCX );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Fast );
			e->CmpReg32ImmX ( RAX, 0x800 );
			e->CmovBRegReg32 ( RDX, RCX );
			*/
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			e->MovRegReg64 ( RCX, RAX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle1 );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			//e->SubRegReg64 ( RAX, RCX );
			//e->CmovBRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddRegReg64 ( RDX, RAX );
			e->SubRegReg64 ( RCX, RDX );
			
#ifdef ENABLE_MULTIPLY_LATENCY
			// add in the latency for multiply
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
#endif

			// write back the new busy until cycle
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// do the multiply
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].sw0 );
			e->MulRegMem32 ( & r->GPR [ i.Rt ].sw0 );
			// *** todo for ps2 *** //
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->Cdqe ();
			e->MovMemReg64 ( & r->LO.sq1, RAX );
			//e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->MovsxdReg64Reg32 ( RDX, RDX );
			e->MovMemReg64 ( & r->HI.sq1, RDX );
			
			// *note*: the R5900 additionally writes to a destination register
			if ( i.Rd )
			{
				e->MovMemReg64 ( & r->GPR [ i.Rd ].s, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::DIV1 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DIV1";
	static const void *c_vFunction = R5900::Instruction::Execute::DIV1;
	
	static const int c_iDivideCycles = 36 / 2;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_DIV1_CODE
			//bResetCycleCount = true;
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			e->MovRegReg64 ( RCX, RAX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle1 );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			//e->SubRegReg64 ( RAX, RCX );
			//e->CmovBRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddRegReg64 ( RDX, RAX );
			e->SubRegReg64 ( RCX, RDX );
			
#ifdef ENABLE_DIVIDE_LATENCY
			// add in the latency for multiply
			e->AddReg64ImmX ( RCX, c_iDivideCycles );
#endif

			// write back the new busy until cycle
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// mult/div unit is busy now
			//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iDivideCycles;
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddReg64ImmX ( RAX, c_iDivideCycles );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RAX );
			
			// divide signed: Lo = rs / rt; Hi = rs % rt
			//if ( r->GPR [ i.Rt ].u != 0 )
			//{
			//	// if rs = 0x80000000 and rt = -1 then hi = 0 and lo = 0x80000000
			//	if ( r->GPR [ i.Rs ].u == 0x80000000 && r->GPR [ i.Rt ].s == -1 )
			//	{
			//		r->HiLo.uHi = 0;
			//		r->HiLo.uLo = 0x80000000;
			//	}
			//	else
			//	{
			//		r->HiLo.sLo = r->GPR [ i.Rs ].s / r->GPR [ i.Rt ].s;
			//		r->HiLo.sHi = r->GPR [ i.Rs ].s % r->GPR [ i.Rt ].s;
			//	}
			//}
			//else
			//{
			//	if ( r->GPR [ i.Rs ].s < 0 )
			//	{
			//		r->HiLo.sLo = 1;
			//	}
			//	else
			//	{
			//		r->HiLo.sLo = -1;
			//	}
			//	r->HiLo.uHi = r->GPR [ i.Rs ].u;
			//}
			//e->MovRegMem32 ( RCX, & r->GPR [ i.Rt ].u );
			//e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].u );
			e->MovsxdReg64Mem32 ( RCX, & r->GPR [ i.Rt ].sw0 );
			e->MovsxdReg64Mem32 ( RAX, & r->GPR [ i.Rs ].sw0 );
			//e->OrRegReg64 ( RCX, RCX );
			//e->Jmp8_E ( 0, 0 );
			e->Jmp8_ECXZ ( 0, 0 );
			
			//e->MovRegReg64 ( RDX, RAX );
			//e->SarRegImm64 ( RDX, 63 );
			e->Cqo ();
			e->IdivRegReg64 ( RCX );
			// *** todo for ps2 *** //
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->Cdqe ();
			e->MovMemReg64 ( & r->LO.sq1, RAX );
			//e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->MovsxdReg64Reg32 ( RDX, RDX );
			e->MovMemReg64 ( & r->HI.sq1, RDX );
			e->Jmp8 ( 0, 1 );
			
			e->SetJmpTarget8 ( 0 );
			
			//e->MovReg64ImmX ( RCX, -1 );
			//e->MovReg64ImmX ( RDX, 1 );
			//e->OrRegReg32 ( RAX, RAX );
			//e->CmovSRegReg64 ( RCX, RDX );
			e->Cqo ();
			e->NotReg64 ( RDX );
			e->OrReg64ImmX ( RDX, 1 );
			
			// *** todo for ps2 *** //
			//e->MovMemReg32 ( & r->HiLo.uHi, RAX );
			e->MovMemReg64 ( & r->HI.uq1, RAX );
			//e->MovMemReg32 ( & r->HiLo.uLo, RCX );
			e->MovMemReg64 ( & r->LO.uq1, RDX );
			
			e->SetJmpTarget8 ( 1 );
			
			// done //
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::DIVU1 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DIVU1";
	static const void *c_vFunction = R5900::Instruction::Execute::DIVU1;
	
	static const int c_iDivideCycles = 36 / 2;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_DIVU1_CODE
			//bResetCycleCount = true;
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			e->MovRegReg64 ( RCX, RAX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle1 );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			//e->SubRegReg64 ( RAX, RCX );
			//e->CmovBRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddRegReg64 ( RDX, RAX );
			e->SubRegReg64 ( RCX, RDX );
			
#ifdef ENABLE_DIVIDE_LATENCY
			// add in the latency for multiply
			e->AddReg64ImmX ( RCX, c_iDivideCycles );
#endif

			// write back the new busy until cycle
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// mult/div unit is busy now
			//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iDivideCycles;
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddReg64ImmX ( RAX, c_iDivideCycles );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RAX );
			
			// divide unsigned: Lo = rs / rt; Hi = rs % rt
			//if ( r->GPR [ i.Rt ].u != 0 )
			//{
			//	r->HiLo.uLo = r->GPR [ i.Rs ].u / r->GPR [ i.Rt ].u;
			//	r->HiLo.uHi = r->GPR [ i.Rs ].u % r->GPR [ i.Rt ].u;
			//}
			//else
			//{
			//	r->HiLo.sLo = -1;
			//	r->HiLo.uHi = r->GPR [ i.Rs ].u;
			//}
			e->MovRegMem32 ( RCX, & r->GPR [ i.Rt ].uw0 );
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].uw0 );
			//e->OrRegReg64 ( RCX, RCX );
			//e->Jmp8_E ( 0, 0 );
			e->Jmp8_ECXZ ( 0, 0 );
			
			e->XorRegReg32 ( RDX, RDX );
			e->DivRegReg32 ( RCX );
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->Cdqe ();
			e->MovMemReg64 ( & r->LO.sq1, RAX );
			//e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->MovsxdReg64Reg32 ( RDX, RDX );
			e->MovMemReg64 ( & r->HI.sq1, RDX );
			e->Jmp8 ( 0, 1 );
			
			e->SetJmpTarget8 ( 0 );
			
			//e->MovMemImm32 ( & r->HiLo.sLo, -1 );
			//e->MovMemReg32 ( & r->HiLo.uHi, RAX );
			e->MovMemImm64 ( & r->LO.sq1, -1 );
			e->MovMemReg64 ( & r->HI.uq1, RAX );
			
			e->SetJmpTarget8 ( 1 );
			
			// done //
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MADD ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MADD";
	static const void *c_vFunction = R5900::Instruction::Execute::MADD;
	
	// if rs is between -0x800 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff or between -0x7ff and -0x100000, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;

	// constant multiply cycles for PS2
	static const int c_iMultiplyCycles = 4 / 2;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_MADD_CODE
			//bResetCycleCount = true;
			
			// calculate cycles mul/div unit will be busy for
			//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Slow;
			//if ( r->GPR [ i.Rs ].s < 0x800 && r->GPR [ i.Rs ].s >= -0x800 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Fast;
			//}
			//else if ( r->GPR [ i.Rs ].s < 0x100000 && r->GPR [ i.Rs ].s >= -0x100000 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Med;
			//}
			/*
			e->MovRegMem32 ( RCX, & r->GPR [ i.Rs ].sw0 );
			e->MovRegReg32 ( RAX, RCX );
			e->XorReg32ImmX ( RAX, -1 );
			e->CmovSRegReg32 ( RAX, RCX );
			e->MovReg32ImmX ( RDX, c_iMultiplyCycles_Slow );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Med );
			e->CmpReg32ImmX ( RAX, 0x100000 );
			e->CmovBRegReg32 ( RDX, RCX );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Fast );
			e->CmpReg32ImmX ( RAX, 0x800 );
			e->CmovBRegReg32 ( RDX, RCX );
			*/
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			e->MovRegReg64 ( RCX, RAX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			//e->SubRegReg64 ( RAX, RCX );
			//e->CmovBRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddRegReg64 ( RDX, RAX );
			e->SubRegReg64 ( RCX, RDX );
			
#ifdef ENABLE_MULTIPLY_LATENCY
			// add in the latency for multiply
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
#endif

			// write back the new busy until cycle
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			
			// do the multiply
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].sw0 );
			e->ImulRegMem32 ( & r->GPR [ i.Rt ].sw0 );
			
			// for MADD, add to LO,HI
			e->AddRegMem32 ( RAX, & r->LO.uw0 );
			e->AdcRegMem32 ( RDX, & r->HI.uw0 );
			
			// *** TODO FOR PS2 *** //
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->Cdqe ();
			e->MovMemReg64 ( & r->LO.sq0, RAX );
			//ret = e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->MovsxdReg64Reg32 ( RDX, RDX );
			ret = e->MovMemReg64 ( & r->HI.sq0, RDX );
			
			// *note*: the R5900 additionally writes to a destination register
			if ( i.Rd )
			{
				ret = e->MovMemReg64 ( & r->GPR [ i.Rd ].s, RAX );
			}
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MADD1 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MADD1";
	static const void *c_vFunction = R5900::Instruction::Execute::MADD1;
	
	// if rs is between -0x800 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff or between -0x7ff and -0x100000, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;

	// constant multiply cycles for PS2
	static const int c_iMultiplyCycles = 4 / 2;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_MADD1_CODE
			//bResetCycleCount = true;
			
			// calculate cycles mul/div unit will be busy for
			//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Slow;
			//if ( r->GPR [ i.Rs ].s < 0x800 && r->GPR [ i.Rs ].s >= -0x800 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Fast;
			//}
			//else if ( r->GPR [ i.Rs ].s < 0x100000 && r->GPR [ i.Rs ].s >= -0x100000 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Med;
			//}
			/*
			e->MovRegMem32 ( RCX, & r->GPR [ i.Rs ].sw0 );
			e->MovRegReg32 ( RAX, RCX );
			e->XorReg32ImmX ( RAX, -1 );
			e->CmovSRegReg32 ( RAX, RCX );
			e->MovReg32ImmX ( RDX, c_iMultiplyCycles_Slow );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Med );
			e->CmpReg32ImmX ( RAX, 0x100000 );
			e->CmovBRegReg32 ( RDX, RCX );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Fast );
			e->CmpReg32ImmX ( RAX, 0x800 );
			e->CmovBRegReg32 ( RDX, RCX );
			*/
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			e->MovRegReg64 ( RCX, RAX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle1 );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			//e->SubRegReg64 ( RAX, RCX );
			//e->CmovBRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddRegReg64 ( RDX, RAX );
			e->SubRegReg64 ( RCX, RDX );
			
#ifdef ENABLE_MULTIPLY_LATENCY
			// add in the latency for multiply
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
#endif

			// write back the new busy until cycle
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// do the multiply
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].sw0 );
			e->ImulRegMem32 ( & r->GPR [ i.Rt ].sw0 );
			
			// for MADD, add to LO,HI
			e->AddRegMem32 ( RAX, & r->LO.uw2 );
			e->AdcRegMem32 ( RDX, & r->HI.uw2 );
			
			// *** TODO FOR PS2 *** //
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->Cdqe ();
			e->MovMemReg64 ( & r->LO.sq1, RAX );
			//ret = e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->MovsxdReg64Reg32 ( RDX, RDX );
			ret = e->MovMemReg64 ( & r->HI.sq1, RDX );
			
			// *note*: the R5900 additionally writes to a destination register
			if ( i.Rd )
			{
				ret = e->MovMemReg64 ( & r->GPR [ i.Rd ].s, RAX );
			}
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MADDU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MADDU";
	static const void *c_vFunction = R5900::Instruction::Execute::MADDU;
	
	// if rs is between -0x800 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff or between -0x7ff and -0x100000, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;

	// constant multiply cycles for PS2
	static const int c_iMultiplyCycles = 4 / 2;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_MADDU_CODE
			//bResetCycleCount = true;
			
			//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Slow;
			//if ( r->GPR [ i.Rs ].u < 0x800 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Fast;
			//}
			//else if ( r->GPR [ i.Rs ].u < 0x100000 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Med;
			//}
			/*
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].sw0 );
			e->MovReg32ImmX ( RDX, c_iMultiplyCycles_Slow );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Med );
			e->CmpReg32ImmX ( RAX, 0x100000 );
			e->CmovBRegReg32 ( RDX, RCX );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Fast );
			e->CmpReg32ImmX ( RAX, 0x800 );
			e->CmovBRegReg32 ( RDX, RCX );
			*/
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			e->MovRegReg64 ( RCX, RAX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			//e->SubRegReg64 ( RAX, RCX );
			//e->CmovBRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddRegReg64 ( RDX, RAX );
			e->SubRegReg64 ( RCX, RDX );
			
#ifdef ENABLE_MULTIPLY_LATENCY
			// add in the latency for multiply
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
#endif

			// write back the new busy until cycle
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			
			// do the multiply
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].sw0 );
			e->MulRegMem32 ( & r->GPR [ i.Rt ].sw0 );
			
			// for MADD, add to LO,HI
			e->AddRegMem32 ( RAX, & r->LO.uw0 );
			e->AdcRegMem32 ( RDX, & r->HI.uw0 );
			
			// *** todo for ps2 *** //
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->Cdqe ();
			e->MovMemReg64 ( & r->LO.sq0, RAX );
			//e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->MovsxdReg64Reg32 ( RDX, RDX );
			ret = e->MovMemReg64 ( & r->HI.sq0, RDX );
			
			// *note*: the R5900 additionally writes to a destination register
			if ( i.Rd )
			{
				ret = e->MovMemReg64 ( & r->GPR [ i.Rd ].s, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MADDU1 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MADDU1";
	static const void *c_vFunction = R5900::Instruction::Execute::MADDU1;
	
	// if rs is between -0x800 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff or between -0x7ff and -0x100000, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;

	// constant multiply cycles for PS2
	static const int c_iMultiplyCycles = 4 / 2;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
#ifdef USE_NEW_MADDU1_CODE
			//bResetCycleCount = true;
			
			//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Slow;
			//if ( r->GPR [ i.Rs ].u < 0x800 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Fast;
			//}
			//else if ( r->GPR [ i.Rs ].u < 0x100000 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Med;
			//}
			/*
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].sw0 );
			e->MovReg32ImmX ( RDX, c_iMultiplyCycles_Slow );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Med );
			e->CmpReg32ImmX ( RAX, 0x100000 );
			e->CmovBRegReg32 ( RDX, RCX );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Fast );
			e->CmpReg32ImmX ( RAX, 0x800 );
			e->CmovBRegReg32 ( RDX, RCX );
			*/
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			e->MovRegReg64 ( RCX, RAX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle1 );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			//e->SubRegReg64 ( RAX, RCX );
			//e->CmovBRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddRegReg64 ( RDX, RAX );
			e->SubRegReg64 ( RCX, RDX );
			
#ifdef ENABLE_MULTIPLY_LATENCY
			// add in the latency for multiply
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
#endif

			// write back the new busy until cycle
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// do the multiply
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].sw0 );
			e->MulRegMem32 ( & r->GPR [ i.Rt ].sw0 );
			
			// for MADD, add to LO,HI
			e->AddRegMem32 ( RAX, & r->LO.uw2 );
			e->AdcRegMem32 ( RDX, & r->HI.uw2 );
			
			// *** todo for ps2 *** //
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->Cdqe ();
			e->MovMemReg64 ( & r->LO.sq1, RAX );
			//e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->MovsxdReg64Reg32 ( RDX, RDX );
			ret = e->MovMemReg64 ( & r->HI.sq1, RDX );
			
			// *note*: the R5900 additionally writes to a destination register
			if ( i.Rd )
			{
				ret = e->MovMemReg64 ( & r->GPR [ i.Rd ].s, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



// Load/Store instructions //

static long R5900::Recompiler::SD ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SD";
	static const void *c_vFunction = R5900::Instruction::Execute::SD;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_STORE_CODE
			ret = Generate_Normal_Store ( i, Address, 0x7, (void*) Playstation2::DataBus::Write_t<0xffffffffffffffffULL> );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::LD ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "LD";
	static const void *c_vFunction = R5900::Instruction::Execute::LD;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			Generate_Normal_Load ( i, Address, 0x7, (void*) Playstation2::DataBus::Read_t<0xffffffffffffffffULL> );
			
			// store result //
			
			if ( i.Rt )
			{
				// sign-extend from byte to 64-bits ??
				//e->Cbw ();
				//e->Cwde ();
				//e->Cdqe ();
				
				// store
				e->MovMemReg64 ( & r->GPR [ i.Rt ].sq0, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::LWU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "LWU";
	static const void *c_vFunction = R5900::Instruction::Execute::LWU;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			ret = Generate_Normal_Load ( i, Address, 0x3, (void*) Playstation2::DataBus::Read_t<0xffffffffULL> );
			
			// store result //
			
			if ( i.Rt )
			{
				// zero-extend 32-bit value to 64-bits
				//e->AndReg32ImmX ( RAX, 0xff );
				
				// store
				e->MovMemReg64 ( & r->GPR [ i.Rt ].sq0, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::SDL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SDL";
	static const void *c_vFunction = R5900::Instruction::Execute::SDL;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::SDR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SDR";
	static const void *c_vFunction = R5900::Instruction::Execute::SDR;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::LDL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "LDL";
	static const void *c_vFunction = R5900::Instruction::Execute::LDL;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::LDR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "LDR";
	static const void *c_vFunction = R5900::Instruction::Execute::LDR;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::LQ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "LQ";
	static const void *c_vFunction = R5900::Instruction::Execute::LQ;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			Generate_Normal_Load ( i, Address, 0, (void*) Playstation2::DataBus::Read_t<0> );
			
			// store result //
			
			if ( i.Rt )
			{
				// sign-extend from byte to 64-bits ??
				//e->Cbw ();
				//e->Cwde ();
				//e->Cdqe ();
				
				// store
				//e->MovMemReg64 ( & r->GPR [ i.Rt ].sq0, RAX );
				e->movdqa_from_mem128 ( RAX, RAX, NO_INDEX, 0, 0 );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rt ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::SQ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SQ";
	static const void *c_vFunction = R5900::Instruction::Execute::SQ;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_STORE_CODE
			ret = Generate_Normal_Store ( i, Address, 0, (void*) Playstation2::DataBus::Write_t<0> );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::MOVZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MOVZ";
	static const void *c_vFunction = R5900::Instruction::Execute::MOVZ;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( i.Rd != i.Rs )
				{
					if ( i.Rd == i.Rt )
					{
						//e->MovRegMem64 ( RAX, & r->GPR [ i.Rt ].s );
						e->MovRegMem64 ( RCX, & r->GPR [ i.Rd ].s );
						e->OrRegReg64 ( RCX, RCX );
						e->CmovERegMem64 ( RCX, & r->GPR [ i.Rs ].s );
						e->MovMemReg64 ( & r->GPR [ i.Rd ].s, RCX );
					}
					if ( i.Rs == i.Rt )
					{
						e->MovRegMem64 ( RAX, & r->GPR [ i.Rt ].s );
						e->MovRegMem64 ( RCX, & r->GPR [ i.Rd ].s );
						e->OrRegReg64 ( RAX, RAX );
						e->CmovERegReg64 ( RCX, RAX );
						e->MovMemReg64 ( & r->GPR [ i.Rd ].s, RCX );
					}
					else
					{
						e->MovRegMem64 ( RAX, & r->GPR [ i.Rt ].s );
						e->MovRegMem64 ( RCX, & r->GPR [ i.Rd ].s );
						e->OrRegReg64 ( RAX, RAX );
						e->CmovERegMem64 ( RCX, & r->GPR [ i.Rs ].s );
						e->MovMemReg64 ( & r->GPR [ i.Rd ].s, RCX );
					}
				}
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MOVN ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MOVN";
	static const void *c_vFunction = R5900::Instruction::Execute::MOVN;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( i.Rd != i.Rs )
				{
					if ( i.Rd == i.Rt )
					{
						//e->MovRegMem64 ( RAX, & r->GPR [ i.Rt ].s );
						e->MovRegMem64 ( RCX, & r->GPR [ i.Rd ].s );
						e->OrRegReg64 ( RCX, RCX );
						e->CmovNERegMem64 ( RCX, & r->GPR [ i.Rs ].s );
						e->MovMemReg64 ( & r->GPR [ i.Rd ].s, RCX );
					}
					if ( i.Rs == i.Rt )
					{
						e->MovRegMem64 ( RAX, & r->GPR [ i.Rt ].s );
						e->MovRegMem64 ( RCX, & r->GPR [ i.Rd ].s );
						e->OrRegReg64 ( RAX, RAX );
						e->CmovNERegReg64 ( RCX, RAX );
						e->MovMemReg64 ( & r->GPR [ i.Rd ].s, RCX );
					}
					else
					{
						e->MovRegMem64 ( RAX, & r->GPR [ i.Rt ].s );
						e->MovRegMem64 ( RCX, & r->GPR [ i.Rd ].s );
						e->OrRegReg64 ( RAX, RAX );
						e->CmovNERegMem64 ( RCX, & r->GPR [ i.Rs ].s );
						e->MovMemReg64 ( & r->GPR [ i.Rd ].s, RCX );
					}
				}
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::MFHI1 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MFHI1";
	static const void *c_vFunction = R5900::Instruction::Execute::MFHI1;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_MFHI1_CODE
			/*
			e->MovRegFromMem64 ( RAX, &r->CycleCount );
			e->CmpRegMem64 ( RAX, &r->MulDiv_BusyUntil_Cycle );
			e->CmovBRegMem64 ( RAX, &r->MulDiv_BusyUntil_Cycle );
			ret = e->MovRegToMem64 ( &r->CycleCount, RAX );
			if ( i.Rd )
			{
				e->MovRegFromMem32 ( RAX, &r->HiLo.uHi );
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RAX );
			}
			Local_LastModifiedReg = i.Rd;
			*/
			
			// this instruction interlocks if multiply/divide unit is busy
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle1 );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			//e->XorRegReg64 ( RDX, RDX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle1 );
			//e->CmovBRegReg64 ( RDX, RAX );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			// move from Hi register
			//r->GPR [ i.Rd ].u = r->HiLo.uHi;
			//CHECK_DELAYSLOT ( i.Rd );
			if ( i.Rd )
			{
				//e->MovRegMem32 ( RAX, & r->HiLo.uHi );
				e->MovRegMem64 ( RAX, & r->HI.uq1 );
				//e->MovMemReg32 ( & r->GPR [ i.Rd ].u, RAX );
				e->MovMemReg64 ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MTHI1 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MTHI1";
	static const void *c_vFunction = R5900::Instruction::Execute::MTHI1;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].uw0 );
			e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].u );
			//ret = e->MovRegToMem32 ( &r->HiLo.uHi, RAX );
			ret = e->MovMemReg64 ( &r->HI.uq1, RAX );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MFLO1 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MFLO1";
	static const void *c_vFunction = R5900::Instruction::Execute::MFLO1;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_MFLO1_CODE
			/*
			e->MovRegFromMem64 ( RAX, &r->CycleCount );
			e->CmpRegMem64 ( RAX, &r->MulDiv_BusyUntil_Cycle );
			e->CmovBRegMem64 ( RAX, &r->MulDiv_BusyUntil_Cycle );
			ret = e->MovRegToMem64 ( &r->CycleCount, RAX );
			if ( i.Rd )
			{
				e->MovRegFromMem32 ( RAX, &r->HiLo.uLo );
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RAX );
			}
			Local_LastModifiedReg = i.Rd;
			*/
			
			// this instruction interlocks if multiply/divide unit is busy
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle1 );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			//e->XorRegReg64 ( RDX, RDX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle1 );
			//e->CmovBRegReg64 ( RDX, RAX );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			// move from Lo register
			//r->GPR [ i.Rd ].u = r->HiLo.uLo;
			//CHECK_DELAYSLOT ( i.Rd );
			if ( i.Rd )
			{
				//e->MovRegMem32 ( RAX, & r->HiLo.uLo );
				e->MovRegMem64 ( RAX, & r->LO.uq1 );
				//e->MovMemReg32 ( & r->GPR [ i.Rd ].u, RAX );
				e->MovMemReg64 ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MTLO1 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MTLO1";
	static const void *c_vFunction = R5900::Instruction::Execute::MTLO1;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			//e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].uw0 );
			e->MovRegMem64 ( RAX, &r->GPR [ i.Rs ].u );
			//ret = e->MovRegToMem32 ( &r->HiLo.uLo, RAX );
			ret = e->MovMemReg64 ( &r->LO.uq1, RAX );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



static long R5900::Recompiler::MFSA ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MFSA";
	static const void *c_vFunction = R5900::Instruction::Execute::MFSA;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			e->MovRegMem32 ( RAX, & r->SA );
			e->MovMemReg64 ( & r->GPR [ i.Rd ].uq0, RAX );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MTSA ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MTSA";
	static const void *c_vFunction = R5900::Instruction::Execute::MTSA;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].uw0 );
			e->AndReg32ImmX ( RAX, 0xf );
			e->MovMemReg32 ( & r->SA, RAX );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MTSAB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MTSAB";
	static const void *c_vFunction = R5900::Instruction::Execute::MTSAB;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].uw0 );
			e->XorReg32ImmX ( RAX, i.uImmediate );
			e->AndReg32ImmX ( RAX, 0xf );
			e->MovMemReg32 ( & r->SA, RAX );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MTSAH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MTSAH";
	static const void *c_vFunction = R5900::Instruction::Execute::MTSAH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].uw0 );
			e->XorReg32ImmX ( RAX, i.uImmediate );
			e->AndReg32ImmX ( RAX, 0x7 );
			e->AddRegReg32 ( RAX, RAX );
			e->MovMemReg32 ( & r->SA, RAX );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}




// Branch instructions //

static long R5900::Recompiler::BEQL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BEQL";
	static const void *c_vFunction = R5900::Instruction::Execute::BEQL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BEQL_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBEQL> );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BNEL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BNEL";
	static const void *c_vFunction = R5900::Instruction::Execute::BNEL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BNEL_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBNEL> );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BGEZL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BGEZL";
	static const void *c_vFunction = R5900::Instruction::Execute::BGEZL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BGEZL_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBGEZL> );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BLEZL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BLEZL";
	static const void *c_vFunction = R5900::Instruction::Execute::BLEZL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BLEZL_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBLEZL> );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BGTZL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BGTZL";
	static const void *c_vFunction = R5900::Instruction::Execute::BGTZL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BGTZL_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBGTZL> );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BLTZL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BLTZL";
	static const void *c_vFunction = R5900::Instruction::Execute::BLTZL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BLTZL_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBLTZL> );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



static long R5900::Recompiler::BLTZALL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BLTZALL";
	static const void *c_vFunction = R5900::Instruction::Execute::BLTZALL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BLTZALL_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBLTZALL> );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BGEZALL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BGEZALL";
	static const void *c_vFunction = R5900::Instruction::Execute::BGEZALL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BGEZALL_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBGEZALL> );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}




static long R5900::Recompiler::BC0T ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BC0T";
	static const void *c_vFunction = R5900::Instruction::Execute::BC0T;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BC0TL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BC0TL";
	static const void *c_vFunction = R5900::Instruction::Execute::BC0TL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BC0F ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BC0F";
	static const void *c_vFunction = R5900::Instruction::Execute::BC0F;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BC0FL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BC0FL";
	static const void *c_vFunction = R5900::Instruction::Execute::BC0FL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BC1T ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BC1T";
	static const void *c_vFunction = R5900::Instruction::Execute::BC1T;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BC1TL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BC1TL";
	static const void *c_vFunction = R5900::Instruction::Execute::BC1TL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BC1F ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BC1F";
	static const void *c_vFunction = R5900::Instruction::Execute::BC1F;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BC1FL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BC1FL";
	static const void *c_vFunction = R5900::Instruction::Execute::BC1FL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BC2T ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BC2T";
	static const void *c_vFunction = R5900::Instruction::Execute::BC2T;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BC2TL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BC2TL";
	static const void *c_vFunction = R5900::Instruction::Execute::BC2TL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BC2F ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BC2F";
	static const void *c_vFunction = R5900::Instruction::Execute::BC2F;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::BC2FL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "BC2FL";
	static const void *c_vFunction = R5900::Instruction::Execute::BC2FL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}






static long R5900::Recompiler::TGEI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "TGEI";
	static const void *c_vFunction = R5900::Instruction::Execute::TGEI;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			ret = Generate_Normal_Trap ( i, Address );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::TGEIU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "TGEIU";
	static const void *c_vFunction = R5900::Instruction::Execute::TGEIU;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			ret = Generate_Normal_Trap ( i, Address );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::TLTI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "TLTI";
	static const void *c_vFunction = R5900::Instruction::Execute::TLTI;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			ret = Generate_Normal_Trap ( i, Address );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::TLTIU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "TLTIU";
	static const void *c_vFunction = R5900::Instruction::Execute::TLTIU;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			ret = Generate_Normal_Trap ( i, Address );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::TEQI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "TEQI";
	static const void *c_vFunction = R5900::Instruction::Execute::TEQI;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			ret = Generate_Normal_Trap ( i, Address );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::TNEI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "TNEI";
	static const void *c_vFunction = R5900::Instruction::Execute::TNEI;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			ret = Generate_Normal_Trap ( i, Address );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::TGE ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "TGE";
	static const void *c_vFunction = R5900::Instruction::Execute::TGE;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			ret = Generate_Normal_Trap ( i, Address );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::TGEU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "TGEU";
	static const void *c_vFunction = R5900::Instruction::Execute::TGEU;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			ret = Generate_Normal_Trap ( i, Address );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::TLT ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "TLT";
	static const void *c_vFunction = R5900::Instruction::Execute::TLT;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			ret = Generate_Normal_Trap ( i, Address );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::TLTU ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "TLTU";
	static const void *c_vFunction = R5900::Instruction::Execute::TLTU;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			ret = Generate_Normal_Trap ( i, Address );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::TEQ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "TEQ";
	static const void *c_vFunction = R5900::Instruction::Execute::TEQ;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			ret = Generate_Normal_Trap ( i, Address );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::TNE ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "TNE";
	static const void *c_vFunction = R5900::Instruction::Execute::TNE;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			ret = Generate_Normal_Trap ( i, Address );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


















// * R5900 Parallel (SIMD) instructions * //


static long R5900::Recompiler::PADSBH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PADSBH";
	static const void *c_vFunction = R5900::Instruction::Execute::PADSBH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PADSBH_CODE
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->movdqa_regmem ( RDX, & r->GPR [ i.Rt ].u );
					e->movdqa_regreg ( RCX, RAX );
					
					e->paddwregreg ( RAX, RDX );
					e->psubwregreg ( RCX, RDX );
					e->pblendwregregimm ( RAX, RCX, 0xf );
					
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PABSH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PABSH";
	static const void *c_vFunction = R5900::Instruction::Execute::PABSH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PABSH_CODE
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->pabswregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PABSW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PABSW";
	static const void *c_vFunction = R5900::Instruction::Execute::PABSW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PABSW_CODE
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->pabsdregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PAND ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PAND";
	static const void *c_vFunction = R5900::Instruction::Execute::PAND;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PAND_CODE
			if ( i.Rd )
			{
				if ( !i.Rs )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else if ( !i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else if ( i.Rs == i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->pandregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PXOR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PXOR";
	static const void *c_vFunction = R5900::Instruction::Execute::PXOR;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PXOR_CODE
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else if ( !i.Rs )
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else if ( !i.Rt )
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->pxorregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::POR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "POR";
	static const void *c_vFunction = R5900::Instruction::Execute::POR;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_POR_CODE
			if ( i.Rd )
			{
				if ( !i.Rs )
				{
					if ( !i.Rt )
					{
						e->pxorregreg ( RAX, RAX );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
					else if ( i.Rd != i.Rt )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( !i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( i.Rs == i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->porregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PNOR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PNOR";
	static const void *c_vFunction = R5900::Instruction::Execute::PNOR;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PNOR_CODE
			if ( i.Rd )
			{
				if ( !i.Rs )
				{
					if ( !i.Rt )
					{
						e->pcmpeqdregreg ( RAX, RAX );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
					else
					{
						//e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
						e->pcmpeqdregreg ( RAX, RAX );
						e->pxorregmem ( RAX, & r->GPR [ i.Rt ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( !i.Rt )
				{
					//e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->pcmpeqdregreg ( RAX, RAX );
					e->pxorregmem ( RAX, & r->GPR [ i.Rs ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else if ( i.Rs == i.Rt )
				{
					//e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->pcmpeqdregreg ( RAX, RAX );
					e->pxorregmem ( RAX, & r->GPR [ i.Rs ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->pcmpeqdregreg ( RCX, RCX );
					e->porregmem ( RAX, & r->GPR [ i.Rt ].u );
					e->pxorregreg ( RAX, RCX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PLZCW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PLZCW";
	static const void *c_vFunction = R5900::Instruction::Execute::PLZCW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PLZCW_CODE
			if ( i.Rd )
			{
				e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].uw0 );
				e->Cdq ();
				e->XorRegReg32 ( RAX, RDX );
				e->BsrRegReg32 ( RAX, RAX );
				e->NegReg32 ( RAX );
				e->AddReg32ImmX ( RAX, 30 );
				e->MovMemReg32 ( & r->GPR [ i.Rd ].uw0, RAX );
				
				e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].uw1 );
				e->Cdq ();
				e->XorRegReg32 ( RAX, RDX );
				e->BsrRegReg32 ( RAX, RAX );
				e->NegReg32 ( RAX );
				e->AddReg32ImmX ( RAX, 30 );
				e->MovMemReg32 ( & r->GPR [ i.Rd ].uw1, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PMFHL_LH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMFHL_LH";
	static const void *c_vFunction = R5900::Instruction::Execute::PMFHL_LH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PMFHL_LH_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			//e->MovRegReg64 ( RCX, RAX );
			//e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			//e->SubRegReg64 ( RCX, RAX );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->LO.u );
				e->movdqa_regmem ( RBX, & r->HI.u );
				
				e->pshuflwregregimm ( RCX, RAX, ( 2 << 2 ) + ( 0 << 0 ) );
				e->pshufhwregregimm ( RCX, RCX, ( 2 << 2 ) + ( 0 << 0 ) );
				e->pshuflwregregimm ( RDX, RBX, ( 3 << 6 ) + ( 1 << 4 ) );
				e->pshufhwregregimm ( RDX, RDX, ( 3 << 6 ) + ( 1 << 4 ) );
				e->pblendwregregimm ( RCX, RDX, 0xcc );
				
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RCX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PMFHL_LW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMFHL_LW";
	static const void *c_vFunction = R5900::Instruction::Execute::PMFHL_LW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PMFHL_LW_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			//e->MovRegReg64 ( RCX, RAX );
			//e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			//e->SubRegReg64 ( RCX, RAX );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			if ( i.Rd )
			{
				e->movdqa_regmem ( RBX, & r->HI.u );
				e->movdqa_regmem ( RAX, & r->LO.u );
				
				e->pshufdregregimm ( RCX, RBX, ( 2 << 6 ) + ( 0 << 2 ) );
				e->pblendwregregimm ( RAX, RCX, 0xcc );
				
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PMFHL_UW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMFHL_UW";
	static const void *c_vFunction = R5900::Instruction::Execute::PMFHL_UW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PMFHL_UW_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			//e->MovRegReg64 ( RCX, RAX );
			//e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			//e->SubRegReg64 ( RCX, RAX );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			if ( i.Rd )
			{
				e->movdqa_regmem ( RBX, & r->HI.u );
				e->movdqa_regmem ( RAX, & r->LO.u );
				
				e->pshufdregregimm ( RCX, RAX, ( 3 << 4 ) + ( 1 << 0 ) );
				e->pblendwregregimm ( RBX, RCX, 0x33 );
				
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RBX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PMTHL_LW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMTHL_LW";
	static const void *c_vFunction = R5900::Instruction::Execute::PMTHL_LW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_PMTHL_LW_CODE
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].uw0 );
			e->MovRegMem32 ( RCX, & r->GPR [ i.Rs ].uw1 );
			e->MovRegMem32 ( RDX, & r->GPR [ i.Rs ].uw2 );
			
			e->MovMemReg32 ( & r->LO.uw0, RAX );
			
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].uw3 );
			
			e->MovMemReg32 ( & r->HI.uw0, RCX );
			e->MovMemReg32 ( & r->LO.uw2, RDX );
			e->MovMemReg32 ( & r->HI.uw2, RAX );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PMFHL_SH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMFHL_SH";
	static const void *c_vFunction = R5900::Instruction::Execute::PMFHL_SH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PMFHL_SH_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			//e->MovRegReg64 ( RCX, RAX );
			//e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			//e->SubRegReg64 ( RCX, RAX );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			if ( i.Rd )
			{
				e->movdqa_regmem ( RBX, & r->HI.u );
				e->movdqa_regmem ( RAX, & r->LO.u );
				
				e->pshufdregregimm ( RCX, RBX, ( 1 << 6 ) + ( 0 << 4 ) );
				e->pblendwregregimm ( RCX, RAX, 0x0f );
				
				e->pshufdregregimm ( RDX, RAX, ( 3 << 2 ) + ( 2 << 0 ) );
				e->pblendwregregimm ( RDX, RBX, 0xf0 );
				
				e->packssdwregreg ( RCX, RDX );
				
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RCX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PMFHL_SLW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMFHL_SLW";
	static const void *c_vFunction = R5900::Instruction::Execute::PMFHL_SLW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PMFHL_SLW_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			//e->MovRegReg64 ( RCX, RAX );
			//e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			//e->SubRegReg64 ( RCX, RAX );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			if ( i.Rd )
			{
				e->MovRegMem32 ( RAX, & r->HI.uw0 );
				e->Cdq ();
				e->MovRegImm32 ( RCX, 0x7fffffff );
				e->XorRegReg32 ( RCX, RDX );
				e->CmpRegReg32 ( RAX, RDX );
				e->CmovERegMem32 ( RCX, & r->LO.uw0 );
				e->MovsxdReg64Reg32 ( RCX, RCX );
				e->MovMemReg64 ( & r->GPR [ i.Rd ].uq0, RCX );
				
				e->MovRegMem32 ( RAX, & r->HI.uw2 );
				e->Cdq ();
				e->MovRegImm32 ( RCX, 0x7fffffff );
				e->XorRegReg32 ( RCX, RDX );
				e->CmpRegReg32 ( RAX, RDX );
				e->CmovERegMem32 ( RCX, & r->LO.uw2 );
				e->MovsxdReg64Reg32 ( RCX, RCX );
				e->MovMemReg64 ( & r->GPR [ i.Rd ].uq1, RCX );
				
				/*
				e->movdqa_regmem ( RBX, & r->HI.u );
				e->movdqa_regmem ( RAX, & r->LO.u );
				
				pshufdregregimm ( RCX, RBX, ( 1 << 6 ) + ( 0 << 4 ) );
				pblendwregregimm ( RCX, RAX, 0x0f );
				
				pshufdregregimm ( RDX, RAX, ( 3 << 2 ) + ( 2 << 0 ) );
				pblendwregregimm ( RDX, RBX, 0xf0 );
				
				e->packssdwregreg ( RCX, RDX );
				
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RCX );
				*/
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



static long R5900::Recompiler::PSLLH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSLLH";
	static const void *c_vFunction = R5900::Instruction::Execute::PSLLH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PSLLH_CODE
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else if ( !( i.Shift & 0xf ) )
				{
					if ( i.Rd != i.Rt )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
					e->psllwregimm ( RAX, i.Shift & 0xf );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PSLLW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSLLW";
	static const void *c_vFunction = R5900::Instruction::Execute::PSLLW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PSLLW_CODE
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else if ( !( i.Shift & 0xf ) )
				{
					if ( i.Rd != i.Rt )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
					e->pslldregimm ( RAX, i.Shift & 0xf );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PSRLH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSRLH";
	static const void *c_vFunction = R5900::Instruction::Execute::PSRLH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PSRLH_CODE
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else if ( !( i.Shift & 0xf ) )
				{
					if ( i.Rd != i.Rt )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
					e->psrlwregimm ( RAX, i.Shift & 0xf );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PSRLW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSRLW";
	static const void *c_vFunction = R5900::Instruction::Execute::PSRLW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PSRLW_CODE
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else if ( !( i.Shift & 0xf ) )
				{
					if ( i.Rd != i.Rt )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
					e->psrldregimm ( RAX, i.Shift & 0xf );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PSRAH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSRAH";
	static const void *c_vFunction = R5900::Instruction::Execute::PSRAH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PSRAH_CODE
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else if ( !( i.Shift & 0xf ) )
				{
					if ( i.Rd != i.Rt )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
					e->psrawregimm ( RAX, i.Shift & 0xf );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PSRAW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSRAW";
	static const void *c_vFunction = R5900::Instruction::Execute::PSRAW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PSRAW_CODE
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else if ( !( i.Shift & 0xf ) )
				{
					if ( i.Rd != i.Rt )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
					e->psradregimm ( RAX, i.Shift & 0xf );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PSLLVW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSLLVW";
	static const void *c_vFunction = R5900::Instruction::Execute::PSLLVW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PSLLVW_CODE
			if ( i.Rd )
			{
				e->MovRegMem32 ( RCX, & r->GPR [ i.Rs ].uw0 );
				e->MovRegMem32 ( RAX, & r->GPR [ i.Rt ].uw0 );
				e->ShlRegReg32 ( RAX );
				e->Cdqe ();
				e->MovMemReg64 ( & r->GPR [ i.Rd ].uq0, RAX );
				e->MovRegMem32 ( RCX, & r->GPR [ i.Rs ].uw2 );
				e->MovRegMem32 ( RAX, & r->GPR [ i.Rt ].uw2 );
				e->ShlRegReg32 ( RAX );
				e->Cdqe ();
				e->MovMemReg64 ( & r->GPR [ i.Rd ].uq1, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PSRLVW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSRLVW";
	static const void *c_vFunction = R5900::Instruction::Execute::PSRLVW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PSRLVW_CODE
			if ( i.Rd )
			{
				e->MovRegMem32 ( RCX, & r->GPR [ i.Rs ].uw0 );
				e->MovRegMem32 ( RAX, & r->GPR [ i.Rt ].uw0 );
				e->ShrRegReg32 ( RAX );
				e->Cdqe ();
				e->MovMemReg64 ( & r->GPR [ i.Rd ].uq0, RAX );
				e->MovRegMem32 ( RCX, & r->GPR [ i.Rs ].uw2 );
				e->MovRegMem32 ( RAX, & r->GPR [ i.Rt ].uw2 );
				e->ShrRegReg32 ( RAX );
				e->Cdqe ();
				e->MovMemReg64 ( & r->GPR [ i.Rd ].uq1, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PSRAVW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSRAVW";
	static const void *c_vFunction = R5900::Instruction::Execute::PSRAVW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PSRAVW_CODE
			if ( i.Rd )
			{
				e->MovRegMem32 ( RCX, & r->GPR [ i.Rs ].uw0 );
				e->MovRegMem32 ( RAX, & r->GPR [ i.Rt ].uw0 );
				e->SarRegReg32 ( RAX );
				e->Cdqe ();
				e->MovMemReg64 ( & r->GPR [ i.Rd ].uq0, RAX );
				e->MovRegMem32 ( RCX, & r->GPR [ i.Rs ].uw2 );
				e->MovRegMem32 ( RAX, & r->GPR [ i.Rt ].uw2 );
				e->SarRegReg32 ( RAX );
				e->Cdqe ();
				e->MovMemReg64 ( & r->GPR [ i.Rd ].uq1, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PADDB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PADDB";
	static const void *c_vFunction = R5900::Instruction::Execute::PADDB;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PADDB_CODE
			if ( i.Rd )
			{
				if ( !i.Rs )
				{
					if ( !i.Rt )
					{
						e->pxorregreg ( RAX, RAX );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
					else if ( i.Rd != i.Rt )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( !i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( i.Rs == i.Rt )
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->paddbregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->paddbregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PADDH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PADDH";
	static const void *c_vFunction = R5900::Instruction::Execute::PADDH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PADDH_CODE
			if ( i.Rd )
			{
				if ( !i.Rs )
				{
					if ( !i.Rt )
					{
						e->pxorregreg ( RAX, RAX );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
					else if ( i.Rd != i.Rt )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( !i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( i.Rs == i.Rt )
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->paddwregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->paddwregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PADDW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PADDW";
	static const void *c_vFunction = R5900::Instruction::Execute::PADDW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PADDW_CODE
			if ( i.Rd )
			{
				if ( !i.Rs )
				{
					if ( !i.Rt )
					{
						e->pxorregreg ( RAX, RAX );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
					else if ( i.Rd != i.Rt )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( !i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( i.Rs == i.Rt )
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->padddregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->padddregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PSUBB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSUBB";
	static const void *c_vFunction = R5900::Instruction::Execute::PSUBB;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PSUBB_CODE
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					if ( !i.Rs )
					{
						e->pxorregreg ( RAX, RAX );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
					else if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( i.Rs == i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->psubbregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PSUBH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSUBH";
	static const void *c_vFunction = R5900::Instruction::Execute::PSUBH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PSUBH_CODE
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					if ( !i.Rs )
					{
						e->pxorregreg ( RAX, RAX );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
					else if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( i.Rs == i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->psubwregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PSUBW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSUBW";
	static const void *c_vFunction = R5900::Instruction::Execute::PSUBW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PSUBW_CODE
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					if ( !i.Rs )
					{
						e->pxorregreg ( RAX, RAX );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
					else if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( i.Rs == i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->psubdregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PADDSB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PADDSB";
	static const void *c_vFunction = R5900::Instruction::Execute::PADDSB;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PADDSB_CODE
			if ( i.Rd )
			{
				if ( !i.Rs )
				{
					if ( !i.Rt )
					{
						e->pxorregreg ( RAX, RAX );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
					else if ( i.Rd != i.Rt )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( !i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( i.Rs == i.Rt )
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->paddsbregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->paddsbregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PADDSH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PADDSH";
	static const void *c_vFunction = R5900::Instruction::Execute::PADDSH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PADDSH_CODE
			if ( i.Rd )
			{
				if ( !i.Rs )
				{
					if ( !i.Rt )
					{
						e->pxorregreg ( RAX, RAX );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
					else if ( i.Rd != i.Rt )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( !i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( i.Rs == i.Rt )
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->paddswregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->paddswregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PADDSW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PADDSW";
	static const void *c_vFunction = R5900::Instruction::Execute::PADDSW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PSUBSB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSUBSB";
	static const void *c_vFunction = R5900::Instruction::Execute::PSUBSB;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PSUBSB_CODE
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					if ( !i.Rs )
					{
						e->pxorregreg ( RAX, RAX );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
					else if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( i.Rs == i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->psubsbregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PSUBSH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSUBSH";
	static const void *c_vFunction = R5900::Instruction::Execute::PSUBSH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PSUBSH_CODE
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					if ( !i.Rs )
					{
						e->pxorregreg ( RAX, RAX );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
					else if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( i.Rs == i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->psubswregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PSUBSW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSUBSW";
	static const void *c_vFunction = R5900::Instruction::Execute::PSUBSW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PADDUB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PADDUB";
	static const void *c_vFunction = R5900::Instruction::Execute::PADDUB;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PADDUB_CODE
			if ( i.Rd )
			{
				if ( !i.Rs )
				{
					if ( !i.Rt )
					{
						e->pxorregreg ( RAX, RAX );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
					else if ( i.Rd != i.Rt )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( !i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( i.Rs == i.Rt )
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->paddusbregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->paddusbregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PADDUH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PADDUH";
	static const void *c_vFunction = R5900::Instruction::Execute::PADDUH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PADDUH_CODE
			if ( i.Rd )
			{
				if ( !i.Rs )
				{
					if ( !i.Rt )
					{
						e->pxorregreg ( RAX, RAX );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
					else if ( i.Rd != i.Rt )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( !i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( i.Rs == i.Rt )
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->padduswregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->padduswregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PADDUW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PADDUW";
	static const void *c_vFunction = R5900::Instruction::Execute::PADDUW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PSUBUB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSUBUB";
	static const void *c_vFunction = R5900::Instruction::Execute::PSUBUB;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PSUBUB_CODE
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					if ( !i.Rs )
					{
						e->pxorregreg ( RAX, RAX );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
					else if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( i.Rs == i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->psubusbregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PSUBUH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSUBUH";
	static const void *c_vFunction = R5900::Instruction::Execute::PSUBUH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PSUBUH_CODE
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					if ( !i.Rs )
					{
						e->pxorregreg ( RAX, RAX );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
					else if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else if ( i.Rs == i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->psubuswregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PSUBUW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PSUBUW";
	static const void *c_vFunction = R5900::Instruction::Execute::PSUBUW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



static long R5900::Recompiler::PMAXH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMAXH";
	static const void *c_vFunction = R5900::Instruction::Execute::PMAXH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PMAXH_CODE
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->pmaxswregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PMAXW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMAXW";
	static const void *c_vFunction = R5900::Instruction::Execute::PMAXW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PMAXW_CODE
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->pmaxsdregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PMINH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMINH";
	static const void *c_vFunction = R5900::Instruction::Execute::PMINH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PMINH_CODE
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->pminswregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PMINW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMINW";
	static const void *c_vFunction = R5900::Instruction::Execute::PMINW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PMINW_CODE
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
						ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
					}
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->pminsdregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}




static long R5900::Recompiler::PPACB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PPACB";
	static const void *c_vFunction = R5900::Instruction::Execute::PPACB;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		
		case 1:
#ifdef USE_NEW_PPACB_CODE
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					
					e->psllwregimm ( RAX, 8 );
					e->psrlwregimm ( RAX, 8 );
					
					e->packuswbregreg ( RAX, RAX );
					
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
					e->movdqa_regmem ( RCX, & r->GPR [ i.Rs ].u );
					
					e->psllwregimm ( RAX, 8 );
					e->psrlwregimm ( RAX, 8 );
					e->psllwregimm ( RCX, 8 );
					e->psrlwregimm ( RCX, 8 );

					e->packuswbregreg ( RAX, RCX );
					
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PPACH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PPACH";
	static const void *c_vFunction = R5900::Instruction::Execute::PPACH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PPACH_CODE
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					
					e->pxorregreg ( RDX, RDX );
					e->pblendwregregimm ( RAX, RDX, 0xaa );
					
					e->packusdwregreg ( RAX, RAX );
					
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
					e->movdqa_regmem ( RCX, & r->GPR [ i.Rs ].u );
					
					e->pxorregreg ( RDX, RDX );
					e->pblendwregregimm ( RAX, RDX, 0xaa );
					e->pblendwregregimm ( RCX, RDX, 0xaa );

					e->packusdwregreg ( RAX, RCX );
					
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PPACW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PPACW";
	static const void *c_vFunction = R5900::Instruction::Execute::PPACW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PPACW_CODE
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					
					e->pshufdregregimm ( RAX, RAX, ( 2 << 6 ) + ( 0 << 4 ) + ( 2 << 2 ) + 0 );
					
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
					e->movdqa_regmem ( RCX, & r->GPR [ i.Rs ].u );
					
					e->pshufdregregimm ( RAX, RAX, ( 2 << 2 ) + 0 );
					e->pshufdregregimm ( RCX, RCX, ( 2 << 6 ) + ( 0 << 4 ) );

					e->pblendwregregimm ( RAX, RCX, 0xf0 );
					
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PEXT5 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PEXT5";
	static const void *c_vFunction = R5900::Instruction::Execute::PEXT5;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PEXT5_CODE
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
				
				// get a
				e->movdqa_regreg ( RBX, RAX );
				e->psrldregimm ( RBX, 15 );
				e->pslldregimm ( RBX, 31 );
				
				// get c3
				e->movdqa_regreg ( RCX, RAX );
				e->psrldregimm ( RCX, 10 );
				e->pslldregimm ( RCX, 27 );
				e->psrldregimm ( RCX, 8 );
				
				// get c2
				e->movdqa_regreg ( RDX, RAX );
				e->psrldregimm ( RDX, 5 );
				e->pslldregimm ( RDX, 27 );
				e->psrldregimm ( RDX, 16 );
				
				// get c1
				e->pslldregimm ( RAX, 27 );
				e->psrldregimm ( RAX, 24 );
				
				// combine
				e->porregreg ( RAX, RBX );
				e->porregreg ( RAX, RCX );
				e->porregreg ( RAX, RDX );
				
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PPAC5 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PPAC5";
	static const void *c_vFunction = R5900::Instruction::Execute::PPAC5;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PPAC5_CODE
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
				
				// get c1
				e->movdqa_regreg ( RBX, RAX );
				e->pslldregimm ( RBX, 24 );
				e->psrldregimm ( RBX, 27 );
				
				// get c2
				e->movdqa_regreg ( RCX, RAX );
				e->pslldregimm ( RCX, 16 );
				e->psrldregimm ( RCX, 27 );
				e->pslldregimm ( RCX, 5 );
				
				// get c3
				e->movdqa_regreg ( RDX, RAX );
				e->pslldregimm ( RDX, 8 );
				e->psrldregimm ( RDX, 27 );
				e->pslldregimm ( RDX, 10 );
				
				// get a
				e->psrldregimm ( RAX, 31 );
				e->pslldregimm ( RAX, 15 );
				
				// combine
				e->porregreg ( RAX, RBX );
				e->porregreg ( RAX, RCX );
				e->porregreg ( RAX, RDX );
				
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PCGTB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PCGTB";
	static const void *c_vFunction = R5900::Instruction::Execute::PCGTB;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PCGTB_CODE
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->pcmpgtbregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PCGTH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PCGTH";
	static const void *c_vFunction = R5900::Instruction::Execute::PCGTH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PCGTH_CODE
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->pcmpgtwregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PCGTW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PCGTW";
	static const void *c_vFunction = R5900::Instruction::Execute::PCGTW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PCGTW_CODE
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					e->pxorregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->pcmpgtdregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PCEQB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PCEQB";
	static const void *c_vFunction = R5900::Instruction::Execute::PCEQB;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PCEQB_CODE
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					e->pcmpeqdregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->pcmpeqbregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PCEQH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PCEQH";
	static const void *c_vFunction = R5900::Instruction::Execute::PCEQH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PCEQH_CODE
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					e->pcmpeqdregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->pcmpeqwregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PCEQW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PCEQW";
	static const void *c_vFunction = R5900::Instruction::Execute::PCEQW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PCEQW_CODE
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					e->pcmpeqdregreg ( RAX, RAX );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
				else
				{
					e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
					e->pcmpeqdregmem ( RAX, & r->GPR [ i.Rt ].u );
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
				}
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}




static long R5900::Recompiler::PEXTLB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PEXTLB";
	static const void *c_vFunction = R5900::Instruction::Execute::PEXTLB;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PEXTLB_CODE
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
				e->punpcklbwregmem ( RAX, & r->GPR [ i.Rs ].u );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PEXTLH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PEXTLH";
	static const void *c_vFunction = R5900::Instruction::Execute::PEXTLH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PEXTLH_CODE
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
				e->punpcklwdregmem ( RAX, & r->GPR [ i.Rs ].u );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PEXTLW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PEXTLW";
	static const void *c_vFunction = R5900::Instruction::Execute::PEXTLW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PEXTLW_CODE
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
				e->punpckldqregmem ( RAX, & r->GPR [ i.Rs ].u );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PEXTUB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PEXTUB";
	static const void *c_vFunction = R5900::Instruction::Execute::PEXTUB;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PEXTUB_CODE
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
				e->punpckhbwregmem ( RAX, & r->GPR [ i.Rs ].u );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PEXTUH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PEXTUH";
	static const void *c_vFunction = R5900::Instruction::Execute::PEXTUH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PEXTUH_CODE
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
				e->punpckhwdregmem ( RAX, & r->GPR [ i.Rs ].u );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PEXTUW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PEXTUW";
	static const void *c_vFunction = R5900::Instruction::Execute::PEXTUW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PEXTUW_CODE
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
				e->punpckhdqregmem ( RAX, & r->GPR [ i.Rs ].u );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}








static long R5900::Recompiler::PMFLO ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMFLO";
	static const void *c_vFunction = R5900::Instruction::Execute::PMFLO;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			//bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PMFLO_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			//e->MovRegReg64 ( RCX, RAX );
			//e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			//e->SubRegReg64 ( RCX, RAX );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->LO.u );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PMFHI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMFHI";
	static const void *c_vFunction = R5900::Instruction::Execute::PMFHI;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			//bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PMFHI_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			//e->MovRegReg64 ( RCX, RAX );
			//e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			//e->SubRegReg64 ( RCX, RAX );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			//e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->HI.u );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PINTH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PINTH";
	static const void *c_vFunction = R5900::Instruction::Execute::PINTH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PINTH_CODE
			if ( i.Rd )
			{
				e->pshufdregmemimm ( RAX, & r->GPR [ i.Rt ].u, ( 1 << 6 ) + ( 0 << 4 ) );
				e->punpckhwdregmem ( RAX, & r->GPR [ i.Rs ].u );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PINTEH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PINTEH";
	static const void *c_vFunction = R5900::Instruction::Execute::PINTEH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PINTEH_CODE
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
				//pshufdregmemimm ( RAX, & r->GPR [ i.Rt ].u, ( 1 << 6 ) + ( 0 << 4 ) );
				e->pslldregimm ( RAX, 16 );
				//e->punpckhwdregmem ( RAX, & r->GPR [ i.Rs ].u );
				e->pblendwregmemimm ( RAX, & r->GPR [ i.Rt ].u, 0x55 );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



// multimedia multiply //


static long R5900::Recompiler::PMADDH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMADDH";
	static const void *c_vFunction = R5900::Instruction::Execute::PMADDH;
	
	static const int c_iMultiplyCycles = 4 / 2;

	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			//bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_PMADDH_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			e->MovRegReg64 ( RCX, RAX );
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			e->SubRegReg64 ( RCX, RAX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
			e->movdqa_regmem ( RCX, & r->GPR [ i.Rt ].u );
			e->movdqa_regreg ( RBX, RAX );
			
			// put low values in A and high values in B
			e->pmullwregreg ( RAX, RCX );
			e->pmulhwregreg ( RBX, RCX );
			
			// get the values to add with Rd
			e->movdqa_regmem ( RDX, & r->LO.u );
			e->movdqa_regmem ( RCX, & r->HI.u );
			e->pshufdregregimm ( RDX, RDX, ( 2 << 2 ) + 0 );
			e->pshufdregregimm ( RCX, RCX, ( 2 << 2 ) + 0 );
			e->punpckhdqregreg ( RDX, RCX );
			
			e->movdqa_regreg ( RCX, RBX );
			e->pslldregimm ( RCX, 16 );
			e->pblendwregregimm ( RCX, RAX, 0x55 );
			
			// do the add
			e->padddregreg ( RCX, RDX );
				
			if ( i.Rd )
			{
				// store Rd
				e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RCX );
			}
				
			e->pshufdregregimm ( RCX, RAX, ( 3 << 6 ) + ( 1 << 4 ) + ( 2 << 2 ) + 0 );
			e->pshufdregregimm ( RDX, RBX, ( 3 << 6 ) + ( 1 << 4 ) + ( 2 << 2 ) + 0 );
			
			// store hi
			e->movdqa_regreg ( RAX, RCX );
			e->punpckhwdregreg ( RAX, RDX );
			
			// add with hi
			e->padddregmem ( RAX, & r->HI.u );
			
			// store to hi
			e->movdqa_memreg ( & r->HI.u, RAX );
			
			// store lo
			e->punpcklwdregreg ( RCX, RDX );
			
			// add with lo
			e->padddregmem ( RCX, & r->LO.u );
			
			// store to lo
			ret = e->movdqa_memreg ( & r->LO.u, RCX );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PMADDW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMADDW";
	static const void *c_vFunction = R5900::Instruction::Execute::PMADDW;
	
	static const int c_iMultiplyCycles = 4 / 2;

	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			//bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_PMADDW_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			e->MovRegReg64 ( RCX, RAX );
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			e->SubRegReg64 ( RCX, RAX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
			e->movdqa_regmem ( RCX, & r->GPR [ i.Rt ].u );
			
			// do the multiply
			e->pmuldqregreg ( RAX, RCX );
			
			// get the values to add from hi/lo
			e->movdqa_regmem ( RCX, & r->LO.u );
			e->movdqa_regmem ( RDX, & r->HI.u );
			e->pshufdregregimm ( RCX, RCX, ( 2 << 2 ) + 0 );
			e->pshufdregregimm ( RDX, RDX, ( 2 << 2 ) + 0 );
			e->punpckldqregreg ( RCX, RDX );
			
			// add the values
			e->paddqregreg ( RAX, RCX );
				
			if ( i.Rd )
			{
				// store 64-bit results
				e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
				
			// get the hi result
			e->pshufdregregimm ( RCX, RAX, ( 3 << 2 ) + ( 1 << 0 ) );
			e->pmovsxdqregreg ( RCX, RCX );
			e->movdqa_memreg ( & r->HI.u, RCX );
			
			// get the lo result
			e->pshufdregregimm ( RCX, RAX, ( 2 << 2 ) + ( 0 << 0 ) );
			e->pmovsxdqregreg ( RCX, RCX );
			ret = e->movdqa_memreg ( & r->LO.u, RCX );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PMADDUW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMADDUW";
	static const void *c_vFunction = R5900::Instruction::Execute::PMADDUW;
	
	static const int c_iMultiplyCycles = 4 / 2;

	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			//bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_PMADDUW_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			e->MovRegReg64 ( RCX, RAX );
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			e->SubRegReg64 ( RCX, RAX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
			e->movdqa_regmem ( RCX, & r->GPR [ i.Rt ].u );
			
			// do the multiply
			e->pmuludqregreg ( RAX, RCX );
			
			// get the values to add from hi/lo
			e->movdqa_regmem ( RCX, & r->LO.u );
			e->movdqa_regmem ( RDX, & r->HI.u );
			e->pshufdregregimm ( RCX, RCX, ( 2 << 2 ) + 0 );
			e->pshufdregregimm ( RDX, RDX, ( 2 << 2 ) + 0 );
			e->punpckldqregreg ( RCX, RDX );
			
			// add the values
			e->paddqregreg ( RAX, RCX );
				
			if ( i.Rd )
			{
				// store 64-bit results
				e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
				
			// get the hi result
			e->pshufdregregimm ( RCX, RAX, ( 3 << 2 ) + ( 1 << 0 ) );
			e->pmovsxdqregreg ( RCX, RCX );
			e->movdqa_memreg ( & r->HI.u, RCX );
			
			// get the lo result
			e->pshufdregregimm ( RCX, RAX, ( 2 << 2 ) + ( 0 << 0 ) );
			e->pmovsxdqregreg ( RCX, RCX );
			ret = e->movdqa_memreg ( & r->LO.u, RCX );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PMSUBH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMSUBH";
	static const void *c_vFunction = R5900::Instruction::Execute::PMSUBH;
	
	static const int c_iMultiplyCycles = 4 / 2;

	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			//bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_PMSUBH_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			e->MovRegReg64 ( RCX, RAX );
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			e->SubRegReg64 ( RCX, RAX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
			e->movdqa_regmem ( RCX, & r->GPR [ i.Rt ].u );
			e->movdqa_regreg ( RBX, RAX );
			
			// put low values in A and high values in B
			e->pmullwregreg ( RAX, RCX );
			e->pmulhwregreg ( RBX, RCX );
			
			// get the values to add with Rd
			e->movdqa_regmem ( RDX, & r->LO.u );
			e->movdqa_regmem ( RCX, & r->HI.u );
			e->pshufdregregimm ( RDX, RDX, ( 2 << 2 ) + 0 );
			e->pshufdregregimm ( RCX, RCX, ( 2 << 2 ) + 0 );
			e->punpckhdqregreg ( RDX, RCX );
			
			e->movdqa_regreg ( RCX, RBX );
			e->pslldregimm ( RCX, 16 );
			e->pblendwregregimm ( RCX, RAX, 0x55 );
			
			// do the sub
			e->psubdregreg ( RDX, RCX );
				
			if ( i.Rd )
			{
				// store Rd
				e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RDX );
			}
				
			e->pshufdregregimm ( RCX, RAX, ( 3 << 6 ) + ( 1 << 4 ) + ( 2 << 2 ) + 0 );
			e->pshufdregregimm ( RDX, RBX, ( 3 << 6 ) + ( 1 << 4 ) + ( 2 << 2 ) + 0 );
			
			// store hi
			e->movdqa_regreg ( RAX, RCX );
			e->punpckhwdregreg ( RAX, RDX );
			
			// sub with hi
			e->movdqa_regmem ( RBX, & r->HI.u );
			e->psubdregreg ( RBX, RAX );
			
			// store to hi
			e->movdqa_memreg ( & r->HI.u, RBX );
			
			// store lo
			e->punpcklwdregreg ( RCX, RDX );
			
			// sub with lo
			e->movdqa_regmem ( RBX, & r->HI.u );
			e->psubdregreg ( RBX, RCX );
			
			// store to lo
			ret = e->movdqa_memreg ( & r->LO.u, RBX );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



static long R5900::Recompiler::PMSUBW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMSUBW";
	static const void *c_vFunction = R5900::Instruction::Execute::PMSUBW;
	
	static const int c_iMultiplyCycles = 4 / 2;

	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			//bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_PMSUBW_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			e->MovRegReg64 ( RCX, RAX );
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			e->SubRegReg64 ( RCX, RAX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
			e->movdqa_regmem ( RCX, & r->GPR [ i.Rt ].u );
			
			// do the multiply
			e->pmuldqregreg ( RAX, RCX );
			
			// get the values to add from hi/lo
			e->movdqa_regmem ( RCX, & r->LO.u );
			e->movdqa_regmem ( RDX, & r->HI.u );
			e->pshufdregregimm ( RCX, RCX, ( 2 << 2 ) + 0 );
			e->pshufdregregimm ( RDX, RDX, ( 2 << 2 ) + 0 );
			e->punpckldqregreg ( RCX, RDX );
			
			// add the values
			e->psubqregreg ( RCX, RAX );
				
			if ( i.Rd )
			{
				// store 64-bit results
				e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RCX );
			}
				
			// get the hi result
			e->pshufdregregimm ( RAX, RCX, ( 3 << 2 ) + ( 1 << 0 ) );
			e->pmovsxdqregreg ( RAX, RAX );
			e->movdqa_memreg ( & r->HI.u, RAX );
			
			// get the lo result
			e->pshufdregregimm ( RAX, RCX, ( 2 << 2 ) + ( 0 << 0 ) );
			e->pmovsxdqregreg ( RAX, RAX );
			ret = e->movdqa_memreg ( & r->LO.u, RAX );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PMULTH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMULTH";
	static const void *c_vFunction = R5900::Instruction::Execute::PMULTH;
	
	static const int c_iMultiplyCycles = 4 / 2;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			//bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_PMULTH_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			e->MovRegReg64 ( RCX, RAX );
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			e->SubRegReg64 ( RCX, RAX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
			e->movdqa_regmem ( RCX, & r->GPR [ i.Rt ].u );
			e->movdqa_regreg ( RBX, RAX );
			
			// put low values in A and high values in B
			e->pmullwregreg ( RAX, RCX );
			e->pmulhwregreg ( RBX, RCX );
				
			if ( i.Rd )
			{
				e->movdqa_regreg ( RCX, RBX );
				e->pslldregimm ( RCX, 16 );
				e->pblendwregregimm ( RCX, RAX, 0x55 );
				e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RCX );
			}
				
			e->pshufdregregimm ( RCX, RAX, ( 3 << 6 ) + ( 1 << 4 ) + ( 2 << 2 ) + 0 );
			e->pshufdregregimm ( RDX, RBX, ( 3 << 6 ) + ( 1 << 4 ) + ( 2 << 2 ) + 0 );
			
			// store hi
			e->movdqa_regreg ( RAX, RCX );
			e->punpckhwdregreg ( RAX, RDX );
			e->movdqa_memreg ( & r->HI.u, RAX );
			
			// store lo
			e->punpcklwdregreg ( RCX, RDX );
			ret = e->movdqa_memreg ( & r->LO.u, RCX );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PMULTW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMULTW";
	static const void *c_vFunction = R5900::Instruction::Execute::PMULTW;
	
	static const int c_iMultiplyCycles = 4 / 2;

	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			//bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_PMULTW_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			e->MovRegReg64 ( RCX, RAX );
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			e->SubRegReg64 ( RCX, RAX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
			e->movdqa_regmem ( RCX, & r->GPR [ i.Rt ].u );
			
			// do the multiply
			e->pmuldqregreg ( RAX, RCX );
				
			if ( i.Rd )
			{
				// store 64-bit results
				e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
				
			// get the hi result
			e->pshufdregregimm ( RCX, RAX, ( 3 << 2 ) + ( 1 << 0 ) );
			e->pmovsxdqregreg ( RCX, RCX );
			e->movdqa_memreg ( & r->HI.u, RCX );
			
			// get the lo result
			e->pshufdregregimm ( RCX, RAX, ( 2 << 2 ) + ( 0 << 0 ) );
			e->pmovsxdqregreg ( RCX, RCX );
			ret = e->movdqa_memreg ( & r->LO.u, RCX );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PMULTUW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMULTUW";
	static const void *c_vFunction = R5900::Instruction::Execute::PMULTUW;
	
	static const int c_iMultiplyCycles = 4 / 2;

	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			//bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_PMULTUW_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			e->MovRegReg64 ( RCX, RAX );
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			e->SubRegReg64 ( RCX, RAX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
			e->movdqa_regmem ( RCX, & r->GPR [ i.Rt ].u );
			
			// do the multiply
			e->pmuludqregreg ( RAX, RCX );
				
			if ( i.Rd )
			{
				// store 64-bit results
				e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
				
			// get the hi result
			e->pshufdregregimm ( RCX, RAX, ( 3 << 2 ) + ( 1 << 0 ) );
			e->pmovsxdqregreg ( RCX, RCX );
			e->movdqa_memreg ( & r->HI.u, RCX );
			
			// get the lo result
			e->pshufdregregimm ( RCX, RAX, ( 2 << 2 ) + ( 0 << 0 ) );
			e->pmovsxdqregreg ( RCX, RCX );
			ret = e->movdqa_memreg ( & r->LO.u, RCX );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}






static long R5900::Recompiler::PHMADH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PHMADH";
	static const void *c_vFunction = R5900::Instruction::Execute::PHMADH;
	
	static const int c_iMultiplyCycles = 4 / 2;

	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_PHMADH_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			e->MovRegReg64 ( RCX, RAX );
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			e->SubRegReg64 ( RCX, RAX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
			e->movdqa_regmem ( RCX, & r->GPR [ i.Rt ].u );
			
			// do the multiply-add
			e->pmaddwdregreg ( RAX, RCX );
				
			if ( i.Rd )
			{
				// store result to Rd
				e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
				
			// get hi result and store to hi
			e->pshufdregregimm ( RCX, RAX, ( 1 << 2 ) + ( 3 << 0 ) );
			e->movdqa_memreg ( & r->HI.u, RCX );
			
			// get lo result and store to lo
			e->pshufdregregimm ( RCX, RAX, ( 2 << 2 ) + ( 0 << 0 ) );
			ret = e->movdqa_memreg ( & r->LO.u, RCX );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



static long R5900::Recompiler::PHMSBH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PHMSBH";
	static const void *c_vFunction = R5900::Instruction::Execute::PHMSBH;
	
	static const int c_iMultiplyCycles = 4 / 2;

	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_PHMSBH_CODE
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			e->MovRegReg64 ( RCX, RAX );
			e->AddReg64ImmX ( RCX, c_iMultiplyCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			e->SubRegReg64 ( RCX, RAX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
			e->movdqa_regmem ( RCX, & r->GPR [ i.Rt ].u );
			
			// even 16-bit values on the right side need to be negated
			e->pxorregreg ( RDX, RDX );
			e->psubwregreg ( RDX, RCX );
			e->pblendwregregimm ( RCX, RDX, 0x55 );
			
			// do the multiply-add
			e->pmaddwdregreg ( RAX, RCX );
				
			if ( i.Rd )
			{
				// store result to Rd
				e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
				
			// get hi result and store to hi
			e->pshufdregregimm ( RCX, RAX, ( 1 << 2 ) + ( 3 << 0 ) );
			e->movdqa_memreg ( & r->HI.u, RCX );
			
			// get lo result and store to lo
			e->pshufdregregimm ( RCX, RAX, ( 2 << 2 ) + ( 0 << 0 ) );
			ret = e->movdqa_memreg ( & r->LO.u, RCX );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



// multimedia divide //

static long R5900::Recompiler::PDIVW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PDIVW";
	static const void *c_vFunction = R5900::Instruction::Execute::PDIVW;
	
	// 37 cycles
	// divide by 2 here since r5900 is currently only running at bus speed for testing
	static const int c_iDivideCycles = 37 / 2;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			//bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_PDIVW_CODE
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			e->MovRegReg64 ( RCX, RAX );
			e->AddReg64ImmX ( RCX, c_iDivideCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			e->SubRegReg64 ( RCX, RAX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			// now do the division //
			
			e->MovsxdReg64Mem32 ( RCX, & r->GPR [ i.Rt ].sw0 );
			e->MovsxdReg64Mem32 ( RAX, & r->GPR [ i.Rs ].sw0 );
			//e->OrRegReg64 ( RCX, RCX );
			//e->Jmp8_E ( 0, 0 );
			e->Jmp8_ECXZ ( 0, 0 );
			
			//e->MovRegReg64 ( RDX, RAX );
			//e->SarRegImm64 ( RDX, 63 );
			e->Cqo ();
			e->IdivRegReg64 ( RCX );
			// *** todo for ps2 *** //
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->Cdqe ();
			e->MovMemReg64 ( & r->LO.s, RAX );
			//e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->MovsxdReg64Reg32 ( RDX, RDX );
			e->MovMemReg64 ( & r->HI.s, RDX );
			e->Jmp8 ( 0, 1 );
			
			e->SetJmpTarget8 ( 0 );
			
			//e->MovReg32ImmX ( RCX, -1 );
			//e->MovReg32ImmX ( RDX, 1 );
			e->MovReg64ImmX ( RCX, -1 );
			e->MovReg64ImmX ( RDX, 1 );
			e->OrRegReg32 ( RAX, RAX );
			//e->CmovSRegReg32 ( RCX, RDX );
			e->CmovSRegReg64 ( RCX, RDX );
			// *** todo for ps2 *** //
			//e->MovMemReg32 ( & r->HiLo.uHi, RAX );
			e->MovMemReg64 ( & r->HI.u, RAX );
			//e->MovMemReg32 ( & r->HiLo.uLo, RCX );
			e->MovMemReg64 ( & r->LO.u, RCX );
			
			e->SetJmpTarget8 ( 1 );

			e->MovsxdReg64Mem32 ( RCX, & r->GPR [ i.Rt ].sw2 );
			e->MovsxdReg64Mem32 ( RAX, & r->GPR [ i.Rs ].sw2 );
			//e->OrRegReg64 ( RCX, RCX );
			//e->Jmp8_E ( 0, 0 );
			e->Jmp8_ECXZ ( 0, 0 );
			
			//e->MovRegReg64 ( RDX, RAX );
			//e->SarRegImm64 ( RDX, 63 );
			e->Cqo ();
			e->IdivRegReg64 ( RCX );
			// *** todo for ps2 *** //
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->Cdqe ();
			e->MovMemReg64 ( & r->LO.sq1, RAX );
			//e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->MovsxdReg64Reg32 ( RDX, RDX );
			e->MovMemReg64 ( & r->HI.sq1, RDX );
			e->Jmp8 ( 0, 1 );
			
			e->SetJmpTarget8 ( 0 );
			
			//e->MovReg32ImmX ( RCX, -1 );
			//e->MovReg32ImmX ( RDX, 1 );
			e->MovReg64ImmX ( RCX, -1 );
			e->MovReg64ImmX ( RDX, 1 );
			e->OrRegReg32 ( RAX, RAX );
			//e->CmovSRegReg32 ( RCX, RDX );
			e->CmovSRegReg64 ( RCX, RDX );
			// *** todo for ps2 *** //
			//e->MovMemReg32 ( & r->HiLo.uHi, RAX );
			e->MovMemReg64 ( & r->HI.uq1, RAX );
			//e->MovMemReg32 ( & r->HiLo.uLo, RCX );
			e->MovMemReg64 ( & r->LO.uq1, RCX );
			
			e->SetJmpTarget8 ( 1 );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PDIVUW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PDIVUW";
	static const void *c_vFunction = R5900::Instruction::Execute::PDIVUW;
	
	// 37 cycles
	// divide by 2 here since r5900 is currently only running at bus speed for testing
	static const int c_iDivideCycles = 37 / 2;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			//bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_PDIVUW_CODE
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			e->MovRegReg64 ( RCX, RAX );
			e->AddReg64ImmX ( RCX, c_iDivideCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			e->SubRegReg64 ( RCX, RAX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			// now do the division //
			
			e->MovRegMem32 ( RCX, & r->GPR [ i.Rt ].uw0 );
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].uw0 );
			//e->OrRegReg64 ( RCX, RCX );
			//e->Jmp8_E ( 0, 0 );
			e->Jmp8_ECXZ ( 0, 0 );
			
			e->XorRegReg32 ( RDX, RDX );
			e->DivRegReg32 ( RCX );
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->Cdqe ();
			e->MovMemReg64 ( & r->LO.s, RAX );
			//e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->MovsxdReg64Reg32 ( RDX, RDX );
			e->MovMemReg64 ( & r->HI.s, RDX );
			e->Jmp8 ( 0, 1 );
			
			e->SetJmpTarget8 ( 0 );
			
			//e->MovMemImm32 ( & r->HiLo.sLo, -1 );
			//e->MovMemReg32 ( & r->HiLo.uHi, RAX );
			e->MovMemImm64 ( & r->LO.s, -1 );
			e->MovMemReg64 ( & r->HI.u, RAX );
			
			e->SetJmpTarget8 ( 1 );

			
			e->MovRegMem32 ( RCX, & r->GPR [ i.Rt ].uw2 );
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].uw2 );
			//e->OrRegReg64 ( RCX, RCX );
			//e->Jmp8_E ( 0, 0 );
			e->Jmp8_ECXZ ( 0, 0 );
			
			e->XorRegReg32 ( RDX, RDX );
			e->DivRegReg32 ( RCX );
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->Cdqe ();
			e->MovMemReg64 ( & r->LO.sq1, RAX );
			//e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->MovsxdReg64Reg32 ( RDX, RDX );
			e->MovMemReg64 ( & r->HI.sq1, RDX );
			e->Jmp8 ( 0, 1 );
			
			e->SetJmpTarget8 ( 0 );
			
			//e->MovMemImm32 ( & r->HiLo.sLo, -1 );
			//e->MovMemReg32 ( & r->HiLo.uHi, RAX );
			e->MovMemImm64 ( & r->LO.sq1, -1 );
			e->MovMemReg64 ( & r->HI.uq1, RAX );
			
			e->SetJmpTarget8 ( 1 );

#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PDIVBW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PDIVBW";
	static const void *c_vFunction = R5900::Instruction::Execute::PDIVBW;
	
	// 37 cycles
	// divide by 2 here since r5900 is currently only running at bus speed for testing
	static const int c_iDivideCycles = 37 / 2;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			//bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_PDIVBW_CODE
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			
			// get maximum of busy until cycle in RDX (to be used as the cycle# Mul/Div unit is busy until)
			e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->MovRegMem64 ( RDX, & r->MulDiv_BusyUntil_Cycle1 );
			e->CmpRegReg64 ( RDX, RCX );
			e->CmovBRegReg64 ( RDX, RCX );
			
			// get current cyclecount in RAX
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// save current cyclecount into RCX + cycles for divide
			e->MovRegReg64 ( RCX, RAX );
			e->AddReg64ImmX ( RCX, c_iDivideCycles );
			
			// get any cycles between current cyclecount and when Mul/Div unit is available
			e->SubRegReg64 ( RAX, RDX );
			e->Cqo ();
			e->AndRegReg64 ( RAX, RDX );
			
			// store the current cyclecount + time to Mul/Div unit available + cycles for divide
			e->SubRegReg64 ( RCX, RAX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle1, RCX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// offset current cyclecount with the number of cycles until mul/div unit is available
			//e->SubMemReg64 ( & r->CycleCount, RDX );
			e->SubMemReg64 ( & r->CycleCount, RAX );
			
			// now do the division //
			
			e->MovRegMem32 ( RCX, & r->GPR [ i.Rt ].sw0 );
			e->MovsxdReg64Mem32 ( RAX, & r->GPR [ i.Rs ].sw0 );
			
			// sign-extend the word
			e->MovsxReg64Reg16 ( RCX, RCX );
			
			e->Jmp8_ECXZ ( 0, 0 );
			
			e->Cqo ();
			e->IdivRegReg64 ( RCX );
			
			e->MovMemReg32 ( & r->LO.sw0, RAX );
			
			e->MovsxReg32Reg16 ( RDX, RDX );
			e->MovMemReg32 ( & r->HI.sw0, RDX );
			e->Jmp8 ( 0, 1 );
			
			e->SetJmpTarget8 ( 0 );
			
			e->MovReg32ImmX ( RCX, -1 );
			e->MovReg32ImmX ( RDX, 1 );
			e->OrRegReg32 ( RAX, RAX );
			e->CmovSRegReg32 ( RCX, RDX );
			
			e->Cwde ();
			e->MovMemReg32 ( & r->HI.uw0, RAX );
			e->MovMemReg32 ( & r->LO.uw0, RCX );
			
			
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].sw1 );
			e->MovReg32ImmX ( RCX, -1 );
			e->OrRegReg32 ( RAX, RAX );
			e->CmovSRegReg32 ( RCX, RDX );
			e->Cwde ();
			e->MovMemReg32 ( & r->HI.uw0, RAX );
			e->MovMemReg32 ( & r->LO.uw0, RCX );

			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].sw2 );
			e->MovReg32ImmX ( RCX, -1 );
			e->OrRegReg32 ( RAX, RAX );
			e->CmovSRegReg32 ( RCX, RDX );
			e->Cwde ();
			e->MovMemReg32 ( & r->HI.uw0, RAX );
			e->MovMemReg32 ( & r->LO.uw0, RCX );

			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].sw3 );
			e->MovReg32ImmX ( RCX, -1 );
			e->OrRegReg32 ( RAX, RAX );
			e->CmovSRegReg32 ( RCX, RDX );
			e->Cwde ();
			e->MovMemReg32 ( & r->HI.uw0, RAX );
			e->MovMemReg32 ( & r->LO.uw0, RCX );
			
			e->Jmp8 ( 0, 0 );
			
			e->SetJmpTarget8 ( 1 );

			e->MovsxdReg64Mem32 ( RAX, & r->GPR [ i.Rs ].sw1 );
			
			e->Cqo ();
			e->IdivRegReg64 ( RCX );
			
			e->MovMemReg32 ( & r->LO.sw1, RAX );
			e->MovsxReg32Reg16 ( RDX, RDX );
			e->MovMemReg32 ( & r->HI.sw1, RDX );
			
			e->MovsxdReg64Mem32 ( RAX, & r->GPR [ i.Rs ].sw2 );
			
			e->Cqo ();
			e->IdivRegReg64 ( RCX );
			
			e->MovMemReg32 ( & r->LO.sw2, RAX );
			e->MovsxReg32Reg16 ( RDX, RDX );
			e->MovMemReg32 ( & r->HI.sw2, RDX );
			
			e->MovsxdReg64Mem32 ( RAX, & r->GPR [ i.Rs ].sw3 );
			
			e->Cqo ();
			e->IdivRegReg64 ( RCX );
			
			e->MovMemReg32 ( & r->LO.sw3, RAX );
			e->MovsxReg32Reg16 ( RDX, RDX );
			e->MovMemReg32 ( & r->HI.sw3, RDX );
			
			e->SetJmpTarget8 ( 0 );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}




static long R5900::Recompiler::PREVH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PREVH";
	static const void *c_vFunction = R5900::Instruction::Execute::PREVH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PREVH_CODE
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
				//e->pshufhwregmemimm ( RAX, & r->GPR [ i.Rt ].u, ( 0 << 6 ) + ( 1 << 4 ) + ( 2 << 2 ) + ( 3 ) );
				e->pshufhwregregimm ( RAX, RAX, ( 0 << 6 ) + ( 1 << 4 ) + ( 2 << 2 ) + ( 3 ) );
				e->pshuflwregregimm ( RAX, RAX, ( 0 << 6 ) + ( 1 << 4 ) + ( 2 << 2 ) + ( 3 ) );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;

		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}




static long R5900::Recompiler::PEXEH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PEXEH";
	static const void *c_vFunction = R5900::Instruction::Execute::PEXEH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PEXEH_CODE
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
				//e->pshufhwregmemimm ( RAX, & r->GPR [ i.Rt ].u, ( 3 << 6 ) + ( 0 << 4 ) + ( 1 << 2 ) + ( 2 ) );
				e->pshufhwregregimm ( RAX, RAX, ( 3 << 6 ) + ( 0 << 4 ) + ( 1 << 2 ) + ( 2 ) );
				e->pshuflwregregimm ( RAX, RAX, ( 3 << 6 ) + ( 0 << 4 ) + ( 1 << 2 ) + ( 2 ) );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;

		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PEXEW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PEXEW";
	static const void *c_vFunction = R5900::Instruction::Execute::PEXEW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PEXEW_CODE
			if ( i.Rd )
			{
				e->pshufdregmemimm ( RAX, & r->GPR [ i.Rt ].u, ( 3 << 6 ) + ( 0 << 4 ) + ( 1 << 2 ) + ( 2 ) );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;

		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PROT3W ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PROT3W";
	static const void *c_vFunction = R5900::Instruction::Execute::PROT3W;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PROT3W_CODE
			if ( i.Rd )
			{
				e->pshufdregmemimm ( RAX, & r->GPR [ i.Rt ].u, ( 3 << 6 ) + ( 0 << 4 ) + ( 2 << 2 ) + ( 1 ) );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;

		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



static long R5900::Recompiler::PMTHI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMTHI";
	static const void *c_vFunction = R5900::Instruction::Execute::PMTHI;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_PMTHI_CODE
			if ( i.Rs )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
				ret = e->movdqa_memreg ( & r->HI.u, RAX );
			}
			else
			{
				e->pxorregreg ( RAX, RAX );
				ret = e->movdqa_memreg ( & r->HI.u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PMTLO ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PMTLO";
	static const void *c_vFunction = R5900::Instruction::Execute::PMTLO;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_PMTLO_CODE
			if ( i.Rs )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
				ret = e->movdqa_memreg ( & r->LO.u, RAX );
			}
			else
			{
				e->pxorregreg ( RAX, RAX );
				ret = e->movdqa_memreg ( & r->LO.u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



static long R5900::Recompiler::PCPYLD ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PCPYLD";
	static const void *c_vFunction = R5900::Instruction::Execute::PCPYLD;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PCPYLD_CODE
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
				e->punpcklqdqregmem ( RAX, & r->GPR [ i.Rs ].u );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;

		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PCPYUD ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PCPYUD";
	static const void *c_vFunction = R5900::Instruction::Execute::PCPYUD;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PCPYUD_CODE
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rs ].u );
				e->punpckhqdqregmem ( RAX, & r->GPR [ i.Rt ].u );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;

		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PCPYH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PCPYH";
	static const void *c_vFunction = R5900::Instruction::Execute::PCPYH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PCPYH_CODE
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
				//e->pshufhwregmemimm ( RAX, & r->GPR [ i.Rt ].u, 0 );
				e->pshufhwregregimm ( RAX, RAX, 0 );
				e->pshuflwregregimm ( RAX, RAX, 0 );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::PEXCH ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PEXCH";
	static const void *c_vFunction = R5900::Instruction::Execute::PEXCH;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PEXCH_CODE
			if ( i.Rd )
			{
				e->movdqa_regmem ( RAX, & r->GPR [ i.Rt ].u );
				//e->pshufhwregmemimm ( RAX, & r->GPR [ i.Rt ].u, ( 3 << 6 ) + ( 1 << 4 ) + ( 2 << 2 ) + ( 0 ) );
				e->pshufhwregregimm ( RAX, RAX, ( 3 << 6 ) + ( 1 << 4 ) + ( 2 << 2 ) + ( 0 ) );
				e->pshuflwregregimm ( RAX, RAX, ( 3 << 6 ) + ( 1 << 4 ) + ( 2 << 2 ) + ( 0 ) );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PEXCW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PEXCW";
	static const void *c_vFunction = R5900::Instruction::Execute::PEXCW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_PEXCW_CODE
			if ( i.Rd )
			{
				e->pshufdregmemimm ( RAX, & r->GPR [ i.Rt ].u, ( 3 << 6 ) + ( 1 << 4 ) + ( 2 << 2 ) + ( 0 ) );
				ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RAX );
			}
#else
			return -1;
#endif
			break;

		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::QFSRV ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "QFSRV";
	static const void *c_vFunction = R5900::Instruction::Execute::QFSRV;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
#ifdef USE_NEW_QFSRV_CODE
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					e->MovRegMem32 ( RCX, &r->SA );
					e->MovRegImm64 ( RAX, (long long) recompiler_qfsrv_shift_table_rev );
					e->AddRegReg32 ( RCX, RCX );
					e->movdqa_from_mem128 ( RBX, RAX, RCX, SCALE_EIGHT, 0 );
					
					e->movdqa_regmem ( RCX, & r->GPR [ i.Rs ].u );
					
					e->pshufbregreg ( RCX, RBX );
					
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RCX );
				}
				else
				{
					e->MovRegMem32 ( RCX, & r->SA );
					e->MovRegImm64 ( RAX, (long long) recompiler_qfsrv_shift_table_rev );
					e->AddRegReg32 ( RCX, RCX );
					e->movdqa_from_mem128 ( RBX, RAX, RCX, SCALE_EIGHT, 0 );
					
					e->MovRegImm64 ( RAX, (long long) recompiler_qfsrv_blend_table_rev );
					
					e->movdqa_regmem ( RDX, & r->GPR [ i.Rt ].u );
					e->movdqa_regmem ( RCX, & r->GPR [ i.Rs ].u );
					
					e->movdqa_from_mem128 ( RAX, RAX, RCX, SCALE_EIGHT, 0 );
					
					e->pshufbregreg ( RDX, RBX );
					e->pshufbregreg ( RCX, RBX );
					
					e->pblendvbregreg ( RDX, RCX );
					
					ret = e->movdqa_memreg ( & r->GPR [ i.Rd ].u, RDX );
				}
			}
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}





// * R5900 COP0 instructions * //


static long R5900::Recompiler::EI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "EI";
	static const void *c_vFunction = R5900::Instruction::Execute::EI;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::DI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DI";
	static const void *c_vFunction = R5900::Instruction::Execute::DI;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::CFC0 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "CFC0";
	static const void *c_vFunction = R5900::Instruction::Execute::CFC0;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::CTC0 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "CTC0";
	static const void *c_vFunction = R5900::Instruction::Execute::CTC0;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}




static long R5900::Recompiler::SYNC ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SYNC";
	static const void *c_vFunction = R5900::Instruction::Execute::SYNC;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::CACHE ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "CACHE";
	static const void *c_vFunction = R5900::Instruction::Execute::CACHE;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::PREF ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "PREF";
	static const void *c_vFunction = R5900::Instruction::Execute::PREF;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::TLBR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "TLBR";
	static const void *c_vFunction = R5900::Instruction::Execute::TLBR;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::TLBWI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "TLBWI";
	static const void *c_vFunction = R5900::Instruction::Execute::TLBWI;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::TLBWR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "TLBWR";
	static const void *c_vFunction = R5900::Instruction::Execute::TLBWR;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::TLBP ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "TLBP";
	static const void *c_vFunction = R5900::Instruction::Execute::TLBP;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::ERET ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "ERET";
	static const void *c_vFunction = R5900::Instruction::Execute::ERET;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



static long R5900::Recompiler::DERET ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DERET";
	static const void *c_vFunction = R5900::Instruction::Execute::DERET;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::WAIT ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "WAIT";
	static const void *c_vFunction = R5900::Instruction::Execute::WAIT;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}





// * COP1 (floating point) instructions * //


static long R5900::Recompiler::MFC1 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MFC1";
	static const void *c_vFunction = R5900::Instruction::Execute::MFC1;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MTC1 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MTC1";
	static const void *c_vFunction = R5900::Instruction::Execute::MTC1;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::CFC1 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "CFC1";
	static const void *c_vFunction = R5900::Instruction::Execute::CFC1;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::CTC1 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "CTC1";
	static const void *c_vFunction = R5900::Instruction::Execute::CTC1;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::LWC1 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "LWC1";
	static const void *c_vFunction = R5900::Instruction::Execute::LWC1;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::SWC1 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SWC1";
	static const void *c_vFunction = R5900::Instruction::Execute::SWC1;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}





static long R5900::Recompiler::ABS_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "ABS_S";
	static const void *c_vFunction = R5900::Instruction::Execute::ABS_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::ADD_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "ADD_S";
	static const void *c_vFunction = R5900::Instruction::Execute::ADD_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::ADDA_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "ADDA_S";
	static const void *c_vFunction = R5900::Instruction::Execute::ADDA_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



static long R5900::Recompiler::CVT_S_W ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "CVT_S_W";
	static const void *c_vFunction = R5900::Instruction::Execute::CVT_S_W;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::SUB_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SUB_S";
	static const void *c_vFunction = R5900::Instruction::Execute::SUB_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MUL_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MUL_S";
	static const void *c_vFunction = R5900::Instruction::Execute::MUL_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MULA_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MULA_S";
	static const void *c_vFunction = R5900::Instruction::Execute::MULA_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::DIV_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "DIV_S";
	static const void *c_vFunction = R5900::Instruction::Execute::DIV_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::SQRT_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SQRT_S";
	static const void *c_vFunction = R5900::Instruction::Execute::SQRT_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::RSQRT_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "RSQRT_S";
	static const void *c_vFunction = R5900::Instruction::Execute::RSQRT_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}




static long R5900::Recompiler::MOV_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MOV_S";
	static const void *c_vFunction = R5900::Instruction::Execute::MOV_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::NEG_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "NEG_S";
	static const void *c_vFunction = R5900::Instruction::Execute::NEG_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



static long R5900::Recompiler::SUBA_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SUBA_S";
	static const void *c_vFunction = R5900::Instruction::Execute::SUBA_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MADD_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MADD_S";
	static const void *c_vFunction = R5900::Instruction::Execute::MADD_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MSUB_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MSUB_S";
	static const void *c_vFunction = R5900::Instruction::Execute::MSUB_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MSUBA_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MSUBA_S";
	static const void *c_vFunction = R5900::Instruction::Execute::MSUBA_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MADDA_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MADDA_S";
	static const void *c_vFunction = R5900::Instruction::Execute::MADDA_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::CVT_W_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "CVT_W_S";
	static const void *c_vFunction = R5900::Instruction::Execute::CVT_W_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MAX_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MAX_S";
	static const void *c_vFunction = R5900::Instruction::Execute::MAX_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::MIN_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "MIN_S";
	static const void *c_vFunction = R5900::Instruction::Execute::MIN_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::C_F_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "C_F_S";
	static const void *c_vFunction = R5900::Instruction::Execute::C_F_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::C_EQ_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "C_EQ_S";
	static const void *c_vFunction = R5900::Instruction::Execute::C_EQ_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::C_LT_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "C_LT_S";
	static const void *c_vFunction = R5900::Instruction::Execute::C_LT_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::C_LE_S ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "C_LE_S";
	static const void *c_vFunction = R5900::Instruction::Execute::C_LE_S;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



// * COP2 (VU0) instrutions * //



// PS2 has LQC2/SQC2 instead of LWC2/SWC2 //
static long R5900::Recompiler::LQC2 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "LQC2";
	static const void *c_vFunction = R5900::Instruction::Execute::LQC2;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::SQC2 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "SQC2";
	static const void *c_vFunction = R5900::Instruction::Execute::SQC2;
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


static long R5900::Recompiler::QMFC2_NI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "QMFC2_NI";
	static const void *c_vFunction = R5900::Instruction::Execute::QMFC2_NI;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::QMFC2_I ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "QMFC2_I";
	static const void *c_vFunction = R5900::Instruction::Execute::QMFC2_I;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::QMTC2_NI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "QMTC2_NI";
	static const void *c_vFunction = R5900::Instruction::Execute::QMTC2_NI;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::QMTC2_I ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "QMTC2_I";
	static const void *c_vFunction = R5900::Instruction::Execute::QMTC2_I;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


//static long R5900::Recompiler::COP2 ( R5900::Instruction::Format i, u32 Address )
//{
//}




// VABS //

static long R5900::Recompiler::VABS ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VABS";
	static const void *c_vFunction = R5900::Instruction::Execute::VABS;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


// VADD //

static long R5900::Recompiler::VADD ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VADD";
	static const void *c_vFunction = R5900::Instruction::Execute::VADD;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VADDi ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VADDi";
	static const void *c_vFunction = R5900::Instruction::Execute::VADDi;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VADDq ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VADDq";
	static const void *c_vFunction = R5900::Instruction::Execute::VADDq;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VADDBCX ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VADDBCX";
	static const void *c_vFunction = R5900::Instruction::Execute::VADDBCX;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VADDBCY ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VADDBCY";
	static const void *c_vFunction = R5900::Instruction::Execute::VADDBCY;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VADDBCZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VADDBCZ";
	static const void *c_vFunction = R5900::Instruction::Execute::VADDBCZ;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VADDBCW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VADDBCW";
	static const void *c_vFunction = R5900::Instruction::Execute::VADDBCW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


// VADDA //

static long R5900::Recompiler::VADDA ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VADDA";
	static const void *c_vFunction = R5900::Instruction::Execute::VADDA;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VADDAi ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VADDAi";
	static const void *c_vFunction = R5900::Instruction::Execute::VADDAi;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VADDAq ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VADDAq";
	static const void *c_vFunction = R5900::Instruction::Execute::VADDAq;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VADDABCX ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VADDABCX";
	static const void *c_vFunction = R5900::Instruction::Execute::VADDABCX;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VADDABCY ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VADDABCY";
	static const void *c_vFunction = R5900::Instruction::Execute::VADDABCY;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VADDABCZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VADDABCZ";
	static const void *c_vFunction = R5900::Instruction::Execute::VADDABCZ;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VADDABCW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VADDABCW";
	static const void *c_vFunction = R5900::Instruction::Execute::VADDABCW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}





// VSUB //

static long R5900::Recompiler::VSUB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSUB";
	static const void *c_vFunction = R5900::Instruction::Execute::VSUB;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VSUBi ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSUBi";
	static const void *c_vFunction = R5900::Instruction::Execute::VSUBi;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VSUBq ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSUBq";
	static const void *c_vFunction = R5900::Instruction::Execute::VSUBq;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VSUBBCX ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSUBBCX";
	static const void *c_vFunction = R5900::Instruction::Execute::VSUBBCX;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VSUBBCY ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSUBBCY";
	static const void *c_vFunction = R5900::Instruction::Execute::VSUBBCY;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VSUBBCZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSUBBCZ";
	static const void *c_vFunction = R5900::Instruction::Execute::VSUBBCZ;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VSUBBCW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSUBBCW";
	static const void *c_vFunction = R5900::Instruction::Execute::VSUBBCW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}




// VMADD //

static long R5900::Recompiler::VMADD ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMADD";
	static const void *c_vFunction = R5900::Instruction::Execute::VMADD;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMADDi ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMADDi";
	static const void *c_vFunction = R5900::Instruction::Execute::VMADDi;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMADDq ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMADDq";
	static const void *c_vFunction = R5900::Instruction::Execute::VMADDq;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMADDBCX ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMADDBCX";
	static const void *c_vFunction = R5900::Instruction::Execute::VMADDBCX;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMADDBCY ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMADDBCY";
	static const void *c_vFunction = R5900::Instruction::Execute::VMADDBCY;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMADDBCZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMADDBCZ";
	static const void *c_vFunction = R5900::Instruction::Execute::VMADDBCZ;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMADDBCW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMADDBCW";
	static const void *c_vFunction = R5900::Instruction::Execute::VMADDBCW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}




// VMSUB //

static long R5900::Recompiler::VMSUB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMSUB";
	static const void *c_vFunction = R5900::Instruction::Execute::VMSUB;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMSUBi ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMSUBi";
	static const void *c_vFunction = R5900::Instruction::Execute::VMSUBi;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMSUBq ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMSUBq";
	static const void *c_vFunction = R5900::Instruction::Execute::VMSUBq;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMSUBBCX ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMSUBBCX";
	static const void *c_vFunction = R5900::Instruction::Execute::VMSUBBCX;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMSUBBCY ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMSUBBCY";
	static const void *c_vFunction = R5900::Instruction::Execute::VMSUBBCY;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMSUBBCZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMSUBBCZ";
	static const void *c_vFunction = R5900::Instruction::Execute::VMSUBBCZ;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMSUBBCW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMSUBBCW";
	static const void *c_vFunction = R5900::Instruction::Execute::VMSUBBCW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}




// VMAX //

static long R5900::Recompiler::VMAX ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMAX";
	static const void *c_vFunction = R5900::Instruction::Execute::VMAX;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMAXi ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMAXi";
	static const void *c_vFunction = R5900::Instruction::Execute::VMAXi;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMAXBCX ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMAXBCX";
	static const void *c_vFunction = R5900::Instruction::Execute::VMAXBCX;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMAXBCY ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMAXBCY";
	static const void *c_vFunction = R5900::Instruction::Execute::VMAXBCY;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMAXBCZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMAXBCZ";
	static const void *c_vFunction = R5900::Instruction::Execute::VMAXBCZ;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMAXBCW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMAXBCW";
	static const void *c_vFunction = R5900::Instruction::Execute::VMAXBCW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}




// VMINI //

static long R5900::Recompiler::VMINI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMINI";
	static const void *c_vFunction = R5900::Instruction::Execute::VMINI;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMINIi ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMINIi";
	static const void *c_vFunction = R5900::Instruction::Execute::VMINIi;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMINIBCX ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMINIBCX";
	static const void *c_vFunction = R5900::Instruction::Execute::VMINIBCX;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMINIBCY ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMINIBCY";
	static const void *c_vFunction = R5900::Instruction::Execute::VMINIBCY;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMINIBCZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMINIBCZ";
	static const void *c_vFunction = R5900::Instruction::Execute::VMINIBCZ;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMINIBCW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMINIBCW";
	static const void *c_vFunction = R5900::Instruction::Execute::VMINIBCW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}




// VMUL //

static long R5900::Recompiler::VMUL ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMUL";
	static const void *c_vFunction = R5900::Instruction::Execute::VMUL;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMULi ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMULi";
	static const void *c_vFunction = R5900::Instruction::Execute::VMULi;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMULq ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMULq";
	static const void *c_vFunction = R5900::Instruction::Execute::VMULq;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMULBCX ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMULBCX";
	static const void *c_vFunction = R5900::Instruction::Execute::VMULBCX;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMULBCY ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMULBCY";
	static const void *c_vFunction = R5900::Instruction::Execute::VMULBCY;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMULBCZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMULBCZ";
	static const void *c_vFunction = R5900::Instruction::Execute::VMULBCZ;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMULBCW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMULBCW";
	static const void *c_vFunction = R5900::Instruction::Execute::VMULBCW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}






static long R5900::Recompiler::VOPMSUB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VOPMSUB";
	static const void *c_vFunction = R5900::Instruction::Execute::VOPMSUB;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VIADD ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VIADD";
	static const void *c_vFunction = R5900::Instruction::Execute::VIADD;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VISUB ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VISUB";
	static const void *c_vFunction = R5900::Instruction::Execute::VISUB;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VIADDI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VIADDI";
	static const void *c_vFunction = R5900::Instruction::Execute::VIADDI;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VIAND ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VIAND";
	static const void *c_vFunction = R5900::Instruction::Execute::VIAND;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VIOR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VIOR";
	static const void *c_vFunction = R5900::Instruction::Execute::VIOR;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



// VCALLMS //

static long R5900::Recompiler::VCALLMS ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VCALLMS";
	static const void *c_vFunction = R5900::Instruction::Execute::VCALLMS;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VCALLMSR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VCALLMSR";
	static const void *c_vFunction = R5900::Instruction::Execute::VCALLMSR;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


// VFTOI //

static long R5900::Recompiler::VFTOI0 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VFTOI0";
	static const void *c_vFunction = R5900::Instruction::Execute::VFTOI0;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VFTOI4 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VFTOI4";
	static const void *c_vFunction = R5900::Instruction::Execute::VFTOI4;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VFTOI12 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VFTOI12";
	static const void *c_vFunction = R5900::Instruction::Execute::VFTOI12;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VFTOI15 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VFTOI15";
	static const void *c_vFunction = R5900::Instruction::Execute::VFTOI15;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}


// VITOF //

static long R5900::Recompiler::VITOF0 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VITOF0";
	static const void *c_vFunction = R5900::Instruction::Execute::VITOF0;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VITOF4 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VITOF4";
	static const void *c_vFunction = R5900::Instruction::Execute::VITOF4;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VITOF12 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VITOF12";
	static const void *c_vFunction = R5900::Instruction::Execute::VITOF12;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VITOF15 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VITOF15";
	static const void *c_vFunction = R5900::Instruction::Execute::VITOF15;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}





static long R5900::Recompiler::VMOVE ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMOVE";
	static const void *c_vFunction = R5900::Instruction::Execute::VMOVE;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VLQI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VLQI";
	static const void *c_vFunction = R5900::Instruction::Execute::VLQI;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VDIV ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VDIV";
	static const void *c_vFunction = R5900::Instruction::Execute::VDIV;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMTIR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMTIR";
	static const void *c_vFunction = R5900::Instruction::Execute::VMTIR;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VRNEXT ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VRNEXT";
	static const void *c_vFunction = R5900::Instruction::Execute::VRNEXT;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMR32 ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMR32";
	static const void *c_vFunction = R5900::Instruction::Execute::VMR32;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VSQI ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSQI";
	static const void *c_vFunction = R5900::Instruction::Execute::VSQI;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VSQRT ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSQRT";
	static const void *c_vFunction = R5900::Instruction::Execute::VSQRT;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMFIR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMFIR";
	static const void *c_vFunction = R5900::Instruction::Execute::VMFIR;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VRGET ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VRGET";
	static const void *c_vFunction = R5900::Instruction::Execute::VRGET;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}




// VSUBA //

static long R5900::Recompiler::VSUBA ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSUBA";
	static const void *c_vFunction = R5900::Instruction::Execute::VSUBA;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VSUBAi ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSUBAi";
	static const void *c_vFunction = R5900::Instruction::Execute::VSUBAi;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VSUBAq ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSUBAq";
	static const void *c_vFunction = R5900::Instruction::Execute::VSUBAq;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VSUBABCX ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSUBABCX";
	static const void *c_vFunction = R5900::Instruction::Execute::VSUBABCX;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VSUBABCY ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSUBABCY";
	static const void *c_vFunction = R5900::Instruction::Execute::VSUBABCY;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VSUBABCZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSUBABCZ";
	static const void *c_vFunction = R5900::Instruction::Execute::VSUBABCZ;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VSUBABCW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSUBABCW";
	static const void *c_vFunction = R5900::Instruction::Execute::VSUBABCW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}



// VMADDA //

static long R5900::Recompiler::VMADDA ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMADDA";
	static const void *c_vFunction = R5900::Instruction::Execute::VMADDA;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMADDAi ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMADDAi";
	static const void *c_vFunction = R5900::Instruction::Execute::VMADDAi;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMADDAq ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMADDAq";
	static const void *c_vFunction = R5900::Instruction::Execute::VMADDAq;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMADDABCX ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMADDABCX";
	static const void *c_vFunction = R5900::Instruction::Execute::VMADDABCX;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMADDABCY ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMADDABCY";
	static const void *c_vFunction = R5900::Instruction::Execute::VMADDABCY;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMADDABCZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMADDABCZ";
	static const void *c_vFunction = R5900::Instruction::Execute::VMADDABCZ;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMADDABCW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMADDABCW";
	static const void *c_vFunction = R5900::Instruction::Execute::VMADDABCW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}




// VMSUBA //

static long R5900::Recompiler::VMSUBA ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMSUBA";
	static const void *c_vFunction = R5900::Instruction::Execute::VMSUBA;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMSUBAi ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMSUBAi";
	static const void *c_vFunction = R5900::Instruction::Execute::VMSUBAi;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMSUBAq ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMSUBAq";
	static const void *c_vFunction = R5900::Instruction::Execute::VMSUBAq;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMSUBABCX ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMSUBABCX";
	static const void *c_vFunction = R5900::Instruction::Execute::VMSUBABCX;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMSUBABCY ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMSUBABCY";
	static const void *c_vFunction = R5900::Instruction::Execute::VMSUBABCY;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMSUBABCZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMSUBABCZ";
	static const void *c_vFunction = R5900::Instruction::Execute::VMSUBABCZ;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMSUBABCW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMSUBABCW";
	static const void *c_vFunction = R5900::Instruction::Execute::VMSUBABCW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}





// VMULA //

static long R5900::Recompiler::VMULA ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMULA";
	static const void *c_vFunction = R5900::Instruction::Execute::VMULA;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMULAi ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMULAi";
	static const void *c_vFunction = R5900::Instruction::Execute::VMULAi;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMULAq ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMULAq";
	static const void *c_vFunction = R5900::Instruction::Execute::VMULAq;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMULABCX ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMULABCX";
	static const void *c_vFunction = R5900::Instruction::Execute::VMULABCX;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMULABCY ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMULABCY";
	static const void *c_vFunction = R5900::Instruction::Execute::VMULABCY;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMULABCZ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMULABCZ";
	static const void *c_vFunction = R5900::Instruction::Execute::VMULABCZ;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VMULABCW ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VMULABCW";
	static const void *c_vFunction = R5900::Instruction::Execute::VMULABCW;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}





static long R5900::Recompiler::VOPMULA ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VOPMULA";
	static const void *c_vFunction = R5900::Instruction::Execute::VOPMULA;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VLQD ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VLQD";
	static const void *c_vFunction = R5900::Instruction::Execute::VLQD;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VRSQRT ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VRSQRT";
	static const void *c_vFunction = R5900::Instruction::Execute::VRSQRT;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VILWR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VILWR";
	static const void *c_vFunction = R5900::Instruction::Execute::VILWR;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VRINIT ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VRINIT";
	static const void *c_vFunction = R5900::Instruction::Execute::VRINIT;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VCLIP ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VCLIP";
	static const void *c_vFunction = R5900::Instruction::Execute::VCLIP;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VNOP ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VNOP";
	static const void *c_vFunction = R5900::Instruction::Execute::VNOP;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VSQD ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VSQD";
	static const void *c_vFunction = R5900::Instruction::Execute::VSQD;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VWAITQ ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VWAITQ";
	static const void *c_vFunction = R5900::Instruction::Execute::VWAITQ;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VISWR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VISWR";
	static const void *c_vFunction = R5900::Instruction::Execute::VISWR;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}

static long R5900::Recompiler::VRXOR ( R5900::Instruction::Format i, u32 Address )
{
	static const char *c_sName = "VRXOR";
	static const void *c_vFunction = R5900::Instruction::Execute::VRXOR;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	return 1;
}






static const R5900::Recompiler::Function R5900::Recompiler::FunctionList []
{
	// instructions on both R3000A and R5900
	// 1 + 56 + 6 = 63 instructions //
	R5900::Recompiler::Invalid,
	R5900::Recompiler::J, R5900::Recompiler::JAL, R5900::Recompiler::JR, R5900::Recompiler::JALR, R5900::Recompiler::BEQ, R5900::Recompiler::BNE, R5900::Recompiler::BGTZ, R5900::Recompiler::BGEZ,
	R5900::Recompiler::BLTZ, R5900::Recompiler::BLEZ, R5900::Recompiler::BGEZAL, R5900::Recompiler::BLTZAL, R5900::Recompiler::ADD, R5900::Recompiler::ADDI, R5900::Recompiler::ADDU, R5900::Recompiler::ADDIU,
	R5900::Recompiler::SUB, R5900::Recompiler::SUBU, R5900::Recompiler::MULT, R5900::Recompiler::MULTU, R5900::Recompiler::DIV, R5900::Recompiler::DIVU, R5900::Recompiler::AND, R5900::Recompiler::ANDI,
	R5900::Recompiler::OR, R5900::Recompiler::ORI, R5900::Recompiler::XOR, R5900::Recompiler::XORI, R5900::Recompiler::NOR, R5900::Recompiler::LUI, R5900::Recompiler::SLL, R5900::Recompiler::SRL,
	R5900::Recompiler::SRA, R5900::Recompiler::SLLV, R5900::Recompiler::SRLV, R5900::Recompiler::SRAV, R5900::Recompiler::SLT, R5900::Recompiler::SLTI, R5900::Recompiler::SLTU, R5900::Recompiler::SLTIU,
	R5900::Recompiler::LB, R5900::Recompiler::LBU, R5900::Recompiler::LH, R5900::Recompiler::LHU, R5900::Recompiler::LW, R5900::Recompiler::LWL, R5900::Recompiler::LWR, R5900::Recompiler::SB,
	R5900::Recompiler::SH, R5900::Recompiler::SW, R5900::Recompiler::SWL, R5900::Recompiler::SWR, R5900::Recompiler::MFHI, R5900::Recompiler::MTHI, R5900::Recompiler::MFLO, R5900::Recompiler::MTLO,
	R5900::Recompiler::MFC0, R5900::Recompiler::MTC0,
	R5900::Recompiler::CFC2_I, R5900::Recompiler::CTC2_I, R5900::Recompiler::CFC2_NI, R5900::Recompiler::CTC2_NI,
	R5900::Recompiler::SYSCALL, R5900::Recompiler::BREAK,
	
	// instructions on R3000A ONLY
	//R5900::Recompiler::MFC2, R5900::Recompiler::MTC2, R5900::Recompiler::LWC2, R5900::Recompiler::SWC2, R5900::Recompiler::RFE,
	//R5900::Recompiler::RTPS, R5900::Recompiler::RTPT, R5900::Recompiler::CC, R5900::Recompiler::CDP, R5900::Recompiler::DCPL, R5900::Recompiler::DPCS, R5900::Recompiler::DPCT, R5900::Recompiler::NCS,
	//R5900::Recompiler::NCT, R5900::Recompiler::NCDS, R5900::Recompiler::NCDT, R5900::Recompiler::NCCS, R5900::Recompiler::NCCT, R5900::Recompiler::GPF, R5900::Recompiler::GPL, R5900::Recompiler::AVSZ3,
	//R5900::Recompiler::AVSZ4, R5900::Recompiler::SQR, R5900::Recompiler::OP, R5900::Recompiler::NCLIP, R5900::Recompiler::INTPL, R5900::Recompiler::MVMVA
	
	// instructions on R5900 ONLY
	// (24*8) + 4 + 6 = 192 + 10 = 202 instructions //
	R5900::Recompiler::BEQL, R5900::Recompiler::BNEL, R5900::Recompiler::BGEZL, R5900::Recompiler::BGTZL, R5900::Recompiler::BLEZL, R5900::Recompiler::BLTZL, R5900::Recompiler::BGEZALL, R5900::Recompiler::BLTZALL,
	R5900::Recompiler::DADD, R5900::Recompiler::DADDI, R5900::Recompiler::DADDU, R5900::Recompiler::DADDIU, R5900::Recompiler::DSUB, R5900::Recompiler::DSUBU, R5900::Recompiler::DSLL, R5900::Recompiler::DSLL32,
	R5900::Recompiler::DSLLV, R5900::Recompiler::DSRA, R5900::Recompiler::DSRA32, R5900::Recompiler::DSRAV, R5900::Recompiler::DSRL, R5900::Recompiler::DSRL32, R5900::Recompiler::DSRLV, R5900::Recompiler::LD,
	R5900::Recompiler::LDL, R5900::Recompiler::LDR, R5900::Recompiler::LWU, R5900::Recompiler::LQ, R5900::Recompiler::PREF, R5900::Recompiler::SD, R5900::Recompiler::SDL, R5900::Recompiler::SDR,
	R5900::Recompiler::SQ, R5900::Recompiler::TEQ, R5900::Recompiler::TEQI, R5900::Recompiler::TNE, R5900::Recompiler::TNEI, R5900::Recompiler::TGE, R5900::Recompiler::TGEI, R5900::Recompiler::TGEU,
	R5900::Recompiler::TGEIU, R5900::Recompiler::TLT, R5900::Recompiler::TLTI, R5900::Recompiler::TLTU, R5900::Recompiler::TLTIU, R5900::Recompiler::MOVN, R5900::Recompiler::MOVZ, R5900::Recompiler::MULT1,
	R5900::Recompiler::MULTU1, R5900::Recompiler::DIV1, R5900::Recompiler::DIVU1, R5900::Recompiler::MADD, R5900::Recompiler::MADD1, R5900::Recompiler::MADDU, R5900::Recompiler::MADDU1, R5900::Recompiler::MFHI1,
	R5900::Recompiler::MTHI1, R5900::Recompiler::MFLO1, R5900::Recompiler::MTLO1, R5900::Recompiler::MFSA, R5900::Recompiler::MTSA, R5900::Recompiler::MTSAB, R5900::Recompiler::MTSAH,
	R5900::Recompiler::PABSH, R5900::Recompiler::PABSW, R5900::Recompiler::PADDB, R5900::Recompiler::PADDH, R5900::Recompiler::PADDW, R5900::Recompiler::PADDSB, R5900::Recompiler::PADDSH, R5900::Recompiler::PADDSW,
	R5900::Recompiler::PADDUB, R5900::Recompiler::PADDUH, R5900::Recompiler::PADDUW, R5900::Recompiler::PADSBH, R5900::Recompiler::PAND, R5900::Recompiler::POR, R5900::Recompiler::PXOR, R5900::Recompiler::PNOR,
	R5900::Recompiler::PCEQB, R5900::Recompiler::PCEQH, R5900::Recompiler::PCEQW, R5900::Recompiler::PCGTB, R5900::Recompiler::PCGTH, R5900::Recompiler::PCGTW, R5900::Recompiler::PCPYH, R5900::Recompiler::PCPYLD,
	R5900::Recompiler::PCPYUD, R5900::Recompiler::PDIVBW, R5900::Recompiler::PDIVUW, R5900::Recompiler::PDIVW, R5900::Recompiler::PEXCH, R5900::Recompiler::PEXCW, R5900::Recompiler::PEXEH, R5900::Recompiler::PEXEW,
	R5900::Recompiler::PEXT5, R5900::Recompiler::PEXTLB, R5900::Recompiler::PEXTLH, R5900::Recompiler::PEXTLW, R5900::Recompiler::PEXTUB, R5900::Recompiler::PEXTUH, R5900::Recompiler::PEXTUW, R5900::Recompiler::PHMADH,
	R5900::Recompiler::PHMSBH, R5900::Recompiler::PINTEH, R5900::Recompiler::PINTH, R5900::Recompiler::PLZCW, R5900::Recompiler::PMADDH, R5900::Recompiler::PMADDW, R5900::Recompiler::PMADDUW, R5900::Recompiler::PMAXH,
	R5900::Recompiler::PMAXW, R5900::Recompiler::PMINH, R5900::Recompiler::PMINW, R5900::Recompiler::PMFHI, R5900::Recompiler::PMFLO, R5900::Recompiler::PMTHI, R5900::Recompiler::PMTLO, R5900::Recompiler::PMFHL_LH,
	R5900::Recompiler::PMFHL_SH, R5900::Recompiler::PMFHL_LW, R5900::Recompiler::PMFHL_UW, R5900::Recompiler::PMFHL_SLW, R5900::Recompiler::PMTHL_LW, R5900::Recompiler::PMSUBH, R5900::Recompiler::PMSUBW, R5900::Recompiler::PMULTH,
	R5900::Recompiler::PMULTW, R5900::Recompiler::PMULTUW, R5900::Recompiler::PPAC5, R5900::Recompiler::PPACB, R5900::Recompiler::PPACH, R5900::Recompiler::PPACW, R5900::Recompiler::PREVH, R5900::Recompiler::PROT3W,
	R5900::Recompiler::PSLLH, R5900::Recompiler::PSLLVW, R5900::Recompiler::PSLLW, R5900::Recompiler::PSRAH, R5900::Recompiler::PSRAW, R5900::Recompiler::PSRAVW, R5900::Recompiler::PSRLH, R5900::Recompiler::PSRLW,
	R5900::Recompiler::PSRLVW, R5900::Recompiler::PSUBB, R5900::Recompiler::PSUBH, R5900::Recompiler::PSUBW, R5900::Recompiler::PSUBSB, R5900::Recompiler::PSUBSH, R5900::Recompiler::PSUBSW, R5900::Recompiler::PSUBUB,
	R5900::Recompiler::PSUBUH, R5900::Recompiler::PSUBUW,
	R5900::Recompiler::QFSRV, R5900::Recompiler::SYNC,
	
	R5900::Recompiler::DI, R5900::Recompiler::EI, R5900::Recompiler::ERET, R5900::Recompiler::CACHE, R5900::Recompiler::TLBP, R5900::Recompiler::TLBR, R5900::Recompiler::TLBWI, R5900::Recompiler::TLBWR,
	R5900::Recompiler::CFC0, R5900::Recompiler::CTC0,
	
	R5900::Recompiler::BC0T, R5900::Recompiler::BC0TL, R5900::Recompiler::BC0F, R5900::Recompiler::BC0FL, R5900::Recompiler::BC1T, R5900::Recompiler::BC1TL, R5900::Recompiler::BC1F, R5900::Recompiler::BC1FL,
	R5900::Recompiler::BC2T, R5900::Recompiler::BC2TL, R5900::Recompiler::BC2F, R5900::Recompiler::BC2FL,
	
	// COP1 floating point instructions
	R5900::Recompiler::LWC1, R5900::Recompiler::SWC1, R5900::Recompiler::MFC1, R5900::Recompiler::MTC1, R5900::Recompiler::CFC1, R5900::Recompiler::CTC1,
	R5900::Recompiler::ABS_S, R5900::Recompiler::ADD_S, R5900::Recompiler::ADDA_S, R5900::Recompiler::C_EQ_S, R5900::Recompiler::C_F_S, R5900::Recompiler::C_LE_S, R5900::Recompiler::C_LT_S, R5900::Recompiler::CVT_S_W,
	R5900::Recompiler::CVT_W_S, R5900::Recompiler::DIV_S, R5900::Recompiler::MADD_S, R5900::Recompiler::MADDA_S, R5900::Recompiler::MAX_S, R5900::Recompiler::MIN_S, R5900::Recompiler::MOV_S, R5900::Recompiler::MSUB_S,
	R5900::Recompiler::MSUBA_S, R5900::Recompiler::MUL_S, R5900::Recompiler::MULA_S, R5900::Recompiler::NEG_S, R5900::Recompiler::RSQRT_S, R5900::Recompiler::SQRT_S, R5900::Recompiler::SUB_S, R5900::Recompiler::SUBA_S,
	
	// VU macro mode instructions
	R5900::Recompiler::QMFC2_NI, R5900::Recompiler::QMFC2_I, R5900::Recompiler::QMTC2_NI, R5900::Recompiler::QMTC2_I, R5900::Recompiler::LQC2, R5900::Recompiler::SQC2,
	
	R5900::Recompiler::VABS,
	R5900::Recompiler::VADD, R5900::Recompiler::VADDi, R5900::Recompiler::VADDq, R5900::Recompiler::VADDBCX, R5900::Recompiler::VADDBCY, R5900::Recompiler::VADDBCZ, R5900::Recompiler::VADDBCW,
	R5900::Recompiler::VADDA, R5900::Recompiler::VADDAi, R5900::Recompiler::VADDAq, R5900::Recompiler::VADDABCX, R5900::Recompiler::VADDABCY, R5900::Recompiler::VADDABCZ, R5900::Recompiler::VADDABCW,
	R5900::Recompiler::VCALLMS, R5900::Recompiler::VCALLMSR, R5900::Recompiler::VCLIP, R5900::Recompiler::VDIV,
	R5900::Recompiler::VFTOI0, R5900::Recompiler::VFTOI4, R5900::Recompiler::VFTOI12, R5900::Recompiler::VFTOI15,
	R5900::Recompiler::VIADD, R5900::Recompiler::VIADDI, R5900::Recompiler::VIAND, R5900::Recompiler::VILWR, R5900::Recompiler::VIOR, R5900::Recompiler::VISUB, R5900::Recompiler::VISWR,
	R5900::Recompiler::VITOF0, R5900::Recompiler::VITOF4, R5900::Recompiler::VITOF12, R5900::Recompiler::VITOF15,
	R5900::Recompiler::VLQD, R5900::Recompiler::VLQI,
	
	R5900::Recompiler::VMADD, R5900::Recompiler::VMADDi, R5900::Recompiler::VMADDq, R5900::Recompiler::VMADDBCX, R5900::Recompiler::VMADDBCY, R5900::Recompiler::VMADDBCZ, R5900::Recompiler::VMADDBCW,
	R5900::Recompiler::VMADDA, R5900::Recompiler::VMADDAi, R5900::Recompiler::VMADDAq, R5900::Recompiler::VMADDABCX, R5900::Recompiler::VMADDABCY, R5900::Recompiler::VMADDABCZ, R5900::Recompiler::VMADDABCW,
	R5900::Recompiler::VMAX, R5900::Recompiler::VMAXi, R5900::Recompiler::VMAXBCX, R5900::Recompiler::VMAXBCY, R5900::Recompiler::VMAXBCZ, R5900::Recompiler::VMAXBCW,
	R5900::Recompiler::VMFIR,
	R5900::Recompiler::VMINI, R5900::Recompiler::VMINIi, R5900::Recompiler::VMINIBCX, R5900::Recompiler::VMINIBCY, R5900::Recompiler::VMINIBCZ, R5900::Recompiler::VMINIBCW,
	R5900::Recompiler::VMOVE, R5900::Recompiler::VMR32,
	
	R5900::Recompiler::VMSUB, R5900::Recompiler::VMSUBi, R5900::Recompiler::VMSUBq, R5900::Recompiler::VMSUBBCX, R5900::Recompiler::VMSUBBCY, R5900::Recompiler::VMSUBBCZ, R5900::Recompiler::VMSUBBCW,
	R5900::Recompiler::VMSUBA, R5900::Recompiler::VMSUBAi, R5900::Recompiler::VMSUBAq, R5900::Recompiler::VMSUBABCX, R5900::Recompiler::VMSUBABCY, R5900::Recompiler::VMSUBABCZ, R5900::Recompiler::VMSUBABCW,
	R5900::Recompiler::VMTIR,
	R5900::Recompiler::VMUL, R5900::Recompiler::VMULi, R5900::Recompiler::VMULq, R5900::Recompiler::VMULBCX, R5900::Recompiler::VMULBCY, R5900::Recompiler::VMULBCZ, R5900::Recompiler::VMULBCW,
	R5900::Recompiler::VMULA, R5900::Recompiler::VMULAi, R5900::Recompiler::VMULAq, R5900::Recompiler::VMULABCX, R5900::Recompiler::VMULABCY, R5900::Recompiler::VMULABCZ, R5900::Recompiler::VMULABCW,
	R5900::Recompiler::VNOP, R5900::Recompiler::VOPMSUB, R5900::Recompiler::VOPMULA, R5900::Recompiler::VRGET, R5900::Recompiler::VRINIT, R5900::Recompiler::VRNEXT, R5900::Recompiler::VRSQRT, R5900::Recompiler::VRXOR,
	R5900::Recompiler::VSQD, R5900::Recompiler::VSQI, R5900::Recompiler::VSQRT,
	R5900::Recompiler::VSUB, R5900::Recompiler::VSUBi, R5900::Recompiler::VSUBq, R5900::Recompiler::VSUBBCX, R5900::Recompiler::VSUBBCY, R5900::Recompiler::VSUBBCZ, R5900::Recompiler::VSUBBCW,
	R5900::Recompiler::VSUBA, R5900::Recompiler::VSUBAi, R5900::Recompiler::VSUBAq, R5900::Recompiler::VSUBABCX, R5900::Recompiler::VSUBABCY, R5900::Recompiler::VSUBABCZ, R5900::Recompiler::VSUBABCW,
	R5900::Recompiler::VWAITQ,
	R5900::Recompiler::COP2
};
