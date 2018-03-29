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


#include <windows.h>

#include <iostream>

//#include "PS1_Timer.h"
#include "ps1_system.h"

using namespace std;

using namespace Playstation1;

// there should be a system object
System _SYSTEM;

void Output_TicksPerCycle ()
{
	// showing the ticks per cycle for each timer
	cout << "\nTicksPerCycle:";
	
	for ( int i = 0; i < _SYSTEM._TIMERS.NumberOfChannels; i++ )
	{
		cout << " Timer#" << dec << i << ": " << _SYSTEM._TIMERS.TheTimers [ i ].dCyclesPerTick;
	}
}


void Output_TimerValues ()
{
	// showing the ticks per cycle for each timer
	cout << "\nTime Values:";
	
	for ( int i = 0; i < _SYSTEM._TIMERS.NumberOfChannels; i++ )
	{
		_SYSTEM._TIMERS.UpdateTimer ( i );
		
		// also update timer interrupt cycles
		_SYSTEM._TIMERS.Get_NextEvent ( i, _SYSTEM._CPU.CycleCount + 1000000 );
		
		cout << " Timer#" << dec << i << ": " << _SYSTEM._TIMERS.GetTimerValue ( i );
	}
}


void Output_TimerComp ()
{
	// showing the ticks per cycle for each timer
	cout << "\nTimer COMP:";
	
	for ( int i = 0; i < _SYSTEM._TIMERS.NumberOfChannels; i++ )
	{
		cout << " Timer#" << dec << i << ": " << _SYSTEM._TIMERS.TheTimers [ i ].COMP.Value;
	}
}


void Output_TimerMode ()
{
	u16 Mode;
	
	// showing the ticks per cycle for each timer
	cout << "\nTimer MODE:";
	
	for ( int i = 0; i < _SYSTEM._TIMERS.NumberOfChannels; i++ )
	{
		cout << "\nTimer#" << dec << i << ": MODE=" << hex << _SYSTEM._TIMERS.TheTimers [ i ].MODE.Value << dec;
		cout << "\n RunMode:" << ( _SYSTEM._TIMERS.TheTimers [ i ].MODE.RunMode ? " Sync" : " Normal" );
		cout << "\n SyncMode: " << _SYSTEM._TIMERS.TheTimers [ i ].MODE.SyncMode;
		
		cout << "\n Count To:";
		cout << ( _SYSTEM._TIMERS.TheTimers [ i ].MODE.CountToTarget ? " Target" : " Overflow" );
		
		cout << "\n IRQ on:";
		cout << ( _SYSTEM._TIMERS.TheTimers [ i ].MODE.IrqOnTarget ? " Target" : "" );
		cout << ( _SYSTEM._TIMERS.TheTimers [ i ].MODE.IrqOnOverflow ? " Overflow" : "" );
		
		cout << "\n IRQ:";
		cout << ( _SYSTEM._TIMERS.TheTimers [ i ].MODE.IrqMode_Repeat ? " Repeat" : " Once" );
		cout << ( _SYSTEM._TIMERS.TheTimers [ i ].MODE.IrqMode_Toggle ? " Toggle" : " Pulse" );
		
		cout << "\n Source:";
		cout << _SYSTEM._TIMERS.TheTimers [ i ].MODE.ClockSource;
		
		cout << "\n IRQ Request:";
		cout << ( _SYSTEM._TIMERS.TheTimers [ i ].MODE.IrqRequest ? " No" : " Yes" );
		
		cout << "\n Target Reached:";
		cout << ( _SYSTEM._TIMERS.TheTimers [ i ].MODE.TargetReached ? " Yes" : " No" );
		
		cout << "\n Overflow Reached:";
		cout << ( _SYSTEM._TIMERS.TheTimers [ i ].MODE.OverflowReached ? " Yes" : " No" );
		
		cout << "\n Cycles Per Tick:";
		cout << dec << _SYSTEM._TIMERS.TheTimers [ i ].dCyclesPerTick;
		
		cout << "\n Next Event:";
		cout << dec << _SYSTEM._TIMERS.TheTimers [ i ].NextEvent_Cycle;
	}
}


