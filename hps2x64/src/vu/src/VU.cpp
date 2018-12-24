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


// need a define to toggle sse2 on and off for now
// on 64-bit systems, sse2 is supposed to come as standard
//#define _ENABLE_SSE2
//#define _ENABLE_SSE41


#include "VU.h"
#include "VU_Print.h"
#include "VU_Execute.h"


// used for restarting VU Dma transfer - unless its going to check every cycle, needs to alert dma to recheck ability to transfer
#include "PS2_Dma.h"

#include "PS2_GPU.h"


#include "VU_Recompiler.h"


using namespace Playstation2;
using namespace Vu;
//using namespace x64Asm::Utilities;
//using namespace Math::Reciprocal;


// this should be enabled to delay the flag update, which is how it appears to work on PS2
#define DELAY_FLAG_UPDATE


//#define ENABLE_NEW_CLIP_BUFFER
//#define ENABLE_NEW_FLAG_BUFFER
#define ENABLE_SNAPSHOTS



#define ENABLE_RECOMPILER_VU
#define ALLOW_RECOMPILE_INTERPRETER



//#define VERBOSE_UNPACK

//#define VERBOSE_MSCAL
//#define VERBOSE_MSCNT
//#define VERBOSE_MSCALF
//#define VERBOSE_MSKPATH3
//#define VERBOSE_VU_MARK
//#define VERBOSE_VUINT


// will need this for accurate operation, and cycle accuracy is required
#define ENABLE_STALLS
#define ENABLE_INTDELAYSLOT

// says whether flags should get updated immediately in vu0 macro mode or not
#define UPDATE_MACRO_FLAGS_IMMEDIATELY


//#define ENABLE_FLOAT_ADD
//#define ENABLE_FLOAT_TOTAL


#define DISABLE_UNPACK_INDETERMINATE


// forces all upper vu0 addresses to access vu1, instead of just address with 0x4000 bit set
//#define ALL_VU0_UPPER_ADDRS_ACCESS_VU1


#define HALT_DIRECT_WHILE_VU_RUNNING
#define HALT_DIRECTHL_WHILE_VU_RUNNING

#define ENABLE_SET_VGW

// enable debugging

#ifdef _DEBUG_VERSION_

#define INLINE_DEBUG_ENABLE


//#define INLINE_DEBUG_SPLIT


/*
#define INLINE_DEBUG


#define INLINE_DEBUG_PIPELINE

#define INLINE_DEBUG_READ
#define INLINE_DEBUG_WRITE
#define INLINE_DEBUG_DMA_READ
#define INLINE_DEBUG_DMA_WRITE
#define INLINE_DEBUG_FIFO

#define INLINE_DEBUG_VUCOM
#define INLINE_DEBUG_VURUN

#define INLINE_DEBUG_UNPACK
#define INLINE_DEBUG_UNPACK_2
#define INLINE_DEBUG_UNPACK_3

// this sends info to the vu execute debug on when vu gets started, etc
//#define INLINE_DEBUG_VUEXECUTE


#define INLINE_DEBUG_GETMEMPTR_INVALID
#define INLINE_DEBUG_INVALID
*/

#endif




u16 VU::Temp_StatusFlag, VU::Temp_MacFlag;
VU::Bitmap128 VU::Temp_Bitmap;


VU *VU::_VU [ VU::c_iMaxInstances ];


Vu::Instruction::Format2 VU::CurInstLOHI;

Vu::Instruction::Format VU::CurInstLO;
Vu::Instruction::Format VU::CurInstHI;


Vu::Recompiler* vrs [ 2 ];


VU *VU0::_VU0;
VU *VU1::_VU1;

int VU::iInstance = 0;


static u32 VU::bCodeModified [ 2 ];


// bitmaps for recompiler
static VU::Bitmap128 VU::FSrcBitmap;
static u64 VU::ISrcBitmap;

static VU::Bitmap128 VU::FDstBitmap;
static u64 VU::IDstBitmap;



u32* VU::_DebugPC;
u64* VU::_DebugCycleCount;

//u32* VU::_Intc_Master;
u32* VU::_Intc_Stat;
u32* VU::_Intc_Mask;
u32* VU::_R5900_Status_12;
u32* VU::_R5900_Cause_13;
u64* VU::_ProcStatus;



//VU* VU::_VU;


u64* VU::_NextSystemEvent;

u32* VU::_NextEventIdx;


// needs to be removed sometime - no longer needed
u32* VU::DebugCpuPC;




bool VU::DebugWindow_Enabled [ VU::c_iMaxInstances ];

#ifdef ENABLE_GUI_DEBUGGER
WindowClass::Window *VU::DebugWindow [ VU::c_iMaxInstances ];
DebugValueList<float> *VU::GPR_ValueList [ VU::c_iMaxInstances ];
DebugValueList<u32> *VU::COP0_ValueList [ VU::c_iMaxInstances ];
DebugValueList<u32> *VU::COP2_CPCValueList [ VU::c_iMaxInstances ];
DebugValueList<u32> *VU::COP2_CPRValueList [ VU::c_iMaxInstances ];
Debug_DisassemblyViewer *VU::DisAsm_Window [ VU::c_iMaxInstances ];
Debug_BreakpointWindow *VU::Breakpoint_Window [ VU::c_iMaxInstances ];
Debug_MemoryViewer *VU::ScratchPad_Viewer [ VU::c_iMaxInstances ];
Debug_BreakPoints *VU::Breakpoints [ VU::c_iMaxInstances ];
#endif



Debug::Log VU::debug;



static const char* VU::VU0RegNames [ 24 ] = { "STAT", "FBRST", "ERR", "MARK", "CYCLE", "MODE", "NUM", "MASK", "CODE", "ITOPS", "RES", "RES", "RES", "ITOP", "RES", "RES",
											"R0", "R1", "R2", "R3", "C0", "C1", "C2", "C3" };
											
static const char* VU::VU1RegNames [ 24 ] = { "STAT", "FBRST", "ERR", "MARK", "CYCLE", "MODE", "NUM", "MASK", "CODE", "ITOPS", "BASE", "OFST", "TOPS", "ITOP", "TOP", "RES",
											"R0", "R1", "R2", "R3", "C0", "C1", "C2", "C3" };




VU::VU ()
{

	cout << "Running VU constructor...\n";
}

void VU::Reset ()
{
	// zero object
	memset ( this, 0, sizeof( VU ) );

	// reset new buffers for the flags
	Reset_CFBuffer ();
	Reset_MFBuffer ();
	Reset_SFBuffer ();
	Reset_BFBuffer ();
	
#ifdef ENABLE_NEW_QP_HANDLING
	// with the new Q,P reg handling, -1 means that the regs are not processing
	PBusyUntil_Cycle = -1LL;
	QBusyUntil_Cycle = -1LL;
#endif
}



// actually, you need to start objects after everything has been initialized
void VU::Start ( int iNumber )
{
	cout << "Running VU::Start...\n";
	
	
#ifdef INLINE_DEBUG_ENABLE
	if ( !iNumber )
	{
#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create ( "PS2_VU_Log.txt" );
	}
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering VU::Start";
#endif


	cout << "Resetting VU...\n";

	Reset ();

	cout << "Starting VU Lookup object...\n";
	
	Vu::Instruction::Lookup::Start ();
	
	cout << "Starting VU Print object...\n";
	
	Vu::Instruction::Print::Start ();
	
	// only need to start the VU Execute once, or else there are issues with debugging
	if ( !iNumber )
	{
		Vu::Instruction::Execute::Start ();
	}
	
	// set the vu number
	Number = iNumber;
	
	// set as current GPU object
	if ( Number < c_iMaxInstances )
	{
		_VU [ Number ] = this;
		
		if ( !Number )
		{
			ulMicroMem_Start = c_ulMicroMem0_Start;
			ulMicroMem_Size = c_ulMicroMem0_Size;
			ulMicroMem_Mask = c_ulMicroMem0_Mask;
			ulVuMem_Start = c_ulVuMem0_Start;
			ulVuMem_Size = c_ulVuMem0_Size;
			ulVuMem_Mask = c_ulVuMem0_Mask;
		}
		else
		{
			ulMicroMem_Start = c_ulMicroMem1_Start;
			ulMicroMem_Size = c_ulMicroMem1_Size;
			ulMicroMem_Mask = c_ulMicroMem1_Mask;
			ulVuMem_Start = c_ulVuMem1_Start;
			ulVuMem_Size = c_ulVuMem1_Size;
			ulVuMem_Mask = c_ulVuMem1_Mask;
		}
		
		// initialize zero registers
		_VU [ Number ]->vi [ 0 ].u = 0;
		
		// f0 always 0, 0, 0, 1 ???
		_VU [ Number ]->vf [ 0 ].uLo = 0;
		_VU [ Number ]->vf [ 0 ].uHi = 0;
		_VU [ Number ]->vf [ 0 ].uw = 0x3f800000;



		//Recompiler ( VU* v, u32 NumberOfBlocks, u32 BlockSize_PowerOfTwo, u32 MaxIStep );
		if ( Number )
		{
			vrs [ Number ] = new Recompiler ( this, 0, 21, 11 );
		}
		else
		{
			vrs [ Number ] = new Recompiler ( this, 0, 21, 9 );
		}
		
		//rs->SetOptimizationLevel ( 1 );
		//vrs [ Number ]->SetOptimizationLevel ( 0 );
		vrs [ Number ]->SetOptimizationLevel ( 1 );
		
		// enable recompiler by default
		bEnableRecompiler = true;

		// should be set when code is modified (start out as code modified to force recompile)
		bCodeModified [ Number ] = 1;
		
#ifdef ENABLE_GUI_DEBUGGER
		cout << "\nVU#" << Number << " breakpoint instance";
		Breakpoints [ Number ] = new Debug_BreakPoints ( NULL, NULL, NULL );
#endif
	}


	// start as not in use
	CycleCount = -1LL;
	//SetNextEvent_Cycle ( -1LL );
	

	// update number of object instances
	iInstance++;

	

	cout << "done\n";

#ifdef INLINE_DEBUG
	debug << "->Exiting VU::Start";
#endif

	cout << "Exiting VU::Start...\n";
}








void VU::SetNextEvent ( u64 CycleOffset )
{
	NextEvent_Cycle = CycleOffset + *_DebugCycleCount;
	
	Update_NextEventCycle ();
}


void VU::SetNextEvent_Cycle ( u64 Cycle )
{
	NextEvent_Cycle = Cycle;
	
	Update_NextEventCycle ();
}

//void VU::Update_NextEventCycle ()
//{
//	if ( NextEvent_Cycle > *_DebugCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_DebugCycleCount ) ) *_NextSystemEvent = NextEvent_Cycle;
//}

void VU::Update_NextEventCycle ()
{
	//if ( NextEvent_Cycle > *_DebugCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_DebugCycleCount ) )
	if ( NextEvent_Cycle < *_NextSystemEvent )
	{
		*_NextSystemEvent = NextEvent_Cycle;
		*_NextEventIdx = NextEvent_Idx;
	}
}





void VU::Write_CTC ( u32 Register, u32 Data )
{
	// check if INT reg
	if ( Register < 16 )
	{
		vi [ Register ].sLo = (s16) Data;
	}
	else
	{
		// control register //
		
		switch ( Register )
		{
			// STATUS FLAG
			case 16:
				// lower 6-bits are ignored when writing status flag
				vi [ 16 ].u = ( vi [ 16 ].u & 0x3f ) | ( Data & 0xfc0 );
				break;
				
			// FBRST
			case 28:
				
				// check if we need to reset VU0
				if ( Data & 0x2 )
				{
					// reset VU0
					// do this for now
					_VU [ 0 ]->lVifIdx = 0;
					_VU [ 0 ]->lVifCodeState = 0;
					
					// set DBF to zero (for VU double buffering)
					_VU [ 0 ]->VifRegs.STAT.DBF = 0;
				}
				
				// check if we need to reset VU1
				if ( Data & 0x200 )
				{
					// reset VU1
					// do this for now
					_VU [ 1 ]->lVifIdx = 0;
					_VU [ 1 ]->lVifCodeState = 0;
					
					// set DBF to zero (for VU double buffering)
					_VU [ 1 ]->VifRegs.STAT.DBF = 0;
				}
				
				// clear bits 0,1 and 8,9
				Data &= ~0x303;
				
				// write register value
				vi [ Register ].u = Data;
				
				break;
				
			// CMSAR1 - runs vu1 from address on write
			case 31:
				cout << "\nhps2x64: ALERT: writing to CMSAR1!\n";
				vi [ Register ].u = Data;
				break;
				
			default:
				vi [ Register ].u = Data;
				break;
		}
	}
	
	
}

u32 VU::Read_CFC ( u32 Register )
{
	switch ( Register )
	{
		// FBRST register
		// bits 0,1,4-7,8,9,12-31 always read zero
		case 28:
			return vi [ Register ].u & ~0xfffff3f3;
			break;
			
		default:
			return vi [ Register ].u;
			break;
	}
}




