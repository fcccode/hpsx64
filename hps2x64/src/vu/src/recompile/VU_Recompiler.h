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



#ifndef _VU_RECOMPILER_H_
#define _VU_RECOMPILER_H_


#include "VU.h"
#include "VU_Lookup.h"
#include "x64Encoder.h"


//class Playstation2::VU;

using namespace Vu::Instruction;

namespace Playstation2
{
	class VU;
}

namespace Vu
{
	
	//using namespace Playstation2;
	//class Playstation2::VU;
	
	// will probably create two recompilers for each processor. one for single stepping and one for multi-stepping
	class Recompiler
	{
	public:
		// number of code cache slots total
		// but the number of blocks should be variable
		//static const int c_iNumBlocks = 1024;
		//static const u32 c_iNumBlocks_Mask = c_iNumBlocks - 1;
		u32 NumBlocks;
		u32 NumBlocks_Mask;
		//static const int c_iBlockSize_Shift = 4;
		//static const int c_iBlockSize_Mask = ( 1 << c_iBlockSize_Shift ) - 1;
		
		static const u32 c_iAddress_Mask = 0x1fffffff;
		
		u32 TotalCodeCache_Size;	// = c_iNumBlocks * ( 1 << c_iBlockSize_Shift );
		
		u32 BlockSize;
		u32 BlockSize_Shift;
		u32 BlockSize_Mask;
		
		// maximum number of instructions that can be encoded/recompiled in a run
		u32 MaxStep;
		u32 MaxStep_Shift;
		u32 MaxStep_Mask;
		
		// zero if not in delay slot, otherwise has the instruction
		//static u32 Local_DelaySlot;
		//static u32 Local_DelayType;
		//static u32 Local_DelayCount;
		//static u32 Local_DelayCond;
		
		// 0 means unconitional branch, 1 means check condition before branching
		//static u32 Local_Condition;
		
		
		// the amount of stack space required for SEH
		static const long c_lSEH_StackSize = 40;
		
		
		union RDelaySlot
		{
			struct
			{
				// instruction in delay slot
				u32 Code;
				
				// type of instruction with delay slot
				// 0 - branch (BEQ,BNE,etc), 1 - jump (J,JAL)
				u16 Type;
				
				// branch condition
				// 0 - unconditional, 1 - conditional
				u16 Condition;
			};
			
			u64 Value;
		};
		
		static RDelaySlot RDelaySlots [ 2 ];
		static u32 DSIndex;
		
		static u32 RDelaySlots_Valid;
		
		inline void ClearDelaySlots ( void ) { RDelaySlots [ 0 ].Value = 0; RDelaySlots [ 1 ].Value = 0; }
		
		// need something to use to keep track of dynamic delay slot stuff
		u32 RecompilerDelaySlot_Flags;
		
		
		// the next instruction in the cache block if there is one
		static Vu::Instruction::Format NextInst;
		
		static u64 MemCycles;

		static u64 LocalCycleCount;
		static u64 CacheBlock_CycleCount;
		
		// also need to know if the addresses in the block are cached or not
		static bool bIsBlockInICache;

		
		// the PC while recompiling
		static u32 LocalPC;
		
		// the optimization level
		// 0 means no optimization at all, anything higher means to optimize
		static s32 OpLevel;
		u32 OptimizeLevel;
		
		// the current enabled encoder
		static x64Encoder *e;
		//static ICache_Device *ICache;
		//static VU::Cpu *r;
		
		// the encoder for this particular instance
		x64Encoder *InstanceEncoder;
		
		// bitmap for branch delay slot
		static u32 Status_BranchDelay;
		static u32 Status_BranchConditional;
		static Vu::Instruction::Format Status_BranchInstruction;
		
		static u32 Status_EBit;
		
		// the maximum number of cache blocks it can encode across
		s32 MaxCacheBlocks;
		
		
		static u32 SetStatus_Flag;
		static u32 SetClip_Flag;

		static Vu::Instruction::Format instLO;
		static Vu::Instruction::Format instHI;
		static Vu::Instruction::Format NextInstLO;
		
