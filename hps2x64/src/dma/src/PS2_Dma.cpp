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


#include "PS2_Dma.h"
#include "PS2_GPU.h"
#include "PS2_SIF.h"
#include "PS2_IPU.h"

#include "VU.h"

//using namespace Playstation2;

//#define ENABLE_SIF_DMA_TIMING
//#define ENABLE_SIF_DMA_SYNC

#define TEST_ASYNC_DMA
#define TEST_ASYNC_DMA_STAGE2


//#define ENABLE_DMA_END_REMOTE


#define VERBOSE_CHAIN

//#define VERBOSE_CYCLESTEAL
//#define VERBOSE_MFIFO
#define VERBOSE_STS
#define VERBOSE_STD
#define VERBOSE_RCYC

/*
#define VERBOSE_NORMAL_TOMEM
#define VERBOSE_NORMAL_FROMMEM
#define VERBOSE_CHAIN_FROMMEM
#define VERBOSE_CHAIN_TOMEM
*/

//#define PRIORITY_DMA0
// don't want this to race through and get invalid vif code errors
//#define PRIORITY_DMA1


//#define VERBOSE_DMA_1_7_TO_MEM

//#define VERBOSE_DMA_UNDERFLOW

#define CLEAR_QWC_ON_COMPLETE

//#define INT_ON_DMA5_STOP

//#define UPDATE_QWC_ONLYONSLICE

//#define CLEAR_TADR_ON_COMPLETE
//#define SET_QWC_CHAIN_TRANSFER


#define FIX_UNALIGNED_READ
#define FIX_UNALIGNED_WRITE



#ifdef _DEBUG_VERSION_

// enable debugging

#define INLINE_DEBUG_ENABLE

/*
#define INLINE_DEBUG_WRITE
#define INLINE_DEBUG_READ
#define INLINE_DEBUG_START
#define INLINE_DEBUG_END

#define INLINE_DEBUG_SPR_IN
#define INLINE_DEBUG_SPR_OUT

#define INLINE_DEBUG_TRANSFER_NORMAL_TOMEM

#define INLINE_DEBUG_TRANSFER
#define INLINE_DEBUG_TRANSFER_NORMAL
#define INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
#define INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
#define INLINE_DEBUG_TRANSFER_CHAIN
#define INLINE_DEBUG_TRANSFER_CHAIN_TOMEM
#define INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
#define INLINE_DEBUG_TRANSFER_CHAIN_0
#define INLINE_DEBUG_TRANSFER_CHAIN_1
#define INLINE_DEBUG_TRANSFER_CHAIN_2
#define INLINE_DEBUG_TRANSFER_CHAIN_3
#define INLINE_DEBUG_TRANSFER_CHAIN_4
#define INLINE_DEBUG_TRANSFER_CHAIN_5
#define INLINE_DEBUG_TRANSFER_CHAIN_6
#define INLINE_DEBUG_TRANSFER_CHAIN_7
#define INLINE_DEBUG_TRANSFER_CHAIN_8

#define INLINE_DEBUG_RUN_DMA5



//#define INLINE_DEBUG_NEXTEVENT
//#define INLINE_DEBUG_TEST5
//#define INLINE_DEBUG_INT



//#define INLINE_DEBUG_COMPLETE

//#define INLINE_DEBUG_READ_CHCR
//#define INLINE_DEBUG_WRITE_CHCR
//#define INLINE_DEBUG_READ_CTRL
//#define INLINE_DEBUG_WRITE_CTRL
//#define INLINE_DEBUG_READ_INVALID
//#define INLINE_DEBUG_WRITE_INVALID

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
//#define INLINE_DEBUG_RUN_DMA4
#define INLINE_DEBUG_RUN
//#define INLINE_DEBUG_CD
//#define INLINE_DEBUG_SPU
//#define INLINE_DEBUG_ACK
*/

#endif


namespace Playstation2
{

u32* Dma::_SBUS_F240;

u32* Dma::_DebugPC;
u64* Dma::_DebugCycleCount;
u32* Dma::_NextEventIdx;

u32* Dma::_R5900_Status;

//u32* Dma::_Intc_Master;
u32* Dma::_Intc_Stat;
u32* Dma::_Intc_Mask;
u32* Dma::_R5900_Cause_13;
u32* Dma::_R5900_Status_12;
u64* Dma::_ProcStatus;

u32* Dma::_CPCOND0_Out;


Debug::Log Dma::debug;

Dma *Dma::_DMA;

bool Dma::DebugWindow_Enabled;
WindowClass::Window *Dma::DebugWindow;
DebugValueList<u32> *Dma::DMA_ValueList;



int DmaChannel::Count = 0;


u64* Dma::_NextSystemEvent;


DataBus *Dma::_BUS;
MDEC *Dma::_MDEC;
GPU *Dma::_GPU;
//R5900::Cpu *Dma::_CPU;


static DmaChannel::RegData* Dma::pRegData [ Dma::c_iNumberOfChannels ];
static Dma::DMARegs_t* Dma::pDMARegs;



const char* Dma::DmaCh_Names [ c_iNumberOfChannels ] = { "VU0/VIF0", "VU1/VIF1", "GPU/GIF", "MDEC/IPU out", "MDEC/IPU in", "SIF0 (from SIF/IOP)", "SIF1 (to SIF/IOP)",
													"SIF2", "SPR out", "SPR in" };

													

const char* Dma::Reg_Names [ 32 * 16 ] = { "D0_CHCR", "D0_MADR", "D0_QWC", "D0_TADR", "D0_ASR0", "D0_ASR1", "D0_Res0", "D0_Res1", "D0_Res2", "D0_Res3", "D0_Res4", "D0_Res5", "D0_Res6", "D0_Res7", "D0_Res8", "D0_Res9",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"D1_CHCR", "D1_MADR", "D1_QWC", "D1_TADR", "D1_ASR0", "D1_ASR1", "D1_Res0", "D1_Res1", "D1_Res2", "D1_Res3", "D1_Res4", "D1_Res5", "D1_Res6", "D1_Res7", "D1_Res8", "D1_Res9",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"D2_CHCR", "D2_MADR", "D2_QWC", "D2_TADR", "D2_ASR0", "D2_ASR1", "D2_Res0", "D2_Res1", "D2_Res2", "D2_Res3", "D2_Res4", "D2_Res5", "D2_Res6", "D2_Res7", "D2_Res8", "D2_Res9",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"D3_CHCR", "D3_MADR", "D3_QWC", "D3_ResA", "D3_ResB", "D3_ResC", "D3_Res0", "D3_Res1", "D3_Res2", "D3_Res3", "D3_Res4", "D3_Res5", "D3_Res6", "D3_Res7", "D3_Res8", "D3_Res9",
										"D4_CHCR", "D4_MADR", "D4_QWC", "D4_TADR", "D4_ResB", "D4_ResC", "D4_Res0", "D4_Res1", "D4_Res2", "D4_Res3", "D4_Res4", "D4_Res5", "D4_Res6", "D4_Res7", "D4_Res8", "D4_Res9",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"D5_CHCR", "D5_MADR", "D5_QWC", "D5_ResA", "D5_ResB", "D5_ResC", "D5_Res0", "D5_Res1", "D5_Res2", "D5_Res3", "D5_Res4", "D5_Res5", "D5_Res6", "D5_Res7", "D5_Res8", "D5_Res9",
										"D6_CHCR", "D6_MADR", "D6_QWC", "D6_TADR", "D6_ResB", "D6_ResC", "D6_Res0", "D6_Res1", "D6_Res2", "D6_Res3", "D6_Res4", "D6_Res5", "D6_Res6", "D6_Res7", "D6_Res8", "D6_Res9",
										"D7_CHCR", "D7_MADR", "D7_QWC", "D7_ResA", "D7_ResB", "D7_ResC", "D7_Res0", "D7_Res1", "D7_Res2", "D7_Res3", "D7_Res4", "D7_Res5", "D7_Res6", "D7_Res7", "D7_Res8", "D7_Res9",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"D8_CHCR", "D8_MADR", "D8_QWC", "D8_ResA", "D8_ResB", "D8_ResC", "D8_Res0", "D8_Res1", "D8_SADR", "D8_Res3", "D8_Res4", "D8_Res5", "D8_Res6", "D8_Res7", "D8_Res8", "D8_Res9",
										"D9_CHCR", "D9_MADR", "D9_QWC", "D9_TADR", "D9_ResB", "D9_ResC", "D9_Res0", "D9_Res1", "D9_SADR", "D9_Res3", "D9_Res4", "D9_Res5", "D9_Res6", "D9_Res7", "D9_Res8", "D9_Res9",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"D_CTRL", "D_STAT", "D_PCR", "D_SQWC", "D_RBSR", "D_RBOR", "D_STADR", "D_Res7", "D_Res8", "D_Res9", "D_ResA", "D_ResB", "D_ResC", "D_ResD", "D_ResE", "D_ResF"
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "ENABLER", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "ENABLEW", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF"
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										};


const u64 Dma::c_iDmaSetupTime [ c_iNumberOfChannels ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

const u64 Dma::c_iDmaTransferTimePerQwc [ c_iNumberOfChannels ] = {
c_iVU0_TransferTime,
c_iVU1_TransferTime,
c_iGIF_TransferTime,
c_iMDECout_TransferTime,
c_iMDECin_TransferTime,
c_iSIF0_TransferTime,
c_iSIF1_TransferTime,
1,	// sif2
1,	// spr out
1 };	// spr in

//const u64 Dma::c_iDmaTransferTimePerQwc [ c_iNumberOfChannels ] = { 32, 32, 32, 32, 32, 32, 32, 32, 32, 32 };
//const u64 Dma::c_iDmaTransferTimePerQwc [ c_iNumberOfChannels ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

const u64 Dma::c_iDeviceBufferSize [ c_iNumberOfChannels ] = { 8, 16, 16, 8, 8, 8, 8, 8, 1000000, 1000000 };

static Dma::fnReady Dma::cbReady [ c_iNumberOfChannels ] = {
// channel 0 - VU0/VIF0
VU0::DMA_Write_Ready,
// channel 1 - VU1/VIF1
VU1::DMA_Write_Ready,
// channel 2 - GPU/GIF
GPU::DMA_Write_Ready,
// channel 3 - MDEC/IPU out
IPU::DMA_Read_Ready,
// channel 4 - MDEC/IPU in
IPU::DMA_Write_Ready,
// channel 5 - SIF0 (from SIF/IOP)
SIF::IOP_DMA_Out_Ready,
// channel 6 - SIF1 (to SIF/IOP)
SIF::IOP_DMA_In_Ready,
// channel 7 - SIF2
NULL,
// channel 8 - SPR out
SPRout_DMA_Ready,
// channel 9 - SPR in
SPRin_DMA_Ready
};

static Dma::fnTransfer_FromMemory Dma::cbTransfer_FromMemory [ c_iNumberOfChannels ] = {
// channel 0 - VU0/VIF0
VU0::DMA_WriteBlock,
// channel 1 - VU1/VIF1
VU1::DMA_WriteBlock,
// channel 2 - GPU/GIF
GPU::DMA_WriteBlock,
// channel 3 - MDEC/IPU out
NULL,
// channel 4 - MDEC/IPU in
IPU::DMA_WriteBlock,
// channel 5 - SIF0 (from SIF/IOP)
NULL,
// channel 6 - SIF1 (to SIF/IOP)
SIF::EE_DMA_WriteBlock,
// channel 7 - SIF2
NULL,
// channel 8 - SPR out
SPRout_DMA_Read,
// channel 9 - SPR in
SPRin_DMA_Write
};


static Dma::fnTransfer_FromMemory Dma::cbTransfer_ToMemory [ c_iNumberOfChannels ] = {
// channel 0 - VU0/VIF0
NULL,
// channel 1 - VU1/VIF1
VU1::DMA_ReadBlock,
// channel 2 - GPU/GIF
NULL,
// channel 3 - MDEC/IPU out
IPU::DMA_ReadBlock,
// channel 4 - MDEC/IPU in
NULL,
// channel 5 - SIF0 (from SIF/IOP)
NULL,
// channel 6 - SIF1 (to SIF/IOP)
NULL,
// channel 7 - SIF2
NULL,
// channel 8 - SPR out
SPRout_DMA_Read,
// channel 9 - SPR in
NULL
};


static const u32 Dma::c_iStallSource_LUT [ 4 ] = { -1, 5, 8, 3 };
static const u32 Dma::c_iStallDest_LUT [ 4 ] = { -1, 1, 2, 6 };
static const u32 Dma::c_iMfifoDrain_LUT [ 4 ] = { -1, -1, 1, 2 };


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
	cout << "Running PS2::DMA::Start...\n";

#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create ( "PS2_DMA_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering DMA::Start";
#endif

	// set the current dma object
	_DMA = this;

	Reset ();
	
	// none of the dma channels are running yet
	for ( int iChannel = 0; iChannel < c_iNumberOfChannels; iChannel++ )
	{
		QWC_Transferred [ iChannel ] = -1;
	}
	
	// ???
	//lDMAC_ENABLE = 0x1201;
	pDMARegs->ENABLEW = 0x1201;
	pDMARegs->ENABLER = 0x1201;
	
	
	// clear events
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		SetNextEventCh_Cycle ( -1ULL, i );
	}
	
#ifdef INLINE_DEBUG
	debug << "->Exiting PS2::DMA::Start";
#endif
}



void Dma::Reset ()
{
	// zero object
	memset ( this, 0, sizeof( Dma ) );
	
	// set static pointers
	Refresh ();
	
	// allow all dma channels to run
	//SelectedDMA_Bitmap = 0xffffffff;
	
	// no dma channels are active
	//ActiveChannel = -1;
}


/*
void Dma::ConnectDevices ( DataBus *BUS, MDEC* mdec, GPU *g, CD *cd, SPU *spu, R5900::Cpu *cpu )
{
	_BUS = BUS;
	_MDEC = mdec;
	_GPU = g;
	_CPU = cpu;
}
*/





void Dma::Run ()
{
	//u32 Temp;
	//u32 Data [ 4 ];
	
	// will use this for MDEC for now
	//u32 NumberOfTransfers;

	
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nDma::Run";
	debug << " NextEvent=" << dec << NextEvent_Cycle;
	debug << " CycleCount=" << *_DebugCycleCount;
#endif

	// check if dma is doing anything starting at this particular cycle
	//if ( NextEvent_Cycle != *_DebugCycleCount ) return;
	
	
	// check for the channel(s) that needs to be run
	for ( int iChannel = 0; iChannel < c_iNumberOfChannels; iChannel++ )
	{
		// need to use the current cycle count to check what dma channel is to run
		//if ( NextEvent_Cycle == NextEventCh_Cycle [ iChannel ] )
		if ( *_DebugCycleCount == NextEventCh_Cycle [ iChannel ] )
		{
			// ***todo*** check channel priority
			Transfer ( iChannel );
		}
	}
	
	// get the cycle number of the next event for device
	Update_NextEventCycle ();
	

}


