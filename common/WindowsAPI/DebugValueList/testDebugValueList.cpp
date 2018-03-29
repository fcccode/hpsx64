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

#include "DebugValueList.h"

int a, b, c;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
	HWND handle;
	int index;
	
	
	a = 1; b = 2; c = 7;
	
	WindowClass::Register ( hInstance );
	
	WindowClass::ListView::InitCommonControls ();
	
	DebugValueList<int> *d = new DebugValueList<int> ();

	WindowClass::Window win1;
	
	cout << "Creating window #1.\n";
	win1.Create ( "test1", 10, 10, 480, 240 );
	
	d->Create ( &win1, 0, 0, 200, 200 );
	d->AddVariable ( "a", &a );
	d->AddVariable ( "b", &b );
	d->AddVariable ( "c", &c );
	d->EnableVariableEdits ();
	
	d->Update ();
	
	b = 11;
	
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


