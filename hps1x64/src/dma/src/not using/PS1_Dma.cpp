/*
	Copyright (C) 2012-2016

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


#ifdef _DEBUG_VERSION_

// enable debugging

#define INLINE_DEBUG_ENABLE


#define INLINE_DEBUG_WRITE 
#define INLINE_DEBUG_COMPLETE
#define INLINE_DEBUG_READ

#define INLINE_DEBUG_AUTODMA
#define INLINE_DEBUG_RUN_AUTODMA

#define INLINE_DEBUG_RUN_DMA3

#define INLINE_DEBUG_RUN_DMA9
#define INLINE_DEBUG_RUN_DMA10


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


#endif


funcVoid Dma::UpdateInterrupts;

u32* Dma::_DebugPC;
u64* Dma::_DebugCycleCount;
u64* Dma::_SystemCycleCount;
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




#ifdef PS2_COMPILE

void Dma::DMA7_Run ( bool ContinueToCompletion )
{
		// DMA 7 is running
#ifdef INLINE_DEBUG_RUN_DMA7
	debug << "\r\n";
	debug << "; DMA7: SPU2";
#endif
		
	u32 Temp;
	u32 Data [ 4 ];
	u32 NumberOfTransfers;
	
	u64 CyclesPerTransfer, BusCyclesPerTransfer;
	u64 BusCycles;
	
	
	/*
	if ( !_BUS->isReady () )
	{
#ifdef INLINE_DEBUG_RUN_DMA4
	debug << "; BUS_BUSY";
#endif

		// bus is not ready //
		// reschedule transfer for when bus is ready
		SetNextEventCh_Cycle ( _BUS->BusyUntil_Cycle, 4 );
	}
	else
	*/
	
	{
#ifdef INLINE_DEBUG_RUN_DMA7
	debug << "; BUS_FREE";
#endif
		// bus is free //

		// check that SPU is ready for DMA transfer
		if ( ( _SPU->DMA_ReadyForRead () && !DmaCh [ 7 ].CHCR.DR ) || ( _SPU->DMA_ReadyForWrite () && DmaCh [ 7 ].CHCR.DR ) )
		{
#ifdef INLINE_DEBUG_RUN_DMA7
	debug << "; SPU_READY";
#endif
				
			/////////////////////////////////////////////////////
			// Check if we are transferring to or from memory
			if ( DmaCh [ 7 ].CHCR.DR )
			{
				/////////////////////////////////////////////////////
				// Transferring FROM memory
#ifdef INLINE_DEBUG_RUN_DMA7
					debug << ";FROM";
					debug << "; BA=" << hex << DmaCh [ 7 ].BCR.BA << "; BS=" << DmaCh [ 7 ].BCR.BS;
#endif

				// only finished when BA is zero!
				if ( !DmaCh [ 7 ].BCR.BA )
				{
#ifdef INLINE_DEBUG_RUN_DMA7
	debug << "; FINISHED";
#endif

					// dma transfer is done //
					DMA_Finished ( 7 );
					
					// let SPU know dma transfer is done also
					SPU2::_SPU2->SPU1.SpuTransfer_Complete ();
					
					return;
				}
				
				
				// *** todo: SPU bus might be 2 bytes wide, so time to transfer is times 2 *** //

				
#ifdef ENABLE_AUTODMA

				// check if ADMA transfer
				if ( SPU2::_SPU2->SPU1.isADMATransferMode () )
				{
#ifdef INLINE_DEBUG_RUN_AUTODMA
					debug << "\r\n***AUTODMA7 STARTED***\r\n";
#endif
					// do not transfer here, because will need separate function to "auto-transfer"
					// will be invoked from SPU side since SPU knows when transfer is needed

					// disable cpu freezing (which needs to be fixed)
					ContinueToCompletion = false;
					
					if ( !SPU2::_SPU2->SPU1.DMA_ReadyForWrite () )
					{
#ifdef VERBOSE_AUTODMA_START
					cout << "\nhps1x64: SPU2: AutoDma7 Transfer Starting!\n";
#endif
#ifdef INLINE_DEBUG_RUN_AUTODMA
					debug << "; DMA7 NOT ReadyForWrite";
#endif

						return;
					}
				
#ifdef INLINE_DEBUG_RUN_AUTODMA
					debug << "; DMA7 ReadyForWrite";
#endif

					/*
					// check SPU is decoding in the other buffer
					// if SPU decoding in the other buffer, transfer all left/right samples for half buffer for now
					// otherwise, set next transfer event for 256 samples * 768 cycles/sample, if unable to transfer now
					// if was able to transfer, then interrupt
					*/
				}
#endif
				
				// update busy cycles
				//NumberOfTransfers = DmaCh [ 7 ].BCR.BS * DmaCh [ 7 ].BCR.BA;
				//_BUS->ReserveBus ( NumberOfTransfers );
				NumberOfTransfers = DmaCh [ 7 ].BCR.BS;
				//_BUS->ReserveBus ( NumberOfTransfers * 2 );
				BusyCycles = NumberOfTransfers + 2;
				
				//SetNextEventCh ( BusyCycles + c_SPU_CycleTime, 7 );
				
				CyclesPerTransfer = ( DmaCh [ 7 ].BCR.BS << 2 ) + c_SPU_CycleTime;
				BusCyclesPerTransfer = ( DmaCh [ 7 ].BCR.BS << 2 );
				
				BusCycles = 0;
				BusyCycles = 0;
				
				if ( DmaCh [ 7 ].BCR.BS != 0x10 )
				{
					cout << "\nhps1x64 ERROR: *** ALERT *** DmaCh[ 7 ].BCR.BS=" << DmaCh [ 7 ].BCR.BS;
				}

				if ( NumberOfTransfers )
				{
				
					do
					{
						// dma 7 actually goes to SPU Core #1
						//_SPU->DMA_Write_Block ( & ( _BUS->MainMemory.b32 [ ( DmaCh [ 7 ].MADR & 0x1fffff ) >> 2 ] ), DmaCh [ 7 ].BCR.BS );
						SPU2::_SPU2->SPU1.DMA_Write_Block ( & ( _BUS->MainMemory.b32 [ ( DmaCh [ 7 ].MADR & 0x1fffff ) >> 2 ] ), DmaCh [ 7 ].BCR.BS );
						
						DmaCh [ 7 ].MADR += 64;
						
						// restore size
						//DmaCh [ 4 ].BCR.Value = TransferSize_Save [ 4 ];
						
						//////////////////////////////////////////////////////////
						// Decrease Block Count by 1
						DmaCh [ 7 ].BCR.BA--;
						
						// save size
						//TransferSize_Save [ 7 ] = DmaCh [ 7 ].BCR.Value;
						
						// update cycles that dma is busy
						BusCycles += BusCyclesPerTransfer;
						BusyCycles += CyclesPerTransfer;
					}
					while ( ContinueToCompletion && DmaCh [ 7 ].BCR.BA );
				}
				
				// clear transfer amount remaining
				// note: don't clear transfer amount remaining since block count should be zero but block size should not change
				// It actually looks like at the end of transfer that BCR is zero for sync mode 1
				// but Martin Korth's psx specifications says only BA is zero after the transfer
				//DmaCh [ 4 ].BCR.Value = 0;
						
						
				if ( ContinueToCompletion )
				{
					// transfer froze the CPU //
					
					// reserve the bus for the time DMA was busy
					// must be enough to freeze CPU
					_BUS->ReserveBus ( BusyCycles );
					
					// mark dma channel as finished
					DMA_Finished ( 7 );
					
					// done
					return;
				}
				else
				{
					// transfer did not freeze CPU //
					
					// reserve the bus for the time it was in use
					_BUS->ReserveBus ( BusCycles );
					
					SetNextEventCh ( BusyCycles, 7 );
				}
			}
			else
			{
				/////////////////////////////////////////////////////
				// Transferring TO memory
#ifdef INLINE_DEBUG_RUN_DMA7
					debug << ";TO";
#endif

				// *** todo: transfer all data at once *** //
				
				// update busy cycles
				NumberOfTransfers = DmaCh [ 7 ].BCR.BS * DmaCh [ 7 ].BCR.BA;
				_BUS->ReserveBus ( NumberOfTransfers );
				BusyCycles = NumberOfTransfers + 2;
				//SetNextEventCh ( BusyCycles, 4 );

				if ( NumberOfTransfers )
				{
					while ( DmaCh [ 7 ].BCR.BA )
					{
						while ( DmaCh [ 7 ].BCR.BS )
						{
							DmaCh [ 7 ].BCR.BS--;
							
							_SPU->DMA_Read ( & (Data [ 0 ]), 1 );
							_BUS->MainMemory.b32 [ ( DmaCh [ 7 ].MADR & 0x1fffff ) >> 2 ] = Data [ 0 ];
							
							///////////////////////////////////////////////////////////
							// Update memory address
							DmaCh [ 7 ].MADR += 4;
						}
						
						// restore size
						DmaCh [ 7 ].BCR.Value = TransferSize_Save [ 7 ];
						
						//////////////////////////////////////////////////////////
						// Decrease Block Count by 1
						DmaCh [ 7 ].BCR.BA--;
						
						// save size
						TransferSize_Save [ 7 ] = DmaCh [ 7 ].BCR.Value;
					}
				}
				
				// clear transfer amount remaining
				// It looks like at the end of transfer that BCR is zero for sync mode 1
				// but Martin Korth's psx specifications says only BA is zero after the transfer
				//DmaCh [ 7 ].BCR.Value = 0;

#ifdef INLINE_DEBUG_RUN_DMA7
	debug << "; FINISHED";
#endif

				// dma transfer is done //
				DMA_Finished ( 7 );
			}
			
		}
	
	}
	
}

#endif



void Dma::DMA6_Run ( bool ContinueToCompletion )
{
#ifdef INLINE_DEBUG_RUN_DMA6
	debug << "; DMA6: OTC";
#endif

	// dma transfer has been started //
	
	u32 Temp;
	u32 Data [ 4 ];
	u32 NumberOfTransfers;
	
	// check if bus is in use by another device //
	/*
	if ( !_BUS->isReady() )
	{
#ifdef INLINE_DEBUG_RUN_DMA6
	debug << "; BUS_BUSY";
#endif

		// bus is busy //
		
		// reschedule transfer for when bus is free
		SetNextEventCh_Cycle ( _BUS->BusyUntil_Cycle, 6 );
	}
	else
	*/
	
	{
#ifdef INLINE_DEBUG_RUN_DMA6
	debug << "; BUS_FREE";
#endif

		// bus is free //
		
		// set the number of transfers to make
		NumberOfTransfers = DmaCh [ 6 ].BCR.BS;
		
		if ( !NumberOfTransfers )
		{
			cout << "\nhps1x64 ALERT: DMA 6 (OTC): BS Transfer size is zero.\n";
			//NumberOfTransfers = 0x10000;
		}
		
		////////////////////////////////////
		// mark bus as in use this cycle
		//_BUS->ReserveBus ( DmaCh [ 6 ].BCR.Value );
		_BUS->ReserveBus ( NumberOfTransfers );
		//BusyCycles = DmaCh [ 6 ].BCR.Value + 2;
		BusyCycles = NumberOfTransfers + 2;
		//SetNextEventCh ( BusyCycles, 6 );

#ifdef INLINE_DEBUG_RUN_DMA6
	debug << "; STORE";
#endif

		// *** do entire dma 6 transfer at once *** //
		// *note* dma6 is always a one-shot transfer
		
		Temp = DmaCh [ 6 ].MADR;
		
		if ( NumberOfTransfers )
		{

			// account for the final transfer
			//DmaCh [ 6 ].BCR.Value--;
			NumberOfTransfers--;
			
			//while ( DmaCh [ 6 ].BCR.Value )
			while ( NumberOfTransfers )
			{
				///////////////////////////////////////////////
				// Send previous address entry to main memory
				_BUS->MainMemory.b32 [ ( DmaCh [ 6 ].MADR & 0x1fffff ) >> 2 ] = ( DmaCh [ 6 ].MADR - 4 ) & 0x1fffff;
				
				////////////////////////////////////////////////////
				// decrease address
				DmaCh [ 6 ].MADR -= 4;
				
				////////////////////////////////////////////
				// decrease count
				//DmaCh [ 6 ].BCR.Value--;
				NumberOfTransfers--;
			}
			
			//////////////////////////////////////////////////
			// Send 0x00ffffff address entry to main memory
			_BUS->MainMemory.b32 [ ( DmaCh [ 6 ].MADR & 0x1fffff ) >> 2 ] = 0x00ffffff;
			
			////////////////////////////////////////////////////
			// decrease address
			//DmaCh [ 6 ].MADR -= 4;
		}
		
		////////////////////////////////////////////
		// zero count
		DmaCh [ 6 ].BCR.Value = 0;
		
		////////////////////////////////////////////////
		// MADR does not change on one-shot transfer
		DmaCh [ 6 ].MADR = Temp;
				
#ifdef INLINE_DEBUG_RUN_DMA6
	debug << "; FINISHED";
#endif

		// dma transfer is done (one-shot)
		DMA_Finished ( 6 );
	}
}


#ifdef PS2_COMPILE


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
	if ( !DmaCh [ iChannel ].CHCR.TR )
	{
		return;
	}
	
	// check that transfer is from memory
	if ( !DmaCh [ iChannel ].CHCR.DR )
	{
		cout << "\nhps2x64: SPU2: ALERT: AutoDma transfer from device to memory!";
		return;
	}
	
	// check that there is data to transfer
	if ( DmaCh [ iChannel ].BCR.BA && DmaCh [ iChannel ].BCR.BS )
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
	
	// doesn't look like the dma sees an interrupt unless it is complete
	// and then by the time it is complete, there should be enough time to restart AutoDMA
	/*
	else
	{
#ifdef INLINE_DEBUG_AUTODMA
	debug << "; BLOCKINT";
#endif

		// interrupt unconditionally for now
		DMA_Finished ( iChannel, false, false, true );
		//SPU2::SetInterrupt ();
		
		// use just a dma interrupt
		//SetInterrupt ();
	}
	*/
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


