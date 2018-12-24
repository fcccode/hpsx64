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


#ifndef _PS2_VU_H_
#define _PS2_VU_H_

#include "types.h"
#include "Debug.h"

//#include "WinApiHandler.h"
//#include "GNUAsmUtility_x64.h"

#ifdef ENABLE_GUI_DEBUGGER
#include "WinApiHandler.h"
// Windows API specific code is in this header file
#include "DebugValueList.h"
#include "DebugMemoryViewer.h"
#include "DisassemblyViewer.h"
#include "BreakpointWindow.h"
#endif


#include "VU_Instruction.h"
//#include "VU_Recompiler.h"

//#include "emmintrin.h"



#ifdef _ENABLE_SSE2

// need to include this file to use SSE2 intrinsics
//#include "emmintrin.h"
//#include "smmintrin.h"

#endif


// will need this for accurate operation, and cycle accuracy is required
#define ENABLE_STALLS


//#define ENABLE_NEW_CLIP_BUFFER
//#define ENABLE_NEW_FLAG_BUFFER
#define ENABLE_SNAPSHOTS


using namespace Vu;
//using namespace x64Asm::Utilities;




namespace Playstation2
{

	class VU1;

	class VU
	{
	
		
	public:
		static Debug::Log debug;
	
		static const int c_iMaxInstances = 2;
		static VU *_VU [ c_iMaxInstances ];
		
		// this variable has nothing to do with whether it will be VU0 or VU1
		static int iInstance;
		
		//////////////////////////
		//	General Parameters	//
		//////////////////////////
		
		// where the registers start at
		static const long Regs_Start = 0x10003000;
		
		// where the registers end at
		static const long Regs_End = 0x100037f0;
	
		// distance between groups of registers
		static const long Reg_Size = 0x10;
		
		// need to pack lots of info into the structure for debugging and read/write of hardware registers
		struct HW_Register
		{
			bool ReadOK;
			bool WriteOK;
			bool Unknown;
			char* Name;
			u32 Address;
			u32 SizeInBytes;
			u32* DataPtr;
		};
		
		HW_Register Registers [ Regs_End - Regs_Start + Reg_Size ];

		static const int c_iNumVuRegs = 24;
		static const char* VU0RegNames [ c_iNumVuRegs ];
		static const char* VU1RegNames [ c_iNumVuRegs ];
		
		// VU Clock Speed in Hertz
		static const unsigned long long c_llClockSpeed1 = 147456000;
		static const unsigned long long c_llClockSpeed2 = 149500000;

		static const double c_dClockSpeed1 = 147456000.0L;
		static const double c_dClockSpeed2 = 149500000.0L;
		
		
		// the number of vu cycles for every cpu cycle or vice versa
		static const double c_dVUPerCPU_Cycles = ( 1.0L / 2.0L );
		static const double c_dCPUPerVU_Cycles = ( 2.0L / 1.0L );
		
		static const u64 c_BIAS = ( 1ULL << 31 );
		static const u64 c_BIAS24 = ( 1ULL << 23 );
		static inline s64 _Round ( s64 FixedPointValue )
		{
			return ( FixedPointValue + c_BIAS );
		}
		
		static inline s64 _Round24 ( s64 FixedPointValue )
		{
			return ( FixedPointValue + c_BIAS24 );
		}
		
		static inline u64 _Abs ( s64 Value )
		{
			return ( ( Value >> 63 ) ^ Value ) - ( Value >> 63 );
		}
		
		static inline u32 _Abs ( s32 Value )
		{
			return ( ( Value >> 31 ) ^ Value ) - ( Value >> 31 );
		}
		
		
		
		// 0x11000000 is physical address of where micromem starts at
		static const u32 c_ulMicroMem0_Start = 0x0;	//0x11000000;
		static const u32 c_ulMicroMem0_Size = 0x1000;	// 4 KB
		static const u32 c_ulMicroMem0_Mask = c_ulMicroMem0_Size - 1;
		
		static const u32 c_ulVuMem0_Start = 0x0;	//0x11004000;
		static const u32 c_ulVuMem0_Size = 0x1000;	// 4 KB
		static const u32 c_ulVuMem0_Mask = c_ulVuMem0_Size - 1;

		
		static const u32 c_ulMicroMem1_Start = 0x0;	//0x11008000;
		static const u32 c_ulMicroMem1_Size = 0x4000;	// 16 KB
		static const u32 c_ulMicroMem1_Mask = c_ulMicroMem1_Size - 1;
		
		static const u32 c_ulVuMem1_Start = 0x0;	//0x1100c000;
		static const u32 c_ulVuMem1_Size = 0x4000;	// 16 KB
		static const u32 c_ulVuMem1_Mask = c_ulVuMem1_Size - 1;
		
		
		// the number of cycles that floating point instructions spend in pipeline before they are available
		static const u64 c_ullFloatPipeline_Cycles = 4;
		
		
		u32* GetMemPtr ( u32 Address32 );
		
		
		// recompiler data
		//Recompiler rs;
		//static Vu::Recompiler* vrs [ 2 ];
		u32 bEnableRecompiler;
		static u32 bCodeModified [ 2 ];

		/*
		struct ControlRegs_t
		{
		
			union
			{

				// there are 32 COP2 control registers
				unsigned long Regs [ 32 ];
				
				struct
				{
					u32 Reg0;
					u32 Reg1;
					u32 Reg2;
					u32 Reg3;
					u32 Reg4;
					u32 Reg5;
					u32 Reg6;
					u32 Reg7;
					u32 Reg8;
					u32 Reg9;
					u32 Reg10;
					u32 Reg11;
					u32 Reg12;
					u32 Reg13;
					u32 Reg14;
					u32 Reg15;
					u32 StatusFlags;
					u32 MACFlags;
					u32 ClippingFlags;
					u32 Reserved0;
					
					// these three cannot be read or written while VU0 is operating
					u32 R;
					u32 I;
					u32 Q;
					
					// this one is only bits 0-15, the rest are zero
					// cannot be read or written while VU0 is operating
					u32 TPC;
				};
			};
			
		};
		*/

		
		// need to know which registers correspond to what
		enum
		{
			REG_STATUSFLAG = 16,
			REG_MACFLAG = 17,
			REG_CLIPFLAG = 18,
			REG_R = 20,
			REG_I = 21,
			REG_Q = 22,
			
			// todo: unsure which one is the P register
			REG_P = 23,
			
			REG_TPC = 26,
			REG_CMSAR0 = 27,
			REG_FBRST = 28,
			REG_VPUSTAT = 29,
			REG_CMSAR1 = 31
		};

		
		inline void SetQ ()
		{
			// set the Q register
			vi [ REG_Q ].s = NextQ.l;
			
			// clear non-sticky div unit flags
			vi [ REG_STATUSFLAG ].uLo &= ~0x30;
			
			// set flags
			vi [ REG_STATUSFLAG ].uLo |= NextQ_Flag;
			
			// don't set the Q register again until div unit is used again
			// should clear to zero to indicate last event happened far in the past
			QBusyUntil_Cycle = -1LL;
			//QBusyUntil_Cycle = 0LL;
		}
		
		inline void SetP ()
		{
			vi [ REG_P ].s = NextP.l;
			
			// should set this to zero to indicate it happened far in the past
			//PBusyUntil_Cycle = -1LL;
			//PBusyUntil_Cycle = 0LL;
		}

		inline void UpdateQ ()
		{
			if ( CycleCount >= QBusyUntil_Cycle )
			{
				// set the q register
				SetQ ();
			}
		}
		
		inline void UpdateP ()
		{
			if ( CycleCount >= PBusyUntil_Cycle )
			{
				// set the p register
				SetP ();
			}
		}




		// check if P register is done processing and set if so
		inline void CheckP ()
		{
			if ( CycleCount >= ( PBusyUntil_Cycle - 1 ) )
			{
				SetP ();
				PBusyUntil_Cycle = -1LL;
			}
		}

		// check P register before processing MFP
		inline void CheckP_MFP ()
		{
			if ( CycleCount >= PBusyUntil_Cycle )
			{
				SetP ();
				PBusyUntil_Cycle = -1LL;
			}
		}
		
