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



#include "PS1_Dma.h"
#include "PS1_SIO.h"
#include "R3000A.h"

#ifdef PS2_COMPILE
#include "PS2_SIF.h"
#include "PS1_SPU2.h"
#include "PS2_Dma.h"
#endif


using namespace Playstation1;


//#define DMA10_FILL_EXTRA_DATA
//#define DMA10_FILL_EXTRA_ZERO
#define DMA10_FILL_EXTRA_NONE

//#define ENABLE_SIF_DMA_TIMING
//#define ENABLE_SIF_DMA_SYNC

#define ENABLE_AUTODMA
//#define DMA_UPDATE_PREVIOUS
//#define VERBOSE_AUTODMA_START


//#define ENABLE_DMA_END_REMOTE

// this one requires 0xc0000000 to end dma9 (IOP->EE)
//#define REQUIRE_FULLSTOP_DMA9

// this one requires 0xc0000000 to end dma10 (EE->IOP)
//#define REQUIRE_FULLSTOP_DMA10

//#define ENABLE_ICR2_BIT31


//#define INT_TRANSITION_ONLY


// sets the interrupt pending flag in ICR/ICR2 only if the interrupt is enabled
#define SET_INTPENDING_ONLY_WHEN_ENABLED


// this allows a transfer with a higher priority to interrupt a transfer with a lower priority
// note: this appears to be the correct operation
#define ALLOW_TRANSFER_INTERRUPTION

// clears bit 30 in CHCR when dma transfer finishes
#define CLEAR_BIT30_ON_FINISH


// outputs alert to console when multiple4 issues are encountered across SIF
//#define VERBOSE_DMA9_MULTIPLE4
#define VERBOSE_DMA10_MULTIPLE4

// determines whether to round up or down when multiple4 issue is encountered
#define DMA9_ROUNDUP_128
//#define DMA10_ROUNDUP_128


// does not release bus to CPU during Auto-DMA transfer if enabled
//#define ENABLE_CONTINUOUS_ADMA


// performs an arbitration when a transfer is not ready to start
//#define ENABLE_ARBITRATE_ON_NOTREADY


// allows "Run" function to assign a new active channel
//#define ALLOW_NEW_ACTIVE_CHANNEL_ONRUN

// one of these (and only one) MUST be defined
//#define ENABLE_PRIORITY_CHOP
#define ENABLE_REVERSE_PRIORITY_CHOP
//#define DISABLE_PRIORITY_CHOP


// calculates priority offset
//#define ENABLE_PRIORITY_OFFSET



#define ENABLE_MEMORY_INVALIDATE



#define USE_NEW_PS1_DMA_RUN



#ifdef _DEBUG_VERSION_

// enable debugging

#define INLINE_DEBUG_ENABLE

/*
#define INLINE_DEBUG_WRITE 
#define INLINE_DEBUG_COMPLETE
#define INLINE_DEBUG_READ

#define INLINE_DEBUG_AUTODMA
#define INLINE_DEBUG_RUN_AUTODMA

#define INLINE_DEBUG_RUN_DMA3

#define INLINE_DEBUG_RUN_DMA9
#define INLINE_DEBUG_RUN_DMA10

#define INLINE_DEBUG_TRANSFER


#define INLINE_DEBUG_TRANSFER_TOMEM_0
#define INLINE_DEBUG_TRANSFER_TOMEM_1
#define INLINE_DEBUG_TRANSFER_TOMEM_2
#define INLINE_DEBUG_TRANSFER_TOMEM_3
#define INLINE_DEBUG_TRANSFER_FROMMEM
#define INLINE_DEBUG_TRANSFER_BOTTOM


#define INLINE_DEBUG_TRANSFER_BLOCK

#define INLINE_DEBUG_READY
#define INLINE_DEBUG_ARBITRATE


//#define INLINE_DEBUG_EVENT


//#define INLINE_DEBUG_WRITE_MADR
//#define INLINE_DEBUG_WRITE_BCR
//#define INLINE_DEBUG_WRITE_CHCR
//#define INLINE_DEBUG_WRITE_CHCR_0
//#define INLINE_DEBUG_WRITE_CHCR_1
//#define INLINE_DEBUG_WRITE_CHCR_2
//#define INLINE_DEBUG_WRITE_CHCR_3
//#define INLINE_DEBUG_WRITE_CHCR_4
//#define INLINE_DEBUG_WRITE_CHCR_5
//#define INLINE_DEBUG_WRITE_CHCR_6

// ??? ***TODO*** fix this - fixed
//#define INLINE_DEBUG_WRITE_CHCR_7

//#define INLINE_DEBUG_WRITE_TADR


//#define INLINE_DEBUG_WRITE_ICR
//#define INLINE_DEBUG_WRITE_PCR
//#define INLINE_DEBUG_WRITE_ICR2
//#define INLINE_DEBUG_WRITE_PCR2
//#define INLINE_DEBUG_WRITE_REG1578
//#define INLINE_DEBUG_WRITE_SIF0CTRL
//#define INLINE_DEBUG_WRITE_SIF1CTRL
//#define INLINE_DEBUG_WRITE_SIF2CTRL

//#define INLINE_DEBUG_UPDATE_ICR


//#define INLINE_DEBUG_RUN_DMA11
//#define INLINE_DEBUG_RUN_DMA12



//#define INLINE_DEBUG_WRITE_DMA2
//#define INLINE_DEBUG_RUN_DMA2
//#define INLINE_DEBUG_RUN_DMA2_CO


//#define INLINE_DEBUG_WRITE_PCR
//#define INLINE_DEBUG_READ_PCR


//#define INLINE_DEBUG
//#define INLINE_DEBUG_RUN_DMA0
//#define INLINE_DEBUG_RUN_DMA1
//#define INLINE_DEBUG_RUN_DMA3
//#define INLINE_DEBUG_RUN_DMA6
#define INLINE_DEBUG_RUN_DMA4
#define INLINE_DEBUG_RUN_DMA7
#define INLINE_DEBUG_RUN
//#define INLINE_DEBUG_CD
//#define INLINE_DEBUG_SPU
//#define INLINE_DEBUG_ACK


//#define INLINE_DEBUG_DMARUN

//#define INLINE_DEBUG_RUN_DMA11_OUTPUT
//#define INLINE_DEBUG_RUN_DMA12_OUTPUT
*/

#endif


DmaChannel::RegData* Dma::pRegData [ Dma::c_iNumberOfChannels ];


funcVoid Dma::UpdateInterrupts;

u32* Dma::_DebugPC;
u64* Dma::_DebugCycleCount;
u64* Dma::_SystemCycleCount;
u32* Dma::_NextEventIdx;

u32* Dma::_R3000a_Status;

//u32* Dma::_Intc_Master;
u32* Dma::_Intc_Stat;
u32* Dma::_Intc_Mask;
u32* Dma::_R3000A_Cause_13;
u32* Dma::_R3000A_Status_12;
u64* Dma::_ProcStatus;


Debug::Log Dma::debug;

volatile Dma *Dma::_DMA;

bool Dma::DebugWindow_Enabled;
WindowClass::Window *Dma::DebugWindow;
DebugValueList<u32> *Dma::DMA_ValueList;



int DmaChannel::Count = 0;


u64* Dma::_NextSystemEvent;


DataBus *Dma::_BUS;
MDEC *Dma::_MDEC;
GPU *Dma::_GPU;
CD *Dma::_CD;
SPU *Dma::_SPU;
//R3000A::Cpu *Dma::_CPU;



const char* Dma::Reg0_Names [ 8 * 4 ] = { "DMA0_MADR", "DMA0_BCR", "DMA0_CHCR", "DMA0_CHCRm",
											"DMA1_MADR", "DMA1_BCR", "DMA1_CHCR", "DMA2_CHCRm",
											"DMA2_MADR", "DMA2_BCR", "DMA2_CHCR", "DMA3_CHCRm",
											"DMA3_MADR", "DMA3_BCR", "DMA3_CHCR", "DMA4_CHCRm",
											"DMA4_MADR", "DMA4_BCR", "DMA4_CHCR", "DMA5_CHCRm",
											"DMA5_MADR", "DMA5_BCR", "DMA5_CHCR", "DMA6_CHCRm",
											"DMA6_MADR", "DMA6_BCR", "DMA6_CHCR", "DMA7_CHCRm",
											"PCR", "ICR", "Unknown0", "Unknown1" };
													
const char* Dma::Reg1_Names [ 8 * 4 ] = { "DMA7_MADR", "DMA7_BCR", "DMA7_CHCR", "DMA7_TADR",
											"DMA8_MADR", "DMA8_BCR", "DMA8_CHCR", "DMA8_TADR",
											"DMA9_MADR", "DMA9_BCR", "DMA9_CHCR", "DMA9_TADR",
											"DMA10_MADR", "DMA10_BCR", "DMA10_CHCR", "DMA10_TADR",
											"DMA11_MADR", "DMA11_BCR", "DMA11_CHCR", "DMA11_TADR",
											"DMA12_MADR", "DMA12_BCR", "DMA12_CHCR", "DMA12_TADR",
											"SIF0_CTRL", "SIF1_CTRL", "SIF2_CTRL", "Unknown2",
											"PCR2", "ICR2", "REG_1578", "Unknown3" };



const u64 Dma::c_ullSetupTime [ c_iNumberOfChannels ] = {
0,	// dma#0
0,	// dma#1
0,	// dma#2
0,	// dma#3
0,	// dma#4
0,	// dma#5
0,	// dma#6
#ifdef PS2_COMPILE
0,	// dma#7
0,	// dma#8
0,	// dma#9
0,	// dma#10
0,	// dma#11
0,	// dma#12
#endif
};

const u64 Dma::c_ullAccessTime [ c_iNumberOfChannels ] = {
1,	// dma#0 - MDECin
1,	// dma#1 - MDECout
1,	// dma#2 - GPU
#ifdef PS2_COMPILE
1,	// 4,	// dma#3 - CDVD (but on PS2 this is probably shorter?)
#else
24,	// dma#3 - CD
#endif
4,	// dma#4 - SPU
1,	// dma#5 - PIO
1,	// dma#6 - OTC
#ifdef PS2_COMPILE
4,	// dma#7 - SPU2
1,	// dma#8 - DEV9
1,	// dma#9 - SIF0 (IOP->EE)
1,	// dma#10 - SIF1 (EE->IOP)
1,	// dma#11 - SIO2in
1,	// dma#12 - SIO2out
#endif
};


const u64 Dma::c_ullReleaseTime [ c_iNumberOfChannels ] = {
1,	// dma#0 - MDECin
1,	// dma#1 - MDECout
0,	// dma#2 - GPU
#ifdef PS2_COMPILE
0,	// dma#3 - CDVD
#else
1,	// dma#3 - CD
#endif
16,	// dma#4 - SPU
1,	// dma#5 - PIO
1,	// dma#6 - OTC
#ifdef PS2_COMPILE
16,	// dma#7 - SPU2
1,	// dma#8 - DEV9
-1ULL, //1,	// dma#9 - SIF0 (IOP->EE)
-1ULL,	// dma#10 - SIF1 (EE->IOP)
1,	// dma#11 - SIO2in
1,	// dma#12 - SIO2out
#endif
};


// these are the pointers to the functions to call for each device to see if it is ready for a dma transfer or not
Dma::fnReady Dma::cbReadyForRead [ c_iNumberOfChannels ] = {
NULL,	// dma#0 - MDECin
MDEC::DMA_ReadyForRead,	// dma#1 - MDECout
GPU::DMA_ReadyForRead,	// dma#2 - GPU
#ifdef PS2_COMPILE
CDVD::DMA_ReadyForRead,	// dma#3 - CDVD (PS2)
#else
CD::DMA_ReadyForRead,	// dma#3 - CD
#endif
#ifdef PS2_COMPILE
SPU2::DMA_ReadyForRead_Core0,	// dma#4 - SPU2
#else
SPU::DMA_ReadyForRead,	// dma#4 - SPU
#endif
NULL,	// dma#5 - PIO
Dma::OTC_ReadyForRead,	// dma#6 - OTC
#ifdef PS2_COMPILE
// *** TODO *** problem here because what if SPU core 1 is ready but not core 2??
SPU2::DMA_ReadyForRead_Core1,	// dma#7 - SPU2
NULL,	// dma#8 - DEV9
NULL,	//Playstation2::SIF::_SIF->EE_DMA_In_Ready,	// dma#9 - SIF0 (IOP->EE)
Playstation2::SIF::EE_DMA_Out_Ready,	// dma#10 - SIF1 (EE->IOP)
NULL,	//Playstation1::SIO::_SIO->SIO2in_DMA_Ready,	// dma#11 - SIO2in
SIO::SIO2out_DMA_Ready,	// dma#12 - SIO2out
#endif
};


Dma::fnReady Dma::cbReadyForWrite [ c_iNumberOfChannels ] = {
MDEC::DMA_ReadyForWrite,	// dma#0 - MDECin
NULL,	// dma#1 - MDECout
GPU::DMA_ReadyForWrite,	// dma#2 - GPU
#ifdef PS2_COMPILE
NULL,	// dma#3 - CDVD (PS2)
#else
NULL,	// dma#3 - CD
#endif
#ifdef PS2_COMPILE
SPU2::DMA_ReadyForWrite_Core0,	// dma#4 - SPU2
#else
SPU::DMA_ReadyForWrite,	// dma#4 - SPU
#endif
NULL,	// dma#5 - PIO
NULL,	// dma#6 - OTC
#ifdef PS2_COMPILE
// *** TODO *** problem here because what if SPU core 1 is ready but not core 2??
SPU2::DMA_ReadyForWrite_Core1,	// dma#7 - SPU2
NULL,	// dma#8 - DEV9
Playstation2::SIF::EE_DMA_In_Ready,	// dma#9 - SIF0 (IOP->EE)
NULL,	//Playstation2::SIF::_SIF->EE_DMA_Out_Ready,	// dma#10 - SIF1 (EE->IOP)
SIO::SIO2in_DMA_Ready,	//Playstation1::SIO::_SIO->SIO2in_DMA_Ready,	// dma#11 - SIO2in
NULL,	// dma#12 - SIO2out
#endif
};

// functions for writing to devices from memory
Dma::fnTransfer_FromMemory Dma::cbTransfer_FromMemory [ c_iNumberOfChannels ] = {
MDEC::DMA_WriteBlock,	// dma#0 - MDECin
NULL,	// dma#1 - MDECout
GPU::DMA_WriteBlock,	// dma#2 - GPU
#ifdef PS2_COMPILE
NULL,	// dma#3 - CDVD (PS2)
#else
NULL,	// dma#3 - CD
#endif
#ifdef PS2_COMPILE
SPU2::DMA_WriteBlock_Core0,	// dma#4 - SPU core 0
#else
SPU::DMA_WriteBlock,	// dma#4 - SPU
#endif
NULL,	// dma#5 - PIO
NULL,	// dma#6 - OTC
#ifdef PS2_COMPILE
SPU2::DMA_WriteBlock_Core1,	// dma#7 - SPU2
NULL,	// dma#8 - DEV9
Playstation2::SIF::IOP_DMA_WriteBlock,	// dma#9 - SIF0 (IOP->EE)
NULL,	// dma#10 - SIF1 (EE->IOP)
SIO::DMA_WriteBlock,	// dma#11 - SIO2in
NULL,	// dma#12 - SIO2out
#endif
};

// functions for reading from devices to memory
Dma::fnTransfer_FromMemory Dma::cbTransfer_ToMemory [ c_iNumberOfChannels ] = {
NULL,	// dma#0 - MDECin
MDEC::DMA_ReadBlock,	// dma#1 - MDECout
GPU::DMA_ReadBlock,	// dma#2 - GPU
#ifdef PS2_COMPILE
CDVD::DMA_ReadBlock,	// dma#3 - CDVD (PS2)
#else
CD::DMA_ReadBlock,	// dma#3 - CD
#endif
#ifdef PS2_COMPILE
SPU2::DMA_ReadBlock_Core0,	// dma#4 - SPU core 0
#else
SPU::DMA_ReadBlock,	// dma#4 - SPU
#endif
NULL,	// dma#5 - PIO
Dma::OTC_Transfer,	// dma#6 - OTC
#ifdef PS2_COMPILE
SPU2::DMA_ReadBlock_Core1,	// dma#7 - SPU2
NULL,	// dma#8 - DEV9
NULL,	// dma#9 - SIF0 (IOP->EE)
Playstation2::SIF::IOP_DMA_ReadBlock,	// dma#10 - SIF1 (EE->IOP)
NULL,	// dma#11 - SIO2in
SIO::DMA_ReadBlock,	// dma#12 - SIO2out
#endif
};


Dma::Dma ()
{
	cout << "Running DMA constructor...\n";

/*	
#ifdef INLINE_DEBUG_ENABLE
	debug.Create ( "DMAController_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering DMA controller constructor";
#endif


	// set the current dma object
	_DMA = this;

	Reset ();
	
	
#ifdef INLINE_DEBUG
	debug << "->Exiting DMA controller constructor";
#endif
*/

}


void Dma::Start ()
{
	cout << "Running DMA::Start...\n";

#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create ( "DMAController_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering DMA::Start";
#endif

	// set the current dma object
	_DMA = this;

	Reset ();
	
	
	// set all events to far in the future initially
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		SetNextEventCh_Cycle ( -1ULL, i );
	}
	
	
#ifdef INLINE_DEBUG
	debug << "->Exiting DMA::Start";
#endif
}



void Dma::Reset ()
{
	// zero object
	memset ( this, 0, sizeof( Dma ) );
	
	// allow all dma channels to run
	SelectedDMA_Bitmap = 0xffffffff;
	
	// no dma channels are active
	ActiveChannel = -1;
	
	// fix the non-static pointers
	Refresh ();
}

void Dma::ConnectDevices ( DataBus *BUS, MDEC* mdec, GPU *g, CD *cd, SPU *spu /* , R3000A::Cpu *cpu */ )
{
	_BUS = BUS;
	_MDEC = mdec;
	_GPU = g;
	_CD = cd;
	_SPU = spu;
	//_CPU = cpu;
}







static u64 Dma::OTC_ReadyForRead ()
{
	// dma#6 is always immediately ready
	return 1;
}


// pMemPtr is a pointer to the start of RAM device
static u32 Dma::OTC_Transfer ( u32* pMemPtr, u32 Address, u32 Count )
{
	u32 CurrAddress, PrevAddress, TransferAmt;
	
	// set the amount to transfer
	TransferAmt = Count;
	
	// transfer the data
	while ( TransferAmt )
	{
		// get the previous address and wrap around
		PrevAddress = ( Address - 4 ) & DataBus::MainMemory_Mask;
		
		// get the current address and wrap around
		CurrAddress = Address & DataBus::MainMemory_Mask;
		
		pMemPtr [ CurrAddress >> 2 ] = PrevAddress;
		
		// decrease the address by 4
		Address -= 4;
		
		// decrease the counter
		TransferAmt--;
	}
	
	// overwrite the last entry with the ordering list terminator
	pMemPtr [ CurrAddress >> 2 ] = 0x00ffffff;
	
	// set BCR to zero on one-shot transfer
	// not here
	
	// MADR does not change on one-shot transfer
	// not here
	
	// return the amount transferred
	return Count;
}




