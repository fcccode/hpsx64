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


#include "R3000A_Recompiler.h"
#include "ps1_system.h"

using namespace R3000A;


#define ENABLE_MULTIPLY_LATENCY
#define ENABLE_DIVIDE_LATENCY


#define USE_NEW_J_CODE
#define USE_NEW_JR_CODE
#define USE_NEW_JAL_CODE
#define USE_NEW_JALR_CODE


#define USE_NEW_BEQ_CODE
#define USE_NEW_BNE_CODE
#define USE_NEW_BLTZ_CODE
#define USE_NEW_BGTZ_CODE
#define USE_NEW_BLEZ_CODE
#define USE_NEW_BGEZ_CODE
#define USE_NEW_BLTZAL_CODE
#define USE_NEW_BGEZAL_CODE


#define USE_NEW_LOAD_CODE
#define USE_NEW_STORE_CODE

#define EXECUTE_LOADS_IMMEDIATELY


#define USE_NEW_MULT_CODE
#define USE_NEW_MULTU_CODE
#define USE_NEW_DIV_CODE
#define USE_NEW_DIVU_CODE

#define USE_NEW_MFHI_CODE
#define USE_NEW_MFLO_CODE

#define USE_NEW_ADD_CODE
#define USE_NEW_ADDI_CODE
#define USE_NEW_SUB_CODE

#define USE_NEW_SYSCALL_CODE


#define ALLOW_ENCODING_DELAYSLOT
#define ENCODE_ALL_POSSIBLE_DELAYSLOTS


#define CHECK_EVENT_AFTER_START


//#define ENABLE_SINGLE_STEP





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
Cpu *Recompiler::r;
s32 Recompiler::OpLevel;
u32 Recompiler::LocalPC;
u32 Recompiler::Local_LastModifiedReg;
u32 Recompiler::Local_NextPCModified;

u32 Recompiler::CurrentCount;

u32 Recompiler::isBranchDelaySlot;
u32 Recompiler::isLoadDelaySlot;

u32 Recompiler::bStopEncodingAfter;
u32 Recompiler::bStopEncodingBefore;


//u32 Recompiler::Local_DelaySlot;
//u32 Recompiler::Local_DelayType;
//u32 Recompiler::Local_DelayCount;
//u32 Recompiler::Local_DelayCond;
//u32 Recompiler::Local_Condition;
Instruction::Format Recompiler::NextInst;

Recompiler::RDelaySlot Recompiler::RDelaySlots [ 2 ];
u32 Recompiler::DSIndex;
u32 Recompiler::RDelaySlots_Valid;

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



// constructor
// NumberOfBlocks MUST be a power of 2, so 1 would mean 2, 2 would mean 4
Recompiler::Recompiler ( Cpu* R3000ACpu, u32 NumberOfBlocks, u32 BlockSize_PowerOfTwo, u32 MaxIStep_Shift )
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
	r = R3000ACpu;
	
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
	
	// reset invalidate arrays
	r->Bus->Reset_Invalidate ();
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
static u64 Recompiler::GetSourceRegs ( Instruction::Format i, u32 Address )
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
static u64 Recompiler::Get_DelaySlot_DestRegs ( Instruction::Format i )
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
	Instruction::Format inst;
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
	DSIndex = 0;
	
	// each instruction takes at least one cycle
	//if ( !MemCycles ) MemCycles = 1;
	
	
	
	// this should get pointer to the instruction
	pSrcCodePtr = RGetPointer ( Address );
	
	// get the cycles per instruction
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
			MemCycles = DataBus::c_iBIOS_Read_Latency;
		}
		else
		{
			// ram region //
			MemCycles = DataBus::c_iRAM_Read_Latency;
		}
		
		// should be plus 1 like in the interpreter
		MemCycles += 1;
	}


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
		
#ifdef ENABLE_SINGLE_STEP
			LocalCycleCount = 0;
#endif

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
		
#ifdef ENABLE_SINGLE_STEP

		CycleCount [ BlockIndex ] = 0;
		
#else
	
#ifdef ALLOW_COP2_CYCLE_RESET
		if ( ( inst.Value >> 26 ) == 0x12 )
		{
#ifdef VERBOSE_RECOMPILE
cout << " COP2";
#endif
			// COP2 instruction //
			CycleCount [ BlockIndex ] = LocalCycleCount;
		}
		else
#endif
		{
			// must add one to the cycle offset for starting point because the interpreter adds an extra cycle at the end of run
			//CycleCount [ BlockIndex ] = LocalCycleCount + 1;
			CycleCount [ BlockIndex ] = LocalCycleCount;
		}
		
#endif
		
		//EndAddress [ BlockIndex ] = -1;
		
		// recompile the instruction
		ret = Recompile ( inst, Address );

		
