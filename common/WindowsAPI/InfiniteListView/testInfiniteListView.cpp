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

#include "InfiniteListView.h"

int a, b, c;

string CellCallback ( unsigned int row, unsigned int col )
{
	stringstream ss;
	ss.str ("");
	ss << "(" << dec << row << "," << col << ")";
	return ss.str ();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
	HWND handle;
	int index;
	
	
	a = 1; b = 2; c = 7;
	
	WindowClass::Register ( hInstance );
	
	WindowClass::ListView::InitCommonControls ();
	
	InfiniteListView *il = new InfiniteListView ();

	WindowClass::Window win1;
	
	cout << "Creating window #1.\n";
	win1.Create ( "test1", 10, 10, 480, 480 );
	
	il->Create ( &win1, 0, 0, 300, 300, 0x3ffffff, CellCallback );
	il->AddColumn ( "col1", 50 );
	il->AddColumn ( "col2", 50 );
	il->AddColumn ( "col3", 50 );
	il->AddColumn ( "col4", 50 );
	il->AddColumn ( "col5", 50 );
	//il->AddAllRows ();
	
	//d->AddVariable ( "a", &a );
	//d->AddVariable ( "b", &b );
	//d->AddVariable ( "c", &c );
	//d->EnableVariableEdits ();
	
	il->Update ();
	il->GoToRow ( 500 );
	il->Update ();
	
	//b = 11;
	
	//d->Update ();
	
	//while ( true )
	//{
	//	DebugValueList<int>::DoEvents ();
	//	Sleep ( 250 );
	//}

	cin.ignore ();

	delete il;
	return 0;
}


