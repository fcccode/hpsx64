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

#include "BreakpointWindow.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
	HWND handle;
	int index;
	
	int a, b, c;

	u32 PC, R1, Last_ReadAddress, Last_WriteAddress;
	
	a = 1; b = 2; c = 7;
	
	WindowClass::Register ( hInstance );
	
	WindowClass::ListView::InitCommonControls ();

	WindowClass::Window win1;
	
	cout << "Creating window #1.\n";
	win1.Create ( "test1", 10, 10, 480, 240 );

	DebugValueList<u32> *dvl = new DebugValueList<u32> ();
	dvl->Create ( &win1, 0, 0, 200, 200 );
	dvl->AddVariable ( "PC", &PC );
	dvl->AddVariable ( "R1", &R1 );
	dvl->AddVariable ( "LastReadAddress", &Last_ReadAddress );
	dvl->AddVariable ( "LastWriteAddress", &Last_WriteAddress );
	//dvl->AddVariable ( "c", &c );
	dvl->EnableVariableEdits ();
	
	Debug_BreakPoints *dbp = new Debug_BreakPoints ();
	
	// add breakpoint
	PC = 0;
	R1 = 3;
	cout << "\nAdding breakpoint#0=" << dbp->Add_BreakPoint ( "__PC<>0&&R1==3", "BreakPoint#1", "Execute" );
	cout << "\nAdding breakpoint#1=" << dbp->Add_BreakPoint ( "__PC==1||R1==4", "BreakPoint#2", "Memory Read/Write" );
	
	
	
	Debug_BreakpointWindow *d = new Debug_BreakpointWindow ( dbp );
	d->Create ( &win1, 220, 0, 240, 200 );
	
	
	d->Update ();
	dvl->Update ();
	
	cout << dec << "\ndbp->Check_IfBreakPointReached()=" << dbp->Check_IfBreakPointReached ();
	
	cout << "\nChanging PC to 2";
	PC = 2;
	cout << dec << "\ndbp->Check_IfBreakPointReached()=" << dbp->Check_IfBreakPointReached ();
	cout << dec << "\ndbp->Check_IfBreakPointReached()=" << dbp->Check_IfBreakPointReached ();
	
	cout << "\nChanging PC to 3";
	PC = 3;
	cout << dec << "\ndbp->Check_IfBreakPointReached()=" << dbp->Check_IfBreakPointReached ();
	cout << dec << "\ndbp->Check_IfBreakPointReached()=" << dbp->Check_IfBreakPointReached ();
	cout << dec << "\ndbp->Check_IfBreakPointReached()=" << dbp->Check_IfBreakPointReached ();
	
	//HMENU hPopup;

	// show a test pop up menu
	//hPopup = CreatePopupMenu();

	//cout << "\nAppending menu items";
	//AppendMenu( hPopup, MF_STRING, 11001, (LPCSTR) "Menu Item#1" );
	//AppendMenu( hPopup, MF_STRING, 11002, (LPCSTR) "Menu Item#2" );
	//AppendMenu( hPopup, MF_STRING, 11003, (LPCSTR) "Menu Item#3" );
	
	//cout << "\nAbout to track pop up menu.";

	//SetForegroundWindow(win1.hWnd);
	//cout << "\nTrackPopupMenu=" << TrackPopupMenu( hPopup, TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD | TPM_NONOTIFY, 0, 0, 0, win1.hWnd, NULL);
	//cout << "\nTrackPopupMenu=" << TrackPopupMenu( hPopup, TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD | TPM_NONOTIFY, 0, 0, 0, win1.hWnd, NULL);
	
	//b = 11;
	
	//d->Update ();

	cin.ignore ();

	delete d;
	return 0;
}