#ifdef PS2_COMPILE

/*
void Dma::AutoDMA_Run ( int iChannel )
{
#ifdef INLINE_DEBUG_AUTODMA
	debug << "\r\n";
	debug << "; AUTODMA: SPU";
	debug << "; Channel#" << iChannel;
	debug << "; (before) BA=" << dec << DmaCh [ iChannel ].BCR.BA << " BS=" << DmaCh [ iChannel ].BCR.BS;
#endif

	// check that transfer is ok
	
	// check that core is in Auto DMA mode
	if ( iChannel == 4 )
	{
		// dma channel #4
		
		if ( !SPU2::_SPU2->SPU0.isADMATransferMode () )
		{
			return;
		}
		
	}
	else
	{
		// dma channel #7
		
		if ( !SPU2::_SPU2->SPU1.isADMATransferMode () )
		{
			return;
		}
	}
	
#ifdef INLINE_DEBUG_AUTODMA
	debug << "; ENABLED";
#endif

	// check that dma channel enabled
	if ( !DmaCh [ iChannel ].pRegData->CHCR.TR )
	{
		return;
	}
	
	// check that transfer is from memory
	if ( !DmaCh [ iChannel ].pRegData->CHCR.DR )
	{
		cout << "\nhps2x64: SPU2: ALERT: AutoDma transfer from device to memory!";
		return;
	}
	
	// check that there is data to transfer
	if ( DmaCh [ iChannel ].pRegData->BCR.BA && DmaCh [ iChannel ].pRegData->BCR.BS )
	{
		// reserve bus
		_BUS->ReserveBus ( c_iAutoDmaTransfer_CycleTime );
		
		if ( iChannel == 4 )
		{
			// dma channel #4
			
			// transfer 256 samples L/R (512 samples total)
			// transfer size is in words, but samples are halfword each, so words to transfer is 512/2
			SPU2::_SPU2->SPU0.DMA_Write_Block ( & ( _BUS->MainMemory.b32 [ ( DmaCh [ iChannel ].MADR & 0x1fffff ) >> 2 ] ), 512 >> 1 );
		}
		else
		{
			// dma channel #7
			
			// transfer 256 samples L/R (512 samples total)
			// transfer size is in words, but samples are halfword each, so words to transfer is 512/2
			SPU2::_SPU2->SPU1.DMA_Write_Block ( & ( _BUS->MainMemory.b32 [ ( DmaCh [ iChannel ].MADR & 0x1fffff ) >> 2 ] ), 512 >> 1 );
		}
		
		// update BA and MADR
		// 512 halfwords transferring at a time for now
		DmaCh [ iChannel ].MADR += ( 512 << 1 );
		
		//////////////////////////////////////////////////////////
		// Decrease Block Count
		// BS is usually 16, so transfer is usually 16 words which is 32 samples, divided by 512 samples is 16 blocks
		DmaCh [ iChannel ].BCR.BA -= 16;
		
#ifdef INLINE_DEBUG_AUTODMA
	debug << "; (after) BA=" << dec << DmaCh [ iChannel ].BCR.BA << " BS=" << DmaCh [ iChannel ].BCR.BS;
#endif

		// still testing for now
		if ( DmaCh [ iChannel ].BCR.BS != 16 )
		{
			cout << "\nhps1x64: ALERT: AutoDMA transfer with BS not equal to 16!! BS=" << dec << DmaCh [ iChannel ].BCR.BS;
		}
	}
	
	// check if transfer is complete
	if ( !DmaCh [ iChannel ].BCR.BA )
	{
#ifdef INLINE_DEBUG_AUTODMA
	debug << "; COMPLETEINT";
#endif

		// transfer is complete
		DMA_Finished ( iChannel );
		
		if ( iChannel == 4 )
		{
			// ??? clear ADMAS ???
			//SPU2::_SPU2->SPU0.pCoreRegs0->ADMAS = 0;
		}
		else
		{
			//SPU2::_SPU2->SPU1.pCoreRegs0->ADMAS = 0;
		}
	}
	
}

// the Auto-DMA gets called from SPU
void Dma::AutoDMA4_Run ()
{
	AutoDMA_Run ( 4 );
}



// the Auto-DMA gets called from SPU
void Dma::AutoDMA7_Run ()
{
	AutoDMA_Run ( 7 );
}

*/


#endif


/*
u32* Dma::DMA0_ReadBlock ()
{
#ifdef INLINE_DEBUG_RUN_DMA0
	debug << "\r\nDMA0_ReadBlock";
	debug << "; DmaCh [ 0 ].BCR.BA=" << hex << DmaCh [ 0 ].BCR.BA << " isEnabledAndActive( 0 )=" << isEnabledAndActive ( 0 );
#endif

	u32 NumberOfTransfers;
	u32* DataOut;
	
	DataOut = 0;
	
	if ( DmaCh [ 0 ].BCR.BA && isEnabledAndActive ( 0 ) )
	{
#ifdef INLINE_DEBUG_RUN_DMA0
	debug << "; DMA0READINGBLOCK";
#endif

		//NumberOfTransfers = DmaCh [ 0 ].BCR.BA * DmaCh [ 0 ].BCR.BS;
		//NumberOfTransfers = ( DmaCh [ 0 ].BCR.BA + 1 ) * DmaCh [ 0 ].BCR.BS;
		NumberOfTransfers = DmaCh [ 0 ].BCR.BS;
		
		//cout << "\nTEST: DMA0 BS=" << hex << DmaCh [ 0 ].BCR.BS;
		
		// update BusyCycles - only transfer BCR.BS cycles
		// transferring all the data at the start of frame does not appear to be a problem yet. Will fix later
		_BUS->ReserveBus ( NumberOfTransfers );
		
		BusyCycles = NumberOfTransfers + 2;
		//SetNextEventCh ( BusyCycles, 0 );
			
		// *** TODO *** since this is probably a continuous transfer will need to load all the data at once
		// *note* not a continuous transfer. only transfers data when MDEC requests
		//_MDEC->DMA_Write ( DmaCh [ 0 ].MADR, NumberOfTransfers, _BUS->MainMemory.b32 );
		DataOut = & ( _BUS->MainMemory.b32 [ ( DmaCh [ 0 ].MADR & 0x1fffff ) >> 2 ] );
		
		// update MADR
		//DmaCh [ 0 ].MADR += ( ( DmaCh [ 0 ].BCR.BA * DmaCh [ 0 ].BCR.BS ) << 2 );
		DmaCh [ 0 ].MADR += ( ( DmaCh [ 0 ].BCR.BS ) << 2 );
		
		//cout << "\nTEST: DMA0 MADR=" << hex << DmaCh [ 0 ].MADR;

		// update BA
		DmaCh [ 0 ].BCR.BA--;
		
		//cout << "\nTEST: DMA0 BA=" << hex << DmaCh [ 0 ].BCR.BA;
		
	}
	
#ifdef INLINE_DEBUG_RUN_DMA0
	debug << "; DataOut=" << hex << (u64) DataOut;
#endif

	return DataOut;
}
*/



#ifdef PS2_COMPILE




// returns 1 when transfer is complete, zero otherwise
void Dma::DMA10_WriteBlock ( u32* Data, u32 WordCount )
{
#ifdef INLINE_DEBUG_RUN_DMA10
	debug << "\r\nDMA10_WriteBlock: SIF1 EE->IOP";
	debug << "; WordCount=" << dec << WordCount;
	//debug << "; WC=" << dec << Data [ 1 ];
#endif

	// dma transfer has been started //
	
	u64 ullCycles, ullAccessTime;
	
	u32 Temp;
	//u32 Data [ 4 ];
	u32 NumberOfTransfers;
	
	u32 Data0, Data1, DestAddress, IRQ, ID;
	u32 WC = 0;
	
	// this cycle delay should be taken care of at the source of the transfer
	static u64 CycleDelay = 0;
	
	// dma should not be acknowledged until it is actually transferred
	if ( CycleDelay > *_DebugCycleCount )
	{
		CycleDelay += c_llSIFDelayPerWC * WordCount;
	}
	else
	{
		CycleDelay = *_DebugCycleCount + ( c_llSIFDelayPerWC * WordCount );
	}


	// if in destination chain mode, then pull tag and set address first
	if ( pRegData [ 10 ]->CHCR.ChainTransferMode /* && DmaCh [ 10 ].CHCR.DestinationChainMode */ )
	{
#ifdef INLINE_DEBUG_RUN_DMA10
	debug << "; DestinationChainMode";
#endif

		// ONLY read tag if DMA Channel is NOT expecting data
		if ( !DmaCh [ 10 ].WordsRemaining )
		{
#ifdef INLINE_DEBUG_RUN_DMA10
	debug << "\r\nReadingIOPDestTag";
#endif

			Data0 = *Data++;
			Data1 = *Data++;
			
		
#ifdef INLINE_DEBUG_RUN_DMA10
	debug << "; IOPDestTag=" << hex << Data0 << " " << Data1;
#endif

			// rest of quadword is ignored
			Data++;
			Data++;
			
			// this part has the MADR
			pRegData [ 10 ]->MADR = Data0 & 0x1fffff;
			DmaCh [ 10 ].LastIOPTag = Data0;
			
			// check if number of words is not multiple of 4
			if ( Data1 & 3 )
			{
#ifdef INLINE_DEBUG_RUN_DMA10
	debug << " WC-NOT-MULTIPLE4";
	debug << " (hex)WC=" << hex << Data1;
#endif

#ifdef VERBOSE_DMA10_MULTIPLE4
				// need to investigate ?? //
				cout << "\nhps1x64: DMA10: ALERT: WC not multiple of 4. (hex)WC=" << hex << Data1;
#endif
			}
			
#ifdef DMA10_ROUNDUP_128
			// ***TESTING*** round number of words UP to next 128-bit boundary
			Data1 = ( Data1 + 3 ) & 0xffffc;
#else
			// ***TESTING*** round number of words DOWN to next 128-bit boundary
			Data1 &= 0xffffc;
#endif
			
			// this part has the amount of data being transfered to IOP
			DmaCh [ 10 ].WordsRemaining = Data1;
			
			// fix the words remaining so that it is a multiple of 4
			DmaCh [ 10 ].WordsRemainingMultiple4 = ( Data1 + 3 ) & ~3;
			
			/*
			if ( Data1 & 0x3 )
			{
				DmaCh [ 10 ].WordsRemainingMultiple4 = ( Data1 + 3 ) & ~3;
			}
			else
			{
				DmaCh [ 10 ].WordsRemainingMultiple4 = ( Data1 + 3 ) & ~3;
			}
			*/
			
			// subtract 1 quadword (4 words) due to the IOP Tag
			// subract only from the total amount of data sent to IOP from EE
			WordCount -= 4;
			
			// subtract the quadword from the words remaining too ??
			
#ifdef INLINE_DEBUG_RUN_DMA10
	debug << "; DMA10_MADR=" << hex << pRegData [ 10 ]->MADR << " WordsToTransfer=" << DmaCh [ 10 ].WordsRemaining << "\r\n";
#endif
		}
		else
		{
#ifdef INLINE_DEBUG_RUN_DMA10
	debug << "\r\nCONTINUEDTRANSFER\r\n";
#endif
		}

	}
	
//#ifdef INLINE_DEBUG_RUN_DMA10
//	debug << "; BUS_FREE";
//#endif

	// check if words remaining turns negative
	if ( DmaCh [ 10 ].WordsRemaining < 0 )
	{
		cout << "\nhp1x64: ***ALERT***: DMA10.WordsRemaining is negative!\n";
	}

		// bus is free //
		
		// set the number of transfers to make
		NumberOfTransfers = WordCount;
		
		if ( !NumberOfTransfers )
		{
			if ( !DmaCh [ 10 ].WordsRemaining )
			{
#ifdef INLINE_DEBUG_RUN_DMA10
	debug << "\r\nTRANSFER_SIZE_IS_ZERO\r\n";
#endif

				cout << "\nhps1x64 ALERT: DMA 10 (SIF1 EE->IOP): Transfer size is zero.\n";
			}
			
			// must have read tag but not data yet
			//return;
			return 0;
		}
		
		// step 5: get the access time
		ullAccessTime = c_ullAccessTime [ 10 ];
		
		////////////////////////////////////
		// mark bus as in use this cycle
		//_BUS->ReserveBus ( WordCount );
		// update CPU cycles
		ullCycles = WordCount * ullAccessTime;
		R3000A::Cpu::_CPU->CycleCount += ullCycles;
		
		//BusyCycles = DmaCh [ 6 ].BCR.Value + 2;
		BusyCycles = WordCount + 2;
		//SetNextEventCh ( BusyCycles, 6 );
		

#ifdef INLINE_DEBUG_RUN_DMA10
	debug << "; STORE";
#endif

		// *** do entire dma 10 transfer at once *** //
		
		Temp = pRegData [ 10 ]->MADR;
		
#ifdef INLINE_DEBUG_RUN_DMA10
	debug << "\r\n***DMA#10 SIF1 (EE->IOP) Writing WC=" << hex << WordCount << " to MADR=" << pRegData [ 10 ]->MADR << " Cycle#" << dec << *_DebugCycleCount << "\r\n";
	debug << dec << " PS2Cycle#" << *Playstation2::SIF::_DebugCycleCount;
#endif

		if ( WordCount > 0 )
		{

			// transfer the smaller value ??
			//WC = ( WordCount < DmaCh [ 10 ].WordsRemaining ) ? WordCount : DmaCh [ 10 ].WordsRemaining;
			WC = WordCount;
			
#ifdef INLINE_DEBUG_RUN_DMA10
	debug << "\r\nLeft(WC)=" << dec << DmaCh [ 10 ].WordsRemaining;
	debug << "\r\nTransferring(WC)=" << dec << WC;
#endif

#ifdef ENABLE_MEMORY_INVALIDATE
			_BUS->InvalidateRange ( pRegData [ 10 ]->MADR, WC );
#endif

			//for ( int i = 0; i < WordCount; i++ )
			for ( int i = 0; i < WC; i++ )
			{
				///////////////////////////////////////////////
				// Send previous address entry to main memory
				_BUS->MainMemory.b32 [ ( pRegData [ 10 ]->MADR & 0x1fffff ) >> 2 ] = *Data++;
				
				
				// invalidate i-cache on transfer from SIF ??
				R3000A::Cpu::_CPU->InvalidateCache ( pRegData [ 10 ]->MADR & 0x1fffff );
				
				
				////////////////////////////////////////////////////
				// update address
				// ***todo*** this is a chain transfer, so maybe MADR should stay at the first address in block?
				pRegData [ 10 ]->MADR += 4;
				
				
				////////////////////////////////////////////
				// decrease count
				//DmaCh [ 10 ].BCR.Value--;
				//NumberOfTransfers--;
			}
			
		}
		
		// subtract from words remaining
		//DmaCh [ 10 ].WordsRemaining -= WordCount;
		DmaCh [ 10 ].WordsRemaining -= WC;
		DmaCh [ 10 ].WordsRemainingMultiple4 -= WC;

#ifdef INLINE_DEBUG_RUN_DMA10
	debug << "\r\nRemaining(WC)=" << dec << DmaCh [ 10 ].WordsRemaining;
#endif

		
//#ifdef INLINE_DEBUG_RUN_DMA10
//	debug << "; FINISHED";
//#endif

		////////////////////////////////////////////
		// zero count
		//DmaCh [ 10 ].BCR.Value = 0;
		
		////////////////////////////////////////////////
		// MADR does not change on one-shot transfer
		//DmaCh [ 10 ].MADR = Temp;
				
//#ifdef INLINE_DEBUG_RUN_DMA10
//	debug << "; FINISHED";
//#endif

		// check if destination tag says transfer is complete
		// check top 2 bits for now (I think that bit 31 is IRQ and bit 30 is part of ID field indicating the stop)
		//if ( Data0 & 0xc0000000 )
		if ( ( DmaCh [ 10 ].WordsRemaining <= 0 ) && ( ( DmaCh [ 10 ].LastIOPTag & 0xc0000000 )
#ifdef REQUIRE_FULLSTOP_DMA10
				== 0xc0000000
#endif
		) )
		{
		
			if ( DmaCh [ 10 ].WordsRemainingMultiple4 > 0 )
			{
				WC = DmaCh [ 10 ].WordsRemainingMultiple4;
			
#ifdef INLINE_DEBUG_RUN_DMA10
	debug << "\r\n***MULTIPLE4***=" << dec << DmaCh [ 10 ].WordsRemainingMultiple4;
#endif

				for ( int i = 0; i < WC; i++ )
				{
#ifdef DMA10_FILL_EXTRA_DATA
					///////////////////////////////////////////////
					// Send previous address entry to main memory
					_BUS->MainMemory.b32 [ ( pRegData [ 10 ]->MADR & 0x1fffff ) >> 2 ] = *Data++;
#endif

#ifdef DMA10_FILL_EXTRA_ZERO
					_BUS->MainMemory.b32 [ ( pRegData [ 10 ]->MADR & 0x1fffff ) >> 2 ] = 0;
#endif

#ifdef DMA10_FILL_EXTRA_NONE
#else

					////////////////////////////////////////////////////
					// update address
					// ***todo*** this is a chain transfer, so maybe MADR should stay at the first address in block?
					pRegData [ 10 ]->MADR += 4;
					
#endif
					
					////////////////////////////////////////////
					// decrease count
					//DmaCh [ 10 ].BCR.Value--;
					//NumberOfTransfers--;
				}
			}

#ifdef ENABLE_SIF_DMA_TIMING
#ifdef INLINE_DEBUG_RUN_DMA10
			debug << " Setting finish delay=" << dec << CycleDelay;
#endif

			// save this for later
			// this delay should be handled at the source of the transfer
			SetNextEventCh_Cycle ( CycleDelay, 10 );
			
			// let the SIF know that the IOP cannot accept transfers for awhile
			Playstation2::SIF::_SIF->IOP_BusyUntil ( CycleDelay );
#else
			// dma transfer is done
			DMA_Finished ( 10, true, true );
			
			// check if EE dma #6 is done?
			if ( !Playstation2::SIF::_SIF->EE_DMA_Out_Ready () )
			{
				// clear bits in sif
				Playstation2::SIF::pRegData->F240 &= ~0x4040;
			}
#endif

			// need to return and transfer is complete
			return 1;
		}
		
	
		/*
		if ( ( Data0 & 0xc0000000 ) == 0xc0000000 )
		{
			DMA_Finished ( 10, true );
			//TransferInProgress = false;
		}
		else if ( ( Data0 & 0x80000000 ) == 0x80000000 )
		{
			//TransferInProgress = false;
		}
		*/
		
	// returning, but transfer not complete
	return 0;
}

#endif