u64 VU::Read ( u32 Address, u64 Mask )
{
	//u32 Temp;
	u64 Output = 0;
	u32 lReg;

#ifdef INLINE_DEBUG_READ
	debug << "\r\n\r\nVU::Read; " << hex << setw ( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << " Mask=" << Mask;
	debug << "; VU#" << Number;
	if ( !Number )
	{
	if ( ( ( ( Address & 0xffff ) - 0x3800 ) >> 4 ) < c_iNumVuRegs )
	{
	// vu0
	debug << "; " << VU0RegNames [ ( ( Address & 0xffff ) - 0x3800 ) >> 4 ];
	}
	}
	else
	{
	if ( ( ( ( Address & 0xffff ) - 0x3c00 ) >> 4 ) < c_iNumVuRegs )
	{
	// vu1
	debug << "; " << VU1RegNames [ ( ( Address & 0xffff ) - 0x3c00 ) >> 4 ];
	}
	}
#endif

	// check if Address is for the registers or the fifo
	if ( ( Address & 0xffff ) < 0x4000 )
	{
		// get the register number
		lReg = ( Address >> 4 ) & 0x1f;
		
		// will set some values here for now
		switch ( lReg )
		{
			default:
				break;
		}
		
		
		if ( lReg < c_iNumVuRegs )
		{
			Output = VifRegs.Regs [ lReg ];
		}

		switch ( lReg )
		{
			// STAT
			case 0x0:
			
				// TESTING
				// clear VPS after a read for testing
				//VifRegs.STAT.VPS = 0x0;
				
				break;
				
			default:
				break;
		}
	}
	
	
#ifdef INLINE_DEBUG_READ
	debug << "; Output=" << hex << Output;
#endif

	return Output;
}


void VU::Write ( u32 Address, u64 Data, u64 Mask )
{
	u32 lReg;
	u32 QWC_Transferred;
	//u32 ulTempArray [ 4 ];
	
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\n\r\nVU::Write; " << hex << setw ( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data << " Mask=" << Mask;
	debug << "; VU#" << Number;
	if ( !Number )
	{
	if ( ( ( ( Address & 0xffff ) - 0x3800 ) >> 4 ) < c_iNumVuRegs )
	{
	// vu0
	debug << "; " << VU0RegNames [ ( ( Address & 0xffff ) - 0x3800 ) >> 4 ];
	}
	else if ( ( Address & 0xffff ) == 0x4000 )
	{
	debug << "; VIF0FIFO";
	}
	}
	else
	{
	if ( ( ( ( Address & 0xffff ) - 0x3c00 ) >> 4 ) < c_iNumVuRegs )
	{
	// vu1
	debug << "; " << VU1RegNames [ ( ( Address & 0xffff ) - 0x3c00 ) >> 4 ];
	}
	else if ( ( Address & 0xffff ) == 0x5000 )
	{
	debug << "; VIF1FIFO";
	}
	}
#endif

	if ( ( Address & 0xffff ) < 0x4000 )
	{
		// get register number being written to
		lReg = ( Address >> 4 ) & 0x1f;
		
		// perform actions before write
		switch ( lReg )
		{
			// STAT
			case 0x0:
			
				// this might be getting or'ed with 0x1f000000 ?
				// no, thats no it
				//Data |= 0x1f000000;
				break;
				
			// FBRST
			case 0x1:
			
				// when 1 is written, it means to reset VU completely
				// bit 0 - reset VU (when writing to memory mapped register. when writing to VU0 int reg 28, then it is bit 1)
				if ( Data & 0x1 )
				{
					// reset VU //
					//Reset ();
					
					// do this for now
					// make sure to specify VU Number (was specifying vu#0 ??)
					//_VU [ 0 ]->lVifIdx = 0;
					//_VU [ 0 ]->lVifCodeState = 0;
					lVifIdx = 0;
					lVifCodeState = 0;
					
					// vif no longer stopped if stopped ??
					VifStopped = 0;
					
					// set DBF to zero (for VU double buffering)
					//_VU [ 0 ]->VifRegs.STAT.DBF = 0;
					//VifRegs.STAT.DBF = 0;
					// clear stat completely
					VifRegs.STAT.Value = 0;
					
					// clear bit 0
					//Data &= 0x1;
					Data &= ~0x1;
				}
				
				// writing 1 to STC (bit 3) clears VSS,VFS,VIS,INT,ER0,ER1 in STAT
				// also cancels any stalls
				if ( Data & 0x8 )
				{
					VifRegs.STAT.VSS = 0;
					VifRegs.STAT.VFS = 0;
					VifRegs.STAT.VIS = 0;
					VifRegs.STAT.INT = 0;
					VifRegs.STAT.ER0 = 0;
					VifRegs.STAT.ER1 = 0;
					
					// also should cancel any stalls for VIF
					VifStopped = 0;
					
					// backup cycle - to prevent skips for now
					// don't do this, since it is cool if done during a write/read
					//*_DebugCycleCount--;
					
					// ?? re-start dma after cancelling stall ??
					Dma::_DMA->Transfer ( Number );
					
					// restore cycle - to prevent skips for now
					//*_DebugCycleCount++;
				}
				
				break;
				
			// MARK
			case 0x3:
			
				// STAT.MRK gets cleared when CPU writes to MARK register
				VifRegs.STAT.MRK = 0;
				break;
				
			// CODE register is probably read-only
			case 0x8:
				return;
				break;
				
				
			default:
				break;
		}

		
		if ( lReg < c_iNumVuRegs )
		{
			VifRegs.Regs [ lReg ] = Data;
		}

		// perform actions after write
		switch ( Address )
		{
			default:
				break;
		}
	}
	else
	{
		// Write to VIF FIFO address //
		
		
		QWC_Transferred = VIF_FIFO_Execute ( (u32*) Data, 4 );
		
		if ( !QWC_Transferred )
		{
#ifdef INLINE_DEBUG_WRITE
			debug << " QWC_Transferred=" << dec << QWC_Transferred;
#endif

			cout << "\nhps2x64 ALERT: VU: non-dma transfer did not completely execute\n";
		}
	}
}


// VIF Codes are 32-bits. can always do a cast to 64/128-bits as needed
// need this to return the number of quadwords read and update the offset so it points to correct data for next time
u32 VU::VIF_FIFO_Execute ( u32* Data, u32 SizeInWords32 )
{
	u32 ulTemp;
	u32 *DstPtr32;
	
	u32 lWordsToTransfer;
	
	u32 QWC_Transferred;
	s32 UnpackX, UnpackY, UnpackZ, UnpackW;
	//s32 lUnpackTemp;
	union { s32 lUnpackData; float fUnpackData; };
	u32 ulUnpackMask;
	u32 ulUnpackRow, ulUnpackCol;
	u32 ulWriteCycle;
	
	u64 PreviousValue;
	
	// need an index for just the data being passed
	u32 lIndex = 0;
	
	u32 lWordsLeft = SizeInWords32;
	
	// also need to subtract lVifIdx & 0x3 from the words left
	// to take over where we left off at
	lWordsLeft -= ( lVifIdx & 0x3 );
	
#ifdef INLINE_DEBUG_VUCOM
	debug << " lVifIdx=" << hex << lVifIdx;
#endif

	// start reading from index offset (may have left off in middle of data QW block)
	Data = & ( Data [ lVifIdx & 0x3 ] );
	
	// TESTING
	// this should mean that VIF is busy
	//VifRegs.STAT.VPS = 0x2;
	
	while ( lWordsLeft )
	{
		// check if vif code processing is in progress
		if ( !lVifCodeState )
		{
			// load vif code
			//VifCode.Value = Data [ lIndex++ ];
			VifCode.Value = *Data++;
			
			// store to CODE register
			VifRegs.CODE = VifCode.Value;
			
			// is now busy transferring data after vif code
			VifRegs.STAT.VPS = 3;
			
			// update index
			lVifIdx++;
			
			// vif code is 1 word
			lWordsLeft--;
		}
		
		// perform the action for the vif code
		switch ( VifCode.CMD )
		{
			// NOP
			case 0:
#ifdef INLINE_DEBUG_VUCOM
	debug << " NOP";
#endif

			
				break;
			
			// STCYCL
			case 1:
#ifdef INLINE_DEBUG_VUCOM
	debug << " STCYCL";
#endif

				// set CYCLE register value - first 8 bits are CL, next 8-bits are WL
				// set from IMM
				// Vu0 and Vu1
				VifRegs.CYCLE.Value = VifCode.IMMED;
				break;

			// OFFSET
			case 2:
#ifdef INLINE_DEBUG_VUCOM
	debug << " OFFSET";
#endif

				// wait if vu is running ??
				if ( Running )
				{
					// a VU program is in progress already //
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFSTOP)";
#endif

					VifStopped = 1;
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (MSCNT_PENDING)";
#endif

					// there is a pending MSCAL Command -> for now just back up index
					lVifIdx--;
					
					// just return for now //
					
					// get the number of full quadwords completely processed
					QWC_Transferred = lVifIdx >> 2;
					
					// get start index of remaining data to process in current block
					lVifIdx &= 0x3;
					
					// return the number of full quadwords transferred/processed
					return QWC_Transferred;
				}
				else
				{
				
					// VU program is NOT in progress //
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFCNT)";
#endif

					VifStopped = 0;

				// set OFFSET register value - first 10 bits of IMM
				// set from IMM
				// Vu1 ONLY
				VifRegs.OFST = VifCode.IMMED;
				
				// ??? only clear DBF flag if VU not running ???
				/*
				if ( !Running )
				{
				*/
					// set DBF Flag to zero
					// *note* probably should not do this. probably only reset should set it to zero
					VifRegs.STAT.DBF = 0;
					
				/*
				}
				*/
				
				// set TOPS to BASE
				// *note* should probably use DBF to see what this gets set to
				VifRegs.TOPS = VifRegs.BASE;
				// set TOPS immediately
				//VifRegs.TOPS = VifRegs.BASE + ( VifRegs.OFST * ( (u32) VifRegs.STAT.DBF ) );
				
#ifdef INLINE_DEBUG_VUCOM
	debug << hex << " (OFFSET=" << VifRegs.OFST << " TOPS=" << VifRegs.TOPS << ")";
#endif

				}

				break;

			// BASE
			case 3:
#ifdef INLINE_DEBUG_VUCOM
	debug << " BASE";
#endif

				// set BASE register value - first 10 bits of IMM
				// set from IMM
				// Vu1 ONLY
				VifRegs.BASE = VifCode.IMMED;
				
#ifdef INLINE_DEBUG_VUCOM
	debug << hex << " (BASE=" << VifRegs.BASE << ")";
#endif
				break;

			// ITOP
			case 4:
#ifdef INLINE_DEBUG_VUCOM
	debug << " ITOP";
#endif

				// set data pointer (ITOPS register) - first 10 bits of IMM
				// set from IMM
				// Vu0 and Vu1
				VifRegs.ITOPS = VifCode.IMMED;
				break;

			// STMOD
			case 5:
#ifdef INLINE_DEBUG_VUCOM
	debug << " STMOD";
#endif

				// set mode of addition decompression (MODE register) - first 2 bits of IMM
				// set from IMM
				// Vu0 and Vu1
				VifRegs.MODE = VifCode.IMMED;
				break;

			// MSKPATH3
			case 6:
#ifdef INLINE_DEBUG_VUCOM
	debug << " MSKPATH3";
#endif


				// set mask of path3 - bit 15 - 0: enable path3 transfer, 1: disable path3 transfer
				// set from IMM
				// Vu1 ONLY
				
				PreviousValue = GPU::_GPU->GIFRegs.STAT.M3P;
				
				// set bit 15 of mskpath3 to bit 1 of gif_stat (M3P)
				GPU::_GPU->GIFRegs.STAT.M3P = ( ( VifCode.Value & ( 1 << 15 ) ) >> 15 );
				
				if ( PreviousValue )
				{
					// check for a transition from 1 to 0
					if ( !GPU::_GPU->GIFRegs.STAT.M3R && !GPU::_GPU->GIFRegs.STAT.M3P )
					{
						// path 3 mask is being disabled //
#ifdef INLINE_DEBUG_VUCOM
	debug << "\r\n*** PATH3 BEING UN-MASKED VIA VU ***\r\n";
#endif
#ifdef VERBOSE_MSKPATH3
	cout << "\n*** PATH3 BEING UN-MASKED VIA VU ***\n";
#endif

						// backup cycle - to prevent skips for now
						// don't do this, since it is cool if done during a write/read
						//*_DebugCycleCount--;
						
						// restart dma#2
						// note: should restart on its own
						//Dma::_DMA->Transfer ( 2 );
						
						// restore cycle - to prevent skips for now
						//*_DebugCycleCount++;
					}
				}
				
				if ( !PreviousValue )
				{
					if ( GPU::_GPU->GIFRegs.STAT.M3P )
					{
#ifdef INLINE_DEBUG_VUCOM
	debug << "\r\n*** PATH3 BEING MASKED VIA VU ***\r\n";
#endif
#ifdef VERBOSE_MSKPATH3
	cout << "\n*** PATH3 BEING MASKED VIA VU ***\n";
#endif
					}
				}
				
				break;

			// MARK
			case 7:
#ifdef INLINE_DEBUG_VUCOM
	debug << " MARK";
#endif

#ifdef VERBOSE_VU_MARK
				// debug
				cout << "\nhps2x64 ALERT: VU: Mark encountered!!!\n";
#endif
				
				// set mark register
				// set from IMM
				// Vu0 and Vu1
				VifRegs.MARK = VifCode.IMMED;
				
				// also need to set MRK bit in STAT
				// this gets cleared when CPU writes to MARK register
				VifRegs.STAT.MRK = 1;
				
				break;

			// FLUSHE
			case 16:
#ifdef INLINE_DEBUG_VUCOM
	debug << " FLUSHE";
#endif

				// waits for vu program to finish
				// Vu0 and Vu1
				
				if ( Running )
				{
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFSTOP)";
#endif

					VifStopped = 1;
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (FLUSHE_PENDING)";
#endif

					// there is a pending FLUSHE Command -> for now just back up index
					lVifIdx--;
					
					// just return for now //
					
					// get the number of full quadwords completely processed
					QWC_Transferred = lVifIdx >> 2;
					
					// get start index of remaining data to process in current block
					lVifIdx &= 0x3;
					
					// return the number of full quadwords transferred/processed
					return QWC_Transferred;
				}
				else
				{
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFCNT)";
#endif

					VifStopped = 0;
				}
				
				break;

			// FLUSH
			case 17:
#ifdef INLINE_DEBUG_VUCOM
	debug << " FLUSH";
#endif

				// waits for end of vu program and end of path1/path2 transfer
				// Vu1 ONLY
				// ***todo*** wait for end of path1/path2 transfer
				if ( Running )
				{
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFSTOP)";
#endif

					VifStopped = 1;
					
					
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (FLUSH_PENDING)";
#endif

					
					// there is a pending FLUSH Command -> for now just back up index
					lVifIdx--;
					
					// just return for now //
					
					// get the number of full quadwords completely processed
					QWC_Transferred = lVifIdx >> 2;
					
					// get start index of remaining data to process in current block
					lVifIdx &= 0x3;
					
					// return the number of full quadwords transferred/processed
					return QWC_Transferred;
				}
				else
				{
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFCNT)";
#endif

					VifStopped = 0;
					
#ifdef ENABLE_SET_VGW
					// not waiting for end of GIF transfer currently
					VifRegs.STAT.VGW = 0;
#endif

				}
				
				break;

			// FLUSHA
			case 19:
#ifdef INLINE_DEBUG_VUCOM
	debug << " FLUSHA";
#endif

				// waits for end of vu program and end of transfer to gif
				// Vu1 ONLY
				// ***todo*** wait for end of transfer to gif
				//if ( Running || ( Dma::cbReady [ 2 ] () ) )
				//if ( Running || ( Dma::pRegData [ 2 ]->CHCR.STR && !( GPU::_GPU->GIFRegs.STAT.M3R || GPU::_GPU->GIFRegs.STAT.M3P ) ) )
				//if ( Running || ( GPU::_GPU->ulTransferCount [ 3 ] ) )
				//if ( Running || ( !GPU::_GPU->EndOfPacket [ 3 ] ) )
				if ( Running || ( Dma::pRegData [ 2 ]->CHCR.STR && GPU::DMA_Write_Ready() ) )
				{
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFSTOP)";
#endif

					VifStopped = 1;
					