		static void AdvanceCycle ( VU* v );
		static void AdvanceCycle_rec ( VU* v );
		
		
		inline void Set_MaxCacheBlocks ( s32 Value ) { MaxCacheBlocks = Value; }
		
		// constructor
		// block size must be power of two, multiplier shift value
		// so for BlockSize_PowerOfTwo, for a block size of 4 pass 2, block size of 8 pass 3, etc
		// for MaxStep, use 0 for single stepping, 1 for stepping until end of 1 cache block, 2 for stepping until end of 2 cache blocks, etc
		// no, for MaxStep, it should be the maximum number of instructions to step
		// NumberOfBlocks MUST be a power of 2, where 1 means 2, 2 means 4, etc
		Recompiler ( VU* v, u32 NumberOfBlocks, u32 BlockSize_PowerOfTwo, u32 MaxIStep );
		
		// destructor
		~Recompiler ();
		
		
		void Reset ();	// { memset ( this, 0, sizeof( Recompiler ) ); }

		
		static bool isBranchDelayOk ( u32 ulInstruction, u32 Address );

		
		u32* RGetPointer ( VU *v, u32 Address );
		
		
		// set the optimization level
		inline void SetOptimizationLevel ( u32 Level ) { OptimizeLevel = Level; }
		
		// accessors
		//inline u32 Get_DoNotCache ( u32 Address ) { return DoNotCache [ ( Address >> 2 ) & NumBlocks_Mask ]; }
		//inline u32 Get_CacheMissCount ( u32 Address ) { return CacheMissCount [ ( Address >> 2 ) & NumBlocks_Mask ]; }
		//inline u32 Get_StartAddress ( u32 Address ) { return StartAddress [ ( Address >> 2 ) & NumBlocks_Mask ]; }
		//inline u32 Get_LastAddress ( u32 Address ) { return LastAddress [ ( Address >> 2 ) & NumBlocks_Mask ]; }
		//inline u32 Get_RunCount ( u32 Address ) { return RunCount [ ( Address >> 2 ) & NumBlocks_Mask ]; }
		//inline u32 Get_MaxCycles ( u32 Address ) { return MaxCycles [ ( Address >> 2 ) & NumBlocks_Mask ]; }
		
		// returns a pointer to the instructions cached starting at Address
		// this function assumes that this instance of recompiler only has one instruction per run
		//inline u32* Get_InstructionsPtr ( u32 Address ) { return & ( Instructions [ ( ( Address >> 2 ) & NumBlocks_Mask ) << MaxStep_Shift ] ); }
		
		// returns a pointer to the start addresses cached starting at Address
		// this function assumes that this instance of recompiler only has one instruction per run
		//inline u32* Get_AddressPtr ( u32 Address ) { return & ( StartAddress [ ( ( Address >> 2 ) & NumBlocks_Mask ) ] ); }
		
		// check that Address is not -1
		// returns 1 if address is ok, returns 0 if it is not ok
		//inline bool isBlockValid ( u32 Address ) { return ( StartAddress [ ( Address >> 2 ) & NumBlocks_Mask ] != 0xffffffff ); }
		
		// check that Address matches StartAddress for the block
		// returns 1 if address is ok, returns 0 if it is not ok
		//inline bool isStartAddressMatch ( u32 Address ) { return ( Address == StartAddress [ ( Address >> 2 ) & NumBlocks_Mask ] ); }
		
		// this function assumes that this instance of recompiler only has one instruction per run
		// returns 1 if the instruction is cached for address, 0 otherwise
		//inline bool isCached ( u32 Address, u32 Instruction ) { return ( Address == StartAddress [ ( Address >> 2 ) & NumBlocks_Mask ] ) && ( Instruction == Instructions [ ( Address >> 2 ) & NumBlocks_Mask ] ); }
		
		inline bool isRecompiled ( u32 Address ) { return ( StartAddress [ ( Address >> ( 2 + MaxStep_Shift ) ) & NumBlocks_Mask ] == ( Address & ~( ( 1 << ( 2 + MaxStep_Shift ) ) - 1 ) ) ); }
		
		
		inline u64 Execute ( u32 Address ) { return InstanceEncoder->ExecuteCodeBlock ( ( Address >> 2 ) & NumBlocks_Mask ); }