static u64 Dma::Read ( u32 Address, u64 Mask )
{
#if defined INLINE_DEBUG_READ
	debug << "\r\n\r\nDMA::Read; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << " Mask=" << Mask;
#endif

	//static const u8 c_ucDMAChannel_LUT [ 32 ] = { 0, -1, -1, -1, 1, -1, -1, -1,
	//												2, -1, -1, -1, 3, 4, -1, -1,
	//												5, 6, 7, -1, 8, 9, -1, -1,
	//												-1, -1, -1, -1, -1, -1, -1, -1 };
	
	//u32 DmaChannelNum;
	u32 Shift;
	u64 Output64;
	
	// check if reading 128-bit value
	if ( !Mask )
	{
#if defined INLINE_DEBUG_READ
	debug << " (128-bit READ)";
#endif

		cout << "\nhps2x64: ALERT: DMA: Reading 128-bit value. Address=" << hex << Address << "\n";
	}


#ifdef VERBOSE_UNALIGNED_READ	
	// check if read not aligned
	if ( Address & 0xf )
	{
#if defined INLINE_DEBUG_READ
	debug << " NOT-ALIGNED";
#endif

		cout << "\nhps2x64: ALERT: DMA: Read not aligned. Address=" << hex << Address << "\n";
	}
#endif

	
#ifdef FIX_UNALIGNED_READ
	// get the amount to shift output right (it is possible to read from the high part or middle part of register if it is aligned)
	Shift = ( Address & 0x7 ) << 3;
#else
	Shift = 0;
#endif
	
	// get the upper part of address
	//Address &= ~0xf;
	
	// get lower 16-bits of address
	Address &= 0xffff;
	
	// subtract the offset (registers start at 0x10008000)
	Address -= 0x8000;
	
	// check that register is in range
	if ( Address < 0x8000 )
	{
#if defined INLINE_DEBUG_READ
			debug << " " << Reg_Names [ ( ( Address & 0xf0 ) >> 4 ) | ( ( Address & 0x7c00 ) >> 6 ) ];
#endif

		// read the value
		Output64 = pDMARegs->Regs [ ( ( Address & 0xf0 ) >> 4 ) | ( ( Address & 0x7c00 ) >> 6 ) ] >> Shift;
	
	}
	else
	{
#if defined INLINE_DEBUG_READ
			debug << " " << "INVALID";
#endif

		Output64 = 0;
	}
	
#if defined INLINE_DEBUG_READ
			debug << " = " << hex << Output64;
#endif

	return Output64;
	
	
	/*
	switch ( Address )
	{
		
		case CTRL:
#ifdef INLINE_DEBUG_READ_CTRL
	debug << "\r\n\r\nDMA::Read; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
#endif
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_CTRL
			debug << "; CTRL = " << hex << _DMA->CTRL_Reg.Value;
#endif

			return _DMA->CTRL_Reg.Value >> Shift;
			break;
			
		case STAT:
#ifdef INLINE_DEBUG_READ
			debug << "; STAT = " << hex << _DMA->STAT_Reg.Value;
#endif

			return _DMA->STAT_Reg.Value >> Shift;
			break;


		case PCR:
#ifdef INLINE_DEBUG_READ
			debug << "; PCR = " << hex << _DMA->PCR_Reg.Value;
#endif

			return _DMA->PCR_Reg.Value >> Shift;
			break;


		case DMA_SQWC:
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_SQWC
			debug << "; SQWC= " << hex << _DMA->SQWC_Reg.Value;
#endif

			return _DMA->SQWC_Reg.Value >> Shift;
			break;
			
		case DMA_RBOR:
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_RBOR
			debug << "; RBOR= " << hex << _DMA->RBOR_Reg;
#endif

			return _DMA->RBOR_Reg >> Shift;
			break;

		case DMA_RBSR:
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_RBSR
			debug << "; RBSR= " << hex << _DMA->RBSR_Reg.Value;
#endif

			return _DMA->RBSR_Reg.Value >> Shift;
			break;

		case DMA_STADR:
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_STADR
			debug << "; STADR= " << hex << _DMA->STADR_Reg;
#endif

			return _DMA->STADR_Reg >> Shift;
			break;

		case DMA_ENABLER:
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_ENABLER
			debug << "; ENABLER= " << hex << _DMA->lDMAC_ENABLE;
#endif

			// dmac enable r
			return _DMA->lDMAC_ENABLE >> Shift;
			break;
			
		case DMA_ENABLEW:
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_ENABLEW
			debug << "; ENABLEW= " << hex << _DMA->lDMAC_ENABLE;
#endif

			// ???
			return _DMA->lDMAC_ENABLE >> Shift;
			break;
			

			
		default:

			// get the dma channel that is being accessed
			DmaChannelNum = c_ucDMAChannel_LUT [ ( Address >> 10 ) & 0x1f ];
			
			if ( Address >= 0x10008000 && Address < 0x1000e000 && DmaChannelNum < c_iNumberOfChannels )
			{

				// get the dma channel number
				//DmaChannelNum = ( ( Address >> 4 ) & 0xf ) - 8;
				

				switch ( ( Address >> 4 ) & 0xf )
				{
					case 0:
#ifdef INLINE_DEBUG_READ_CHCR
	debug << "\r\n\r\nDMA::Read; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
#endif
#ifdef INLINE_DEBUG_READ || INLINE_DEBUG_READ_CHCR
					debug << "; DMA" << DmaChannelNum << "_CHCR = " << hex << _DMA->DmaCh [ DmaChannelNum ].CHCR_Reg.Value;
#endif

						return _DMA->DmaCh [ DmaChannelNum ].CHCR_Reg.Value >> Shift;
						break;
					
					case 1:
#ifdef INLINE_DEBUG_READ
					debug << "; DMA" << DmaChannelNum << "_MADR = " << hex << _DMA->DmaCh [ DmaChannelNum ].MADR_Reg.Value;
#endif

						return _DMA->DmaCh [ DmaChannelNum ].MADR_Reg.Value >> Shift;
						break;
					
					case 2:
#ifdef INLINE_DEBUG_READ
					debug << "; DMA" << DmaChannelNum << "_QWC = " << hex << _DMA->DmaCh [ DmaChannelNum ].QWC_Reg.Value;
#endif

						return _DMA->DmaCh [ DmaChannelNum ].QWC_Reg.Value >> Shift;
						break;
				
					
					case 3:
#ifdef INLINE_DEBUG_READ
					debug << "; DMA" << DmaChannelNum << "_TADR = " << hex << _DMA->DmaCh [ DmaChannelNum ].TADR_Reg.Value;
#endif

						return _DMA->DmaCh [ DmaChannelNum ].TADR_Reg.Value >> Shift;
						break;
					
					case 4:
#ifdef INLINE_DEBUG_READ
					debug << "; DMA" << DmaChannelNum << "_ASR0 = " << hex << _DMA->DmaCh [ DmaChannelNum ].ASR0_Reg.Value;
#endif

						return _DMA->DmaCh [ DmaChannelNum ].ASR0_Reg.Value >> Shift;
						break;
					
					case 5:
#ifdef INLINE_DEBUG_READ
					debug << "; DMA" << DmaChannelNum << "_ASR1 = " << hex << _DMA->DmaCh [ DmaChannelNum ].ASR1_Reg.Value;
#endif

						return _DMA->DmaCh [ DmaChannelNum ].ASR1_Reg.Value >> Shift;
						break;
					
					
					case 8:
#ifdef INLINE_DEBUG_READ
					debug << "; DMA" << DmaChannelNum << "_SADR = " << hex << _DMA->DmaCh [ DmaChannelNum ].SADR_Reg.Value;
#endif

						return _DMA->DmaCh [ DmaChannelNum ].SADR_Reg.Value >> Shift;
						break;
					
						
					default:
#ifdef INLINE_DEBUG_READ_INVALID
	debug << "\r\n\r\nDMA::Read; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
#endif
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_INVALID
						debug << "; Invalid";
#endif
		
						// invalid DMA Register
						cout << "\nhps2x64 ALERT: Unknown DMA READ @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << "\n";
						break;
				}
			}
			else
			{
#ifdef INLINE_DEBUG_READ_INVALID
	debug << "\r\n\r\nDMA::Read; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
#endif
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_INVALID
				debug << "; Invalid";
#endif

				cout << "\nhps2x64 WARNING: READ from unknown DMA Register @ Cycle#" << dec << *_DebugCycleCount << " PC=" << hex << *_DebugPC << " Address=" << Address << "\n";
			}

			break;
			
	}
			
	return 0;
	*/

}



static void Dma::Write ( u32 Address, u64 Data, u64 Mask )
{
#if defined INLINE_DEBUG_WRITE
	debug << "\r\n\r\nDMA::Write; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << " Mask=" << Mask << " Data = " << Data;
#endif

	static const u8 c_ucDMAChannel_LUT [ 32 ] = { 0, -1, -1, -1, 1, -1, -1, -1,
													2, -1, -1, -1, 3, 4, -1, -1,
													5, 6, 7, -1, 8, 9, -1, -1,
													-1, -1, -1, -1, -1, -1, -1, -1 };

	u32 DmaChannelNum;
	u32 Temp;
	u32 Shift;
	u32 Offset;
	u32 PreviousValue;
	u64 Output64;
	
	// didn't account for a 128-bit write to the dma
	if ( !Mask )
	{
#if defined INLINE_DEBUG_WRITE
	debug << " (128-bit WRITE)";
#endif

		cout << "\nhps2x64: ALERT: DMA: 128-bit write. Address=" << hex << Address;
	}
	
	
#ifdef VERBOSE_UNALIGNED_WRITE
	// check if write not aligned
	if ( Address & 0xf )
	{
#if defined INLINE_DEBUG_WRITE
	debug << " NOT-ALIGNED";
#endif

		cout << "\nhps2x64: ALERT: DMA: WRITE not aligned. Address=" << hex << Address << "\n";
	}
#endif


#ifdef FIX_UNALIGNED_WRITE
	// get the amount to shift input left (it is possible to read from the high part or middle part of register if it is aligned)
	Shift = ( Address & 0x7 ) << 3;
	Mask <<= Shift;
	Data <<= Shift;
#else
	Shift = 0;
#endif
	
	// apply write mask here for now
	Data &= Mask;
	
	// get the upper part of address
	Address &= ~0xf;
	
	// get lower 16-bits of address
	Address &= 0xffff;
	
	// subtract the offset (registers start at 0x10008000)
	Address -= 0x8000;
	
	// check that register is in range
	if ( Address < 0x8000 )
	{
#if defined INLINE_DEBUG_WRITE
			debug << " " << Reg_Names [ ( ( Address & 0xf0 ) >> 4 ) | ( ( Address & 0x7c00 ) >> 6 ) ];
#endif

		// check if ENABLER register
		if ( Address == ( ( DMA_ENABLER & 0xffff ) - 0x8000 ) )
		{
			// this should be read-only
			return;
		}
		
		// check if STAT register
		if ( Address == ( ( DMA_STAT & 0xffff ) - 0x8000 ) )
		{
			// the bottom 16-bits get cleared when a one is written to the bit
			pDMARegs->STAT.Value &= ~( Data & 0xffff );
			
			// the upper 16-bits get inverted when a one is written to the bit
			// but keep bits 26-28 zero, and bit 31 zero
			pDMARegs->STAT.Value ^= ( Data & 0x63ff0000 );
			
			// *** TODO *** INT1 == CIS&CIM || BEIS
			// update interrupts when STAT gets modified
			_DMA->UpdateInterrupt ();
			
			// update CPCOND0
			_DMA->Update_CPCOND0 ();
			
			return;
		}
		

		// get offset to hardware register
		Offset = ( ( Address & 0xf0 ) >> 4 ) | ( ( Address & 0x7c00 ) >> 6 );
		
		// write the value
		u32 PreviousValue = pDMARegs->Regs [ Offset ];
		pDMARegs->Regs [ Offset ] = ( PreviousValue & ~Mask ) | ( Data );
		

		// check if PCR register
		if ( Address == ( ( DMA_PCR & 0xffff ) - 0x8000 ) )
		{
			// update CPCOND0
			_DMA->Update_CPCOND0 ();
			
			return;
		}
		
		// check if ENABLEW register
		if ( Address == ( ( DMA_ENABLEW & 0xffff ) - 0x8000 ) )
		{
			// also write value to DMA ENABLER in case for when it gets read
			pDMARegs->ENABLER = pDMARegs->ENABLEW;
			
			// check if there is a transition from one to zero
			//if ( ( Data ^ 0x10000 ) & _DMA->lDMAC_ENABLE & 0x10000 )
			if ( ( Data ^ 0x10000 ) & PreviousValue & 0x10000 )
			{
				
				// transition from zero to one, so store and update transfers
				//_DMA->lDMAC_ENABLE = ( _DMA->lDMAC_ENABLE & ~Mask ) | ( Data );
				_DMA->UpdateTransfer ();
			}
			//else
			//{
			//	// ???
			//	//_DMA->lDMAC_ENABLE = Data;
			//	_DMA->lDMAC_ENABLE = ( _DMA->lDMAC_ENABLE & ~Mask ) | ( Data );
			//}
			
			return;
		}
		
		// check for CHCR
		if ( !( Address & 0xf0 ) )
		{
			if ( Address < ( ( DMA_CTRL & 0xffff ) - 0x8000 ) )
			{
				// lookup what dma channel corresponds to the address
				DmaChannelNum = c_ucDMAChannel_LUT [ ( Address >> 10 ) & 0x1f ];
				
				if ( pDMARegs->CTRL.DMAE && ( !pDMARegs->PCR.PCE || ( pDMARegs->PCR.Value & ( 1 << ( DmaChannelNum + 16 ) ) ) ) && pRegData [ DmaChannelNum ]->CHCR.STR )
				{
					// transfer is set to start //
					
					// start transfer
					_DMA->StartTransfer ( DmaChannelNum );
					_DMA->Transfer ( DmaChannelNum );
				}
				
				return;
			}
		}
	}
	else
	{
#if defined INLINE_DEBUG_WRITE
			debug << " " << "INVALID";
#endif

	}


	return;
	
	
	
	/*
	
	switch ( Address )
	{
		case CTRL:
#ifdef INLINE_DEBUG_WRITE_CTRL
	debug << "\r\n\r\nDMA::Write; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CTRL
			debug << "; (Before) CTRL= " << hex << _DMA->CTRL_Reg.Value;
#endif

			_DMA->CTRL_Reg.Value = ( _DMA->CTRL_Reg.Value & ~Mask ) | ( Data );
			
			// alerts for testing
			if ( _DMA->CTRL_Reg.CycleStealMode )
			{
#ifdef VERBOSE_CYCLESTEAL
				cout << "\nhps2x64: ALERT: DMA: CycleStealMode=" << _DMA->CTRL_Reg.CycleStealMode;
#endif
			}
			if ( _DMA->CTRL_Reg.MFIFO )
			{
#ifdef VERBOSE_MFIFO
				cout << "\nhps2x64: ALERT: DMA: MFIFO=" << _DMA->CTRL_Reg.MFIFO;
#endif
			}
			if ( _DMA->CTRL_Reg.STS )
			{
#ifdef VERBOSE_STS
				cout << "\nhps2x64: ALERT: DMA: STS=" << _DMA->CTRL_Reg.STS;
#endif
			}
			if ( _DMA->CTRL_Reg.STD )
			{
#ifdef VERBOSE_STD
				cout << "\nhps2x64: ALERT: DMA: STD=" << _DMA->CTRL_Reg.STD;
#endif
			}
			if ( _DMA->CTRL_Reg.RCYC )
			{
#ifdef VERBOSE_RCYC
				cout << "\nhps2x64: ALERT: DMA: RCYC=" << _DMA->CTRL_Reg.RCYC;
#endif
			}
			
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CTRL
			debug << "; (After) CTRL= " << hex << _DMA->CTRL_Reg.Value;
#endif
			break;

			
		case STAT:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_STAT
			debug << "; (Before) STAT= " << hex << _DMA->STAT_Reg.Value;
#endif

			//Data = ( Data << Shift ) & Mask;
			
			// the bottom 16-bits get cleared when a one is written to the bit
			_DMA->STAT_Reg.Value &= ~( Data & 0xffff );
			
			// the upper 16-bits get inverted when a one is written to the bit
			// but keep bits 26-28 zero, and bit 31 zero
			_DMA->STAT_Reg.Value ^= ( Data & 0x63ff0000 );
			
			// *** TODO *** INT1 == CIS&CIM || BEIS
			// update interrupts when STAT gets modified
			_DMA->UpdateInterrupt ();
			
			// update CPCOND0
			_DMA->Update_CPCOND0 ();

#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_STAT
			debug << "; (After) STAT= " << hex << _DMA->STAT_Reg.Value;
#endif

			break;


		case PCR:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_PCR
			debug << "; (Before) PCR= " << hex << _DMA->PCR_Reg.Value;
#endif

			//_DMA->PCR_Reg.Value = Data;
			_DMA->PCR_Reg.Value = ( _DMA->PCR_Reg.Value & ~Mask ) | ( Data );
			
			// update CPCOND0
			_DMA->Update_CPCOND0 ();

#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_PCR
			debug << "; (After) PCR= " << hex << _DMA->PCR_Reg.Value;
#endif

			break;


		case DMA_SQWC:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_SQWC
			debug << "; (Before) SQWC= " << hex << _DMA->SQWC_Reg.Value;
#endif

			//_DMA->SQWC_Reg.Value = Data;
			_DMA->SQWC_Reg.Value = ( _DMA->SQWC_Reg.Value & ~Mask ) | ( Data );
			
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_SQWC
			debug << "; (After) SQWC= " << hex << _DMA->SQWC_Reg.Value;
#endif
			break;
			
		case DMA_RBOR:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_RBOR
			debug << "; (Before) RBOR= " << hex << _DMA->RBOR_Reg;
#endif

			//_DMA->RBOR_Reg = Data;
			_DMA->RBOR_Reg = ( _DMA->RBOR_Reg & ~Mask ) | ( Data );
			
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_RBOR
			debug << "; (After) RBOR= " << hex << _DMA->RBOR_Reg;
#endif
			break;

		case DMA_RBSR:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_RBSR
			debug << "; (Before) RBSR= " << hex << _DMA->RBSR_Reg.Value;
#endif

			//_DMA->RBSR_Reg.Value = Data;
			_DMA->RBSR_Reg.Value = ( _DMA->RBSR_Reg.Value & ~Mask ) | ( Data );
			
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_RBSR
			debug << "; (After) RBSR= " << hex << _DMA->RBSR_Reg.Value;
#endif
			break;

		case DMA_STADR:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_STADR
			debug << "; (Before) STADR= " << hex << _DMA->STADR_Reg;
#endif

			//_DMA->STADR_Reg = Data;
			_DMA->STADR_Reg = ( _DMA->STADR_Reg & ~Mask ) | ( Data );
			
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_STADR
			debug << "; (After) STADR= " << hex << _DMA->STADR_Reg;
#endif
			break;
			
			
		case DMA_ENABLER:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_ENABLER
			debug << "; (Before) ENABLER= " << hex << _DMA->lDMAC_ENABLE;
#endif

			// dmac enable r
			// READ ONLY
			
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_ENABLER
			debug << "; (After) ENABLER= " << hex << _DMA->lDMAC_ENABLE;
#endif
			break;
			
		case DMA_ENABLEW:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_ENABLEW
			debug << "; (Before) ENABLEW= " << hex << _DMA->lDMAC_ENABLE;
#endif

			// check if there is a transition from one to zero
			if ( ( Data ^ 0x10000 ) & _DMA->lDMAC_ENABLE & 0x10000 )
			{
				// transition from zero to one, so store and update transfers
				//_DMA->lDMAC_ENABLE = Data;
				_DMA->lDMAC_ENABLE = ( _DMA->lDMAC_ENABLE & ~Mask ) | ( Data );
				_DMA->UpdateTransfer ();
			}
			else
			{
				// ???
				//_DMA->lDMAC_ENABLE = Data;
				_DMA->lDMAC_ENABLE = ( _DMA->lDMAC_ENABLE & ~Mask ) | ( Data );
			}
			
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_ENABLEW
			debug << "; (After) ENABLEW= " << hex << _DMA->lDMAC_ENABLE;
#endif
			break;
			
			
		default:
		
			// get the dma channel that is being accessed
			DmaChannelNum = c_ucDMAChannel_LUT [ ( Address >> 10 ) & 0x1f ];
			
			if ( Address >= 0x10008000 && Address < 0x1000e000 && DmaChannelNum < c_iNumberOfChannels )
			{
				// get the dma channel number
				//DmaChannelNum = ( ( Address >> 4 ) & 0xf ) - 8;
				

				switch ( ( Address >> 4 ) & 0xf )
				{
					case 0:
#ifdef INLINE_DEBUG_WRITE_CHCR
	debug << "\r\n\r\nDMA::Write; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
#endif
#ifdef INLINE_DEBUG_WRITE
					debug << "; (Before) DMA" << DmaChannelNum << "_CHCR = " << hex << _DMA->DmaCh [ DmaChannelNum ].CHCR_Reg.Value;
#endif

						// must set the full value of CHCR
						//_DMA->DmaCh [ DmaChannelNum ].CHCR_Reg.Value = Data;
						_DMA->DmaCh [ DmaChannelNum ].CHCR_Reg.Value = ( _DMA->DmaCh [ DmaChannelNum ].CHCR_Reg.Value & ~Mask ) | ( Data );
						
						// check if set to start transfer (check for transition from one to zero)
						// before a dma transfer can start, need CTRL_Reg.DMAE AND (!PCR_Reg.PCE OR PCR.CDE) AND CHCR_Reg.STR
						if ( _DMA->CTRL_Reg.DMAE && ( !_DMA->PCR_Reg.PCE || ( _DMA->PCR_Reg.Value & ( 1 << ( DmaChannelNum + 16 ) ) ) ) && _DMA->DmaCh [ DmaChannelNum ].CHCR_Reg.STR )
						{
							// transfer is set to start //
							
							// start transfer
							_DMA->StartTransfer ( DmaChannelNum );
							_DMA->Transfer ( DmaChannelNum );
						}


#ifdef INLINE_DEBUG_WRITE
					debug << "; (After) DMA" << DmaChannelNum << "_CHCR = " << hex << _DMA->DmaCh [ DmaChannelNum ].CHCR_Reg.Value;
#endif

						break;
						
					
					case 1:
#ifdef INLINE_DEBUG_WRITE
					debug << "; (Before) DMA" << DmaChannelNum << "_MADR = " << hex << _DMA->DmaCh [ DmaChannelNum ].MADR_Reg.Value;
#endif



						//_DMA->DmaCh [ DmaChannelNum ].MADR_Reg.Value = Data;
						_DMA->DmaCh [ DmaChannelNum ].MADR_Reg.Value = ( _DMA->DmaCh [ DmaChannelNum ].MADR_Reg.Value & ~Mask ) | ( Data );
						

#ifdef INLINE_DEBUG_WRITE
					debug << "; (After) DMA" << DmaChannelNum << "_MADR = " << hex << _DMA->DmaCh [ DmaChannelNum ].MADR_Reg.Value;
#endif

						break;
				
					case 2:
#ifdef INLINE_DEBUG_WRITE
					debug << "; (Before) DMA" << DmaChannelNum << "_QWC = " << hex << _DMA->DmaCh [ DmaChannelNum ].QWC_Reg.Value;
#endif


						//_DMA->DmaCh [ DmaChannelNum ].QWC_Reg.Value = Data;
						_DMA->DmaCh [ DmaChannelNum ].QWC_Reg.Value = ( _DMA->DmaCh [ DmaChannelNum ].QWC_Reg.Value & ~Mask ) | ( Data );
						

#ifdef INLINE_DEBUG_WRITE
					debug << "; (After) DMA" << DmaChannelNum << "_QWC = " << hex << _DMA->DmaCh [ DmaChannelNum ].QWC_Reg.Value;
#endif

						break;

						
					case 3:
#ifdef INLINE_DEBUG_WRITE
					debug << "; (Before) DMA" << DmaChannelNum << "_TADR = " << hex << _DMA->DmaCh [ DmaChannelNum ].TADR_Reg.Value;
#endif



						//_DMA->DmaCh [ DmaChannelNum ].TADR_Reg.Value = Data;
						_DMA->DmaCh [ DmaChannelNum ].TADR_Reg.Value = ( _DMA->DmaCh [ DmaChannelNum ].TADR_Reg.Value & ~Mask ) | ( Data );
						

#ifdef INLINE_DEBUG_WRITE
					debug << "; (After) DMA" << DmaChannelNum << "_TADR = " << hex << _DMA->DmaCh [ DmaChannelNum ].TADR_Reg.Value;
#endif

						break;
						
						
					case 4:
#ifdef INLINE_DEBUG_WRITE
					debug << "; (Before) DMA" << DmaChannelNum << "_ASR0 = " << hex << _DMA->DmaCh [ DmaChannelNum ].ASR0_Reg.Value;
#endif



						//_DMA->DmaCh [ DmaChannelNum ].ASR0_Reg.Value = Data;
						_DMA->DmaCh [ DmaChannelNum ].ASR0_Reg.Value = ( _DMA->DmaCh [ DmaChannelNum ].ASR0_Reg.Value & ~Mask ) | ( Data );
						

#ifdef INLINE_DEBUG_WRITE
					debug << "; (After) DMA" << DmaChannelNum << "_ASR0 = " << hex << _DMA->DmaCh [ DmaChannelNum ].ASR0_Reg.Value;
#endif

						break;
						
						
					case 5:
#ifdef INLINE_DEBUG_WRITE
					debug << "; (Before) DMA" << DmaChannelNum << "_ASR1 = " << hex << _DMA->DmaCh [ DmaChannelNum ].ASR1_Reg.Value;
#endif



						//_DMA->DmaCh [ DmaChannelNum ].ASR1_Reg.Value = Data;
						_DMA->DmaCh [ DmaChannelNum ].ASR1_Reg.Value = ( _DMA->DmaCh [ DmaChannelNum ].ASR1_Reg.Value & ~Mask ) | ( Data );
						

#ifdef INLINE_DEBUG_WRITE
					debug << "; (After) DMA" << DmaChannelNum << "_ASR1 = " << hex << _DMA->DmaCh [ DmaChannelNum ].ASR1_Reg.Value;
#endif

						break;
						
						
					case 8:
#ifdef INLINE_DEBUG_WRITE
					debug << "; (Before) DMA" << DmaChannelNum << "_SADR = " << hex << _DMA->DmaCh [ DmaChannelNum ].SADR_Reg.Value;
#endif



						//_DMA->DmaCh [ DmaChannelNum ].SADR_Reg.Value = Data;
						_DMA->DmaCh [ DmaChannelNum ].SADR_Reg.Value = ( _DMA->DmaCh [ DmaChannelNum ].SADR_Reg.Value & ~Mask ) | ( Data );
						

#ifdef INLINE_DEBUG_WRITE
					debug << "; (After) DMA" << DmaChannelNum << "_SADR = " << hex << _DMA->DmaCh [ DmaChannelNum ].SADR_Reg.Value;
#endif

						break;
						
						
					default:
#ifdef INLINE_DEBUG_WRITE_INVALID
	debug << "\r\n\r\nDMA::Write; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_INVALID
						debug << "; Invalid";
#endif
		
						// invalid DMA Register
						cout << "\nhps2x64 ALERT: Unknown DMA WRITE @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << " Data=" << Data << "\n";
						break;
					
				}
				
			}
			else
			{
#ifdef INLINE_DEBUG_WRITE_INVALID
	debug << "\r\n\r\nDMA::Write; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_INVALID
				debug << "; Invalid";
#endif

				cout << "\nhps2x64 WARNING: WRITE to unknown DMA Register @ Cycle#" << dec << *_DebugCycleCount << " PC=" << hex << *_DebugPC << " Address=" << Address << "\n";
			}

			break;
	}
	*/


}