void Output_TimerTarget ()
{
	// showing the ticks per cycle for each timer
	cout << "\nTimer Target:";
	
	for ( int i = 0; i < _SYSTEM._TIMERS.NumberOfChannels; i++ )
	{
		cout << " Timer#" << dec << i << ": " << _SYSTEM._TIMERS.GetTimerTarget ( i );
	}
}



void Output_TimerInfo ()
{
	Output_TicksPerCycle ();
	Output_TimerValues ();
	Output_TimerComp ();
	Output_TimerMode ();
	Output_TimerTarget ();
	
	cout << "\nCycles Per Scanline:" << dec << _SYSTEM._GPU.dCyclesPerScanline;
	cout << "\nCycles Per HBlank:" << _SYSTEM._GPU.dHBlankArea_Cycles;
	cout << "\nCycles Per VBlank0:" << _SYSTEM._GPU.dVBlank0Area_Cycles;
	cout << "\nCycles Per VBlank1:" << _SYSTEM._GPU.dVBlank1Area_Cycles;
	cout << "\nCycles Per Field0:" << _SYSTEM._GPU.dCyclesPerField0;
	cout << "\nCycles Per Field1:" << _SYSTEM._GPU.dCyclesPerField1;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
	Timer::MODE_Format m0;
	Timer::MODE_Format m1;
	
	// must start the system before using it
	_SYSTEM.Start ();
	
	// sync mode 0: pause during blank //
	// sync mode 1: reset to zero at blank //
	// sync mode 2: reset to zero at blank and pause outside of blank //
	// sync mode 3: pause until blank, then do free run //
	
	// set a timer mode for timer 0
	m0.Value = 0;
	m0.RunMode = 0;
	m0.SyncMode = 0;
	m0.ClockSource = 0;
	m0.IrqOnTarget = false;
	m0.IrqOnOverflow = false;
	m0.CountToTarget = false;

	
	// set a timer mode for timer 1
	m1.Value = 0;
	m1.RunMode = 0;
	m1.SyncMode = 0;
	m1.ClockSource = 1;
	m1.IrqOnTarget = true;
	m1.IrqOnOverflow = false;
	m1.CountToTarget = true;
	
	// get the cycles per tick for timer 0
	_SYSTEM._TIMERS.SetMode ( 0, m0.Value );
	_SYSTEM._TIMERS.SetMode ( 1, m1.Value );
	
	// set the compare value for timers
	_SYSTEM._TIMERS.SetComp ( 0, 0 );
	_SYSTEM._TIMERS.SetComp ( 1, 1 );
	
	cout << "\nCycles to next field:" << dec << _SYSTEM._GPU.GetCycles_ToNextFieldStart ( _SYSTEM._CPU.CycleCount );
	cout << "\nCycles since last field:" << dec << _SYSTEM._GPU.GetCycles_SinceLastFieldStart ( _SYSTEM._CPU.CycleCount );
	
	// get the value for timer 0
	Output_TimerInfo ();
	
	
	// advance 2000 cycles
	//_SYSTEM._CPU.CycleCount += 2000;
	_SYSTEM.Run_Cycles ( 10000 );
	
	// update the timer
	//_SYSTEM._TIMERS.UpdateTimer ( 0 );

	cout << "\n\nAfter\n\n";
	
	cout << "\nCycles to next field:" << dec << _SYSTEM._GPU.GetCycles_ToNextFieldStart ( _SYSTEM._CPU.CycleCount );
	cout << "\nCycles since last field:" << dec << _SYSTEM._GPU.GetCycles_SinceLastFieldStart ( _SYSTEM._CPU.CycleCount );
	
	// get the value for timer 0
	Output_TimerInfo ();
	
	cin.ignore ();
	
	return 0;
}

