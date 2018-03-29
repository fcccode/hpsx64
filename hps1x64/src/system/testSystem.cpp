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



#include <iostream>


#include "Debug.h"

#include "ps1_system.h"


using namespace std;
using namespace Playstation1;

// enable inline debugging
#define INLINE_DEBUG

System _PS1SYSTEM;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
	// create inline debugger
	Debug::Log debug;
	debug.Create ( "Test_Log.txt" );
	
	int Status;
	
	// set priority for process
	//SetPriorityClass ( GetCurrentProcess (), REALTIME_PRIORITY_CLASS );

#ifdef INLINE_DEBUG
	debug << "\r\ncalling WindowClass::Register";
#endif

	WindowClass::Register ( hInstance, "testSystem" );
	
#ifdef INLINE_DEBUG
	debug << "\r\nCreating new Playstation 1 System instance";
#endif

	//System *_PS1SYSTEM = new System ();

#ifdef INLINE_DEBUG
	debug << "\r\nRunning test program\r\n";
#endif

	do
	{

		Status = _PS1SYSTEM.RunTestProgram ();
		
		if ( Status == 1 )
		{
			_PS1SYSTEM.SaveState ();
		}
		
		if ( Status == 2 )
		{
			_PS1SYSTEM.LoadState ();
		}
		
	} while ( Status );

#ifdef INLINE_DEBUG
	debug << "\r\nPausing for input\r\n";
#endif

	cin.ignore();
	
#ifdef INLINE_DEBUG
	debug << "\r\nDeleting PS1 object\r\n";
#endif

	//delete _PS1SYSTEM;
	
#ifdef INLINE_DEBUG
	debug << "\r\nReturning from program\r\n";
#endif

	return 0;
}

