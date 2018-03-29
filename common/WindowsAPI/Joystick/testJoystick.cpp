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


//#include "Debug.h"
#include "WinJoy.h"

using namespace std;
using namespace WinApi;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
	HWND handle;
	int index;
	
	int a, b, c;
	
	Joysticks *j = new Joysticks ();
	
	a = j->InitJoysticks ();

	if ( a == 2 /*ERR_NODRIVER*/ )
	{
		cout << "\nNo Joystick Driver.";
	}

	if ( a == 4 /*ERR_NODEVICE*/ )
	{
		cout << "\nNo Joystick Device.";
	}
	
	if ( !a )
	{
		cout << "\nJoystick ok.";
		cout << "\nThere are " << j->wNumDevs << " joysticks found.";
		cout << "\nJoystick 1 attched?" << j->bDev1Attached;
		cout << "\nJoystick 2 attched?" << j->bDev2Attached;
		cout << "\nDevice Id=" << j->wDeviceID;
	}
	
	j->ReadJoystick ( 0 );
	
	while ( !j->joyinfo.dwButtons )
	{
		Sleep ( 1 );
		j->ReadJoystick ( 0 /*j->wDeviceID*/ );
	}
	
	cout << "\nButton#" << j->joyinfo.dwButtonNumber << " was pressed. Buttons state=" << hex << j->joyinfo.dwButtons;
	cout << "\nPOV=" << dec << j->joyinfo.dwPOV << " xPos=" << j->joyinfo.dwXpos << " yPos=" << j->joyinfo.dwYpos << " zPos=" << j->joyinfo.dwZpos << " rPos=" << j->joyinfo.dwRpos << " uPos=" << j->joyinfo.dwUpos << " vPos=" << j->joyinfo.dwVpos;
	
	cin.ignore ();
	
	delete j;

	return 0;
}