void Dma::StartTransfer ( int iChannel )
{
#ifdef INLINE_DEBUG_START
	debug << "\r\nDma::StartTransfer; Channel#" << dec << iChannel;
#endif

	// check if channel is already running and was simply suspended
	/*
	if ( ( ( DmaCh [ iChannel ].CHCR_Reg.TAG >> 12 ) & 7 ) != 0 )
	{
#ifdef INLINE_DEBUG_START
	debug << "\r\nRestarting suspended Dma Channel#" << dec << iChannel;
#endif

		// set the source dma tag info
		SourceDMATag [ iChannel ].Value = ( ( (u64) DmaCh [ iChannel ].MADR_Reg.Value ) << 32 );
		SourceDMATag [ iChannel ].Value |= ( DmaCh [ iChannel ].CHCR_Reg.Value & 0xffff0000 ) | ( DmaCh [ iChannel ].QWC_Reg.Value & 0x0000ffff );

		// set the current transfer info
		QWC_Transferred [ iChannel ] = 0;
		QWC_BlockTotal [ iChannel ] = DmaCh [ iChannel ].QWC_Reg.Value;
		
		// TADR updates before ??
		DmaCh [ iChannel ].TADR_Reg.Value -= 16;

		// continue the transfer
		Transfer ( iChannel );
		
		// done here
		return;
	}
	*/

	// clear previous tag in CHCR
	// must write tag back anyway when writing to CHCR
	//DmaCh [ iChannel ].CHCR_Reg.Value &= 0xffff;
	
	// reset the QWC that has been transferred in current block
	// -1 means that block still needs to be started
	QWC_Transferred [ iChannel ] = -1;
	
	// set the cycle# that transfer was started at
	//DmaCh [ iChannel ].StartCycle = *_DebugCycleCount;
	
	// clear CISx ??
	//STAT_Reg.Value &= ~( 1 << iChannel );
	

	switch ( iChannel )
	{
		// SIF0
		case 5:
			
			// starting dma#5 sets bit 13 (0x2000) in SBUS CTRL register
			//DataBus::_BUS->lSBUS_F240 |= 0x2000;
			*_SBUS_F240 |= 0x2000;
			
			break;
			
		// SIF1
		case 6:
		
			// starting dma#6 sets bit 14 (0x4000) in SBUS CTRL register
			//DataBus::_BUS->lSBUS_F240 |= 0x4000;
			*_SBUS_F240 |= 0x4000;
		
			break;
	}
}



void Dma::SuspendTransfer ( int iChannel )
{
#ifdef INLINE_DEBUG_END
	debug << "\r\nDma::SuspendTransfer; Channel#" << dec << iChannel;
	debug << "; (before) CHCR=" << hex << pRegData [ iChannel ]->CHCR.Value << " STAT=" << DMARegs.STAT.Value;
#endif

	// clear STR for channel
	pRegData [ iChannel ]->CHCR.STR = 0;
}



void Dma::EndTransfer ( int iChannel, bool SuppressEventUpdate )
{
#ifdef INLINE_DEBUG_END
	debug << "\r\nDma::EndTransfer; Channel#" << dec << iChannel;
	debug << "; (before) CHCR=" << hex << pRegData [ iChannel ]->CHCR.Value << " STAT=" << DMARegs.STAT.Value;
#endif

	// channel is done with transfer //
	
	// clear STR for channel
	pRegData [ iChannel ]->CHCR.STR = 0;
	
	// set CIS for channel - (STAT register)
	DMARegs.STAT.Value |= ( 1 << iChannel );
	
	// check for interrupt
	UpdateInterrupt ();
	
	// update CPCOND0
	Update_CPCOND0 ();

#ifdef CLEAR_QWC_ON_COMPLETE
	pRegData [ iChannel ]->QWC.Value = 0;
#endif

	switch ( iChannel )
	{
		case 1:
			// for dma#1, if transfer ends then un-mask path3 transfers
			GPU::_GPU->GIFRegs.STAT.M3R = 0;
			GPU::_GPU->GIFRegs.STAT.M3P = 0;
			
			// also restart dma#2
			//Transfer ( 2 );
			
			break;
			
		case 2:
			// if dma#2 completes, then restart dma#1 if it is active
			// this is because GPU path 3 (dma#2) can stop dma#1 for now if it is currently transferring a packet
			//Transfer ( 1 );
			break;
			
		// SIF0
		case 5:

#ifdef ENABLE_DMA_END_REMOTE
			// ***TESTING*** suspend channel#9 on IOP
			SIF::_SIF->SuspendTransfer_IOP ( 9 );
#endif
			
			// ending dma#5 clears bits 5 (0x20) and 13 (0x2000) in SBUS CTRL register
#ifdef ENABLE_SIF_DMA_SYNC
			*_SBUS_F240 &= ~0x2000;
#else
			//DataBus::_BUS->lSBUS_F240 &= ~0x2020;
			
			// check if IOP dma #9 is done also ?
			if ( !SIF::_SIF->IOP_DMA_Out_Ready () )
			{
#ifdef INLINE_DEBUG_END
	debug << " CLEAR_F240_DMA5_END";
#endif

				*_SBUS_F240 &= ~0x2020;
			}
			
			//*_SBUS_F240 &= ~0x2000;
#endif
			
			break;
			
		// SIF1
		case 6:
		
#ifdef ENABLE_DMA_END_REMOTE
			// ***TESTING*** suspend channel#10 on IOP
			//SIF::_SIF->SuspendTransfer_IOP ( 10 );
#endif
		
			// ending dma#6 clears bits 6 (0x40) and 14 (0x4000) in SBUS CTRL register
#ifdef ENABLE_SIF_DMA_SYNC
			*_SBUS_F240 &= ~0x4000;
#else
			//DataBus::_BUS->lSBUS_F240 &= ~0x4040;
			
			// check if IOP dma #10 is done also?
			if ( !SIF::_SIF->IOP_DMA_In_Ready () )
			{
#ifdef INLINE_DEBUG_END
	debug << " CLEAR_F240_DMA6_END";
#endif

				*_SBUS_F240 &= ~0x4040;
			}
			
			//*_SBUS_F240 &= ~0x4000;
#endif
		
			break;
	}
	
	if ( !SuppressEventUpdate )
	{
		Update_NextEventCycle ();
	}
	
	// as long as this is not dma#5 that is ending (could happen at anytime), restart any pending dma channels
	//UpdateTransfer ();
	// get the next transfer started
	for ( int tChannel = 0; tChannel < c_iNumberOfChannels; tChannel++ )
	{
		// need to use the current cycle count to check what dma channel is to run
		//if ( NextEvent_Cycle == NextEventCh_Cycle [ tChannel ] )
		if ( pRegData [ tChannel ]->CHCR.STR && NextEventCh_Cycle [ tChannel ] <= *_DebugCycleCount )
		{
#ifdef INLINE_DEBUG_END
	debug << " RESUMING-DMA#" << dec << tChannel;
#endif

			// ***todo*** check channel priority
			Transfer ( tChannel );
			
			// just get the next transfer started, recursion would probably start anything else anyway
			break;
		}
	}
	
#ifdef ENABLE_SIF_DMA_TIMING
	// if channel#5, then check if channel#6 is ready to go since it would have been held up
	if( iChannel == 5 )
	{
		SIF::_SIF->Check_TransferToIOP ();
	}
#endif
	
#ifdef INLINE_DEBUG_END
	debug << "; (after) CHCR=" << hex << pRegData [ iChannel ]->CHCR.Value << " STAT=" << DMARegs.STAT.Value;
#endif
}


// need to call this whenever updating interrupt related registers
void Dma::UpdateInterrupt ()
{
#ifdef INLINE_DEBUG_INT
	debug << "; Dma::UpdateInterrupt";
#endif

	// check for interrupt
	if ( DMARegs.STAT.Value & ( DMARegs.STAT.Value >> 16 ) & 0x63ff )
	{
#ifdef INLINE_DEBUG_INT
	debug << "; INT";
#endif

		// interrupt (SET INT1 on R5900)
		SetInterrupt ();
	}
	else
	{
#ifdef INLINE_DEBUG_INT
	debug << "; CLEAR_INT";
#endif

		// this should clear the interrupt on INT1 on R5900
		ClearInterrupt ();
	}
	
}


// this needs to be called whenever PCR gets changed or STAT gets changed for any reason
void Dma::Update_CPCOND0 ()
{
	// update CPCOND0
	if ( ( ( DMARegs.STAT.Value | ~DMARegs.PCR.Value ) & 0x3ff ) == 0x3ff )
	{
		// CPCOND0 = 1
		*_CPCOND0_Out = 1;
	}
	else
	{
		// CPCOND0 = 0
		*_CPCOND0_Out = 0;
	}
}