#ifdef ENABLE_SET_VGW
					//if ( Dma::pRegData [ 2 ]->CHCR.STR && !( GPU::_GPU->GIFRegs.STAT.M3R || GPU::_GPU->GIFRegs.STAT.M3P ) )
					//if ( GPU::_GPU->ulTransferCount [ 3 ] )
					//if ( !GPU::_GPU->EndOfPacket [ 3 ] )
					//{
						// when vif is stalled via DIRECTHL or FLUSHA, need to set STAT.VGW
						VifRegs.STAT.VGW = 1;
					//}
#endif
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (FLUSHA_PENDING)";
#endif

					// there is a pending FLUSHA Command -> for now just back up index
					lVifIdx--;
					
					// just return for now //
					
					// get the number of full quadwords completely processed
					QWC_Transferred = lVifIdx >> 2;
					
					// get start index of remaining data to process in current block
					lVifIdx &= 0x3;
					
					// return the number of full quadwords transferred/processed
					return QWC_Transferred;
				}
				else
				{
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFCNT)";
#endif

					VifStopped = 0;
					
#ifdef ENABLE_SET_VGW
					// not waiting for end of GIF transfer currently
					VifRegs.STAT.VGW = 0;
#endif
				}
				
				break;

			// MSCAL
			case 20:
#ifdef INLINE_DEBUG_VUCOM
	debug << " MSCAL";
#endif

				// waits for end of current vu program and runs new vu program starting at IMM*8
				// Vu0 and Vu1
				
				if ( Running )
				{
					// a VU program is in progress already //
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFSTOP)";
#endif

					VifStopped = 1;
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (MSCAL_PENDING)";
#endif

					// there is a pending MSCAL Command -> for now just back up index
					lVifIdx--;
					
					// just return for now //
					
					// get the number of full quadwords completely processed
					QWC_Transferred = lVifIdx >> 2;
					
					// get start index of remaining data to process in current block
					lVifIdx &= 0x3;
					
					// return the number of full quadwords transferred/processed
					return QWC_Transferred;
				}
				else
				{
					// VU program is NOT in progress //
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFCNT)";
#endif

					VifStopped = 0;
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (RUNVU)";
#endif

					// set PC = IMM * 8
					PC = VifCode.IMMED << 3;
					
					// need to set next pc too since this could get called in the middle of VU Main CPU loop
					NextPC = PC;
					
					// VU is now running
					Running = 1;
					CycleCount = *_DebugCycleCount + 1;
					//SetNextEvent_Cycle ( *_DebugCycleCount + 1 );
					
					// set VBSx in VPU STAT to 1 (running)
					VU0::_VU0->vi [ 29 ].uLo |= ( 1 << ( Number << 3 ) );
					
					// also set VIFx STAT to indicate program is running
					VifRegs.STAT.VEW = 1;
					
					
					// ***todo*** also set VPU STAT COP2 r29 ??
					

					// set TOP to TOPS
					VifRegs.TOP = VifRegs.TOPS;
					
					// set ITOP to ITOPS ??
					VifRegs.ITOP = VifRegs.ITOPS;
					
					// reverse DBF flag
					VifRegs.STAT.DBF ^= 1;
					
					// set TOPS
					VifRegs.TOPS = VifRegs.BASE + ( VifRegs.OFST * ( (u32) VifRegs.STAT.DBF ) );
					
#ifdef VERBOSE_MSCAL
					// debugging
					cout << "\nhps2x64: VU#" << Number << ": MSCAL";
					cout << " StartPC=" << hex << PC;
#endif
					
#ifdef INLINE_DEBUG_VUEXECUTE
					Vu::Instruction::Execute::debug << "\r\n*** MSCAL";
					Vu::Instruction::Execute::debug << " VU#" << Number;
					Vu::Instruction::Execute::debug << " StartPC=" << hex << PC;
					Vu::Instruction::Execute::debug << " VifCode=" << hex << VifCode.Value;
					Vu::Instruction::Execute::debug << " ***";
#endif
				}
				
				
				break;

			// MSCALF
			case 21:
#ifdef INLINE_DEBUG_VUCOM
	debug << " MSCALF";
#endif

				// waits for end of current vu program and gif path1/path2 transfer, then runs program starting from IMM*8
				// Vu0 and Vu1
				
				// *** TODO *** wait for end of PATH1/PATH2 transfer
				if ( Running )
				{
					// VU program is in progress //
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFSTOP)";
#endif

					VifStopped = 1;
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (MSCALF_PENDING)";
#endif

					// there is a pending MSCALF Command -> for now just back up index
					lVifIdx--;
					
					// just return for now //
					
					// get the number of full quadwords completely processed
					QWC_Transferred = lVifIdx >> 2;
					
					// get start index of remaining data to process in current block
					lVifIdx &= 0x3;
					
					// return the number of full quadwords transferred/processed
					return QWC_Transferred;
				}
				else
				{
					// VU program is NOT in progress //
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFCNT)";
#endif

					VifStopped = 0;
					
#ifdef ENABLE_SET_VGW
					// not waiting for end of GIF transfer currently
					VifRegs.STAT.VGW = 0;
#endif
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (RUNVU)";
#endif

					// set PC = IMM * 8
					PC = VifCode.IMMED << 3;
					
					// need to set next pc too since this could get called in the middle of VU Main CPU loop
					NextPC = PC;
					
					// VU is now running
					Running = 1;
					CycleCount = *_DebugCycleCount + 1;
					//SetNextEvent_Cycle ( *_DebugCycleCount + 1 );
					
					// set VBSx in VPU STAT to 1 (running)
					VU0::_VU0->vi [ 29 ].uLo |= ( 1 << ( Number << 3 ) );
					
					// also set VIFx STAT to indicate program is running
					VifRegs.STAT.VEW = 1;
					
					// set TOPS to TOP
					VifRegs.TOP = VifRegs.TOPS;
					
					// set ITOPS to ITOP ??
					VifRegs.ITOP = VifRegs.ITOPS;
					
					// reverse DBF flag
					VifRegs.STAT.DBF ^= 1;
					
					// set TOPS
					VifRegs.TOPS = VifRegs.BASE + ( VifRegs.OFST * ( (u32) VifRegs.STAT.DBF ) );
					
#ifdef VERBOSE_MSCALF
					// debugging
					cout << "\nhps2x64: VU#" << Number << ": MSCALF";
					cout << " StartPC=" << hex << PC;
#endif
					
#ifdef INLINE_DEBUG_VUEXECUTE
					Vu::Instruction::Execute::debug << "\r\n*** MSCALF";
					Vu::Instruction::Execute::debug << " VU#" << Number;
					Vu::Instruction::Execute::debug << " StartPC=" << hex << PC;
					Vu::Instruction::Execute::debug << " VifCode=" << hex << VifCode.Value;
					Vu::Instruction::Execute::debug << " ***";
#endif
				}
				
				break;
				
				
			// MSCNT
			case 23:
#ifdef INLINE_DEBUG_VUCOM
	debug << " MSCNT";
#endif

				// waits for end of current vu program and starts next one where it left off
				// Vu0 and Vu1
				
				if ( Running )
				{
					// VU program is in progress //
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFSTOP)";
#endif

					VifStopped = 1;
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (MSCNT_PENDING)";
#endif

					// there is a pending MSCNT Command -> for now just back up index
					lVifIdx--;
					
					// just return for now //
					
					// get the number of full quadwords completely processed
					QWC_Transferred = lVifIdx >> 2;
					
					// get start index of remaining data to process in current block
					lVifIdx &= 0x3;
					
					// return the number of full quadwords transferred/processed
					return QWC_Transferred;
				}
				else
				{
					// VU program is NOT in progress //
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFCNT)";
#endif

					VifStopped = 0;
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (RUNVU)";
#endif

					// VU is now running
					Running = 1;
					CycleCount = *_DebugCycleCount + 1;
					//SetNextEvent_Cycle ( *_DebugCycleCount + 1 );
					
					// set VBSx in VPU STAT to 1 (running)
					VU0::_VU0->vi [ 29 ].uLo |= ( 1 << ( Number << 3 ) );
					
					// also set VIFx STAT to indicate program is running
					VifRegs.STAT.VEW = 1;
					
					// set TOP to TOPS
					VifRegs.TOP = VifRegs.TOPS;
					
					// set ITOP to ITOPS ??
					VifRegs.ITOP = VifRegs.ITOPS;
					
					// reverse DBF flag
					VifRegs.STAT.DBF ^= 1;
					
					// set TOPS
					VifRegs.TOPS = VifRegs.BASE + ( VifRegs.OFST * ( (u32) VifRegs.STAT.DBF ) );
					
#ifdef VERBOSE_MSCNT
					// debugging
					cout << "\nhps2x64: VU#" << Number << ": MSCNT";
					cout << " StartPC=" << hex << PC;
#endif
					
#ifdef INLINE_DEBUG_VUEXECUTE
					Vu::Instruction::Execute::debug << "\r\n*** MSCNT";
					Vu::Instruction::Execute::debug << " VU#" << Number;
					Vu::Instruction::Execute::debug << " PC=" << hex << PC;
					Vu::Instruction::Execute::debug << " VifCode=" << hex << VifCode.Value;
					Vu::Instruction::Execute::debug << " ***";
#endif
				}
				
				break;

			// STMASK
			case 32:
#ifdef INLINE_DEBUG_VUCOM
	debug << " STMASK";
#endif

				// Vu0 and Vu1
				
				if ( !lVifCodeState )
				{
					// there is more incoming data than just the tag
					lVifCodeState++;
				}

				// if processing next element, also check there is data to process
				if ( lWordsLeft )
				{
					// store mask
					//VifRegs.MASK = Data [ lIndex++ ];
					VifRegs.MASK = *Data++;
					
					// update index
					lVifIdx++;
					
					// command size is 1+1 words
					lWordsLeft--;
					
					// command is done
					lVifCodeState = 0;
				}
				
				break;

			// STROW
			case 48:
#ifdef INLINE_DEBUG_VUCOM
	debug << " STROW";
#endif

				static const int c_iSTROW_CommandSize = 5;

				// Vu0 and Vu1
				
				if ( !lVifCodeState )
				{
					// there is more incoming data than just the tag
					lVifCodeState++;
				}

				// if processing next element, also check there is data to process
				if ( lWordsLeft )
				{
					while ( lWordsLeft && ( lVifCodeState < c_iSTROW_CommandSize ) )
					{
						// store ROW (R0-R3)
						// register offset starts at 16, buf lVifCodeState will start at 1
						//VifRegs.Regs [ 15 + lVifCodeState ] = Data [ lIndex++ ];
						VifRegs.Regs [ c_iRowRegStartIdx + lVifCodeState - 1 ] = *Data++;
						
						// update index
						lVifIdx++;
						
						// move to next data input element
						lVifCodeState++;
						
						// command size is 1+4 words
						lWordsLeft--;
					}
					
					if ( lVifCodeState >= c_iSTROW_CommandSize )
					{
						// command is done
						lVifCodeState = 0;
					}
				}
				
				break;

			// STCOL
			case 49:
#ifdef INLINE_DEBUG_VUCOM
	debug << " STCOL";
#endif

				static const int c_iSTCOL_CommandSize = 5;

				// Vu0 and Vu1
				
				if ( !lVifCodeState )
				{
					// there is more incoming data than just the tag
					lVifCodeState++;
				}

				// if processing next element, also check there is data to process
				if ( lWordsLeft )
				{
					while ( lWordsLeft && ( lVifCodeState < c_iSTCOL_CommandSize ) )
					{
						// store ROW (R0-R3)
						// register offset starts at 16, buf lVifCodeState will start at 1
						//VifRegs.Regs [ 19 + lVifCodeState ] = Data [ lIndex++ ];
						VifRegs.Regs [ c_iColRegStartIdx + lVifCodeState - 1 ] = *Data++;
						
						// update index
						lVifIdx++;
						
						// move to next data input element
						lVifCodeState++;
						
						// command size is 1+4 words
						lWordsLeft--;
					}
					
					if ( lVifCodeState >= c_iSTCOL_CommandSize )
					{
						// command is done
						lVifCodeState = 0;
					}
				}
				
				break;

			// MPG
			case 74:
#ifdef INLINE_DEBUG_VUCOM
	debug << " MPG";
#endif

				// load following vu program of size NUM*2 into address IMM*8
				// Vu0 and Vu1
				
				// ***TODO*** MPG is supposed to wait for the end of the microprogram before executing
				
				if ( Running )
				{
					// VU program is in progress //
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFSTOP)";
#endif

					VifStopped = 1;
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (MPG_PENDING)";
#endif

					// there is a pending DIRECT Command -> for now just back up index
					lVifIdx--;
					
					// just return for now //
					
					// get the number of full quadwords completely processed
					QWC_Transferred = lVifIdx >> 2;
					
					// get start index of remaining data to process in current block
					lVifIdx &= 0x3;
					
					// return the number of full quadwords transferred/processed
					return QWC_Transferred;
				}
				else
				{
					// VU program is NOT in progress //
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFCNT)";
#endif

					VifStopped = 0;
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (RUNMPG)";
#endif
					if ( !lVifCodeState )
					{
						// set the command size
						// when zero is specified, it means 256
						lVifCommandSize = 1 + ( ( ( !VifCode.NUM ) ? 256 : VifCode.NUM ) << 1 );
						
						// for MPG command, NUM register is set to NUM field
						VifRegs.NUM = VifCode.NUM;
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " CommandSize32=" << dec << lVifCommandSize;
#endif

						// there is more incoming data than just the tag
						lVifCodeState++;
						
						
						// code has been modified for VU
						bCodeModified [ Number ] = 1;
						
#ifdef INLINE_DEBUG_VUEXECUTE
					Vu::Instruction::Execute::debug << "\r\n*** MPG";
					Vu::Instruction::Execute::debug << " VU#" << Number;
					Vu::Instruction::Execute::debug << " VifCode=" << hex << VifCode.Value;
					Vu::Instruction::Execute::debug << " ***";
					// looks like sometimes the VifCode can be "unaligned"
					// it would be unaligned if it were not zero ??
					if ( lVifIdx & 3 )
					{
						Vu::Instruction::Execute::debug << "(UNALIGNED lVifIdx_Offset=" << (lVifIdx & 3) << ")";
					}
#endif
					}

					// if processing next element, also check there is data to process
					if ( lWordsLeft )
					{
						DstPtr32 = & MicroMem32 [ ( VifCode.IMMED << 1 ) + ( lVifCodeState - 1 ) ];
						while ( lWordsLeft && ( lVifCodeState < lVifCommandSize ) )
						{
							// store ROW (R0-R3)
							// register offset starts at 16, buf lVifCodeState will start at 1
							*DstPtr32++ = *Data++;
							
							// update index
							lVifIdx++;
							
							// move to next data input element
							lVifCodeState++;
							
							// command size is 1+NUM*2 words
							lWordsLeft--;
						}
						
						// update NUM register with amount of data left
						// state counts 32-bit words, so shifted right to count 64-bit values
						VifRegs.NUM = ( ( lVifCommandSize - lVifCodeState ) >> 1 ) & 0xff;
						
						if ( lVifCodeState >= lVifCommandSize )
						{
							// command is done
							lVifCodeState = 0;
						}
					}
				}

				break;

			// DIRECT
			case 80:
#ifdef INLINE_DEBUG_VUCOM
	debug << " DIRECT";
#endif

				// transfers data to GIF via path2 (correct GIF Tag required)
				// size 1+IMM*4, but IMM is 65536 when it is zero
				// Vu1 ONLY
				
				// don't perform DIRECT/DIRECTHL command yet while vu1 (path1?) is executing ??

#ifdef HALT_DIRECT_WHILE_VU_RUNNING
				//if ( Running || ( !GPU::_GPU->EndOfPacket [ 3 ] ) )
				if ( Running )
				{
					// VU program is in progress //
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFSTOP)";
#endif

					VifStopped = 1;
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (DIRECT_PENDING)";
#endif

					// there is a pending DIRECT Command -> for now just back up index
					lVifIdx--;
					
					// just return for now //
					
					// get the number of full quadwords completely processed
					QWC_Transferred = lVifIdx >> 2;
					
					// get start index of remaining data to process in current block
					lVifIdx &= 0x3;
					
					// return the number of full quadwords transferred/processed
					return QWC_Transferred;
				}
				else
#endif
				{
					// VU program is NOT in progress //
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFCNT)";
#endif

					VifStopped = 0;
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (RUNDIRECT)";
#endif
					if ( !lVifCodeState )
					{
						// set the command size
						lVifCommandSize = 1 + ( ( !VifCode.IMMED ? 65536 : VifCode.IMMED ) << 2 );
						
						// looks like it might be possible to send unaligned graphics data ??
						// need to assume that direct command always begins with a new packet due to this for now ??
						// note: it does not reset these to zero when direct command is run since it could be transfering graphics data
						// but still need to figure out the unaligned direct/directhl transfers
						//GPU::_GPU->ulTransferCount [ 2 ] = 0;
						//GPU::_GPU->ulTransferSize [ 2 ] = 0;
						
						// for DIRECT command, NUM register is set to value of the IMMEDIATE field
						VifRegs.NUM = VifCode.IMMED;
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " CommandSize32=" << dec << lVifCommandSize;
#endif

						// there is more incoming data than just the tag
						lVifCodeState++;
						
						// direct command is now transferring data
						// gets priority over path 3 for this
						bTransferringDirectViaPath2 = true;
					}

					// if processing next element, also check there is data to process
					if ( lWordsLeft )
					{
						while ( ( lWordsLeft > 0 ) && ( lVifCodeState < lVifCommandSize ) )
						{
							// get amount to transfer from this block
							lWordsToTransfer = lVifCommandSize - lVifCodeState;
							if ( lWordsToTransfer > lWordsLeft ) lWordsToTransfer = lWordsLeft;
							
							// ***TODO*** send graphics data to GIF
							// store ROW (R0-R3)
							// register offset starts at 16, buf lVifCodeState will start at 1
							//GPU::Path2_WriteBlock ( (u64*) Data, lWordsToTransfer >> 2 );
							//GPU::Path2_WriteBlock ( (u64*) Data, lWordsToTransfer >> 1 );
							GPU::Path2_WriteBlock ( Data, lWordsToTransfer );
							
							//Data += 4;
							Data += lWordsToTransfer;
							
							// update index
							lVifIdx += lWordsToTransfer;
							
							// move to next data input element
							//lVifCodeState++;
							//lVifCodeState += 4;
							lVifCodeState += lWordsToTransfer;
							
							// command size is 1+NUM*2 words
							//lWordsLeft--;
							//lWordsLeft -= 4;
							lWordsLeft -= lWordsToTransfer;
						}
						
						// for DIRECT command, NUM register is number of 128-bit QWs left
						VifRegs.NUM = ( ( lVifCommandSize - lVifCodeState ) >> 2 ) & 0xffff;
						
						if ( lVifCodeState >= lVifCommandSize )
						{
							// command is done
							lVifCodeState = 0;
							
							// also need to clear/flush the path2 pipeline
							GPU::_GPU->ulPath2_DataWaiting = 0;
							
							// Direct command no longer transferring data
							bTransferringDirectViaPath2 = false;
						}
					}
					
				}
				
				break;

			// DIRECTHL
			case 81:
#ifdef INLINE_DEBUG_VUCOM
	debug << " DIRECTHL";
#endif

				// transfers data to GIF via path2 (correct GIF Tag required)
				// size 1+IMM*4, but IMM is 65536 when it is zero
				// stalls until PATH3 transfer is complete
				// Vu1 ONLY
				
				// don't perform DIRECT/DIRECTHL command yet while vu1 (path1?) is executing ??

				//if ( Running || ( GPU::_GPU->ulTransferCount [ 3 ] ) )
#ifdef HALT_DIRECTHL_WHILE_VU_RUNNING
				if ( Running || ( !GPU::_GPU->EndOfPacket [ 3 ] ) )
#else
				if ( !GPU::_GPU->EndOfPacket [ 3 ] )
#endif
				{
					// VU program is in progress //
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFSTOP)";
#endif

					VifStopped = 1;
					
#ifdef ENABLE_SET_VGW
					//if ( GPU::_GPU->ulTransferCount [ 3 ] )
					//if ( !GPU::_GPU->EndOfPacket [ 3 ] )
					//{
						// when vif is stalled via DIRECTHL or FLUSHA, need to set STAT.VGW
						VifRegs.STAT.VGW = 1;
					//}
#endif
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (DIRECTHL_PENDING)";
#endif

					// there is a pending DIRECT Command -> for now just back up index
					lVifIdx--;
					
					// just return for now //
					
					// get the number of full quadwords completely processed
					QWC_Transferred = lVifIdx >> 2;
					
					// get start index of remaining data to process in current block
					lVifIdx &= 0x3;
					
					// return the number of full quadwords transferred/processed
					return QWC_Transferred;
				}
				else
				{
					// VU program is NOT in progress //
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFCNT)";
#endif

					VifStopped = 0;
					
#ifdef ENABLE_SET_VGW
					// not waiting for end of GIF transfer currently
					VifRegs.STAT.VGW = 0;