		static int Recompiler::Prefix_MADDW ( VU* v, Vu::Instruction::Format i );
		static int Recompiler::Postfix_MADDW ( VU* v, Vu::Instruction::Format i );
		
		
		// initializes a code block that is not being used yet
		u32 InitBlock ( u32 Block );
		
		
		inline static void Clear_FSrcReg () { VU::ClearBitmap ( VU::FSrcBitmap ); }
		inline static void Add_FSrcReg ( u32 i, u32 SrcReg ) { if ( SrcReg ) { VU::AddBitmap ( VU::FSrcBitmap, ( i >> 21 ) & 0xf, SrcReg ); } }
		inline static void Add_FSrcRegBC ( u32 i, u32 SrcReg ) { if ( SrcReg ) { VU::AddBitmap ( VU::FSrcBitmap, 0x8 >> ( i & 0x3 ), SrcReg ); } }

		inline static void Clear_ISrcReg () { VU::ISrcBitmap = 0; }
		inline static void Add_ISrcReg ( u32 SrcReg ) { if ( SrcReg & 31 ) { VU::ISrcBitmap |= SrcReg; } }

		inline static void Clear_DstReg () { VU::ClearBitmap ( VU::FDstBitmap ); VU::IDstBitmap = 0; }
		inline static void Add_FDstReg ( u32 i, u32 DstReg ) { if ( DstReg ) { VU::AddBitmap ( VU::FDstBitmap, ( i >> 21 ) & 0xf, DstReg ); VU::IDstBitmap |= ( 1ULL << DstReg ); } }
		inline static void Add_IDstReg ( u32 DstReg ) { if ( DstReg & 31 ) { VU::IDstBitmap |= ( 1ULL << DstReg ); } }
		
		static void PipelineWaitQ ( VU* v );
		
		static void TestStall ( VU* v );
		static void TestStall_INT ( VU* v );

		static void SetDstBitmap ( VU* v, u64 b0, u64 b1, u64 i0 );

		// recompilation of SetDstBitmap
		static void SetDstBitmap_rec ( VU* v, u64 b0, u64 b1, u64 i0 );

		
		long ProcessBranch ( VU* v, Vu::Instruction::Format i, u32 Address );

