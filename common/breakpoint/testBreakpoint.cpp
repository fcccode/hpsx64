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

#include "breakpoint.h"
#include "DebugValueList.h"

int a, b, c;

u32 PC, R1, R2;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
	HWND handle;
	int index;
	
	
	a = 1; b = 2; c = 7;
	
	WindowClass::Register ( hInstance );
	
	WindowClass::ListView::InitCommonControls ();
	
	DebugValueList<u32> *d = new DebugValueList<u32> ();
	
	Debug_BreakPoints *b = new Debug_BreakPoints ();

	WindowClass::Window win1;
	
	cout << "Creating window #1.\n";
	win1.Create ( "test1", 10, 10, 480, 240 );
	
	d->Create ( &win1, 0, 0, 200, 200 );
	d->AddVariable ( "PC", &PC );
	d->AddVariable ( "R1", &R1 );
	d->AddVariable ( "R2", &R2 );
	d->EnableVariableEdits ();
	
	d->Update ();
	
	// add breakpoint
	PC = 1;
	R1 = 4;
	cout << "\nAdding breakpoint#0=" << b->Add_BreakPoint ( "PC==1&&R1==5" );
	cout << "\nAdding breakpoint#1=" << b->Add_BreakPoint ( "PC==1&&R1==4" );
	
	// test breakpoint
	cout << "\nTesting breakpoint=" << b->Check_IfBreakPointReached ();
	
	
	//while ( true )
	//{
	//	DebugValueList<int>::DoEvents ();
	//	Sleep ( 250 );
	//}

	cin.ignore ();

	delete d;
	return 0;
}