		// wait for the P register to be ready and then set
		inline void WaitP ()
		{
			if ( PBusyUntil_Cycle != -1LL )
			{
				if ( CycleCount < ( PBusyUntil_Cycle - 1 ) )
				{
					CycleCount = ( PBusyUntil_Cycle - 1 );
				}
				
				SetP ();
				PBusyUntil_Cycle = -1LL;
			}
		}
		
		
		// check if Q register is done processing and set if so
		inline void CheckQ ()
		{
			if ( CycleCount >= QBusyUntil_Cycle )
			{
				SetQ ();
				QBusyUntil_Cycle = -1LL;
			}
		}
		
		// wait for the Q register to be ready and then set
		inline void WaitQ ()
		{
			if ( QBusyUntil_Cycle != -1LL )
			{
				if ( CycleCount < QBusyUntil_Cycle )
				{
					CycleCount = QBusyUntil_Cycle;
				}
				
				SetQ ();
				QBusyUntil_Cycle = -1LL;
			}
		}
		
		
		/*
		// read and write index for the vu flags circular buffer
		u64 ullFlagReadIndex, ullFlagWriteIndex;
		
		inline void UpdateNextFlag ()
		{
			int iFlagIdx;
			
			// check if there are any flags to process
			if ( ullFlagReadIndex < ullFlagWriteIndex )
			{
				iFlagIdx = ullFlagReadIndex & c_lFlag_Delay_Mask;
				
				// update the read index to the next item for next time
				ullFlagReadIndex++;
				
				// check if it is time to apply the flag yet
				if ( FlagSave [ iFlagIdx ].ullBusyUntil_Cycle <= CycleCount )
				{
					switch ( FlagSave [ iFlagIdx ].FlagsAffected )
					{
						case RF_SET_MACSTAT:
							// set status flag
							vi [ 16 ].uLo &= ~0xf;
							vi [ 16 ].uLo |= FlagSave [ iFlagIdx ].StatusFlag;
							
							// set MAC flag
							vi [ 17 ].uLo = FlagSave [ iFlagIdx ].MACFlag;
							
							break;

						case RF_SET_MACSTICKY:
							// set sticky status flag
							vi [ REG_STATUSFLAG ].uLo &= 0x3f;
							vi [ REG_STATUSFLAG ].uLo |= FlagSave [ iFlagIdx ].StatusFlag;
							
							// set MAC flag
							vi [ 17 ].uLo = FlagSave [ iFlagIdx ].MACFlag;
							break;
							
						case RF_SET_STICKY:
							// set status flag
							vi [ REG_STATUSFLAG ].uLo &= 0x3f;
							vi [ REG_STATUSFLAG ].uLo |= FlagSave [ iFlagIdx ].StatusFlag;
							break;
							
						// *TODO* clip instructions can be handled on a different pipeline
						case RF_SET_CLIP:
							// set clipping flag
							vi [ REG_CLIPFLAG ].u = FlagSave [ iFlagIdx ].ClippingFlag;
							break;
							
						// *TODO* clip instructions can be handled on a different pipeline
						case RF_UPDATE_CLIP:
							// update clipping flag
							vi [ REG_CLIPFLAG ].u = ( ( vi [ REG_CLIPFLAG ].u << 6 ) | ( FlagSave [ iFlagIdx ].ClippingFlag & 0x3f ) ) & 0xffffff;
							break;
					}
				}
			}
		}
		*/


		// flag get/set instructions will need the current flags up to date
		inline void UpdateCurrentFlags ()
		{
		}
		
		// update the next bitmap
		inline void UpdateNextBitmap ()
		{
		}
		
		// process bitmaps until the registers in bitmap are no longer in use
		inline void WaitBitmap ( u64 Bitmap )
		{
		}

		
		inline void UpdateFlags ()
		{

			// this can use the cycle counter later possibly
			iFlagSave_Index++;
			
			// set the flags
			int FlagIndex = ( iFlagSave_Index - 4 ) & c_lFlag_Delay_Mask;

#ifdef ENABLE_SNAPSHOTS
			FlagSave [ FlagIndex ].StatusFlag = vi [ REG_STATUSFLAG ].u;
			FlagSave [ FlagIndex ].MACFlag = vi [ REG_MACFLAG ].u;
			FlagSave [ FlagIndex ].ClipFlag = vi [ REG_CLIPFLAG ].u;
#else			
			// MAC and status flags
			if ( FlagSave [ FlagIndex ].FlagsAffected )
			{
				switch ( FlagSave [ FlagIndex ].FlagsAffected )
				{
#ifndef ENABLE_NEW_FLAG_BUFFER
					case RF_SET_MACSTAT:
						// set status flag
						vi [ 16 ].uLo &= ~0xf;
						vi [ 16 ].uLo |= FlagSave [ FlagIndex ].StatusFlag;
						
						// set MAC flag
						vi [ 17 ].uLo = FlagSave [ FlagIndex ].MACFlag;
						
						break;
						
					case RF_SET_STICKY:
						// set status flag
						vi [ REG_STATUSFLAG ].uLo &= 0x3f;
						vi [ REG_STATUSFLAG ].uLo |= FlagSave [ FlagIndex ].StatusFlag;
						break;
#endif
						
#ifndef ENABLE_NEW_CLIP_BUFFER
					case RF_SET_CLIP:
						// set clipping flag
						vi [ REG_CLIPFLAG ].u = FlagSave [ FlagIndex ].ClippingFlag;
						break;
						
					case RF_UPDATE_CLIP:
						// update clipping flag
						vi [ REG_CLIPFLAG ].u = ( ( vi [ REG_CLIPFLAG ].u << 6 ) | ( FlagSave [ FlagIndex ].ClippingFlag & 0x3f ) ) & 0xffffff;
						break;
#endif
						
					default:
						break;
				}
				
				
				// disable flag entry
				FlagSave [ FlagIndex ].FlagsAffected = 0;
			}

			if ( FlagSave [ FlagIndex ].FlagsAffected_Lower )
			{
				switch ( FlagSave [ FlagIndex ].FlagsAffected_Lower )
				{
						
#ifndef ENABLE_NEW_FLAG_BUFFER
					case RF_SET_STICKY:
						// set status flag
						vi [ REG_STATUSFLAG ].uLo &= 0x3f;
						vi [ REG_STATUSFLAG ].uLo |= FlagSave [ FlagIndex ].FlagsSet_Lower;
						break;
#endif
						
#ifndef ENABLE_NEW_CLIP_BUFFER
					case RF_SET_CLIP:
						// set clipping flag
						vi [ REG_CLIPFLAG ].u = FlagSave [ FlagIndex ].FlagsSet_Lower;
						break;
#endif
						
					default:
						break;
				}
				
				FlagSave [ FlagIndex ].FlagsAffected_Lower = 0;
			}
#endif


#ifdef ENABLE_STALLS
			// for now, will process bitmap on every instruction
			
			// remove bitmap from pipeline
			//RemoveBitmap ( Pipeline_Bitmap, FlagSave [ FlagIndex ].Bitmap );
			RemovePipeline ();
			
			// need to clear the bitmap for the entry for now
			ClearBitmap ( FlagSave [ FlagIndex ].Bitmap );
			
			// remove from MACRO pipeline also
			Int_Pipeline_Bitmap &= ~FlagSave [ FlagIndex ].Int_Bitmap;
			FlagSave [ FlagIndex ].Int_Bitmap = 0;
#endif


#ifdef CYCLE_SKIP_FLAGS
			// if the cycle number for the flags is greater, then update
			if ( CycleCount < FlagSave [ FlagIndex ].ullBusyUntil_Cycle )
			{
				CycleCount = FlagSave [ FlagIndex ].ullBusyUntil_Cycle;
			}
#endif


		}
		
		// will use this for now for VU#0 macro mode, but will probably need something better later
		inline void SetCurrentFlags ()
		{
#ifndef ENABLE_SNAPSHOTS
			if ( FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].FlagsAffected )
			{
				switch ( FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].FlagsAffected )
				{
#ifndef ENABLE_NEW_FLAG_BUFFER
					case RF_SET_MACSTAT:
						// set status flag
						vi [ 16 ].uLo &= ~0xf;
						vi [ 16 ].uLo |= FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].StatusFlag;
						
						// set MAC flag
						vi [ 17 ].uLo = FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].MACFlag;
						
						break;
			
					case RF_SET_STICKY:
						// set status flag
						vi [ REG_STATUSFLAG ].uLo &= 0x3f;
						vi [ REG_STATUSFLAG ].uLo |= FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].StatusFlag;
						break;
#endif
						