// should return 1 if transfer is complete on PS2 side, zero otherwise
void Dma::DMA5_WriteBlock ( u64* Data64, u32 QWC_Count )
{
#ifdef INLINE_DEBUG_RUN_DMA5
	debug << "\r\nDMA5_WriteBlock: DMA5: SIF0 IOP->EE";
	debug << " QWC=" << dec << QWC_Count;
	debug << " Cycle#" << dec << *_DebugCycleCount;
#endif

	// dma transfer has been started //
	
	u32 Temp;
	//u32 Data [ 4 ];
	//u32 NumberOfTransfers;
	
	u32 TransferCount;
	u32 QWC_Remaining;
	
	u32 Data0, Data1, DestAddress, IRQ, ID;
	
	//DMATag EETag;
	//EETag.Value = EEDMATag;
	
	u64 *DstPtr64;
	//u64 *Data64;
	
	bool TransferInProgress = true;
	
	// this delay should be handled at the source of the transfer
	static u64 CycleDelay = 0;
	
	// set pointer into data
	//Data64 = (u64*) & ( pMemory [ Address >> 2 ] );
	
	/*
	if ( CycleDelay > *_DebugCycleCount )
	{
		CycleDelay += c_llSIFDelayPerQWC * (u64) Count;
	}
	else
	{
		CycleDelay = *_DebugCycleCount + (u64) ( c_llSIFDelayPerQWC * Count );
	}
	*/

	//Data0 = *Data++;
	//Data1 = *Data++;
	

	// rest of quadword is ignored
	//Data++;
	//Data++;
	

//#ifdef INLINE_DEBUG_RUN_DMA5
//#endif

	// just out of curiosity, check if tag transfer is enabled for IOP->EE DMA#5 transfer
	if ( pRegData [ 5 ]->CHCR.TTE )
	{
#ifdef INLINE_DEBUG_RUN_DMA5
	debug << "\r\nhps2x64: DMA: ALERT: TagTransfer enabled for DMA#5 (IOP->EE) transfer!";
#endif

#ifdef VERBOSE_CHAIN
		cout << "\nhps2x64: DMA: ALERT: TagTransfer enabled for DMA#5 (IOP->EE) transfer!";
#endif
	}

	// transfer all the data that was sent
	while ( TransferInProgress )
	{
		// if in destination chain mode, then pull tag and set address first
		if ( pRegData [ 5 ]->CHCR.MOD == 1 )
		{

			//if ( !DmaCh [ 5 ].QWCRemaining )
			if ( !pRegData [ 5 ]->QWC.QWC )
			{
#ifdef INLINE_DEBUG_RUN_DMA5
	debug << "; QWCRemaining=0 -> Getting TAG";
	//debug << "; EETag=" << hex << EEDMATag;
#endif

				// get the IOP tag
				IOPDMATag [ 5 ].Value = Data64 [ 0 ];

				// set the tag
				//SourceDMATag [ 5 ].Value = EEDMATag;
				SourceDMATag [ 5 ].Value = Data64 [ 1 ];
				
				// subtract from count of data sent
				// the QWC in TAG does not include the tag
				//QWC_Count--;
				
				// also set upper bits to tag upper bits in chcr
				//DmaCh [ 5 ].CHCR_Reg.Value = ( DmaCh [ 5 ].CHCR_Reg.Value & 0xffffL ) | ( SourceDMATag [ 5 ].Value & 0xffff0000L );
				pRegData [ 5 ]->CHCR.Value = ( pRegData [ 5 ]->CHCR.Value & 0xffffL ) | ( Data64 [ 1 ] & 0xffff0000L );
				
				// set MADR
				//DmaCh [ 5 ].MADR_Reg.Value = ( SourceDMATag [ 5 ].Value >> 32 );
				pRegData [ 5 ]->MADR.Value = ( Data64 [ 1 ] >> 32 );
				
				// set the QWC to transfer
				//DmaCh [ 5 ].QWCRemaining = SourceDMATag [ 5 ].QWC;
				DmaCh [ 5 ].QWCRemaining = Data64 [ 1 ] & 0xffff;
			
				// need to set QWC
				pRegData [ 5 ]->QWC.QWC = Data64 [ 1 ] & 0xffff;
				
				
				// just read the tag, so QWC-1
				QWC_Count -= 1;
				
				
				// have not transferred anything for current tag yet
				DmaCh [ 5 ].QWCTransferred = 0;
			
#ifdef INLINE_DEBUG_RUN_DMA5
	debug << " EETag.QWC=" << dec << SourceDMATag [ 5 ].QWC;
	debug << " Tag.ID=" << SourceDMATag [ 5 ].ID << " Tag.IRQ=" << SourceDMATag [ 5 ].IRQ << " Tag.PCE=" << SourceDMATag [ 5 ].PCE;
	debug << "; Tag.MADR=" << hex << pRegData [ 5 ]->MADR.Value;
#endif
			}

			// check if there is data remaining in transfer from IOP besides the EE tag
			if ( !QWC_Count )
			{
				// will need to wait for more data
				return;
				//return 0;
			}

			//DstPtr64 = GetMemoryPtr ( DmaCh [ 5 ].MADR_Reg.Value + ( DmaCh [ 5 ].QWCTransferred << 4 ) );
			DstPtr64 = GetMemoryPtr ( pRegData [ 5 ]->MADR.Value );



			// check the ID
			switch ( SourceDMATag [ 5 ].ID )
			{
				// ID: CNTS
				case 0:
#ifdef INLINE_DEBUG_RUN_DMA5
	debug << "; ID=CNTS";
	debug << "; NOT IMPLEMENTED";
#endif

					// need some type of message here..
					cout << "\nhps2x64: ***ALERT***: DMA Destination Tag CNTS not implemented\n";
					
					// STALL CONTROL //
					// since chain transfer, first check that tag is cnts (destination tag=0)
					// if channel is a stall source channel, then copy MADR to STADR
					//if ( c_iStallSource_LUT [ CTRL_Reg.STS ] == 5 )
					//{
					//	STADR_Reg = DmaCh [ iChannel ].MADR_Reg.Value;
					//}
					
					break;
					
				// ID: CNT
				case 1:
#ifdef INLINE_DEBUG_RUN_DMA5
	debug << "; ID=CNT";
#endif

					//TransferCount = ( QWC_Count > DmaCh [ 5 ].QWCRemaining ) ? DmaCh [ 5 ].QWCRemaining : QWC_Count;
					TransferCount = ( QWC_Count > pRegData [ 5 ]->QWC.QWC ) ? pRegData [ 5 ]->QWC.QWC : QWC_Count;

#ifdef INLINE_DEBUG_RUN_DMA5
	debug << "\r\n***EE SIF0 (IOP->EE) Writing QWC=" << dec << TransferCount << hex << " to MADR=" << pRegData [ 5 ]->MADR.Value << " Cycle#" << dec << *_DebugCycleCount << "\r\n";
#endif

					// transfer the data after the tag
					//for ( int i = 0; i < EETag.QWC; i++ )
					for ( int i = 0; i < TransferCount; i++ )
					{
						// transfer 128-bit quadword
						*DstPtr64++ = *Data64++;
						*DstPtr64++ = *Data64++;
						//*DstPtr++ = *Data++;
						//*DstPtr++ = *Data++;
					}
					
					// update QWC Transferred for tag
					DmaCh [ 5 ].QWCTransferred += TransferCount;
					QWC_Count -= TransferCount;
					
					DmaCh [ 5 ].QWCRemaining -= TransferCount;
					
					// update MADR
					pRegData [ 5 ]->MADR.Value += ( TransferCount << 4 );
					
					// update QWC
					pRegData [ 5 ]->QWC.QWC -= TransferCount;
					
					
#ifdef INLINE_DEBUG_RUN_DMA5
	debug << " Transferred=" << dec << TransferCount;
	debug << " Remaining=" << dec << DmaCh [ 5 ].QWCRemaining;
#endif

					// check transfer is complete
					//if ( DmaCh [ 5 ].QWCRemaining <= 0 )
					if ( !pRegData [ 5 ]->QWC.QWC )
					{
						// make sure this value is zero
						DmaCh [ 5 ].QWCRemaining = 0;
						
						// ***TODO*** NEED TO CHECK TIE BIT ????
						// check if IRQ requested
						//if ( SourceDMATag [ 5 ].IRQ && DmaCh [ 5 ].CHCR_Reg.TIE )
						if ( pRegData [ 5 ]->CHCR.IRQ && pRegData [ 5 ]->CHCR.TIE )
						{
#ifdef ENABLE_SIF_DMA_TIMING
							// actually end the transfer after data has been transferred serially
							// this delay should be handled at the source of the transfer
							SetNextEventCh_Cycle ( CycleDelay, 5 );
							
							// let the SIF know that the EE will be busy for awhile with the data just transferred
							SIF::_SIF->EE_BusyUntil ( CycleDelay );
#else
							// set CIS
							//Stat_Reg.Value |= ( 1 << 5 );
							
							// clear STR
							//DmaCh [ 5 ].CHCR_Reg.STR = 0;
							
							// end transfer
							EndTransfer ( 5, true );
							
							// interrupt for sif ??
							//SIF::SetInterrupt_EE_SIF ();
							
							// done - and finished
							return;
							//return 1;
#endif
						}
						
						// check if IOP was done after transfer
						/*
						if ( IOPDMATag [ 5 ].IRQ )
						{
#ifdef INT_ON_DMA5_STOP
							// send interrupt but keep dma 5 active ??
							
							// set CIS for channel - (STAT register)
							STAT_Reg.Value |= ( 1 << 5 );
							
							// check for interrupt
							UpdateInterrupt ();
							
							// update CPCOND0
							Update_CPCOND0 ();
#else
							
							// send interrupt and end dma 5 ??
							EndTransfer ( 5, true );
							
#endif
							
							return;
						}
						*/
						
						// has not transferred new tag yet, so need to return until next transfer comes in to get the new tag
						return;
						//return 0;
					}
					
					break;
				
				// ID: END
				case 7:
#ifdef INLINE_DEBUG_RUN_DMA5
	debug << "; ID=END";
	debug << "; NOT IMPLEMENTED";
#endif

					// need some type of message here..
					cout << "\nhps2x64: ***ALERT***: DMA Destination Tag END not implemented\n";
					break;
				
				default:
#ifdef INLINE_DEBUG_RUN_DMA5
	debug << "; DestTag=Unimplemented/Unknown";
#endif

					// need some type of message here..
					cout << "\nhps2x64: ***ALERT***: DMA Destination Tag UNKNOWN/IMPOSSIBLE not implemented\n";
					break;
			}	// end switch ( ID )
		
		}	// end if
		else
		{
			// not in destination chain mode for channel 5??
			cout << "\nhps2x64: DMA: ***ALERT***: DMA Channel#5 not in destination chain mode.\n";
		}
		
	}	// end while ( QWC_Count )
	
	// transfer not finished, but returning
	//return 0;
}




void Dma::NormalTransfer_ToMemory ( int iChannel )
{
	u32 SrcAddress, DstAddress, TransferCount, TransferIndex;
	u64 *SrcDataPtr, *DstDataPtr;
	
	u32 NextTagAddress, NextDataAddress;
	
	u64 QWC_TransferCount;
	
	u32 iDrainChannel, iQWRemaining1, iQWRemaining2, TransferAddress;
	
	static const u64 c_LoopTimeout = 33554432;
	u64 LoopCount;
	
	bool TransferInProgress = true;
	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
	debug << "; Normal";
#endif

#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
	debug << "; ToMemory";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	debug << " SADR=" << hex << pRegData [ iChannel ]->SADR.Value;
	debug << " QWC=" << hex << pRegData [ iChannel ]->QWC.Value;
#endif

	for ( LoopCount = 0; LoopCount < c_LoopTimeout; LoopCount++ )
	{
	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
	debug << " @Cycle#" << dec << *_DebugCycleCount;
#endif

		// check if transfer of block has started yet
		if ( QWC_Transferred [ iChannel ] < 0 )
		{
			// set the amount total in the block to be transferred
			QWC_BlockTotal [ iChannel ] = pRegData [ iChannel ]->QWC.QWC;
			
			// nothing transferred yet
			QWC_Transferred [ iChannel ] = 0;
		}

		
		// check if channel has a ready function to check if its ready
		// if not, then skip
		if ( cbReady [ iChannel ] )
		{
			// check that channel is ready
			if ( !( cbReady [ iChannel ] () ) )
			{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
	debug << "; ChannelNOTReady";
#endif

				return;
			}
		}
		
		
		// check if channel has a transfer function
		// if not, then unable to transfer
		if ( !cbTransfer_ToMemory [ iChannel ] )
		{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
	debug << "; ***TransferNOTImplemented***";
#endif

#ifdef DEBUG_COUT
			cout << "\nhps2x64: ALERT: PS2 DMA Normal Transfer not implemented. Channel=" << dec << iChannel << "\n";
#endif

			return;
		}
		
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
	debug << "; TransferingData";
#endif


		// transfer all data at once for now
		
		// get pointer to source data
		//SrcDataPtr = & DataBus::_BUS->MainMemory.b64 [ ( ( DmaCh [ iChannel ].MADR_Reg.Value & DataBus::MainMemory_Mask ) & ~0xf ) >> 3 ];
		SrcDataPtr = GetMemoryPtr ( pRegData [ iChannel ]->MADR.Value );
		
		// get pointer to where transfer last left off from
		// not sure if dma has an internal counter... will tak a look at this later
		//SrcDataPtr = & ( SrcDataPtr [ QWC_Transferred [ iChannel ] << 1 ] );

		// the amount of data to attemp to transfer depends on the buffer size for the device

#ifdef TEST_ASYNC_DMA
		QWC_TransferCount = ( c_iDeviceBufferSize [ iChannel ] < ( QWC_BlockTotal [ iChannel ] - QWC_Transferred [ iChannel ] ) ) ? c_iDeviceBufferSize [ iChannel ] : ( QWC_BlockTotal [ iChannel ] - QWC_Transferred [ iChannel ] );
		
		// STALL CONTROL //
		// check if channel is a stall destination
		if ( iChannel == c_iStallDest_LUT [ DMARegs.CTRL.STD ] )
		{
			if ( pRegData [ iChannel ]->MADR.Value >= DMARegs.STADR )
			{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
	debug << " *STALL*";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	debug << " STADR=" << DMARegs.STADR;
#endif

				// there is no data to transfer for stall destination
				
				// set stall interrupt
				DMARegs.STAT.SIS = 1;
				
				// update interrupts
				UpdateInterrupt ();
				
				// don't do this for stall interrupt
				//Update_CPCOND0 ();
				
				return;
			}
		}
		
		// only transfer if there is data to transfer
		if ( QWC_TransferCount )
		{
			if ( ( !DMARegs.CTRL.MFIFO ) || ( iChannel != 8 ) )
			{
				QWC_TransferCount = cbTransfer_ToMemory [ iChannel ] ( SrcDataPtr, QWC_TransferCount );
			}
			else
			{
				// source channel //
				
				
				// the amount of data that can be written to mfifo is difference between the MADRs minus the size of mfifo
				// note: probably supposed to compare with tadr from drain channel instead ??
				iQWRemaining2 = ( ( DMARegs.RBSR.Value + ( 1 << 4 ) ) - ( pRegData [ 8 ]->MADR.Value - pRegData [ iDrainChannel ]->MADR.Value ) ) >> 4;
				
				// then get the amount of data to write before you wraparound
				iQWRemaining1 = ( ( ( pRegData [ 8 ]->MADR.Value | DMARegs.RBSR.Value ) + ( 1 << 4 ) ) - pRegData [ 8 ]->MADR.Value ) >> 4;
				
				// check if amount that wants to be transfered is larger than amount that can be transferred
				iQWRemaining2 = ( ( QWC_TransferCount <= iQWRemaining2 ) ? QWC_TransferCount : iQWRemaining2 );
				
				// check if the amount to transfer before wrap around is greater then total amount to transfer
				iQWRemaining1 = ( ( iQWRemaining1 > iQWRemaining2 ) ? iQWRemaining2 : iQWRemaining1 );
				
				// get address MADR
				TransferAddress = pRegData [ iChannel ]->MADR.Value;
				
				// initialize amount of data transferred
				//QWC_TransferCount = 0;
				
				if ( iQWRemaining1 )
				{
					// get the data pointer
					SrcDataPtr = GetMemoryPtr ( ( TransferAddress & DMARegs.RBSR.Value ) | DMARegs.RBOR );
					
					// transfer the first run
					QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, iQWRemaining1 );
					
					// update transfer address
					TransferAddress += ( QWC_TransferCount << 4 );
					
					// remove from the total amount to transfer
					iQWRemaining2 -= iQWRemaining1;
					
					// if all the data was transferred, then transfer the wraparound also
					// note: can do this here since iQWRemaining1 is only zero when there is nothing to tranfer at all
					if ( iQWRemaining2 && ( QWC_TransferCount == iQWRemaining1 ) )
					{
						// get the data pointer
						SrcDataPtr = GetMemoryPtr ( ( TransferAddress & DMARegs.RBSR.Value ) | DMARegs.RBOR );
						
						// transfer the second run (wraparound)
						QWC_TransferCount += cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, iQWRemaining2 );
						
						// note: transfer address gets updated below
					}
					
				}
			}
		}
#else
		// perform transfer
		QWC_TransferCount = cbTransfer_ToMemory [ iChannel ] ( SrcDataPtr, QWC_BlockTotal [ iChannel ] - QWC_Transferred [ iChannel ] );
#endif
	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
		debug << " QWC_TransferCount=" << dec << QWC_TransferCount;
		debug << " Transferred=" << dec << QWC_Transferred [ iChannel ];
		debug << " Total=" << dec << QWC_BlockTotal [ iChannel ];
#endif

		// update address for next data to transfer
		//DmaCh [ iChannel ].MADR_Reg.Value += ( (u32) DmaCh [ iChannel ].QWC_Reg.QWC ) << 4;
		pRegData [ iChannel ]->MADR.Value += QWC_TransferCount << 4;
		
		// STALL CONTROL //
		// if channel is a stall source channel, then copy MADR to STADR
		if ( iChannel == c_iStallSource_LUT [ DMARegs.CTRL.STS ] )
		{
			DMARegs.STADR = pRegData [ iChannel ]->MADR.Value;
			
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
	debug << " *STALLCTRL*";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	debug << " STADR=" << DMARegs.STADR;
#endif
		}
		
		// does update MADR, but also updates QWC on slice
#ifdef UPDATE_QWC_ONLYONSLICE
		if ( iChannel < 8 )
		{
#endif
			// non-spr dma channel //
			
			// update QWC after transfer of block
			pRegData [ iChannel ]->QWC.QWC -= QWC_TransferCount;
			
#ifdef UPDATE_QWC_ONLYONSLICE
		}
#endif
		
		// update QWC transferred so far
		QWC_Transferred [ iChannel ] += QWC_TransferCount;
		
		// check if all data in the block has transferred
		if ( QWC_Transferred [ iChannel ] >= QWC_BlockTotal [ iChannel ] )
		{
			// all the data has been transferred in block, since this is not a chain transfer we are done
			EndTransfer ( iChannel );
			
			// transfer complete
			QWC_Transferred [ iChannel ] = -1;
			return;
		}
#ifdef TEST_ASYNC_DMA_STAGE2
		else if ( c_iDmaTransferTimePerQwc [ iChannel ] )
		{
			// if transfer is not finished, then schedule transfer to continue later
			SetNextEventCh ( ( c_iDmaTransferTimePerQwc [ iChannel ] * QWC_TransferCount ) + c_iSetupTime, iChannel );
			
			// continue transfer later
			return;
		}
#endif

	} // for LoopCount
	
	// if code ever reaches here, that means there was a timeout
	cout << "\nhps2x64 ERROR: Normal DMA Transfer to Channel#" << iChannel << " TIMED OUT";
}