#endif



void Dma::DMA4_Run ( bool ContinueToCompletion )
{
		// DMA 4 is running
#ifdef INLINE_DEBUG_RUN_DMA4
	debug << "\r\n";
	debug << "; DMA4: SPU";
#ifdef PS2_COMPILE
	debug << " SPU2 NextEvent=" << dec << SPU2::_SPU2->NextEvent_Cycle;
#endif
#endif
		
	u32 Temp;
	u32 Data [ 4 ];
	u32 NumberOfTransfers;
	
	u64 CyclesPerTransfer, BusCyclesPerTransfer;
	u64 BusCycles;
	
	
	/*
	if ( !_BUS->isReady () )
	{
#ifdef INLINE_DEBUG_RUN_DMA4
	debug << "; BUS_BUSY";
#endif

		// bus is not ready //
		// reschedule transfer for when bus is ready
		SetNextEventCh_Cycle ( _BUS->BusyUntil_Cycle, 4 );
	}
	else
	*/
	
	{
#ifdef INLINE_DEBUG_RUN_DMA4
	debug << "; BUS_FREE";
#endif
		// bus is free //

		// check that SPU is ready for DMA transfer
		if ( ( _SPU->DMA_ReadyForRead () && !DmaCh [ 4 ].CHCR.DR ) || ( _SPU->DMA_ReadyForWrite () && DmaCh [ 4 ].CHCR.DR ) )
		{
#ifdef INLINE_DEBUG_RUN_DMA4
	debug << "; SPU_READY";
#endif
				
			/////////////////////////////////////////////////////
			// Check if we are transferring to or from memory
			if ( DmaCh [ 4 ].CHCR.DR )
			{
				/////////////////////////////////////////////////////
				// Transferring FROM memory
#ifdef INLINE_DEBUG_RUN_DMA4
					debug << ";FROM";
					debug << "; BA=" << hex << DmaCh [ 4 ].BCR.BA << "; BS=" << DmaCh [ 4 ].BCR.BS;
#endif

				// only finished when BA is zero!
				if ( !DmaCh [ 4 ].BCR.BA )
				{
#ifdef INLINE_DEBUG_RUN_DMA4
	debug << "; FINISHED";
#endif

					// dma transfer is done //
					DMA_Finished ( 4 );

#ifdef PS2_COMPILE
					// let SPU know dma transfer is done also
					SPU2::_SPU2->SPU0.SpuTransfer_Complete ();
#endif

#ifdef PS2_COMPILE
					// ***testing***
					// interrupt spu??
					//SPUCore::SetInterrupt ();
#endif

					return;
				}
				
#ifdef INLINE_DEBUG_RUN_DMA4
		debug << ";NOTDONE";
#endif

				// *** todo: SPU bus might be 2 bytes wide, so time to transfer is times 2 *** //

#ifdef PS2_COMPILE

#ifdef ENABLE_AUTODMA
				// check if ADMA transfer
				if ( SPU2::_SPU2->SPU0.isADMATransferMode () )
				{
#ifdef INLINE_DEBUG_RUN_AUTODMA
					debug << "\r\n***AUTODMA4 STARTED***\r\n";
#endif

					// do not transfer here, because will need separate function to "auto-transfer"
					// will be invoked from SPU side since SPU knows when transfer is needed
					
					// disable cpu freezing (which needs to be fixed)
					ContinueToCompletion = false;

					if ( !SPU2::_SPU2->SPU0.DMA_ReadyForWrite () )
					{
#ifdef VERBOSE_AUTODMA_START
					cout << "\nhps1x64: SPU2: AutoDma4 Transfer Starting!\n";
#endif

						return;
					}
				
					/*
					// check SPU is decoding in the other buffer
					// if SPU decoding in the other buffer, transfer all left/right samples for half buffer for now
					// otherwise, set next transfer event for 256 samples * 768 cycles/sample, if unable to transfer now
					// if was able to transfer, then interrupt
					*/
				}
#endif

#endif

#ifdef INLINE_DEBUG_RUN_DMA4
	debug << "; NOTAUTO";
#endif

				// ***TODO*** this needs to be implemented properly //
				if ( ( *_DebugPC & 0xffc00000 ) == 0xbfc00000 )
				{
					// pc is reading from bios //
					// if pc is reading from un-cached bios
					// then the reads might be getting cancelled out ??
					// since they take so long ??
					if ( c_SPU_CycleTime < DataBus::c_iBIOS_Read_Latency )
					{
						// not enough time for a successful read from bios ??
						ContinueToCompletion = true;
					}
				}


				// update busy cycles
				//NumberOfTransfers = DmaCh [ 4 ].BCR.BS * DmaCh [ 4 ].BCR.BA;
				//_BUS->ReserveBus ( NumberOfTransfers );
				NumberOfTransfers = DmaCh [ 4 ].BCR.BS;
				//_BUS->ReserveBus ( NumberOfTransfers * 2 );
				BusyCycles = NumberOfTransfers + 2;
				
				//SetNextEventCh ( BusyCycles + c_SPU_CycleTime, 4 );
				
				CyclesPerTransfer = ( DmaCh [ 4 ].BCR.BS << 1 ) + c_SPU_CycleTime;
				BusCyclesPerTransfer = ( DmaCh [ 4 ].BCR.BS << 1 );
				
				BusCycles = 0;
				BusyCycles = 0;
				
				if ( DmaCh [ 4 ].BCR.BS != 0x10 )
				{
					cout << "\nhps1x64 ERROR: *** ALERT *** DmaCh[ 4 ].BCR.BS=" << DmaCh [ 4 ].BCR.BS;
				}

#ifdef INLINE_DEBUG_RUN_DMA4
	debug << "; NumberOfTransfers=" << dec << NumberOfTransfers;
#endif

				if ( NumberOfTransfers )
				{
				
					do
					{
#ifdef PS2_COMPILE
						// on the PS2, SPU0 is part of the SPU2 device
						SPU2::_SPU2->SPU0.DMA_Write_Block ( & ( _BUS->MainMemory.b32 [ ( DmaCh [ 4 ].MADR & 0x1fffff ) >> 2 ] ), DmaCh [ 4 ].BCR.BS );
#else
						_SPU->DMA_Write_Block ( & ( _BUS->MainMemory.b32 [ ( DmaCh [ 4 ].MADR & 0x1fffff ) >> 2 ] ), DmaCh [ 4 ].BCR.BS );
#endif
						
						// ***todo*** this should be BS*4, but need to test more
						DmaCh [ 4 ].MADR += 64;
						
						// restore size
						//DmaCh [ 4 ].BCR.Value = TransferSize_Save [ 4 ];
						
						//////////////////////////////////////////////////////////
						// Decrease Block Count by 1
						DmaCh [ 4 ].BCR.BA--;
						
						// save size
						//TransferSize_Save [ 4 ] = DmaCh [ 4 ].BCR.Value;
						
						// update cycles that dma is busy
						BusCycles += BusCyclesPerTransfer;
						BusyCycles += CyclesPerTransfer;
					}
					while ( ContinueToCompletion && DmaCh [ 4 ].BCR.BA );
				}
				
				// clear transfer amount remaining
				// note: don't clear transfer amount remaining since block count should be zero but block size should not change
				// It actually looks like at the end of transfer that BCR is zero for sync mode 1
				// but Martin Korth's psx specifications says only BA is zero after the transfer
				//DmaCh [ 4 ].BCR.Value = 0;
						
						
				if ( ContinueToCompletion )
				{
#ifdef INLINE_DEBUG_RUN_DMA4
	debug << "; ContinueToCompletion";
#endif

					// transfer froze the CPU //
					
					// reserve the bus for the time DMA was busy
					// must be enough to freeze CPU
					_BUS->ReserveBus ( BusyCycles );
					
					// mark dma channel as finished
					DMA_Finished ( 4 );
					
#ifdef PS2_COMPILE
					// ***testing***
					// interrupt spu??
					//SPUCore::SetInterrupt ();
#endif
					
					// done
					return;
				}
				else
				{
#ifdef INLINE_DEBUG_RUN_DMA4
	debug << "; ContinueLater";
#endif

					// transfer did not freeze CPU //
					
					// reserve the bus for the time it was in use
					_BUS->ReserveBus ( BusCycles );
					
					SetNextEventCh ( BusyCycles, 4 );
				}
			}
			else
			{
				/////////////////////////////////////////////////////
				// Transferring TO memory
#ifdef INLINE_DEBUG_RUN_DMA4
					debug << ";TO";
#endif

				// *** todo: transfer all data at once *** //
				
				// update busy cycles
				NumberOfTransfers = DmaCh [ 4 ].BCR.BS * DmaCh [ 4 ].BCR.BA;
				_BUS->ReserveBus ( NumberOfTransfers );
				BusyCycles = NumberOfTransfers + 2;
				//SetNextEventCh ( BusyCycles, 4 );

				if ( NumberOfTransfers )
				{
					while ( DmaCh [ 4 ].BCR.BA )
					{
						while ( DmaCh [ 4 ].BCR.BS )
						{
							DmaCh [ 4 ].BCR.BS--;
							
							_SPU->DMA_Read ( & (Data [ 0 ]), 1 );
							_BUS->MainMemory.b32 [ ( DmaCh [ 4 ].MADR & 0x1fffff ) >> 2 ] = Data [ 0 ];
							
							///////////////////////////////////////////////////////////
							// Update memory address
							DmaCh [ 4 ].MADR += 4;
						}
						
						// restore size
						DmaCh [ 4 ].BCR.Value = TransferSize_Save [ 4 ];
						
						//////////////////////////////////////////////////////////
						// Decrease Block Count by 1
						DmaCh [ 4 ].BCR.BA--;
						
						// save size
						TransferSize_Save [ 4 ] = DmaCh [ 4 ].BCR.Value;
					}
				}
				
				// clear transfer amount remaining
				// It looks like at the end of transfer that BCR is zero for sync mode 1
				// but Martin Korth's psx specifications says only BA is zero after the transfer
				//DmaCh [ 4 ].BCR.Value = 0;

#ifdef INLINE_DEBUG_RUN_DMA4
	debug << "; FINISHED";
#endif

				// dma transfer is done //
				DMA_Finished ( 4 );
			}
			
		}
	
	}
	
}