// returns: 0 - device not ready, 1 - device ready immediately, other - device ready in the future at returned cycle number
u64 Dma::isDeviceReady ( int iChannel )
{
	// if channel number is invalid, then return that device is not ready
	if ( ((u32) iChannel) >= c_iNumberOfChannels ) return 0;

	if ( pRegData [ iChannel ]->CHCR.DR )
	{
		// transferring from memory to device //
		if ( cbReadyForWrite [ iChannel ] )
		{
			// device has a ready function
			// check if device is ready for a transfer
			return cbReadyForWrite [ iChannel ] ();

		} // end if ( cbReadyForWrite [ iChannel ] )
		else
		{
#ifdef INLINE_DEBUG_READY
	debug << "\r\nDevice for dma#" << dec << iChannel << " does not have a READY function for from memory.";
#endif

			cout << "\nDevice for dma#" << dec << iChannel << " does not have a READY function for from memory.";
		}
	}
	else
	{
		// transferring to memory from device //
		if ( cbReadyForRead [ iChannel ] )
		{
			// device has a ready function
			// check if device is ready for a transfer
			return cbReadyForRead [ iChannel ] ();
			
		} // end if ( cbReadyForRead [ iChannel ] )
		else
		{
#ifdef INLINE_DEBUG_READY
	debug << "\r\nDevice for dma#" << dec << iChannel << " does not have a READY function for to memory.";
#endif

			cout << "\nDevice for dma#" << dec << iChannel << " does not have a READY function for to memory.";
		}
		
	} // end if ( DmaCh [ iChannel ].CHCR.DR )

	// does not have a ready function for device
	return 0;
}


u32 Dma::Priority ( int iChannel )
{
	u32 ulPriority, ulPriorityOffset;
	
	// if channel number is invalid, then priority is zero
	if ( ((u32) iChannel) >= c_iNumberOfChannels ) return 0;
	
	// if channel is not transferring, then priority is zero
	if ( !pRegData [ iChannel ]->CHCR.TR ) return 0;
	
	// if channel is not enabled to transfer in PCR, then priority is zero
	if ( !isEnabled ( iChannel ) ) return 0;
	
	// check if the device is ready for the transfer
	if ( !isDeviceReady ( iChannel ) )
	{
		// channel is not ready to transfer
		return 0;
	}
	
	
	ulPriority = GetPriority ( iChannel );
	
#ifdef PS2_COMPILE
#ifdef ENABLE_PRIORITY_OFFSET
	ulPriorityOffset = GetPriorityOffset ( iChannel );
	ulPriority += ulPriorityOffset;
#endif
#endif

	
#ifdef ENABLE_PRIORITY_CHOP
	// otherwise just return the priority
	return ( iChannel | ( ( c_iPriorityLevels - ulPriority ) << 8 ) ) | ( ( pRegData [ iChannel ]->CHCR.Value & 0x40000000 ) ^ 0x40000000 );
#endif

#ifdef ENABLE_REVERSE_PRIORITY_CHOP
	return ( iChannel | ( ( c_iPriorityLevels - ulPriority ) << 8 ) ) | ( pRegData [ iChannel ]->CHCR.Value & 0x40000000 );
#endif

#ifdef DISABLE_PRIORITY_CHOP
	// otherwise just return the priority
	return ( iChannel | ( ( c_iPriorityLevels - ulPriority ) << 8 ) );
#endif
}


u32 Dma::GetPriorityOffset ( int iChannel )
{
#ifdef PS2_COMPILE
	// get the ps1 priority offset
	if ( iChannel < 7 )
	{
#endif

		return ( DMARegs0.PCR.Value >> 28 ) & 7;

#ifdef PS2_COMPILE
	}
	
	return ( DMARegs1.PCR2.Value >> 24 ) & 7;
#endif
}

// should return the amount that was transferred
u32 Dma::TransferBlock ( int iChannel, u32 Address, u32 WordCount )
{
#ifdef INLINE_DEBUG_TRANSFER_BLOCK
	debug << "\r\nDma::TransferBlock";
	debug << " DMA#" << dec << iChannel;
	debug << " Address=" << hex << Address;
	debug << " WC=" << dec << WordCount;
#endif

	//u32 Address, WordCount;
	u32 TransferAmt;
	u32 *pMemoryPtr;
	
	// if the amount to transfer is zero, then go ahead and return zero now
	// don't want to do this here, because the CDROM peripheral might accept a zero transfer
	//if ( !WordCount )
	//{
	//	return 0;
	//}
	
	// clear the transfer amount
	TransferAmt = 0;
	
	// get pointer into memory device
	pMemoryPtr = _BUS->MainMemory.b32;
	
	// get MADR
	//Address = DmaCh [ iChannel ].MADR;
	
				// get the transfer count (CD dma chop mode will be different)
				//WordCount = DmaCh [ iChannel ].BCR.BS;
				
				// perform the transfer
				
				// ***TODO*** account for CD DMA#3 chop modes
				
			// check if the direction on the transfer
			if ( pRegData [ iChannel ]->CHCR.DR )
			{
#ifdef INLINE_DEBUG_TRANSFER_BLOCK
	debug << " FROM";
#endif

				// transferring FROM memory //
				
				// make sure there is a transfer function
				if ( cbTransfer_FromMemory [ iChannel ] )
				{
					// there is a transfer function
					
					// ***TODO*** check for CD DMA#3 chop modes before transferring
					
					// so perform the transfer
					TransferAmt = cbTransfer_FromMemory [ iChannel ] ( pMemoryPtr, Address, WordCount );
					
					
				}
				else
				{
#ifdef INLINE_DEBUG_TRANSFER_BLOCK
	debug << "\r\nhps1x64: DMA: ERROR: There is not transfer function to transfer from memory for DMA#" << dec << iChannel;
#endif

					// there is no transfer function
					cout << "\nhps1x64: DMA: ERROR: There is not transfer function to transfer from memory for DMA#" << dec << iChannel;
				} // end if ( cbTransfer_FromMemory [ iChannel ] )
			}
			else
			{
#ifdef INLINE_DEBUG_TRANSFER_BLOCK
	debug << " TO";
#endif

				// transferring TO memory //
				
				// make sure there is a transfer function
				if ( cbTransfer_ToMemory [ iChannel ] )
				{
					// there is a transfer function
					
					// ***TODO*** check for CD DMA#3 chop modes before transferring
					
					// so perform the transfer
					TransferAmt = cbTransfer_ToMemory [ iChannel ] ( pMemoryPtr, Address, WordCount );
					
#ifdef ENABLE_MEMORY_INVALIDATE
					_BUS->InvalidateRange ( Address, WordCount );
#endif
				}
				else
				{
#ifdef INLINE_DEBUG_TRANSFER_BLOCK
	debug << "\r\nhps1x64: DMA: ERROR: There is not transfer function to transfer from memory for DMA#" << dec << iChannel;
#endif

					// there is no transfer function
					cout << "\nhps1x64: DMA: ERROR: There is not transfer function to transfer from memory for DMA#" << dec << iChannel;
				} // end if ( cbTransfer_ToMemory [ iChannel ] )

			} // end if ( DmaCh [ iChannel ].CHCR.DR )
				
#ifdef INLINE_DEBUG_TRANSFER_BLOCK
	debug << " TransferAmt" << dec << TransferAmt;
	debug << "\r\n";
#endif
				
	return TransferAmt;
}



void Dma::Arbitrate ( int iChannel )
{
#ifdef INLINE_DEBUG_ARBITRATE
	debug << "\r\nDma::Arbitrate";
#endif

	u32 TestChannel;
	u64 ullCycles;
	
	// check for another transfer
	TestChannel = GetActiveChannel ();
	
	if ( TestChannel == ActiveChannel )
	{
#ifdef INLINE_DEBUG_ARBITRATE
	debug << " ARBITRATE-CHANNEL-SAME";
#endif
		return;
	}
	
	// set the newly active channel
	ActiveChannel = TestChannel;

#ifdef INLINE_DEBUG_ARBITRATE
	debug << " NEW-ACTIVE-CHANNEL=" << dec << ActiveChannel;
	debug << " Priority=" << Priority ( ActiveChannel );
#endif
	
	// check if channel is ready immediately or in the future
	ullCycles = isDeviceReady ( ActiveChannel );
	
	// if device is not ready and it is not known if it will be ready, then done
	if ( !ullCycles )
	{
#ifdef INLINE_DEBUG_ARBITRATE
	debug << " DEVICE-NOT-READY";
#endif

		return;
	}
		
#ifdef INLINE_DEBUG_ARBITRATE
	debug << " DMA#" << dec << ActiveChannel << "-PENDING";
#endif

	// if channel is ready immediately, go ahead and start the transfer now
	if ( ullCycles == 1 )
	{
#ifdef INLINE_DEBUG_ARBITRATE
	debug << " TRANSFER-IMMEDIATELY";
#endif

		Transfer ( ActiveChannel );
		
		// done
		return;
	}
	else
	{
#ifdef INLINE_DEBUG_ARBITRATE
	debug << " TRANSFER-AT-CYCLE#" << dec << ullCycles;
#endif

		// transfer later at specified cycle number //
		
		SetNextEventCh_Cycle ( ullCycles, ActiveChannel );
		
		// need to return here since channel has changed?
		return;
	} // end if ( ullCycles == 1 )

}



void Dma::Transfer ( int iChannel, bool FORCE_TRANSFER )
{
#ifdef INLINE_DEBUG_TRANSFER
	debug << "\r\nDma::Transfer Channel=" << dec << iChannel;
	debug << " ActiveChannel=" << dec << ActiveChannel;
#endif

	u64 ullCycles;
	u64 ullReady;
	u32 ulNewChannelPriority, ulActiveChannelPriority;
	u32 Address, WordCount, TransferAmt;
	u32 NextAddress, Tag;
	u32 *pMemoryPtr;
	
	u32 TransferCount;
	u32 Data0, Data1;
	u32 *Data;

	// make sure the dma channel number is valid
	if ( ((u32) iChannel) >= c_iNumberOfChannels )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " INVALIDCHANNEL";
#endif
		// not a valid dma channel number
		return;
	}
	
	// step 0: determine if channel is enabled
	/*
	if ( !pRegData [ iChannel ]->CHCR.TR )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " !TR";
#endif

		// channel is not set to transfer anything
		return;
	}
	*/
	
	// make sure channel is enabled
	if ( ! isEnabled ( iChannel ) )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " !E";
#endif

		// reset events
		SetNextEventCh_Cycle ( -1ULL, iChannel );

		// channel is not set to transfer anything
		return;
	}

	// make sure channel is active
	if ( ! isActive ( iChannel ) )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " !A";
#endif

		// reset events
		SetNextEventCh_Cycle ( -1ULL, iChannel );
		
		// channel is not set to transfer anything
		return;
	}
	
	// step 2: determine if device is ready for the transfer (and when it will be ready if not)
	// if device is not ready for the transfer, then mark channel as "not currently transferring"
	// check if transferring to or from memory
	ullReady = isDeviceReady ( iChannel );
	if ( !ullReady )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " !READY";
#endif

		// device is NOT ready //

		// exceptions
		switch ( iChannel )
		{
			case 0:
				// this is similar to how I had it before. Will try to leave it like that until I get more time to get it perfect.
				Transfer ( 1 );
				break;
				
			default:
				// reset events
				SetNextEventCh_Cycle ( -1ULL, iChannel );
				
#ifndef USE_NEW_PS1_DMA_RUN
#ifdef ENABLE_ARBITRATE_ON_NOTREADY
				Arbitrate ( iChannel );
#endif
#endif

				break;
		}

		return;
	}

#ifdef INLINE_DEBUG_TRANSFER
	debug << " READY=" << dec << ullReady;
#endif
	
	
#ifndef USE_NEW_PS1_DMA_RUN
	// if this is a forced transfer, then skip the priority check
	if ( !FORCE_TRANSFER )
	{
	
	// step 3: determine if there is a channel already in progress, and if so determine if it can be interrupted
	// make sure the new channel is not the same as the active one
	// also make sure the active channel has a valid channel number
	if ( ( iChannel != ActiveChannel ) && ( ((u32) ActiveChannel) < c_iNumberOfChannels ) )
	{
		// the transfer trying to run is not the active one //
		
#ifdef ALLOW_TRANSFER_INTERRUPTION
		// a different channel wants to transfer, so we need to see if it needs to wait
		// make sure that the active channel number is valid?
		if (  ( (u32)ActiveChannel ) < c_iNumberOfChannels )
		{
			// calculate priority for the active channel
			ulActiveChannelPriority = Priority ( ActiveChannel );
			
			// calculate priority for the channel wanting to transfer data
			ulNewChannelPriority = Priority ( iChannel );
			
#ifdef INLINE_DEBUG_TRANSFER
	debug << " NewChannelPriority=" << dec << ulNewChannelPriority;
	debug << " ActiveChannelPriority=" << dec << ulActiveChannelPriority;
#endif

			// determine which channel gets to transfer
			if ( ulNewChannelPriority < ulActiveChannelPriority )
			{
				// the new channel trying to interrupt the current transfer needs to wait
				return;
			} // end if ( ulNewChannelPriority < ulActiveChannelPriority )
		}
#else
		// running the new channel would interrupt the one that is currently running
		return;
#endif
	} // end if ( iChannel != ActiveChannel )
	
		// another transfer is starting or continuing //
		

		// step 4: mark channel as "transfer in progress"
		ActiveChannel = iChannel;
	}
	else
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " FORCE-TRANSFER";
#endif
		
	} // end if ( !FORCE_TRANSFER )
#endif

#ifdef INLINE_DEBUG_TRANSFER
	debug << " NewActiveChannel=" << dec << ActiveChannel;
#endif

	
	// determine if device is ready immediately for active channel or will be ready at some known point in the future
	if ( ullReady > 1 )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " DEVICE-NOT-READY-UNTIL-CYCLE#" << dec << ullReady;
#endif

		// device is not ready immediately, but will be ready at the specified cycle
		// set DMA to continue at that cycle#
		SetNextEventCh_Cycle ( ullReady, iChannel );
		
		return;
	}
	
	// step 1: check if dma channel has a setup time
	// had to first make sure it was active channel and can now check for this
	if ( *_DebugCycleCount < DmaCh [ iChannel ].ullStartCycle )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " DEVICE-START-CYCLE#" << dec << DmaCh [ iChannel ].ullStartCycle;
#endif

		// not yet time to start the transfer (dma setup time?) //
		
		// return to complete the transfer after the setup time
		SetNextEventCh_Cycle ( DmaCh [ iChannel ].ullStartCycle, iChannel );
		
		return;
	}
	
	// if syncmode is 1, but BA is zero, then no transfer either, as long as it is not channel#10 (which is a special case)
	/*
	if ( ( DmaCh [ iChannel ].CHCR.SyncMode == 1 ) && ( !DmaCh [ iChannel ].BCR.BA )
#ifdef PS2_COMPILE
		&& ( iChannel != 10 )
#endif
	)
	{
		return;
	}
	*/
	

	// step 5: get the access time
	ullAccessTime = c_ullAccessTime [ iChannel ];
	
#ifdef INLINE_DEBUG_TRANSFER
	debug << " AccessTime=" << dec << ullAccessTime;