void Dma::NormalTransfer_FromMemory ( int iChannel )
{
	//u64 Data0, Data1;
	//DMATag SrcDtag;
	//DMATag DstDtag;
	
	u32 SrcAddress, DstAddress, TransferCount, TransferIndex;
	u64 *SrcDataPtr, *DstDataPtr;
	
	u32 NextTagAddress, NextDataAddress;
	
	u64 QWC_TransferCount;
	
	static const u64 c_LoopTimeout = 33554432;
	u64 LoopCount;
	
	bool TransferInProgress = true;
	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
	debug << "; Normal";
#endif

#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
	debug << "; FromMemory";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
#endif

	for ( LoopCount = 0; LoopCount < c_LoopTimeout; LoopCount++ )
	{
	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
	debug << " @Cycle#" << dec << *_DebugCycleCount;
#endif

		// check if transfer of block has started yet
		if ( QWC_Transferred [ iChannel ] < 0 )
		{
			// set the amount total in the block to be transferred
			QWC_BlockTotal [ iChannel ] = pRegData [ iChannel ]->QWC.QWC;
			
			// nothing transferred yet
			QWC_Transferred [ iChannel ] = 0;
		}

		
		// check if channel has a ready function to check if its ready
		// if not, then skip
		if ( cbReady [ iChannel ] )
		{
			// check that channel is ready
			if ( !( cbReady [ iChannel ] () ) )
			{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
	debug << "; ChannelNOTReady";
#endif

				return;
			}
		}
		
		
		// check if channel has a transfer function
		// if not, then unable to transfer
		if ( !cbTransfer_FromMemory [ iChannel ] )
		{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
	debug << "; ***TransferNOTImplemented***";
#endif

#ifdef DEBUG_COUT
			cout << "\nhps2x64: ALERT: PS2 DMA Normal Transfer not implemented. Channel=" << dec << iChannel << "\n";
#endif

			return;
		}
		
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
	debug << "; TransferingData";
#endif


		// transfer all data at once for now
		
		// get pointer to source data
		//SrcDataPtr = & DataBus::_BUS->MainMemory.b64 [ ( ( DmaCh [ iChannel ].MADR_Reg.Value & DataBus::MainMemory_Mask ) & ~0xf ) >> 3 ];
		SrcDataPtr = GetMemoryPtr ( pRegData [ iChannel ]->MADR.Value );
		
		// get pointer to where transfer last left off from
		// not sure if dma has an internal counter... will tak a look at this later
		//SrcDataPtr = & ( SrcDataPtr [ QWC_Transferred [ iChannel ] << 1 ] );

		// the amount of data to attemp to transfer depends on the buffer size for the device

#ifdef TEST_ASYNC_DMA
		QWC_TransferCount = ( c_iDeviceBufferSize [ iChannel ] < ( QWC_BlockTotal [ iChannel ] - QWC_Transferred [ iChannel ] ) ) ? c_iDeviceBufferSize [ iChannel ] : ( QWC_BlockTotal [ iChannel ] - QWC_Transferred [ iChannel ] );

		// STALL CONTROL //
		// check if channel is a stall destination
		if ( iChannel == c_iStallDest_LUT [ DMARegs.CTRL.STD ] )
		{
			if ( pRegData [ iChannel ]->MADR.Value >= DMARegs.STADR )
			{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
	debug << " *STALL*";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	debug << " STADR=" << DMARegs.STADR;
#endif

				// there is no data to transfer for stall destination
				
				// set stall interrupt
				DMARegs.STAT.SIS = 1;
				
				// update interrupts
				UpdateInterrupt ();
				
				// don't do this for stall interrupt
				//Update_CPCOND0 ();
				
				return;
			}
			
			// make sure that MADR + transfercount <= STADR
			if ( ( pRegData [ iChannel ]->MADR.Value + ( QWC_TransferCount << 4 ) ) > DMARegs.STADR )
			{
				QWC_TransferCount = ( DMARegs.STADR - pRegData [ iChannel ]->MADR.Value ) >> 4;
			}
		}
		
		// only transfer if there is data to transfer
		if ( QWC_TransferCount )
		{
			QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, QWC_TransferCount );
		}
#else
		// perform transfer
		QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, QWC_BlockTotal [ iChannel ] - QWC_Transferred [ iChannel ] );
#endif
	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
		debug << " QWC_TransferCount=" << hex << QWC_TransferCount;
#endif

		// update address for next data to transfer
		//DmaCh [ iChannel ].MADR_Reg.Value += ( (u32) DmaCh [ iChannel ].QWC_Reg.QWC ) << 4;
		pRegData [ iChannel ]->MADR.Value += QWC_TransferCount << 4;
		
		// STALL CONTROL //
		// if channel is a stall source channel, then copy MADR to STADR
		if ( iChannel == c_iStallSource_LUT [ DMARegs.CTRL.STS ] )
		{
			DMARegs.STADR = pRegData [ iChannel ]->MADR.Value;
			
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
	debug << " *STALLCTRL*";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	debug << " STADR=" << DMARegs.STADR;
#endif
		}
		
		// does update MADR, but also updates QWC on slice
#ifdef UPDATE_QWC_ONLYONSLICE
		if ( iChannel < 8 )
		{
#endif
			// non-spr dma channel //
			
			// update QWC after transfer of block
			pRegData [ iChannel ]->QWC.QWC -= QWC_TransferCount;
			
#ifdef UPDATE_QWC_ONLYONSLICE
		}
#endif
		
		// update QWC transferred so far
		QWC_Transferred [ iChannel ] += QWC_TransferCount;
		
		// check if all data in the block has transferred
		if ( QWC_Transferred [ iChannel ] >= QWC_BlockTotal [ iChannel ] )
		{
			// all the data has been transferred in block, since this is not a chain transfer we are done
			EndTransfer ( iChannel );
			
			// transfer complete
			QWC_Transferred [ iChannel ] = -1;
			return;
		}
#ifdef TEST_ASYNC_DMA_STAGE2
		else if ( c_iDmaTransferTimePerQwc [ iChannel ] )
		{
			// if transfer is not finished, then schedule transfer to continue later
			SetNextEventCh ( ( c_iDmaTransferTimePerQwc [ iChannel ] * QWC_TransferCount ) + c_iSetupTime, iChannel );
			
			// continue transfer later
			return;
		}
#endif

	} // for LoopCount
	
	// if code ever reaches here, that means there was a timeout
	cout << "\nhps2x64 ERROR: Normal DMA Transfer to Channel#" << iChannel << " TIMED OUT";
}

void Dma::ChainTransfer_ToMemory ( int iChannel )
{
}


// should return the amount of data transferred
u64 Dma::Chain_TransferBlock ( int iChannel )
{
	u64 *TagDataPtr, *SrcDataPtr;
	u64 ullTagToTransfer [ 2 ];
	u32 QWC_TransferCount = 0;
	
	u32 iDrainChannel, iQWRemaining1, iQWRemaining2, TransferAddress;

	SrcDataPtr = GetMemoryPtr ( pRegData [ iChannel ]->MADR.Value );
	
	// check if transfer is just starting
	if ( QWC_Transferred [ iChannel ] < 0 )
	{

		/*
		// check if tag should be transferred
		// only transfer tag if transfer count > 0
		//if ( DmaCh [ iChannel ].CHCR_Reg.TTE && TransferCount )
		if ( DmaCh [ iChannel ].CHCR_Reg.TTE && QWC_BlockTotal [ iChannel ] )
		{
			
			// check if channel has a transfer function
			if ( cbTransfer_FromMemory [ iChannel ] )
			{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_0
	debug << "; TagTransfer";
#endif

				// Tag should be transferred //
				
				
				TagDataPtr = GetMemoryPtr ( DmaCh [ iChannel ].TADR_Reg.Value );
				
				// ***TODO*** when transferring tag, need to zero bottom 64-bits
				ullTagToTransfer [ 0 ] = 0;
				ullTagToTransfer [ 1 ] = TagDataPtr [ 1 ];
				
				// ?? don't actually transfer tag ??
				// or don't transfer for dma#2 ??
				if ( iChannel != 2 )
				{
				//cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, 1 );
				QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( ullTagToTransfer, 1 );
				
				// check if the tag was not transferred
				if ( !QWC_TransferCount )
				{
#if defined INLINE_DEBUG_TRANSFER
	debug << "; UnableToTransferTagYet";
#endif
					// the tag did not fully transfer
					return 0;
				}
				}
				
				
				// update amount transferred
				//QWC_Transferred [ iChannel ] += QWC_TransferCount;
				QWC_Transferred [ iChannel ] = 0;
			}
			else
			{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_1
	debug << "\r\n***ChainTransferNotImplemented; DMA Channel Number=" << dec << iChannel << " ***";
#endif

				// no transfer function, no transfer
#ifdef DEBUG_COUT
				cout << "\nhps2x64: ALERT: PS2 DMA Chain Transfer not implemented. Channel=" << dec << iChannel << "\n";
#endif

				return;
			}
			
			// before and after check is needed for transfer of TAG
			// since flush command could be dropped in
			// might need to fix up better later
			if ( cbReady [ iChannel ] )
			{
				// check if channel is ready for transfer
				if ( !( cbReady [ iChannel ] () ) )
				{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_2
	debug << "; DeviceNotReady";
#endif

					return 0;
				}
			}
			
		}
		else
		*/
		
		
		{
			// tag should NOT be transferred //
			
			// if transfer is just starting but no tag transferring, then enable transfer of data
			QWC_Transferred [ iChannel ] = 0;
		}
		
//#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_3
//	debug << " QWC_Transferred=" << hex << QWC_Transferred [ iChannel ];
//	debug << " ADDR=" << SourceDMATag [ iChannel ].ADDR;
//#endif
	}


//#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN
//	debug << " SrcDataPtr=" << (u64) SrcDataPtr;
//	debug << " Ptr=" << (u64) & ( DataBus::_BUS->MainMemory.b64 [ ( SourceDMATag [ iChannel ].ADDR & DataBus::MainMemory_Mask ) >> 3 ] );
//#endif
	
	// added - update source data pointer so that it points to the current quadword to transfer next
	//SrcDataPtr = & ( SrcDataPtr [ QWC_Transferred [ iChannel ] << 1 ] );

	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_4
	debug << "\r\n***DMA#" << iChannel << " " << DmaCh_Names [ iChannel ] << " Transfering QWC=" << hex << SourceDMATag [ iChannel ].QWC;
	debug << " from ADDR=" << SourceDMATag [ iChannel ].ADDR;
	debug << " TADR=" << pRegData [ iChannel ]->TADR.Value;
	debug << " MADR=" << pRegData [ iChannel ]->MADR.Value;
	debug << " SADR=" << pRegData [ iChannel ]->SADR.Value;
	debug << " EETag=" << SourceDMATag [ iChannel ].Value << " IOPTag=" << *((u64*) SrcDataPtr) << "\r\n";
#endif

	//if ( QWC_BlockTotal [ iChannel ] )
	if ( pRegData [ iChannel ]->QWC.QWC )
	{
		
		// check if channel has a transfer function
		if ( cbTransfer_FromMemory [ iChannel ] )
		{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_5
	debug << "\r\nLeft(QWC)=" << dec << pRegData [ iChannel ]->QWC.QWC;
#endif

//#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_6
// check if the size of block on PS2 side is same on PS1 side
// show address and size on PS1 side??
//if ( !QWC_Transferred [ iChannel ] )
//{
//	debug << " TAG??=" << hex << SrcDataPtr [ 0 ];
//	
//	if ( ( ( ( ((u32*)SrcDataPtr) [ 1 ] + 2 ) >> 2 ) + ( ( ( ((u32*)SrcDataPtr) [ 1 ] + 2 ) & 0x3 ) ? 1 : 0 ) ) != QWC_BlockTotal [ iChannel ] )
//	{
//	debug << "***PS1ALERT*** " << " BlockTotal=" << dec << QWC_BlockTotal [ iChannel ] << " PS1Total=" << ( ( ( ((u32*)SrcDataPtr) [ 1 ] + 2 ) >> 2 ) + ( ( ( ((u32*)SrcDataPtr) [ 1 ] + 2 ) & 0x3 ) ? 1 : 0 ) );
//	}
//}
//#endif

#ifdef TEST_ASYNC_DMA

			// actually need to account for cycle steal mode and dma channel
			//QWC_TransferCount = ( c_iDeviceBufferSize [ iChannel ] < ( DmaCh [ iChannel ].QWC_Reg.QWC ) ) ? c_iDeviceBufferSize [ iChannel ] : ( DmaCh [ iChannel ].QWC_Reg.QWC );
			
			// the amount to transfer in one go depends on whether Cycle Stealing is on (meaning the cpu can take over) and the dma channel
			// has the most effect on channels 8 and 9, so must handle that
			if ( ( !DMARegs.CTRL.CycleStealMode ) && ( iChannel >= 8 ) )
			{
#if defined INLINE_DEBUG_TRANSFER
	debug << "; CYCLESTEALOFF";
#endif

				// cycle steal mode is off and dma# 8 or 9 //
				
				// transfer all at once
				QWC_TransferCount = pRegData [ iChannel ]->QWC.QWC;
			}
			else
			{
				// the maximum amount that can be transferred is what can fit in the device buffer (for channels 8 and 9 use 8 QWs)
				QWC_TransferCount = ( c_iDeviceBufferSize [ iChannel ] < ( pRegData [ iChannel ]->QWC.QWC ) ) ? c_iDeviceBufferSize [ iChannel ] : ( pRegData [ iChannel ]->QWC.QWC );
			}

			// STALL CONTROL //
			// since chain transfer, check for refs tag
			if ( pRegData [ iChannel ]->CHCR.ID == 4 )
			{
				// check if channel is a stall destination
				if ( iChannel == c_iStallDest_LUT [ DMARegs.CTRL.STD ] )
				{
					if ( pRegData [ iChannel ]->MADR.Value >= DMARegs.STADR )
					{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_5
	debug << " *STALL*";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	debug << " STADR=" << DMARegs.STADR;
#endif

						// there is no data to transfer for stall destination
						
						// set stall interrupt
						DMARegs.STAT.SIS = 1;
						
						// update interrupts
						UpdateInterrupt ();
						
						// don't do this for stall interrupt
						//Update_CPCOND0 ();
						
						return;
					}
					
					// make sure that MADR + transfercount <= STADR
					if ( ( pRegData [ iChannel ]->MADR.Value + ( QWC_TransferCount << 4 ) ) > DMARegs.STADR )
					{
						QWC_TransferCount = ( DMARegs.STADR - pRegData [ iChannel ]->MADR.Value ) >> 4;
					}
				}
			}
			
			// only transfer if there is data to transfer
			if ( QWC_TransferCount )
			{
				// get drain channel
				iDrainChannel = DMARegs.CTRL.MFIFO - 1;
				
#ifdef VERBOSE_MFIFO_INVALID
				if ( !iDrainChannel )
				{
					// alert if mfifo is set to an invalid value
					cout << "\nhps2x64: ALERT: DMA: MFIFO=1!!!\n";
				}
#endif
				
				// check if mfifo is set
				// no need to check if dma#8 since it can only be run in normal mode for mfifo
				if ( ( !DMARegs.CTRL.MFIFO ) || ( iChannel != iDrainChannel ) )
				{
#if defined INLINE_DEBUG_TRANSFER
	debug << "; NO_MFIFO_TRANSFER";
#endif

					// no mfifo //
					
					QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, QWC_TransferCount );
				}
				else
				{
#if defined INLINE_DEBUG_TRANSFER
	debug << "; MFIFO";
#endif

					// mfifo //
					
					
					// channel must be the drain channel since this is a chain transfer
					
					
					// drain channel //
					
					// if drain channel TADR is same as source channel MADR, then mfifo is empty
					// must check TADR and not MADR for drain channel or else TADR will update
					if ( pRegData [ 8 ]->MADR.Value == pRegData [ iChannel ]->TADR.Value )
					{
#if defined INLINE_DEBUG_TRANSFER
	debug << "; MFIFO_EMPTY";
#endif

						// mfifo is empty //
						
						// set mfifo empty interrupt
						DMARegs.STAT.MEIS = 1;
						
						// update interrupts
						UpdateInterrupt ();
						
						// no data was transferred
						return 0;
					}
					
					// check if tag is cnt or end
					if ( ( pRegData [ iChannel ]->CHCR.ID == 1 ) || ( pRegData [ iChannel ]->CHCR.ID == 7 ) )
					{
#if defined INLINE_DEBUG_TRANSFER
	debug << "; MFIFO_TRANSFER";
	debug << "; RBSR=" << hex << DMARegs.RBSR.Value;
	debug << "; RBOR=" << hex << DMARegs.RBOR;
#endif

						// transfer data but check for mfifo wrap around
					
						// the amount of data to read from buffer is distance between source/drain madr
						// note: probably supposed to compare with tadr from drain channel instead ??
						//iQWRemaining2 = ( DmaCh [ 8 ].MADR_Reg.Value - DmaCh [ iChannel ].MADR_Reg.Value ) >> 4;
						
						// start by wrapping drain channel MADR around mfifo just in case
						pRegData [ iChannel ]->MADR.Value = Get_MfifoAddr ( pRegData [ iChannel ]->MADR.Value );
						
						// then get the amount of data to read before you wraparound
						iQWRemaining1 = ( ( ( pRegData [ iChannel ]->MADR.Value | DMARegs.RBSR.Value ) + ( 1 << 4 ) ) - pRegData [ iChannel ]->MADR.Value ) >> 4;
						
						// check if amount that wants to be transfered is larger than amount that can be transferred
						//iQWRemaining2 = ( ( QWC_TransferCount <= iQWRemaining2 ) ? QWC_TransferCount : iQWRemaining2 );
						
						// check if the amount to transfer before wrap around is greater then total amount to transfer
						//iQWRemaining1 = ( ( iQWRemaining1 > iQWRemaining2 ) ? iQWRemaining2 : iQWRemaining1 );
						iQWRemaining1 = ( ( iQWRemaining1 > QWC_TransferCount ) ? QWC_TransferCount : iQWRemaining1 );
						
						// get address MADR
						//TransferAddress = DmaCh [ iChannel ].MADR_Reg.Value;
						
						// initialize amount of data transferred
						//QWC_TransferCount = 0;
						
						if ( iQWRemaining1 )
						{
							// get the data pointer
							//SrcDataPtr = GetMemoryPtr ( ( TransferAddress & RBSR_Reg.Value ) | RBOR_Reg );
							SrcDataPtr = GetMemoryPtr ( pRegData [ iChannel ]->MADR.Value );
							
							// transfer the first run
							iQWRemaining2 = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, iQWRemaining1 );
							
							// update transfer address
							pRegData [ iChannel ]->MADR.Value += ( iQWRemaining2 << 4 );
							
							// update QWC
							pRegData [ iChannel ]->QWC.QWC -= iQWRemaining2;
							
							// wrap drain MADR around mfifo
							pRegData [ iChannel ]->MADR.Value = Get_MfifoAddr ( pRegData [ iChannel ]->MADR.Value );
							
							// update amount remaining to be transferred for now
							//QWC_TransferCount -= iQWRemaining2;
							
							// remove from the total amount to transfer
							//iQWRemaining2 -= iQWRemaining1;
							
							// if all the data was transferred, then transfer the wraparound also
							// note: can do this here since iQWRemaining1 is only zero when there is nothing to tranfer at all
							//if ( iQWRemaining2 && ( QWC_TransferCount == iQWRemaining1 ) )
							if ( ( iQWRemaining1 == iQWRemaining2 ) && ( ( QWC_TransferCount - iQWRemaining2 ) > 0 ) )
							{
								// get the data pointer
								SrcDataPtr = GetMemoryPtr ( pRegData [ iChannel ]->MADR.Value );
								
								// transfer the second run (wraparound)
								iQWRemaining1 = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, QWC_TransferCount - iQWRemaining2 );
								
								// update transfer address
								pRegData [ iChannel ]->MADR.Value += ( iQWRemaining1 << 4 );
								
								// update QWC
								pRegData [ iChannel ]->QWC.QWC -= iQWRemaining1;
								
								// wrap drain MADR around mfifo
								pRegData [ iChannel ]->MADR.Value = Get_MfifoAddr ( pRegData [ iChannel ]->MADR.Value );
								
								// set the total amount of data that was transferred
								QWC_TransferCount = iQWRemaining1 + iQWRemaining2;
								
							}
							else
							{
								// set the amount of data that was transferred
								QWC_TransferCount = iQWRemaining2;
							} // end if ( ( iQWRemaining1 == iQWRemaining2 ) && ( ( QWC_TransferCount - iQWRemaining2 ) > 0 ) )
							
							// update total data transferred
							QWC_Transferred [ iChannel ] += QWC_TransferCount;
							
							return QWC_TransferCount;
						}
						else
						{
							// nothing was transferred
							return 0;
						} // end if ( iQWRemaining1 )
					}
					else
					{
#if defined INLINE_DEBUG_TRANSFER
	debug << "; REGULAR_TRANSFER";
#endif

						// if tag is NOT cnt and NOT end, then just transfer the data
						
						// for now, just read the data
						QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, QWC_TransferCount );
						
					}
					
				}
			}