void Dma::DMA3_Run ( bool ContinueToCompletion )
{
		// DMA 3 is running
//#ifdef INLINE_DEBUG_RUN_DMA3
//	debug << "; DMA3: CD";
//#endif
#ifdef INLINE_DEBUG_RUN_DMA3
	debug << "\r\nStarting DMA#3 (CD)";
	debug << " CycleCount=" << dec << *_DebugCycleCount;
	debug << hex << " PC=" << *_DebugPC;
	debug << " CHCR=" << _DMA->DmaCh [ 3 ].CHCR.Value;
	debug << " MADR=" << _DMA->DmaCh [ 3 ].MADR;
	debug << " BCR=" << _DMA->DmaCh [ 3 ].BCR.Value;
#endif

	// do entire dma3 transfer at once
	// one-shot transfer

	u32 Temp;
	u32 Data [ 4 ];
	u32 NumberOfTransfers;

	// check if bus is busy //
	/*
	if ( !_BUS->isReady () )
	{
		// reschedule dma for when bus is free
		SetNextEventCh_Cycle ( _BUS->BusyUntil_Cycle, 3 );
	}
	else
	*/
	
	{

		// do entire dma3 transfer at once
		// check that CD is ready for DMA transfer
		if ( _CD->DMA_ReadyForRead () )
		{
			// I'll use it bitmap later to determine when a transfer is just starting
			_CD->DMA_Start ();
			
			// schedule event to terminate dma transfer
			//SetNextEventCh ( BusyCycles, 3 );
			
			// save start address
			Temp = DmaCh [ 3 ].MADR;
			
			// save transfer amount
			TransferSize_Save [ 3 ] = DmaCh [ 3 ].BCR.Value;
			
			// get block size
			NumberOfTransfers = DmaCh [ 3 ].BCR.BS;
			
			// check if a block size of zero was specified
			if ( !NumberOfTransfers )
			{
				// check if a block amount of 1 is specified
				if ( DmaCh [ 3 ].BCR.BA )
				{
					// transfer a block of data
					NumberOfTransfers = 0x200;
				}
			}

#ifdef PS2_COMPILE
	// alert if PS2 and BA is zero before transfer for now
	if ( !DmaCh [ 3 ].BCR.BA )
	{
		cout << "\nhp1x64: ***ALERT***: CD: BA is zero before transfer.\n";
	}
#endif

			
			// should return the amount transfered in 32-bit words
			NumberOfTransfers = _CD->DMA_ReadBlock ( & _BUS->MainMemory.b32 [ ( DmaCh [ 3 ].MADR & 0x1fffff ) >> 2 ], DmaCh [ 3 ].BCR.BS, DmaCh [ 3 ].BCR.BA );
			

			// check if nothing was transferred
			if ( !NumberOfTransfers )
			{
				// done ??
				return;
			}
			

			// invalidate addresses in cpu i-cache ???
			//for ( int i = 0; i < NumberOfTransfers; i++ ) _CPU->InvalidateCache ( ( DmaCh [ 3 ].MADR + ( i << 2 ) ) & 0x1fffff );
			
			// update BusyCycles
			// note: this is on a 16-bit bus, so the 32-bit transfers are actually split into 16-bit transfers, meaning twice as many transfers
			// actually, according to martin korth's psx spec, this is an 8-bit bus
			//_BUS->ReserveBus ( NumberOfTransfers << 1 );
			//BusyCycles = ( NumberOfTransfers << 1 ) + 2;
			_BUS->ReserveBus ( NumberOfTransfers << 2 );
			BusyCycles = ( NumberOfTransfers << 2 ) + 2;
			
			// no more data to transfer
			//DmaCh [ 3 ].BCR.Value = 0;
			
			// restore transfer amount, as BCR should not change at all
			//DmaCh [ 3 ].BCR.Value = TransferSize_Save [ 3 ];
			
			// MADR should not change on one-shot transfer
			//DmaCh [ 3 ].MADR = Temp;
			
			


			

				// need to call this when dma transfer is done
				_CD->DMA_End ();
				
				/////////////////////////////////////
				// we are done
				DMA_Finished ( 3 );
				

		} // end if dma ready for read
		else
		{
		
#ifdef PS2_COMPILE
			if ( !CDVD::_CDVD->AvailableSectorCount )
			{
#endif
		
#ifdef INLINE_DEBUG_RUN_DMA3
	debug << "\r\nDMA#3 Not Ready For Reading\r\n";
#endif

#ifdef PS2_COMPILE
			}
#endif


#ifdef PS2_COMPILE

			// make sure DMA3 is active
			// ***todo*** actually, this is wrong. Needs to be enabled AND active
			if ( !isActive( 3 ) ) return;

			while ( CDVD::_CDVD->AvailableSectorCount && isEnabledAndActive ( 3 ) )
			{
				// I'll use it bitmap later to determine when a transfer is just starting
				_CD->DMA_Start ();

				// read from cd
				CD::_CD->cd_image.ReadNextSector ();
				
				CD::_CD->SectorDataBuffer_Index = CD::_CD->cd_image.GetCurrentBufferIndex ();
				
				// currently need to set this
				// no, no... dma transfer is actually not ready until 0x80 is written to request data
				//CD::_CD->isSectorDataReady = true;
				
				// this stuff needs to be set again ?? (fix later)
				// set the sector size for ps1 cd handler
				// need to set the data buffer size again since the ps1 handler resets it for now
				CD::_CD->DataBuffer_Size = CDVD::_CDVD->SectorReadSize;
				//CD::_CD->ReadMode = _CDVD->SectorReadSize;
			
			
				// set the sector number that is being read
				CD::_CD->DVDSectorNumber = CDVD::_CDVD->SeekSectorNum;
				
#ifdef INLINE_DEBUG_RUN_DMA3
	debug << "\r\nDMA3 Transfer to " << hex << DmaCh [ 3 ].MADR << " where (before) BS=" << DmaCh [ 3 ].BCR.BS << " BA=" << DmaCh [ 3 ].BCR.BA;
#endif
			
				// should return the amount transfered in 32-bit words
				NumberOfTransfers = _CD->DMA_ReadBlock ( & _BUS->MainMemory.b32 [ ( DmaCh [ 3 ].MADR & 0x1fffff ) >> 2 ], DmaCh [ 3 ].BCR.BS, DmaCh [ 3 ].BCR.BA );
			
			
#ifdef INLINE_DEBUG_RUN_DMA3
	debug << " Transferred=" << dec << NumberOfTransfers << " 32-bit words";
	debug << "\r\n";
#endif

				// check if nothing was transferred
				if ( !NumberOfTransfers )
				{
					// done ??
					return;
				}
				
				// must increase the sector number here so correct data gets put in when reading
				// advance sector number
				CDVD::_CDVD->SeekSectorNum++;
				
				// decrease the number of available sectors
				CDVD::_CDVD->AvailableSectorCount--;
				

				// update BusyCycles
				// note: this is on a 16-bit bus, so the 32-bit transfers are actually split into 16-bit transfers, meaning twice as many transfers
				// actually, according to martin korth's psx spec, this is an 8-bit bus
				_BUS->ReserveBus ( NumberOfTransfers << 2 );
				BusyCycles = ( NumberOfTransfers << 2 ) + 2;

				u32 BAUpdate;
				if ( DmaCh [ 3 ].BCR.BS )
				{
					if ( _CD->ReadMode == 1024 )
					{
						// read toc command //
						BAUpdate = DmaCh [ 3 ].BCR.BA;
						
						// can set the read mode to 2064 at this point
						_CD->ReadMode = 2064;
					}
					else
					{
						// command to read disk sector //
						BAUpdate = ( _CD->ReadMode / DmaCh [ 3 ].BCR.BS ) >> 2;
					}
				}
				else
				{
					// for now.. and this should work for PS1 also...
					BAUpdate = ( _CD->ReadMode / 0x200 ) >> 2;
				}
				
				// update BA
				// ***todo*** this is just a placeholder for now
				if ( DmaCh [ 3 ].BCR.BA >= BAUpdate )
				{
					DmaCh [ 3 ].BCR.BA -= BAUpdate;
					
					// also update MADR ??
					DmaCh [ 3 ].MADR += _CD->ReadMode;
				}

#ifdef INLINE_DEBUG_RUN_DMA3
	debug << "(after) BA=" << DmaCh [ 3 ].BCR.BA;
	debug << "\r\n";
#endif

			
				// check if transfer is done yet
				if ( DmaCh [ 3 ].BCR.BA < BAUpdate )
				{

					// need to call this when dma transfer is done
					_CD->DMA_End ();
					
					/////////////////////////////////////
					// we are done
					DMA_Finished ( 3 );
				
#ifdef INLINE_DEBUG_RUN_DMA3
	debug << "\r\n";
	debug << "DMA#3 Transfer is complete";
	debug << "\r\n";
#endif

				}

				
			} // end while
				
#endif

		}	

	}
}



void Dma::DMA2_Run ( bool ContinueToCompletion )
{

	u32 Previous_LL_Address = 0;

#ifdef INLINE_DEBUG_RUN_DMA2_CO
	if ( !DmaCh [ 2 ].CHCR.LI )
	{
		debug << "\r\nDMA2; CycleCount=" << dec << *_DebugCycleCount << " NextEventCh_Cycle=" << NextEventCh_Cycle [ 2 ];
	}
#endif

#ifdef INLINE_DEBUG_RUN_DMA2
			// check if dma channel is active but cycle count is less than system cycle count
			if ( ( ChannelEnable_Bitmap & ( 1 << 2 ) ) && ( NextEventCh_Cycle [ 2 ] < *_DebugCycleCount ) )
			{
				debug << "\r\nDMA2; LESS_THAN_CYCLE";
			}
#endif

	u32 Temp;
	u32 Data [ 4 ];
	u32 NumberOfTransfers;
	
	u32 Offset;
	u32 Count;
	u32 BusCycles = 0;
	
	/////////////////////////////////////////////
	// Check if GPU is ready to receive data
	/*
	if ( *_DebugCycleCount < _GPU->BusyUntil_Cycle )
	{
#ifdef INLINE_DEBUG_RUN_DMA2
					debug << "\r\n;GPU_BUSY; CycleCount=" << dec << *_DebugCycleCount << "; GPU_BusyUntilCycle=" << _GPU->BusyUntil_Cycle;
#endif

		// gpu not ready to receive data //
		
		// reschedule dma for when gpu is ready
		SetNextEventCh_Cycle ( _GPU->BusyUntil_Cycle, 2 );
		return;
	}
	else
	*/
	
	if ( !DmaCh [ 2 ].CHCR.TR )
	{
		// !!! IMPORTANT !!! //
		// dma2 can be stopped at any time in the middle of a transfer!!! //
		// *** todo *** fix dma operation
		
		DMA_Finished ( 2 );
		return;
	}
	
	// if GPU is busy, then reschedule transfer
	
	{
#ifdef INLINE_DEBUG_RUN_DMA2_CO
		if ( !DmaCh [ 2 ].CHCR.LI )
		{
			debug << ";GPU_READY";
		}
#endif

		// gpu is ready to receive data //
	
		////////////////////////////////////////////
		// Check if this is a linked list transfer
		if ( DmaCh [ 2 ].CHCR.LI )
		{
//#ifdef INLINE_DEBUG_RUN_DMA2
//				debug << ";LL;Starting Primitive;";
//#endif

			// linked-list transfer //
			
			// check if LL transfer is done
			if ( DmaCh [ 2 ].MADR & 0xffffff == 0x00ffffff )
			{
#ifdef INLINE_DEBUG_RUN_DMA2
	debug << "\r\nDMA2 is FINISHED";
#endif

				DMA_Finished ( 2 );
			}
			else
			{

//#ifdef INLINE_DEBUG_RUN_DMA2
//				debug << "\r\nDmaCh2.MADR=" << hex << DmaCh [ 2 ].MADR;
//#endif

				// check if bus is free
				/*
				if ( !_BUS->isReady () )
				{
#ifdef INLINE_DEBUG_RUN_DMA2
	debug << "\r\nDMA2-LL: BUS IS BUSY; Bus_BusyUntilCycle=" << dec << _BUS->BusyUntil_Cycle;
#endif

					// reschedule dma for when bus is free
					SetNextEventCh_Cycle ( _BUS->BusyUntil_Cycle, 2 );
				}
				else
				*/
				
				{
					BusyCycles = 0;
					
					BusCycles = 0;
					
					do
					{
					
						// clear the gpu busy cycles
						_GPU->BusyCycles = 0;
					
						//while ( !_GPU->BusyCycles && DmaCh [ 2 ].MADR != 0xffffff /*&& DmaCh [ 2 ].MADR != 0*/ )
						{
			
						_GPU->BusyCycles = 0;
						
						/*
						if ( LL_NextAddress == -1 )
						{
							///////////////////////////////////
							// Read word from memory
							Temp = _BUS->MainMemory.b32 [ ( DmaCh [ 2 ].MADR & 0x1fffff ) >> 2 ];
						}
						else
						{
							DmaCh [ 2 ].MADR = LL_NextAddress;
							
							if ( DmaCh [ 2 ].MADR == 0xffffff ) break;
							
							Temp = _BUS->MainMemory.b32 [ ( DmaCh [ 2 ].MADR & 0x1fffff ) >> 2 ];
						}
						*/
						
						DmaCh [ 2 ].MADR = LL_NextAddress;
						
						// check for termination of linked list
						if ( DmaCh [ 2 ].MADR == 0xffffff ) break;
						
						Temp = _BUS->MainMemory.b32 [ ( DmaCh [ 2 ].MADR & 0x1fffff ) >> 2 ];
						
						// get the next address
						LL_NextAddress = Temp & 0xffffff;
						
						// if next address is zero, then something went wrong probably
						//if ( !LL_NextAddress )
						if ( LL_NextAddress != 0xffffff && ( /* !LL_NextAddress || */ LL_NextAddress >= 0x800000 ) )
						{
							cout << "\nhps1x64 ALERT: Next DMA2 address error. Next_MADR=" << hex << LL_NextAddress << " MADR=" << DmaCh [ 2 ].MADR << " CycleCount=" << dec << *_DebugCycleCount;
						}
						
						// get the count
						LL_Count = ( Temp >> 24 );
						
						// update busy cycles - need to add 1 since we just read from memory using the bus
						//BusyCycles = LL_Count + 1;
						BusyCycles += LL_Count + 1;
						BusCycles += LL_Count + 1;
						
						// reserve bus only for the time it is in use
						//_BUS->ReserveBus ( BusyCycles );
						
						BusyCycles += c_LinkedListSetupTime;
						//BusCycles += c_LinkedListSetupTime;
						
						// get the next address
						//LL_NextAddress = ( Temp & 0x00ffffff );
						
						// *** testing *** alert on invalid DMA2 addresses
						if ( DmaCh [ 2 ].MADR != 0xffffff && ( /* !DmaCh [ 2 ].MADR || */ DmaCh [ 2 ].MADR >= 0x800000 ) )
						{
							//cout << "\nhps1x64 WARNING: Invalid DMA2 address: " << hex << LL_NextAddress << "\n";
							cout << "\nhps1x64 WARNING: Invalid DMA2 address: " << hex << DmaCh [ 2 ].MADR << "\n";
							DmaCh [ 2 ].MADR = 0xffffff;
							LL_Count = 0;
							LL_NextAddress = 0xffffff;
							break;
						}
						
						
						// *** testing *** alert on never ending DMA2
						/*
						if ( LL_NextAddress == Previous_LL_Address )
						{
							cout << "\nhps1x64 WARNING: Recursive DMA2 address: " << hex << LL_NextAddress << "\n";
							LL_Count = 0;
							LL_NextAddress = 0xffffff;
						}
						
						Previous_LL_Address = LL_NextAddress;
						*/
						
						// *** testing *** mask count?? two dma2's ???
						if ( LL_Count > 32 )
						{
							// !!! IMPORTANT !!! //
							// if dma2 linked-list transfer count is too large then the dma2 may stop??
							// *** todo *** investigate and implement proper fix
							cout << "\nhps1x64 WARNING: Transfer Count for Primitive too large. LL_Count=" << dec << LL_Count << " LL_NextAddress=" << hex << LL_NextAddress << "\n";
							//DmaCh [ 2 ].MADR = 0xffffff;
							LL_Count = 0;
							//LL_NextAddress = 0xffffff;
							//break;
						}

						
#ifdef INLINE_DEBUG_RUN_DMA2
				debug << "\r\nDMA2_WRITE; MADR=" << hex << DmaCh [ 2 ].MADR << " Count=" << dec << (Temp>>24) << " NextAddress=" << hex << (Temp & 0xffffff);
#endif


						// clear busy cycles for gpu
						_GPU->BusyCycles = 0;
							
							Offset = 0;
							
							Count = LL_Count;
							
							// transfer data to GPU
							while ( Count )
							{
							
								//////////////////////////////////////////////////////
								// go to next word of data
								//DmaCh [ 2 ].MADR += 4;
								Offset += 4;
							
#ifdef INLINE_DEBUG_WRITE_DMA2
				debug << "\r\nDMA2_WRITE; MADR=" << hex << ( DmaCh [ 2 ].MADR + Offset );
#endif

								///////////////////////////////////
								// Read word from memory
								Temp = _BUS->MainMemory.b32 [ ( ( DmaCh [ 2 ].MADR + Offset ) & 0x1fffff ) >> 2 ];
					
#ifdef INLINE_DEBUG_WRITE_DMA2
				debug << " Data=" << hex << Temp;
#endif

								//if ( Temp == 0x7b0063 )
								//{
								//	cout << "\nhps1x64 Testing: Temp=0x7b0063 and MADR+Offset=" << hex << ( DmaCh [ 2 ].MADR + Offset ) << " MADR=" << DmaCh [ 2 ].MADR << " NextAddress=" << LL_NextAddress << " Count=" << dec << LL_Count << "\n";
								//}

								/////////////////////////////////////////
								// Transfer data to GPU
								_GPU->DMA_Write ( &Temp, 1 );
						
//#ifdef INLINE_DEBUG_RUN_DMA2
//				debug << " LL_Count=" << dec << LL_Count;
//#endif

								Count--;
							}	// end while ll_count
						
#ifdef INLINE_DEBUG_RUN_DMA2
							//if ( _GPU->BusyCycles )
							//{
								debug << "\r\nDMA2-LL: Sent List. GPUBusyCycles=" << dec << _GPU->BusyCycles << "; CycleCount=" << *_DebugCycleCount;
							//}
#endif

							// *** testing *** set the next address
							// note: don't do this yet
							//DmaCh [ 2 ].MADR = LL_NextAddress;

						}	// end while ( !_GPU->BusyCycles && DmaCh [ 2 ].MADR != 0xffffff )

						//////////////////////////////////////////////////////
						// Next time we need to start from our new address
						DmaCh [ 2 ].MADR = LL_NextAddress;
				
						// if the cpu is frozen because it is writing to dma without enough priority, freeze out cpu completely
						// update busy cycles
						BusyCycles += _GPU->BusyCycles;
						
					} while ( DmaCh [ 2 ].MADR != 0xffffff /*&& DmaCh [ 2 ].MADR != 0*/ && ContinueToCompletion );
					
					if ( DmaCh [ 2 ].MADR == 0xffffff )
					{
						// dma is finished
						DMA_Finished ( 2 );
					}
					else
					{
						// add an event for when gpu is done processing command so we can put in the next one
						SetNextEventCh ( BusyCycles, 2 );
					}
					

					// add an event for when gpu is done processing command so we can put in the next one
					//SetNextEventCh ( BusyCycles, 2 );
					
#ifdef INLINE_DEBUG_RUN_DMA2
	debug << " BusyCycles=" << dec << BusyCycles << " NextEvent_Cycle=" << NextEvent_Cycle << " NextEventCh_Cycle [ 2 ]=" << NextEventCh_Cycle [ 2 ];
#endif

					if ( !ContinueToCompletion )
					{
						// just freeze bus for the amount of time it was used
						BusyCycles = BusCycles;
					}
					
					// reserve bus only for the time it is in use
					// completely freeze cpu because it wrote to dma without enough priority
					// don't subtract the final gpu cycles since it probably still reads end code
					_BUS->ReserveBus ( BusyCycles );
						
					// add event
					//SetNextEventCh ( BusyCycles, 2 );
					
					// reserve bus only for the time it is in use
					//_BUS->ReserveBus ( BusyCycles );
				
				}
			}
		}
		else
		{
#ifdef INLINE_DEBUG_RUN_DMA2_CO
					debug << ";NON-LL";
#endif

			// non-linked list gpu transfer //
			
			// check if transfer is complete
			if ( !DmaCh [ 2 ].BCR.BA )
			{
#ifdef INLINE_DEBUG_RUN_DMA2_CO
					debug << ";FINISHED";
#endif

				// dma transfer is done //
				DMA_Finished ( 2 );
			}
			else
			{
#ifdef INLINE_DEBUG_RUN_DMA2_CO
					debug << ";STARTING";
#endif

				// check if the bus is free
				/*
				if ( !_BUS->isReady () )
				{
					// reschedule dma for when bus is free
					SetNextEventCh_Cycle ( _BUS->BusyUntil_Cycle, 2 );
				}
				else
				*/
				
				{
					//////////////////////////////////////////////////////
					// assume this is a continuous transfer for now
					
					// *** testing *** transfer block to block //
					
					NumberOfTransfers = DmaCh [ 2 ].BCR.BS * DmaCh [ 2 ].BCR.BA;
					//NumberOfTransfers = DmaCh [ 2 ].BCR.BS /* * DmaCh [ 2 ].BCR.BA*/;
					
					_BUS->ReserveBus ( NumberOfTransfers );
					
					//BusyCycles = NumberOfTransfers + 2;
					BusyCycles = NumberOfTransfers + c_GPU_CycleTime;
					
					SetNextEventCh ( BusyCycles, 2 );
					
					// select this dma as the only one that can run until it finishes
					SelectedDMA_Bitmap = ( 1 << 2 );
					
					/////////////////////////////////////////////////////
					// Check if we are transferring to or from memory
					if ( DmaCh [ 2 ].CHCR.DR )
					{
						/////////////////////////////////////////////////////
						// Transferring FROM memory
#ifdef INLINE_DEBUG_RUN_DMA2_CO
					debug << ";FROM";
#endif

						// *** todo: transfer all data at once *** //
						
						// transfer all the data at once
						while ( DmaCh [ 2 ].BCR.BA )
						{
							/////////////////////////////////////////////////////////////
							// transfer just max of 1 word
							while ( DmaCh [ 2 ].BCR.BS )
							{
#ifdef INLINE_DEBUG_WRITE_DMA2
				debug << "\r\nDMA2_WRITE; MADR=" << hex << DmaCh [ 2 ].MADR;
#endif

								///////////////////////////////////
								// Read word from memory
								Temp = _BUS->MainMemory.b32 [ ( DmaCh [ 2 ].MADR & 0x1fffff ) >> 2 ];
									
#ifdef INLINE_DEBUG_WRITE_DMA2
				debug << " Data=" << hex << Temp;
#endif

								_GPU->DMA_Write ( &Temp, 1 );
								

								///////////////////////////////////////////////////////////
								// Update memory address on NON linked-list transfer
								DmaCh [ 2 ].MADR += 4;
								
								DmaCh [ 2 ].BCR.BS--;
							}
							
							// restore size
							DmaCh [ 2 ].BCR.Value = TransferSize_Save [ 2 ];
							
							//////////////////////////////////////////////////////////
							// Decrease Block Count by 1
							DmaCh [ 2 ].BCR.BA--;
							
							// save size
							TransferSize_Save [ 2 ] = DmaCh [ 2 ].BCR.Value;
						}
						
						// set remaining size to zero
						// note: only BCR.BA gets updated to zero
						// It actually looks like at the end of transfer BCR is zero
						//DmaCh [ 2 ].BCR.Value = 0;
					}
					else
					{
						/////////////////////////////////////////////////////
						// Transferring TO memory
#ifdef INLINE_DEBUG_RUN_DMA2
					debug << ";TO";
#endif

						// *** todo: transfer all data at once *** //

						while ( DmaCh [ 2 ].BCR.BA )
						{
							while ( DmaCh [ 2 ].BCR.BS )
							{
								_GPU->DMA_Read ( & (Data [ 0 ]), 1 );
								_BUS->MainMemory.b32 [ ( DmaCh [ 2 ].MADR & 0x1fffff ) >> 2 ] = Data [ 0 ];
								
								///////////////////////////////////////////////////////////
								// Update memory address on NON linked-list transfer
								DmaCh [ 2 ].MADR += 4;
								
								DmaCh [ 2 ].BCR.BS--;
							}
							
							// restore size
							DmaCh [ 2 ].BCR.Value = TransferSize_Save [ 2 ];
							
							//////////////////////////////////////////////////////////
							// Decrease Block Count by 1
							DmaCh [ 2 ].BCR.BA--;
							
							// save size
							TransferSize_Save [ 2 ] = DmaCh [ 2 ].BCR.Value;
						}
						
						// set remaining size to zero
						// note: only BCR.BA gets updated to zero
						// It actually looks like at the end of transfer that BCR is zero
						//DmaCh [ 2 ].BCR.Value = 0;
					}
				}
			}
		}
	}
}