#ifdef VERBOSE_RECOMPILE
cout << " ret=" << ret;
//cout << " ENC0=" << hex << (((u64*) (pCodeStart [ BlockIndex ])) [ 0 ]);
//cout << " ENC1=" << hex << (((u64*) (pCodeStart [ BlockIndex ])) [ 1 ]);
cout << " ASM: " << Print::PrintInstruction ( inst.Value ).c_str ();
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
			
				//cout << "\nhps1x64: R3000A: Recompiler: Error: Unable to encode instruction.";
				
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
			LocalCycleCount = 0;
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
static long Recompiler::Generate_Normal_Store ( Instruction::Format i, u32 Address, u32 BitTest, void* StoreFunctionToCall )
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
	e->DecReg64 ( RAX );
	e->MovMemReg64 ( & Playstation1::System::_SYSTEM->CycleCount, RAX );
	
	// part 2: check for synchronous interrupt //
	
	// this is where the entry point should be if this is the first instruction in the run
	pCodeStart [ BlockIndex ] = e->Get_CodeBlock_CurrentPtr ();
#endif
	
	// get the store address
	//u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	e->MovRegFromMem32 ( RDX, &r->GPR [ i.Base ].s );
	e->AddReg32ImmX ( RDX, i.sOffset );

	// check for synchronous interrupt
	
	// if there is a synchronous interrupt possible, then check for it
	if ( BitTest )
	{
		// if ( StoreAddress & 0x1 )
		e->TestReg32ImmX ( RDX, BitTest );

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
			
	// get the value to store
	e->MovRegFromMem32 ( RCX, &r->GPR [ i.Rt ].s );
	
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
static long Recompiler::Generate_Normal_Load ( Instruction::Format i, u32 Address, u32 BitTest, void* LoadFunctionToCall )
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
	e->MovRegFromMem32 ( RDX, &r->GPR [ i.Base ].s );
	e->AddReg32ImmX ( RDX, i.sOffset );

	// check for synchronous interrupt
	
	// if there is a synchronous interrupt possible, then check for it
	if ( BitTest )
	{
		// if ( StoreAddress & 0x1 )
		e->TestReg32ImmX ( RDX, BitTest );

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
	
	// part 3: execute the load //
	
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

	return ret;
}


static long Recompiler::Generate_Normal_Branch ( Instruction::Format i, u32 Address, void* BranchFunctionToCall )
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
		e->MovRegMem32 ( RDX, & r->GPR [ i.Rs ].u );
		
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
			e->MovRegMem32 ( RCX, & r->GPR [ i.Rs ].u );
			e->CmpMemReg32 ( & r->GPR [ i.Rt ].u, RCX );
			//e->Jmp8_NE ( 0, 0 );
			e->Jmp_NE ( 0, 0 );
			break;
			
		case OPBNE:
			e->MovRegMem32 ( RCX, & r->GPR [ i.Rs ].u );
			e->CmpMemReg32 ( & r->GPR [ i.Rt ].u, RCX );
			//e->Jmp8_E ( 0, 0 );
			e->Jmp_E ( 0, 0 );
			break;
			
		case OPBLEZ:
			e->CmpMem32ImmX ( & r->GPR [ i.Rs ].s, 0 );
			//e->Jmp8_G ( 0, 0 );
			e->Jmp_G ( 0, 0 );
			break;
			
		case OPBGTZ:
			e->CmpMem32ImmX ( & r->GPR [ i.Rs ].s, 0 );
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
					e->CmpMem32ImmX ( & r->GPR [ i.Rs ].s, 0 );
					//e->Jmp8_GE ( 0, 0 );
					e->Jmp_GE ( 0, 0 );
					break;
				case RTBGEZ:
					e->CmpMem32ImmX ( & r->GPR [ i.Rs ].s, 0 );
					//e->Jmp8_L ( 0, 0 );
					e->Jmp_L ( 0, 0 );
					break;
			
				case RTBLTZAL:
					e->CmpMem32ImmX ( & r->GPR [ i.Rs ].s, 0 );
					e->MovMemImm32 ( & r->GPR [ 31 ].u, Address + 8 );
					//e->Jmp8_GE ( 0, 0 );
					e->Jmp_GE ( 0, 0 );
					break;
			
				case RTBGEZAL:
					e->CmpMem32ImmX ( & r->GPR [ i.Rs ].s, 0 );
					e->MovMemImm32 ( & r->GPR [ 31 ].u, Address + 8 );
					//e->Jmp8_L ( 0, 0 );
					e->Jmp_L ( 0, 0 );
					break;
			}
			
			break;
			
		case OPJAL:
			e->MovMemImm32 ( & r->GPR [ 31 ].u, Address + 8 );
			break;
			
		case OPJALR:
		
			if ( i.Funct == 9 )
			{
				// JALR //
				
				// make sure Rd is not r0
				if ( i.Rd )
				{
					// save return address in Rd
					e->MovMemImm32 ( & r->GPR [ i.Rd ].u, Address + 8 );
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
			cout << "\nR3000A: Recompiler: Error encoding branch in delay slot.";
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
				cout << "\nR3000A: Recompiler: Problem setting NextPC for branch after delay slot.";
			}
		}
		
		// done - return
		e->Ret ();