#endif
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (RUNDIRECTHL)";
#endif

					if ( !lVifCodeState )
					{
						// set the command size
						lVifCommandSize = 1 + ( ( !VifCode.IMMED ? 65536 : VifCode.IMMED ) << 2 );
					
						// looks like it might be possible to send unaligned graphics data ??
						// need to assume that direct command always begins with a new packet due to this for now ??
						// note: it does not reset these to zero when directhl command is run since it could be transfering graphics data
						// but still need to figure out the unaligned direct/directhl transfers
						//GPU::_GPU->ulTransferCount [ 2 ] = 0;
						//GPU::_GPU->ulTransferSize [ 2 ] = 0;
						
						// for DIRECTHL command, NUM register is set to value of the IMMEDIATE field
						VifRegs.NUM = VifCode.IMMED;
						
#ifdef INLINE_DEBUG_VUCOM
	debug << " CommandSize32=" << dec << lVifCommandSize;
#endif

						// there is more incoming data than just the tag
						lVifCodeState++;
					}

					// if processing next element, also check there is data to process
					if ( lWordsLeft )
					{
						while ( ( lWordsLeft > 0 ) && ( lVifCodeState < lVifCommandSize ) )
						{
							// get amount to transfer from this block
							lWordsToTransfer = lVifCommandSize - lVifCodeState;
							if ( lWordsToTransfer > lWordsLeft ) lWordsToTransfer = lWordsLeft;
							
							// ***TODO*** send graphics data to GIF
							// store ROW (R0-R3)
							// register offset starts at 16, buf lVifCodeState will start at 1
							//GPU::Path2_WriteBlock ( (u64*) Data, lWordsToTransfer >> 2 );
							//GPU::Path2_WriteBlock ( (u64*) Data, lWordsToTransfer >> 1 );
							GPU::Path2_WriteBlock ( Data, lWordsToTransfer );
							
							//Data += 4;
							Data += lWordsToTransfer;
							
							// update index
							lVifIdx += lWordsToTransfer;
							
							// move to next data input element
							//lVifCodeState++;
							//lVifCodeState += 4;
							lVifCodeState += lWordsToTransfer;
							
							// command size is 1+NUM*2 words
							//lWordsLeft--;
							//lWordsLeft -= 4;
							lWordsLeft -= lWordsToTransfer;
						}
						
						// for DIRECTHL command, NUM register is number of 128-bit QWs left
						VifRegs.NUM = ( ( lVifCommandSize - lVifCodeState ) >> 2 ) & 0xffff;
						
						if ( lVifCodeState >= lVifCommandSize )
						{
							// command is done
							lVifCodeState = 0;
							
							// also need to clear/flush the path2 pipeline
							GPU::_GPU->ulPath2_DataWaiting = 0;
						}
					}
					
				}
				
				break;
				
			default:
			
				if ( ( ( VifCode.Value >> 29 ) & 0x3 ) == 0x3 )
				{
#ifdef INLINE_DEBUG_VUCOM
	debug << " UNPACK";
#endif

/*
					if ( ! ( VifCode.IMMED & 0x8000 ) )
					{
						if ( Running )
						{
							// VU program is in progress //
							
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFSTOP)";
#endif

							VifStopped = 1;
							
#ifdef INLINE_DEBUG_VUCOM
	debug << " (DIRECT_PENDING)";
#endif

							// there is a pending DIRECT Command -> for now just back up index
							lVifIdx--;
							
							// just return for now //
							
							// get the number of full quadwords completely processed
							QWC_Transferred = lVifIdx >> 2;
							
							// get start index of remaining data to process in current block
							lVifIdx &= 0x3;
							
							// return the number of full quadwords transferred/processed
							return QWC_Transferred;
						}
					}
				
					// VU program is NOT in progress //
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (VIFCNT)";
#endif

					VifStopped = 0;
					
#ifdef INLINE_DEBUG_VUCOM
	debug << " (RUNUNPACK)";
#endif
*/

					// unpacks data into vu data memory
					// Vu0 and Vu1
					if ( !lVifCodeState )
					{
						// clear the the counter for unpack
						lVifUnpackIndex = 0;
						
						/*
						// set the command size
						// note: actually this is just getting the amount of data to read from FIFO
						if ( VifRegs.CYCLE.WL <= VifRegs.CYCLE.CL )
						{
							// WL <= CL
							ulTemp = VifCode.NUM;
							//ulTemp = ( ( ( 32 >> VifCode.vl ) * ( VifCode.vn + 1 ) ) * VifCode.NUM );
							
						}
						else
						{
							// WL > CL
							
							// NOTE: either this could cause a crash if WL is zero...
							ulTemp = VifCode.NUM % VifRegs.CYCLE.WL;
							ulTemp = ( ulTemp > VifRegs.CYCLE.CL ) ? VifRegs.CYCLE.CL : ulTemp;
							ulTemp = VifRegs.CYCLE.CL * ( VifCode.NUM / VifRegs.CYCLE.WL ) + ulTemp;
							//ulTemp = ( ( ( 32 >> VifCode.vl ) * ( VifCode.vn + 1 ) ) * ulTemp );
						}
						
						ulTemp = ( ( 32 >> VifCode.vl ) * ( VifCode.vn + 1 ) ) * ulTemp;
						lVifCommandSize = 1 + ( ulTemp >> 5 ) + ( ( ulTemp & 0x1f ) ? 1 : 0 );
						*/
						
						// looks like NUM=0 might be same as 256 ?? even for unpack ??
						lVifCommandSize = ( VifCode.NUM ? VifCode.NUM : 256 );
						
						// the actual amount of data to write in 32-bit words is NUM*4
						//lVifCommandSize = 1 + ( VifCode.NUM << 2 );
						lVifCommandSize = 1 + ( lVifCommandSize << 2 );
						
#ifdef INLINE_DEBUG_VUCOM
	debug << " CommandSize32=" << dec << lVifCommandSize;
#endif

#ifdef VERBOSE_UNPACK
						// alert for values of m, vn, and vl for now
						// m=0, vn=3, vl=0
						cout << "\nhps2x64: VU#" << Number << ": m=" << dec << VifCode.m << " vn=" << VifCode.vn << " vl=" << VifCode.vl << " CL=" << VifRegs.CYCLE.CL << " WL=" << VifRegs.CYCLE.WL << " FLG=" << ( ( VifCode.IMMED & 0x8000 ) >> 15 ) << " ADDR=" << hex << ( VifCode.IMMED & 0x3ff );
#endif

						// there is more incoming data than just the tag
						lVifCodeState++;
						
						
						// need to start read offset at zero, and update when reading from dma/memory
						ulReadOffset32 = 0;
						
						// get size of data elements in bytes
						ulUnpackItemWidth = 4 >> VifCode.vl;
						
						// get number of components to unpack
						ulUnpackNum = VifCode.vn + 1;
						ulUnpackNumIdx = 0;
						
						// get wl/cl
						ulWL = VifRegs.CYCLE.WL;
						ulCL = VifRegs.CYCLE.CL;
						ulWLCycle = 0;
						ulCLCycle = 0;

#ifdef VERBOSE_FILLING_WRITE
						// need to find and debug filling writes ??
						if ( ulCL < ulWL )
						{
							cout << "\nhps2x64: ALERT: VIF: UNPACK: Filling Write. CL=" << dec << ulCL << " WL=" << ulWL << "\n";
						}
#endif
						
						// initialize count of elements read
						ulReadCount = 0;

						// for UNPACK command, NUM register is set to NUM field
						VifRegs.NUM = VifCode.NUM;

						// check if FLG bit is set
						if ( VifCode.IMMED & 0x8000 )
						{
							// ***todo*** update to offset from current data being stored
							//DstPtr32 = & VuMem32 [ ( ( VifCode.IMMED & 0x3ff ) + ( VifRegs.TOPS & 0x3ff ) ) << 2 ];
							
							// where the data is getting stored to has to do with the write offset, which is the same as lVifCodeState-1
							//DstPtr32 += ( lVifCodeState - 1 );
							
							// need to keep track of where writing to in VU memory
							ulWriteOffset32 = ( ( ( VifCode.IMMED & 0x3ff ) + ( VifRegs.TOPS & 0x3ff ) ) << 2 ) + ( lVifCodeState - 1 );
						}
						else
						{
							// ***todo*** update to offset from current data being stored
							//DstPtr32 = & VuMem32 [ ( VifCode.IMMED & 0x3ff ) << 2 ];
							//DstPtr32 += ( lVifCodeState - 1 );
							
							// need to keep track of where writing to in VU memory
							ulWriteOffset32 = ( ( ( VifCode.IMMED & 0x3ff ) ) << 2 ) + ( lVifCodeState - 1 );
						}


						
#ifdef INLINE_DEBUG_VUCOM
	debug << hex << " (BASE=" << VifRegs.BASE << " OFFSET=" << VifRegs.OFST << " TOP=" << VifRegs.TOP << " TOPS=" << VifRegs.TOPS << ")";
#endif

#ifdef INLINE_DEBUG_VUEXECUTE
					Vu::Instruction::Execute::debug << "\r\n*** UNPACK";
					Vu::Instruction::Execute::debug << " VU#" << Number;
					Vu::Instruction::Execute::debug << " VifCode=" << hex << VifCode.Value;
					Vu::Instruction::Execute::debug << hex << " BASE=" << VifRegs.BASE << " OFFSET=" << VifRegs.OFST << " TOP=" << VifRegs.TOP << " TOPS=" << VifRegs.TOPS;
					Vu::Instruction::Execute::debug << hex << " m=" << VifCode.m << " vn=" << VifCode.vn << " vl=" << VifCode.vl << " CL=" << VifRegs.CYCLE.CL << " WL=" << VifRegs.CYCLE.WL << " FLG=" << ( ( VifCode.IMMED & 0x8000 ) >> 15 ) << " ADDR=" << hex << ( VifCode.IMMED & 0x3ff );
					Vu::Instruction::Execute::debug << " ***";
					// looks like sometimes the VifCode can be "unaligned"
					// it would be unaligned if it were not zero ??
					if ( lVifIdx & 3 )
					{
						Vu::Instruction::Execute::debug << "(UNALIGNED lVifIdx_Offset=" << (lVifIdx & 3) << ")";
					}
#endif

					}
					
					//if ( lWordsLeft )
					if ( lVifCodeState )
					{
						// do this for now
						// add tops register if FLG set
						
						// get dst pointer
						DstPtr32 = & VuMem32 [ ulWriteOffset32 ];
						
						//while ( lWordsLeft && ( lVifCodeState < lVifCommandSize ) )
						while ( lVifCodeState < lVifCommandSize )
						{
							// first determine where data should be read from //
							// WL must be within range before data is read
							if ( ulWLCycle < ulWL )
							{
								// WL is within range
								// so we must read data from someplace
								
								// get Write Cycle
								// *todo* unsure if the "Write Cycle" is reset after CL or WL or both ??
								ulWriteCycle = ulWLCycle;
								
								// get unpack row/col
								ulUnpackRow = ( ( ulWriteCycle < 4 ) ? ulWriteCycle : 3 );
								ulUnpackCol = ulWriteOffset32 & 3;
								
								// to read data from dma/memory, CL must be within range, and Col must be within correct range
								if ( ( ulCLCycle < ulCL ) && ( ulUnpackCol < ulUnpackNum ) )
								{
									// data source is dma/ram //
								
									// make sure that there is data left to read
									if ( lWordsLeft <= 0 )
									{
										// there is no data to read //
										
										// return sequence when lVifCodeState is NOT zero //
										break;
										
										/*
										// get the number of full quadwords completely processed
										QWC_Transferred = lVifIdx >> 2;
										
										// get start index of remaining data to process in current block
										lVifIdx &= 0x3;
										
										// return the number of full quadwords transferred/processed
										return QWC_Transferred;
										*/
									}
									
									// determine data element size
									if ( !ulUnpackItemWidth )
									{
										// must be unpacking pixels //
										// *todo* check for vn!=3 which would mean it is not unpacking pixels but rather an invalid command
										//cout << "\nhps2x64: ALERT: VU: UNPACK: Pixel unpack unsupported!!\n";
										
										// similar to unpacking 16-bit halfwords //
										
										if ( ! ( ulReadCount & 7 ) )
										{
											ulUnpackBuf = *Data++;
										
											// get the correct element of 32-bit word
											//lUnpackTemp >>= ( ( ulReadCount & 4 ) << 2 );
											//lUnpackTemp &= 0xffff;
										}
										
										
										// pull correct component
										if ( ulUnpackCol < 3 )
										{
											// RGB
											lUnpackLastData = ulUnpackBuf & 0x1f;
											lUnpackLastData <<= 3;
											
											ulUnpackBuf >>= 5;
										}
										else
										{
											// A
											lUnpackLastData = ulUnpackBuf & 1;
											lUnpackLastData <<= 7;
											
											ulUnpackBuf >>= 1;
										}
										
										ulReadCount++;
										
										// one pixel gets unpacked per 8 components RGBA
										if ( ! ( ulReadCount & 7 ) )
										{
											//Data++;
											lVifIdx++;
											
											// this is counting the words left in the input
											lWordsLeft--;
											
#ifdef INLINE_DEBUG_UNPACK
	debug << " VifIdx=" << dec << lVifIdx << " WordsLeft=" << lWordsLeft;
#endif
										}
									}
									else
									{
										// unpacking NON-pixels //
										
										// check if only 1 element (vn=0) and not first col
										if ( VifCode.vn || !ulUnpackCol )
										{
											if ( ! ( ulReadCount & ( 7 >> ulUnpackItemWidth ) ) )
											{
												ulUnpackBuf = *Data++;
											
												// get the correct element of 32-bit word
												//lUnpackTemp >>= ( ( ulReadCount & 4 ) << 2 );
												//lUnpackTemp &= 0xffff;
											}
											
											
											lUnpackLastData = ulUnpackBuf & ( 0xffffffffUL >> ( ( 4 - ulUnpackItemWidth ) << 3 ) );
											
											ulUnpackBuf >>= ( ulUnpackItemWidth << 3 );
											
											// check if data item is signed and 1 or 2 bytes long
											if ( ( !VifCode.USN ) && ( ( ulUnpackItemWidth == 1 ) || ( ulUnpackItemWidth == 2 ) ) )
											{
												// signed data //
												lUnpackLastData <<= ( ( 4 - ulUnpackItemWidth ) << 3 );
												lUnpackLastData >>= ( ( 4 - ulUnpackItemWidth ) << 3 );
											}
											
											ulReadCount++;
											
											if ( ! ( ulReadCount & ( 7 >> ulUnpackItemWidth ) ) )
											{
												//Data++;
												lVifIdx++;
												
												// command size is 1+NUM*2 words
												// this is counting the words left in the input, but should ALSO be counting words left in output
												lWordsLeft--;
												
#ifdef INLINE_DEBUG_UNPACK
	debug << " VifIdx=" << dec << lVifIdx << " WordsLeft=" << lWordsLeft;
#endif
											}
											
										} // end if ( ulUnpackNum || !ulUnpackCol )
											
									} // end if ( !ulUnpackItemWidth )
									
									
									
								} // end if ( ulCLCycle < ulCL )
								
								
								// data source is determined by m and MASK //
								
								// if m bit is 0, then mask is zero
								ulUnpackMask = 0;
								
								if ( VifCode.m )
								{
									// mask comes from MASK register //
									
									
									// mask index is (Row*4)+Col
									// but if the row (write cycle??) is 3 or 4 or greater then just use maximum row value
									ulUnpackMask = ( VifRegs.MASK >> ( ( ( ulUnpackRow << 2 ) + ulUnpackCol ) << 1 ) ) & 3;
									
#ifdef INLINE_DEBUG_UNPACK_2
	debug << "\r\nRow#" << ulUnpackRow << " Col#" << ulUnpackCol << " TO " << hex << ulWriteOffset32 << " FullMsk=" << hex << VifRegs.MASK << " Shift=" << dec << ( ( ( ulUnpackRow << 2 ) + ulUnpackCol ) << 1 );
#endif
								}
								
								// determine source of data
								switch ( ulUnpackMask )
								{
									case 0:
										// data comes from dma/ram //
										lUnpackData = lUnpackLastData;
										break;
										
									case 1:
										// data comes from ROW register //
										lUnpackData = VifRegs.Regs [ c_iRowRegStartIdx + ulUnpackCol ];
										break;
										
									case 2:
										// data comes from COL register //
										lUnpackData = VifRegs.Regs [ c_iColRegStartIdx + ulUnpackRow ];
										break;
										
									case 3:
										// write is masked/not happening/prevented //
										break;
								}
								
								// for adding/totalling with row register, you need either m=0 or m[x]=0
								if ( !VifCode.m || !ulUnpackMask )
								{
									// check if data should be added or totaled with ROW register
									if ( VifRegs.MODE & 3 )
									{
										// 1 means add, 2 means total
										
										switch ( VifRegs.MODE & 3 )
										{
											case 1:
#ifdef INLINE_DEBUG_UNPACK_2
	debug << " ADD";
#endif
											
												// add to unpacked data //
												
												// I guess the add should be an integer one?
#ifdef ENABLE_FLOAT_ADD
												fUnpackData += VifRegs.fRegs [ c_iRowRegStartIdx + ulUnpackCol ];
#else
												lUnpackData += VifRegs.Regs [ c_iRowRegStartIdx + ulUnpackCol ];
#endif
												break;
												
											case 2:
#ifdef INLINE_DEBUG_UNPACK_2
	debug << " TOTAL";
#endif

												// add to unpacked data and write result back to ROW register //
												
												// I guess the add/total should be an integer one?
#ifdef ENABLE_FLOAT_TOTAL
												fUnpackData += VifRegs.fRegs [ c_iRowRegStartIdx + ulUnpackCol ];
												VifRegs.fRegs [ c_iRowRegStartIdx + ulUnpackCol ] = fUnpackData;
#else
												lUnpackData += VifRegs.Regs [ c_iRowRegStartIdx + ulUnpackCol ];
												VifRegs.Regs [ c_iRowRegStartIdx + ulUnpackCol ] = lUnpackData;
#endif
												break;
												
											
											// undocumented ??
											case 3:
#ifdef INLINE_DEBUG_UNPACK_2
	debug << " UNDOCUMENTED";
#endif

												VifRegs.Regs [ c_iRowRegStartIdx + ulUnpackCol ] = lUnpackData;
												break;
												
											default:
#ifdef INLINE_DEBUG_UNPACK
	debug << " INVALID";
#endif

												cout << "\nhps2x64: ALERT: VU: UNPACK: Invalid MODE setting:" << dec << VifRegs.MODE << "\n";
												break;
										}
									}
								} // end if ( !VifCode.m || !ulUnpackMask )
								
								
#ifdef DISABLE_UNPACK_INDETERMINATE
								// only write data if the column is within the amount being written, or it is masked
								if ( ( ulUnpackNum == 1 ) || ( ulUnpackMask ) || ( ulUnpackCol < ulUnpackNum ) )
								{
#endif
								
								// write data to vu memory if write is not masked //
								if ( ulUnpackMask != 3 )
								{
#ifdef INLINE_DEBUG_UNPACK_2
	debug << "\r\nCol#" << ulUnpackCol << " Data=" << hex << lUnpackData << " TO " << ulWriteOffset32 << " Msk=" << ulUnpackMask;
#endif

									*DstPtr32 = lUnpackData;
									
#ifdef INLINE_DEBUG_UNPACK_3
	debug << " VuMem=" << hex << VuMem32 [ ulWriteOffset32 ];
#endif
								}
#ifdef INLINE_DEBUG_UNPACK_2
								else
								{
	debug << "\r\nCol#" << ulUnpackCol << " MSKD TO " << hex << ulWriteOffset32 << " Msk=" << ulUnpackMask;
								}
#endif

#ifdef DISABLE_UNPACK_INDETERMINATE
								} // end if ( ( ulUnpackNum == 1 ) || ( ulUnpackCol < ulUnpackNum ) )
#ifdef INLINE_DEBUG_UNPACK_2
								else
								{
	debug << "\r\nCol#" << ulUnpackCol << " UNDT TO " << hex << ulWriteOffset32;
								}
#endif
								
#endif
								
								// move to next data input element
								// this should update per 32-bit word that is written into VU memory
								// WL is a write cycle
								lVifCodeState++;
								
							} // end if ( ulWLCycle < ulWL )
						
						
							// store ROW (R0-R3)
							// register offset starts at 16, buf lVifCodeState will start at 1
							//*DstPtr32++ = *Data++;
							
							// update index
							//lVifIdx++;
							
							// update write offset
							ulWriteOffset32++;
							
							// when write offset is updated, must also update ptr
							DstPtr32++;
							
							// check if going to the next row
							//if ( ( ( lVifCodeState - 1 ) & 3 ) == 3 )
							if ( !( ulWriteOffset32 & 3 ) )
							{
								// if just completed a write cycle, then decrement NUM register
								if ( ulWLCycle < ulWL )
								{
									// NUM register counts number 128-bit QWs written
									VifRegs.NUM--;
									VifRegs.NUM &= 0xff;
								}
								
								// update WL,CL
								ulWLCycle++;
								ulCLCycle++;
							}
							
							// reset both if they are both outside of range
							if ( ( ulWLCycle >= ulWL ) && ( ulCLCycle >= ulCL ) )
							{
								ulWLCycle = 0;
								ulCLCycle = 0;
							}
							
							
							
#ifdef INLINE_DEBUG_UNPACK
	debug << " State=" << dec << lVifCodeState;
#endif
						}
						
						if ( lVifCodeState >= lVifCommandSize )
						{
							// command is done
							lVifCodeState = 0;
							
							// it is possible that unpacking is done but that there was padding left in the word
							// in that case need to skip the padding and go to the next word
							if ( !ulUnpackItemWidth )
							{
								if ( ulReadCount & 7 )
								{
									//Data++;
									lVifIdx++;
									
									// this is counting the words left in the input
									lWordsLeft--;
								}
							}
							else
							{
								 if ( ulReadCount & ( 7 >> ulUnpackItemWidth ) )
								 {
									//Data++;
									lVifIdx++;
									
									// this is counting the words left in the input
									lWordsLeft--;
								 }
							}

						}
					
					}

					break;
				}
				
#ifdef INLINE_DEBUG_VUCOM
	debug << " INVALID";
#endif
#ifdef INLINE_DEBUG_INVALID
	debug << "\r\nVU#" << Number << " Error. Invalid VIF Code=" << hex << VifCode.Value << " Cycle#" << dec << *_DebugCycleCount;