#endif
	
	// step 6: determine if there is a setup time
	// ***TODO***
	
	
	// get the release time (do it here, so it can be changed below if needed, before bus is released back to cpu)
	ullReleaseTime = c_ullReleaseTime [ iChannel ];
	
	
	// step 7: transfer to/from device
	
	// get pointer into memory device
	pMemoryPtr = _BUS->MainMemory.b32;
	
	// get the block count (this would be different for cd dma chop mode or linked-list, etc)
	WordCount = pRegData [ iChannel ]->BCR.BS;
	
	
	// loop this if needed (if cycles to release bus to cpu is zero)
	do
	{
	
	// get the Address
	Address = pRegData [ iChannel ]->MADR & DataBus::MainMemory_Mask;
	
	
	// determine if we are transfering to or from device
	if ( pRegData [ iChannel ]->CHCR.DR )
	{
#ifdef INLINE_DEBUG_TRANSFER_FROMMEM
	debug << " FROM-MEM";
#endif

		// Transferring FROM memory to device //
		
		// determine sync mode
		switch ( pRegData [ iChannel ]->CHCR.SyncMode )
		{
			// transfer all at once (CDROM, OTC)
			case 0:
#ifdef INLINE_DEBUG_TRANSFER_FROMMEM
	debug << " ONE-SHOT";
#endif

				// do the transfer
				TransferAmt = TransferBlock ( iChannel, Address, WordCount );
				
#ifdef INLINE_DEBUG_TRANSFER_FROMMEM
	debug << " TransferAmt=" << dec << TransferAmt;
#endif

					// note: Do not update MADR unless BA is set ??
					
					// subtract from block amount if there is anything there
					//if ( pRegData [ iChannel ]->BCR.BA && TransferAmt )
					//{
					//	pRegData [ iChannel ]->BCR.BA--;
					//	
					//	// ***TODO*** determine if MADR needs to be update here, also for PS2??
					//}
					

				// update CPU cycles
				ullCycles = TransferAmt * ullAccessTime;
				R3000A::Cpu::_CPU->CycleCount += ullCycles;
				
				// check if transfer is complete
				// ***TODO*** must check differently for CD DMA#3 chop modes
				//if ( !pRegData [ iChannel ]->BCR.BA )
				if ( TransferAmt )
				{
					// Transfer is complete //
					
					// set BCR to zero
					// note: BCR only decrements to zero when chopping is enabled per Martin Korth psx spec
					//pRegData [ iChannel ]->BCR.Value = 0;
					
					DMA_Finished ( iChannel );
				}
				
				break;
				
			// sync blocks to dma requests (MDEC, SPU, GPU)
			case 1:
#ifdef INLINE_DEBUG_TRANSFER_FROMMEM
	debug << " SYNC";
#endif
				
				// basically, transfer a block, then release bus to CPU for X cycles, then come back and transfer another block
				
				// do the transfer
				TransferAmt = TransferBlock ( iChannel, Address, WordCount );
				
#ifdef INLINE_DEBUG_TRANSFER_FROMMEM
	debug << " TransferAmt=" << dec << TransferAmt;
#endif
				
					// note: Do not update MADR unless BA is set ??
					
					// check if anything was transferred
					if ( TransferAmt )
					{
						// dec the block amount
						pRegData [ iChannel ]->BCR.BA--;
					}
						
					// for sync mode 1, update MADR
					pRegData [ iChannel ]->MADR += ( TransferAmt << 2 );
						
				// update CPU cycles
				ullCycles = TransferAmt * ullAccessTime;
				R3000A::Cpu::_CPU->CycleCount += ullCycles;
					
				// check if transfer is complete
				if ( !pRegData [ iChannel ]->BCR.BA )
				{
					// Transfer is complete //
					
					DMA_Finished ( iChannel );
				}
				
				break;
				
			// linked list mode
			case 2:
#ifdef INLINE_DEBUG_TRANSFER_FROMMEM
	debug << " LINKED-LIST";
#endif

				//cout << "\nhps1x64: ALERT: DMA: Linked-List transfer not yet implemented in new dma scheme.";
				
				// get tag
				Tag = pMemoryPtr [ ( Address & DataBus::MainMemory_Mask ) >> 2 ];
				
#ifdef INLINE_DEBUG_TRANSFER_FROMMEM
	debug << " TAG=" << hex << Tag;
#endif

				// get the next address
				NextAddress = Tag & 0xffffff;
				
#ifdef INLINE_DEBUG_TRANSFER_FROMMEM
	debug << " NEXTADDRESS=" << hex << NextAddress;
#endif

				// get the word count
				WordCount = Tag >> 24;
				
#ifdef INLINE_DEBUG_TRANSFER_FROMMEM
	debug << " WC=" << dec << WordCount;
#endif

				// check for any odd errors
				if ( WordCount > 32 )
				{
					cout << "\nhps1x64: GPU: ALERT: WC>32 for linked-list transfer to GPU.\n";
				}
				
				// nothing transferred yet
				TransferAmt = 0;

				// check if there is anything to transfer
				if ( WordCount )
				{
					// transfer the block (not including the tag)
					TransferAmt = TransferBlock ( iChannel, Address + 4, WordCount );
				}
				
				
				if ( TransferAmt )
				{
					// update the release time based on amount of time to process linked-list count/address,etc
					ullReleaseTime += c_LinkedListSetupTime;
				}
				
				// also count the tag
				TransferAmt++;
				
				// update CPU cycles
				ullCycles = TransferAmt * ullAccessTime;
				R3000A::Cpu::_CPU->CycleCount += ullCycles;
				
				// store next address in MADR?
				pRegData [ iChannel ]->MADR = NextAddress;
				
				
				
				// check for linked list terminator
				if ( NextAddress == 0xffffff )
				{
					// transfer is complete
					DMA_Finished ( iChannel );
				}
				
				break;

#ifdef PS2_COMPILE
			// PS2 ONLY? - linked list mode and sync blocks to dma requests
			case 3:
#ifdef INLINE_DEBUG_TRANSFER_FROMMEM
	debug << " PS2-SYNC-LINKED-LIST";
#endif

				if ( !DmaCh [ iChannel ].WordsRemaining )
				{
				// get pointer to tag data
				Data = & _BUS->MainMemory.b32 [ ( pRegData [ iChannel ]->TADR & 0x1fffff ) >> 2 ];
				Address = pRegData [ iChannel ]->TADR & 0x1fffff;
				
				// it appears that the tag address TADR points to ONLY the tags, IOP followed by EE, with the IOP/EE tag pairs separated by 128-bits (16 bytes)
				// bits 30 and 31 of Data0 (the address) are used for IRQ/STOP bits
		
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; IOP DMA Tag=" << *((u64*)Data);
#endif




				// transfer the tag //
				
				//Playstation2::SIF::_SIF->IOP_DMA_WriteBlock ( 0, Data, 4 );
				TransferBlock ( iChannel, Address, 4 );
				
				
				// read IOP TAG //

				// read the tag values
				Data0 = *Data++;
				Data1 = *Data++;
				

				// read the tag for EE
				DmaCh [ iChannel ].EEDMATag = *((u64*)Data);
				Data += 2;


					
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; Data0=" << hex << Data0 << "; Data1=" << Data1 << "; EEDMATag=" << DmaCh [ iChannel ].EEDMATag;
	debug << " NEXTTAG0=" << hex << ((u64*)Data) [ 0 ] << " NEXTTAG1=" << hex << ((u64*)Data) [ 1 ];
#endif

				// data0 looks like the address
				// data1 looks like tag/id/etc
				
				//TagID = ( Data1 >> 28 ) & 7;
				
				// set MADR
				// upper 8-bits of Data0 hold IRQ/ID/etc for IOP Tag
				// MADR can only address 2MB max on PS1
				pRegData [ iChannel ]->MADR = Data0 & 0x1fffff;
				
				// store the rest of the tag for later to check when transfer is done
				DmaCh [ iChannel ].LastIOPTag = Data0;
				
				// make sure that WordCount is a multiple of 4
				if ( Data1 & 3 )
				{
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << " WC-NOT-MULTIPLE4";
	debug << " (hex)WC=" << hex << Data1;
	debug << " (hex)QWC=" << hex << DmaCh [ iChannel ].EEDMATag;
#endif

#ifdef VERBOSE_DMA9_MULTIPLE4
					cout << "\nhps1x64: DMA9: ALERT: WC is not multiple of 4. (hex)WC=" << hex << Data1 << " QWC=" << DmaCh [ iChannel ].EEDMATag;
#endif
				}
				
#ifdef DMA9_ROUNDUP_128
				// *** TESTING *** round UP number of words to transfer to next 128-bit boundary
				Data1 = ( Data1 + 0x3 ) & 0xffffc;
#else
				// *** TESTING *** round DOWN number of words to transfer to next 128-bit boundary
				Data1 = Data1 & 0xffffc;
#endif
				
				// set the total number of words in block
				DmaCh [ iChannel ].BlockTotal = Data1;
				
				// the amount remaining to transfer is in Data1
				DmaCh [ iChannel ].WordsRemaining = Data1;
				
				// no data has been transferred yet for block
				DmaCh [ iChannel ].WCTransferred = 0;
				
				
				
				// has to transfer the tag first, so should probably start with a transfer count of 1
				//TransferCount = 1;
				
				}
				//else
				//{
				
				// don't transfer more data at a time than can be fit in the SIF buffer
				TransferAmt = ( c_llSIF0_BufferSize > DmaCh [ iChannel ].WordsRemaining ) ? DmaCh [ iChannel ].WordsRemaining : c_llSIF0_BufferSize;
				
				//}

				// the data for DMA9 is transferred to another device through SIF, so it has to be transferred on the opposite end of bus timing
				// so there should be a "wait" (where the cpu is doing nothing cuz bus if occupied) before the transfer
				// going in the opposite direction should be less of an issue possibly
				
				// double check
				if ( !TransferAmt )
				{
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; ALERT: TransferCount is zero!";
#endif

					cout << "\nhps1x64: ALERT: TransferCount is zero!\n";
				}

				
				
				// it is time to actually transfer the data //
				

#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "\r\n***DMA#9 SIF0 (IOP->EE) Transfering data from MADR=" << hex << ( pRegData [ iChannel ]->MADR + ( DmaCh [ iChannel ].WCTransferred << 2 ) ) << " to SIF. IOPTag IRQ/ID/ADDR=" << Data0 << dec << " Transferring(WC)=" << TransferCount << " out of " << DmaCh [ iChannel ].BlockTotal << " Cycle#" << dec << *_DebugCycleCount << "\r\n";
#endif

		

				// get pointer to data
				//Data = & _BUS->MainMemory.b32 [ ( ( DmaCh [ 9 ].MADR + ( DmaCh [ 9 ].WCTransferred << 2 ) ) & 0x1fffff ) >> 2 ];
				Address = ( pRegData [ iChannel ]->MADR + ( DmaCh [ iChannel ].WCTransferred << 2 ) );
				
				// check that there is data to transfer
				if ( TransferAmt )
				{
					// transfer the data to EE
					//Playstation2::SIF::_SIF->IOP_DMA_WriteBlock ( DmaCh [ 9 ].EEDMATag, Data, TransferCount );
					//Playstation2::SIF::_SIF->IOP_DMA_WriteBlock ( pMemory, Address, TransferCount );
					TransferAmt = TransferBlock ( iChannel, Address, TransferAmt );
				}
				
				// update amount transferred and remaining
				DmaCh [ iChannel ].WordsRemaining -= TransferAmt;
				DmaCh [ iChannel ].WCTransferred += TransferAmt;
				
				
				// include tag in transfer count ??
				// this needs to be nonzero for now??
				//TransferAmt += 1;
				
		
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "\r\n";
	debug << " WCTransferred=" << dec << DmaCh [ iChannel ].WCTransferred;
	debug << " WCRemaining=" << dec << DmaCh [ iChannel ].WordsRemaining;
	debug << dec << " PS2Cycle#" << *Playstation2::SIF::_DebugCycleCount;
#endif

				// the actual transfer has to be after the bus is marked as busy
				//_BUS->ReserveBus ( TransferCount );
				// update CPU cycles
				ullCycles = TransferAmt * ullAccessTime;
				R3000A::Cpu::_CPU->CycleCount += ullCycles;
				
				// just in case
				if ( DmaCh [ iChannel ].WordsRemaining < 0 )
				{
					cout << "\nhps2x64: ***ALERT***: DMA CH#9: Words remaining is negative!\n";
					DmaCh [ iChannel ].WordsRemaining = 0;
				}
				
				
				
				// check if transfer of block is complete
				if ( !DmaCh [ iChannel ].WordsRemaining )
				{
					// update TADR
					pRegData [ iChannel ]->TADR += 16;
					
					Data = & _BUS->MainMemory.b32 [ ( pRegData [ iChannel ]->TADR & 0x1fffff ) >> 2 ];
					
					// check if transfer should end
					//if ( Data0 & 0xc0000000 )
					if ( DmaCh [ iChannel ].LastIOPTag & 0xc0000000 )
					//if ( ! DmaCh [ iChannel ].EEDMATag )
					//if ( ! *((u64*)Data) )
					//if ( ( DmaCh [ iChannel ].EEDMATag & ( 1LL << 31 ) ) || ( ! *((u64*)Data) ) )
					{
						
						DMA_Finished ( iChannel, false );
						
						// check if EE dma #5 is done?
						if ( !Playstation2::SIF::_SIF->EE_DMA_In_Ready () )
						{
							// clear bits in sif
							Playstation2::SIF::pRegData->F240 &= ~0x2020;
						}

						
						//return;
					}
					
					/*
					// check if transfer should STOP??
					//if ( Data0 & 0xc0000000 )
					else if ( DmaCh [ iChannel ].LastIOPTag & 0xc0000000 )
					//if ( ! DmaCh [ iChannel ].EEDMATag )
					//if ( ! *((u64*)Data) )
					{
						
						DMA_Finished ( iChannel, false, false, true );
						
						
						//return;
					}
					*/

					
				}

				break;
#endif
		} // end switch ( DmaCh [ iChannel ].CHCR.SyncMode )
	}
	else
	{
#ifdef INLINE_DEBUG_TRANSFER_TOMEM_0
	debug << " TO-MEM";
#endif

		// Transferring TO memory from device //
		
		// determine sync mode
		switch ( pRegData [ iChannel ]->CHCR.SyncMode )
		{
			// transfer all at once (CDROM, OTC)
			case 0:
#ifdef INLINE_DEBUG_TRANSFER_TOMEM_1
	debug << " ONE-SHOT";
#endif

				// do the transfer
				TransferAmt = TransferBlock ( iChannel, Address, WordCount );
				
#ifdef INLINE_DEBUG_TRANSFER_TOMEM_2
	debug << " TransferAmt=" << dec << TransferAmt;
#endif

					// note: Do not update MADR unless BA is set ??
					
					// subtract from block amount if there is anything there
					//if ( pRegData [ iChannel ]->BCR.BA && TransferAmt )
					//{
					//	pRegData [ iChannel ]->BCR.BA--;
					//}
					
						// ***TODO*** determine if MADR needs to be update here, also for PS2??
					
				// update CPU cycles
				ullCycles = TransferAmt * ullAccessTime;
				R3000A::Cpu::_CPU->CycleCount += ullCycles;
				
				// check if transfer is complete
				// ***TODO*** must check differently for CD DMA#3 chop modes
				//if ( !pRegData [ iChannel ]->BCR.BA )
				if ( TransferAmt )
				{
					// Transfer is complete //
					
					// set BCR to zero
					// note: BCR only decrements to zero when chopping is enabled per Martin Korth psx spec
					//pRegData [ iChannel ]->BCR.Value = 0;
					
					DMA_Finished ( iChannel );
				}
				break;
				
			// sync blocks to dma requests (MDEC, SPU, GPU)
			case 1:

#ifdef PS2_COMPILE
				if ( iChannel == 10 )
				{
					TransferAmt = TransferBlock ( iChannel, Address, WordCount );
				}
				else
				{
#endif

					if ( pRegData [ iChannel ]->BCR.BA )
					{
						// do the transfer
						TransferAmt = TransferBlock ( iChannel, Address, WordCount );
					
#ifdef INLINE_DEBUG_TRANSFER_TOMEM_3
	debug << " TransferAmt=" << dec << TransferAmt;
#endif

					// note: Do not update MADR unless BA is set ??
					
						// for sync mode 1, update MADR
						pRegData [ iChannel ]->MADR += ( TransferAmt << 2 );
						
					
						// check if anything was transferred
						if ( TransferAmt )
						{
							// dec the block amount
							pRegData [ iChannel ]->BCR.BA--;
						}
							
						// update CPU cycles
						ullCycles = TransferAmt * ullAccessTime;
						R3000A::Cpu::_CPU->CycleCount += ullCycles;
						
					} // end if ( DmaCh [ iChannel ].BCR.BA )
					
							
						
					// check if transfer is complete
					if ( !pRegData [ iChannel ]->BCR.BA )
					{
						// Transfer is complete //
						
						DMA_Finished ( iChannel );
					}
					
#ifdef PS2_COMPILE
				} // end if ( iChannel == 10 )
#endif
				
				break;
				
			// linked list mode
			case 2:
				cout << "\nhps1x64: ALERT: attempting to perform linked-list transfer TO memory.";
				break;
				
#ifdef PS2_COMPILE
			// PS2 ONLY? - linked list mode and sync blocks to dma requests
			case 3:
				// the address and wordcount are probably wrong, but it probably does not matter here
				// for now it just jump starts the transfer from presumably the EE side
				TransferAmt = TransferBlock ( iChannel, 0, 0 );
				break;
#endif

		} // end switch ( DmaCh [ iChannel ].CHCR.SyncMode )
	} // end if ( DmaCh [ iChannel ].CHCR.DR )

#ifdef INLINE_DEBUG_TRANSFER_BOTTOM
	debug << " CPU-CYCLE=" << dec << R3000A::Cpu::_CPU->CycleCount;
#endif

	if ( FORCE_TRANSFER )
	{
#ifdef INLINE_DEBUG_TRANSFER_BOTTOM
	debug << " FORCE-TRANSFER-DONE";
#endif

		return;
	} // end if ( FORCE_TRANSFER )

	
	// step 8: determine if transfer is done. If so, check if there is another transfer waiting
	
	// check if transfer is complete
	if ( !pRegData [ iChannel ]->CHCR.TR )
	{
#ifdef INLINE_DEBUG_TRANSFER_BOTTOM
	debug << " TRANSFER-COMPLETE";
#endif

		// transfer is complete //
		
		
		
		// run execeptions for transfer completion
		switch ( iChannel )
		{
			case 0:
				MDEC::_MDEC->DMA0_End ();
				break;
				
			default:
				break;
		}
		
#ifndef USE_NEW_PS1_DMA_RUN
		// check for any other active channels and get them started
		Arbitrate ( iChannel );
#endif
		
		// must return here because the active channel might have changed
		return;
		
		/*
		// check for another transfer
		ActiveChannel = GetActiveChannel ();
		
		// check if channel is ready immediately or in the future
		ullCycles = isDeviceReady ( ActiveChannel );
		
		// if device is not ready and it is not known if it will be ready, then done
		if ( !ullCycles )
		{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " NO-DMA-PENDING";
#endif

			return;
		}
		
#ifdef INLINE_DEBUG_TRANSFER
	debug << " DMA#" << dec << ActiveChannel << "-PENDING";
#endif

		// if channel is ready immediately, go ahead and start the transfer now
		if ( ullCycles == 1 )
		{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " TRANSFER-IMMEDIATELY";
#endif

			Transfer ( ActiveChannel );
			
			// done
			return;
		}
		else
		{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " TRANSFER-AT-CYCLE#" << dec << ullCycles;
#endif

			// transfer later at specified cycle number //
			
			SetNextEventCh_Cycle ( ullCycles, ActiveChannel );
			
			// need to return here since channel has changed?
			return;
		} // end if ( ullCycles == 1 )
		*/
		
	} // end if ( !DmaCh [ iChannel ].CHCR.TR )
	
	// step 9: determine how long to release the bus for if device is still ready, otherwise
	ullReady = isDeviceReady ( iChannel );
	
	// if device is obviously not ready and if we do not know when device will be ready again, then done
	if ( ( !ullReady ) || ( !TransferAmt ) )
	{
#ifdef INLINE_DEBUG_TRANSFER_BOTTOM
	debug << " DEVICE-NO-LONGER-READY";
#endif

#ifndef USE_NEW_PS1_DMA_RUN
		// since device is no longer ready, that changes the active channel
		// check for any other active channels and get them started
		Arbitrate ( iChannel );
#endif

		return;
	}
	else if ( ullReady == 1 )
	{
		// get number of cycles to release bus for
		// note: got this value above, so it could be changed during processing of dma transfer
		//ullReleaseTime = c_ullReleaseTime [ iChannel ];
		
#ifdef INLINE_DEBUG_TRANSFER_BOTTOM
	debug << " RELEASE-TIME=" << dec << ullReleaseTime;
#endif

		if ( ullReleaseTime != -1ULL )
		{
			// set the variables to release the bus to CPU
			ullReturnCycle = R3000A::Cpu::_CPU->CycleCount + ullReleaseTime;
		}
		else
		{
			ullReturnCycle = -1ULL;
		}
		
#ifdef INLINE_DEBUG_TRANSFER_BOTTOM
	debug << " RETURN-AT#" << dec << ullReturnCycle;
#endif

		// exceptions
		switch ( iChannel )
		{
			case 4:

#ifdef PS2_COMPILE			
#ifdef ENABLE_CONTINUOUS_ADMA
				// also check for auto dma
				if ( SPU2::_SPU2->SPU0.isADMATransferMode () )
				{
					// if this is an auto dma transfer, don't release bus to CPU ??
					R3000A::Cpu::_CPU->CycleCount = ullReturnCycle;
					ullReleaseTime = 0;
					break;
				}
#endif
#endif

				// ***TODO*** this needs to be implemented properly ??? //
				if ( ( *_DebugPC & 0xffc00000 ) == 0xbfc00000 )
				{
					// pc is reading from bios //
					// if pc is reading from un-cached bios
					// then the reads might be getting cancelled out ??
					// since they take so long ??
					if ( ullReleaseTime < DataBus::c_iBIOS_Read_Latency )
					{
						// not enough time for a successful read from bios ??
						//ContinueToCompletion = true;
						R3000A::Cpu::_CPU->CycleCount = ullReturnCycle;
						ullReleaseTime = 0;
						
						break;
					}
				}
				
				// set the DMA to continue at that cycle#
				SetNextEventCh_Cycle ( ullReturnCycle, iChannel );
				
				return;
				
				break;
				
#ifdef PS2_COMPILE			
			case 7:
			
#ifdef ENABLE_CONTINUOUS_ADMA
				// also check for auto dma
				if ( SPU2::_SPU2->SPU1.isADMATransferMode () )
				{
					// if this is an auto dma transfer, don't release bus to CPU ??
					R3000A::Cpu::_CPU->CycleCount = ullReturnCycle;
					ullReleaseTime = 0;
					break;
				}
#endif

				// set the DMA to continue at that cycle#
				SetNextEventCh_Cycle ( ullReturnCycle, iChannel );
				
				// done
				return;
				break;
#endif

				
			default:

				// set the DMA to continue at that cycle#
				SetNextEventCh_Cycle ( ullReturnCycle, iChannel );
				
				// done
				return;
				
				break;
		} // end switch ( iChannel )
		

	}
	
	// step 10: determine if device is now busy. If so then transfer the rest later
	
	else
	{
#ifdef INLINE_DEBUG_TRANSFER_BOTTOM
	debug << " DEVICE-WILL-BE-READY-AT-CYCLE#" << dec << ullReady;
#endif
	
		// if device will be ready in the future, then continue transfer at that time
		// *** TODO *** consider having peripheral return the amount of time it will be busy for instead of cycle it is busy until
		// *** TODO *** potentially isn't possible for peripheral to know when it will be busy until (lots to consider here)
		//ullCycles = ullReady;
		
		// go ahead and set the cycle we will be returning at
		//ullReturnCycle = ullCycles;
		
		// set DMA to continue at that cycle#
		SetNextEventCh_Cycle ( ullReady, iChannel );
		
		return;
	}
	
	} while ( !ullReleaseTime );
}