#else
			// perform transfer
			//QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, TransferCount );
			QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, QWC_BlockTotal [ iChannel ] - QWC_Transferred [ iChannel ] );
#endif

			// update QWC
			pRegData [ iChannel ]->QWC.QWC -= QWC_TransferCount;
			
			// update MADR
			pRegData [ iChannel ]->MADR.Value += ( QWC_TransferCount << 4 );
			
			// STALL CONTROL //
			// since chain transfer, first check that tag is cnts (destination tag=0)
			if ( !pRegData [ iChannel ]->CHCR.ID )
			{
				// if channel is a stall source channel, then copy MADR to STADR
				if ( iChannel == c_iStallSource_LUT [ DMARegs.CTRL.STS ] )
				{
					DMARegs.STADR = pRegData [ iChannel ]->MADR.Value;
					
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_5
	debug << " *STALLCTRL*";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	debug << " STADR=" << DMARegs.STADR;
#endif
				}
			}
			
//#ifdef INLINE_DEBUG_TRANSFER
//	debug << " QWC_TransferCount=" << hex << QWC_TransferCount;
//#endif

			// update total data transferred
			QWC_Transferred [ iChannel ] += QWC_TransferCount;
			
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_7
	debug << "\r\nTransferred(QWC)=" << dec << QWC_TransferCount;
	debug << "\r\nRemaining(QWC)=" << dec << pRegData [ iChannel ]->QWC.QWC;
#endif
		}
		else
		{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_8
	debug << "\r\n***ChainTransferNotImplemented; DMA Channel Number=" << dec << iChannel << " ***";
#endif

			// no transfer function, no transfer
#ifdef DEBUG_COUT
			cout << "\nhps2x64: ALERT: PS2 DMA Chain Transfer not implemented. Channel=" << dec << iChannel << "\n";
#endif

			return 0;
		}
	}
	
	// return the number of QWs transferred to device
	return QWC_TransferCount;
}


void Dma::ChainTransfer_FromMemory ( int iChannel )
{
	u64 Data0, Data1;
	//DMATag SrcDtag;
	//DMATag DstDtag;
	
	u64 ullTagToTransfer [ 2 ];
	
	u32 TransferAddress;
	
	//u32 SrcAddress, DstAddress, TransferCount, TransferIndex;
	u64 *SrcDataPtr, *DstDataPtr, *TagDataPtr;
	
	//u32 NextTagAddress, NextDataAddress;
	
	u64 QWC_TransferCount = 0;
	
	//bool TransferInProgress = true;
	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << "; Chain (Source [FromMemory])";
#endif

				

	//while ( TransferInProgress )
	while ( true )
	{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << "; TagSource: MainMemory";
	debug << " @Cycle#" << dec << *_DebugCycleCount;
#endif



		// check if transfer of block/tag has started yet
		//if ( QWC_Transferred [ iChannel ] < 0 )
		if ( !pRegData [ iChannel ]->QWC.QWC )
		{
		
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << "\r\nDMA#" << iChannel << " Loading Tag from TADR=" << hex << pRegData [ iChannel ]->TADR.Value << "\r\n";
#endif

			// if QWC is non-zero, then the transfer is being resumed ??
			/*
			if ( DmaCh [ iChannel ].QWC_Reg.QWC )
			{
#ifdef INLINE_DEBUG_START
	debug << "\r\nRestarting suspended Dma Channel#" << dec << iChannel;
#endif

				// set the source dma tag info
				SourceDMATag [ iChannel ].Value = ( ( (u64) DmaCh [ iChannel ].MADR_Reg.Value ) << 32 );
				SourceDMATag [ iChannel ].Value |= ( DmaCh [ iChannel ].CHCR_Reg.Value & 0xffff0000 ) | ( DmaCh [ iChannel ].QWC_Reg.Value & 0x0000ffff );

				// set the current transfer info
				QWC_Transferred [ iChannel ] = 0;
				QWC_BlockTotal [ iChannel ] = DmaCh [ iChannel ].QWC_Reg.QWC;
			}
			else
			*/
			
			
			{


			// MFIFO //
			// if mfifo, then make sure TADR is less than MADR for source
			if ( ( DMARegs.CTRL.MFIFO ) && ( iChannel == c_iMfifoDrain_LUT [ DMARegs.CTRL.MFIFO ] ) )
			{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << "; MFIFO";
#endif

				if ( pRegData [ iChannel ]->TADR.Value == pRegData [ 8 ]->MADR.Value )
				{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << " MFIFOEMPTY";
#endif

					// mfifo is empty //
					
					// set mfifo empty interrupt
					DMARegs.STAT.MEIS = 1;
					
					// update interrupts
					UpdateInterrupt ();
					
					// no data was transferred
					return;
				}
				
				// get transfer address for TAG (needs to wrap around for mfifo but not write back to TADR?)
				TransferAddress = ( pRegData [ iChannel ]->TADR.Value & DMARegs.RBSR.Value ) | DMARegs.RBOR;
			}
			else
			{
				// NOT mfifo //
				
				// otherwise, tag address is just TADR
				TransferAddress = pRegData [ iChannel ]->TADR.Value;
			}
			
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << " TADR=" << hex << pRegData [ iChannel ]->TADR.Value;
	debug << " TADR_TransferAddress=" << TransferAddress;
#endif

			// if destination chain mode channel#8, then the tag comes from SADR, and on channel#5 comes from the input data
			if ( iChannel == 8 )
			{
				TagDataPtr = & ( DataBus::_BUS->ScratchPad.b64 [ ( ( pRegData [ 8 ]->SADR.Address & DataBus::ScratchPad_Mask ) >> 3 ) & ~1 ] );
				Data0 = TagDataPtr [ 0 ];
				Data1 = TagDataPtr [ 1 ];
				
				// the data is 16 bytes after the tag in destination chain mode
				pRegData [ iChannel ]->SADR.Value += 16;
			}
			else
			{
				// load 128-bits from the tag address
				//TagDataPtr = GetMemoryPtr ( DmaCh [ iChannel ].TADR_Reg.Value );
				TagDataPtr = GetMemoryPtr ( TransferAddress );
				Data0 = TagDataPtr [ 0 ];
				Data1 = TagDataPtr [ 1 ];
			}

#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << "; TagData0=" << hex << Data0 << "; TagData1=" << Data1;
#endif


			// transfer the tag
			// ***todo*** this is probably the point at which you're supposed to transfer the tag
			//if ( DmaCh [ iChannel ].CHCR_Reg.TTE && QWC_BlockTotal [ iChannel ] )
			if ( pRegData [ iChannel ]->CHCR.TTE )
			{
			
				// make sure that device is ready for transfer of the next block
				// check if device is ready for transfer
				// check if channel has a ready function
				if ( cbReady [ iChannel ] )
				{
					// check if channel is ready for transfer
					if ( !( cbReady [ iChannel ] () ) )
					{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << "; DeviceNotReady";
#endif

						
						return;
					}
					
				} // end if ( cbReady [ iChannel ] )
				
		
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << "; DeviceIsReady";
#endif

				
				// check if channel has a transfer function
				if ( cbTransfer_FromMemory [ iChannel ] )
				{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_0
	debug << "; TagTransfer";
#endif

					// Tag should be transferred //
					
					
					//TagDataPtr = GetMemoryPtr ( DmaCh [ iChannel ].TADR_Reg.Value );
					
					// ***TODO*** when transferring tag, need to zero bottom 64-bits
					ullTagToTransfer [ 0 ] = 0;
					ullTagToTransfer [ 1 ] = TagDataPtr [ 1 ];
					
					// ?? don't actually transfer tag ??
					// or don't transfer for dma#2 ??
					if ( iChannel != 2 )
					{
					//cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, 1 );
					QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( ullTagToTransfer, 1 );
					
					// check if the tag was not transferred
					if ( !QWC_TransferCount )
					{
#if defined INLINE_DEBUG_TRANSFER
	debug << "; UnableToTransferTagYet";
#endif
						// the tag did not fully transfer
						//return 0;
						return;
					}
					}
					
					
					// update amount transferred
					//QWC_Transferred [ iChannel ] = 0;
				}

			}

			// looks like the upper 32-bits of Data0 is the address and the lower 32-bits are ID/PCE/FLG etc
			// Data1 appears to be zero and not used anyway
			// this is where I'll need to process the TAG since the address could mean anything
			
			// set the source dma tag
			//SrcDtag.Value = Data0;
			SourceDMATag [ iChannel ].Value = Data0;
			
			// set bits 16-31 of CHCR to the last tag read bits 16-31
			//DmaCh [ iChannel ].CHCR_Reg.Value = ( DmaCh [ iChannel ].CHCR_Reg.Value & 0xffffL ) | ( SourceDMATag [ iChannel ].Value & 0xffff0000L );
			pRegData [ iChannel ]->CHCR.Value = ( pRegData [ iChannel ]->CHCR.Value & 0xffffL ) | ( Data0 & 0xffff0000L );
			
// if pce is non-zero, then need to alert for now
if ( pRegData [ iChannel ]->CHCR.PCE )
{
cout << "\nhps2x64: ALERT: PCE is non-zero in dma tag!!!";
}

#if defined INLINE_DEBUG_TRANSFER
	debug << "; SETMADR";
#endif

			// if this is destination chain mode, then MADR gets set with ADDR for now
			// ***todo*** error checking
			if ( iChannel == 8 )
			{
				switch ( pRegData [ iChannel ]->CHCR.ID )
				{
					case 0:
					case 1:
					case 7:
						pRegData [ iChannel ]->MADR.Value = Data0 >> 32;
						break;
						
					default:
						cout << "\nhps2x64: DMA: ERROR: Invalid Destination tag ID=" << pRegData [ iChannel ]->CHCR.ID;
						break;
				}
			}
			else
			{

				// should probably also set MADR here
				switch ( pRegData [ iChannel ]->CHCR.ID )
				{
					// 0: refe - ADDR
					// 3: ref - ADDR
					// 4: refs - ADDR
					case 0:
					case 3:
					case 4:
						pRegData [ iChannel ]->MADR.Value = Data0 >> 32;
						break;
						
					// 1: cnt - next to tag
					// 2: next - next to tag
					// 5: call - next to tag
					// 6: ret - next to tag
					// 7: end - next to tag
					case 1:
					case 2:
					case 5:
					case 6:
					case 7:
						pRegData [ iChannel ]->MADR.Value = pRegData [ iChannel ]->TADR.Value + 16;
						break;
				}
				
			

#if defined INLINE_DEBUG_TRANSFER
	debug << "; SETTADR";
#endif

				// this is probably where the TAG address gets updated if necessary
				switch ( pRegData [ iChannel ]->CHCR.ID )
				{
					// 0: refe - does not update tag (but probably does??)
					// 3: ref - next to tag
					// 4: refs - next to tag
					case 0:
					case 3:
					case 4:
#if defined INLINE_DEBUG_TRANSFER
	debug << "; REF/REFE/REFS";
#endif

						pRegData [ iChannel ]->TADR.Value += 16;
						break;
					
					// looks like cnt is handled after transfer of block completes??
					// 1: cnt - next to transfer data
					//case 1:
					//	DmaCh [ iChannel ].TADR_Reg.Value += ( ( Data0 & 0xffff ) << 4 ) + 16;
					//	break;
						
					// 2: next - ADDR
					case 2:
#if defined INLINE_DEBUG_TRANSFER
	debug << "; NEXT";
#endif

						pRegData [ iChannel ]->TADR.Value = Data0 >> 32;
						break;
						
					// 5: call - ADDR (special handling)
					case 5:
#if defined INLINE_DEBUG_TRANSFER
	debug << "; CALL";
#endif
					
						// if ASP is 2 when the CALL tag is read, then the packet is NOT transferred
						// and also the address of the CALL tag is left in TADR
						if ( pRegData [ iChannel ]->CHCR.ASP >= 2 )
						{
							// end transfer
							EndTransfer ( iChannel );
							
							// done
							return;
						}
						
						// set next tag address to ADDR
						// this is probably done before transfer starts
						pRegData [ iChannel ]->TADR.Value = Data0 >> 32;
						
						// tag to push onto stack is next to data
						// this stuff is probably done when transfer completes
						//DmaCh [ iChannel ].TADR_Reg.Value += ( ( Data0 & 0xffff ) << 4 ) + 16;
						
						// push address onto stack
						//switch ( DmaCh [ iChannel ].CHCR_Reg.ASP )
						//{
						//	case 0:
						//		DmaCh [ iChannel ].ASR0_Reg.Value = DmaCh [ iChannel ].TADR_Reg.Value;
						//		break;
						//		
						//	case 1:
						//		DmaCh [ iChannel ].ASR1_Reg.Value = DmaCh [ iChannel ].TADR_Reg.Value;
						//		break;
						//}
							
						// increase the amound pushed onto stack
						//DmaCh [ iChannel ].CHCR_Reg.ASP += 1;
						
						
						break;
						
					// 6: ret - DxASR
					//case 6:
					//	// probably should be handled at end of transfer
					//	break;
				}

			}
			
			// MFIFO //
			// check if channel is drain channel
			// if so, then need to wrap around TADR
			/*
			if ( ( CTRL_Reg.MFIFO ) && ( iChannel == c_iMfifoDrain_LUT [ CTRL_Reg.MFIFO ] ) )
			{
				// mfifo is activated and this is the drain channel //
				
				// wrap around TADR for mfifo drain channel
				// possibly TADR does not get wrapped arround
				//DmaCh [ iChannel ].TADR_Reg.Value = Get_MfifoAddr ( DmaCh [ iChannel ].TADR_Reg.Value );
				
				// if the tag is cnt or end, then also wrap around MADR
				if ( ( DmaCh [ iChannel ].CHCR_Reg.ID == 1 ) || ( DmaCh [ iChannel ].CHCR_Reg.ID == 7 ) )
				{
					// mfifo is activated and dma tag is cnt or end //
					
					// wrap around MADR for mfifo drain channel
					DmaCh [ iChannel ].MADR_Reg.Value = Get_MfifoAddr ( DmaCh [ iChannel ].MADR_Reg.Value );
				}
			}
			*/
			
			// ?? put address into MADR ??
			//DmaCh [ iChannel ].MADR_Reg.Value = SrcDtag.ADDR;
			
			// set the transfer count
			//TransferCount = SrcDtag.QWC;
			//QWC_BlockTotal [ iChannel ] = SrcDtag.QWC;
			//QWC_BlockTotal [ iChannel ] = SourceDMATag [ iChannel ].QWC;
			QWC_BlockTotal [ iChannel ] = Data0 & 0xffff;
			
			// this is also probably supposed to set QWC
			pRegData [ iChannel ]->QWC.Value = Data0 & 0xffff;
			
			
#ifdef SET_QWC_CHAIN_TRANSFER
			pRegData [ iChannel ]->QWC.Value = 1;	//QWC_BlockTotal [ iChannel ];
#endif

			
			// ***todo*** might need to add onto block total if tag is being transferred too
			// instead set to -1 if transferring tag
			
			//if ( ! DmaCh [ iChannel ].CHCR_Reg.TTE )
			//{
			//	// transfer of block/tag is starting now
			//	QWC_Transferred [ iChannel ] = 0;
			//}
			//else
			//{
				QWC_Transferred [ iChannel ] = -1;
			//}
			
			}
			
			
			
		}

		
		
		if ( pRegData [ iChannel ]->QWC.QWC )
		{

			if ( cbReady [ iChannel ] )
			{
				// check if channel is ready for transfer
				if ( !( cbReady [ iChannel ] () ) )
				{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << "; DeviceNotReady";
#endif

					return;
				}
				
			} // end if ( cbReady [ iChannel ] )

#if defined INLINE_DEBUG_TRANSFER
	debug << "; READY";
	debug << " (before)QWC=" << dec << pRegData [ iChannel ]->QWC.QWC;
	debug << " TADR=" << hex << pRegData [ iChannel ]->TADR.Value;
#endif


			// perform the transfer
			QWC_TransferCount = Chain_TransferBlock ( iChannel );
			
#if defined INLINE_DEBUG_TRANSFER
	debug << " (after)QWC=" << dec << pRegData [ iChannel ]->QWC.QWC << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	debug << " TADR=" << hex << pRegData [ iChannel ]->TADR.Value;
	debug << " QWC_TransferCount=" << dec << QWC_TransferCount;
#endif

		} // end if ( DmaCh [ iChannel ].QWC_Reg.QWC )
		
		

		// check if transfer of block is complete
		//if ( QWC_Transferred [ iChannel ] >= QWC_BlockTotal [ iChannel ] )
		if ( !pRegData [ iChannel ]->QWC.QWC )
		{
#if defined INLINE_DEBUG_TRANSFER
	debug << "; !QWC";
	debug << " Channel#" << iChannel;
#endif

			// next tag is next to data
			//DmaCh [ iChannel ].TADR_Reg.Value += ( QWC_BlockTotal [ iChannel ] << 4 ) + 16;
			
			// start new tag
			QWC_Transferred [ iChannel ] = -1;
			
			// check if it is destination chain mode
			if ( iChannel == 8 )
			{
			
				// destination chain mode //
			
				switch ( pRegData [ iChannel ]->CHCR.ID )
				{
					// if the tag is end, then end the transfer
					case 7:
					
						// end transfer
						EndTransfer ( iChannel );
						
						// done
						return;
						
						break;
						
						
					default:
						break;
						
				}
				
			}
			else
			{
			
				// source chain mode //
			
				switch ( pRegData [ iChannel ]->CHCR.ID )
				{
					// if the tag is end or refe, then end the transfer
					case 0:
					case 7:
					
						// end transfer
						EndTransfer ( iChannel );
						
						// done
						return;
						
						break;
						
					// cnt probably does not update the tag address until after the end of transfer it appears
					// 1: cnt - next to transfer data
					case 1:
#if defined INLINE_DEBUG_TRANSFER
	debug << " CNT";
#endif

						//DmaCh [ iChannel ].TADR_Reg.Value += ( ( Data0 & 0xffff ) << 4 ) + 16;
						//DmaCh [ iChannel ].TADR_Reg.Value = DmaCh [ iChannel ].MADR_Reg.Value;
						pRegData [ iChannel ]->TADR.Value += ( SourceDMATag [ iChannel ].QWC << 4 ) + 16;
						break;
					
					// if the tag is call, probably best if it is handled...
					case 5:
#if defined INLINE_DEBUG_TRANSFER
	debug << " CALL";
#endif

						// tag to push onto stack is next to data
						// this stuff is probably done when transfer completes
						//DmaCh [ iChannel ].TADR_Reg.Value += ( ( Data0 & 0xffff ) << 4 ) + 16;
						
						// push address onto stack
						switch ( pRegData [ iChannel ]->CHCR.ASP )
						{
							case 0:
								pRegData [ iChannel ]->ASR0.Value = pRegData [ iChannel ]->MADR.Value;
								break;
								
							case 1:
								pRegData [ iChannel ]->ASR1.Value = pRegData [ iChannel ]->MADR.Value;
								break;
						}
							
						// increase the amound pushed onto stack
						pRegData [ iChannel ]->CHCR.ASP += 1;
						
						break;
						
						
					// if the tag is ret, then return, but if ASP is zero, then done
					case 6:
#if defined INLINE_DEBUG_TRANSFER
	debug << " RET";
#endif
					
						if ( pRegData [ iChannel ]->CHCR.ASP > 0 )
						{
#if defined INLINE_DEBUG_TRANSFER
	debug << " ASP>0";
#endif

							// decrease the amound pushed onto stack
							pRegData [ iChannel ]->CHCR.ASP -= 1;
							
							// next tag is popped from stack
							switch ( pRegData [ iChannel ]->CHCR.ASP )
							{
								case 0:
									pRegData [ iChannel ]->TADR.Value = pRegData [ iChannel ]->ASR0.Value;
									break;
									
								case 1:
									pRegData [ iChannel ]->TADR.Value = pRegData [ iChannel ]->ASR1.Value;
									break;
							}
						}
						else
						{
#if defined INLINE_DEBUG_TRANSFER
	debug << " ASP=0";
#endif

							// ret when ASP is zero //
							
							// set TADR to zero ?
							pRegData [ iChannel ]->TADR.Value = 0;
							
							// end transfer
							EndTransfer ( iChannel );
							
							// done
							return;
						}
						
						break;
				}
				
			}
			
			// if IRQ bit is set, then transfer no longer in progress
			// also check Tag Interrupt Enable (TIE)
			//if ( SourceDMATag [ iChannel ].IRQ && DmaCh [ iChannel ].CHCR_Reg.TIE )
			if ( pRegData [ iChannel ]->CHCR.IRQ && pRegData [ iChannel ]->CHCR.TIE )
			{
				// interrupt??
				EndTransfer ( iChannel );
				
				//TransferInProgress = false;
				return;
			}
		}



		
#ifdef TEST_ASYNC_DMA_STAGE2
		// block of data has been transferred, so if there delay for data transfer implement here
		if ( c_iDmaTransferTimePerQwc [ iChannel ] )
		{
#ifdef INLINE_DEBUG_TEST5
	debug << "\r\n***Testing***";
	debug << " iChannel=" << dec << iChannel;
	debug << " c_iDmaTransferTimePerQwc[]=" << c_iDmaTransferTimePerQwc [ iChannel ];
	debug << " QWC_TransferCount=" << QWC_TransferCount;
	debug << " c_iSetupTime=" << c_iSetupTime;
	debug << " Passing: " << ( c_iDmaTransferTimePerQwc [ iChannel ] * QWC_TransferCount ) + c_iSetupTime;
#endif

#if defined INLINE_DEBUG_TRANSFER
	debug << " TRANSFERTIME";
#endif

			// don't want to stop here if cpu is unable to interrupt the transfer
			if ( ( DMARegs.CTRL.CycleStealMode ) || ( iChannel < 8 ) )
			{
#if defined INLINE_DEBUG_TRANSFER
	debug << " DELAY";
#endif

				// continue transfer after data in device buffer has been processed
				SetNextEventCh ( ( c_iDmaTransferTimePerQwc [ iChannel ] * QWC_TransferCount ) + c_iSetupTime, iChannel );
				
#if defined INLINE_DEBUG_TRANSFER
	debug << " UNTIL " << dec << NextEventCh_Cycle [ iChannel ];
#endif
				
				return;
			}
		}
#endif
		
	}	// while loop
}