#endif
		
		// the cache-line is not loaded //
		
		// also jump here from above if needed
		if ( !e->SetJmpTarget8 ( 1 ) )
		{
			cout << "\nR3000A: Recompiler: Short branch1 too far.";
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
		e->OrMem64ImmX ( &r->Status.Value, 2 );
		
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
		e->OrMem64ImmX ( &r->Status.Value, 2 );
		
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
		cout << "\nR3000A: Recompiler: Short branch0 too far.";
	}
	*/
	
	if ( !e->SetJmpTarget ( 0 ) )
	{
		cout << "\nR3000A: Recompiler: Short branch0 too far.";
	}
	
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nEND";
#endif

	// done
	return ret;
}




// regular arithemetic //

// *** todo *** no need to save LastModifiedRegister unless instruction is KNOWN to be in a delay slot on run
long Recompiler::ADDU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "ADDU";
	static const void* c_vFunction = Instruction::Execute::ADDU;
	
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
				ret = e->Call ( c_vFunction );
				
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
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( !i.Rt )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rs )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->AddMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rt )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					ret = e->AddMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->AddRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}

long Recompiler::SUBU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SUBU";
	static const void* c_vFunction = Instruction::Execute::SUBU;
	
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
				ret = e->Call ( c_vFunction );
				
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
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rs )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->SubMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].u );
					e->SubRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	
	return 1;
}

long Recompiler::AND ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "AND";
	static const void* c_vFunction = Instruction::Execute::AND;
	
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
				ret = e->Call ( c_vFunction );
				
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
					ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( !i.Rt )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( i.Rd == i.Rs )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->AndMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rt )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					ret = e->AndMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].u );
					e->AndRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RAX );
				}
				
				// note: last modified register can be cached locally in this case, and written back after all instructions encoded
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	
	return 1;
}

long Recompiler::OR ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "OR";
	static const void* c_vFunction = Instruction::Execute::OR;
	
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
				ret = e->Call ( c_vFunction );
				
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
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( !i.Rt )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rs )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->OrMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rt )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					ret = e->OrMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].u );
					e->OrRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	
	return 1;
}

long Recompiler::XOR ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "XOR";
	static const void* c_vFunction = Instruction::Execute::XOR;
	
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
				ret = e->Call ( c_vFunction );
				
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
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( !i.Rt )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rs )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->XorMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rt )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					ret = e->XorMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].u );
					e->XorRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NOR ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "NOR";
	static const void* c_vFunction = Instruction::Execute::NOR;
	
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
				ret = e->Call ( c_vFunction );
				
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
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					e->NotReg32 ( RAX );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( !i.Rt )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->NotReg32 ( RAX );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].u );
					e->OrRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
					e->NotReg32 ( RAX );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SLT ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SLT";
	static const void* c_vFunction = Instruction::Execute::SLT;
	
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
				ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				e->XorRegReg32 ( RCX, RCX );
				e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].u );
				e->CmpRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
				e->Set_L ( RCX );
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RCX );
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SLTU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SLTU";
	static const void* c_vFunction = Instruction::Execute::SLTU;
	
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
				ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				e->XorRegReg32 ( RCX, RCX );
				e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].u );
				e->CmpRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
				e->Set_B ( RCX );
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].u, RCX );
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}


////////////////////////////////////////////
// I-Type Instructions (non-interrupt)



long Recompiler::ADDIU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "ADDIU";
	static const void* c_vFunction = Instruction::Execute::ADDIU;
	
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
				ret = e->Call ( c_vFunction );
				
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
					e->MovMemImm32 ( &r->GPR [ i.Rt ].s, i.sImmediate );
				}
				else if ( i.Rt == i.Rs )
				{
					e->AddMem32ImmX ( &r->GPR [ i.Rt ].s, i.sImmediate );
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					//e->AddRegImm32 ( RAX, i.sImmediate );
					e->AddReg32ImmX ( RAX, i.sImmediate );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rt );
			}
			Local_LastModifiedReg = i.Rt;
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

long Recompiler::ANDI ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "ANDI";
	static const void* c_vFunction = Instruction::Execute::ANDI;
	
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
				ret = e->Call ( c_vFunction );
				
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
					e->MovMemImm32 ( &r->GPR [ i.Rt ].s, 0 );
				}
				else if ( i.Rt == i.Rs )
				{
					e->AndMem32ImmX ( &r->GPR [ i.Rt ].s, i.uImmediate );
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					//e->AndRegImm32 ( RAX, i.uImmediate );
					e->AndReg32ImmX ( RAX, i.uImmediate );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rt );
			}
			Local_LastModifiedReg = i.Rt;
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

long Recompiler::ORI ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "ORI";
	static const void* c_vFunction = Instruction::Execute::ORI;
	
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
				ret = e->Call ( c_vFunction );
				
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
					e->MovMemImm32 ( &r->GPR [ i.Rt ].s, i.uImmediate );
				}
				else if ( i.Rt == i.Rs )
				{
					e->OrMem32ImmX ( &r->GPR [ i.Rt ].s, i.uImmediate );
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					//e->OrRegImm32 ( RAX, i.uImmediate );
					e->OrReg32ImmX ( RAX, i.uImmediate );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rt );
			}
			Local_LastModifiedReg = i.Rt;
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

