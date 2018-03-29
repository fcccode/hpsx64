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

#include "Debug.h"
#include "ps1_system.h"
#include "PS1_Gpu.h"

using namespace std;

using namespace Playstation1;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
	
	// set priority for process
	//SetPriorityClass ( GetCurrentProcess (), REALTIME_PRIORITY_CLASS );
	
	Debug::Log debug;
	debug.Create ( "GPUTest.Log" );

#ifdef INLINE_DEBUG
	debug << "\r\ncalling WindowClass::Register";
#endif

	WindowClass::Register ( hInstance, "testGPU" );
	
#ifdef INLINE_DEBUG
	debug << "\r\nCreating new Playstation 1 System instance";
#endif

	System *_PS1SYSTEM = new System ();
	
	///////////////////////////////////////////////////////////
	// Debugging must be enabled here
	_PS1SYSTEM->EnableDebugging ();
	_PS1SYSTEM->UpdateDebugWindow ();
	
	////////////////////////////////////////////
	// test drawing of monochrome triangle

	_PS1SYSTEM->_GPU.DrawArea_TopLeftX = 0;
	_PS1SYSTEM->_GPU.DrawArea_TopLeftY = 0;
	_PS1SYSTEM->_GPU.DrawArea_BottomRightX = 640;
	_PS1SYSTEM->_GPU.DrawArea_BottomRightY = 480;
	_PS1SYSTEM->_GPU.DrawArea_OffsetX = 0;
	_PS1SYSTEM->_GPU.DrawArea_OffsetY = 0;

/*	
	_PS1SYSTEM->_GPU.bgr = 0x1f;
	_PS1SYSTEM->_GPU.x0 = 20;
	_PS1SYSTEM->_GPU.y0 = 20;
	_PS1SYSTEM->_GPU.x1 = 200;
	_PS1SYSTEM->_GPU.y1 = 150;
	_PS1SYSTEM->_GPU.x2 = 10;
	_PS1SYSTEM->_GPU.y2 = 70;

	_PS1SYSTEM->_GPU.Draw_MonoTriangle_20 ();
*/

/*	
	_PS1SYSTEM->_GPU.bgr = 0x1f;
	_PS1SYSTEM->_GPU.x0 = 0;
	_PS1SYSTEM->_GPU.y0 = 0;
	_PS1SYSTEM->_GPU.x1 = 640;
	_PS1SYSTEM->_GPU.y1 = 0;
	_PS1SYSTEM->_GPU.x2 = 0;
	_PS1SYSTEM->_GPU.y2 = 480;
	_PS1SYSTEM->_GPU.x3 = 640;
	_PS1SYSTEM->_GPU.y3 = 480;

	_PS1SYSTEM->_GPU.Draw_MonoRectangle_28 ();
*/


	_PS1SYSTEM->_GPU.bgr0 = 0x1f;
	_PS1SYSTEM->_GPU.x0 = 20;
	_PS1SYSTEM->_GPU.y0 = 20;
	_PS1SYSTEM->_GPU.bgr0 = ( 0x1f << 5 );
	_PS1SYSTEM->_GPU.x1 = 200;
	_PS1SYSTEM->_GPU.y1 = 20;
	_PS1SYSTEM->_GPU.bgr2 = ( 0x1f << 10 );
	_PS1SYSTEM->_GPU.x2 = 20;
	_PS1SYSTEM->_GPU.y2 = 100;


	_PS1SYSTEM->_GPU.Draw_GradientTriangle_30 ();
	
	_PS1SYSTEM->_GPU.bgr0 = ( 0x1f << 5 );
	_PS1SYSTEM->_GPU.x0 = 300;
	_PS1SYSTEM->_GPU.y0 = 100;
	_PS1SYSTEM->_GPU.bgr1 = ( 0x1f << 10 );
	_PS1SYSTEM->_GPU.x1 = 500;
	_PS1SYSTEM->_GPU.y1 = 100;
	_PS1SYSTEM->_GPU.bgr2 = ( 0x1f );
	_PS1SYSTEM->_GPU.x2 = 300;
	_PS1SYSTEM->_GPU.y2 = 400;
	_PS1SYSTEM->_GPU.bgr3 = ( 0x1f << 5 ) | 0x1f;
	_PS1SYSTEM->_GPU.x3 = 500;
	_PS1SYSTEM->_GPU.y3 = 400;

	_PS1SYSTEM->_GPU.Draw_GradientRectangle_38 ();

	
	_PS1SYSTEM->UpdateDebugWindow ();
	
	cin.ignore();
	
	delete _PS1SYSTEM;
	
	return 0;
}