void Dma::DMA1_Run ( bool ContinueToCompletion )
{
		// DMA 1 is running
#ifdef INLINE_DEBUG_RUN_DMA1
	debug << "; DMA1_Run; DMA1: MDECout";
#endif

	u32 Temp;
	u32 Data [ 4 ];
	u32 NumberOfTransfers;
	
	u32 NumberOfBlocksRead = 0, TotalNumBlocksRead = 0;
	
	u64 BusCycles;
	
	//u32 DMA0Start;
	
	// check if bus is free //
	/*
	if ( !_BUS->isReady () )
	{
		// bus is not free //
		
		// reschedule transfer for when bus is free
		SetNextEventCh_Cycle ( _BUS->BusyUntil_Cycle, 1 );
	}
	else
	*/
	
	{
		// bus is free //
		
		// check if mdec is busy
		if ( *_DebugCycleCount < MDEC::_MDEC->mess_mdec.n_busyuntil_cycle )
		{
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_RUN_DMA1
	debug << "; MDECBUSY";
#endif

			// can't transfer anything when mdec is busy processing data
			SetNextEventCh_Cycle ( MDEC::_MDEC->mess_mdec.n_busyuntil_cycle, 1 );
			
			// stop here
			return;
		}
		
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_RUN_DMA1
	debug << "; MDECNOTBUSY";
	debug << " DmaCh [ 1 ].BCR.BA=" << hex << DmaCh [ 1 ].BCR.BA << " DmaCh [ 1 ].BCR.BS=" << DmaCh [ 1 ].BCR.BS;
#endif

		// check if dma1 is ready
		if ( _MDEC->DMA_ReadyForRead () )
		{
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_RUN_DMA1
	debug << "; ReadyForRead";
#endif
		
			// check if we are done
			if ( !DmaCh [ 1 ].BCR.BA )
			{
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_RUN_DMA1
	debug << "; DMA1TransferComplete";
#endif
				// no more data to transfer
				//DmaCh [ 1 ].BCR.Value = 0;
				
				// need to call this when dma transfer is done
				//_MDEC->DMA0_End ();
				
				// dma1 is finished
				DMA_Finished ( 1 );
				
				// return
				return;
			}
			
			//DMA0Start = DmaCh [ 0 ].BCR.BA;
			
			BusCycles = 0;
			BusyCycles = 0;
			
			
			// only transfer BCR.BS cycles
			NumberOfTransfers = DmaCh [ 1 ].BCR.BA * DmaCh [ 1 ].BCR.BS;
			//NumberOfTransfers = DmaCh [ 1 ].BCR.BS;
			
			do
			{
			
			// *** TODO *** MDEC DMA probably does not actually work like this
			NumberOfBlocksRead = _MDEC->DMA_Read ( DmaCh [ 1 ].MADR & 0x1fffff, NumberOfTransfers, _BUS->MainMemory.b32 );
			
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_RUN_DMA1
	debug << "; NumberOfBlocksRead=" << dec << NumberOfBlocksRead;
#endif

			if ( NumberOfBlocksRead )
			{
				// update BusyCycles - only transfer BCR.BS cycles
				//_BUS->ReserveBus ( NumberOfTransfers );
				//BusyCycles = NumberOfTransfers + 2;
				
				//BusyCycles = NumberOfTransfers + c_MDEC_CycleTime;
				
				//SetNextEventCh ( BusyCycles, 1 );
				
				BusCycles += DmaCh [ 1 ].BCR.BS * NumberOfBlocksRead;
				BusyCycles += /*DmaCh [ 1 ].BCR.BS +*/ MDEC::_MDEC->mess_mdec.n_cycles_used;
				
					
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_RUN_DMA1
	debug << hex << setw ( 8 ) << " " << _BUS->MainMemory.b32 [ ( DmaCh [ 1 ].MADR & 0x1fffff ) >> 2 ];
#endif
					
				// update MADR
				//DmaCh [ 1 ].MADR += ( NumberOfTransfers << 2 );
				DmaCh [ 1 ].MADR += ( DmaCh [ 1 ].BCR.BS * NumberOfBlocksRead * 4 );
				
				// dec BCR.BA
				//DmaCh [ 1 ].BCR.BA--;
				
				// no more data to transfer
				//DmaCh [ 1 ].BCR.Value = 0;
				//DmaCh [ 1 ].BCR.BA = 0;
				//DmaCh [ 1 ].BCR.BA--;
				
				// PROBLEM - can possibly output more data than requested
				if ( NumberOfBlocksRead > DmaCh [ 1 ].BCR.BA )
				{
					cout << "\nhps1x64: DMA1 (MDECout): Read more data than requested.\n";
					DmaCh [ 1 ].BCR.BA = 0;
				}
				else
				{
					DmaCh [ 1 ].BCR.BA -= NumberOfBlocksRead;
				}
				
				TotalNumBlocksRead += NumberOfBlocksRead;
				
				
				//SetNextEventCh_Cycle ( MDEC::_MDEC->mess_mdec.n_busyuntil_cycle, 1 );
				
				//_BUS->ReserveBus ( DmaCh [ 1 ].BCR.BS * NumberOfBlocksRead );
				
				//cout << "\nTEST: mdec busyuntil cycle=" << dec << MDEC::_MDEC->mess_mdec.n_busyuntil_cycle;
				//cout << "\nTEST: current cycle=" << dec << *_DebugCycleCount;
			}
			
			} while ( NumberOfBlocksRead && ContinueToCompletion );
			
			_BUS->ReserveBus ( DmaCh [ 1 ].BCR.BS * TotalNumBlocksRead );
			SetNextEventCh ( BusyCycles, 1 );
				
			// check if that completes dma0
			// check if dma transfer is done
			if ( !DmaCh [ 0 ].BCR.BA && isEnabledAndActive ( 0 ) )
			{
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_RUN_DMA1
	debug << "; DMA0TransferComplete";
#endif
				// no more data to transfer
				// note: only BA goes to zero probably
				//DmaCh [ 0 ].BCR.Value = 0;
					
				// need to call this when dma transfer is done
				//_MDEC->DMA0_End ();
				
				/////////////////////////////////////
				// we are done
				DMA_Finished ( 0, true );
			}
		}
	}
}