long Recompiler::XORI ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "XORI";
	static const void* c_vFunction = Instruction::Execute::XORI;
	
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
				ret = e->Call ( c_vFunction );
				
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
					e->MovMemImm32 ( &r->GPR [ i.Rt ].s, i.uImmediate );
				}
				else if ( i.Rt == i.Rs )
				{
					e->XorMem32ImmX ( &r->GPR [ i.Rt ].s, i.uImmediate );
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					//e->XorRegImm32 ( RAX, i.uImmediate );
					e->XorReg32ImmX ( RAX, i.uImmediate );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rt );
			}
			Local_LastModifiedReg = i.Rt;
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

long Recompiler::SLTI ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SLTI";
	static const void* c_vFunction = Instruction::Execute::SLTI;
	
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
				ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				e->XorRegReg32 ( RAX, RAX );
				//e->MovRegFromMem32 ( 0, &r->GPR [ i.Rs ].u );
				e->CmpMemImm32 ( &r->GPR [ i.Rs ].s, i.sImmediate );
				e->Set_L ( RAX );
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].u, RAX );
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rt );
			}
			Local_LastModifiedReg = i.Rt;
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SLTIU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SLTIU";
	static const void* c_vFunction = Instruction::Execute::SLTIU;
	
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
				ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				e->XorRegReg32 ( RAX, RAX );
				//e->MovRegFromMem32 ( 0, &r->GPR [ i.Rs ].u );
				e->CmpMemImm32 ( &r->GPR [ i.Rs ].s, i.sImmediate );
				e->Set_B ( RAX );
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].u, RAX );
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rt );
			}
			Local_LastModifiedReg = i.Rt;
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::LUI ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LUI";
	static const void* c_vFunction = Instruction::Execute::LUI;
	
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
				ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				ret = e->MovMemImm32 ( &r->GPR [ i.Rt ].u, ( i.uImmediate << 16 ) );
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rt );
			}
			Local_LastModifiedReg = i.Rt;
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LUI instruction.\n";
		return -1;
	}
	return 1;
}







//////////////////////////////////////////////////////////
// Shift instructions



long Recompiler::SLL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SLL";
	static const void* c_vFunction = Instruction::Execute::SLL;
	
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
				ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].u );
				//e->MovRegImm32 ( RCX, (u32) i.Shift );
				e->ShlRegImm32 ( RAX, (u32) i.Shift );
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SLL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SRL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SRL";
	static const void* c_vFunction = Instruction::Execute::SRL;
	
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
				ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].u );
				//e->MovRegImm32 ( RCX, (u32) i.Shift );
				e->ShrRegImm32 ( RAX, (u32) i.Shift );
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SRA ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SRA";
	static const void* c_vFunction = Instruction::Execute::SRA;
	
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
				ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				e->MovRegFromMem32 ( 0, &r->GPR [ i.Rt ].u );
				//e->MovRegImm32 ( RCX, (u32) i.Shift );
				e->SarRegImm32 ( 0, (u32) i.Shift );
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, 0 );
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SLLV ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SLLV";
	static const void* c_vFunction = Instruction::Execute::SLLV;
	
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
				ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].u );
				e->MovRegFromMem32 ( RCX, &r->GPR [ i.Rs ].u );
				e->ShlRegReg32 ( RAX );
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
			}
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SRLV ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SRLV";
	static const void* c_vFunction = Instruction::Execute::SRLV;
	
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
				ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].u );
				e->MovRegFromMem32 ( RCX, &r->GPR [ i.Rs ].u );
				e->ShrRegReg32 ( RAX );
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SRAV ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SRAV";
	static const void* c_vFunction = Instruction::Execute::SRAV;
	
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
				ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].u );
				e->MovRegFromMem32 ( RCX, &r->GPR [ i.Rs ].u );
				e->SarRegReg32 ( RAX );
				e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}


//----------------------------------------------------------------------------


////////////////////////////////////////////
// Jump/Branch Instructions



long Recompiler::J ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "J";
	static const void* c_vFunction = Instruction::Execute::J;
	
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
			ret = e->Call ( c_vFunction );
			
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //J instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::JR ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "JR";
	static const void* c_vFunction = Instruction::Execute::JR;
	
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
			ret = e->Call ( c_vFunction );
			
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //JR instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::JAL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "JAL";
	static const void* c_vFunction = Instruction::Execute::JAL;
	
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
			ret = e->Call ( c_vFunction );
			
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //JAL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::JALR ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "JALR";
	static const void* c_vFunction = Instruction::Execute::JALR;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// make sure Rd is not r0
			if ( !i.Rd )
			{
				// make sure r0 stays at zero
				e->MovMemImm32 ( &r->GPR [ 0 ].u, 0 );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //JALR instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BEQ ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BEQ";
	static const void* c_vFunction = Instruction::Execute::BEQ;
	
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
			ret = e->Call ( c_vFunction );
			
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BEQ instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BNE ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BNE";
	static const void* c_vFunction = Instruction::Execute::BNE;
	
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
			ret = e->Call ( c_vFunction );
			
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BNE instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BLEZ ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BLEZ";
	static const void* c_vFunction = Instruction::Execute::BLEZ;
	
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
			ret = e->Call ( c_vFunction );
			
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BLEZ instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BGTZ ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BGTZ";
	static const void* c_vFunction = Instruction::Execute::BGTZ;
	
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
			ret = e->Call ( c_vFunction );
			
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BGTZ instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BLTZ ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BLTZ";
	static const void* c_vFunction = Instruction::Execute::BLTZ;
	
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
			ret = e->Call ( c_vFunction );
			
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BLTZ instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BGEZ ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BGEZ";
	static const void* c_vFunction = Instruction::Execute::BGEZ;
	
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
			ret = e->Call ( c_vFunction );
			
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BGEZ instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BLTZAL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BLTZAL";
	static const void* c_vFunction = Instruction::Execute::BLTZAL;
	
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
			ret = e->Call ( c_vFunction );
			
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BLTZAL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BGEZAL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BGEZAL";
	static const void* c_vFunction = Instruction::Execute::BGEZAL;
	
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
			ret = e->Call ( c_vFunction );
			
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BGEZAL instruction.\n";
		return -1;
	}
	return 1;
}