		static long Generate_VABSp ( VU* v, Vu::Instruction::Format i );
		static long Generate_VMAXp ( VU* v, Vu::Instruction::Format i, u32 *pFt = NULL, u32 FtComponent = 4 );
		static long Generate_VMINp ( VU* v, Vu::Instruction::Format i, u32 *pFt = NULL, u32 FtComponent = 4 );
		static long Generate_VFTOIXp ( VU* v, Vu::Instruction::Format i, u32 IX );
		static long Generate_VITOFXp ( VU* v, Vu::Instruction::Format i, u32 FX );
		static long Generate_VMOVEp ( VU* v, Vu::Instruction::Format i );
		static long Generate_VMR32p ( VU* v, Vu::Instruction::Format i );
		static long Generate_VMFIRp ( VU* v, Vu::Instruction::Format i );
		static long Generate_VMTIRp ( VU* v, Vu::Instruction::Format i );
		static long Generate_VADDp ( VU* v, u32 bSub, Vu::Instruction::Format i, u32 FtComponent = 4, void *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VMULp ( VU* v, Vu::Instruction::Format i, u32 FtComponentp = 0x1b, void *pFd = NULL, u32 *pFt = NULL, u32 FsComponentp = 0x1b );
		static long Generate_VMADDp ( VU* v, u32 bSub, Vu::Instruction::Format i, u32 FtComponentp = 0x1b, void *pFd = NULL, u32 *pFt = NULL, u32 FsComponentp = 0x1b );

		
		

		static u64 GetFSourceRegsHI_LoXYZW ( Vu::Instruction::Format i );
		static u64 GetFSourceRegsHI_HiXYZW ( Vu::Instruction::Format i );
		static u64 GetFSourceRegsLO_LoXYZW ( Vu::Instruction::Format i );
		static u64 GetFSourceRegsLO_HiXYZW ( Vu::Instruction::Format i );

		static u64 GetFDestRegsHI_LoXYZW ( Vu::Instruction::Format i );
		static u64 GetFDestRegsHI_HiXYZW ( Vu::Instruction::Format i );
		
		// get a bitmap for the source registers used by the specified instruction
		static u64 GetFSourceRegsHI ( Vu::Instruction::Format i );
		static u64 GetFSourceRegsLO ( Vu::Instruction::Format i );
		static u64 GetFDestRegsHI ( Vu::Instruction::Format i );
		static u64 Get_DelaySlot_DestRegs ( Vu::Instruction::Format i );
		
		//static long Generate_Normal_Store ( R5900::Instruction::Format i, u32 Address, u32 BitTest, void* StoreFunctionToCall );
		//static long Generate_Normal_Load ( R5900::Instruction::Format i, u32 Address, u32 BitTest, void* LoadFunctionToCall );
		//static long Generate_Normal_Branch ( R5900::Instruction::Format i, u32 Address, void* BranchFunctionToCall );
		//static long Generate_Normal_Trap ( R5900::Instruction::Format i, u32 Address );


		static long Generate_VABSp ( Vu::Instruction::Format i );
		static long Generate_VMAXp ( Vu::Instruction::Format i, u32 *pFt = NULL, u32 FtComponent = 4 );
		static long Generate_VMINp ( Vu::Instruction::Format i, u32 *pFt = NULL, u32 FtComponent = 4 );
		static long Generate_VFTOIXp ( Vu::Instruction::Format i, u32 IX );
		static long Generate_VITOFXp ( Vu::Instruction::Format i, u32 FX );
		static long Generate_VMOVEp ( Vu::Instruction::Format i );
		static long Generate_VMR32p ( Vu::Instruction::Format i );
		static long Generate_VMFIRp ( Vu::Instruction::Format i );
		static long Generate_VMTIRp ( Vu::Instruction::Format i );
		static long Generate_VADDp ( u32 bSub, Vu::Instruction::Format i, u32 FtComponent = 4, void *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VMULp ( Vu::Instruction::Format i, u32 FtComponentp = 0x1b, void *pFd = NULL, u32 *pFt = NULL, u32 FsComponentp = 0x1b );
		static long Generate_VMADDp ( u32 bSub, Vu::Instruction::Format i, u32 FtComponentp = 0x1b, void *pFd = NULL, u32 *pFt = NULL, u32 FsComponentp = 0x1b );
		
		
		static long Generate_VABS ( Vu::Instruction::Format i, u32 Address, u32 Component );
		static long Generate_VMAX ( Vu::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFt = NULL );
		static long Generate_VMIN ( Vu::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFt = NULL );
		static long Generate_VFTOI0 ( Vu::Instruction::Format i, u32 Address, u32 FtComponent, u32 FsComponent );
		static long Generate_VFTOIX ( Vu::Instruction::Format i, u32 Address, u32 FtComponent, u32 FsComponent, u32 IX );
		static long Generate_VITOFX ( Vu::Instruction::Format i, u32 Address, u32 FtComponent, u32 FsComponent, u32 FX );
		static long Generate_VMOVE ( Vu::Instruction::Format i, u32 Address, u32 FtComponent, u32 FsComponent );
		
		static long Generate_VMR32_Load ( Vu::Instruction::Format i, u32 Address, u32 FsComponent );
		static long Generate_VMR32_Store ( Vu::Instruction::Format i, u32 Address, u32 FtComponent );
		
		static long Generate_VMFIR ( Vu::Instruction::Format i, u32 Address, u32 FtComponent );
		static long Generate_VADD ( Vu::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VSUB ( Vu::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VMUL ( Vu::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VMADD ( Vu::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VMSUB ( Vu::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFd = NULL, u32 *pFt = NULL );
		
		
		
		// says if block points to an R3000A instruction that should NOT be EVER cached
		// *note* this should be dynamically allocated
		//u8* DoNotCache;	// [ c_iNumBlocks ];
		
		// number of times a cache miss was encountered while recompiling for this block
		// *note* this should be dynamically allocated
		//u32* CacheMissCount;	// [ c_iNumBlocks ];
		
		// code cache block not invalidated
		// *note* this should be dynamically allocated
		//u8 isValid [ c_iNumBlocks ];
		
		// start address for instruction encodings (inclusive)
		// *note* this should be dynamically allocated
		u32* StartAddress;	// [ c_iNumBlocks ];
		
		
		// last address that instruction encoding is valid for (inclusive)
		// *note* this should be dynamically allocated
		// It actually makes things MUCH simpler if you store the number of instructions recompiled instead of the last address
		//u32* LastAddress;	// [ c_iNumBlocks ];
		//u8* RunCount;
		
		//u32* Instructions;
		
		// pointer to where the prefix for the instruction starts at (used only internally by recompiler)
		u8** pPrefix_CodeStart;
		
		// where the actual instruction starts at in code block
		u8** pCodeStart;
		
		// the number of offset cycles from this instruction in the code block
		u32* CycleCount;
		
		
		// list of branch targets when jumping forwards in code block
		u32* pForwardBranchTargets;
		
		static const int c_ulForwardBranchIndex_Start = 5;
		static u32 ForwardBranchIndex;
		
		static u32 StartBlockIndex;
		static u32 BlockIndex;
		
		// not needed
		//u32* EndAddress;
		
		
		long Generate_Prefix_EventCheck ( u32 Address, bool bIsBranchOrJump );
		
		
		
		// need to know what address current block starts at
		static u32 CurrentBlock_StartAddress;
		
		// also need to know what address the next block starts at
		static u32 NextBlock_StartAddress;
		
		
		// max number of cycles that instruction encoding could use up if executed
		// need to know this to single step when there are interrupts in between
		// code block is not valid when this is zero
		// *note* this should be dynamically allocated
		//u64* MaxCycles;	// [ c_iNumBlocks ];
		
		static u32 Local_LastModifiedReg;
		static u32 Local_NextPCModified;
		
		static u32 CurrentCount;
		
		static u32 isBranchDelaySlot;
		static u32 isLoadDelaySlot;
		
		static u32 bStopEncodingAfter;
		static u32 bStopEncodingBefore;
		
		// set the local cycle count to reset (start from zero) for the next cycle
		static u32 bResetCycleCount;
		
		inline void ResetFlags ( void ) { bStopEncodingBefore = false; bStopEncodingAfter = false; Local_NextPCModified = false; }

		
		
		// recompiler function
		// returns -1 if the instruction cannot be recompiled, otherwise returns the maximum number of cycles the instruction uses
		typedef long (*Function) ( VU *v, Vu::Instruction::Format Instruction, u32 Address );

		// *** todo *** do not recompile more than one instruction if currently in a branch delay slot or load delay slot!!
		u32 Recompile ( VU* v, u32 StartAddress );
		//void Invalidate ( u32 Address );
		void Invalidate ( u32 Address, u32 Count );
		
		u32 CloseOpLevel ( u32 OptLevel, u32 Address );

		bool isNopHi ( Vu::Instruction::Format i );
		bool isNopLo ( Vu::Instruction::Format i );
		
		
		u32 ulIndex_Mask;
		
		inline u32 Get_Block ( u32 Address ) { return ( Address >> ( 3 + MaxStep_Shift ) ) & NumBlocks_Mask; }
		inline u32 Get_Index ( u32 Address ) { return ( Address >> 3 ) & ulIndex_Mask; }

		
		//void Recompile ( u32 Instruction );
		
			static const Function FunctionList [];
			
		// used by object to recompile an instruction into a code block
		// returns -1 if the instruction cannot be recompiled
		// returns 0 if the instruction was recompiled, but MUST start a new block for the next instruction (because it is guaranteed in a delay slot)
		// returns 1 if successful and can continue recompiling
		inline static long RecompileHI ( VU *v, Vu::Instruction::Format i, u32 Address ) { return Vu::Recompiler::FunctionList [ Vu::Instruction::Lookup::FindByInstructionHI ( i.Value ) ] ( v, i, Address ); }
		inline static long RecompileLO ( VU *v, Vu::Instruction::Format i, u32 Address ) { return Vu::Recompiler::FunctionList [ Vu::Instruction::Lookup::FindByInstructionLO ( i.Value ) ] ( v, i, Address ); }
		
		
		inline void Run ( u32 Address ) { InstanceEncoder->ExecuteCodeBlock ( ( Address >> 2 ) & NumBlocks_Mask ); }


			static long INVALID ( VU *v, Vu::Instruction::Format i, u32 Address );
			

			static long ADDBCX ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDBCY ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDBCZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDBCW ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long SUBBCX ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBBCY ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBBCZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBBCW ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MADDBCX ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDBCY ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDBCZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDBCW ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MSUBBCX ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBBCY ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBBCZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBBCW ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MAXBCX ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MAXBCY ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MAXBCZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MAXBCW ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MINIBCX ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MINIBCY ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MINIBCZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MINIBCW ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MULBCX ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULBCY ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULBCZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULBCW ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MULq ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MAXi ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULi ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MINIi ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDq ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDq ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDi ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDi ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long OPMSUB ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBq ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBq ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBi ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBi ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADD ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			//static long ADDi ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long ADDq ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long ADDAi ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long ADDAq ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long ADDABCX ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDABCY ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDABCZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDABCW ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MADD ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MUL ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MAX ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUB ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUB ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long OPMSUM ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MINI ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long IADD ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ISUB ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long IADDI ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long IAND ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long IOR ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long CALLMS ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long CALLMSR ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ITOF0 ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long FTOI0 ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULAq ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDAq ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBAq ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDA ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBA ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MOVE ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long LQI ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long DIV ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MTIR ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long RNEXT ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ITOF4 ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long FTOI4 ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ABS ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDAq ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBAq ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDA ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBA ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long MR32 ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long SQI ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long SQRT ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long MFIR ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long RGET ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			//static long ADDABCX ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long ADDABCY ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long ADDABCZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long ADDABCW ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long SUBABCX ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBABCY ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBABCZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBABCW ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MADDABCX ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDABCY ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDABCZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDABCW ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MSUBABCX ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBABCY ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBABCZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBABCW ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long ITOF12 ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long FTOI12 ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MULABCX ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULABCY ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULABCZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULABCW ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MULAi ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDAi ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBAi ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULA ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long OPMULA ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long LQD ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long RSQRT ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long ILWR ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long RINIT ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ITOF15 ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long FTOI15 ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long CLIP ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDAi ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBAi ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long NOP ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long SQD ( VU *v, Vu::Instruction::Format i, u32 Address );


			// lower instructions

			
			static long LQ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long SQ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ILW ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ISW ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long IADDIU ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ISUBIU ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long FCEQ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long FCSET ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long FCAND ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long FCOR ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long FSEQ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long FSSET ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long FSAND ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long FSOR ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long FMEQ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long FMAND ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long FMOR ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long FCGET ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long B ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long BAL ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long JR ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long JALR ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long IBEQ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long IBNE ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long IBLTZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long IBGTZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long IBLEZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long IBGEZ ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			
			//static long DIV ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long EATANxy ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long EATANxz ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long EATAN ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long IADD ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long ISUB ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long IADDI ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long IAND ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long IOR ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long MOVE ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long LQI ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long DIV ( VU *v, Vu::Instruction::Format i, u32 Address );
			//static long MTIR ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long RNEXT ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MFP ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long XTOP ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long XGKICK ( VU *v, Vu::Instruction::Format i, u32 Address );

			static long MR32 ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long SQI ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long SQRT ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long MFIR ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long RGET ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long XITOP ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ESADD ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long EATANxy ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ESQRT ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ESIN ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ERSADD ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long EATANxz ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ERSQRT ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long EATAN ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long LQD ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long RSQRT ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ILWR ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long RINIT ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ELENG ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ESUM ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ERCPR ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long EEXP ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long SQD ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long WAITQ ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ISWR ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long RXOR ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long ERLENG ( VU *v, Vu::Instruction::Format i, u32 Address );
			static long WAITP ( VU *v, Vu::Instruction::Format i, u32 Address );
			
			
	};
};

//};

#endif