#endif

				cout << "\nhps2x64: VU#" << Number << " Error. Invalid VIF Code=" << hex << VifCode.Value;

				break;

		}
	}
	
	// get the number of full quadwords completely processed
	QWC_Transferred = lVifIdx >> 2;
	
	// get start index of remaining data to process in current block
	lVifIdx &= 0x3;
	
	// if vif code state is zero, then check if the previous code had an interrupt, and trigger if so
	if ( !lVifCodeState )
	{
		// TESTING
		// command is done so set stat to vif idle?
		VifRegs.STAT.VPS = 0x0;
		
		// check for interrupt
		if ( VifCode.INT )
		{
#ifdef INLINE_DEBUG_VUINT
			debug << " ***INT***";
#endif
#ifdef VERBOSE_VUINT
			cout << "\n***VUINT***\n";
#endif
			
			// ***todo*** need to send interrupt signal and stop transfer to Vif //
			SetInterrupt_VIF ();
			
			// set STAT.INT
			VifRegs.STAT.INT = 1;
			
			// start reading from index offset (may have left off in middle of data QW block)
			Data = & ( Data [ lVifIdx & 0x3 ] );
			
			// check if next is MARK (don't stall on Vif Code MARK) or if there is no more data
			// if the next Vif Code is mark, then go ahead and execute it for now
			// note: it is possible this could be missed if more data is needed to be loaded ***NEEDS FIXING***
			if ( lWordsLeft )
			{
				if ( ( ( Data [ 0 ] >> 24 ) & 0x7f ) == 0x7 )
				{
					// VIF Code MARK after a vif code with I-bit set //
					
					// load vif code
					//VifCode.Value = Data [ lIndex++ ];
					VifCode.Value = *Data++;
					
					// store to CODE register
					VifRegs.CODE = VifCode.Value;
					
					// update index
					lVifIdx++;
					
					// vif code is 1 word
					lWordsLeft--;
					
					// MARK //
					
					// debug
					cout << "\nhps2x64 ALERT: VU: Mark encountered!!!\n";
					
					// set mark register
					// set from IMM
					// Vu0 and Vu1
					VifRegs.MARK = VifCode.IMMED;
					
					// also need to set MRK bit in STAT
					// this gets cleared when CPU writes to MARK register
					VifRegs.STAT.MRK = 1;
				}
			}
			
			
			// ***todo*** stop vif transfer
#ifdef VERBOSE_VUINT
			cout << "\nhps2x64: VU: stopping VIF transfer due to interrupt bit set!!!\n";
#endif

			VifStopped = 1;
			
			// set STAT.VIS due to interrupt stall ??
			VifRegs.STAT.VIS = 1;
		}
	}
	
	// return the number of full quadwords transferred/processed
	return QWC_Transferred;
}