void Dma::Run ()
{
	//u32 Temp;
	//u32 Data [ 4 ];
	
	// will use this for MDEC for now
	//u32 NumberOfTransfers;
		

	// check if dma is doing anything starting at this particular cycle
	//if ( NextEvent_Cycle != *_DebugCycleCount ) return;

#ifdef INLINE_DEBUG_RUN
	debug << "\r\nDma::Run";
	debug << " NextEvent=" << dec << NextEvent_Cycle;
	debug << " CycleCount=" << *_DebugCycleCount;
	debug << " ActiveChannel=" << ActiveChannel;
	// list out the events for channels
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		debug << " " << hex << i << "=" << dec << ((s64) NextEventCh_Cycle [ i ] ) << ( ( isEnabled(i) ) ? " E" : " NE" ) << ( ( isActive(i) ) ? " A" : " NA" );
	}
#endif

/*	
#ifdef PS2_COMPILE
	// check if this is for channel 9
	if ( NextEvent_Cycle == NextEventCh_Cycle [ 9 ] )
	{
#ifdef INLINE_DEBUG_RUN_DMA9
		debug << "\r\nChannel 9 RUNNING";
#endif

		//DMA_Finished ( 10, true );
		DMA9_Run ();
		Update_NextEventCycle ();
		return;
	}
#endif
*/


#ifdef USE_NEW_PS1_DMA_RUN

	int iChNumber;
	u64 ullChCycle;
	

	//while ( NextEvent_Cycle <= R3000A::Cpu::_CPU->CycleCount )
	do
	{
		iChNumber = -1;
		ullChCycle = -1ULL;
		

		// need to find dma channel that needs to run and is enabled and active
		for ( int i = 0; i < c_iNumberOfChannels; i++ )
		{
			if ( NextEventCh_Cycle [ i ] != -1ULL )
			{
				if ( isEnabled ( i ) )
				{
					if ( isActive ( i ) )
					{
						// check if cycle count comes before
						// if at same priority level, then later channel has higher priority?
						if ( NextEventCh_Cycle [ i ] <= ullChCycle )
						{
							if ( NextEventCh_Cycle [ i ] <= *_DebugCycleCount )
							{
								iChNumber = i;
								ullChCycle = NextEventCh_Cycle [ i ];
							}
						}
					}
				}
			}
		}
		
#ifdef INLINE_DEBUG_RUN
	debug << "\r\n->Dma::Run::Loop";
	debug << " Ch#=" << dec << iChNumber;
	debug << " ChNextEventCycle#" << ullChCycle;
	debug << " SysCycleCount=" << *_DebugCycleCount;
	debug << " CpuCycleCount=" << R3000A::Cpu::_CPU->CycleCount;
	debug << " NextEvent_Cycle=" << NextEvent_Cycle;
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		debug << " " << hex << i << "=" << dec << ((s64) NextEventCh_Cycle [ i ] ) << ( ( isEnabled(i) ) ? " E" : " NE" ) << ( ( isActive(i) ) ? " A" : " NA" );
	}
#endif

		if ( iChNumber != -1 )
		{
//#ifdef INLINE_DEBUG_RUN
//	debug << "\r\n->Dma::Run::Loop2";
//	debug << " Ch#=" << dec << iChNumber;
//	debug << " ChNextEventCycle#" << ullChCycle;
//	debug << " CpuCycleCount=" << R3000A::Cpu::_CPU->CycleCount;
//#endif

			// handling the event here, so clear the current event for the channel
			SetNextEventCh_Cycle ( -1ULL, iChNumber );
			
			// channel is next in line to be run?
			Transfer ( iChNumber );
		
			// check which channel is next to run
			Update_NextEventCycle ();
		}
		
	} while ( iChNumber != -1 );

#ifdef INLINE_DEBUG_RUN
	debug << "\r\n->Dma::Run";
	debug << " NextEvent=" << dec << NextEvent_Cycle;
	debug << " SysCycleCount=" << *_DebugCycleCount;
	debug << " CpuCycleCount=" << R3000A::Cpu::_CPU->CycleCount;
	debug << " ActiveChannel=" << ActiveChannel;
	// list out the events for channels
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		debug << " " << hex << i << "=" << dec << ((s64) NextEventCh_Cycle [ i ] ) << ( ( isEnabled(i) ) ? " E" : " NE" ) << ( ( isActive(i) ) ? " A" : " NA" );
	}
#endif

#else


#ifdef PS2_COMPILE

#ifdef ALLOW_NEW_ACTIVE_CHANNEL_ONRUN
	// if this is a PS2, then needs to recalcualate active channel, since this can change due to factors on ps2 side
	// (was causing issues)
	ActiveChannel = GetActiveChannel ();
#endif

#endif
	
	// check if dma even has any channels active first
	if ( ( (u32)ActiveChannel ) >= c_iNumberOfChannels )
	{
#ifdef INLINE_DEBUG_RUN
	debug << " INVALID-ACTIVE-CHANNEL=" << dec << ActiveChannel;
	debug << " DONE";
#endif

		// clear the next inactive event for now
		for ( int i = 0; i < c_iNumberOfChannels; i++ )
		{
			if ( NextEvent_Cycle == NextEventCh_Cycle [ i ] )
			{
				// for now, dma only transfers the "ActiveChannel", so can cancel this event
				SetNextEventCh_Cycle ( -1ULL, i );
				
				// done - it can return if there are more non-active events at same or other cycle numbers
				break;
			}
		}

		// if there is an active channel, but it is not set to run on this cycle, then update the next cycle //
		Update_NextEventCycle ();
		
		// active channel number is invalid
		return;
	}
	
	// check if the active channel is set to run
	if ( NextEvent_Cycle == NextEventCh_Cycle [ ActiveChannel ] )
	{
#ifdef INLINE_DEBUG_RUN
	debug << " TRANSFER";
	debug << " ActiveChannel=" << ActiveChannel << " NextEventCh_Cycle [ ActiveChannel ]=" << NextEventCh_Cycle [ ActiveChannel ];
#endif

		// handling the event here, so clear the current event for the channel
		SetNextEventCh_Cycle ( -1ULL, ActiveChannel );
		
		
		// run the active channel (either 0,1,2,4)
		// note: could also be dma#3 in chop mode
		//DMA_Run ( ActiveChannel );
		Transfer ( ActiveChannel );
	}
	else
	{
#ifdef INLINE_DEBUG_RUN
	debug << " NO-TRANSFER";
	debug << " UPDATE-NEXT-EVENT";
#endif

		// clear the next inactive event for now
		for ( int i = 0; i < c_iNumberOfChannels; i++ )
		{
			if ( NextEvent_Cycle == NextEventCh_Cycle [ i ] )
			{
				// for now, dma only transfers the "ActiveChannel", so can cancel this event
				SetNextEventCh_Cycle ( -1ULL, i );
				
				// done - it can return if there are more non-active events at same or other cycle numbers
				break;
			}
		}

		// if there is an active channel, but it is not set to run on this cycle, then update the next cycle //
		Update_NextEventCycle ();
		
#ifdef INLINE_DEBUG_RUN
	debug << " NEXT-EVENT-CYCLE=" << dec << NextEvent_Cycle;
#endif
	}
	
#endif

}

 
static u32 Dma::Read ( u32 Address )
{
#if defined INLINE_DEBUG_READ
	debug << "\r\n\r\nDMA::Read; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
#endif

	//u32 DmaChannelNum;
	u32 ulOutput;
	u32 Shift;
	
	Shift = ( ( Address & 0x3 ) << 3 );
	
	// get register offset
	Address &= 0xffff;
	
	// make sure the register value is in range
	if ( Address < 0x1100 )
	{
#if defined INLINE_DEBUG_READ
	debug << " " << Reg0_Names [ ( Address - 0x1080 ) >> 2 ];
#endif

		// addresses start at 0x1f801080 for ps1 dma registers //
		Address -= 0x1080;
		Address >>= 2;
		
		// if CHCR or mirror, then read value from CHCR
		if ( Address & 2 )
		{
			Address &= ~1;
		}
		
		// if ps1, just read the register
		ulOutput = _DMA->DMARegs0.Regs [ Address ] >> Shift;
		
	} // end if ( Address < 0x1100 )
	
#ifdef PS2_COMPILE

	// if ps2, then check if it is in the second range of DMA registers for ps2
	else if ( Address >= 0x1500 && Address < 0x1580 )
	{
#if defined INLINE_DEBUG_READ
	debug << " " << Reg1_Names [ ( Address - 0x1500 ) >> 2 ];
#endif

		// addresses start at 0x1f801500 for second range of dma registers on ps2 //
		Address -= 0x1500;
		
		// read the register
		ulOutput = _DMA->DMARegs1.Regs [ Address >> 2 ] >> Shift;
		
		// assuming there is no CHCR mirror for this second group of registers
	}
#endif
	
	// address is outside of range ?
	else
	{
#if defined INLINE_DEBUG_READ
	debug << " " << "INVALID";
#endif

		// invalid DMA Register
		cout << "\nhps1x64 ALERT: Unknown DMA READ @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << "\n";
		ulOutput = 0;
	}
	
#if defined INLINE_DEBUG_READ
	debug << " Output=" << hex << ulOutput;
#endif
	
	return ulOutput;
	
	
	
	/*
	switch ( Address )
	{
		case 0x1f8010f6:
		
			return ( _DMA->ICR_Reg.Value >> 16 );
			
			break;
		
		case PCR:
#if defined INLINE_DEBUG_READ_PCR
	debug << "\r\n\r\nDMA::Read; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
#endif

#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_PCR
			debug << "; PCR = " << hex << _DMA->PCR_Reg.Value;
#endif

			return _DMA->PCR_Reg.Value;
			break;
			
		case ICR:
#ifdef INLINE_DEBUG_READ
			debug << "; ICR = " << hex << _DMA->ICR_Reg.Value;
#endif

			return _DMA->ICR_Reg.Value;
			break;


#ifdef PS2_COMPILE

		case PCR2:
#if defined INLINE_DEBUG_READ_PCR2
	debug << "\r\n\r\nDMA::Read; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
#endif

#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_PCR2
			debug << "; PCR2 = " << hex << _DMA->PCR2_Reg.Value;
#endif

			// *** todo ***
			return _DMA->PCR2_Reg.Value;
			break;
			
			
		case ICR2:
#if defined INLINE_DEBUG_READ_ICR2
	debug << "\r\n\r\nDMA::Read; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
#endif

#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_ICR2
			debug << "; ICR2 = " << hex << _DMA->ICR2_Reg.Value;
#endif

			// *** todo ***
			return _DMA->ICR2_Reg.Value;
			break;
			
			
		case REG_1578:
#if defined INLINE_DEBUG_READ_REG1578
	debug << "\r\n\r\nDMA::Read; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
#endif

#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_REG1578
			debug << "; REG1578 = " << hex << _DMA->lReg_1578;
#endif
			
			return _DMA->lReg_1578;
			//return 1;
			break;


		case DMA_SIF0_CTRL:
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_REG_SIF0CTRL
			debug << "; SIF0_CTRL = " << hex << _DMA->lSIF0_CTRL;
#endif

			return _DMA->lSIF0_CTRL;
			break;
			
		case DMA_SIF1_CTRL:
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_REG_SIF1CTRL
			debug << "; SIF1_CTRL = " << hex << _DMA->lSIF1_CTRL;
#endif

			return _DMA->lSIF1_CTRL;
			break;
			
		case DMA_SIF2_CTRL:
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_REG_SIF2CTRL
			debug << "; SIF2_CTRL = " << hex << _DMA->lSIF2_CTRL;
#endif

			return _DMA->lSIF2_CTRL;
			break;
			
#endif



			
		default:

#ifdef PS2_COMPILE
			if ( ( Address >= 0x1f801080 && Address < 0x1f8010f0 ) || ( Address >= 0x1f801500 && Address < 0x1f801560 ) )
#else
			if ( Address >= 0x1f801080 && Address < 0x1f8010f0 )
#endif
			{

				// get the dma channel number
				DmaChannelNum = ( ( Address >> 4 ) & 0xf ) - 8;
				
#ifdef PS2_COMPILE
				if ( ( Address & 0xffff ) >= 0x1500 )
				{
					DmaChannelNum += ( 7 + 8 );
				}
#endif


				//switch ( Address & 0xf )
				switch ( Address & 0xc )
				{
					case 0:
#ifdef INLINE_DEBUG_READ
					debug << "; DMA" << DmaChannelNum << "_MADR = " << hex << _DMA->DmaCh [ DmaChannelNum ].MADR;
#endif

						return _DMA->DmaCh [ DmaChannelNum ].MADR;
						break;
					
					case 4:
#ifdef INLINE_DEBUG_READ
					debug << "; DMA" << DmaChannelNum << "_BCR = " << hex << _DMA->DmaCh [ DmaChannelNum ].BCR.Value;
#endif

						return _DMA->DmaCh [ DmaChannelNum ].BCR.Value;
						break;
				
				case 8:
				case 0xc:
#ifdef INLINE_DEBUG_READ
					debug << "; DMA" << DmaChannelNum << "_CHCR = " << hex << _DMA->DmaCh [ DmaChannelNum ].CHCR.Value;
#endif

#ifdef PS2_COMPILE

						if ( ( DmaChannelNum > 6 ) && ( ( Address & 0xf ) == 0xc ) )
						{
#ifdef INLINE_DEBUG_WRITE
					debug << "; (Before) DMA" << DmaChannelNum << "_TADR=" << hex << _DMA->DmaCh [ DmaChannelNum ].TADR;
#endif

							// these are the new PS2 registers - treat as TADR //
							return _DMA->DmaCh [ DmaChannelNum ].TADR;
							
#ifdef INLINE_DEBUG_WRITE
					debug << "; (After) DMA" << DmaChannelNum << "_TADR=" << hex << _DMA->DmaCh [ DmaChannelNum ].TADR;
#endif

							// done
							break;
						}
						
#endif


						return _DMA->DmaCh [ DmaChannelNum ].CHCR.Value;
						break;
					
						
					default:
#ifdef INLINE_DEBUG_READ
						debug << "; Invalid";
#endif
		
						// invalid DMA Register
						cout << "\nhps1x64 ALERT: Unknown DMA READ @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << "\n";
						break;
				}
			}
			else
			{
				cout << "\nhps1x64 WARNING: READ from unknown DMA Register @ Cycle#" << dec << *_DebugCycleCount << " PC=" << hex << *_DebugPC << " Address=" << Address << "\n";
			}

			break;
			
	}
	*/

}