#ifndef ENABLE_NEW_CLIP_BUFFER
					case RF_SET_CLIP:
						// set clipping flag
						vi [ REG_CLIPFLAG ].u = FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].ClippingFlag;
						break;
						
					case RF_UPDATE_CLIP:
						// update clipping flag
						vi [ REG_CLIPFLAG ].u = ( ( vi [ REG_CLIPFLAG ].u << 6 ) | ( FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].ClippingFlag & 0x3f ) ) & 0xffffff;
						break;
#endif
						
					default:
						break;
				}
				
				// disable flag entry
				FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].FlagsAffected = 0;
			}

#ifdef ENABLE_NEW_CLIP_BUFFER
			Get_CFBuffer_Force ( 3 );
#endif
			
#ifdef ENABLE_NEW_FLAG_BUFFER
			Get_MFBuffer_Force ( 3 );
			Get_SFBuffer_Force ( 3 );
#endif

#endif


#ifdef ENABLE_STALLS
			// for now, will process bitmap on every instruction
			
			// remove bitmap from pipeline
			//RemoveBitmap ( Pipeline_Bitmap, FlagSave [ FlagIndex ].Bitmap );
			RemovePipeline ();
			
			// need to clear the bitmap for the entry for now
			ClearBitmap ( FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].Bitmap );
			
			// remove from MACRO pipeline also
			Int_Pipeline_Bitmap &= ~FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].Int_Bitmap;
			FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].Int_Bitmap = 0;