void Dma::DMA0_Run ( bool ContinueToCompletion )
{
		// DMA 0 is running
#ifdef INLINE_DEBUG_RUN_DMA0
	debug << "; DMA0_Run; DMA0: MDECin";
	debug << "; DmaCh [ 0 ].BCR.BA=" << hex << DmaCh [ 0 ].BCR.BA;
#endif

	u32 Temp;
	u32 Data [ 4 ];
	u32 NumberOfTransfers;
	
	// check if bus is free
	/*
	if ( !_BUS->isReady () )
	{
		// bus is not free //
		
		// reschedule transfer for when bus is free
		SetNextEventCh_Cycle ( _BUS->BusyUntil_Cycle, 0 );
	}
	else
	*/
	
	
	{
		// bus is free //

		// only transfer BCR.BS cycles
		//NumberOfTransfers = ( DmaCh [ 0 ].BCR.BA + 1 ) * DmaCh [ 0 ].BCR.BS;
		NumberOfTransfers = DmaCh [ 0 ].BCR.BA * DmaCh [ 0 ].BCR.BS;
		
		/*
		// update BusyCycles - only transfer BCR.BS cycles
		// transferring all the data at the start of frame does not appear to be a problem yet. Will fix later
		_BUS->ReserveBus ( NumberOfTransfers );
		
		BusyCycles = NumberOfTransfers + 2;
		//SetNextEventCh ( BusyCycles, 0 );
		*/
		
		
		// *** TODO *** since this is probably a continuous transfer will need to load all the data at once
		// *note* not a continuous transfer. only transfers data when MDEC requests
		if ( _MDEC->DMA_ReadyForWrite () )
		{
#ifdef INLINE_DEBUG_RUN_WRITE
	debug << "; DMA0ReadyForWrite";
	debug << "; DMA0_BCR=" << hex << DmaCh [ 0 ].BCR.Value;
#endif

			_MDEC->DMA_Write ( DmaCh [ 0 ].MADR, NumberOfTransfers, _BUS->MainMemory.b32 );
			
			DmaCh [ 0 ].BCR.BA--;
			
			_BUS->ReserveBus ( NumberOfTransfers );
			
			// check if dma transfer is done
			if ( !DmaCh [ 0 ].BCR.BA )
			{
				DmaCh [ 0 ].MADR += ( DmaCh [ 0 ].BCR.BS << 2 );
				DmaCh [ 0 ].BCR.BA = 0;
				DMA_Finished ( 0 );
			}
		}
		else
		{
#ifdef INLINE_DEBUG_WRITE
	debug << "; DMA0NOTReadyForWrite";
	debug << "; DMA0_BCR=" << hex << DmaCh [ 0 ].BCR.Value;
#endif

			_MDEC->DMA_Write ( DmaCh [ 0 ].MADR, NumberOfTransfers, _BUS->MainMemory.b32 );
			
			if ( isEnabledAndActive ( 1 ) )
			{
#ifdef INLINE_DEBUG_WRITE
	debug << "; DMA0RUNNINGDMA1";
#endif
				// run dma 1 channel if it is enabled and active
				DMA1_Run ( false );
			}
		}
		
		/*
		// update MADR
		DmaCh [ 0 ].MADR += ( ( DmaCh [ 0 ].BCR.BA * DmaCh [ 0 ].BCR.BS ) << 2 );
		
		// no more data to transfer
		DmaCh [ 0 ].BCR.Value = 0;
			
		// need to call this when dma transfer is done
		//_MDEC->DMA0_End ();
		
		/////////////////////////////////////
		// we are done
		DMA_Finished ( 0 );
		*/
	
	}
}


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
		
		// check if dma transfer is done
		/*
		if ( !DmaCh [ 0 ].BCR.BA )
		{
			// no more data to transfer
			// note: only BA goes to zero probably
			//DmaCh [ 0 ].BCR.Value = 0;
				
			// need to call this when dma transfer is done
			//_MDEC->DMA0_End ();
			
			/////////////////////////////////////
			// we are done
			DMA_Finished ( 0 );
		}
		*/
	}
	
#ifdef INLINE_DEBUG_RUN_DMA0
	debug << "; DataOut=" << hex << (u64) DataOut;
#endif

	return DataOut;
}




#ifdef PS2_COMPILE


void Dma::DMA12_Run ()
{
#ifdef INLINE_DEBUG_RUN_DMA12
	debug << "; DMA12_Run; DMA12: SIO2out";
	debug << "; BA=" << hex << DmaCh [ 12 ].BCR.BA << "; BS=" << DmaCh [ 12 ].BCR.BS;
#endif

	u32 Temp;
	u32 NumberOfTransfers;
	u8 *Data;
	u8 *BufPtr;
	
	// make sure SIO output is ready
	// *** testing *** transfer whether device is ready or not
	// looks like the SIO2 does NOT wait for "bd" or "3bd" to be written before the DMA READ operation executes, so this MUST be removed or stay commented out
	/*
	if( !Playstation1::SIO::_SIO->SIO2out_DMA_Ready () )
	{
#ifdef INLINE_DEBUG_RUN_DMA12
	debug << "; SIO2out device not ready";
#endif

		// SIO2out not ready for dma transfer
		return;
	}
	
#ifdef INLINE_DEBUG_RUN_DMA12
	debug << "; SIO2out device READY";
#endif
	*/

	// make sure dma channel is ok to run
	if ( !isEnabledAndActive ( 12 ) )
	{
#ifdef INLINE_DEBUG_RUN_DMA12
	debug << "; DMA#12 not enabled or active";
#endif

		return;
	}

#ifdef INLINE_DEBUG_RUN_DMA12
	debug << "; DMA#12 ENABLED AND ACTIVE";
#endif

	// bus is free //

	// only transfer BCR.BS cycles
	//NumberOfTransfers = ( DmaCh [ 0 ].BCR.BA + 1 ) * DmaCh [ 0 ].BCR.BS;
	//NumberOfTransfers = DmaCh [ 0 ].BCR.BA * DmaCh [ 0 ].BCR.BS;
	
	// *** todo *** don't forget to reserve bus on IOP side too
	
	// number of transfers is not known yet
	NumberOfTransfers = DmaCh [ 12 ].BCR.BS * DmaCh [ 12 ].BCR.BA;
	
	// bus should be busy during this time
	_BUS->ReserveBus ( NumberOfTransfers );

	// read from cache
	//BufPtr = & ( SIO2_Buffer [ 0 ] );
	
	while ( DmaCh [ 12 ].BCR.BA )
	{
		// ** todo ** initiate transfer
		//Playstation1::SIO::_SIO->StartNewCommand ();
		
		// get pointer to data destination
		Data = (u8*) ( & ( _BUS->MainMemory.b32 [ ( DmaCh [ 12 ].MADR & 0x1fffff ) >> 2 ] ) );
		
		// transfer the data to SIO
		// *** todo *** should number of transfers be in bytes or words??
		Playstation1::SIO::_SIO->DMA_ReadBlock ( (u8*) Data, DmaCh [ 12 ].BCR.BS << 2 );
		
#ifdef INLINE_DEBUG_RUN_DMA12_OUTPUT
	debug << "\r\nDMA12OutputFromSIO=" << hex << setw ( 2 );
	//u8 *Data8 = (u8*) BufPtr;
	//for ( int i = 0; i < ( DmaCh [ 12 ].BCR.BS << 2 ); i++ )
	//{
	//	debug << " " << (u32) *Data8++;
	//}
#endif

		//for ( int i = 0; i < ( DmaCh [ 12 ].BCR.BS << 2 ); i++ )
		//{
		//	*Data++ = *BufPtr++;
		//}

		// dec BA
		DmaCh [ 12 ].BCR.BA--;
		
		if ( DmaCh [ 12 ].BCR.BA )
		{
			// update MADR ??
			DmaCh [ 12 ].MADR += (u32) DmaCh [ 12 ].BCR.BS << 2;
		}
	}
	
	// check if transfer should end
	// must allow dma restart for now
	//DMA_Finished ( 12, true );
	DMA_Finished ( 12 );
}


void Dma::DMA11_Run ()
{
#ifdef INLINE_DEBUG_RUN_DMA11
	debug << "; DMA11_Run; DMA11: SIO2in";
	debug << "; BA=" << hex << DmaCh [ 11 ].BCR.BA << "; BS=" << DmaCh [ 11 ].BCR.BS;
#endif

	u32 Temp;
	u32 NumberOfTransfers;
	u32 *Data;
	u32 *BufPtr;
	
	// make sure SIO input is ready
	if( !Playstation1::SIO::_SIO->SIO2in_DMA_Ready () )
	{
#ifdef INLINE_DEBUG_RUN_DMA11
	debug << "; SIO2in device not ready";
#endif

		// SIO2in not ready for dma transfer
		return;
	}
	
#ifdef INLINE_DEBUG_RUN_DMA11
	debug << "; SIO2in device READY";
#endif

	// make sure dma channel is ok to run
	if ( !isEnabledAndActive ( 11 ) )
	{
#ifdef INLINE_DEBUG_RUN_DMA11
	debug << "; DMA#11 not enabled or active";
#endif

		return;
	}

#ifdef INLINE_DEBUG_RUN_DMA11
	debug << "; DMA#11 ENABLED AND ACTIVE";
#endif


	// bus is free //

	// only transfer BCR.BS cycles
	//NumberOfTransfers = ( DmaCh [ 0 ].BCR.BA + 1 ) * DmaCh [ 0 ].BCR.BS;
	//NumberOfTransfers = DmaCh [ 0 ].BCR.BA * DmaCh [ 0 ].BCR.BS;
	
	// ** todo ** don't forget to check BCR.BA
	
	// number of transfers is not known yet
	NumberOfTransfers = DmaCh [ 11 ].BCR.BS * DmaCh [ 11 ].BCR.BA;
	
	// bus should be busy during this time
	_BUS->ReserveBus ( NumberOfTransfers );
	
	// cache data into SIO2 Buffer
	BufPtr = (u32*) ( & ( SIO2_Buffer [ 0 ] ) );

	while ( DmaCh [ 11 ].BCR.BA )
	{
		// ***todo*** initiate new transfer
		//Playstation1::SIO::_SIO->StartNewCommand ();
		
		// get pointer to data
		Data = & _BUS->MainMemory.b32 [ ( DmaCh [ 11 ].MADR & 0x1fffff ) >> 2 ];
		
		// transfer the data to SIO
		// *** todo *** should number of transfers be in bytes or words??
		Playstation1::SIO::_SIO->DMA_WriteBlock ( (u8*) Data, DmaCh [ 11 ].BCR.BS << 2 );
		
#ifdef INLINE_DEBUG_RUN_DMA11_OUTPUT
	debug << "\r\nDMA11InputToSIO=" << hex << setw ( 2 );
	u8 *Data8 = (u8*) Data;
	for ( int i = 0; i < ( DmaCh [ 11 ].BCR.BS << 2 ); i++ )
	{
		debug << " " << (u32) *Data8++;
	}
#endif

		// cache the command response
		//Playstation1::SIO::_SIO->DMA_ReadBlock ( (u8*) BufPtr, DmaCh [ 11 ].BCR.BS << 2 );
		
		// update cache buffer ptr
		//BufPtr += (u32) DmaCh [ 11 ].BCR.BS;
		
		// dec BCR.BA
		DmaCh [ 11 ].BCR.BA--;

		if ( DmaCh [ 11 ].BCR.BA )
		{
			// update MADR ??
			DmaCh [ 11 ].MADR += (u32) ( DmaCh [ 11 ].BCR.BS << 2 );
		}
	}
	
	// check if transfer should end
	// probably need transfer restart for now
	//DMA_Finished ( 11, true );
	DMA_Finished ( 11 );
}