void Dma::InterleaveTransfer_ToMemory ( int iChannel )
{
}

void Dma::InterleaveTransfer_FromMemory ( int iChannel )
{
	u64 QWC_TransferCount = 0;
	u64 *SrcDataPtr;

	// check if transfer of block/tag has started yet
	if ( !pRegData [ iChannel ]->QWC.QWC )
	{
		// if QWC is zero, it means there is no transfer and complete
		
		// if STR is set, then should probably also call end transfer?
		if ( pRegData [ iChannel ]->CHCR.STR )
		{
			EndTransfer ( iChannel );
		}
		
		// done
		return;
	}
	
	// check if channel is a function to check if it is ready
	if ( cbReady [ iChannel ] )
	{
		// check if channel is ready for transfer
		if ( !( cbReady [ iChannel ] () ) )
		{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_INTERLEAVE
	debug << "; DeviceNotReady";
#endif

			// channel/device not ready for transfer
			return;
		}
	}
	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_INTERLEAVE
	debug << "; DeviceIsReady";
#endif


	// transfer the data
		
	// check if channel has a transfer function
	if ( cbTransfer_FromMemory [ iChannel ] )
	{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_INTERLEAVE
	//debug << "\r\nLeft(QWC)=" << dec << ( QWC_BlockTotal [ iChannel ] - QWC_Transferred [ iChannel ] );
#endif

		while ( true )
		{
			// get memory data pointer
			SrcDataPtr = GetMemoryPtr ( pRegData [ iChannel ]->MADR.Value );
			
			// get the amount to transfer
			QWC_TransferCount = ( ( (u32) DMARegs.SQWC.TQWC ) < ( pRegData [ iChannel ]->QWC.QWC ) ) ? ( (u32) DMARegs.SQWC.TQWC ) : ( pRegData [ iChannel ]->QWC.QWC );

			// perform the transfer
			QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, QWC_TransferCount );

			// update QWC
			pRegData [ iChannel ]->QWC.QWC -= QWC_TransferCount;
			
			// update MADR
			pRegData [ iChannel ]->MADR.Value += ( QWC_TransferCount << 4 ) + ( ( (u32) DMARegs.SQWC.SQWC ) << 4 );
			
			// check if done
			if ( !pRegData [ iChannel ]->QWC.QWC )
			{
				// if QWC is zero, it means there is no transfer and complete
				
				// if STR is set, then should probably also call end transfer?
				if ( pRegData [ iChannel ]->CHCR.STR )
				{
					EndTransfer ( iChannel );
				}
				
				// done
				return;
			}
			
			
			// check if transferring all at once or not
			if ( ( DMARegs.CTRL.CycleStealMode ) || ( iChannel < 8 ) )
			{
				// cycle steal mode is either on or it doesn't matter for now //
				
				// come back later
				SetNextEventCh ( ( c_iDmaTransferTimePerQwc [ iChannel ] * QWC_TransferCount ) + c_iSetupTime, iChannel );
				
				// done for now
				return;
				
			}
			
		} // end while ( true )
		
	} // end if ( cbTransfer_FromMemory [ iChannel ] )
		
}


// use this to complete transfer after dmas are restarted after a suspension
void Dma::UpdateTransfer ()
{
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		if ( i != 7 )
		{
			if ( pRegData [ i ]->CHCR.STR )
			{
				Transfer ( i );
			}
		}
	}
	
	/*
	// channel 0 has the highest priority
	if ( DmaCh [ 0 ].CHCR_Reg.STR )
		Transfer ( 0 );
	
	// the other channels come after that
	// skip channel 7
	if ( DmaCh [ 1 ].CHCR_Reg.STR )
		Transfer ( 1 );
		
	if ( DmaCh [ 2 ].CHCR_Reg.STR )
		Transfer ( 2 );
		
	if ( DmaCh [ 3 ].CHCR_Reg.STR )
		Transfer ( 3 );
		
	if ( DmaCh [ 4 ].CHCR_Reg.STR )
		Transfer ( 4 );
		
	if ( DmaCh [ 5 ].CHCR_Reg.STR )
		Transfer ( 5 );
		
	if ( DmaCh [ 6 ].CHCR_Reg.STR )
		Transfer ( 6 );
		
	if ( DmaCh [ 8 ].CHCR_Reg.STR )
		Transfer ( 8 );
		
	if ( DmaCh [ 9 ].CHCR_Reg.STR )
		Transfer ( 9 );
	*/
}


void Dma::Transfer ( int iChannel )
{
#ifdef INLINE_DEBUG_TRANSFER
	debug << "\r\nDma::Transfer; Channel#" << dec << iChannel;
	debug << "; (before) CHCR=" << hex << pRegData [ iChannel ]->CHCR.Value << " STAT=" << DMARegs.STAT.Value;
	debug << "; (before) MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
#endif

	//u64 Data0, Data1;
	
	//DMATag DstDtag;
	
	//u32 SrcAddress, DstAddress, TransferCount, TransferIndex;
	//u64 *SrcDataPtr, *DstDataPtr;
	
	//u32 NextTagAddress, NextDataAddress;
	
	//bool TransferInProgress = true;
	
	// check if dma transfers are being held
	if ( ( DMARegs.ENABLEW & 0x10000 ) /* && ( iChannel != 5 ) */ )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; HOLD";
	debug << "; MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
#endif

		// all dma transfers disabled
		return;
	}
	
	// check if channel STR is 1
	if ( !pRegData [ iChannel ]->CHCR.STR )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; Channel STR=0; Channel not enabled";
#endif

		return;
	}
	
	
#ifdef PRIORITY_DMA0
	// if dma#0 is running and this is not dma#0, dma#0 has priority
	if ( pRegData [ 0 ]->CHCR.STR && iChannel )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " DMA#0-HAS-PRIORITY";
#endif

		Transfer ( 0 );
		return;
	}
#endif
	

#ifdef PRIORITY_DMA1
	// if trying to transfer via dma#2, then check if there is currently a packet transferring for dma#2
	// if there is not, then if dma#1 is running give it priority?
	if ( pRegData [ 1 ]->CHCR.STR && iChannel == 2 && !( _GPU->PacketInProgress [ 2 ] ) )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " DMA#1-HAS-PRIORITY";
#endif

		Transfer ( 1 );
		return;
	}
#endif
	
	// if channel 5/6 (SIF0/SIF1) then make sure IOP is ready for transfer
	switch ( iChannel )
	{
		case 5:
		
			if ( !SIF::_SIF->IOP_DMA_Out_Ready () )
			{
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; PS1 SIF0 NOT READY";
#endif

				return;
			}
			
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; PS1 SIF0 READY";
#endif

			break;
			
		case 6:
		
			if ( !SIF::_SIF->IOP_DMA_In_Ready () )
			{
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; PS1 SIF1 NOT READY";
#endif

				return;
			}
			
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; PS1 SIF1 READY";
#endif

			break;
	}
	
	
	// check that dma is ready to run
	if ( *_DebugCycleCount < DmaCh [ iChannel ].ullStartCycle )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " DMA-SETUP";
#endif

		SetNextEventCh_Cycle ( DmaCh [ iChannel ].ullStartCycle, iChannel );
		return;
	}
	
	// check if channel has a dma setup time
	// dma setup times are disabled for now
	// will use the ullStartCycle variable for this
	/*
	if ( c_iDmaSetupTime [ iChannel ] )
	{
		// check if it is not time for DMA transfer to continue
		if ( NextEvent_Cycle != NextEventCh_Cycle [ iChannel ] )
		//if ( *_DebugCycleCount < ( DmaCh [ iChannel ].StartCycle + c_iDmaSetupTime [ iChannel ] ) )
		{
			// set transfer to continue after setup time
			SetNextEventCh ( c_iDmaSetupTime [ iChannel ], iChannel );
			//SetNextEventCh_Cycle ( DmaCh [ iChannel ].StartCycle + c_iDmaSetupTime [ iChannel ], iChannel );

			return;
		}
	}
	*/

	
	

#ifdef INLINE_DEBUG_TRANSFER
	debug << "; CHCR=" << hex << pRegData [ iChannel ]->CHCR.Value;
