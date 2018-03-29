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

#include "WinApiHandler.h"

#include "DisassemblyViewer.h"
#include "R3000ADebugPrint.h"

int a, b, c;

u32 MainRam [ 1024 ];
u32 BIOS [ 512 ];



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
	HWND handle;
	int index;

	u32 ProgramCounter = 4;
	
	a = 1; b = 2; c = 7;
	
	MainRam [ 0 ] = 0xc;
	MainRam [ 1 ] = 4;
	MainRam [ 2 ] = 7;
	MainRam [ 3 ] = 12;
	BIOS [ 0 ] = 25;
	BIOS [ 1 ] = 33;
	BIOS [ 2 ] = 57;
	BIOS [ 3 ] = 71;
	
	WindowClass::Register ( hInstance );
	
	WindowClass::ListView::InitCommonControls ();
	
	Debug_DisassemblyViewer *d = new Debug_DisassemblyViewer ();

	WindowClass::Window win1;
	
	cout << "Creating window #1.\n";
	win1.Create ( "test1", 10, 10, 480, 240 );
	
	d->Create ( &win1, 0, 0, 255, 200, R3000ADebugPrint::PrintInstruction );
	d->Add_MemoryDevice ( "RAM", 0x00000000, 1024, (u8*) MainRam );
	d->Add_MemoryDevice ( "BIOS", 0x1fc00000, 512, (u8*) BIOS );
	
	d->SetProgramCounter ( &ProgramCounter );
	
	d->Update ();
	
	//while ( true )
	//{
	//	DebugValueList<int>::DoEvents ();
	//	Sleep ( 250 );
	//}

	cin.ignore ();

	delete d;
	return 0;
}