/////////////////////////////////////////////////////////////
// Multiply/Divide Instructions

long Recompiler::MULT ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MULT";
	static const void* c_vFunction = Instruction::Execute::MULT;
	
	// if rs is between -0x800 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff or between -0x7ff and -0x100000, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
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
			ret = e->Call ( c_vFunction );
			
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
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].s );
			e->Cdq ();
			e->XorRegReg32 ( RAX, RDX );
			//e->MovRegReg32 ( RAX, RCX );
			//e->XorReg32ImmX ( RAX, -1 );
			//e->CmovSRegReg32 ( RAX, RCX );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Slow );
			e->MovReg32ImmX ( RDX, c_iMultiplyCycles_Med );
			e->CmpReg32ImmX ( RAX, 0x100000 );
			e->CmovBRegReg32 ( RCX, RDX );
			e->MovReg32ImmX ( RDX, c_iMultiplyCycles_Fast );
			e->CmpReg32ImmX ( RAX, 0x800 );
			e->CmovBRegReg32 ( RCX, RDX );
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			e->AddRegReg64 ( RCX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle );
			//e->CmovBRegReg64 ( RDX, RAX );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			// store the correct busy until cycle back
			e->SubRegReg64 ( RCX, RDX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			
			// do the multiply
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].s );
			e->ImulRegMem32 ( & r->GPR [ i.Rt ].s );
			e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			ret = e->MovMemReg32 ( & r->HiLo.sHi, RDX );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MULT instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::MULTU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MULTU";
	static const void* c_vFunction = Instruction::Execute::MULTU;
	
	// if rs is between 0 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
	
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
			ret = e->Call ( c_vFunction );
			
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
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].s );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Slow );
			e->MovReg32ImmX ( RDX, c_iMultiplyCycles_Med );
			e->CmpReg32ImmX ( RAX, 0x100000 );
			e->CmovBRegReg32 ( RCX, RDX );
			e->MovReg32ImmX ( RDX, c_iMultiplyCycles_Fast );
			e->CmpReg32ImmX ( RAX, 0x800 );
			e->CmovBRegReg32 ( RCX, RDX );
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, & r->CycleCount );
			//e->MovRegMem64 ( RCX, & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			e->AddRegReg64 ( RCX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			e->SubRegMem64 ( RAX, & r->MulDiv_BusyUntil_Cycle );
			//e->CmovBRegReg64 ( RDX, RAX );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( & r->CycleCount, RAX );
			e->SubMemReg64 ( & r->CycleCount, RDX );
			
			// store the correct busy until cycle back
			e->SubRegReg64 ( RCX, RDX );
			e->MovMemReg64 ( & r->MulDiv_BusyUntil_Cycle, RCX );
			
			// do the multiply
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].s );
			e->MulRegMem32 ( & r->GPR [ i.Rt ].s );
			e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->MovMemReg32 ( & r->HiLo.sHi, RDX );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MULTU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::DIV ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "DIV";
	static const void* c_vFunction = Instruction::Execute::DIV;
	
	static const int c_iDivideCycles = 36;
	
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
			e->MovsxdReg64Mem32 ( RCX, & r->GPR [ i.Rt ].u );
			e->MovsxdReg64Mem32 ( RAX, & r->GPR [ i.Rs ].u );
			//e->OrRegReg64 ( RCX, RCX );
			//e->Jmp8_E ( 0, 0 );
			e->Jmp8_ECXZ ( 0, 0 );
			
			//e->MovRegReg64 ( RDX, RAX );
			//e->SarRegImm64 ( RDX, 63 );
			e->Cqo ();
			e->IdivRegReg64 ( RCX );
			e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->Jmp8 ( 0, 1 );
			
			e->SetJmpTarget8 ( 0 );
			
			//e->MovReg32ImmX ( RCX, -1 );
			//e->MovReg32ImmX ( RDX, 1 );
			//e->OrRegReg32 ( RAX, RAX );
			//e->CmovSRegReg32 ( RCX, RDX );
			e->Cdq ();
			e->NotReg32 ( RDX );
			e->OrReg32ImmX ( RDX, 1 );
			
			e->MovMemReg32 ( & r->HiLo.uHi, RAX );
			e->MovMemReg32 ( & r->HiLo.uLo, RDX );
			
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //DIV instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::DIVU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "DIVU";
	static const void* c_vFunction = Instruction::Execute::DIVU;
	
	static const int c_iDivideCycles = 36;
	
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
			ret = e->Call ( c_vFunction );
			
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
			e->MovRegMem32 ( RCX, & r->GPR [ i.Rt ].u );
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].u );
			//e->OrRegReg64 ( RCX, RCX );
			//e->Jmp8_E ( 0, 0 );
			e->Jmp8_ECXZ ( 0, 0 );
			
			e->XorRegReg32 ( RDX, RDX );
			e->DivRegReg32 ( RCX );
			e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			e->Jmp8 ( 0, 1 );
			
			e->SetJmpTarget8 ( 0 );
			
			e->MovMemImm32 ( & r->HiLo.sLo, -1 );
			e->MovMemReg32 ( & r->HiLo.uHi, RAX );
			
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //DIVU instruction.\n";
		return -1;
	}
	return 1;
}