static void Dma::Write ( u32 Address, u32 Data, u32 Mask )
{
#if defined INLINE_DEBUG_WRITE
	debug << "\r\n\r\nDMA::Write; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
#endif

	u32 iChannel;
	u32 Temp;
	u32 Shift;
	u32 PreviousValue;
	
	u32 NewActiveChannel;
	//bool FreezeCPU;
	
	// if offset, then shift data+mask over
	Shift = ( Address & 3 ) << 3;
	Data <<= Shift;
	Mask <<= Shift;
	
	// should apply write mask ahead of time, just to make sure
	Data &= Mask;
	
	// get register offset
	Address &= 0xffff;
	
	// make sure the register value is in range
	if ( Address < 0x1100 )
	{
#if defined INLINE_DEBUG_WRITE
	debug << " " << Reg0_Names [ ( Address - 0x1080 ) >> 2 ];
#endif

		// addresses start at 0x1f801080 for ps1 dma registers //
		Address -= 0x1080;
		Address >>= 2;
		
		// check if ICR, and if so call ICR update
		if ( Address == ( ( ( ICR & 0xffff ) - 0x1080 ) >> 2 ) )
		{
			// update the ICR register
			_DMA->Update_ICR ( Data );
			
			// done
			return;
		}
				
		// if CHCR or mirror, then get previous value
		if ( Address & 2 )
		{
			if ( Address < ( ( ( PCR & 0xffff ) - 0x1080 ) >> 2 ) )
			{
				// set address to CHCR
				Address &= ~1;
				
				// get previous value
				PreviousValue = _DMA->DMARegs0.Regs [ Address ];
				
				// write value to CHCR
				_DMA->DMARegs0.Regs [ Address ] = ( Data ) | ( PreviousValue & ~Mask );
				
				// get the channel number
				iChannel = ( Address >> 2 ) & 0xf;
				
				// also make a call 
				// allow for a setup time before transfer by checking for transition of TR from 0 to 1
				if ( ( _DMA->DMARegs0.Regs [ Address ] & 0x1000000 ) && !( PreviousValue & 0x1000000 ) )
				{
					// dma channel has just been turned on, so set the start time for channel
					_DMA->DmaCh [ iChannel ].ullStartCycle = *_DebugCycleCount + c_ullSetupTime [ iChannel ];
				}
				
				// update dma channel
				_DMA->DMA_Update ( iChannel );
				
				// done
				return;
			}
		}
		
		
		// if ps1, start by doing the write
		_DMA->DMARegs0.Regs [ Address ] = ( Data ) | ( _DMA->DMARegs0.Regs [ Address ] & ~Mask );
		
		// if PCR, then get the active channel after write and update dma
		if ( Address == ( ( ( PCR & 0xffff ) - 0x1080 ) >> 2 ) )
		{
			// *** check to see if this changes the active channel *** //
			// recalculate the active channel
			NewActiveChannel = _DMA->GetActiveChannel ();
			
#ifdef INLINE_DEBUG_WRITE
		debug << "\r\nActiveChannel=" << dec << _DMA->ActiveChannel << " NewActiveChannel=" << NewActiveChannel;
#endif

#ifdef DMA_UPDATE_PREVIOUS
			_DMA->DMA_Update ( -1 );
#else
			_DMA->DMA_Update ( NewActiveChannel );
#endif
		}
			
		
	} // end if ( Address < 0x1100 )
	
#ifdef PS2_COMPILE

	// if ps2, then check if it is in the second range of DMA registers for ps2
	else if ( Address >= 0x1500 && Address < 0x1580 )
	{
#if defined INLINE_DEBUG_WRITE
	debug << " " << Reg1_Names [ ( Address - 0x1500 ) >> 2 ];
#endif

		// addresses start at 0x1f801500 for second range of dma registers on ps2 //
		Address -= 0x1500;
		Address >>= 2;
		
		// check if ICR2, and if so call ICR update
		if ( Address == ( ( ( ICR2 & 0xffff ) - 0x1500 ) >> 2 ) )
		{
			// update the ICR register
			_DMA->Update_ICR2 ( Data );
			
			// done
			return;
		}
				
		// if CHCR or mirror, then get previous value
		if ( ( Address & 3 ) == 2 )
		{
			if ( Address < ( ( ( DMA_SIF0_CTRL & 0xffff ) - 0x1500 ) >> 2 ) )
			{
				// set address to CHCR
				Address &= ~1;
				
				// get previous value
				PreviousValue = _DMA->DMARegs1.Regs [ Address ];
				
				// write value to CHCR
				_DMA->DMARegs1.Regs [ Address ] = ( Data ) | ( PreviousValue & ~Mask );
				
				// get the channel number
				iChannel = ( ( Address >> 2 ) + 7 ) & 0xf;
				
				// also make a call 
				// allow for a setup time before transfer by checking for transition of TR from 0 to 1
				if ( ( _DMA->DMARegs1.Regs [ Address ] & 0x1000000 ) && !( PreviousValue & 0x1000000 ) )
				{
					// dma channel has just been turned on, so set the start time for channel
					_DMA->DmaCh [ iChannel ].ullStartCycle = *_DebugCycleCount + c_ullSetupTime [ iChannel ];
				}
				
				// update dma channel
				_DMA->DMA_Update ( iChannel );
				
				// done
				return;
			}
		}
		
		// if ps1, start by doing the write
		_DMA->DMARegs1.Regs [ Address ] = ( Data ) | ( _DMA->DMARegs1.Regs [ Address ] & ~Mask );
		
		// assuming there is no CHCR mirror for this second group of registers
		
		// if PCR2, then get the active channel after write and update dma
		if ( Address == ( ( ( PCR2 & 0xffff ) - 0x1500 ) >> 2 ) )
		{
			// *** check to see if this changes the active channel *** //
			// recalculate the active channel
			NewActiveChannel = _DMA->GetActiveChannel ();
			
#ifdef INLINE_DEBUG_WRITE
		debug << "\r\nActiveChannel=" << dec << _DMA->ActiveChannel << " NewActiveChannel=" << NewActiveChannel;
#endif

#ifdef DMA_UPDATE_PREVIOUS
			_DMA->DMA_Update ( -1 );
#else
			_DMA->DMA_Update ( NewActiveChannel );
#endif
		}
	}
#endif
	
	// address is outside of range ?
	else
	{
#if defined INLINE_DEBUG_WRITE
	debug << " " << "INVALID";
#endif

		// invalid DMA Register
		cout << "\nhps1x64 ALERT: Unknown DMA WRITE @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << " Data=" << Data << "\n";
	}
	
	
	return;
	

	/*
	switch ( Address & ~3 )
	{
		case 0x1f8010f6:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_ICR
			debug << "; (Before) ICR = " << hex << _DMA->ICR_Reg.Value << "; Interrupt_State=" << _DMA->Interrupt_State;
#endif

			// update ICR register
			_DMA->Update_ICR ( Data << 16 );

			
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_ICR
			debug << "; (After) ICR = " << hex << _DMA->ICR_Reg.Value << "; Interrupt_State=" << _DMA->Interrupt_State;
#endif
			break;

			
		case ICR:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_ICR
			debug << "; (Before) ICR = " << hex << _DMA->ICR_Reg.Value << "; _Intc_Stat=" << *_Intc_Stat << " _Intc_Mask=" << *_Intc_Mask << " _R3000A_Status=" << *_R3000A_Status_12;
#endif

			// update the ICR register
			_DMA->Update_ICR ( Data );

#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_ICR
			debug << "; (After) ICR = " << hex << _DMA->ICR_Reg.Value << "; _Intc_Stat=" << *_Intc_Stat << " _Intc_Mask=" << *_Intc_Mask << " _R3000A_Status=" << *_R3000A_Status_12;
#endif

			break;
		
		case PCR:
#if defined INLINE_DEBUG_WRITE_PCR
	debug << "\r\n\r\nDMA::Write; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
#endif

#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_PCR
			debug << "; (Before) PCR = " << hex << _DMA->PCR_Reg.Value;
#endif
			u32 NewActiveChannel;
		
			// set value
			//_DMA->PCR_Reg.Value = Data;	//n_dpcp = ( n_dpcp & ~mem_mask ) | data;
			_DMA->PCR_Reg.Value = ( Data ) | ( _DMA->PCR_Reg.Value & ~Mask );
			
			// *** check to see if this changes the active channel *** //
			// recalculate the active channel
			NewActiveChannel = _DMA->GetActiveChannel ();
			
#ifdef INLINE_DEBUG_WRITE
		debug << "\r\nActiveChannel=" << dec << _DMA->ActiveChannel << " NewActiveChannel=" << NewActiveChannel << " DmaChannelNum=" << DmaChannelNum;
#endif

#ifdef DMA_UPDATE_PREVIOUS
			_DMA->DMA_Update ( -1 );
#else
			_DMA->DMA_Update ( NewActiveChannel );
#endif

			
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_PCR
			debug << "; (After) PCR = " << hex << _DMA->PCR_Reg.Value;
#endif

			break;
			
			
#ifdef PS2_COMPILE

		case PCR2:
#if defined INLINE_DEBUG_WRITE_PCR2
	debug << "\r\n\r\nDMA::Write; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
#endif

#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_PCR2
			debug << "; (Before) PCR2 = " << hex << _DMA->PCR2_Reg.Value;
#endif

			// *** todo ***
			//_DMA->PCR2_Reg.Value = Data;
			_DMA->PCR2_Reg.Value = ( Data ) | ( _DMA->PCR2_Reg.Value & ~Mask );
			
			// *** check to see if this changes the active channel *** //
			// recalculate the active channel
			NewActiveChannel = _DMA->GetActiveChannel ();
			
#ifdef INLINE_DEBUG_WRITE
		debug << "\r\nActiveChannel=" << dec << _DMA->ActiveChannel << " NewActiveChannel=" << NewActiveChannel << " DmaChannelNum=" << DmaChannelNum;
#endif


#ifdef DMA_UPDATE_PREVIOUS
			_DMA->DMA_Update ( -1 );
#else
			_DMA->DMA_Update ( NewActiveChannel );
#endif
			
			break;
			
			
		case ICR2:
#if defined INLINE_DEBUG_WRITE_ICR2
	debug << "\r\n\r\nDMA::Write; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
#endif

#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_ICR2
			debug << "; (Before) ICR2 = " << hex << _DMA->ICR2_Reg.Value;
#endif

			// *** todo ***
			//_DMA->ICR2_Reg.Value = Data;
			_DMA->Update_ICR2 ( Data );
			
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_ICR2
			debug << "; (After) ICR2 = " << hex << _DMA->ICR2_Reg.Value;
#endif
			break;

			
		case REG_1578:
#if defined INLINE_DEBUG_WRITE_REG1578
	debug << "\r\n\r\nDMA::Write; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
#endif

#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_REG1578
			debug << "; (Before) REG1578 = " << hex << _DMA->lReg_1578;
#endif
			
			_DMA->lReg_1578 = Data;
			break;


		case DMA_SIF0_CTRL:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_SIF0CTRL
			debug << "; (Before) SIF0_CTRL = " << hex << _DMA->lSIF0_CTRL;
#endif

			_DMA->lSIF0_CTRL = Data;
			break;
			
		case DMA_SIF1_CTRL:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_SIF1CTRL
			debug << "; (Before) SIF1_CTRL = " << hex << _DMA->lSIF1_CTRL;
#endif

			_DMA->lSIF1_CTRL = Data;
			break;
			
		case DMA_SIF2_CTRL:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_SIF2CTRL
			debug << "; (Before) SIF2_CTRL = " << hex << _DMA->lSIF2_CTRL;
#endif

			_DMA->lSIF2_CTRL = Data;
			break;
			
#endif
			
		default:
		
#ifdef PS2_COMPILE
			if ( ( Address >= 0x1f801080 && Address < 0x1f8010f0 ) || ( Address >= 0x1f801500 && Address < 0x1f801560 ) )
#else
			if ( Address >= 0x1f801080 && Address < 0x1f8010f0 )
#endif
			{
				// get the dma channel number
				DmaChannelNum = ( ( Address >> 4 ) & 0xf ) - 8;
			
#ifdef PS2_COMPILE
				if ( ( Address & 0xffff ) >= 0x1500 )
				{
					DmaChannelNum += ( 7 + 8 );
				}
#endif

				//switch ( Address & 0xf )
				switch ( Address & 0xc )
				{
					case 0:
#if defined INLINE_DEBUG_WRITE_MADR
	debug << "\r\n\r\nDMA::Write; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_MADR
					debug << "; (Before) DMA" << DmaChannelNum << "_MADR = " << hex << _DMA->DmaCh [ DmaChannelNum ].MADR;
#endif

#ifdef INLINE_DEBUG_RUN_DMA4
	if ( DmaChannelNum == 4 )
	{
	debug << "\r\nDMA" << DmaChannelNum << "_MADR=" << hex << Data;
	debug << " CycleCount=" << dec << *_DebugCycleCount;
	debug << hex << " PC=" << *_DebugPC;
	}
#endif

						// *** PROBLEM *** DMA2 gets written to sometimes while it is already in use //

						// note: upper 8-bits are always zero
						//_DMA->DmaCh [ DmaChannelNum ].MADR = Data & 0x00ffffff;
						_DMA->DmaCh [ DmaChannelNum ].MADR = ( Data ) | ( _DMA->DmaCh [ DmaChannelNum ].MADR & ~Mask ) & 0x00ffffff;
						
						if ( DmaChannelNum == 2 )
						{
							_DMA->LL_NextAddress = Data & 0x00ffffff;
						}

#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_MADR
					debug << "; (After) DMA" << DmaChannelNum << "_MADR = " << hex << _DMA->DmaCh [ DmaChannelNum ].MADR;
#endif

						break;
				
					case 4:
#if defined INLINE_DEBUG_WRITE_BCR
	debug << "\r\n\r\nDMA::Write; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_BCR
					debug << "; (Before) DMA" << DmaChannelNum << "_BCR = " << hex << _DMA->DmaCh [ DmaChannelNum ].BCR.Value;
#endif

#ifdef INLINE_DEBUG_RUN_DMA4
	if ( DmaChannelNum == 4 )
	{
	debug << "\r\nDMA" << DmaChannelNum << "_BCR=" << hex << Data;
	debug << " CycleCount=" << dec << *_DebugCycleCount;
	debug << hex << " PC=" << *_DebugPC;
	}
#endif

						// appears that DMA has all sorts of write types implemented
						//_DMA->DmaCh [ DmaChannelNum ].BCR.Value = Data;
						_DMA->DmaCh [ DmaChannelNum ].BCR.Value = ( Data ) | ( _DMA->DmaCh [ DmaChannelNum ].BCR.Value & ~Mask );
						
						// also need to save the size of the transfer for after decrementing the block count
						//_DMA->TransferSize_Save [ DmaChannelNum ] = Data;
						_DMA->TransferSize_Save [ DmaChannelNum ] = _DMA->DmaCh [ DmaChannelNum ].BCR.Value;

#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_BCR
					debug << "; (After) DMA" << DmaChannelNum << "_BCR = " << hex << _DMA->DmaCh [ DmaChannelNum ].BCR.Value;
#endif

						break;

					case 8:
					case 0xc:	// CHCR MIRROR for PS1, TADR for PS2 on new registers

#ifdef PS2_COMPILE

						if ( ( DmaChannelNum == 9 ) && ( ( Address & 0xf ) >= 0xc ) )
						{
#if defined INLINE_DEBUG_WRITE_TADR
	debug << "\r\n\r\nDMA::Write; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_TADR
					debug << "; (Before) DMA" << DmaChannelNum << "_TADR=" << hex << _DMA->DmaCh [ DmaChannelNum ].TADR;
#endif

							// these are the new PS2 registers - treat as TADR //
							//_DMA->DmaCh [ DmaChannelNum ].TADR = Data;
							_DMA->DmaCh [ DmaChannelNum ].TADR = ( Data ) | ( _DMA->DmaCh [ DmaChannelNum ].TADR & ~Mask );
							
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_TADR
					debug << "; (After) DMA" << DmaChannelNum << "_TADR=" << hex << _DMA->DmaCh [ DmaChannelNum ].TADR;
#endif

							// done
							return;
							//break;
						}
						
#endif
					
#if defined INLINE_DEBUG_WRITE_CHCR_0
	debug << "\r\n\r\nDMA::Write; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CHCR_0
					debug << "; (Before) DMA" << DmaChannelNum << "_CHCR = " << hex << _DMA->DmaCh [ DmaChannelNum ].CHCR.Value;
#endif

#ifdef INLINE_DEBUG_RUN_DMA4
	if ( DmaChannelNum == 4 )
	{
	debug << "\r\nDMA" << DmaChannelNum << "_CHCR=" << hex << Data;
	debug << " CycleCount=" << dec << *_DebugCycleCount;
	debug << hex << " PC=" << *_DebugPC;
	}
#endif

						// get channel CHCR
						Previous_Value = _DMA->DmaCh [ DmaChannelNum ].CHCR.Value;


						//_DMA->DmaCh [ DmaChannelNum ].CHCR.Value = Data;
						_DMA->DmaCh [ DmaChannelNum ].CHCR.Value = ( Data ) | ( _DMA->DmaCh [ DmaChannelNum ].CHCR.Value & ~Mask );
						
						
						// allow for a setup time before transfer by checking for transition of TR from 0 to 1
						if ( _DMA->DmaCh [ DmaChannelNum ].CHCR.TR && !( Previous_Value & 0x1000000 ) )
						{
							// dma channel has just been turned on, so set the start time for channel
							_DMA->DmaCh [ DmaChannelNum ].ullStartCycle = *_DebugCycleCount + c_ullSetupTime [ DmaChannelNum ];
						}
						
						
						_DMA->DMA_Update ( DmaChannelNum );
						


					//DmaChannelNum;
					//_DMA->DmaCh [ DmaChannelNum ].CHCR.Value;
					//_DMA->NextEvent_Cycle;
					//_DMA->NextEventCh_Cycle [ DmaChannelNum ];

						break;
						
					
					default:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_UNKNOWN
						debug << "; Invalid";
#endif
		
						// invalid DMA Register
						cout << "\nhps1x64 ALERT: Unknown DMA WRITE @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << " Data=" << Data << "\n";
						break;
					
				}
				
			}
			else
			{
				cout << "\nhps1x64 WARNING: WRITE to unknown DMA Register @ Cycle#" << dec << *_DebugCycleCount << " PC=" << hex << *_DebugPC << " Address=" << Address << "\n";
			}

			break;
	}
	*/


}




void Dma::Update_ICR ( u32 Data )
{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << "\r\nDma::Update_ICR";
#endif

	u32 ICR_Prev;
	
	ICR_Prev = DMARegs0.ICR.Value;
	
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " ICR_Prev=" << hex << ICR_Prev;
#endif

	// bit 31 is "read only"
	DMARegs0.ICR.Value = ( DMARegs0.ICR.Value & 0x80000000 ) | ( DMARegs0.ICR.Value & ~Data & 0x7f000000 ) | ( Data & 0x00ffffff );
	
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " ICR_Reg=" << hex << DMARegs0.ICR.Value;
#endif

	// ***testing*** check if we should clear all the interrupts
	if ( Data & 0x80000000 )
	{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " CLEARALL";
#endif

		// *** testing *** clear all pending interrupts
		DMARegs0.ICR.Value &= 0x00ffffff;
		
		// should this be done for ICR2 also??
		
		// *** testing ***
		//_DMA->ClearInterrupt ();
	}
	
	// check if interrupt was disabled
	// if so, then clear interrupt flag
	//_DMA->ICR_Reg.Value = ( _DMA->ICR_Reg.Value & 0x80ffffff ) | ( ( ( _DMA->ICR_Reg.Value >> 24 ) & ( _DMA->ICR_Reg.Value >> 16 ) & 0x7f ) << 24 );
	
	// check if interrupts have been all acknowledged/cleared
	if ( ! ( DMARegs0.ICR.Value & 0x7f000000 )
#ifdef PS2_COMPILE
			&& ! ( DMARegs1.ICR2.Value & 0x7f000000 )
#endif
		)
	{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " CLEARINT";
#endif

		// all interrupts have been cleared, so there are none pending
		// clear interrupt pending flag
		DMARegs0.ICR.Value &= 0x7fffffff;
		
		// *** testing ***
		//_DMA->ClearInterrupt ();
	}
	else
	{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " CHECKINT";
#endif

		// check that interrupts are enabled
		if ( DMARegs0.ICR.Value & 0x00800000 )
		{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " INTENABLED";
#endif

			// Interrupts are enabled //
			
			// check if interrupt bit 31 should be set for ICR1
			if ( ( DMARegs0.ICR.Value & ( DMARegs0.ICR.Value << 8 ) & 0x7f000000 )
#ifdef PS2_COMPILE
				|| ( DMARegs1.ICR2.Value & ( DMARegs1.ICR2.Value << 8 ) & 0x7f000000 )
#endif
				)
			{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " SETINTBIT";
#endif

				// set the interrupt bit //
				DMARegs0.ICR.Value |= 0x80000000;

#ifdef INT_TRANSITION_ONLY
				// if bit 31 went from 0 to 1, then interrupt
				// ***todo*** for not interrupt regardless since dma might be sending a continuous interrupt
				if ( ( ICR_Prev ^ 0x80000000 ) & DMARegs0.ICR.Value & 0x80000000 )
				{
#endif

#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " SETINT";
#endif
					// *** testing ***
					// ***todo*** there should be any need to set the interrupt again since it is edge triggered - need to take another look here
					// would also need to check if interrupts were enabled for dma
					// *** PROBABLY BE WRONG *** NEEDS FIXING *** probably should only do this on a "DMA Finished" condition
					// and even if it was right, would still need to set bit 31...
					_DMA->SetInterrupt ();
					
#ifdef INT_TRANSITION_ONLY
				}
#endif

			}
		}
		
	}
	
	// bits 6-14 are always zero
	DMARegs0.ICR.Value &= ~0x00007fc0;
	
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " (final) ICR_Reg=" << hex << DMARegs0.ICR.Value;
#endif
}


#ifdef PS2_COMPILE