#endif
		}
		
		
		union StatusFlags_t
		{
			u32 Value;
			
			struct
			{
				// bit 0 - zero flag - gets set when any of the Zx, Zy, Zz, Zw flags are set in MAC flag
				u32 ZeroFlag : 1;
				
				// bit 1 - sign flag
				u32 SignFlag : 1;
				
				// bit 2 - underflow flag
				u32 UnderflowFlag : 1;
				
				// bit 3 - overflow flag
				u32 OverflowFlag : 1;
				
				// bit 4 - invalid flag - gets set on either 0/0 or SQRT/RSQRT on a negative number
				u32 InvalidFlag : 1;
				
				// bit 5 - division by zero flag - get set when either DIV/RSQRT does division by zero
				u32 DivideByZeroFlag : 1;
				
				// bit 6 - sticky zero flag - gets set when any of the Zx, Zy, Zz, Zw flags are set in MAC flag
				u32 StickyZeroFlag : 1;
				
				// bit 7 - sticky sign flag
				u32 StickySignFlag : 1;
				
				// bit 8 - sticky underflow flag
				u32 StickyUnderflowFlag : 1;
				
				// bit 9 - sticky overflow flag
				u32 StickyOverflowFlag : 1;
				
				// bit 10 - sticky invalid flag - gets set on either 0/0 or SQRT/RSQRT on a negative number
				u32 StickyInvalidFlag : 1;
				
				// bit 11 - sticky division by zero flag - get set when either DIV/RSQRT does division by zero
				u32 StickyDivideByZeroFlag : 1;
			};
		};
		
		StatusFlags_t StatusFlags;
		
		
		// vu registers

		// COP2 //
		
		// last 16 registers are control registers
		// #16 - Status flag
		// #17 - MAC flag
		// read only
		// #18 - clipping flag
		// #19 - reserved
		// #20 - R register
		// #21 - I register
		// #22 - Q register
		// #23 - P register
		// #24-#25 - reserved
		// #26 - TPC register
		// read only except while running
		// #27 - CMSAR0 register
		// #28 - FBRST register
		// #29 - VPU-STAT register
		// read only
		// #30 - reserved
		// #31 - CMSAR1 register
		// while VU1 is stopped, writing to here starts it at address that is written
		
		// COP2 registers (for now)
		Reg128 vf [ 32 ];
		Reg32 vi [ 32 ];
		
		// note: these should be mapped to VI registers
		//Reg32 I, P, Q, R;
		
		
		// accumulator
		// make just float for now
		//DoubleLong dACC [ 4 ];
		FloatLong dACC [ 4 ];
		
		// flags ??
		Reg128 zero_flag, sign_flag, overflow_flag, underflow_flag;
		Reg128 zero_stickyflag, sign_stickyflag, overflow_stickyflag, underflow_stickyflag;
		
		u32 divide_flag, invalid_negative_flag, invalid_zero_flag;
		u32 divide_stickyflag, invalid_negative_stickyflag, invalid_zero_stickyflag;
		
		// vector constants for the recompiler
		u32 CountOfVConsts;
		u32 LastVConstCount;
		Reg128 VectorConstants [ 4096 ];
		
		
		u32 Recompiler_BranchDelayAddress;
		
		union
		{
			struct
			{
				u32 Recompiler_EnableBranchDelay;
				u32 Recompiler_EnableEBitDelay;
			};
			
			u64 Recompiler_EnableFlags;
		};

		
		
		union ClippingFlag_t
		{
			struct
			{
				u32 x_plus0 : 1;
				u32 x_minus0 : 1;
				u32 y_plus0 : 1;
				u32 y_minus0 : 1;
				u32 z_plus0 : 1;
				u32 z_minus0 : 1;
				
				u32 x_plus1 : 1;
				u32 x_minus1 : 1;
				u32 y_plus1 : 1;
				u32 y_minus1 : 1;
				u32 z_plus1 : 1;
				u32 z_minus1 : 1;
				
				u32 x_plus2 : 1;
				u32 x_minus2 : 1;
				u32 y_plus2 : 1;
				u32 y_minus2 : 1;
				u32 z_plus2 : 1;
				u32 z_minus2 : 1;
				
				u32 x_plus3 : 1;
				u32 x_minus3 : 1;
				u32 y_plus3 : 1;
				u32 y_minus3 : 1;
				u32 z_plus3 : 1;
				u32 z_minus3 : 1;
			};
			
			u32 Value;
		};
		
		ClippingFlag_t ClippingFlag;
		
		u32 PC;
		u32 NextPC;
		u32 LastPC;
		u32 Last_ReadAddress;
		u32 Last_WriteAddress;
		u32 Last_ReadWriteAddress;
		
		u64 CycleCount;
		
		// use this flag to control whether VU is running or not
		// once the VU gets the command to run code, this should get set to 1, and then get cleared to 0 when done
		u32 Running;
		
		// use this flag to stop transfer via VIF, for example, while VU is running
		u32 VifStopped;
		
		// these need to be set by creator of object
		u32 ulVuMem_Start, ulMicroMem_Start;
		u32 ulVuMem_Size, ulMicroMem_Size;
		u32 ulVuMem_Mask, ulMicroMem_Mask;
		
		// cycle that the next event will happen at for this device
		u64 NextEvent_Cycle;
		
		
		
		u32 NextEvent_Idx;
		

		// the cycle that device is busy until
		u64 BusyUntil_Cycle;

		u32 Number;

		u64 Read ( u32 Address, u64 Mask );
		void Write ( u32 Address, u64 Data, u64 Mask );
		
		void DMA_Read ( u32* Data, int ByteReadCount );
		void DMA_Write ( u32* Data, int ByteWriteCount );
		
		// this will need to return how many quad words were fully written out of number sent
		u32 DMA_WriteBlock ( u64* Data, u32 QuadwordCount );
		
		// this will need to return how many quadwords were fully read from device out of number requested
		u32 DMA_ReadBlock ( u64* Data, u32 QuadwordCount );
		
		bool DMA_Write_Ready ();
		bool DMA_Read_Ready ();
		
		void Start ( int Number );

		// returns either vblank interrupt signal, gpu interrupt signal, or no interrupt signal
		void Run ();
		void Reset ();
		
		void ProcessBranchDelaySlot ();
		
		
		u32 Read_CFC ( u32 Register );
		void Write_CTC ( u32 Register, u32 Data );

		void Execute_XgKick ();
		
		
		union VifCode_t
		{
			struct
			{
				// the immediate value - bits 0-15
				u32 IMMED : 16;
				
				// num field - bits 16-23
				u32 NUM : 8;
				
				// cmd field - bits 24-30
				u32 CMD : 7;
				
				// interrupt bit
				u32 INT : 1;
			};
			
			struct
			{
				u32 ADDR : 10;

				u32 zero0 : 4;
				
				// bit 14 - usn
				// 1: unsigned, 0: signed
				u32 USN : 1;
				
				// bit 15 - FLG
				u32 FLG : 1;
				
				// bits 16-23 - varies
				u32 padding0 : 8;
				
				u32 vl : 2;
				u32 vn : 2;
				u32 m : 1;
				u32 CMDHIGH : 2;
				u32 padding : 1;
			};
			
			u32 Value;
		};
		
		// ***todo*** also reset these when VU gets reset
		u32 lVifIdx;
		VifCode_t VifCode;
		u32 lVifCodeState;
		u32 lVifCommandSize;
		
		// need to know the number of elements read (bytes, halfwords, etc)
		u32 ulReadCount;
		
		// need to determine offset data is being read from (offset is in 32-bit words)
		u32 ulReadOffset32;
		
		// need to keep track of offset data is being written to in VU memory (offset is in 32-bit words)
		u32 ulWriteOffset32;
		
		// need to know width of data being unpacked in bytes
		u32 ulUnpackItemWidth;
		
		// need to know number of elements to unpack (x,y,z, etc)
		u32 ulUnpackNum, ulUnpackNumIdx;
		
		// also will need to keep track of the last piece of unpacked data
		s32 lUnpackLastData;
		
		// buffer to hold unpacking data
		u32 ulUnpackBuf;
		
		// need to keep track of WL/CL cycle
		u32 ulWLCycle, ulCLCycle, ulWL, ulCL;
		
		// need to determine amount to shift during some unpacks
		u32 lVifUnpackIndex;
		
		// need this to return the number of quadwords read and update the offset so it points to correct data for next time
		u32 VIF_FIFO_Execute ( u32* Data, u32 SizeInWords32 );
		
		
		bool bTransferringDirectViaPath2;
		
		
		// these need to be set externally for now
		static const int c_iMaxMemSize = 0x4000;
		u32 MemSize;
		u32 MemMask;
		
		// this is the vu program memory
		union
		{
			u8 MicroMem8 [ c_iMaxMemSize ] ALIGN16;
			u16 MicroMem16 [ c_iMaxMemSize >> 1 ] ALIGN16;
			u32 MicroMem32 [ c_iMaxMemSize >> 2 ] ALIGN16;
			u64 MicroMem64 [ c_iMaxMemSize >> 3 ] ALIGN16;
		} ALIGN16;
		
		// this is the vu data memory
		union
		{
			u8 VuMem8 [ c_iMaxMemSize ] ALIGN16;
			u16 VuMem16 [ c_iMaxMemSize >> 1 ] ALIGN16;
			u32 VuMem32 [ c_iMaxMemSize >> 2 ] ALIGN16;
			u64 VuMem64 [ c_iMaxMemSize >> 3 ] ALIGN16;
		} ALIGN16;
		
		
		union CYCLE_t
		{
			struct
			{
				u32 CL : 8;
				u32 WL : 8;
				u32 padding0 : 16;
			};
			
			u32 Value;
		};
		
		
		// STAT
		union VifStat_t
		{
			struct
			{
				// VIF Status - bits 0-1 - read only
				// 00: Idle, 01: Waiting for data after VIF Code, 10: Processing VIF Code, 11: Processing/Transferring data after VIF Code
				u32 VPS : 2;
				
				// E-bit/Executing - bit 2 - read only
				// 0: Not executing program, 1: Executing program
				u32 VEW : 1;
				
				// GIF Transfer status - bit 3 - read only
				// 0: Not waiting for GIF, 1: Waiting for GIF due to FLUSH/FLUSHA or DIRECT/DIRECTHL or MSCALF
				// VIF1 only
				u32 VGW : 1;
				
				// zero - bits 4-5
				u32 zero0 : 2;
				
				// MARK detected - bit 6 - read only
				// 0: MARK not detected, 1: MARK detected
				u32 MRK : 1;
				
				// Double Buffer bit - bit 7 - read only
				// 0: TOPS is buffer 0 (BASE), 1: TOPS is buffer 1 (BASE+OFFSET)
				// VIF1 only
				u32 DBF : 1;
				
				// encountered STOP - bit 8 - read only
				// 0: No, 1: stalled due to STOP
				u32 VSS : 1;
				
				// encountered ForceBreak - bit 9 - read only
				// 0: No, 1: stalled due to ForceBreak
				u32 VFS : 1;
				
				// encountered VIF1 interrupt - bit 10 - read only
				// 0: No, 1: stalled due to VIF1 interrupt
				u32 VIS : 1;
				
				// encountered I-bit interrupt - bit 11 - read only
				// 0: No, 1: detected I-bit interrupt
				u32 INT : 1;
				
				// dma tag error - bit 12 - read only
				// 0: No, 1: error
				u32 ER0 : 1;
				
				// vif code error - bit 13 - read only
				// 0: No, 1: error
				u32 ER1 : 1;
				
				// zero - bits 14-22
				u32 zero1 : 9;
				
				// VIF-FIFO transfer direction - bit 23 - read/write
				// 0: memory->VIF, 1: VIF->memory
				// VIF1 only
				u32 FDR : 1;
				
				// Count of Quadwords of data in VIF fifo - bits 24-28 - read only
				// 00000: 0 qw (empty), 00001: 1 qw, ... 10000: 16 qw (full)
				u32 FQC : 5;
			};
			
			u32 Value;
		};
		
		
		// vif registers
		union VifRegs_t
		{
			u32 Regs [ c_iNumVuRegs ];
			
			float fRegs [ c_iNumVuRegs ];
			
			struct
			{
				VifStat_t STAT;	// register #0
				u32 FBRST;
				u32 ERR;
				u32 MARK;
				CYCLE_t CYCLE;
				u32 MODE;
				u32 NUM;
				u32 MASK;
				u32 CODE;	// register #8
				u32 ITOPS;
				u32 BASE;
				u32 OFST;
				u32 TOPS;
				u32 ITOP;
				u32 TOP;
				u32 RES;
				u32 R0;		// register #16
				u32 R1;
				u32 R2;
				u32 R3;
				u32 C0;		// register #20
				u32 C1;
				u32 C2;
				u32 C3;
			};
		};
		
		VifRegs_t VifRegs;
		
		static const int c_iRowRegStartIdx = 16;
		static const int c_iColRegStartIdx = 20;

		
		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////
		
		
		// callback functions //
		typedef void (*cbFunction) ( void );
		
		
		union ProcStatus
		{
			struct
			{
				u8 CheckInterrupt;
				u8 DelaySlot_Valid;
				u8 SetStatus_Flag;
				u8 SetClip_Flag;
				u8 EBitDelaySlot_Valid;
				u8 EnableLoadMoveDelaySlot;
				u8 IntDelayValid;
				
				// temporary measure for testing, don't execute xgkick until after next instruction
				u8 XgKickDelay_Valid;
			};
			
			struct
			{
				u32 ValueLo;
				u32 ValueHi;
			};
			
			u64 Value;
		};

		// processor status - I'll use this to make it run faster
		ProcStatus Status;
		
		// this should save the xgkick instruction in the delay slot
		//u32 XgKick_Instruction;
		u32 XgKick_Address;
		
		
		struct DelaySlot
		{
			union
			{
				struct
				{
					Instruction::Format Instruction;
					u32 Data;
					cbFunction cb;
				};
				
				struct
				{
					u64 Value;
					cbFunction Value2;
				};
			};
			
			//Execute::Callback_Function cb;
		};
		
		DelaySlot DelaySlot0;
		DelaySlot DelaySlot1;

		u32 NextDelaySlotIndex;
		DelaySlot DelaySlots [ 2 ];

		// vu integer delay slot
		u32 IntDelayValue;
		u32 IntDelayReg;
		
		// execute integer delay slot
		inline void Execute_IntDelaySlot ()
		{
			if ( Status.IntDelayValid )
			{
				Status.IntDelayValid >>= 1;
				
				if ( !Status.IntDelayValid )
				{
					vi [ IntDelayReg ].u = IntDelayValue;
				}
			}
		}
		
		// set integer delay slot value
		inline void Set_IntDelaySlot ( u32 Register, u32 Value ) { IntDelayReg = Register; IntDelayValue = Value; Status.IntDelayValid = 0x2; }

		// need to know what register was modified by lower instruction in case of delay slot
		// since if upper and lower modify same register, then the upper instruction is the only one that matters
		u32 LastModifiedRegister;
		
		// need a quick load delay slot
		// since what if lower instruction is a LQ, but upper instruction reads from the register being written to
		// it should probably read the previous value of the register due to the pipeline running concurrently
		// this could actually just go in the branch delay slot and get pulled out early
		// no, no, I'll put branches and integer arithemetic instructions (not flag operations) in the branch delay slot
		// then I would have to process non-branches before executing arithemetic instructions, or they would all go in the delay slot?
		//u32 LoadRegister;
		//u32 LoadAddress;
		// this one is for LQ,LQD,LQI,MFIR,MOVE
		// if upper instruction writes to the same register, then it will be cancelled
		// if not cancelled, will set the bitmap for the instruction when it is executed after the slight delay (1/2 of an instruction?)
		// then needs to OR that bitmap with the pipeline bitmap even with the previous stuff that might be on it
		DelaySlot LoadMoveDelaySlot;
		u32 LoadMoveDelay_Enabled;
		
		// also need a copy of the value to store from delay slot since it can change while we are waiting, like in the case of MOVE instruction
		//u64 LoadMove_Value0;
		//u64 LoadMove_Value1;
		Reg128 LoadMoveDelayReg;
	
		// also need a bitmap that says what register is being modified, so instruction can be cancelled if needed
		// bits 0-31, float registers, the rest will represent the integer registers including flags..
		// can use a bitmap for the whole register
		u64 LowerDest_Bitmap;
		u64 UpperDest_Bitmap;
		
		inline void Clear_DestReg_Upper ( u32 Register ) { UpperDest_Bitmap = 0; }
		inline void Set_DestReg_Upper ( u32 Instruction, u32 Register )
		{
			u32 xyzw32 = ( Instruction >> 21 ) & 0xf;
			FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].Int_Bitmap |= ( 1 << Register );
			AddPipeline ( Instruction, Register );
		}
		
		inline void Add_DestReg_Upper ( u32 Register )
		{
			UpperDest_Bitmap |= ( 1 << Register );
		}

		inline void Clear_DestReg_Lower ( u32 Register ) { LowerDest_Bitmap = 0; }
		inline void Set_DestReg_Lower ( u32 Register )
		{
			// TODO: may also need to set correct flag registers as modified here
			//LowerDest_Bitmap = ( 1 << Register );
			
			// add into macro pipeline bitmap (assuming lower instructions get executed "first")
			Int_Pipeline_Bitmap = ( 1 << Register );
			FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].Int_Bitmap = ( 1 << Register );
		}
		
		inline void Add_DestReg_Lower ( u32 Register )
		{
			//LowerDest_Bitmap |= ( 1 << Register );
			Int_Pipeline_Bitmap |= ( 1 << Register );
			FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].Int_Bitmap |= ( 1 << Register );
		}
		
		// we need to know if the GPU is busy or not
		s64 BusyCycles;
		
		// cycle number that the registers are in pipeline/busy until
		// will use this instead of bitmap circular buffer
		u64 Reg_BusyUntil_Cycle [ 32 * 4 ];
		
		struct Bitmap128
		{
			u64 b0;
			u64 b1;
		} ALIGN16;
		
		// it looks like flag results don't appear for four cycles on most instructions??
		union ResultFlags
		{
			struct
			{
#ifdef ENABLE_SNAPSHOTS
				u16 StatusFlag;
				u16 MACFlag;
				u32 ClipFlag;
#else
				u32 FlagsAffected;
				
				u32 FlagsAffected_Lower;
				u32 FlagsSet_Lower;
				
				union
				{
					struct
					{
						u16 MACFlag;
						u16 StatusFlag;
					};
					
					// clipping flags are related to clip instructions, and can be handled separately
					u32 ClippingFlag;
				};
#endif
				
				// integer bitmap, but int regs start at +32 in bitmap
				u64 Int_Bitmap;

				//u64 Bitmap0;
				//u64 Bitmap1;
				Bitmap128 Bitmap;
				
				// the cycle# at which this change is to take place
				//u64 ullBusyUntil_Cycle;
			};
			
			u64 Value;
			u64 Value1;
			u64 Value2;
			u64 Value3;
		} ALIGN16;
		
		
		
		static u16 Temp_StatusFlag, Temp_MacFlag;
		static Bitmap128 Temp_Bitmap;
		
		struct ClipFlags
		{
			u64 BusyUntil_Cycle;
			
			union
			{
				struct
				{
					u32 ClipFlag;
					u32 ClipShift;
				};
				
				// can use this to clear the data quickly when needed
				u64 ClipData;
			};
		};
		
		// for the clip flags buffer, can just keep dumping data
		// if slot is emptied, then set BusyUntil_Cycle to zero
		u32 iCFBufIndex;
		ClipFlags CFBuffer [ 4 ];

		inline void Get_CFBuffer ( u32 Offset )
		{
			// want to update index before setting sticky flag because the lower instruction can still set the sticky flag and mask
			
			u32 iIndex;
			iIndex = ( iCFBufIndex + Offset ) & c_lFlag_Delay_Mask;
			if ( CycleCount >= CFBuffer [ iIndex ].BusyUntil_Cycle )
			{
				// update clip flag
				vi [ REG_CLIPFLAG ].u = ( vi [ REG_CLIPFLAG ].u << CFBuffer [ iIndex ].ClipShift ) | CFBuffer [ iIndex ].ClipFlag;
				vi [ REG_CLIPFLAG ].u &= 0xffffff;
				
				// clear the data
				CFBuffer [ iIndex ].ClipData = 0;
				
				// clear the shift
				CFBuffer [ iIndex ].ClipShift = 0;
				
				// set to update far in the future since entry is emptied now
				CFBuffer [ iIndex ].BusyUntil_Cycle = -1LL;
			}
		}

		
		inline void Get_CFBuffer_Force ( u32 Offset )
		{
			// want to update index before setting sticky flag because the lower instruction can still set the sticky flag and mask
			
			u32 iIndex;
			iIndex = ( iCFBufIndex + Offset ) & c_lFlag_Delay_Mask;
			
			if ( CFBuffer [ iIndex ].BusyUntil_Cycle != -1LL )
			{
				// update clip flag
				vi [ REG_CLIPFLAG ].u = ( vi [ REG_CLIPFLAG ].u << CFBuffer [ iIndex ].ClipShift ) | CFBuffer [ iIndex ].ClipFlag;
				vi [ REG_CLIPFLAG ].u &= 0xffffff;
				
				// clear the data
				//CFBuffer [ iIndex ].ClipData = 0;
				
				// clear the shift
				//CFBuffer [ iIndex ].ClipShift = 0;
				
				// set to update far in the future since entry is emptied now
				CFBuffer [ iIndex ].BusyUntil_Cycle = -1LL;
			}
		}


		inline void Set_CFBuffer ( u32 Value, u32 Shift )
		{
			// want to update index before setting sticky flag because the lower instruction can still set the sticky flag and mask

			CFBuffer [ iCFBufIndex ].ClipFlag = Value;
			CFBuffer [ iCFBufIndex ].ClipShift = Shift;
			
			// set cycle value is valid at
			CFBuffer [ iCFBufIndex ].BusyUntil_Cycle = CycleCount + c_ullFloatPipeline_Cycles;
			
			// update index
			iCFBufIndex++;
			
			// mask index
			iCFBufIndex &= c_lFlag_Delay_Mask;
		}

		inline void Update_CFBuffer ()
		{
			Get_CFBuffer ( 0 );
			Get_CFBuffer ( 1 );
			Get_CFBuffer ( 2 );
			Get_CFBuffer ( 3 );
		}

		
		inline void Reset_CFBuffer ()
		{
			CFBuffer [ 0 ].BusyUntil_Cycle = -1LL;
			CFBuffer [ 1 ].BusyUntil_Cycle = -1LL;
			CFBuffer [ 2 ].BusyUntil_Cycle = -1LL;
			CFBuffer [ 3 ].BusyUntil_Cycle = -1LL;
		}


		
		struct SFlags
		{
			u64 BusyUntil_Cycle;
			
			u32 StatusFlag;
			u32 StatusMask;
		};


		struct MFlags
		{
			u64 BusyUntil_Cycle;
			
			u32 MACFlag;
			
			// for padding to make an even 128-bits
			u32 Dummy0;
		};


		struct BFlags
		{
			u64 BusyUntil_Cycle;
			
			Bitmap128 BFlags;
			
			// for padding to make an even 128-bits
			u64 Dummy0;
		};
		
		
		// must set the sticky flags when advancing
		// but the other flags only need to be set if it was the last value that was set (when requested)
		// when setting, can take the value of the 4 that has the highest cycle less than or equal to current
		u32 iSFBufIndex;
		SFlags SFBuffer [ 4 ];

		u32 iMFBufIndex;
		MFlags MFBuffer [ 4 ];
		
		u32 iBFBufIndex;
		BFlags BFBuffer [ 4 ];
		

		//inline u16* Get_MFBuffer_Ptr ()
		//{
		//	// want to update index before setting sticky flag because the lower instruction can still set the sticky flag and mask
		//	
		//	return (u16*) ( & MFBuffer [ iMFBufIndex ].MACFlag );
		//}
		
		// no need to get the MAC flag, since it is not like the clip flag or sticky status flag bits
		inline void Get_MFBuffer ( u32 Offset )
		{
			// want to update index before setting sticky flag because the lower instruction can still set the sticky flag and mask
			
			u32 iIndex;
			iIndex = ( iMFBufIndex + Offset ) & c_lFlag_Delay_Mask;
			if ( CycleCount >= MFBuffer [ iIndex ].BusyUntil_Cycle )
			{
				// update clip flag
				vi [ REG_MACFLAG ].u = MFBuffer [ iIndex ].MACFlag;
				
				// set to update far in the future since entry is emptied now
				MFBuffer [ iIndex ].BusyUntil_Cycle = -1LL;
			}
		}


		inline void Get_MFBuffer_Force ( u32 Offset )
		{
			// want to update index before setting sticky flag because the lower instruction can still set the sticky flag and mask
			
			u32 iIndex;
			iIndex = ( iMFBufIndex + Offset ) & c_lFlag_Delay_Mask;
			
			if ( MFBuffer [ iIndex ].BusyUntil_Cycle != -1LL )
			{
				// update MAC flag
				vi [ REG_MACFLAG ].u = MFBuffer [ iIndex ].MACFlag;
				
				// set to update far in the future since entry is emptied now
				MFBuffer [ iIndex ].BusyUntil_Cycle = -1LL;
			}
		}
		

		inline void Set_MFBuffer ( u32 Value )
		{
			// want to update index before setting sticky flag because the lower instruction can still set the sticky flag and mask

			MFBuffer [ iMFBufIndex ].MACFlag = Value;
			
			// set cycle value is valid at
			MFBuffer [ iMFBufIndex ].BusyUntil_Cycle = CycleCount + c_ullFloatPipeline_Cycles;
			
			// update index
			iMFBufIndex++;
			
			// mask index
			iMFBufIndex &= c_lFlag_Delay_Mask;
		}

		inline void Update_MFBuffer ()
		{
			Get_MFBuffer ( 0 );
			Get_MFBuffer ( 1 );
			Get_MFBuffer ( 2 );
			Get_MFBuffer ( 3 );
		}

		
		inline void Reset_MFBuffer ()
		{
			MFBuffer [ 0 ].BusyUntil_Cycle = -1LL;
			MFBuffer [ 1 ].BusyUntil_Cycle = -1LL;
			MFBuffer [ 2 ].BusyUntil_Cycle = -1LL;
			MFBuffer [ 3 ].BusyUntil_Cycle = -1LL;
		}

		
		//inline u16* Get_SFBuffer_Ptr ()
		//{
		//	// want to update index before setting sticky flag because the lower instruction can still set the sticky flag and mask
		//	
		//	return (u16*) ( & SFBuffer [ iSFBufIndex ].StatusFlag );
		//}

		
		inline void Get_SFBuffer ( u32 Offset )
		{
			// want to update index before setting sticky flag because the lower instruction can still set the sticky flag and mask
			
			u32 iIndex;
			iIndex = ( iSFBufIndex + Offset ) & c_lFlag_Delay_Mask;
			if ( CycleCount >= SFBuffer [ iIndex ].BusyUntil_Cycle )
			{
				// update clip flag
				vi [ REG_STATUSFLAG ].u = ( vi [ REG_STATUSFLAG ].u & SFBuffer [ iIndex ].StatusMask ) | SFBuffer [ iIndex ].StatusFlag;
				
				// set to update far in the future since entry is emptied now
				SFBuffer [ iIndex ].BusyUntil_Cycle = -1LL;
			}
		}

		inline void Get_SFBuffer_Force ( u32 Offset )
		{
			// want to update index before setting sticky flag because the lower instruction can still set the sticky flag and mask
			
			u32 iIndex;
			iIndex = ( iSFBufIndex + Offset ) & c_lFlag_Delay_Mask;
			if ( SFBuffer [ iIndex ].BusyUntil_Cycle != -1LL )
			{
				// update clip flag
				vi [ REG_STATUSFLAG ].u = ( vi [ REG_STATUSFLAG ].u & SFBuffer [ iIndex ].StatusMask ) | SFBuffer [ iIndex ].StatusFlag;
				
				// set to update far in the future since entry is emptied now
				SFBuffer [ iIndex ].BusyUntil_Cycle = -1LL;
			}
		}


		inline void Set_SFBuffer ( u32 Value, u32 Mask )
		{
			// want to update index before setting sticky flag because the lower instruction can still set the sticky flag and mask

			SFBuffer [ iSFBufIndex ].StatusFlag = Value;
			SFBuffer [ iSFBufIndex ].StatusMask = Mask;
			
			// set cycle value is valid at
			SFBuffer [ iSFBufIndex ].BusyUntil_Cycle = CycleCount + c_ullFloatPipeline_Cycles;
			
			// update index
			iSFBufIndex++;
			
			// mask index
			iSFBufIndex &= c_lFlag_Delay_Mask;
		}

		inline void Update_SFBuffer ()
		{
			Get_SFBuffer ( 0 );
			Get_SFBuffer ( 1 );
			Get_SFBuffer ( 2 );
			Get_SFBuffer ( 3 );
		}

		
		inline void Reset_SFBuffer ()
		{
			SFBuffer [ 0 ].BusyUntil_Cycle = -1LL;
			SFBuffer [ 1 ].BusyUntil_Cycle = -1LL;
			SFBuffer [ 2 ].BusyUntil_Cycle = -1LL;
			SFBuffer [ 3 ].BusyUntil_Cycle = -1LL;
		}

		
		inline void Get_BFBuffer ( u32 Offset )
		{
			// want to update index before setting sticky flag because the lower instruction can still set the sticky flag and mask
			
			
			// update bitmap
			//vi [ REG_MACFLAG ].u = BFBuffer [ iBFBufIndex ].MACFlag;
			RemoveBitmap ( Pipeline_Bitmap, BFBuffer [ iBFBufIndex ].BFlags );
			
			// set to update far in the future since entry is emptied now
			if ( BFBuffer [ iBFBufIndex ].BusyUntil_Cycle > CycleCount )
			{
				CycleCount = BFBuffer [ iBFBufIndex ].BusyUntil_Cycle;
			}
			
			// clear the entry
			ClearBitmap ( BFBuffer [ iBFBufIndex ].BFlags );
			BFBuffer [ iBFBufIndex ].BusyUntil_Cycle = 0;
			
			iBFBufIndex++;
			iBFBufIndex &= c_lFlag_Delay_Mask;
		}
		

		inline void Set_BFBuffer ( Bitmap128 bm )
		{
			// want to update index before setting sticky flag because the lower instruction can still set the sticky flag and mask

			//BFBuffer [ iBFBufIndex ].MACFlag = Value;
			CombineBitmap ( Pipeline_Bitmap, bm );
			CombineBitmap ( BFBuffer [ iBFBufIndex ].BFlags, bm );
			
			// set cycle value is valid at
			BFBuffer [ iBFBufIndex ].BusyUntil_Cycle = CycleCount + c_ullFloatPipeline_Cycles;
			
			// update index
			iBFBufIndex++;
			
			// mask index
			iBFBufIndex &= c_lFlag_Delay_Mask;
		}
		
		inline void Reset_BFBuffer ()
		{
			BFBuffer [ 0 ].BusyUntil_Cycle = 0;
			BFBuffer [ 1 ].BusyUntil_Cycle = 0;
			BFBuffer [ 2 ].BusyUntil_Cycle = 0;
			BFBuffer [ 3 ].BusyUntil_Cycle = 0;
			
			ClearBitmap( BFBuffer [ 0 ].BFlags );
			ClearBitmap( BFBuffer [ 1 ].BFlags );
			ClearBitmap( BFBuffer [ 2 ].BFlags );
			ClearBitmap( BFBuffer [ 3 ].BFlags );
		}

		
		
		
		enum
		{
			RF_EMPTY = 0,
			RF_SET_MACSTAT = 1,
			RF_SET_STICKY = 2,
			RF_SET_CLIP = 3,
			RF_UPDATE_CLIP = 4,
			RF_UPDATE_BITMAPONLY = 5,
			RF_SET_MACSTICKY = 6
		};

		static const int c_lFlag_Delay = 4;
		static const int c_lFlag_Delay_Mask = c_lFlag_Delay - 1;
		
		u32 iFlagSave_Index;
		ResultFlags FlagSave [ c_lFlag_Delay ] ALIGN16;
		
		FloatLong NextQ, NextP;
		u16 NextQ_Flag;
		u64 QBusyUntil_Cycle, PBusyUntil_Cycle;
		
		
		// need to make the current instruction more accesible, so it can be seen if M bit is set or not more easily
		static Instruction::Format CurInstLO;
		static Instruction::Format CurInstHI;
		
		static Instruction::Format2 CurInstLOHI;
		
		// bitmap setting for the source registers needed for the current instruction
		Bitmap128 SrcRegs_Bitmap;
		u64 Int_SrcRegs_Bitmap;
		
		
		// looks like the vu needs to be cycle accurate, so must watch the pipeline closely
		// vu in micro mode has granularity at x,y,z,w but macro mode uses the whole register
		Bitmap128 Pipeline_Bitmap ALIGN16;
		
		// integer pipleline bitmap
		// since integer instructions may need to wait for integer registers to finish loading
		u64 Int_Pipeline_Bitmap;

		
		// recompiler bitmaps
		static Bitmap128 FSrcBitmap;
		static u64 ISrcBitmap;

		static Bitmap128 FDstBitmap;
		static u64 IDstBitmap;
		
		
		inline static void ClearBitmap ( Bitmap128& Bitmap )
		{
			Bitmap.b0 = 0;
			Bitmap.b1 = 0;
		}
		
		// create bitmap from one source register
		inline static void CreateBitmap ( Bitmap128& Bitmap, u64 xyzw64, u64 Source0 )
		{
			// get the xyzw field shifted
			//xyzw64 = ( Instruction >> 21 ) & 0xf;
			xyzw64 <<= ( Source0 << 2 ) & 0x3f;
			
			// set the bitmap
			if ( Source0 < 16 )
			{
				Bitmap.b0 = xyzw64;
				Bitmap.b1 = 0;
			}
			else
			{
				Bitmap.b0 = 0;
				Bitmap.b1 = xyzw64;
			}
		}

		// add bits to bitmap from 1 source register
		inline static void AddBitmap ( Bitmap128& Bitmap, u64 xyzw64, u64 Source0 )
		{
			// get the xyzw field
			//xyzw64 = ( Instruction >> 21 ) & 0xf;
			xyzw64 <<= ( Source0 << 2 ) & 0x3f;
			
			// set the bitmap
			if ( Source0 < 16 )
			{
				Bitmap.b0 |= xyzw64;
			}
			else
			{
				Bitmap.b1 |= xyzw64;
			}
		}
		
		
		// remove the bits in SrcBitmap from DstBitmap
		inline static void RemoveBitmap ( Bitmap128& DstBitmap, Bitmap128 SrcBitmap )
		{
			DstBitmap.b0 &= ~SrcBitmap.b0;
			DstBitmap.b1 &= ~SrcBitmap.b1;
		}

		inline static void CombineBitmap ( Bitmap128& DstBitmap, Bitmap128 SrcBitmap )
		{
			DstBitmap.b0 |= SrcBitmap.b0;
			DstBitmap.b1 |= SrcBitmap.b1;
		}
		
		
		// returns zero if all is clear, returns non-zero otherwise
		inline static u64 TestBitmap ( Bitmap128 Bitmap0, Bitmap128 Bitmap1 )
		{
			u64 Result64;
			
			Bitmap0.b0 &= Bitmap1.b0;
			Bitmap0.b1 &= Bitmap1.b1;
			
			// Exclude r0.xyzw (the shift right)
			Result64 = ( Bitmap0.b0 >> 4 ) | Bitmap0.b1;
			
			return Result64;
		}
		
		// test the pipeline bitmap against the source registers bitmap to check for a pipeline stall
		inline u64 TestStall ()
		{
			return TestBitmap ( Pipeline_Bitmap, SrcRegs_Bitmap );
		}

		inline u64 TestStall_INT ()
		{
			// test integer pipeline for a stall (like if the integer register is not loaded yet)
			return Int_Pipeline_Bitmap & Int_SrcRegs_Bitmap;
		}
		
		// will have to probably clear the old bitmap when it is removed from pipeline, so probably will not need this
		inline void ClearCurrentBitmap ()
		{
			ClearBitmap ( FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].Bitmap );
		}
		
		// adds a register into the current bitmap at the current position in the pipeline
		inline void AddCurrentBitmap ( u32 xyzw32, u32 Register )
		{
			AddBitmap ( FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].Bitmap, xyzw32, Register );
		}
		
		// set destination register in dest bitmap in FMAC pipeline
		inline void SetDest ( u32 Instruction, u32 Register )
		{
			AddBitmap ( FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].Bitmap, ( Instruction >> 21 ) & 0xf, Register );
		}
		
		// clear the dest bitmap, like for vu macro mode for now
		inline void ClearDest ()
		{
			ClearBitmap ( FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].Bitmap );
		}
		
		// remove dest bitmap from pipeline
		inline void RemoveDest ()
		{
			RemoveBitmap ( Pipeline_Bitmap, FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].Bitmap );
		}
		
		// create a source register bitmap from 1 register
		inline void CreateSrc1 ( Bitmap128& SrcBitmap, u32 Instruction, u32 Register0 )
		{
		}
		
		inline void Set_SrcReg ( u32 Instruction, u32 Register0 )
		{
			u32 xyzw32 = ( Instruction >> 21 ) & 0xf;
			CreateBitmap ( SrcRegs_Bitmap, xyzw32, Register0 );
		}

		inline void Set_SrcRegBC ( u32 Instruction, u32 RegisterBC )
		{
			CreateBitmap ( SrcRegs_Bitmap, 0x8 >> ( Instruction & 0x3 ), RegisterBC );
		}

		inline void Add_SrcRegBC ( u32 Instruction, u32 RegisterBC )
		{
			AddBitmap ( SrcRegs_Bitmap, 0x8 >> ( Instruction & 0x3 ), RegisterBC );
		}
		
		inline void Set_SrcRegs ( u32 Instruction, u32 Register0, u32 Register1 )
		{
			u32 xyzw32 = ( Instruction >> 21 ) & 0xf;
			CreateBitmap ( SrcRegs_Bitmap, xyzw32, Register0 );
			AddBitmap ( SrcRegs_Bitmap, xyzw32, Register1 );
		}

		inline void Set_SrcRegsBC ( u32 Instruction, u32 Register0, u32 RegisterBC )
		{
			u32 xyzw32 = ( Instruction >> 21 ) & 0xf;
			CreateBitmap ( SrcRegs_Bitmap, xyzw32, Register0 );
			AddBitmap ( SrcRegs_Bitmap, 0x8 >> ( Instruction & 0x3 ), RegisterBC );
		}


		inline void Set_Int_SrcReg ( u32 Register0 )
		{
			Int_SrcRegs_Bitmap = ( 1 << Register0 );
		}

		inline void Set_Int_SrcRegs ( u32 Register0, u32 Register1 )
		{
			Int_SrcRegs_Bitmap = ( 1 << Register0 ) | ( 1 << Register1 );
		}
		
		
		inline void AddPipeline ( u32 Instruction, u32 FloatReg0 )
		{
			u32 xyzw32 = ( Instruction >> 21 ) & 0xf;
			AddBitmap ( FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].Bitmap, xyzw32, FloatReg0 );
			AddBitmap ( Pipeline_Bitmap, xyzw32, FloatReg0 );
		}
		
		inline void RemovePipeline ()
		{
			RemoveBitmap ( Pipeline_Bitmap, FlagSave [ iFlagSave_Index & c_lFlag_Delay_Mask ].Bitmap );
		}
		
		// set the register that was modified by the current upper instruction
		// allows cancelling of lower instruction LQ,LQI,LQD,MFIR when it writes to same register as upper instruction, if it is needed
		// note: MUST be cleared before the next instruction is executed!!!
		inline void SetModifiedRegister ( u32 Register )
		{
			LastModifiedRegister = Register;
		}
		
		// clear the pipeline, like when execution is first started on VU
		inline void ClearPipeline ()
		{
			Int_Pipeline_Bitmap = 0;
			ClearBitmap ( Pipeline_Bitmap );
		}
		
		void PipelineWaitCycle ( u64 WaitUntil_Cycle );
		void PipelineWait_FMAC ();
		void PipelineWait_INT ();
		
		// force pipeline to wait for 1 register to be ready before executing Upper instruction
		//void PipelineWaitFMAC1 ( u32 UpperInstruction, u32 Register0 );

		// force pipeline to wait for 2 registers to be ready before executing Upper instruction
		//void PipelineWaitFMAC2 ( u32 UpperInstruction, u32 Register0, u32 Register1 );

		// force pipeline to wait for the Q register
		void PipelineWaitQ ();

		// force pipeline to wait for the P register
		void PipelineWaitP ();
		
		// whenever the pipeline advances, there's a bunch of stuff that must be updated
		// or at least theoretically
		// this will advance to the next cycle in VU
		void AdvanceCycle ();
		void MacroMode_AdvanceCycle ( u32 Instruction );
		
		
		// constructor
		VU ();

		
		void SetNextEvent ( u64 CycleOffset );
		void SetNextEvent_Cycle ( u64 Cycle );
		void Update_NextEventCycle ();

		
		// debug info
		static u32* DebugCpuPC;
		

		

		static const u32 c_InterruptCpuNotifyBit = 0;
		static const u32 c_InterruptBit_VIF = 4;
		static const u32 c_InterruptBit_VU = 6;

		static u32* _Intc_Stat;
		static u32* _Intc_Mask;
		static u32* _R5900_Status_12;
		static u32* _R5900_Cause_13;
		static u64* _ProcStatus;
		
		inline static void ConnectInterrupt ( u32* _IStat, u32* _IMask, u32* _R5900_Status, u32* _R5900_Cause, u64* _ProcStat )
		{
			_Intc_Stat = _IStat;
			_Intc_Mask = _IMask;
			_R5900_Cause_13 = _R5900_Cause;
			_R5900_Status_12 = _R5900_Status;
			_ProcStatus = _ProcStat;
		}
		
		
		
		inline void SetInterrupt_VIF ()
		{
			*_Intc_Stat |= ( 1 << ( c_InterruptBit_VIF + Number ) );
			if ( *_Intc_Stat & *_Intc_Mask ) *_R5900_Cause_13 |= ( 1 << 10 );
			
			// ***TODO*** more stuff needs to probably be checked on an R5900 to determine if interrupts are enabled or not
			// it probably doesn't matter, but should probably correct that
			if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0xff00 ) && ( *_R5900_Status_12 & 1 ) )
			{
				*_ProcStatus |= ( 1 << c_InterruptCpuNotifyBit );
			}
			else
			{
				*_ProcStatus &= ~( 1 << c_InterruptCpuNotifyBit );
			}
		}


		inline void SetInterrupt_VU ()
		{
			*_Intc_Stat |= ( 1 << ( c_InterruptBit_VU + Number ) );
			if ( *_Intc_Stat & *_Intc_Mask ) *_R5900_Cause_13 |= ( 1 << 10 );
			
			// ***TODO*** more stuff needs to probably be checked on an R5900 to determine if interrupts are enabled or not
			// it probably doesn't matter, but should probably correct that
			if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0xff00 ) && ( *_R5900_Status_12 & 1 ) )
			{
				*_ProcStatus |= ( 1 << c_InterruptCpuNotifyBit );
			}
			else
			{
				*_ProcStatus &= ~( 1 << c_InterruptCpuNotifyBit );
			}
		}
		
		
		/*
		inline void ClearInterrupt ()
		{
			//*_Intc_Master &= ~( 1 << c_InterruptBit );
			*_Intc_Stat &= ~( 1 << c_InterruptBit );
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 ); else *_ProcStatus &= ~( 1 << 20 );
		}

		inline void ClearInterrupt_Vsync ()
		{
			//*_Intc_Master &= ~( 1 << c_InterruptBit_Vsync );
			*_Intc_Stat &= ~( 1 << c_InterruptBit_Vsync );
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 ); else *_ProcStatus &= ~( 1 << 20 );
		}
		*/

		
		static u64* _NextSystemEvent;

		
		////////////////////////////////
		// Debug Info
		static u32* _DebugPC;
		static u64* _DebugCycleCount;
		static u32* _NextEventIdx;

		