long Recompiler::MFHI ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MFHI";
	static const void* c_vFunction = Instruction::Execute::MFHI;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// make sure Rd is not r0
			if ( !i.Rd )
			{
				// make sure r0 stays at zero
				ret = e->MovMemImm32 ( &r->GPR [ 0 ].u, 0 );
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
				e->MovRegMem32 ( RAX, & r->HiLo.uHi );
				e->MovMemReg32 ( & r->GPR [ i.Rd ].u, RAX );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MFHI instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::MFLO ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MFLO";
	static const void* c_vFunction = Instruction::Execute::MFLO;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// make sure Rd is not r0
			if ( !i.Rd )
			{
				// make sure r0 stays at zero
				ret = e->MovMemImm32 ( &r->GPR [ 0 ].u, 0 );
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
				e->MovRegMem32 ( RAX, & r->HiLo.uLo );
				e->MovMemReg32 ( & r->GPR [ i.Rd ].u, RAX );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MFLO instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::MTHI ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MTHI";
	static const void* c_vFunction = Instruction::Execute::MTHI;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].u );
			ret = e->MovRegToMem32 ( &r->HiLo.uHi, RAX );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MTHI instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::MTLO ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MTLO";
	static const void* c_vFunction = Instruction::Execute::MTLO;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].u );
			ret = e->MovRegToMem32 ( &r->HiLo.uLo, RAX );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MTLO instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::RFE ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "RFE";
	static const void* c_vFunction = Instruction::Execute::RFE;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //RFE instruction.\n";
		return -1;
	}
	return 1;
}




////////////////////////////////////////////////////////
// Instructions that can cause Synchronous Interrupts //
////////////////////////////////////////////////////////


long Recompiler::ADD ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "ADD";
	static const void* c_vFunction = Instruction::Execute::ADD;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			if ( !i.Rd )
			{
				// make sure r0 stays at zero
				ret = e->MovMemImm32 ( &r->GPR [ 0 ].u, 0 );
			}
			
			break;
			
		case 1:
#ifdef USE_NEW_ADD_CODE
			e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
			e->AddRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
			
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
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADD instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::ADDI ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "ADDI";
	static const void* c_vFunction = Instruction::Execute::ADDI;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			if ( !i.Rt )
			{
				// make sure r0 stays at zero
				ret = e->MovMemImm32 ( &r->GPR [ 0 ].u, 0 );
			}
			
			break;
			
		case 1:
#ifdef USE_NEW_ADDI_CODE
			e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
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
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDI instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SUB ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SUB";
	static const void* c_vFunction = Instruction::Execute::SUB;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			if ( !i.Rd )
			{
				// make sure r0 stays at zero
				ret = e->MovMemImm32 ( &r->GPR [ 0 ].u, 0 );
			}
			break;
			
		case 1:
#ifdef USE_NEW_SUB_CODE
			e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
			e->SubRegMem32 ( RAX, &r->GPR [ i.Rt ].u );
			
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
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SUB instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::SYSCALL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SYSCALL";
	static const void* c_vFunction = Instruction::Execute::SYSCALL;
	
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
			ret = e->Call ( c_vFunction );
			
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SYSCALL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BREAK ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BREAK";
	static const void* c_vFunction = Instruction::Execute::BREAK;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BREAK instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::Invalid ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "Invalid";
	static const void* c_vFunction = Instruction::Execute::Invalid;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //Invalid instruction.\n";
		return -1;
	}
	return 1;
}





long Recompiler::MFC0 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MFC0";
	static const void* c_vFunction = Instruction::Execute::MFC0;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MFC0 instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::MTC0 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MTC0";
	static const void* c_vFunction = Instruction::Execute::MTC0;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MTC0 instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::MFC2 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MFC2";
	static const void* c_vFunction = Instruction::Execute::MFC2;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MFC2 instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::MTC2 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MTC2";
	static const void* c_vFunction = Instruction::Execute::MTC2;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MTC2 instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::CFC2 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "CFC2";
	static const void* c_vFunction = Instruction::Execute::CFC2;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //CFC2 instruction.\n";
		return -1;
	}
	return 1;
}