#endif

	// check if transfer is in chain mode
	switch ( pRegData [ iChannel ]->CHCR.MOD )
	{
		// Normal transfer //
		case 0:
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; Normal";
#endif

			switch ( iChannel )
			{

				// the channels that are always TO memory are 3, 5, 8
				case 3:
				case 5:	// will pull this in later
				case 8:
				
#ifdef VERBOSE_NORMAL_TOMEM
					cout << "\nhps2x64 ALERT: DMA: attempted NORMAL transfer TO memory via DMA Channel#" << dec << iChannel << "\n";
#endif

					// a normal transfer from dma#5 isn't accounted for
					// so alert if this happens and stop transfer
					if ( iChannel == 5 )
					{
						cout << "\nhps2x64: ALERT: DMA: Normal transfer for dma#5!!!\n";
						return;
					}
					
					// will use the transfer FROM memory function for now
					// perform NORMAL transfer FROM memory
					NormalTransfer_ToMemory ( iChannel );
					
					break;
					
				// the channels that are always FROM memory are 0, 2, 4, 6, 9
				case 0:
				case 2:
				case 4:
				case 6:
				case 9:
				
					if ( iChannel != 2 )
					{
#ifdef VERBOSE_NORMAL_FROMMEM
						cout << "\nhps2x64: ALERT: DMA: attempted non-gpu normal transfer from memory. Channel#" << iChannel << "\n";
#endif
					}
				
					// a normal transfer from dma#6 isn't accounted for
					// so alert if this happens and stop transfer
					if ( iChannel == 6 )
					{
						cout << "\nhps2x64: ALERT: DMA: Normal transfer for dma#6!!!\n";
						return;
					}
					
					// perform NORMAL transfer FROM memory
					NormalTransfer_FromMemory ( iChannel );
				
					break;
					
				// direction of dma transfer only matters for channels 1 and 7
				case 1:
				case 7:
				
					// check if this is going from memory or to memory
					switch ( pRegData [ iChannel ]->CHCR.DIR )
					{
						// to memory
						case 0:
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; ToMemory";
#endif

#ifdef VERBOSE_DMA_1_7_TO_MEM
							cout << "\nhps2x64 ALERT: DMA: attempted NORMAL transfer to memory via DMA Channel#" << dec << iChannel << "\n";
#endif
							
							NormalTransfer_ToMemory ( iChannel );
							break;
							
						// from memory
						case 1:
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; FromMemory";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
#endif

							//if ( iChannel != 2 )
							//{
							//	cout << "\nhps2x64: ALERT: DMA: attempted non-gpu normal transfer from memory. Channel#" << iChannel << "\n";
							//}
							
							// perform NORMAL transfer FROM memory
							NormalTransfer_FromMemory ( iChannel );

							break;
					}
						
					break;

			}	// switch for channel number
				
			break;	// NORMAL Transfer

				
		// Chain transfer (Source) //
		case 1:
		case 3:	// ??
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; Chain (Source [FromMemory])";
#endif


			// check the channel number
			switch ( iChannel )
			{
				case 5:
				
					// pull data from SIF for transfer
					// ***TODO*** this should actually read the data using a consistent method
					SIF::EE_DMA_ReadBlock ();
				
					break;
					
				/*
				case 6:
				
					
					if ( c_ullSIFOverhead )
					{
					
						if ( *_DebugCycleCount == NextEventCh_Cycle [ 6 ] )
						{
#ifdef INLINE_DEBUG_TRANSFER
	debug << "\r\nCH#6 TRANSFER";
#endif

							// pull data from SIF for transfer
							// ***TODO*** this should actually read the data using a consistent method
							ChainTransfer_FromMemory ( 6 );
						}
						else
						{
#ifdef INLINE_DEBUG_TRANSFER
	debug << "\r\nCH#6 OVERHEAD";
#endif

							// need to make the transfer later
							// can't transfer and then wait on the receiving side, have wait, then transfer on the sending side
							// might be that PS1 is clearing out data quickly after starting transfer
							// SIF buffer is 8 qwords, so I'll try waiting 8*128*2=2048 PS1 bus cycles
							//BusyUntil_Cycle [ 5 ] = *_DebugCycleCount + c_ullSIFOverhead;
							
							// repeat transfer after wait (transfer overhead)
							SetNextEventCh( c_ullSIFOverhead, 6 );
						}
					
					}
					else
					{
						ChainTransfer_FromMemory ( 6 );
					}
					
					
					break;
				*/
				
				// the channels that probably have direction controlled on the device side are 0,2,3,
				
				// the channels that are always TO memory are 3, 5, 8
				case 3:
				//case 5:	// will pull this in later
				case 8:

#ifdef VERBOSE_CHAIN_TOMEM
					cout << "\nhps2x64 ALERT: DMA: attempted DESTINATION CHAIN transfer TO memory via DMA Channel#" << dec << iChannel << "\n";
					//cout << "\n*** UNIMPLEMENTED ***\n";
#endif
					
					ChainTransfer_FromMemory ( iChannel );
					break;
				
				// the channels that are always FROM memory are 0, 2, 4, 6, 9
				case 0:
				case 2:
				case 4:
				case 6:
				case 9:
				
					// check for unimplemented items for testing
#ifdef VERBOSE_CHAIN_FROMMEM
					if ( iChannel == 4 || iChannel == 9 )
					{
						cout << "\nhps2x64 ALERT: DMA: attempted CHAIN transfer FROM memory via DMA Channel#" << dec << iChannel << "\n";
					}
#endif
					
					// perform CHAIN transfer FROM memory
					ChainTransfer_FromMemory ( iChannel );
				
					break;
					
				// direction of dma transfer only matters for channels 1 and 7
				case 1:
				case 7:
						
					// check if this is going from memory or to memory
					switch ( pRegData [ iChannel ]->CHCR.DIR )
					{
						// to memory
						case 0:
							cout << "\nhps2x64 ALERT: DMA: attempted CHAIN transfer TO memory via DMA Channel#" << dec << iChannel << "\n";
							break;
						
						// from memory
						case 1:
							ChainTransfer_FromMemory ( iChannel );
							break;
							
					}	// switch for DMA CHAIN transfer direction
					
			}	// switch for channel number
			
			
			break;	// CHAIN transfer
			
		// Interleave transfer (Scratch Pad)
		case 2:
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; Interleave";
#endif

			if ( iChannel >= 8 )
			{
#ifdef VERBOSE_INTERLEAVE
				// Interleave mode transfers data in a more rectangular pattern to/from scratchpad
				cout << "\nhps2x64: ALERT: DMA: Attempting Interleave DMA transfer. DMA Channel#" << dec << iChannel << "\n";
#endif
				
				// perform interleave transfer
				InterleaveTransfer_FromMemory ( iChannel );
			}
			else
			{
				// invalid interleave transfer
				cout << "\nhps2x64: ALERT: DMA: INVALID Interleave DMA transfer. DMA Channel#" << dec << iChannel << "\n";
			}

			break;
			
		default:
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; INVALID/ERROR";
#endif

			cout << "\nhps2x64: ALERT: DMA: INVALID Interleave DMA transfer mode. MOD=" << dec << pRegData [ iChannel ]->CHCR.MOD << "\n";
			break;
			
	}	// switch for DMA transfer type (normal, chain, interleave)
	
	// MFIFO //
	// check if the source channel (dma#8) has just written data for mfifo
	if ( ( DMARegs.CTRL.MFIFO > 1 ) && ( iChannel == 8 ) )
	{
		// if the drain channel is enabled, then attempt to run it
		if ( pRegData [ DMARegs.CTRL.MFIFO - 1 ]->CHCR.STR )
		{
			Transfer ( DMARegs.CTRL.MFIFO - 1 );
		}
	}
	
	// check if the drain channel has just written data for mfifo
	/*
	if ( ( CTRL_Reg.MFIFO > 1 ) && ( iChannel == c_iMfifoDrain_LUT [ CTRL_Reg.MFIFO ] ) )
	{
		// if the source channel is enabled, then attempt to run it
		if ( DmaCh [ 8 ].CHCR_Reg.STR )
		{
			Transfer ( 8 );
		}
	}
	*/
	
	// STALL CONTROL //
	// check if there is a stall destination channel with source channel
	// if the source channel has just performed a transfer
	if ( iChannel == c_iStallSource_LUT [ DMARegs.CTRL.STS ] )
	{
		if ( DMARegs.CTRL.STD && DMARegs.CTRL.STS )
		{
			// there is a stall destination channel set //
			
			// make sure it is not the same channel we are processing
			if ( iChannel != c_iStallDest_LUT [ DMARegs.CTRL.STD ] )
			{
				// if the drain channel is enabled, then run it
				if ( pRegData [ c_iStallDest_LUT [ DMARegs.CTRL.STD ] ]->CHCR.STR )
				{
					// retry the stall destination transfer
					Transfer ( c_iStallDest_LUT [ DMARegs.CTRL.STD ] );
				}
			}
		}
	}
}




static bool Dma::SPRout_DMA_Ready ()
{
	return true;
}

static bool Dma::SPRin_DMA_Ready ()
{
	return true;
}

// dma channel #8
static u32 Dma::SPRout_DMA_Read ( u64* Data, u32 QuadwordCount )
{
#ifdef INLINE_DEBUG_SPR_OUT
	debug << " SPRout";
	debug << " CycleStealing=" << pDMARegs->CTRL.CycleStealMode;
#endif

	u64 *pSrcDataPtr64;
	u32 MADR;
	u32 iDrainChannel;
	u32 iQWRemaining;
	
	// note: the transfer possibly does not update SADR or MADR, but can update this later
	// note: transfer is possibly a one-shot transfer
	
	// it is possible that scratchpad address is actually something else, so check
	if ( ( ( pRegData [ 8 ]->SADR.Address >> 24 ) & 0x11 ) == 0x11 )
	{
		cout << "hps2x64: ALERT: DMA8 SADR does not have a ScratchPad Address!!! Address=" << hex << pRegData [ 8 ]->SADR.Address;
	}

#ifdef VERBOSE_DMA8_MADR
	// it is possible that scratchpad address is actually something else, so check
	if ( ( ( pRegData [ 8 ]->MADR.Address >> 24 ) & 0x11 ) == 0x11 )
	{
		cout << "hps2x64: ALERT: DMA8 MADR does not have a RAM Address!!! Address=" << hex << pRegData [ 8 ]->MADR.Address;
	}
#endif
	
	// get ptr into scratch pad
	pSrcDataPtr64 = & ( DataBus::_BUS->ScratchPad.b64 [ ( ( pRegData [ 8 ]->SADR.Address & DataBus::ScratchPad_Mask ) >> 3 ) & ~1 ] );
	
	for ( int i = 0; i < QuadwordCount; i++ )
	{
		*Data++ = *pSrcDataPtr64++;
		*Data++ = *pSrcDataPtr64++;
	}
	
	// update SADR
	pRegData [ 8 ]->SADR.Address += ( QuadwordCount << 4 );
	
	// return amount transferred
	return QuadwordCount;
}

// dma channel #9
static u32 Dma::SPRin_DMA_Write ( u64* Data, u32 QuadwordCount )
{
#ifdef INLINE_DEBUG_SPR_IN
	debug << " SPRin";
	debug << " CycleStealing=" << pDMARegs->CTRL.CycleStealMode;
#endif

	u64 *pDstDataPtr64;
	
	// note: the transfer possibly does not update SADR or MADR, but can update this later
	// note: transfer is possibly a one-shot transfer
	
	// it is possible that scratchpad address is actually something else, so check
	if ( ( ( pRegData [ 9 ]->SADR.Address >> 24 ) & 0x11 ) == 0x11 )
	{
		cout << "hps2x64: ALERT: DMA9 SADR does not have a ScratchPad Address!!! Address=" << hex << pRegData [ 9 ]->SADR.Address;
	}

#ifdef VERBOSE_DMA9_MADR
	if ( ( ( pRegData [ 9 ]->MADR.Address >> 24 ) & 0x11 ) == 0x11 )
	{
		cout << "hps2x64: ALERT: DMA9 MADR does not have a RAM Address!!! Address=" << hex << pRegData [ 9 ]->MADR.Address;
	}
#endif
	
	// get ptr into scratch pad
	pDstDataPtr64 = & ( DataBus::_BUS->ScratchPad.b64 [ ( ( pRegData [ 9 ]->SADR.Address & DataBus::ScratchPad_Mask ) >> 3 ) & ~1 ] );
	
	for ( int i = 0; i < QuadwordCount; i++ )
	{
		*pDstDataPtr64++ = *Data++;
		*pDstDataPtr64++ = *Data++;
	}
	
	// update SADR
	pRegData [ 9 ]->SADR.Address += ( QuadwordCount << 4 );
	
	// return amount transferred
	return QuadwordCount;
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
	//MADR_Reg.Value = 0;
	//QWC_Reg.Value = 0;
	//CHCR_Reg.Value = 0;
	memset ( this, 0, sizeof(DmaChannel) );
}


// returns interrupt status
void Dma::DMA_Finished ( int index, bool SuppressDMARestart )
{
#ifdef INLINE_DEBUG_COMPLETE
	debug << "\r\n\r\nDMA" << dec << index << "::Finished; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << hex << "; Intc_Stat=" << *_Intc_Stat << "; _Intc_Mask=" << *_Intc_Mask << "; _R3000a_Status=" << *_R3000a_Status;
#endif
	
	/*
	u32 ICR_Prev;
	
	EndA = DmaCh [ index ].MADR + DmaCh [ index ].BCR.BS;
	
	// get previous value of ICR
	ICR_Prev = ICR_Reg.Value;
	
	// allow all dma's to operate at next opportunity
	SelectedDMA_Bitmap = 0xffffffff;
	
	// clear bit in bitmap for dma channel
	ChannelEnable_Bitmap &= ~( 1 << index );
	
	// stop the dma channel
	// note: both bits 24 and 28 get reset after the transfer
	DmaCh [ index ].CHCR.Value &= ~( ( 1L << 0x18 ) | ( 1L << 0x1c ) );	//dma->n_channelcontrol &= ~( ( 1L << 0x18 ) | ( 1L << 0x1c ) );
	
	// *** testing *** if the dma is finished, then bcr should be zero
	// note: this actually depends on the dma channel
	// note: only sync mode 1 decrements the upper 16-bits of BCR, all other transfer modes leave it as is
	//DmaCh [ index ].BCR.Value = 0;
	
	// check if dma interrupts are enabled for channel
	if ( ICR_Reg.Value & ( 1 << ( 16 + index ) ) )
	{
		// set interrupt pending for channel
		ICR_Reg.Value |= ( 1 << ( 24 + index ) );
		
		// only allow interrupt pending if the interrupt is enabled
		ICR_Reg.Value &= ( ( ICR_Reg.Value << 8 ) | 0x80ffffff );

		// check if there are any interrupts pending
		if ( ICR_Reg.Value & 0x7f000000 )
		{
			// set interrupt pending flag
			ICR_Reg.Value |= 0x80000000;
		}
		else
		{
			// clear interrupt pending flag
			ICR_Reg.Value &= 0x7fffffff;
			
			
			// *** TESTING ***
			//ClearInterrupt ();
		}
		
		// check if dma interrupts are enabled globally
		// also check that bit 31 transitioned from 0 to 1
		//if ( ( ( ICR_Reg.Value >> 23 ) & 1 ) )
		if ( ( ! ( ICR_Prev & 0x80000000 ) ) && ( ( ICR_Reg.Value & 0x80800000 ) == 0x80800000 ) )
		{
			// check if dma interrupts are enabled for channel
			
			// send interrupt signal
			SetInterrupt ();
		}
	}
	
	// now that the dma channel is finished, check what channel is next and run it immediately
	ActiveChannel = GetActiveChannel ();
	
	if ( !SuppressDMARestart )
	{
		DMA_Run ( ActiveChannel );
	}
	
	// make sure the cycle number for the next dma event is updated
	Update_NextEventCycle ();
	
	// no more events for this particular channel, cuz it is finished
	//SetNextEventCh ( 0, index );
	*/
}



void Dma::SetNextEventCh ( u64 Cycles, u32 Channel )
{
#ifdef INLINE_DEBUG_NEXTEVENT
	debug << "\r\nDma::SetNextEventCh; CycleCount=" << dec << *_DebugCycleCount;
	debug << " (before) Cycles=" << Cycles << " Channel=" << Channel;
#endif

	NextEventCh_Cycle [ Channel ] = Cycles + *_DebugCycleCount;
	
	Update_NextEventCycle ();
}


void Dma::SetNextEventCh_Cycle ( u64 Cycle, u32 Channel )
{
#ifdef INLINE_DEBUG_NEXTEVENT
	debug << "\r\nDma::SetNextEventCh_Cycle; CycleCount=" << dec << *_DebugCycleCount;
	debug << " (before) Cycle=" << Cycle << " Channel=" << Channel;
#endif

	NextEventCh_Cycle [ Channel ] = Cycle;
	
	//cout << "\nTEST: Channel=" << dec << Channel << " NextEventCh_Cycle [ Channel ]=" << NextEventCh_Cycle [ Channel ] << " Cycle=" << Cycle;
	
	Update_NextEventCycle ();
}

void Dma::Update_NextEventCycle ()
{
#ifdef INLINE_DEBUG_NEXTEVENT
	debug << "\r\nDma::Update_NextEventCycle; Cycle=" << dec << *_DebugCycleCount;
	debug << " (before) NextEvent_Cycle=" << NextEvent_Cycle << " NextSystemEvent=" << *_NextSystemEvent;
#endif

	NextEvent_Cycle = -1LL;
	
	for ( int i = 0; i < NumberOfChannels; i++ )
	{
		//if ( NextEventCh_Cycle [ i ] > *_DebugCycleCount && ( NextEventCh_Cycle [ i ] < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) )
		//if ( NextEventCh_Cycle [ i ] < NextEvent_Cycle )
		if ( ( NextEventCh_Cycle [ i ] > *_DebugCycleCount ) && ( NextEventCh_Cycle [ i ] < NextEvent_Cycle ) )
		{
			// the next event is the next event for device
			NextEvent_Cycle = NextEventCh_Cycle [ i ];
		}
	}

	//if ( NextEvent_Cycle > *_DebugCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_DebugCycleCount ) )
	if ( NextEvent_Cycle < *_NextSystemEvent )
	{
		*_NextSystemEvent = NextEvent_Cycle;
		*_NextEventIdx = NextEvent_Idx;
	}
	
	//cout << "\nTEST: dma1 next event cycle=" << dec << NextEventCh_Cycle [ 1 ];
	//cout << "\nTEST: dma next event cycle=" << dec << NextEvent_Cycle;
	
#ifdef INLINE_DEBUG_NEXTEVENT
	debug << " (after) NextEvent_Cycle=" << NextEvent_Cycle << " NextSystemEvent=" << *_NextSystemEvent;
#endif
}



static u64* Dma::GetMemoryPtr ( u32 Address )
{
	if ( Address >> 31 )
	{
		// if SPR bit is set, then it is an SPR Memory address
		return & ( DataBus::_BUS->ScratchPad.b64 [ ( ( Address & DataBus::ScratchPad_Mask ) >> 3 ) & ~1 ] );
	}
	
	if ( ( Address >> 24 ) == 0x11 )
	{
		// in this case, it must be a VU memory address
		if ( Address < 0x11004000 )
		{
			// micro mem 0 address
			return & ( DataBus::_BUS->MicroMem0 [ ( ( Address & DataBus::MicroMem0_Mask ) >> 3 ) & ~1 ] );
		}
		else if ( Address < 0x11008000 )
		{
			// vu mem 0 //
			Address -= 0x11004000;
			return & ( DataBus::_BUS->VuMem0 [ ( ( Address & DataBus::MicroMem0_Mask ) >> 3 ) & ~1 ] );
		}
		else if ( Address < 0x1100c000 )
		{
			// micro mem 1 //
			Address -= 0x10008000;
			return & ( DataBus::_BUS->MicroMem1 [ ( ( Address & DataBus::MicroMem1_Mask ) >> 3 ) & ~1 ] );
		}
		
		// vu mem 1 //
		Address -= 0x1100c000;
		return & ( DataBus::_BUS->VuMem1 [ ( ( Address & DataBus::MicroMem1_Mask ) >> 3 ) & ~1 ] );
	}
	
	// otherwise, it is a main memory address
	return & ( DataBus::_BUS->MainMemory.b64 [ ( ( Address & DataBus::MainMemory_Mask ) >> 3 ) & ~1 ] );
}




static void Dma::DebugWindow_Enable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	static const char* DebugWindow_Caption = "PS2 DMA Debug Window";
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
		
		DMA_ValueList->AddVariable ( "DMA_CTRL", &( pDMARegs->CTRL.Value ) );
		DMA_ValueList->AddVariable ( "DMA_STAT", &( pDMARegs->STAT.Value ) );
		DMA_ValueList->AddVariable ( "DMA_PCR", &( pDMARegs->PCR.Value ) );

		for ( i = 0; i < NumberOfChannels; i++ )
		{
			ss.str ("");
			ss << "DMA" << i << "_MADR";
			DMA_ValueList->AddVariable ( ss.str().c_str(), &( pRegData [ i ]->MADR.Value ) );
			
			ss.str ("");
			ss << "DMA" << i << "_BCR";
			DMA_ValueList->AddVariable ( ss.str().c_str(), &( pRegData [ i ]->QWC.Value ) );
			
			ss.str ("");
			ss << "DMA" << i << "_CHCR";
			DMA_ValueList->AddVariable ( ss.str().c_str(), &( pRegData [ i ]->CHCR.Value ) );
		}
		
		// add start and end addresses for dma transfers
		//DMA_ValueList->AddVariable ( "StartA", &( _DMA->StartA ) );
		//DMA_ValueList->AddVariable ( "EndA", &( _DMA->EndA ) );
		
		// add primitive count and frame count here for now
		//DMA_ValueList->AddVariable ( "PCount", &( _GPU->Primitive_Count ) );
		//DMA_ValueList->AddVariable ( "FCount", &( _GPU->Frame_Count ) );
		
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


}