void VU::DMA_Read ( u32* Data, int ByteReadCount )
{
#ifdef INLINE_DEBUG_DMA_READ
	debug << "\r\n\r\nDMA_Read " << hex << setw ( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Data = " << hex << Data [ 0 ];
#endif

}




void VU::DMA_Write ( u32* Data, int ByteWriteCount )
{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "\r\n\r\nDMA_Write " << hex << setw ( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Data = " << hex << Data [ 0 ];
#endif

}


u32 VU::DMA_WriteBlock ( u64* Data, u32 QuadwordCount )
{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "\r\n\r\nDMA_WriteBlock " << hex << setw ( 8 ) << *_DebugPC << " QWC=" << QuadwordCount << " " << dec << *_DebugCycleCount << hex << "; Data= ";
	debug << "; VU#" << Number;
	for ( int i = 0; i < ( QuadwordCount * 2 ); i++ ) debug << " " << Data [ i ];
	debug << "\r\n";
#endif

	u32 QWC_Transferred;
	
	// check if QWC is zero
	if ( !QuadwordCount )
	{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << " QWC_IS_ZERO";
#endif

		return 0;
	}
	
	
	// send data to VIF FIFO
	QWC_Transferred = VIF_FIFO_Execute ( (u32*) Data, QuadwordCount << 2 );
	
	if ( QWC_Transferred == QuadwordCount )
	{
		// transfer complete //
		lVifIdx = 0;
	}
	
	// return the amount of quadwords processed
	return QWC_Transferred;

	/*
	for ( int i = 0; i < QuadwordCount; i++ )
	{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "\r\n";
	debug << hex << Data [ 0 ] << " " << Data [ 1 ];
#endif

		VIF_FIFO_Execute ( Data [ 0 ], Data [ 1 ] );
		
		Data += 2;
	}
	*/
	
//#ifdef INLINE_DEBUG_DMA_WRITE
//	debug << " VIF#" << Number << " INT";
//#endif
	
	// ***testing*** interrupt??
	//SetInterrupt_VIF ();
}


u32 VU::DMA_ReadBlock ( u64* Data, u32 QuadwordCount )
{
#ifdef INLINE_DEBUG_DMA_READ
	debug << "\r\n\r\nDMA_ReadBlock " << hex << setw ( 8 ) << *_DebugPC << " QWC=" << QuadwordCount << " " << dec << *_DebugCycleCount << hex << "; Data= ";
	debug << "; VU#" << Number;
	debug << "\r\n";
#endif

	// read data from gpu
	GPU::Path2_ReadBlock ( Data, QuadwordCount );
	
	return QuadwordCount;
}


bool VU::DMA_Read_Ready ()
{
	// do this for now
	// ***todo*** check if read from VU1 is ready
	return !VifStopped;
}

bool VU::DMA_Write_Ready ()
{
	// if the VIF has been stopped, or if there is a packet in progress in path 3, then do not run dma1??
	//if ( ( VifStopped ) || ( GPU::_GPU->ulTransferCount [ 3 ] ) )
	if ( ( VifStopped ) )
	{
		return 0;
	}
	
	if ( bTransferringDirectViaPath2 && !GPU::_GPU->EndOfPacket [ 3 ] )
	{
		return 0;
	}
	
	// next, make sure this is not vu#0
	/*
	if ( Number )
	{
		// if path 3 packet transfer is in progress and IMT is set to transfer in continuous mode
		if ( ( GPU::_GPU->ulTransferCount [ 3 ] ) && ( !GPU::_GPU->GIFRegs.MODE.IMT ) )
		{
			return 0;
		}
	}
	*/

	return 1;
}



void VU::Run ()
{
	u32 Index;
	

	// making these part of object rather than local function
	//Instruction::Format CurInstLO;
	//Instruction::Format CurInstHI;
	

	/////////////////////////////////
	// VU components:
	// 1. Instruction Execute Unit
	// 2. Delay Slot Unit
	// 3. Multiply/Divide Unit
	/////////////////////////////////
	
	
	// if VU is not running, update vu cycle and then return
	if ( !Running )
	{
		//CycleCount = *_DebugCycleCount;
		CycleCount = -1LL;
		return;
	}
	

#ifdef INLINE_DEBUG
	debug << "\r\n->PC = " << hex << setw( 8 ) << PC << dec;
	debug << " VU#" << dec << Number;
#endif


	//cout << "\nVU -> running=" << Running;
	
	
	/////////////////////////
	// Execute Instruction //
	/////////////////////////
	
	
	// execute instruction
	//NextPC = PC + 4;
	
#ifdef INLINE_DEBUG
	debug << ";Execute";
#endif

	///////////////////////////////////////////////////////////////////////////////////////////
	// R0 is always zero - must be cleared before any instruction is executed, not after
	//GPR [ 0 ].u = 0;
	vi [ 0 ].u = 0;
	
	// f0 always 0, 0, 0, 1 ???
	vf [ 0 ].uLo = 0;
	vf [ 0 ].uHi = 0;
	vf [ 0 ].uw = 0x3f800000;

	// update instruction
	NextPC = PC + 8;

#ifdef ENABLE_RECOMPILER_VU
	if ( !bEnableRecompiler )
	{
#endif
	
	//cout << "\nVU -> load lo";
	
	// load LO instruction
	//CurInstLO.Value = MicroMem32 [ PC >> 2 ];
	
	//cout << "\nVU -> load hi";
	
	// load HI instruction
	//CurInstHI.Value = MicroMem32 [ ( PC + 4 ) >> 2 ];
	
	CurInstLOHI.ValueLoHi = MicroMem64 [ PC >> 3 ];
	
	//cout << "\nVU -> execute lo";
	
	// check if E-bit is set (means end of execution after E-bit delay slot)
	//if ( CurInstHI.E )
	if ( CurInstLOHI.E )
	{
#ifdef INLINE_DEBUG
	debug << "; ***E-BIT SET***";
#endif

		Status.EBitDelaySlot_Valid |= 0x2;
	}
	
	// alert if d or t is set
	//if ( CurInstHI.D )
	if ( CurInstLOHI.D )
	{
		// register #28 is the FBRST register
		// the de bit says if the d-bit is enabled or not
		// de0 register looks to be bit 2
		// de1 register looks to be bit 10
		if ( !Number )
		{
			// check de0
			if ( vi [ 28 ].u & ( 1 << 2 ) )
			{
				cout << "\nhps2x64: ALERT: VU#" << Number << " D-bit is set!\n";
			}
		}
		else
		{
			// check de1
			if ( vi [ 28 ].u & ( 1 << 10 ) )
			{
				cout << "\nhps2x64: ALERT: VU#" << Number << " D-bit is set!\n";
			}
		}
	}
	
	//if ( CurInstHI.T )
	if ( CurInstLOHI.T )
	{
		cout << "\nhps2x64: ALERT: VU#" << Number << " T-bit is set!\n";
	}
	
	
	// execute HI instruction first ??
	
	// check if Immediate or End of execution bit is set
	//if ( CurInstHI.I )
	if ( CurInstLOHI.I )
	{
		// lower instruction contains an immediate value //
		
		// *important* MUST execute the HI instruction BEFORE storing the immediate
		//Instruction::Execute::ExecuteInstructionHI ( this, CurInstHI );
		Instruction::Execute::ExecuteInstructionHI ( this, CurInstLOHI.Hi );
		
		// load immediate regiser with LO instruction
		//vi [ 21 ].u = CurInstLO.Value;
		vi [ 21 ].u = CurInstLOHI.Lo.Value;
	}
	else
	{
		// execute lo/hi instruction normally //
		// unsure of order
		
		// set the lo instruction
		//CurInstLO.Value = CurInstLOHI.Lo.Value;
		
		// execute LO instruction since it is an instruction rather than an immediate value
		//Instruction::Execute::ExecuteInstructionLO ( this, CurInstLO );
		Instruction::Execute::ExecuteInstructionLO ( this, CurInstLOHI.Lo );
		
		// execute HI instruction
		//Instruction::Execute::ExecuteInstructionHI ( this, CurInstHI );
		Instruction::Execute::ExecuteInstructionHI ( this, CurInstLOHI.Hi );
		
		// needs to be cleared to zero when done with it, since it is just to let hi instruction know what is going on
		//CurInstLO.Value = 0;
	}
	
	//cout << "\nVU -> execute hi";
	
	
	//cout << "\nVU -> update pc";	
	
	// update instruction
	//NextPC = PC + 8;


#ifdef ENABLE_RECOMPILER_VU
	}
	else
	{
// no need to interpret while single-stepping during testing
#ifdef ALLOW_RECOMPILE_INTERPRETER
		if ( Status.Value )
		{
#ifdef INLINE_DEBUG
	debug << ";Interpret";
	debug << " Status=" << hex << Status.Value;
#endif

			// load the instruction
			CurInstLOHI.ValueLoHi = MicroMem64 [ PC >> 3 ];
			

			// check if E-bit is set (means end of execution after E-bit delay slot)
			//if ( CurInstHI.E )
			if ( CurInstLOHI.E )
			{
#ifdef INLINE_DEBUG
			debug << "; ***E-BIT SET***";
#endif

				Status.EBitDelaySlot_Valid |= 0x2;
			}
			

			if ( CurInstLOHI.I )
			{
				// lower instruction contains an immediate value //
				
				// *important* MUST execute the HI instruction BEFORE storing the immediate
				//Instruction::Execute::ExecuteInstructionHI ( this, CurInstHI );
				Instruction::Execute::ExecuteInstructionHI ( this, CurInstLOHI.Hi );
				
				// load immediate regiser with LO instruction
				//vi [ 21 ].u = CurInstLO.Value;
				vi [ 21 ].u = CurInstLOHI.Lo.Value;
			}
			else
			{
				// execute lo/hi instruction normally //
				// unsure of order
				
				// set the lo instruction
				//CurInstLO.Value = CurInstLOHI.Lo.Value;
				
				// execute LO instruction since it is an instruction rather than an immediate value
				//Instruction::Execute::ExecuteInstructionLO ( this, CurInstLO );
				Instruction::Execute::ExecuteInstructionLO ( this, CurInstLOHI.Lo );
				
				// execute HI instruction
				//Instruction::Execute::ExecuteInstructionHI ( this, CurInstHI );
				Instruction::Execute::ExecuteInstructionHI ( this, CurInstLOHI.Hi );
				
				// needs to be cleared to zero when done with it, since it is just to let hi instruction know what is going on
				//CurInstLO.Value = 0;
			}
		}
		else
#endif
		{
		
		// check that address block is encoded
		//if ( ! vrs [ Number ]->isRecompiled ( PC ) )
		if ( bCodeModified [ Number ] )
		{
#ifdef INLINE_DEBUG
	debug << ";NOT Recompiled";
#endif
			// address is NOT encoded //
			
			// recompile block
			vrs [ Number ]->Recompile ( this, PC );
			
			// code has been recompiled so only need to recompile again if modified
			bCodeModified [ Number ] = 0;
		}
		
#ifdef INLINE_DEBUG
	debug << ";Recompiled";
	debug << ";PC=" << hex << PC;
	debug << " Status=" << hex << Status.Value;
#endif

		// clear branches
		//Recompiler::Status_BranchDelay = 0;
		Recompiler_EnableFlags = 0;
		
		// get the block index
		Index = vrs [ Number ]->Get_Index ( PC );

#ifdef INLINE_DEBUG
	debug << ";Index(dec)=" << dec << Index;
	debug << ";Index(hex)=" << hex << Index;
#endif
		
		// offset cycles before the run, so that it updates to the correct value
		//CycleCount -= rs->CycleCount [ Index ];

		// already checked that is was either in cache, or etc
		// execute from address
		( (func2) (vrs [ Number ]->pCodeStart [ Index ]) ) ();
		
#ifdef INLINE_DEBUG
	debug << "\r\n->RecompilerReturned";
	debug << " VU#" << dec << Number;
#endif

		} // end if ( Status.Value )
			
	}
#endif


	
	///////////////////////////////////////////////////
	// Check if there is anything else going on
	
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// Check delay slots
	// need to do this before executing instruction because load delay slot might stall pipeline

	// check if anything is going on with the delay slots
	// *note* by doing this before execution of instruction, pipeline can be stalled as needed
	if ( Status.Value )
	{
		// clear status/clip set flags
		Status.SetStatus_Flag = 0;
		Status.SetClip_Flag = 0;

		/////////////////////////////////////////////
		// check for anything in delay slots

#ifdef ENABLE_INTDELAYSLOT
		if ( Status.IntDelayValid )
		{
			Status.IntDelayValid >>= 1;
			
			if ( !Status.IntDelayValid )
			{
				vi [ IntDelayReg ].u = IntDelayValue;
			}
		}
#endif
		

#ifdef ENABLE_STALLS
		// for load/move delay slot (must only write after execution of upper instruction unless writing to integer register
		if ( Status.EnableLoadMoveDelaySlot )
		{
			// this clears the EnableLoadMoveDelaySlot, so no need to do it here
			//Instruction::Execute::Execute_LoadDelaySlot ( this, CurInstLO );
			Instruction::Execute::Execute_LoadDelaySlot ( this, CurInstLOHI.Lo );
		}
#endif

		
#ifdef INLINE_DEBUG
		debug << ";DelaySlotValid";
		debug << "; Status=" << hex << Status.Value;
#endif
		
		//if ( Status.DelaySlot_Valid & 1 )
		if ( Status.DelaySlot_Valid )
		{
#ifdef INLINE_DEBUG
			debug << ";Delay1.Value";
#endif
			
			Status.DelaySlot_Valid >>= 1;
			
			if ( !Status.DelaySlot_Valid )
			{
				ProcessBranchDelaySlot ();
			}
			
			///////////////////////////////////
			// move delay slot
			NextDelaySlotIndex ^= 1;
		}
		

		
		if ( Status.XgKickDelay_Valid )
		{
			Status.XgKickDelay_Valid >>= 1;
			if ( !Status.XgKickDelay_Valid )
			{
				Execute_XgKick ();
			}
		}
		
		// check for end of execution for VU
		//if ( Status.EBitDelaySlot_Valid & 1 )
		if ( Status.EBitDelaySlot_Valid )
		{
#ifdef INLINE_DEBUG_VURUN
			debug << "\r\nEBitDelaySlot_Valid; VU#" << dec << Number;
			debug << " @Cycle#" << dec << *_DebugCycleCount;
			debug << " EBitDelaySlot_Valid=" << hex << (u32) Status.EBitDelaySlot_Valid;
#endif

			Status.EBitDelaySlot_Valid >>= 1;

			if ( !Status.EBitDelaySlot_Valid )
			{
#ifdef INLINE_DEBUG_VURUN
			debug << "\r\nEBitDelaySlot; VU#" << dec << Number << " DONE";
			debug << " @Cycle#" << dec << *_DebugCycleCount;
#endif

				Running = 0;
				CycleCount = -1LL;
				
				// for VU#1, clear pipelines
				//if ( Number )
				//{
				//	Pipeline_Bitmap = 0;
				//	Int_Pipeline_Bitmap = 0;
				//}
				
				
				// set VBSx in VPU STAT to be zero (idle)
				VU0::_VU0->vi [ 29 ].uLo &= ~( 1 << ( Number << 3 ) );
				
				// also clear VIFx STAT bit to indicate program is NOT running
				VifRegs.STAT.VEW = 0;
				
				// if Vif is stopped for some reason, it can resume transfer now
				VifStopped = 0;

				
				// problem: when running dma before events for the current cycle are executed, could skip cycle
				// events are running simultaneously, so this is ok
				// backup cycle - to prevent skips for now
				*_DebugCycleCount = *_DebugCycleCount - 1;
				

				// ***todo*** also need to notify DMA to restart if needed
				Dma::_DMA->Transfer ( Number );

				// problem: when running dma before events for the current cycle are executed, could skip cycle
				// restore cycle
				*_DebugCycleCount = *_DebugCycleCount + 1;

#ifdef INLINE_DEBUG_VURUN
			debug << "->DMA Should be restarted";
#endif

			}
		}

		//cout << hex << "\n" << DelaySlot1.Value << " " << DelaySlot1.Value2 << " " << DelaySlot0.Value << " " << DelaySlot0.Value2;
		
		///////////////////////////////////////////////////
		// Advance status bits for checking delay slot
		//Status.DelaySlot_Valid = ( Status.DelaySlot_Valid << 1 ) & 0x3;
		// ***todo*** could possibly combine these shifts into one
		
		//////////////////////////////////////////////
		// check for Asynchronous Interrupts
		// also make sure interrupts are enabled
		//if ( Status.CheckInterrupt )
		//{
		//}
	}


	/////////////////////////////////////
	// Update Program Counter
	LastPC = PC;
	PC = NextPC;

#ifdef ENABLE_STALLS

	// advance the vu cyclecount/pipeline, etc
	AdvanceCycle ();
#else
	// this counts the bus cycles, not R5900 cycles
	CycleCount++;

	// update q and p registers here for now
	UpdateQ ();
	UpdateP ();

	// update the flags here for now
	UpdateFlags ();
#endif

}



void VU::ProcessBranchDelaySlot ()
{
	Vu::Instruction::Format i;
	u32 Address;
	
	DelaySlot *d = & ( DelaySlots [ NextDelaySlotIndex ] );
	
	//i = DelaySlot1.Instruction;
	i = d->Instruction;
	
	
	switch ( i.Opcode )
	{
		// B
		case 0x20:
			
		// BAL
		case 0x21:
	
		// IBEQ
		case 0x28:
		
		// IBGEZ
		case 0x2f:
		
		// IBGTZ
		case 0x2d:
		
		// IBLEZ
		case 0x2e:
		
		// IBLTZ
		case 0x2c:
		
		// IBNE
		case 0x29:
		
			NextPC = PC + ( i.Imm11 << 3 );
			
			// make sure that address is not outside range (no memory management on VU)
			NextPC &= ulVuMem_Mask;
			
			break;
			
			
		// JALR
		case 0x25:
		
		// JR
		case 0x24:
		
			// it must be multiplying the register address by 8 before jumping to it
			//NextPC = d->Data;
			NextPC = d->Data << 3;
			
			// make sure that address is not outside range (no memory management on VU)
			NextPC &= ulVuMem_Mask;
			
			break;
	}
	
}



u32* VU::GetMemPtr ( u32 Address32 )
{
	u32 Reg;
	
#ifdef INLINE_DEBUG_GETMEMPTR
	debug << "; Address32=" << hex << Address32;
	debug << "; Number=" << Number;
#endif

	// check if this is VU#0
	if ( !Number )
	{
		// VU#0 //
		
		// the upper part of address should probably be 0x4000 exactly to access vu1 regs
#ifdef ALL_VU0_UPPER_ADDRS_ACCESS_VU1
		if ( Address32 & ( 0x4000 >> 2 ) )
#else
		if ( ( Address32 & ( 0xf000 >> 2 ) ) == ( 0x4000 >> 2 ) )
#endif
		{
			// get the register number
			Reg = ( Address32 >> 2 ) & 0x1f;
			
			// check if these are float/int/etc registers being accessed
			// note: makes more sense to shift over one more and check
			// unknown what addresses might be mirrored here
			//switch ( ( Address32 >> 6 ) & 0xf )
			switch ( ( Address32 >> 7 ) & 0x1f )
			{
				// float
				case 0:
				//case 1:
					return & VU1::_VU1->vf [ Reg ].uw0;
					break;
					
				// int/control
				//case 2:
				//case 3:
				case 1:
					return & VU1::_VU1->vi [ Reg ].u;
					break;
					
				default:
#ifdef INLINE_DEBUG_GETMEMPTR_INVALID
	debug << "\r\nERROR: VU0: referencing VU1 reg outside of range. Address=" << hex << ( Address32 << 2 );
#endif

					cout << "\nhps2x64: ERROR: VU0: referencing VU1 reg outside of range. Address=" << hex << ( Address32 << 2 );
					break;
			}
		}
		
		return & ( VuMem32 [ Address32 & ( c_ulVuMem0_Mask >> 2 ) ] );
	}
	
	return & ( VuMem32 [ Address32 & ( c_ulVuMem1_Mask >> 2 ) ] );
}



void VU::PipelineWait_FMAC ()
{
	// FMAC pipeline isn't more than 4 cycles long, so this guards against an infinite loop
	static const u32 c_CycleTimeout = 3;
	
	u32 Count;
	
	for ( Count = 0; Count < c_CycleTimeout; Count++ )
	{
		// advance to next pipeline stage/next cycle
		AdvanceCycle ();
		
		// check if pipeline is still stalled
		if ( !TestStall () )
		{
			// the registers that were needed are now ready for use
			return;
		}
		
	}
	
	// time out, which should never happen theoretically!!!
	cout << "\nhps2x64: VU" << dec << Number << ": SERIOUS ERROR: FMAC Pipeline wait timeout!!! Should never happen!\n";
	
#ifdef INLINE_DEBUG_PIPELINE
	debug << "\r\nhps2x64: VU#" << dec << Number << ": SERIOUS ERROR: FMAC Pipeline wait timeout!!! Should never happen! P0=" << hex << Pipeline_Bitmap.b0 << " P1=" << Pipeline_Bitmap.b1 << "\r\n";
#endif
}

void VU::PipelineWait_INT ()
{
	// Integer pipeline isn't more than 4 cycles long, so this guards against an infinite loop
	static const u32 c_CycleTimeout = 3;
	
	u32 Count;
	
	for ( Count = 0; Count < c_CycleTimeout; Count++ )
	{
		// advance to next pipeline stage/next cycle
		AdvanceCycle ();
		
		// check if pipeline is still stalled
		if ( !TestStall_INT () )
		{
			// the registers that were needed are now ready for use
			return;
		}
		
	}
	
	// time out, which should never happen theoretically!!!
	cout << "\nhps2x64: VU: SERIOUS ERROR: INT Pipeline wait timeout!!! Should never happen!\n";
	
#ifdef INLINE_DEBUG_PIPELINE
	debug << "\r\nhps2x64: VU: SERIOUS ERROR: INT Pipeline wait timeout!!! Should never happen!\r\n";
#endif
}


/*
// force pipeline to wait for 1 register to be ready before executing Upper instruction
void VU::PipelineWaitFMAC1 ( u32 UpperInstruction, u32 Register0 )
{
	u32 xyzw32;
	Bitmap128 WaitForBitmap;
	
	// get the xyzw
	xyzw32 = ( UpperInstruction >> 21 ) & 0xf;
	
	// create bitmap
	CreateBitmap ( WaitForBitmap, xyzw32, Register0 );
	
	PipelineWaitFMAC ( WaitForBitmap );
}

// force pipeline to wait for 2 registers to be ready before executing Upper instruction
void VU::PipelineWaitFMAC2 ( u32 UpperInstruction, u32 Register0, u32 Register1 )
{
	u32 xyzw32;
	Bitmap128 WaitForBitmap;
	
	// get the xyzw
	xyzw32 = ( UpperInstruction >> 21 ) & 0xf;
	
	// create bitmap
	CreateBitmap ( WaitForBitmap, xyzw32, Register0 );
	AddBitmap ( WaitForBitmap, xyzw32, Register1 );
	
	PipelineWaitFMAC ( WaitForBitmap );
}
*/


void VU::PipelineWaitCycle ( u64 WaitUntil_Cycle )
{
	// FMAC pipeline isn't more than 4 cycles long, so this guards against an infinite loop
	static const u32 c_CycleTimeout = 3;
	
	u32 Count;
	u64 Diff;
	
	Diff = WaitUntil_Cycle - CycleCount;
	
	if ( Diff > c_CycleTimeout )
	{
		Diff = c_CycleTimeout;
	}
	
	
	/*
	// check if the P register was set in the meantime too
	if ( CycleCount >= ( PBusyUntil_Cycle - 1 ) )
	{
		SetP ();
	}
	
	// first check that the current cycle is not past the busy until cycle
	if ( CycleCount >= WaitUntil_Cycle )
	{
		// no need to wait
		return;
	}
	*/
	
	// exaust the fmac pipeline for a maximum of 4 cycles
	//for ( Count = 0; Count < c_CycleTimeout; Count++ )
	for ( Count = 0; Count < Diff; Count++ )
	{
		AdvanceCycle ();
		
		// check if we are at the correct cycle yet
		//if ( CycleCount >= WaitUntil_Cycle ) return;
	}
	
	// at this point can just set the cycle# to the correct value
	// since there is nothing else to wait for
	CycleCount = WaitUntil_Cycle;
	
	/*
	// check if the Q register was set in the meantime too
	if ( CycleCount >= QBusyUntil_Cycle )
	{
		SetQ ();
	}
	
	// check if the P register was set in the meantime too
	if ( CycleCount >= ( PBusyUntil_Cycle - 1 ) )
	{
		SetP ();
	}
	*/
}

// force pipeline to wait for the Q register
void VU::PipelineWaitQ ()
{
	PipelineWaitCycle ( QBusyUntil_Cycle );
	
	// done waiting for Q register
	//QBusyUntil_Cycle = 0LL;
	
	if ( QBusyUntil_Cycle != -1LL )
	{
		SetQ ();
	}
}

// force pipeline to wait for the P register
void VU::PipelineWaitP ()
{
	// note: must wait only until one cycle before the P register is supposed to be free
	// because "PBusyUntil_Cycle" actually specifies the cycle the P register should be updated for "MFP" instruction
	// any stalls actually only stall until one cycle before that
	//PipelineWaitCycle ( PBusyUntil_Cycle );
	PipelineWaitCycle ( PBusyUntil_Cycle - 1 );
	
	// done waiting for P register
	//PBusyUntil_Cycle = 0LL;
	
	SetP ();
}


// whenever the pipeline advances, there's a bunch of stuff that must be updated
// or at least theoretically
// this will advance to the next cycle in VU
void VU::AdvanceCycle ()
{
	// this counts the bus cycles, not R5900 cycles
	CycleCount++;

	
#ifndef ENABLE_NEW_QP_HANDLING
	// update q and p registers here for now
	UpdateQ ();
	//UpdateP ();
#endif


#ifdef INLINE_DEBUG_VUEXECUTE	
if ( FlagSave [ ( iFlagSave_Index + 1 ) & c_lFlag_Delay_Mask ].FlagsAffected == 1 )
{
Vu::Instruction::Execute::debug << hex << " Aff=" << FlagSave [ ( iFlagSave_Index + 1 ) & c_lFlag_Delay_Mask ].FlagsAffected << " STAT=" << FlagSave [ ( iFlagSave_Index + 1 ) & c_lFlag_Delay_Mask ].StatusFlag << " MAC=" << FlagSave [ ( iFlagSave_Index + 1 ) & c_lFlag_Delay_Mask ].MACFlag;
}
#endif

	// update the flags here for now
	UpdateFlags ();
}

// can modify this later to include the R5900 cycle number, to keep more in sync if needed in macro mode
void VU::MacroMode_AdvanceCycle ( u32 Instruction )
{
	///////////////////////////////////////////////////////////////////////////////////////////
	// R0 is always zero - must be cleared before any instruction is executed, not after
	vi [ 0 ].u = 0;
	
	// f0 always 0, 0, 0, 1 ???
	vf [ 0 ].uLo = 0;
	vf [ 0 ].uHi = 0;
	vf [ 0 ].uw = 0x3f800000;
	
	// process load delay slot
	if ( Status.EnableLoadMoveDelaySlot )
	{
		// this clears the EnableLoadMoveDelaySlot, so no need to do it here
		Instruction::Execute::Execute_LoadDelaySlot ( this, (Vu::Instruction::Format&) Instruction );
	}

#ifdef ENABLE_INTDELAYSLOT	
	// go ahead and set the integer delay slot result
	//Execute_IntDelaySlot ();
	if ( Status.IntDelayValid )
	{
		vi [ IntDelayReg ].u = IntDelayValue;
		//Status.IntDelayValid = 0;
	}
#endif

	// clear the status flag here for macro mode (no longer needed)
	Status.Value = 0;

#ifdef UPDATE_MACRO_FLAGS_IMMEDIATELY
	SetCurrentFlags ();
#else

	// this counts the bus cycles, not R5900 cycles
	CycleCount++;
	
	// update q and p registers here for now
	UpdateQ ();
	UpdateP ();

	// update the flags here for now
	UpdateFlags ();

#endif
}



void VU::Execute_XgKick ()
{
/*
#if defined INLINE_DEBUG_XGKICK || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_STALLS

	// set the source integer register
	Set_Int_SrcReg ( i.is + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		PipelineWait_INT ();
	}
	
	// note: don't want to set destination register until after upper instruction is executed!!!
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	Execute_IntDelaySlot ();
#endif

#if defined INLINE_DEBUG_XGKICK || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " vi=" << hex << vi [ i.is ].uLo;
#endif
*/

	// looks like this is only supposed to write one 128-bit value to PATH1
	// no, this actually writes an entire gif packet to path1
	// the address should only be a maximum of 10-bits, so must mask
	//GPU::Path1_WriteBlock ( & VuMem64 [ ( XgKick_Address & 0x3ff ) << 1 ] );
	GPU::Path1_WriteBlock ( VuMem64, XgKick_Address & 0x3ff );
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void VU::DebugWindow_Enable ( int Number )
{

#ifndef _CONSOLE_DEBUG_ONLY_

	static const char* COP0_Names [] = { "Index", "Random", "EntryLo0", "EntryLo1", "Context", "PageMask", "Wired", "Reserved",
								"BadVAddr", "Count", "EntryHi", "Compare", "Status", "Cause", "EPC", "PRId",
								"Config", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "BadPAddr",
								"Debug", "Perf", "Reserved", "Reserved", "TagLo", "TagHi", "ErrorEPC", "Reserved" };
								
	static const char* DisAsm_Window_ColumnHeadings [] = { "Address", "@", ">", "Instruction", "Inst (hex)" };
								
	static const char* FontName = "Courier New";
	static const int FontSize = 6;
	
	static const char* DebugWindow_Caption = "VU Debug Window";
	static const int DebugWindow_X = 10;
	static const int DebugWindow_Y = 10;
	static const int DebugWindow_Width = 995;
	static const int DebugWindow_Height = 420;
	
	static const int GPRList_X = 0;
	static const int GPRList_Y = 0;
	static const int GPRList_Width = 190;
	static const int GPRList_Height = 380;

	static const int COP1List_X = GPRList_X + GPRList_Width;
	static const int COP1List_Y = 0;
	static const int COP1List_Width = 175;
	static const int COP1List_Height = 300;
	
	static const int COP2_CPCList_X = COP1List_X + COP1List_Width;
	static const int COP2_CPCList_Y = 0;
	static const int COP2_CPCList_Width = 175;
	static const int COP2_CPCList_Height = 300;
	
	static const int COP2_CPRList_X = COP2_CPCList_X + COP2_CPCList_Width;
	static const int COP2_CPRList_Y = 0;
	static const int COP2_CPRList_Width = 175;
	static const int COP2_CPRList_Height = 300;
	
	static const int DisAsm_X = COP2_CPRList_X + COP2_CPRList_Width;
	static const int DisAsm_Y = 0;
	static const int DisAsm_Width = 270;
	static const int DisAsm_Height = 380;
	
	static const int MemoryViewer_Columns = 8;
	static const int MemoryViewer_X = GPRList_X + GPRList_Width;
	static const int MemoryViewer_Y = 300;
	static const int MemoryViewer_Width = 250;
	static const int MemoryViewer_Height = 80;
	
	static const int BkptViewer_X = MemoryViewer_X + MemoryViewer_Width;
	static const int BkptViewer_Y = 300;
	static const int BkptViewer_Width = 275;
	static const int BkptViewer_Height = 80;
	
	int i;
	stringstream ss;
	
#ifdef INLINE_DEBUG
	debug << "\r\nStarting Cpu::DebugWindow_Enable";
#endif
	
	if ( !DebugWindow_Enabled [ Number ] )
	{
		// create the main debug window
		DebugWindow [ Number ] = new WindowClass::Window ();
		DebugWindow [ Number ]->Create ( DebugWindow_Caption, DebugWindow_X, DebugWindow_Y, DebugWindow_Width, DebugWindow_Height );
		DebugWindow [ Number ]->Set_Font ( DebugWindow [ Number ]->CreateFontObject ( FontSize, FontName ) );
		DebugWindow [ Number ]->DisableCloseButton ();
		
		// create "value lists"
		GPR_ValueList [ Number ] = new DebugValueList<float> ();
		COP0_ValueList [ Number ] = new DebugValueList<u32> ();
		COP2_CPCValueList [ Number ] = new DebugValueList<u32> ();
		COP2_CPRValueList [ Number ] = new DebugValueList<u32> ();
		
		// create the value lists
		GPR_ValueList [ Number ]->Create ( DebugWindow [ Number ], GPRList_X, GPRList_Y, GPRList_Width, GPRList_Height );
		COP0_ValueList [ Number ]->Create ( DebugWindow [ Number ], COP1List_X, COP1List_Y, COP1List_Width, COP1List_Height );
		COP2_CPCValueList [ Number ]->Create ( DebugWindow [ Number ], COP2_CPCList_X, COP2_CPCList_Y, COP2_CPCList_Width, COP2_CPCList_Height );
		COP2_CPRValueList [ Number ]->Create ( DebugWindow [ Number ], COP2_CPRList_X, COP2_CPRList_Y, COP2_CPRList_Width, COP2_CPRList_Height );
		
		GPR_ValueList [ Number ]->EnableVariableEdits ();
		COP0_ValueList [ Number ]->EnableVariableEdits ();
		COP2_CPCValueList [ Number ]->EnableVariableEdits ();
		COP2_CPRValueList [ Number ]->EnableVariableEdits ();
	
		// add variables into lists
		for ( i = 0; i < 32; i++ )
		{
			ss.str ("");
			ss << "vf" << dec << i << "_x";
			//GPR_ValueList [ Number ]->AddVariable ( ss.str().c_str(), &(_VU [ Number ]->vf [ i ].uw0) );
			GPR_ValueList [ Number ]->AddVariable ( ss.str().c_str(), &(_VU [ Number ]->vf [ i ].fx) );
			ss.str ("");
			ss << "vf" << dec << i << "_y";
			//GPR_ValueList [ Number ]->AddVariable ( ss.str().c_str(), &(_VU [ Number ]->vf [ i ].uw1) );
			GPR_ValueList [ Number ]->AddVariable ( ss.str().c_str(), &(_VU [ Number ]->vf [ i ].fy) );
			ss.str ("");
			ss << "vf" << dec << i << "_z";
			//GPR_ValueList [ Number ]->AddVariable ( ss.str().c_str(), &(_VU [ Number ]->vf [ i ].uw2) );
			GPR_ValueList [ Number ]->AddVariable ( ss.str().c_str(), &(_VU [ Number ]->vf [ i ].fz) );
			ss.str ("");
			ss << "vf" << dec << i << "_w";
			//GPR_ValueList [ Number ]->AddVariable ( ss.str().c_str(), &(_VU [ Number ]->vf [ i ].uw3) );
			GPR_ValueList [ Number ]->AddVariable ( ss.str().c_str(), &(_VU [ Number ]->vf [ i ].fw) );
			
			//COP0_ValueList->AddVariable ( COP0_Names [ i ], &(_CPU->CPR0.Regs [ i ]) );

			if ( i < 16 )
			{
				ss.str("");
				ss << "vi" << dec << i;
				COP2_CPCValueList [ Number ]->AddVariable ( ss.str().c_str(), &(_VU [ Number ]->vi [ i ].u) );
				
				//ss.str("");
				//ss << "VI_" << dec << ( ( i << 1 ) + 1 );
				//COP2_CPCValueList->AddVariable ( ss.str().c_str(), &(_CPU->COP2.CPC2.Regs [ ( i << 1 ) + 1 ]) );
			}
			else
			{
				ss.str("");
				ss << "CPC2_" << dec << i;
				COP2_CPRValueList [ Number ]->AddVariable ( ss.str().c_str(), &(_VU [ Number ]->vi [ i ].u) );
				
				//ss.str("");
				//ss << "CPR2_" << dec << ( ( ( i - 16 ) << 1 ) + 1 );
				//COP2_CPRValueList->AddVariable ( ss.str().c_str(), &(_CPU->COP2.CPR2.Regs [ ( ( i - 16 ) << 1 ) + 1 ]) );
			}
		}
		
		
		// also add PC and CycleCount
		COP0_ValueList [ Number ]->AddVariable ( "PC", &(_VU [ Number ]->PC) );
		COP0_ValueList [ Number ]->AddVariable ( "NextPC", &(_VU [ Number ]->NextPC) );
		COP0_ValueList [ Number ]->AddVariable ( "LastPC", &(_VU [ Number ]->LastPC) );
		COP0_ValueList [ Number ]->AddVariable ( "CycleLO", (u32*) &(_VU [ Number ]->CycleCount) );
		
		COP0_ValueList [ Number ]->AddVariable ( "LastReadAddress", &(_VU [ Number ]->Last_ReadAddress) );
		COP0_ValueList [ Number ]->AddVariable ( "LastWriteAddress", &(_VU [ Number ]->Last_WriteAddress) );
		COP0_ValueList [ Number ]->AddVariable ( "LastReadWriteAddress", &(_VU [ Number ]->Last_ReadWriteAddress) );
		
		if ( Number )
		{
			// also add in the count for path2 transfers for debugging vu#1 direct/directhl etc
			COP0_ValueList [ Number ]->AddVariable ( "Path2Cnt", &(GPU::_GPU->ulTransferCount [ 2 ]) );
			COP0_ValueList [ Number ]->AddVariable ( "Path2Sze", &(GPU::_GPU->ulTransferSize [ 2 ]) );
		}
		
		// need to add in load delay slot values
		//GPR_ValueList->AddVariable ( "D0_INST", &(_VU [ Number ]->DelaySlot0.Instruction.Value) );
		//GPR_ValueList->AddVariable ( "D0_VAL", &(_VU [ Number ]->DelaySlot0.Data) );
		//GPR_ValueList->AddVariable ( "D1_INST", &(_VU [ Number ]->DelaySlot1.Instruction.Value) );
		//GPR_ValueList->AddVariable ( "D1_VAL", &(_VU [ Number ]->DelaySlot1.Data) );
		
		//GPR_ValueList->AddVariable ( "SPUCC", (u32*) _SpuCycleCount );
		//GPR_ValueList->AddVariable ( "Trace", &TraceValue );

		// add some things to the cop0 value list
		COP0_ValueList [ Number ]->AddVariable ( "RUN", &(_VU [ Number ]->Running) );
		COP0_ValueList [ Number ]->AddVariable ( "VifStop", &(_VU [ Number ]->VifStopped) );

		// create the disassembly window
		DisAsm_Window [ Number ] = new Debug_DisassemblyViewer ( Breakpoints [ Number ] );
		DisAsm_Window [ Number ]->Create ( DebugWindow [ Number ], DisAsm_X, DisAsm_Y, DisAsm_Width, DisAsm_Height, Vu::Instruction::Print::PrintInstructionLO, Vu::Instruction::Print::PrintInstructionHI );
		
		DisAsm_Window [ Number ]->Add_MemoryDevice ( "MicroMem", _VU [ Number ]->ulMicroMem_Start, _VU [ Number ]->ulMicroMem_Size, (u8*) _VU [ Number ]->MicroMem8 );
		
		DisAsm_Window [ Number ]->SetProgramCounter ( &_VU [ Number ]->PC );
		
		
		// create window area for breakpoints
		Breakpoint_Window [ Number ] = new Debug_BreakpointWindow ( Breakpoints [ Number ] );
		Breakpoint_Window [ Number ]->Create ( DebugWindow [ Number ], BkptViewer_X, BkptViewer_Y, BkptViewer_Width, BkptViewer_Height );
		
		// create the viewer for D-Cache scratch pad
		ScratchPad_Viewer [ Number ] = new Debug_MemoryViewer ();
		
		ScratchPad_Viewer [ Number ]->Create ( DebugWindow [ Number ], MemoryViewer_X, MemoryViewer_Y, MemoryViewer_Width, MemoryViewer_Height, MemoryViewer_Columns );
		ScratchPad_Viewer [ Number ]->Add_MemoryDevice ( "VuMem", _VU [ Number ]->ulVuMem_Start, _VU [ Number ]->ulVuMem_Size, (u8*) _VU [ Number ]->VuMem8 );
		
		cout << "\nDebug Enable";
		
		// mark debug as enabled now
		DebugWindow_Enabled [ Number ] = true;
		
		cout << "\nUpdate Start";
		
		// update the value lists
		DebugWindow_Update ( Number );
	}
	
#endif

}

static void VU::DebugWindow_Disable ( int Number )
{

#ifndef _CONSOLE_DEBUG_ONLY_

	if ( DebugWindow_Enabled [ Number ] )
	{
		delete DebugWindow [ Number ];
		delete GPR_ValueList [ Number ];
		delete COP0_ValueList [ Number ];
		delete COP2_CPCValueList [ Number ];
		delete COP2_CPRValueList [ Number ];

		delete DisAsm_Window [ Number ];
		
		delete Breakpoint_Window [ Number ];
		delete ScratchPad_Viewer [ Number ];
		
	
		// disable debug window
		DebugWindow_Enabled [ Number ] = false;
	}
	
#endif

}

static void VU::DebugWindow_Update ( int Number )
{

#ifndef _CONSOLE_DEBUG_ONLY_

	if ( DebugWindow_Enabled [ Number ] )
	{
		GPR_ValueList [ Number ]->Update();
		COP0_ValueList [ Number ]->Update();
		COP2_CPCValueList [ Number ]->Update();
		COP2_CPRValueList [ Number ]->Update();
		DisAsm_Window [ Number ]->GoTo_Address ( _VU [ Number ]->PC );
		DisAsm_Window [ Number ]->Update ();
		Breakpoint_Window [ Number ]->Update ();
		ScratchPad_Viewer [ Number ]->Update ();
	}
	
#endif

}