long Recompiler::CTC2 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "CTC2";
	static const void* c_vFunction = Instruction::Execute::CTC2;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //CTC2 instruction.\n";
		return -1;
	}
	return 1;
}




// Load/Store - will need to use address translation to get physical addresses when needed

//////////////////////////////////////////////////////////////////////////
// store instructions

// store instructions
long Recompiler::SB ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SB";
	static const void* c_vFunction = Instruction::Execute::SB;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_STORE_CODE
			ret = Generate_Normal_Store ( i, Address, 0x0, Instruction::Execute::SB_Recompiler );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SB instruction.\n";
		return -1;
	}
	return 1;
}





long Recompiler::SH ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SH";
	static const void* c_vFunction = Instruction::Execute::SH;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;

			
		case 1:
#ifdef USE_NEW_STORE_CODE
			ret = Generate_Normal_Store ( i, Address, 0x1, Instruction::Execute::SH_Recompiler );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SH instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SW ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SW";
	static const void* c_vFunction = Instruction::Execute::SW;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		
		case 1:
#ifdef USE_NEW_STORE_CODE
			ret = Generate_Normal_Store ( i, Address, 0x3, Instruction::Execute::SW_Recompiler );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SW instruction.\n";
		return -1;
	}
	return 1;
}


long Recompiler::SWC2 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SWC2";
	static const void* c_vFunction = Instruction::Execute::SWC2;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SWC2 instruction.\n";
		return -1;
	}
	return 1;
}



long Recompiler::SWL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SWL";
	static const void* c_vFunction = Instruction::Execute::SWL;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		
		case 1:
#ifdef USE_NEW_STORE_CODE
			ret = Generate_Normal_Store ( i, Address, 0x0, Instruction::Execute::SWL_Recompiler );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SWL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SWR ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SWR";
	static const void* c_vFunction = Instruction::Execute::SWR;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
			
		case 1:
#ifdef USE_NEW_STORE_CODE
			ret = Generate_Normal_Store ( i, Address, 0x0, Instruction::Execute::SWR_Recompiler );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SWR instruction.\n";
		return -1;
	}
	return 1;
}



/////////////////////////////////////////////////
// load instructions

// load instructions with delay slot
// *** todo *** it is also possible to this and just process load after load delay slot has executed - would still need previous load address before delay slot
// *** todo *** could also skip delay slot zero and put straight into delay slot 1 after next instruction, or just process load delay slot after next instruction
long Recompiler::LB ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LB";
	static const void* c_vFunction = Instruction::Execute::LB;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			Generate_Normal_Load ( i, Address, 0x0, (void*) Cpu::ProcessLoadDelaySlot_t<OPLB,0> );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LB instruction.\n";
		return -1;
	}
	return 1;
}






long Recompiler::LH ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LH";
	static const void* c_vFunction = Instruction::Execute::LH;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			ret = Generate_Normal_Load ( i, Address, 0x1, (void*) Cpu::ProcessLoadDelaySlot_t<OPLH,0> );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LB instruction.\n";
		return -1;
	}
	return 1;
}








long Recompiler::LW ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LW";
	static const void* c_vFunction = Instruction::Execute::LW;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			ret = Generate_Normal_Load ( i, Address, 0x3, (void*) Cpu::ProcessLoadDelaySlot_t<OPLW,0> );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LB instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::LBU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LBU";
	static const void* c_vFunction = Instruction::Execute::LBU;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			ret = Generate_Normal_Load ( i, Address, 0x0, (void*) Cpu::ProcessLoadDelaySlot_t<OPLBU,0> );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LBU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::LHU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LHU";
	static const void* c_vFunction = Instruction::Execute::LHU;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			ret = Generate_Normal_Load ( i, Address, 0x1, (void*) Cpu::ProcessLoadDelaySlot_t<OPLHU,0> );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LB instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::LWC2 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LWC2";
	static const void* c_vFunction = Instruction::Execute::LWC2;
	
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
			bStopEncodingBefore = true;
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LWC2 instruction.\n";
		return -1;
	}
	return 1;
}




// load instructions without load-delay slot
long Recompiler::LWL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LWL";
	static const void* c_vFunction = Instruction::Execute::LWL;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LB instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::LWR ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LWR";
	static const void* c_vFunction = Instruction::Execute::LWR;
	
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
			ret = e->Call ( c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LWR instruction.\n";
		return -1;
	}
	return 1;
}




















///////////////////////////
// GTE instructions