void Dma::Update_ICR2 ( u32 Data )
{
	u32 ICR_Prev;
	
	ICR_Prev = DMARegs0.ICR.Value;
	
	// bit 31 is "read only"
	DMARegs1.ICR2.Value = ( DMARegs1.ICR2.Value & 0x80000000 ) | ( DMARegs1.ICR2.Value & ~Data & 0x7f000000 ) | ( Data & 0x00ffffff );
	
	// ***testing*** check if we should clear all the interrupts
	if ( Data & 0x80000000 )
	{
		// *** testing *** clear all pending interrupts
		// not sure if this is valid for ICR2 though
		DMARegs1.ICR2.Value &= 0x00ffffff;
		
		// *** testing ***
		//_DMA->ClearInterrupt ();
	}
	
	// check if interrupt was disabled
	// if so, then clear interrupt flag
	//_DMA->ICR_Reg.Value = ( _DMA->ICR_Reg.Value & 0x80ffffff ) | ( ( ( _DMA->ICR_Reg.Value >> 24 ) & ( _DMA->ICR_Reg.Value >> 16 ) & 0x7f ) << 24 );
	
	// check if interrupts have been all acknowledged/cleared
	if ( ! ( DMARegs1.ICR2.Value & 0x7f000000 ) )
	{
		// all interrupts have been cleared, so there are none pending
		// clear interrupt pending flag
		// ***todo*** not sure if this is needed
		DMARegs1.ICR2.Value &= 0x7fffffff;
		
		// check if this is also the case for ICR1
		if ( ! ( DMARegs0.ICR.Value & 0x7f000000 ) )
		{
			// this should DEFINITELY be done for ICR1
			// clear interrupt bit 31 since all interrupts are cleared/acknowledged
			DMARegs0.ICR.Value &= 0x7fffffff;
		}
		
		// *** testing ***
		//_DMA->ClearInterrupt ();
	}
	else
	{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " CHECKINT";
#endif

		// check that interrupts are enabled
		if ( DMARegs0.ICR.Value & 0x00800000 )
		{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " INTENABLED";
#endif

			// Interrupts are enabled //
			
			// check if interrupt bit 31 should be set for ICR1
			if ( ( DMARegs0.ICR.Value & ( DMARegs0.ICR.Value << 8 ) & 0x7f000000 )
#ifdef PS2_COMPILE
				|| ( DMARegs1.ICR2.Value & ( DMARegs1.ICR2.Value << 8 ) & 0x7f000000 )
#endif
				)
			{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " SETINTBIT";
#endif

				// set the interrupt bit //
				DMARegs0.ICR.Value |= 0x80000000;

#ifdef INT_TRANSITION_ONLY
				// if bit 31 went from 0 to 1, then interrupt
				// ***todo*** for now interrupt anyway to test if dma is sending a continuous interrupt
				if ( ( ICR_Prev ^ 0x80000000 ) & DMARegs0.ICR.Value & 0x80000000 )
				{
#endif

#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " SETINT";
#endif
					// *** testing ***
					// ***todo*** there should be any need to set the interrupt again since it is edge triggered - need to take another look here
					// would also need to check if interrupts were enabled for dma
					// *** PROBABLY BE WRONG *** NEEDS FIXING *** probably should only do this on a "DMA Finished" condition
					// and even if it was right, would still need to set bit 31...
					SetInterrupt ();
					
#ifdef INT_TRANSITION_ONLY
				}
#endif

			}
		}
		
		// *** testing ***
		// don't do this for ICR2.. or even ICR1 probably too
		//_DMA->SetInterrupt ();
	}
	
	// bits 6-14 are always zero
	DMARegs1.ICR2.Value &= ~0x00007fc0;
	
	// restart dma9 if interrupt condition cleared ??
	
	/*
	if ( ( _DMA->ICR2_Reg.Value & 0x04000000 ) != 0x04000000 )
	{
		// restart dma#9 ??
		_DMA->Transfer ( 9 );
	}
	*/
	
}

#endif





void Dma::DMA_Update ( int DmaChannelNum )
{
	u32 NewActiveChannel;
	
	// make sure dma channel number is within proper range
	//if ( DmaChannelNum >= c_iNumberOfChannels || DmaChannelNum < 0 ) return;
	
	// recalculate the active channel
	//NewActiveChannel = GetActiveChannel ();
	
	// run the transfer
	Transfer ( DmaChannelNum );

	/*
#ifdef PS2_COMPILE	
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CHCR_2
	debug << "\r\nActiveChannel=" << dec << ActiveChannel << " NewActiveChannel=" << NewActiveChannel << " DmaChannelNum=" << DmaChannelNum;
	debug << " Enabled=" << isEnabled( DmaChannelNum ) << " Active=" << isActive( DmaChannelNum ) << " Score=" << GetPriorityScore ( DmaChannelNum );
	debug << " DMA5Ready=" << Playstation2::SIF::_SIF->EE_DMA_In_Ready () << " DMACE=" << Playstation2::Dma::_DMA->lDMAC_ENABLE;
	debug << " SIFReady=" << Playstation2::SIF::_SIF->lSBUS_F240 << " DMA5E=" << Playstation2::Dma::_DMA->DmaCh [ 5 ].CHCR_Reg.STR;
#endif
#endif

	// check if the channel that is now active is the same as the channel that is being written to
	
#ifdef DMA_UPDATE_PREVIOUS
	if ( NewActiveChannel != DmaChannelNum && isEnabledAndActive( DmaChannelNum ) )
#else
	if ( NewActiveChannel != DmaChannelNum && isEnabledAndActive( NewActiveChannel ) && isEnabledAndActive( DmaChannelNum ) 
#ifdef PS2_COMPILE
		&& ( DmaCh [ DmaChannelNum ].CHCR.StartOnReadySignal == 0 )
#endif
	)
#endif
	{
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CHCR_3
		debug << "; FREEZECPU";
#endif

		// the channel being written to has been activated, but it is of lower priority, so freeze CPU //
		
		// only dma channels 2 and 4 can freeze cpu
		// the channel that is now active has not been started yet //
		ActiveChannel = NewActiveChannel;
		DMA_Run ( ActiveChannel, true );
		// when the channel finishes, it should automatically check for any other pending transfers
	}
	else if ( ( NewActiveChannel == DmaChannelNum )
#ifdef PS2_COMPILE
			|| ( isEnabledAndActive ( DmaChannelNum ) && DmaCh [ DmaChannelNum ].CHCR.StartOnReadySignal )
#endif
	)
	// channel probably just needs to be enabled and not have an event coming up on the next cycle or after (not be in progress already)
	//if ( isEnabledAndActive ( DmaChannelNum ) && ( NextEventCh_Cycle [ DmaChannelNum ] < *_DebugCycleCount || NextEventCh_Cycle [ DmaChannelNum ] == -1LL ) )
	{
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CHCR_4
		debug << "; NEWTRANSFER";
#endif

		// the new transfer is interrupting the one in progress //
		
#ifdef PS2_COMPILE
		if ( isEnabledAndActive ( DmaChannelNum ) && DmaCh [ DmaChannelNum ].CHCR.StartOnReadySignal )
		{
			// set the last dma start address
			StartA = DmaCh [ DmaChannelNum ].MADR;
			
			// start the new dma transfer
			//DMA_Run ( ActiveChannel, false );
			DMA_Run ( DmaChannelNum, false );
		}
		else
		{
#endif

#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CHCR_5
			debug << "\r\nStarting DMA#" << NewActiveChannel;
#endif

			// for ps1, should possibly do this unconditionally for now

			// set the new active channel
			ActiveChannel = NewActiveChannel;
			
			// set the last dma start address
			StartA = DmaCh [ DmaChannelNum ].MADR;
			
			// start the new dma transfer
			//DMA_Run ( ActiveChannel, false );
			DMA_Run ( DmaChannelNum, false );
			// when the channel finishes, it should automatically check for any other pending transfers

#ifdef PS2_COMPILE
		}
#endif
	}
	
#if defined INLINE_DEBUG_WRITE_CHCR_7
	debug << "";	//"; (After) DMA";	// << DmaChannelNum << "_CHCR = " << hex << _DMA->DmaCh [ DmaChannelNum ].CHCR.Value << " NextEvent=" << dec << _DMA->NextEvent_Cycle << " NextChannelEvent=" << _DMA->NextEventCh_Cycle [ DmaChannelNum ];
#endif
	*/

}




u32 Dma::GetPriority ( int Channel )
{
#ifdef PS2_COMPILE
	if ( Channel >= 7 )
	{
		return ( DMARegs1.PCR2.Value >> ( ( Channel - 7 ) << 2 ) ) & 0x7;
	}
#endif

	return ( DMARegs0.PCR.Value >> ( Channel << 2 ) ) & 0x7;
}


u32 Dma::GetPriority_Offset ( int Channel )
{
#ifdef PS2_COMPILE
	if ( Channel >= 7 )
	{
		return ( DMARegs1.PCR2.Value >> 24 ) & 0x7;
	}
#endif

	return ( DMARegs0.PCR.Value >> 28 ) & 0x7;
}

u32 Dma::isEnabled ( int Channel )
{
	if ( ((u32)Channel) >= c_iNumberOfChannels ) return 0;

#ifdef PS2_COMPILE
	// there are 7 PS1 DMA Channels, numbered 0-6
	if ( Channel >= 7 )
	{
		return ( DMARegs1.PCR2.Value >> ( ( ( Channel - 7 ) << 2 ) + 3 ) ) & 0x1;
	}
#endif

	return ( DMARegs0.PCR.Value >> ( ( Channel << 2 ) + 3 ) ) & 0x1;
}


// check if any channel is enabled
u32 Dma::isEnabled ()
{
#ifdef PS2_COMPILE
	return ( DMARegs0.PCR.Value & 0x8888888 ) | ( DMARegs1.PCR2.Value & 0x888888 );
#endif

	return DMARegs0.PCR.Value & 0x8888888;
}




// check if channel has a transfer in progress
u32 Dma::isActive ( int Channel )
{
	if ( ((u32)Channel) >= c_iNumberOfChannels ) return 0;
	
#ifdef PS2_COMPILE
	switch ( Channel )
	{
		/*
		case 4:
			// for now, let's try making auto-dma non-active //
			if ( SPU2::_SPU2->SPU0.isADMATransferMode () )
			{
				return 0;
			}
			
			break;
			
		case 7:
			// for now, let's try making auto-dma non-active //
			if ( SPU2::_SPU2->SPU1.isADMATransferMode () )
			{
				return 0;
			}
			
			break;
		*/
			
		case 9:
			// if EE DMA 5 not ready, then return false
			if ( !Playstation2::SIF::_SIF->EE_DMA_In_Ready () )
			{
				return 0;
			}
			
			break;
			
		case 10:
			if ( !Playstation2::SIF::_SIF->EE_DMA_Out_Ready () )
			{
				return 0;
			}
			
			break;
			
		case 11:
			if ( !Playstation1::SIO::_SIO->SIO2in_DMA_Ready () )
			{
				return 0;
			}
			
			break;
			
		case 12:
			if ( !Playstation1::SIO::_SIO->SIO2out_DMA_Ready () )
			{
				return 0;
			}
			
			break;
	}
#endif
	
	if ( Channel == 0 )
	{
		if ( pRegData [ 0 ]->CHCR.TR /*&& _MDEC->DMA_ReadyForWrite ()*/ )
		{
			return 1;
		}
		
		return 0;
	}
	
	if ( Channel == 1 )
	{
		if ( pRegData [ 1 ]->CHCR.TR /*&& _MDEC->DMA_ReadyForRead ()*/ )
		{
			return 1;
		}
		
		return 0;
	}
	
	return pRegData [ Channel ]->CHCR.TR;
}


// check if any channel on dma device is active
u32 Dma::isActive ()
{
#ifdef PS2_COMPILE
	return ( pRegData [ 0 ]->CHCR.TR /*&& _MDEC->DMA_ReadyForWrite ()*/ ) || ( pRegData [ 1 ]->CHCR.TR /*&& _MDEC->DMA_ReadyForRead ()*/ ) ||
			pRegData [ 2 ]->CHCR.TR || pRegData [ 3 ]->CHCR.TR || pRegData [ 4 ]->CHCR.TR || pRegData [ 5 ]->CHCR.TR || pRegData [ 6 ]->CHCR.TR ||
			pRegData [ 7 ]->CHCR.TR || pRegData [ 8 ]->CHCR.TR ||
			( pRegData [ 9 ]->CHCR.TR && Playstation2::SIF::_SIF->EE_DMA_In_Ready () ) || ( pRegData [ 10 ]->CHCR.TR && Playstation2::SIF::_SIF->EE_DMA_Out_Ready () ) ||
			( pRegData [ 11 ]->CHCR.TR && Playstation1::SIO::_SIO->SIO2in_DMA_Ready () ) || ( pRegData [ 12 ]->CHCR.TR && Playstation1::SIO::_SIO->SIO2out_DMA_Ready () );
#endif

	return ( pRegData [ 0 ]->CHCR.TR /*&& _MDEC->DMA_ReadyForWrite ()*/ ) || ( pRegData [ 1 ]->CHCR.TR /*&& _MDEC->DMA_ReadyForRead ()*/ ) ||
			pRegData [ 2 ]->CHCR.TR || pRegData [ 3 ]->CHCR.TR || pRegData [ 4 ]->CHCR.TR || pRegData [ 5 ]->CHCR.TR || pRegData [ 6 ]->CHCR.TR;
}


u32 Dma::isEnabledAndActive ( int Channel )
{
	return isEnabled ( Channel ) && isActive ( Channel );
}


u32 Dma::isEnabledAndActive ()
{
#ifdef PS2_COMPILE
	return isEnabledAndActive ( 0 ) | isEnabledAndActive ( 1 ) | isEnabledAndActive ( 2 ) | isEnabledAndActive ( 3 )
			| isEnabledAndActive ( 4 ) | isEnabledAndActive ( 5 ) | isEnabledAndActive ( 6 )
			| isEnabledAndActive ( 7 ) | isEnabledAndActive ( 8 ) | isEnabledAndActive ( 9 ) | isEnabledAndActive ( 10 )
			| isEnabledAndActive ( 11 ) | isEnabledAndActive ( 12 );
#endif

	return isEnabledAndActive ( 0 ) | isEnabledAndActive ( 1 ) | isEnabledAndActive ( 2 ) | isEnabledAndActive ( 3 )
			| isEnabledAndActive ( 4 ) | isEnabledAndActive ( 5 ) | isEnabledAndActive ( 6 );
}

// sometimes, a DMA channel is enabled but not currently transferring data
// other channels should be able to transfer data during this time
bool Dma::isTransferring ( int Channel )
{
	switch ( Channel )
	{
#ifdef PS2_COMPILE
		case 3:
			if ( pRegData [ 3 ]->CHCR.StartOnReadySignal )
			{
				return false;
			}
			break;
			
		case 4:
			// for now, let's try making auto-dma non-actively transferring //
			if ( SPU2::_SPU2->SPU0.isADMATransferMode () )
			{
				return false;
			}
			
			break;
			
		case 7:
			// for now, let's try making auto-dma non-actively transferring //
			if ( SPU2::_SPU2->SPU1.isADMATransferMode () )
			{
				return false;
			}
			
			break;
#endif
			
		default:
			break;
	}
	
	return true;
}

// check if channel is the one that has priority
bool Dma::CheckPriority ( int Channel )
{
	u32 ChannelPriority;
	
	// check if channel is enabled
	if ( !isEnabledAndActive ( Channel ) ) return false;
	
	ChannelPriority = GetPriority ( Channel );

	// higher numbered channels would need priority equal to or lesser to have priority
	for ( int i = Channel + 1; i < c_iNumberOfChannels; i++ )
	{
		// check if priority of channel is equal or lesser and would need to be enabled
		if ( GetPriority ( i ) <= ChannelPriority && isEnabledAndActive ( i )
#ifdef USE_NEW_SCORING_SYSTEM
			&& isTransferring ( i )
#endif
			)
		{
			return false;
		}
	}
	
	// lower numbered channels would need priority stricly lesser to have priority
	for ( int i = Channel - 1; i >= 0; i-- )
	{
		// check if priority of channel is strictly lesser and would need to be enabled
		if ( GetPriority ( i ) < ChannelPriority && isEnabledAndActive ( i )
#ifdef USE_NEW_SCORING_SYSTEM
		&& isTransferring ( i )
#endif
			)
		{
			return false;
		}
	}
	
	return true;
}





//const char* DmaChannelLogText [ 7 ] = { "DMA0_Log.txt", "DMA1_Log.txt", "DMA2_Log.txt", "DMA3_Log.txt", "DMA4_Log.txt", "DMA5_Log.txt", "DMA6_Log.txt" };

DmaChannel::DmaChannel ()
{
	// set the dma channel number
	Number = Count++;
	
	Reset ();
}


void DmaChannel::Reset ()
{
	// initialize MADR, BCR, & CHCR
	//MADR = 0;
	//BCR.Value = 0;
	//CHCR.Value = 0;
	memset ( this, 0, sizeof(DmaChannel) );
}


void Dma::SuspendTransfer ( int index )
{
#ifdef INLINE_DEBUG_COMPLETE
	debug << "\r\n\r\nDMA" << dec << index << "::SuspendTransfer; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << hex << ";(before) Intc_Stat=" << *_Intc_Stat << "; _Intc_Mask=" << *_Intc_Mask << "; _R3000a_Status=" << *_R3000a_Status;
#endif

	// allow all dma's to operate at next opportunity
	SelectedDMA_Bitmap = 0xffffffff;
	
	// clear bit in bitmap for dma channel
	ChannelEnable_Bitmap &= ~( 1 << index );
	
	// clear bit for the transfer for channel being in progress
	// transfer for channel is no longer in progress
	Channels_InProgress &= ~( 1 << index );
	
	// stop the dma channel
	// note: both bits 24 and 28 get reset after the transfer
	pRegData [ index ]->CHCR.Value &= ~( ( 1L << 0x18 ) | ( 1L << 0x1c ) );	//dma->n_channelcontrol &= ~( ( 1L << 0x18 ) | ( 1L << 0x1c ) );

}