#ifdef ENABLE_GUI_DEBUGGER
		static WindowClass::Window *DebugWindow [ c_iMaxInstances ];
		static DebugValueList<float> *GPR_ValueList [ c_iMaxInstances ];
		static DebugValueList<u32> *COP0_ValueList [ c_iMaxInstances ];
		static DebugValueList<u32> *COP2_CPCValueList [ c_iMaxInstances ];
		static DebugValueList<u32> *COP2_CPRValueList [ c_iMaxInstances ];
		static Debug_DisassemblyViewer *DisAsm_Window [ c_iMaxInstances ];
		static Debug_BreakpointWindow *Breakpoint_Window [ c_iMaxInstances ];
		static Debug_MemoryViewer *ScratchPad_Viewer [ c_iMaxInstances ];
		static Debug_BreakPoints *Breakpoints [ c_iMaxInstances ];
#endif


		static bool DebugWindow_Enabled [ c_iMaxInstances ];
		static void DebugWindow_Enable ( int Number );
		static void DebugWindow_Disable ( int Number );
		static void DebugWindow_Update ( int Number );

	};
	
	// ***TODO*** I should probably use a class template here instead
	// wrapper for static functions
	class VU0
	{
	public:
	
		static VU *_VU0;
		
		VU VU0;
		
		inline void Start ()
		{
			_VU0 = & VU0;
			
			VU0.Start ( 0 );
			//VU0.Number = 0;
		}
		
		inline void Reset () { VU0.Reset (); }
		
		inline static u64 Read ( u32 Address, u64 Mask ) { return _VU0->Read ( Address, Mask ); }
		inline static void Write ( u32 Address, u64 Data, u64 Mask ) { _VU0->Write( Address, Data, Mask ); }
		
		inline static void DMA_WriteBlock ( u64* Data, u32 QuadwordCount ) { _VU0->DMA_WriteBlock ( Data, QuadwordCount ); }
		inline static void DMA_ReadBlock ( u64* Data, u32 QuadwordCount ) { _VU0->DMA_ReadBlock ( Data, QuadwordCount ); }
		
		inline static bool DMA_Write_Ready () { return _VU0->DMA_Write_Ready (); }
		inline static bool DMA_Read_Ready () { return _VU0->DMA_Read_Ready (); }
		
		static void sRun () { _VU0->Run (); }
		static void Set_EventCallback ( funcVoid1 UpdateEvent_CB ) { _VU0->NextEvent_Idx = UpdateEvent_CB ( sRun ); };
	};
	
	
	class VU1
	{
	public:
	
		static VU *_VU1;
		
		VU VU1;
		
		inline void Start ()
		{
			_VU1 = & VU1;
			
			VU1.Start ( 1 );
			//VU1.Number = 1;
		}
		
		inline void Reset () { VU1.Reset (); }
		
		inline static u64 Read ( u32 Address, u64 Mask ) { return _VU1->Read ( Address, Mask ); }
		inline static void Write ( u32 Address, u64 Data, u64 Mask ) { _VU1->Write( Address, Data, Mask ); }
		
		inline static void DMA_WriteBlock ( u64* Data, u32 QuadwordCount ) { _VU1->DMA_WriteBlock ( Data, QuadwordCount ); }
		inline static void DMA_ReadBlock ( u64* Data, u32 QuadwordCount ) { _VU1->DMA_ReadBlock ( Data, QuadwordCount ); }
		
		inline static bool DMA_Write_Ready () { return _VU1->DMA_Write_Ready (); }
		inline static bool DMA_Read_Ready () { return _VU1->DMA_Read_Ready (); }
		
		static void sRun () { _VU1->Run (); }
		static void Set_EventCallback ( funcVoid1 UpdateEvent_CB ) { _VU1->NextEvent_Idx = UpdateEvent_CB ( sRun ); };
	};
}




#endif