long Recompiler::COP2 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "COP2";
	static const void* c_vFunction = Instruction::Execute::COP2;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //COP2 instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::RTPS ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "RTPS";
	static const void* c_vFunction = Instruction::Execute::RTPS;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //RTPS instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCLIP ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "NCLIP";
	static const void* c_vFunction = Instruction::Execute::NCLIP;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCLIP instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::OP ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "OP";
	static const void* c_vFunction = Instruction::Execute::OP;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //OP instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::DPCS ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "DPCS";
	static const void* c_vFunction = Instruction::Execute::DPCS;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //DPCS instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::INTPL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "INTPL";
	static const void* c_vFunction = Instruction::Execute::INTPL;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //INTPL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::MVMVA ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MVMVA";
	static const void* c_vFunction = Instruction::Execute::MVMVA;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MVMVA instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCDS ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "NCDS";
	static const void* c_vFunction = Instruction::Execute::NCDS;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCDS instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::CDP ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "CDP";
	static const void* c_vFunction = Instruction::Execute::CDP;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //CDP instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCDT ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "NCDT";
	static const void* c_vFunction = Instruction::Execute::NCDT;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCDT instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCCS ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "NCCS";
	static const void* c_vFunction = Instruction::Execute::NCCS;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCCS instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::CC ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "CC";
	static const void* c_vFunction = Instruction::Execute::CC;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //CC instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCS ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "NCS";
	static const void* c_vFunction = Instruction::Execute::NCS;
	
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
			ret = e->Call ( Instruction::Execute::NCS );
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCS instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCT ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "NCT";
	static const void* c_vFunction = Instruction::Execute::NCT;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCT instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SQR ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SQR";
	static const void* c_vFunction = Instruction::Execute::SQR;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SQR instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::DCPL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "DCPL";
	static const void* c_vFunction = Instruction::Execute::DCPL;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //DCPL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::DPCT ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "DPCT";
	static const void* c_vFunction = Instruction::Execute::DPCT;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //DPCT instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::AVSZ3 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "AVSZ3";
	static const void* c_vFunction = Instruction::Execute::AVSZ3;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //AVSZ3 instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::AVSZ4 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "AVSZ4";
	static const void* c_vFunction = Instruction::Execute::AVSZ4;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //AVSZ4 instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::RTPT ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "RTPT";
	static const void* c_vFunction = Instruction::Execute::RTPT;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //RTPT instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::GPF ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "GPF";
	static const void* c_vFunction = Instruction::Execute::GPF;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //GPF instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::GPL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "GPL";
	static const void* c_vFunction = Instruction::Execute::GPL;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //GPL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCCT ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "NCCT";
	static const void* c_vFunction = Instruction::Execute::NCCT;
	
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
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCCT instruction.\n";
		return -1;
	}
	return 1;
}



static const Recompiler::Function Recompiler::FunctionList []
{
	// instructions on both R3000A and R5900
	Recompiler::Invalid,
	Recompiler::J, Recompiler::JAL, Recompiler::JR, Recompiler::JALR, Recompiler::BEQ, Recompiler::BNE, Recompiler::BGTZ, Recompiler::BGEZ,
	Recompiler::BLTZ, Recompiler::BLEZ, Recompiler::BGEZAL, Recompiler::BLTZAL, Recompiler::ADD, Recompiler::ADDI, Recompiler::ADDU, Recompiler::ADDIU,
	Recompiler::SUB, Recompiler::SUBU, Recompiler::MULT, Recompiler::MULTU, Recompiler::DIV, Recompiler::DIVU, Recompiler::AND, Recompiler::ANDI,
	Recompiler::OR, Recompiler::ORI, Recompiler::XOR, Recompiler::XORI, Recompiler::NOR, Recompiler::LUI, Recompiler::SLL, Recompiler::SRL,
	Recompiler::SRA, Recompiler::SLLV, Recompiler::SRLV, Recompiler::SRAV, Recompiler::SLT, Recompiler::SLTI, Recompiler::SLTU, Recompiler::SLTIU,
	Recompiler::LB, Recompiler::LBU, Recompiler::LH, Recompiler::LHU, Recompiler::LW, Recompiler::LWL, Recompiler::LWR, Recompiler::SB,
	Recompiler::SH, Recompiler::SW, Recompiler::SWL, Recompiler::SWR, Recompiler::MFHI, Recompiler::MTHI, Recompiler::MFLO, Recompiler::MTLO,
	Recompiler::MFC0, Recompiler::MTC0, Recompiler::CFC2, Recompiler::CTC2, Recompiler::SYSCALL, Recompiler::BREAK,
	
	// instructions on R3000A ONLY
	Recompiler::MFC2, Recompiler::MTC2, Recompiler::LWC2, Recompiler::SWC2, Recompiler::RFE,
	Recompiler::RTPS, Recompiler::RTPT, Recompiler::CC, Recompiler::CDP, Recompiler::DCPL, Recompiler::DPCS, Recompiler::DPCT, Recompiler::NCS,
	Recompiler::NCT, Recompiler::NCDS, Recompiler::NCDT, Recompiler::NCCS, Recompiler::NCCT, Recompiler::GPF, Recompiler::GPL, Recompiler::AVSZ3,
	Recompiler::AVSZ4, Recompiler::SQR, Recompiler::OP, Recompiler::NCLIP, Recompiler::INTPL, Recompiler::MVMVA
};