void Dma::DMA9_Run ()
{
		// DMA 9 is running
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; DMA9_Run; DMA9: SIF0 IOP->EE";
	debug << "; BA=" << hex << DmaCh [ 9 ].BCR.BA;
#endif

	u32 Temp;
	//u32 Data [ 4 ];
	u32 NumberOfTransfers;
	u32 Data0, Data1;
	u64 EEDMATag;
	u32 *Data;
	
	u32 PS2Complete;
	
	u32 TagID;
	
	u64 ullBusCycles, ullDmaCycles;
	u32 TransferCount;
	
	bool TransferInProgress = true;
	
			
	// the data for DMA9 is transferred to another device through SIF, so it has to be transferred on the opposite end of bus timing
	// so there should be a "wait" (where the cpu is doing nothing cuz bus if occupied) before the transfer
	// going in the opposite direction should be less of an issue possibly
	/*
	if ( c_llSIF0_DmaSetupTime )
	{
		// make sure that this is the exact cycle to run transfer at
		if ( *_DebugCycleCount != NextEventCh_Cycle [ 9 ] )
		{
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; CH#9 (IOP->EE) DMA SETUP TIME";
#endif

			// transfer has just been initiated //
			// this is where dma setup time should be accounted for

			// there is probably a transfer overhead
			SetNextEventCh ( c_llSIF0_DmaSetupTime, 9 );
			
			// get the tag values
			
			return;
		}
	}
	*/
	
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; CH#9 (IOP->EE) TRANSFER";
#endif


	if ( !DmaCh [ 9 ].CHCR.TR )
	{
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; CHCR.TR NOT SET (DMA9)!!";
#endif

		return;

	}

	while ( TransferInProgress )
	{
		// check if PS2 is ready for transfer
		if ( !Playstation2::SIF::_SIF->EE_DMA_In_Ready () )
		{
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; EE SIF0 (EE Ch#5) NOT READY";
#endif

			return;
		}
		
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; EE SIF0 (EE Ch#5) READY";
#endif

		// bus is free //

		// only transfer BCR.BS cycles
		//NumberOfTransfers = ( DmaCh [ 0 ].BCR.BA + 1 ) * DmaCh [ 0 ].BCR.BS;
		//NumberOfTransfers = DmaCh [ 0 ].BCR.BA * DmaCh [ 0 ].BCR.BS;
		
		// *** todo *** don't forget to reserve bus on IOP side too
		
		// number of transfers is not known yet
		//NumberOfTransfers = 0;
		
		// start reading from TADR
		//DmaCh [ 9 ].MADR = DmaCh [ 9 ].TADR;
		
		
		if ( !DmaCh [ 9 ].WordsRemaining )
		{
		// get pointer to tag data
		Data = & _BUS->MainMemory.b32 [ ( DmaCh [ 9 ].TADR & 0x1fffff ) >> 2 ];
		
		// it appears that the tag address TADR points to ONLY the tags, IOP followed by EE, with the IOP/EE tag pairs separated by 128-bits (16 bytes)
		// bits 30 and 31 of Data0 (the address) are used for IRQ/STOP bits
		
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; IOP DMA Tag=" << *((u64*)Data);
#endif

		// transfer the tag //
		
		Playstation2::SIF::_SIF->IOP_DMA_WriteBlock ( 0, Data, 4 );
		
		
		// read IOP TAG //

		// read the tag values
		Data0 = *Data++;
		Data1 = *Data++;
		

		// read the tag for EE
		DmaCh [ 9 ].EEDMATag = *((u64*)Data);
		Data += 2;
		
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; Data0=" << hex << Data0 << "; Data1=" << Data1 << "; EEDMATag=" << DmaCh [ 9 ].EEDMATag;
#endif

		// data0 looks like the address
		// data1 looks like tag/id/etc
		
		//TagID = ( Data1 >> 28 ) & 7;
		
		// set MADR
		// upper 8-bits of Data0 hold IRQ/ID/etc for IOP Tag
		// MADR can only address 2MB max on PS1
		DmaCh [ 9 ].MADR = Data0 & 0x1fffff;
		
		// store the rest of the tag for later to check when transfer is done
		DmaCh [ 9 ].LastIOPTag = Data0;
		
		// *** TESTING *** round up number of words to transfer to next 128-bit boundary
		Data1 = ( Data1 + 0x3 ) & 0xffffc;
		
		// set the total number of words in block
		DmaCh [ 9 ].BlockTotal = Data1;
		
		// the amount remaining to transfer is in Data1
		DmaCh [ 9 ].WordsRemaining = Data1;
		
		// no data has been transferred yet for block
		DmaCh [ 9 ].WCTransferred = 0;
		
		
		
		// has to transfer the tag first, so should probably start with a transfer count of 1
		//TransferCount = 1;
		
		}
		//else
		//{
		
		// don't transfer more data at a time than can be fit in the SIF buffer
		TransferCount = ( c_llSIF0_BufferSize > DmaCh [ 9 ].WordsRemaining ) ? DmaCh [ 9 ].WordsRemaining : c_llSIF0_BufferSize;
		
		//}

		// the data for DMA9 is transferred to another device through SIF, so it has to be transferred on the opposite end of bus timing
		// so there should be a "wait" (where the cpu is doing nothing cuz bus if occupied) before the transfer
		// going in the opposite direction should be less of an issue possibly
		
		// double check
		if ( !TransferCount )
		{
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; ALERT: TransferCount is zero!";
#endif

			cout << "\nhps1x64: ALERT: TransferCount is zero!\n";
		}

		
		// wait before transferring data //
		
		
		// make sure that this is the exact cycle to run transfer at
		/*
		if ( *_DebugCycleCount > NextEventCh_Cycle [ 9 ] )
		{
			// must be first starting transfer of block //
			
			// need to let the bus know that it is busy and transferring data
			_BUS->ReserveBus ( TransferCount );
			
			if ( c_llSIF0_TransferTime )
			{
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; CH#9 (IOP->EE) DMA SETUP TIME";
#endif

				// transfer of block has just been initiated //
				
				// the transfer to sif should take a certain number of cycles
				ullDmaCycles = TransferCount;	// * c_llSIF0_TransferTime;
				
				// after the transfer, the dma won't be ready again for a certain number of cycles
				// should be the amount of transfer divided by 4 words per quadword + any overhead
				//ullDmaCycles += ( TransferCount >> 2 ) + ( ( TransferCount & 3 ) ? 1 : 0 );

				// transfer the next block after that much time
				SetNextEventCh ( ullDmaCycles, 9 );
				
				// see you later
				return;
			}
		}
		else if ( *_DebugCycleCount < NextEventCh_Cycle [ 9 ] )
		{
			// there is an event scheduled for channel, but it hasn't happened yet //
			return;
		}
		*/
		
		// it is time to actually transfer the data //
		

#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "\r\n***DMA#9 SIF0 (IOP->EE) Transfering data from MADR=" << hex << ( DmaCh [ 9 ].MADR + ( DmaCh [ 9 ].WCTransferred << 2 ) ) << " to SIF. IOPTag IRQ/ID/ADDR=" << Data0 << dec << " Transferring(WC)=" << TransferCount << " out of " << DmaCh [ 9 ].BlockTotal << " Cycle#" << dec << *_DebugCycleCount << "\r\n";
#endif

		/*
		ulTransferCount = Data1;
		
		// if accounting for data transit time, then modify transfer amount
		if ( c_llSIF0_WaitCyclesPerTransfer )
		{
			ulTransferCount = ( ulTransferCount < 32 ) ? ulTransferCount : 32;
			
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; TRANSFER COUNT=" << dec << ulTransferCount;
#endif
		}
		*/
		

		// get pointer to data
		Data = & _BUS->MainMemory.b32 [ ( ( DmaCh [ 9 ].MADR + ( DmaCh [ 9 ].WCTransferred << 2 ) ) & 0x1fffff ) >> 2 ];
		
		// check that there is data to transfer
		if ( TransferCount )
		{
			// transfer the data to EE
			Playstation2::SIF::_SIF->IOP_DMA_WriteBlock ( DmaCh [ 9 ].EEDMATag, Data, TransferCount );
			//PS2Complete = Playstation2::SIF::_SIF->IOP_DMA_WriteBlock ( DmaCh [ 9 ].EEDMATag, Data, TransferCount );
		}
		
		/*
		if ( c_llSIF0_BusCyclesPerTransfer )
		{
			// hold the bus for the time it was in use
			ullBusCycles = Data1 * c_llSIF0_BusCyclesPerTransfer;
			//_BUS->ReserveBus ( Data1 );
			_BUS->ReserveBus ( ullBusCycles );
			
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; RESERVE BUS=" << dec << ullBusCycles;
#endif
		}
		*/
		
		
		// update amount transferred and remaining
		DmaCh [ 9 ].WordsRemaining -= TransferCount;
		DmaCh [ 9 ].WCTransferred += TransferCount;
		
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "\r\n";
	debug << " WCTransferred=" << dec << DmaCh [ 9 ].WCTransferred;
	debug << " WCRemaining=" << dec << DmaCh [ 9 ].WordsRemaining;
	debug << dec << " PS2Cycle#" << *Playstation2::SIF::_DebugCycleCount;
#endif

		// the actual transfer has to be after the bus is marked as busy
		_BUS->ReserveBus ( TransferCount );
		
		// just in case
		if ( DmaCh [ 9 ].WordsRemaining < 0 )
		{
			cout << "\nhps2x64: ***ALERT***: DMA CH#9: Words remaining is negative!\n";
			DmaCh [ 9 ].WordsRemaining = 0;
		}
		
		
		// need to let the bus know that it is busy and transferring data
		//_BUS->ReserveBus ( TransferCount );
		
		
		// check if transfer of block is complete
		if ( !DmaCh [ 9 ].WordsRemaining )
		{
			// update TADR
			DmaCh [ 9 ].TADR += 16;
			
			// check if transfer should end
			//if ( Data0 & 0xc0000000 )
			if ( DmaCh [ 9 ].LastIOPTag & 0xc0000000
#ifdef REQUIRE_FULLSTOP_DMA9
				== 0xc0000000
#endif
			)
			{
				
				DMA_Finished ( 9, false );
				
				// check if EE dma #5 is done?
				if ( !Playstation2::SIF::_SIF->EE_DMA_In_Ready () )
				{
					// clear bits in sif
					Playstation2::SIF::_SIF->lSBUS_F240 &= ~0x2020;
				}

				
				//TransferInProgress = false;
				return;
			}
		}
		
		
		// if transfer is complete on PS2 side, then need to clear TR and return
		/*
		if ( PS2Complete )
		{
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; TRANSFER STOPPED ON EE SIDE";
#endif

			DmaCh [ 9 ].CHCR.TR = 0;
			return;
		}
		*/
		
		
		// wait after transferring data //
		
		
		if ( c_llSIF0_TransferTime )
		{
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; CH#9 (IOP->EE) DMA SETUP TIME";
#endif

			// transfer of block has just been initiated //
			
			// the transfer to sif should take a certain number of cycles
			ullDmaCycles = TransferCount * c_llSIF0_TransferTime;
			
			// after the transfer, the dma won't be ready again for a certain number of cycles
			// should be the amount of transfer divided by 4 words per quadword + any overhead
			//ullDmaCycles += ( TransferCount >> 2 ) + ( ( TransferCount & 3 ) ? 1 : 0 );
			
			// if zero, then needs to be a minimum of one
			// but then this just adds fuel to the problem
			//ullDmaCycles = ( ( ullDmaCycles <= 0 ) ? 1 : ullDmaCycles );

			// transfer the next block after that much time
			SetNextEventCh ( ullDmaCycles, 9 );
			
			// see you later
			return;
		}
		
		
		
		
		/*
		if ( c_llSIF0_TransferTime )
		{
			// there an extra 2 to 4 cycles added on top of transfer time
			//ullDmaCycles = ulTransferCount * c_llSIF0_WaitCyclesPerTransfer;
			ullDmaCycles = 4 + ( TransferCount * c_llSIF0_TransferTime );
			
			// need to transfer more data when transfer is ready //
			SetNextEventCh ( ullDmaCycles, 9 );
			
			//TransferInProgress = false;
			return;
			
#ifdef INLINE_DEBUG_RUN_DMA9
	debug << "; WAIT=" << dec << ullDmaCycles;
#endif
		}
		*/
		
		// there is more data to transfer, so continue the transfer //
		
		// since NextEventCh_Cycle [ 9 ] == Cycle#, this should continue the transfer
		NextEventCh_Cycle [ 9 ] = 0;
		
		
	}
}


// returns 1 when transfer is complete, zero otherwise
void Dma::DMA10_WriteBlock ( u32* Data, u32 WordCount )
{
#ifdef INLINE_DEBUG_RUN_DMA10
	debug << "\r\nDMA10_WriteBlock: SIF1 EE->IOP";
	debug << "; WordCount=" << dec << WordCount;
	//debug << "; WC=" << dec << Data [ 1 ];
#endif

	// dma transfer has been started //
	
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
	if ( DmaCh [ 10 ].CHCR.ChainTransferMode /* && DmaCh [ 10 ].CHCR.DestinationChainMode */ )
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
			DmaCh [ 10 ].MADR = Data0 & 0x1fffff;
			DmaCh [ 10 ].LastIOPTag = Data0;
			
			// ***TESTING*** round number of words DOWN to next 128-bit boundary
			Data1 &= 0xffffc;
			
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
	debug << "; DMA10_MADR=" << hex << DmaCh [ 10 ].MADR << " WordsToTransfer=" << DmaCh [ 10 ].WordsRemaining << "\r\n";
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
		
		////////////////////////////////////
		// mark bus as in use this cycle
		//_BUS->ReserveBus ( DmaCh [ 6 ].BCR.Value );
		_BUS->ReserveBus ( WordCount );
		
		//BusyCycles = DmaCh [ 6 ].BCR.Value + 2;
		BusyCycles = WordCount + 2;
		//SetNextEventCh ( BusyCycles, 6 );
		

#ifdef INLINE_DEBUG_RUN_DMA10
	debug << "; STORE";
#endif

		// *** do entire dma 10 transfer at once *** //
		
		Temp = DmaCh [ 10 ].MADR;
		
#ifdef INLINE_DEBUG_RUN_DMA10
	debug << "\r\n***DMA#10 SIF1 (EE->IOP) Writing WC=" << hex << WordCount << " to MADR=" << DmaCh [ 10 ].MADR << " Cycle#" << dec << *_DebugCycleCount << "\r\n";
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

			//for ( int i = 0; i < WordCount; i++ )
			for ( int i = 0; i < WC; i++ )
			{
				///////////////////////////////////////////////
				// Send previous address entry to main memory
				_BUS->MainMemory.b32 [ ( DmaCh [ 10 ].MADR & 0x1fffff ) >> 2 ] = *Data++;
				
				
				// invalidate i-cache on transfer from SIF ??
				R3000A::Cpu::_CPU->InvalidateCache ( DmaCh [ 10 ].MADR & 0x1fffff );
				
				
				////////////////////////////////////////////////////
				// update address
				// ***todo*** this is a chain transfer, so maybe MADR should stay at the first address in block?
				DmaCh [ 10 ].MADR += 4;
				
				
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
					_BUS->MainMemory.b32 [ ( DmaCh [ 10 ].MADR & 0x1fffff ) >> 2 ] = *Data++;
#endif

#ifdef DMA10_FILL_EXTRA_ZERO
					_BUS->MainMemory.b32 [ ( DmaCh [ 10 ].MADR & 0x1fffff ) >> 2 ] = 0;
#endif

#ifdef DMA10_FILL_EXTRA_NONE
#else

					////////////////////////////////////////////////////
					// update address
					// ***todo*** this is a chain transfer, so maybe MADR should stay at the first address in block?
					DmaCh [ 10 ].MADR += 4;
					
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
				Playstation2::SIF::_SIF->lSBUS_F240 &= ~0x4040;
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






static void Dma::RequestData ( int Channel, int NumberOfWords )
{
	// this should load the data immediately //

	// set an event for dma channel for the next cycle
	//_DMA->SetNextEventCh ( 1, Channel );
}


// this does not appear to be called from anywhere anymore and can possibly be removed?
void Dma::StartDMA ( int iChannel )
{
//#ifdef INLINE_DEBUG_COMPLETE
//	debug << "\r\nStarting DMA#" << iChannel;
//#endif

	ChannelEnable_Bitmap |= ( 1 << iChannel );
	
	switch ( iChannel )
	{
		case 6:
			DMA6_Run ( true );
			break;
			
		case 5:
			break;
		
		case 4:
			DMA4_Run ( false );
			break;
		
		case 3:
			DMA3_Run ( true );
			break;
		
		case 2:
			// *** testing *** i think this stuff should be cleared out before starting dma
			LL_Count = 0;
			LL_NextAddress = -1;
			
			DMA2_Run ( false );
			//DMA2_Run ( true );
			break;
		
		case 1:
			DMA1_Run ( false );
			break;
		
		case 0:
			DMA0_Run ( false );
			break;
			
	}
}



void Dma::DMA_Run ( int iChannel, bool CycleStealMode )
{
#ifdef INLINE_DEBUG_DMARUN
	debug << "\r\nDma::DMA_Run; Channel=" << dec << iChannel;
#endif

	if ( iChannel == -1 ) return;

//#ifdef INLINE_DEBUG_COMPLETE
//	debug << "\r\nStarting DMA#" << iChannel;
//#endif

	ChannelEnable_Bitmap |= ( 1 << iChannel );
	
	switch ( iChannel )
	{
#ifdef PS2_COMPILE

		case 12:
#ifdef INLINE_DEBUG_DMARUN
	debug << "; DMA12RUN";
#endif

			DMA12_Run ();
			break;

		case 11:
#ifdef INLINE_DEBUG_DMARUN
	debug << "; DMA11RUN";
#endif

			DMA11_Run ();
			break;

		case 10:
		
			// make sure EE is ready for the DMA transfer
			if ( !Playstation2::SIF::_SIF->EE_DMA_Out_Ready () )
			{
#ifdef INLINE_DEBUG_DMARUN
	debug << "; PS2 DMA OUT NOT READY";
#endif

				// PS2 dma is not ready
				return;
			}
			
#ifdef INLINE_DEBUG_DMARUN
	debug << "; PS2 DMA OUT READY";
#endif

			// triggers PS2 dma to transfer data it has into IOP
			Playstation2::SIF::_SIF->IOP_DMA_ReadBlock ();
			
			break;
			
		case 9:
		
			// make sure EE is ready for the DMA transfer
			if ( !Playstation2::SIF::_SIF->EE_DMA_In_Ready () )
			{
#ifdef INLINE_DEBUG_DMARUN
	debug << "; PS2 DMA IN NOT READY";
#endif

				// PS2 dma is not ready
				return;
			}
			
#ifdef INLINE_DEBUG_DMARUN
	debug << "; PS2 DMA IN READY";
#endif

			// triggers IOP dma to transfer data it has into EE
			DMA9_Run ();
			
			break;
			
			
		case 7:
			DMA7_Run ( CycleStealMode );
			break;
			
#endif
	
		case 6:
			DMA6_Run ( true );
			break;
			
		case 5:
			break;
		
		case 4:
			DMA4_Run ( CycleStealMode );
			break;
		
		case 3:
			DMA3_Run ( true );
			break;
		
		case 2:
			// *** testing *** i think this stuff should be cleared out before starting dma
			//LL_Count = 0;
			//LL_NextAddress = -1;
			
			DMA2_Run ( CycleStealMode );
			//DMA2_Run ( true );
			break;
		
		case 1:
			DMA1_Run ( false );
			break;
		
		case 0:
			DMA0_Run ( false );
			break;
			
	}
}




void Dma::Run ()
{
	//u32 Temp;
	//u32 Data [ 4 ];
	
	// will use this for MDEC for now
	//u32 NumberOfTransfers;
		

	// check if dma is doing anything starting at this particular cycle
	if ( NextEvent_Cycle != *_DebugCycleCount ) return;

#ifdef INLINE_DEBUG_RUN
	debug << "\r\nDma::Run";
	debug << " NextEvent=" << dec << NextEvent_Cycle;
	debug << " CycleCount=" << *_DebugCycleCount;
	debug << " ActiveChannel=" << ActiveChannel;
	// list out the events for channels
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		debug << " " << hex << i << "=" << dec << NextEventCh_Cycle [ i ];
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

#ifdef PS2_COMPILE
	// if this is a PS2, then needs to recalcualate active channel, since this can change due to factors on ps2 side
	// (was causing issues)
	ActiveChannel = GetActiveChannel ();
#endif
	
	// check if dma even has any channels active first
	if ( ActiveChannel == -1 ) return;
	
	// check if the active channel is set to run
	if ( NextEvent_Cycle == NextEventCh_Cycle [ ActiveChannel ] )
	{
#ifdef INLINE_DEBUG_RUN
	debug << " Running";
	debug << " ActiveChannel=" << ActiveChannel << " NextEventCh_Cycle [ ActiveChannel ]=" << NextEventCh_Cycle [ ActiveChannel ];
#endif

		// run the active channel (either 0,1,2,4)
		DMA_Run ( ActiveChannel );
	}
	else
	{
		// if there is an active channel, but it is not set to run on this cycle, then update the next cycle //
		Update_NextEventCycle ();
	}

}

 
static u32 Dma::Read ( u32 Address )
{
#if defined INLINE_DEBUG_READ
	debug << "\r\n\r\nDMA::Read; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
#endif

	u32 DmaChannelNum;

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
			


}


void Dma::Update_ICR ( u32 Data )
{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << "\r\nDma::Update_ICR";
#endif

	u32 ICR_Prev;
	
	ICR_Prev = _DMA->ICR_Reg.Value;
	
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " ICR_Prev=" << hex << ICR_Prev;
#endif

	// bit 31 is "read only"
	_DMA->ICR_Reg.Value = ( _DMA->ICR_Reg.Value & 0x80000000 ) | ( _DMA->ICR_Reg.Value & ~Data & 0x7f000000 ) | ( Data & 0x00ffffff );
	
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " ICR_Reg=" << hex << _DMA->ICR_Reg.Value;
#endif

	// ***testing*** check if we should clear all the interrupts
	if ( Data & 0x80000000 )
	{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " CLEARALL";
#endif

		// *** testing *** clear all pending interrupts
		_DMA->ICR_Reg.Value &= 0x00ffffff;
		
		// should this be done for ICR2 also??
		
		// *** testing ***
		//_DMA->ClearInterrupt ();
	}
	
	// check if interrupt was disabled
	// if so, then clear interrupt flag
	//_DMA->ICR_Reg.Value = ( _DMA->ICR_Reg.Value & 0x80ffffff ) | ( ( ( _DMA->ICR_Reg.Value >> 24 ) & ( _DMA->ICR_Reg.Value >> 16 ) & 0x7f ) << 24 );
	
	// check if interrupts have been all acknowledged/cleared
	if ( ! ( _DMA->ICR_Reg.Value & 0x7f000000 )
#ifdef PS2_COMPILE
			&& ! ( _DMA->ICR2_Reg.Value & 0x7f000000 )
#endif
		)
	{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " CLEARINT";
#endif

		// all interrupts have been cleared, so there are none pending
		// clear interrupt pending flag
		_DMA->ICR_Reg.Value &= 0x7fffffff;
		
		// *** testing ***
		//_DMA->ClearInterrupt ();
	}
	else
	{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " CHECKINT";
#endif

		// check that interrupts are enabled
		if ( _DMA->ICR_Reg.Value & 0x00800000 )
		{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " INTENABLED";
#endif

			// Interrupts are enabled //
			
			// check if interrupt bit 31 should be set for ICR1
			if ( ( _DMA->ICR_Reg.Value & ( _DMA->ICR_Reg.Value << 8 ) & 0x7f000000 )
#ifdef PS2_COMPILE
				|| ( _DMA->ICR2_Reg.Value & ( _DMA->ICR2_Reg.Value << 8 ) & 0x7f000000 )
#endif
				)
			{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " SETINTBIT";
#endif

				// set the interrupt bit //
				_DMA->ICR_Reg.Value |= 0x80000000;

#ifdef INT_TRANSITION_ONLY
				// if bit 31 went from 0 to 1, then interrupt
				// ***todo*** for not interrupt regardless since dma might be sending a continuous interrupt
				if ( ( ICR_Prev ^ 0x80000000 ) & _DMA->ICR_Reg.Value & 0x80000000 )
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
	_DMA->ICR_Reg.Value &= ~0x00007fc0;
	
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " (final) ICR_Reg=" << hex << _DMA->ICR_Reg.Value;
#endif
}


#ifdef PS2_COMPILE

void Dma::Update_ICR2 ( u32 Data )
{
	u32 ICR_Prev;
	
	ICR_Prev = _DMA->ICR_Reg.Value;
	
	// bit 31 is "read only"
	_DMA->ICR2_Reg.Value = ( _DMA->ICR2_Reg.Value & 0x80000000 ) | ( _DMA->ICR2_Reg.Value & ~Data & 0x7f000000 ) | ( Data & 0x00ffffff );
	
	// ***testing*** check if we should clear all the interrupts
	if ( Data & 0x80000000 )
	{
		// *** testing *** clear all pending interrupts
		// not sure if this is valid for ICR2 though
		_DMA->ICR2_Reg.Value &= 0x00ffffff;
		
		// *** testing ***
		//_DMA->ClearInterrupt ();
	}
	
	// check if interrupt was disabled
	// if so, then clear interrupt flag
	//_DMA->ICR_Reg.Value = ( _DMA->ICR_Reg.Value & 0x80ffffff ) | ( ( ( _DMA->ICR_Reg.Value >> 24 ) & ( _DMA->ICR_Reg.Value >> 16 ) & 0x7f ) << 24 );
	
	// check if interrupts have been all acknowledged/cleared
	if ( ! ( _DMA->ICR2_Reg.Value & 0x7f000000 ) )
	{
		// all interrupts have been cleared, so there are none pending
		// clear interrupt pending flag
		// ***todo*** not sure if this is needed
		_DMA->ICR2_Reg.Value &= 0x7fffffff;
		
		// check if this is also the case for ICR1
		if ( ! ( _DMA->ICR_Reg.Value & 0x7f000000 ) )
		{
			// this should DEFINITELY be done for ICR1
			// clear interrupt bit 31 since all interrupts are cleared/acknowledged
			_DMA->ICR_Reg.Value &= 0x7fffffff;
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
		if ( _DMA->ICR_Reg.Value & 0x00800000 )
		{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " INTENABLED";
#endif

			// Interrupts are enabled //
			
			// check if interrupt bit 31 should be set for ICR1
			if ( ( _DMA->ICR_Reg.Value & ( _DMA->ICR_Reg.Value << 8 ) & 0x7f000000 )
#ifdef PS2_COMPILE
				|| ( _DMA->ICR2_Reg.Value & ( _DMA->ICR2_Reg.Value << 8 ) & 0x7f000000 )
#endif
				)
			{
#ifdef INLINE_DEBUG_UPDATE_ICR
	debug << " SETINTBIT";
#endif

				// set the interrupt bit //
				_DMA->ICR_Reg.Value |= 0x80000000;

#ifdef INT_TRANSITION_ONLY
				// if bit 31 went from 0 to 1, then interrupt
				// ***todo*** for now interrupt anyway to test if dma is sending a continuous interrupt
				if ( ( ICR_Prev ^ 0x80000000 ) & _DMA->ICR_Reg.Value & 0x80000000 )
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
		
		// *** testing ***
		// don't do this for ICR2.. or even ICR1 probably too
		//_DMA->SetInterrupt ();
	}
	
	// bits 6-14 are always zero
	_DMA->ICR2_Reg.Value &= ~0x00007fc0;
}

#endif


static void Dma::Write ( u32 Address, u32 Data, u32 Mask )
{
#if defined INLINE_DEBUG_WRITE
	debug << "\r\n\r\nDMA::Write; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
#endif

	u32 DmaChannelNum;
	u32 Temp;
	u32 Shift;
	u32 Previous_Value;
	
	//u32 NewActiveChannel;
	//bool FreezeCPU;
	
	// if offset, then shift data+mask over
	Shift = ( Address & 3 ) << 3;
	Data <<= Shift;
	Mask <<= Shift;
	
	// should apply write mask ahead of time, just to make sure
	Data &= Mask;
	
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

			/*
			// check if a channel with higher priority has been newly activated
			if ( NewActiveChannel != _DMA->ActiveChannel && _DMA->isEnabledAndActive( NewActiveChannel ) )
			{
#ifdef INLINE_DEBUG_WRITE
		debug << "; FREEZECPU";
#endif

				// the channel being written to has been activated, but it is of lower priority, so freeze CPU //
				// only dma channels 2 and 4 can freeze cpu
				// the channel that is now active has not been started yet //
				_DMA->ActiveChannel = NewActiveChannel;
				_DMA->DMA_Run ( _DMA->ActiveChannel, true );
				// when the channel finishes, it should automatically check for any other pending transfers
			}
			*/
			
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
						
						
						// check if bit 24 transitioned from 0 to 1
						/*
						if ( ( Previous_Value ^ 0x01000000 ) & _DMA->DmaCh [ DmaChannelNum ].CHCR.Value & 0x01000000 )
						{
							// make sure channel is enabled in PCR
							if ( _DMA->isEnabled ( DmaChannelNum ) )
							{
								// transfer has started and is now in progress until it finishes
								_DMA->Channels_InProgress |= ( 1 << DmaChannelNum );
							
								_DMA->DMA_Run ( DmaChannelNum );
							}
							
						}
						*/
						
						_DMA->DMA_Update ( DmaChannelNum );
						
						// check if channel
						
						/*
						// recalculate the active channel
						NewActiveChannel = _DMA->GetActiveChannel ();
						
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CHCR_2
					debug << "\r\nActiveChannel=" << dec << _DMA->ActiveChannel << " NewActiveChannel=" << NewActiveChannel << " DmaChannelNum=" << DmaChannelNum;
#endif

						// check if the channel that is now active is the same as the channel that is being written to
						if ( NewActiveChannel != DmaChannelNum && _DMA->isEnabledAndActive( DmaChannelNum ) )
						{
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CHCR_3
					debug << "; FREEZECPU";
#endif

							// the channel being written to has been activated, but it is of lower priority, so freeze CPU //
							
							// only dma channels 2 and 4 can freeze cpu
							// the channel that is now active has not been started yet //
							_DMA->ActiveChannel = NewActiveChannel;
							_DMA->DMA_Run ( _DMA->ActiveChannel, true );
							// when the channel finishes, it should automatically check for any other pending transfers
						}
						else if ( NewActiveChannel == DmaChannelNum )
						{
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CHCR_4
					debug << "; NEWTRANSFER";
#endif

							// the new transfer is interrupting the one in progress //
							
							// make sure it is not any dma channels already in progress
							if ( NewActiveChannel != _DMA->ActiveChannel )
							{
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CHCR_5
						debug << "\r\nStarting DMA#" << NewActiveChannel;
#endif

								// set the new active channel
								_DMA->ActiveChannel = NewActiveChannel;
								
								// set the last dma start address
								_DMA->StartA = _DMA->DmaCh [ DmaChannelNum ].MADR;
								
								// start the new dma transfer
								_DMA->DMA_Run ( _DMA->ActiveChannel, false );
								// when the channel finishes, it should automatically check for any other pending transfers
							}
						}
						*/


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


}



void Dma::DMA_Update ( int DmaChannelNum )
{
	u32 NewActiveChannel;
	
	// make sure dma channel number is within proper range
	//if ( DmaChannelNum >= c_iNumberOfChannels || DmaChannelNum < 0 ) return;
	
	// recalculate the active channel
	NewActiveChannel = GetActiveChannel ();

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

}




u32 Dma::GetPriority ( int Channel )
{
#ifdef PS2_COMPILE
	if ( Channel >= 7 )
	{
		return ( PCR2_Reg.Value >> ( ( Channel - 7 ) << 2 ) ) & 0x7;
	}
#endif

	return ( PCR_Reg.Value >> ( Channel << 2 ) ) & 0x7;
}

u32 Dma::isEnabled ( int Channel )
{
	if ( ((u32)Channel) >= c_iNumberOfChannels ) return 0;

#ifdef PS2_COMPILE
	// there are 7 PS1 DMA Channels, numbered 0-6
	if ( Channel >= 7 )
	{
		return ( PCR2_Reg.Value >> ( ( ( Channel - 7 ) << 2 ) + 3 ) ) & 0x1;
	}
#endif

	return ( PCR_Reg.Value >> ( ( Channel << 2 ) + 3 ) ) & 0x1;
}


// check if any channel is enabled
u32 Dma::isEnabled ()
{
#ifdef PS2_COMPILE
	return ( PCR_Reg.Value & 0x8888888 ) | ( PCR2_Reg.Value & 0x888888 );
#endif

	return PCR_Reg.Value & 0x8888888;
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
		if ( DmaCh [ 0 ].CHCR.TR /*&& _MDEC->DMA_ReadyForWrite ()*/ )
		{
			return 1;
		}
		
		return 0;
	}
	
	if ( Channel == 1 )
	{
		if ( DmaCh [ 1 ].CHCR.TR /*&& _MDEC->DMA_ReadyForRead ()*/ )
		{
			return 1;
		}
		
		return 0;
	}
	
	return DmaCh [ Channel ].CHCR.TR;
}


// check if any channel on dma device is active
u32 Dma::isActive ()
{
#ifdef PS2_COMPILE
	return ( DmaCh [ 0 ].CHCR.TR /*&& _MDEC->DMA_ReadyForWrite ()*/ ) || ( DmaCh [ 1 ].CHCR.TR /*&& _MDEC->DMA_ReadyForRead ()*/ ) ||
			DmaCh [ 2 ].CHCR.TR || DmaCh [ 3 ].CHCR.TR || DmaCh [ 4 ].CHCR.TR || DmaCh [ 5 ].CHCR.TR || DmaCh [ 6 ].CHCR.TR ||
			DmaCh [ 7 ].CHCR.TR || DmaCh [ 8 ].CHCR.TR ||
			( DmaCh [ 9 ].CHCR.TR && Playstation2::SIF::_SIF->EE_DMA_In_Ready () ) || ( DmaCh [ 10 ].CHCR.TR && Playstation2::SIF::_SIF->EE_DMA_Out_Ready () ) ||
			( DmaCh [ 11 ].CHCR.TR && Playstation1::SIO::_SIO->SIO2in_DMA_Ready () ) || ( DmaCh [ 12 ].CHCR.TR && Playstation1::SIO::_SIO->SIO2out_DMA_Ready () );
#endif

	return ( DmaCh [ 0 ].CHCR.TR /*&& _MDEC->DMA_ReadyForWrite ()*/ ) || ( DmaCh [ 1 ].CHCR.TR /*&& _MDEC->DMA_ReadyForRead ()*/ ) ||
			DmaCh [ 2 ].CHCR.TR || DmaCh [ 3 ].CHCR.TR || DmaCh [ 4 ].CHCR.TR || DmaCh [ 5 ].CHCR.TR || DmaCh [ 6 ].CHCR.TR;
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
			if ( DmaCh [ 3 ].CHCR.StartOnReadySignal )
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


#ifdef PS2_COMPILE

u32 Dma::isWaiting ( int Channel )
{
	if( DmaCh [ Channel ].CHCR.StartOnReadySignal ) return 1;
	
	// also return true for auto dma
	switch ( Channel )
	{

		case 4:
			// for now, let's try making auto-dma non-actively transferring //
			if ( SPU2::_SPU2->SPU0.isADMATransferMode () && !SPU2::_SPU2->SPU0.SoundDataInput_Enable )
			{
				return 1;
			}
			//else if ( SPU2::_SPU2->SPU0.isADMATransferMode () )
			//{
			//	return 10000;
			//}
			
			break;
			
		case 7:
			// for now, let's try making auto-dma non-actively transferring //
			if ( SPU2::_SPU2->SPU1.isADMATransferMode () && !SPU2::_SPU2->SPU1.SoundDataInput_Enable )
			{
				return 1;
			}
			//else if ( SPU2::_SPU2->SPU1.isADMATransferMode () )
			//{
			//	return 10001;
			//}
			
			break;
		
		default:
			break;
	}

	return 0;
}

#endif



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
	MADR = 0;
	BCR.Value = 0;
	CHCR.Value = 0;
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
	DmaCh [ index ].CHCR.Value &= ~( ( 1L << 0x18 ) | ( 1L << 0x1c ) );	//dma->n_channelcontrol &= ~( ( 1L << 0x18 ) | ( 1L << 0x1c ) );

}


// returns interrupt status
void Dma::DMA_Finished ( int index, bool SuppressDMARestart, bool SuppressEventUpdate, bool SuppressDMAStop )
{
#ifdef INLINE_DEBUG_COMPLETE
	debug << "\r\n\r\nDMA" << dec << index << "::Finished; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << hex << ";(before) Intc_Stat=" << *_Intc_Stat << "; _Intc_Mask=" << *_Intc_Mask << "; _R3000a_Status=" << *_R3000a_Status;
	debug << "; ICR=" << ICR_Reg.Value;
#ifdef PS2_COMPILE
	debug << "; ICR2=" << ICR2_Reg.Value;
#endif
#endif
	
	u32 ICR_Prev;
	
	EndA = DmaCh [ index ].MADR + DmaCh [ index ].BCR.BS;
	
	// allow all dma's to operate at next opportunity
	SelectedDMA_Bitmap = 0xffffffff;
	
	// clear bit in bitmap for dma channel
	ChannelEnable_Bitmap &= ~( 1 << index );
	
	// clear bit for the transfer for channel being in progress
	// transfer for channel is no longer in progress
	Channels_InProgress &= ~( 1 << index );
	
	
#ifdef PS2_COMPILE
	if ( !SuppressDMAStop )
#endif
	{

		// stop the dma channel
		// note: both bits 24 and 28 get reset after the transfer
		DmaCh [ index ].CHCR.Value &= ~( ( 1L << 0x18 ) | ( 1L << 0x1c ) );	//dma->n_channelcontrol &= ~( ( 1L << 0x18 ) | ( 1L << 0x1c ) );
		
	}
	
	// *** testing *** if the dma is finished, then bcr should be zero
	// note: this actually depends on the dma channel
	// note: only sync mode 1 decrements the upper 16-bits of BCR, all other transfer modes leave it as is
	//DmaCh [ index ].BCR.Value = 0;

	// get previous value of ICR
	ICR_Prev = ICR_Reg.Value;
	
#ifdef PS2_COMPILE
	if ( index <= 6 )
	{
#endif
	
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
		
		// check if dma interrupts are enabled for channel
		if ( ICR2_Reg.Value & ( 1 << ( ( 16 + index ) - 7 ) ) )
		{
			// set interrupt pending for channel
			ICR2_Reg.Value |= ( 1 << ( ( 24 + index ) - 7 ) );
			
			// only allow interrupt pending if the interrupt is enabled
			ICR2_Reg.Value &= ( ( ICR2_Reg.Value << 8 ) | 0x80ffffff );

			// check if there are any interrupts pending for ICR1 OR ICR2
			if ( ( ( ICR_Reg.Value & 0x7f000000 ) || ( ICR2_Reg.Value & 0x7f000000 ) ) && ( ( ICR_Reg.Value & 0x00800000 ) == 0x00800000 ) )
			{
				// should definitely set interrupt pending flag for ICR1 probably
				ICR_Reg.Value |= 0x80000000;
				
#ifdef ENABLE_ICR2_BIT31
				// set interrupt pending flag
				// need to test this one, might just need to set for ICR1
				ICR2_Reg.Value |= 0x80000000;
#endif
			}
			else
			{
				// only clear interrupt pending for both ICR1 and ICR2
				ICR_Reg.Value &= 0x7fffffff;
				
				// clear interrupt pending flag
				ICR2_Reg.Value &= 0x7fffffff;
				
				
				// *** TESTING ***
				//ClearInterrupt ();
			}
			
			// check if dma interrupts are enabled globally
			// also check that bit 31 transitioned from 0 to 1
			// do this for just ICR1 to check for interrupt
			if ( ( ! ( ICR_Prev & 0x80000000 ) ) && ( ( ICR_Reg.Value & 0x80800000 ) == 0x80800000 ) )
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

// returns: Interrupt data
/*
u32 Dma::DMA_Interrupt_Update ()
{
	u32 _Int;	//int n_int;
	u32 _Mask;	//int n_mask;
	u32 Interrupt = 0;

	_Int = ( ICR_Reg.Value >> 24 ) & 0x7f;	//n_int = ( n_dicr >> 24 ) & 0x7f;
	_Mask = ( ICR_Reg.Value >> 16 ) & 0xff;	//n_mask = ( n_dicr >> 16 ) & 0xff;


	if( ( _Mask & 0x80 ) != 0 && ( _Int & _Mask ) != 0 )	//if( ( n_mask & 0x80 ) != 0 && ( n_int & n_mask ) != 0 )
	{
		//verboselog( machine(), 2, "dma_interrupt_update( %02x, %02x ) interrupt triggered\n", n_int, n_mask );
		ICR_Reg.Value |= 0x80000000;	//n_dicr |= 0x80000000;
		Interrupt = ( 1 << 3 );	//psx_irq_set( machine(), PSX_IRQ_DMA );
	}
	//else if( n_int != 0 )
	//{
	//	verboselog( machine(), 2, "dma_interrupt_update( %02x, %02x ) interrupt not enabled\n", n_int, n_mask );
	//}
	ICR_Reg.Value &= ( 0x00ffffff | ( ICR_Reg.Value << 8 ) );	//n_dicr &= 0x00ffffff | ( n_dicr << 8 );


	return Interrupt;
}
*/


void Dma::SetNextEventCh ( u64 Cycles, u32 Channel )
{
	NextEventCh_Cycle [ Channel ] = Cycles + *_DebugCycleCount;
	
	Update_NextEventCycle ();
}


void Dma::SetNextEventCh_Cycle ( u64 Cycle, u32 Channel )
{
	NextEventCh_Cycle [ Channel ] = Cycle;
	
	//cout << "\nTEST: Channel=" << dec << Channel << " NextEventCh_Cycle [ Channel ]=" << NextEventCh_Cycle [ Channel ] << " Cycle=" << Cycle;
	
	Update_NextEventCycle ();
}

void Dma::Update_NextEventCycle ()
{
#ifdef INLINE_DEBUG_EVENT
	debug << "\r\nDma::Update_NextEventCycle";
	debug << " (before)NextSystemEvent=" << dec << *_NextSystemEvent;
	debug << " Cycle#" << dec << *_DebugCycleCount;
#endif

	NextEvent_Cycle = -1LL;
	
	for ( int i = 0; i < NumberOfChannels; i++ )
	{
		if ( NextEventCh_Cycle [ i ] > *_SystemCycleCount && ( NextEventCh_Cycle [ i ] < NextEvent_Cycle || NextEvent_Cycle <= *_SystemCycleCount ) )
		{
			// the next event is the next event for device
			NextEvent_Cycle = NextEventCh_Cycle [ i ];
		}
	}

#ifdef INLINE_DEBUG_EVENT
	debug << " NextDMAEvent=" << dec << NextEvent_Cycle;
#endif

	if ( NextEvent_Cycle > *_SystemCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_SystemCycleCount ) ) *_NextSystemEvent = NextEvent_Cycle;
	
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
		
		DMA_ValueList->AddVariable ( "DMA_ICR", &( _DMA->ICR_Reg.Value ) );
		DMA_ValueList->AddVariable ( "DMA_PCR", &( _DMA->PCR_Reg.Value ) );
		
#ifdef PS2_COMPILE
		DMA_ValueList->AddVariable ( "DMA_ICR2", &( _DMA->ICR2_Reg.Value ) );
		DMA_ValueList->AddVariable ( "DMA_PCR2", &( _DMA->PCR2_Reg.Value ) );
		DMA_ValueList->AddVariable ( "SPU2", (u32*) &( SPU2::_SPU2->NextEvent_Cycle ) );
#endif

		for ( i = 0; i < NumberOfChannels; i++ )
		{
			ss.str ("");
			ss << "DMA" << i << "_MADR";
			DMA_ValueList->AddVariable ( ss.str().c_str(), &( _DMA->DmaCh [ i ].MADR ) );
			
			ss.str ("");
			ss << "DMA" << i << "_BCR";
			DMA_ValueList->AddVariable ( ss.str().c_str(), &( _DMA->DmaCh [ i ].BCR.Value ) );
			
			ss.str ("");
			ss << "DMA" << i << "_CHCR";
			DMA_ValueList->AddVariable ( ss.str().c_str(), &( _DMA->DmaCh [ i ].CHCR.Value ) );
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