// interrupt for auto dma?
void Dma::AutoDMA_Interrupt ( int index )
{
#ifdef INLINE_DEBUG_COMPLETE
	debug << "\r\n\r\nDMA" << dec << index << "::Finished; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << hex << ";(before) Intc_Stat=" << *_Intc_Stat << "; _Intc_Mask=" << *_Intc_Mask << "; _R3000a_Status=" << *_R3000a_Status;
	debug << "; ICR=" << DMARegs0.ICR.Value;
#ifdef PS2_COMPILE
	debug << "; ICR2=" << DMARegs1.ICR2.Value;
#endif
#endif
	
	u32 ICR_Prev;
	
	
	// *** testing *** if the dma is finished, then bcr should be zero
	// note: this actually depends on the dma channel
	// note: only sync mode 1 decrements the upper 16-bits of BCR, all other transfer modes leave it as is
	//DmaCh [ index ].BCR.Value = 0;

	// get previous value of ICR
	ICR_Prev = DMARegs0.ICR.Value;
	
#ifdef PS2_COMPILE
	if ( index <= 6 )
	{
#endif
	
#ifdef SET_INTPENDING_ONLY_WHEN_ENABLED
	// check if dma interrupts are enabled for channel
	if ( DMARegs0.ICR.Value & ( 1 << ( 16 + index ) ) )
#endif
	{

		// set interrupt pending for channel
		DMARegs0.ICR.Value |= ( 1 << ( 24 + index ) );
		
#ifdef SET_INTPENDING_ONLY_WHEN_ENABLED
		// only allow interrupt pending if the interrupt is enabled
		DMARegs0.ICR.Value &= ( ( DMARegs0.ICR.Value << 8 ) | 0x80ffffff );

		// check if there are any interrupts pending
		if ( DMARegs0.ICR.Value & 0x7f000000 )
#else
		// check if a dma interrupt is both pending and enabled
		if ( DMARegs0.ICR.Value & ( DMARegs0.ICR.Value << 8 ) & 0x7f000000 )
#endif
		{
			// set interrupt pending flag
			DMARegs0.ICR.Value |= 0x80000000;
		}
		else
		{
			// clear interrupt pending flag
			DMARegs0.ICR.Value &= 0x7fffffff;
			
			
			// *** TESTING ***
			//ClearInterrupt ();
		}
		
		// check if dma interrupts are enabled globally
		// also check that bit 31 transitioned from 0 to 1
		//if ( ( ( ICR_Reg.Value >> 23 ) & 1 ) )
		if ( ( ! ( ICR_Prev & 0x80000000 ) ) && ( ( DMARegs0.ICR.Value & 0x80800000 ) == 0x80800000 ) )
		{
#ifdef INLINE_DEBUG_COMPLETE
	debug << "; INT";
#endif

			// check if dma interrupts are enabled for channel
			
			// send interrupt signal
			SetInterrupt ();
		}
	}
	
#ifdef PS2_COMPILE
	}
	else
	{
		
#ifdef SET_INTPENDING_ONLY_WHEN_ENABLED
		// check if dma interrupts are enabled for channel
		if ( DMARegs1.ICR2.Value & ( 1 << ( ( 16 + index ) - 7 ) ) )
#endif
		{
			// set interrupt pending for channel
			DMARegs1.ICR2.Value |= ( 1 << ( ( 24 + index ) - 7 ) );
			
#ifdef SET_INTPENDING_ONLY_WHEN_ENABLED
			// only allow interrupt pending if the interrupt is enabled
			DMARegs1.ICR2.Value &= ( ( DMARegs1.ICR2.Value << 8 ) | 0x80ffffff );

			// check if there are any interrupts pending for ICR1 OR ICR2
			if ( ( ( DMARegs0.ICR.Value & 0x7f000000 ) || ( DMARegs1.ICR2.Value & 0x7f000000 ) ) && ( ( DMARegs0.ICR.Value & 0x00800000 ) == 0x00800000 ) )
#else
			// check if a dma interrupt is both pending and enabled
			if ( ( ( DMARegs0.ICR.Value & ( DMARegs0.ICR.Value << 8 ) & 0x7f000000 ) || ( DMARegs1.ICR2.Value & ( DMARegs1.ICR2.Value << 8 ) & 0x7f000000 ) ) && ( ( DMARegs0.ICR.Value & 0x00800000 ) == 0x00800000 ) )
#endif
			{
				// should definitely set interrupt pending flag for ICR1 probably
				DMARegs0.ICR.Value |= 0x80000000;
				
#ifdef ENABLE_ICR2_BIT31
				// set interrupt pending flag
				// need to test this one, might just need to set for ICR1
				DMARegs1.ICR2.Value |= 0x80000000;
#endif
			}
			else
			{
				// only clear interrupt pending for both ICR1 and ICR2
				DMARegs0.ICR.Value &= 0x7fffffff;
				
				// clear interrupt pending flag
				DMARegs1.ICR2.Value &= 0x7fffffff;
				
				
				// *** TESTING ***
				//ClearInterrupt ();
			}
			
			// check if dma interrupts are enabled globally
			// also check that bit 31 transitioned from 0 to 1
			// do this for just ICR1 to check for interrupt
			if ( ( ! ( ICR_Prev & 0x80000000 ) ) && ( ( DMARegs0.ICR.Value & 0x80800000 ) == 0x80800000 ) )
			{
#ifdef INLINE_DEBUG_COMPLETE
	debug << "; INT";
#endif

				// check if dma interrupts are enabled for channel
				
				// send interrupt signal
				SetInterrupt ();
			}
		}
			
	}
#endif

}




// returns interrupt status
void Dma::DMA_Finished ( int index, bool SuppressDMARestart, bool SuppressEventUpdate, bool SuppressDMAStop )
{
#ifdef INLINE_DEBUG_COMPLETE
	debug << "\r\n\r\nDMA" << dec << index << "::Finished; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << hex << ";(before) Intc_Stat=" << *_Intc_Stat << "; _Intc_Mask=" << *_Intc_Mask << "; _R3000a_Status=" << *_R3000a_Status;
	debug << "; ICR=" << DMARegs0.ICR.Value;
#ifdef PS2_COMPILE
	debug << "; ICR2=" << DMARegs1.ICR2.Value;
#endif
#endif
	
	u32 ICR_Prev;
	
	EndA = pRegData [ index ]->MADR + pRegData [ index ]->BCR.BS;
	
	// allow all dma's to operate at next opportunity
	SelectedDMA_Bitmap = 0xffffffff;
	
	// clear bit in bitmap for dma channel
	ChannelEnable_Bitmap &= ~( 1 << index );
	
	// clear bit for the transfer for channel being in progress
	// transfer for channel is no longer in progress
	Channels_InProgress &= ~( 1 << index );
	
	// channel is done, so no more events
	SetNextEventCh_Cycle ( -1ULL, index );
	
#ifdef PS2_COMPILE
	if ( !SuppressDMAStop )
#endif
	{

		// stop the dma channel
		// note: both bits 24 and 28 get reset after the transfer
		pRegData [ index ]->CHCR.Value &= ~( ( 1L << 0x18 ) | ( 1L << 0x1c ) );	//dma->n_channelcontrol &= ~( ( 1L << 0x18 ) | ( 1L << 0x1c ) );
		
#ifdef CLEAR_BIT30_ON_FINISH
		// bit 30 should get reset after the transfer also ??
		pRegData [ index ]->CHCR.Value &= ~( 1L << 0x1e );	//dma->n_channelcontrol &= ~( ( 1L << 0x18 ) | ( 1L << 0x1c ) );
#endif
		
	}
	
	// *** testing *** if the dma is finished, then bcr should be zero
	// note: this actually depends on the dma channel
	// note: only sync mode 1 decrements the upper 16-bits of BCR, all other transfer modes leave it as is
	//DmaCh [ index ].BCR.Value = 0;

	// get previous value of ICR
	ICR_Prev = DMARegs0.ICR.Value;
	
#ifdef PS2_COMPILE
	if ( index <= 6 )
	{
#endif
	
#ifdef SET_INTPENDING_ONLY_WHEN_ENABLED
	// check if dma interrupts are enabled for channel
	if ( DMARegs0.ICR.Value & ( 1 << ( 16 + index ) ) )
#endif
	{
		// set interrupt pending for channel
		DMARegs0.ICR.Value |= ( 1 << ( 24 + index ) );
		
#ifdef SET_INTPENDING_ONLY_WHEN_ENABLED
		// only allow interrupt pending if the interrupt is enabled
		DMARegs0.ICR.Value &= ( ( DMARegs0.ICR.Value << 8 ) | 0x80ffffff );

		// check if there are any interrupts pending
		if ( DMARegs0.ICR.Value & 0x7f000000 )
#else
		// check if a dma interrupt is both pending and enabled
		if ( DMARegs0.ICR.Value & ( DMARegs0.ICR.Value << 8 ) & 0x7f000000 )
#endif
		{
			// set interrupt pending flag
			DMARegs0.ICR.Value |= 0x80000000;
		}
		else
		{
			// clear interrupt pending flag
			DMARegs0.ICR.Value &= 0x7fffffff;
			
			
			// *** TESTING ***
			//ClearInterrupt ();
		}
		
		// check if dma interrupts are enabled globally
		// also check that bit 31 transitioned from 0 to 1
		//if ( ( ( ICR_Reg.Value >> 23 ) & 1 ) )
		if ( ( ! ( ICR_Prev & 0x80000000 ) ) && ( ( DMARegs0.ICR.Value & 0x80800000 ) == 0x80800000 ) )
		{
#ifdef INLINE_DEBUG_COMPLETE
	debug << "; INT";
#endif

			// check if dma interrupts are enabled for channel
			
			// send interrupt signal
			SetInterrupt ();
		}
	}
	
#ifdef PS2_COMPILE
	}
	else
	{
		
#ifdef SET_INTPENDING_ONLY_WHEN_ENABLED
		// check if dma interrupts are enabled for channel
		if ( DMARegs1.ICR2.Value & ( 1 << ( ( 16 + index ) - 7 ) ) )
#endif
		{
			// set interrupt pending for channel
			DMARegs1.ICR2.Value |= ( 1 << ( ( 24 + index ) - 7 ) );
			
#ifdef SET_INTPENDING_ONLY_WHEN_ENABLED
			// only allow interrupt pending if the interrupt is enabled
			DMARegs1.ICR2.Value &= ( ( DMARegs1.ICR2.Value << 8 ) | 0x80ffffff );

			// check if there are any interrupts pending for ICR1 OR ICR2
			if ( ( ( DMARegs0.ICR.Value & 0x7f000000 ) || ( DMARegs1.ICR2.Value & 0x7f000000 ) ) && ( ( DMARegs0.ICR.Value & 0x00800000 ) == 0x00800000 ) )
#else
			// check if a dma interrupt is both pending and enabled
			if ( ( ( DMARegs0.ICR.Value & ( DMARegs0.ICR.Value << 8 ) & 0x7f000000 ) || ( DMARegs1.ICR2.Value & ( DMARegs1.ICR2.Value << 8 ) & 0x7f000000 ) ) && ( ( DMARegs0.ICR.Value & 0x00800000 ) == 0x00800000 ) )
#endif
			{
				// should definitely set interrupt pending flag for ICR1 probably
				DMARegs0.ICR.Value |= 0x80000000;
				
#ifdef ENABLE_ICR2_BIT31
				// set interrupt pending flag
				// need to test this one, might just need to set for ICR1
				DMARegs1.ICR2_Reg.Value |= 0x80000000;
#endif
			}
			else
			{
				// only clear interrupt pending for both ICR1 and ICR2
				DMARegs0.ICR.Value &= 0x7fffffff;
				
				// clear interrupt pending flag
				DMARegs1.ICR2.Value &= 0x7fffffff;
				
				
				// *** TESTING ***
				//ClearInterrupt ();
			}
			
			// check if dma interrupts are enabled globally
			// also check that bit 31 transitioned from 0 to 1
			// do this for just ICR1 to check for interrupt
			if ( ( ! ( ICR_Prev & 0x80000000 ) ) && ( ( DMARegs0.ICR.Value & 0x80800000 ) == 0x80800000 ) )
			{
#ifdef INLINE_DEBUG_COMPLETE
	debug << "; INT";
#endif

				// check if dma interrupts are enabled for channel
				
				// send interrupt signal
				SetInterrupt ();
			}
		}
			
	}

	
	

#ifdef PS2_COMPILE
	switch ( index )
	{
		case 9:
			// IOP->EE
			//Playstation2::SIF::_SIF->lSBUS_F240 &= ~0x20;
			
#ifdef ENABLE_DMA_END_REMOTE
			// ***TESTING*** suspend channel#5 on EE
			//Playstation2::SIF::_SIF->SuspendTransfer_EE ( 5 );
#endif

			break;
			
		case 10:
			// EE->IOP
			//Playstation2::SIF::_SIF->lSBUS_F240 &= ~0x40;
			
#ifdef ENABLE_DMA_END_REMOTE
			// ***TESTING*** suspend channel#6 on EE
			Playstation2::SIF::_SIF->SuspendTransfer_EE ( 6 );
#endif
			
			break;
	}
#endif
	
	
#ifdef ENABLE_SIF_DMA_SYNC
	// if dma channel 9 or 10 is finished, then clear the correct bit in SBUS_F240
	// also have SIF recheck for any transfers that need to be restarted
	switch ( index )
	{
		case 9:
			// IOP->EE
			Playstation2::SIF::_SIF->lSBUS_F240 &= ~0x20;
			
			break;
			
		case 10:
			// EE->IOP
			Playstation2::SIF::_SIF->lSBUS_F240 &= ~0x40;
			
			break;
	}
#endif

#if defined ENABLE_SIF_DMA_TIMING || defined ENABLE_SIF_DMA_SYNC
	// if channel#10, then check if channel#9 (IOP->EE) is ready to go since it would have been held up
	if ( index == 10 )
	{
		Playstation2::SIF::_SIF->Check_TransferFromIOP ();
	}
#endif

#endif



	/*
	// now that the dma channel is finished, check what channel is next and run it immediately
	ActiveChannel = GetActiveChannel ();
	
	
	if ( !SuppressDMARestart )
	{
#ifdef INLINE_DEBUG_COMPLETE
	debug << "\r\n; DMARESTART!!!";
	debug << " ActiveChannel=" << ActiveChannel;
#endif

		DMA_Run ( ActiveChannel );
	}
	
	
	// will want to suppress event updates for now when finished event is called before current cycle has run (dma10)
	if ( !SuppressEventUpdate )
	{
		// make sure the cycle number for the next dma event is updated
		Update_NextEventCycle ();
	}
	
#ifdef INLINE_DEBUG_COMPLETE
	debug << hex << ";(after) Intc_Stat=" << *_Intc_Stat << "; _Intc_Mask=" << *_Intc_Mask << "; _R3000a_Status=" << *_R3000a_Status;
	debug << "; ICR=" << ICR_Reg.Value;
#ifdef PS2_COMPILE
	debug << "; ICR2=" << ICR2_Reg.Value;
#endif
#endif
	*/


	// no more events for this particular channel, cuz it is finished
	//SetNextEventCh ( 0, index );
	
	// check which channel is next to fire
	/*
	if ( isEnabledAndActive () )
	{
#ifdef INLINE_DEBUG_COMPLETE
	debug << "; EnabledAndActive";
#endif

		// there is a dma channel next in line to fire //
		
		// get the next dma channel to run
		for ( int i = 0; i < c_iNumberOfChannels; i++ )
		{
			if ( CheckPriority ( i ) )
			{
#ifdef INLINE_DEBUG_COMPLETE
	debug << "; NextChannel=" << i;
#endif

				// found next channel to fire
				SetNextEventCh ( 1, i );
				break;
			}
		}
		
	}
	*/
	
	
	//return Interrupt;
	
	// *** TODO *** figure out issue with interrupts
	//return 0;
	
	//return DMA_Interrupt_Update ();	//dma_interrupt_update();
	//dma_stop_timer( index );
}


void Dma::SetNextEventCh ( u64 Cycles, u32 Channel )
{
	NextEventCh_Cycle [ Channel ] = Cycles + *_DebugCycleCount;
	
	Update_NextEventCycle ();
	
	/*
	if ( NextEventCh_Cycle [ Channel ] <= NextEvent_Cycle )
	{
		NextEvent_Cycle = NextEventCh_Cycle [ Channel ];
	}
	
	if ( NextEvent_Cycle <= *_NextSystemEvent )
	{
		*_NextSystemEvent = NextEvent_Cycle;
	}
	*/
}


void Dma::SetNextEventCh_Cycle ( u64 Cycle, u32 Channel )
{
	NextEventCh_Cycle [ Channel ] = Cycle;
	
	// rather than loop through all the channels, makes more sense to just update the values
	Update_NextEventCycle ();
	
	/*
	if ( Cycle <= NextEvent_Cycle )
	{
		NextEvent_Cycle = Cycle;
	}
	
	if ( NextEvent_Cycle <= *_NextSystemEvent )
	{
		*_NextSystemEvent = NextEvent_Cycle;
	}
	*/
}

void Dma::Update_NextEventCycle ()
{
#ifdef INLINE_DEBUG_EVENT
	debug << "\r\nDma::Update_NextEventCycle";
	debug << " (before)NextSystemEvent=" << dec << *_NextSystemEvent;
	debug << " Cycle#" << dec << *_DebugCycleCount;
#endif

	NextEvent_Cycle = -1LL;
	
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		//if ( NextEventCh_Cycle [ i ] > *_SystemCycleCount && ( NextEventCh_Cycle [ i ] < NextEvent_Cycle || NextEvent_Cycle <= *_SystemCycleCount ) )
		if ( NextEventCh_Cycle [ i ] < NextEvent_Cycle )
		{
			// the next event is the next event for device
			NextEvent_Cycle = NextEventCh_Cycle [ i ];
		}
	}
	
	//_BUS->NextEvent_Cycle = NextEvent_Cycle;

#ifdef INLINE_DEBUG_EVENT
	debug << " NextDMAEvent=" << dec << NextEvent_Cycle;
#endif

	if ( NextEvent_Cycle < *_NextSystemEvent )
	{
		*_NextSystemEvent = NextEvent_Cycle;
		*_NextEventIdx = NextEvent_Idx;
	}
	
	//cout << "\nTEST: dma1 next event cycle=" << dec << NextEventCh_Cycle [ 1 ];
	//cout << "\nTEST: dma next event cycle=" << dec << NextEvent_Cycle;
	
#ifdef INLINE_DEBUG_EVENT
	debug << " (after)NextSystemEvent=" << dec << *_NextSystemEvent;
#endif
}




static void Dma::DebugWindow_Enable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	static const char* DebugWindow_Caption = "PS1 DMA Debug Window";
	static const int DebugWindow_X = 10;
	static const int DebugWindow_Y = 10;
	static const int DebugWindow_Width = 250;
	static const int DebugWindow_Height = 300;
	
	static const int DMAList_X = 0;
	static const int DMAList_Y = 0;
	static const int DMAList_Width = 220;
	static const int DMAList_Height = 250;
	
	int i;
	stringstream ss;
	
	if ( !DebugWindow_Enabled )
	{
		// create the main debug window
		DebugWindow = new WindowClass::Window ();
		DebugWindow->Create ( DebugWindow_Caption, DebugWindow_X, DebugWindow_Y, DebugWindow_Width, DebugWindow_Height );
		DebugWindow->DisableCloseButton ();
		
		// create "value lists"
		DMA_ValueList = new DebugValueList<u32> ();
		DMA_ValueList->Create ( DebugWindow, DMAList_X, DMAList_Y, DMAList_Width, DMAList_Height );
		
		DMA_ValueList->AddVariable ( "DMA_ICR", &( _DMA->DMARegs0.ICR.Value ) );
		DMA_ValueList->AddVariable ( "DMA_PCR", &( _DMA->DMARegs0.PCR.Value ) );
		
#ifdef PS2_COMPILE
		DMA_ValueList->AddVariable ( "DMA_ICR2", &( _DMA->DMARegs1.ICR2.Value ) );
		DMA_ValueList->AddVariable ( "DMA_PCR2", &( _DMA->DMARegs1.PCR2.Value ) );
		DMA_ValueList->AddVariable ( "SPU2", (u32*) &( SPU2::_SPU2->NextEvent_Cycle ) );
#endif

		for ( i = 0; i < NumberOfChannels; i++ )
		{
			ss.str ("");
			ss << "DMA" << i << "_MADR";
			DMA_ValueList->AddVariable ( ss.str().c_str(), &( pRegData [ i ]->MADR ) );
			
			ss.str ("");
			ss << "DMA" << i << "_BCR";
			DMA_ValueList->AddVariable ( ss.str().c_str(), &( pRegData [ i ]->BCR.Value ) );
			
			ss.str ("");
			ss << "DMA" << i << "_CHCR";
			DMA_ValueList->AddVariable ( ss.str().c_str(), &( pRegData [ i ]->CHCR.Value ) );
		}
		
		// add start and end addresses for dma transfers
		DMA_ValueList->AddVariable ( "StartA", &( _DMA->StartA ) );
		DMA_ValueList->AddVariable ( "EndA", &( _DMA->EndA ) );
		
		// add primitive count and frame count here for now
		DMA_ValueList->AddVariable ( "PCount", &( _GPU->Primitive_Count ) );
		DMA_ValueList->AddVariable ( "FCount", &( _GPU->Frame_Count ) );
		
		DebugWindow_Enabled = true;
		
		// update the value lists
		DebugWindow_Update ();
	}

#endif

}

static void Dma::DebugWindow_Disable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	if ( DebugWindow_Enabled )
	{
		delete DebugWindow;
		delete DMA_ValueList;
	
		// disable debug window
		DebugWindow_Enabled = false;
	}
	
#endif

}

static void Dma::DebugWindow_Update ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	if ( DebugWindow_Enabled )
	{
		DMA_ValueList->Update();
	}
	
#endif

}



