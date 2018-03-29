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

using namespace std;

void TestButtonCallback ( HWND hCtrl, int id, unsigned int message, WPARAM wParam, LPARAM lParam )
{
	MessageBox ( NULL, "You clicked on the button.", NULL, NULL );
}

void DefaultCheckBoxHandler ( HWND hCtrl, int id, unsigned int message, WPARAM wParam, LPARAM lParam )
{
	if ( WindowClass::Button::GetCheck ( hCtrl ) == BST_CHECKED )
	{
		WindowClass::Button::SetCheck ( hCtrl, BST_UNCHECKED );
	}
	else
	{
		WindowClass::Button::SetCheck ( hCtrl, BST_CHECKED );
	}
}

#define IDC_LISTVIEW1	3010

void TestListViewHandler ( HWND hCtrl, int id, unsigned int message, WPARAM wParam, LPARAM lParam )
{
	// check that this is the list view that we want
	if ( id == IDC_LISTVIEW1 )
	{
		if ( ((LPNMHDR)lParam)->code == NM_RCLICK )
		{
			MessageBox ( NULL, "You right clicked on the list-view control.", NULL, NULL );
		}
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
	HWND handle;
	int index;
	
	WindowClass::Register ( hInstance );
	
	WindowClass::ListView::InitCommonControls ();

	WindowClass::Window win1, win2, win3;
	
	cout << "Creating window #1.\n";
	win1.Create ( "test1", 10, 10, 480, 240 );
	
	cout << "\nText width=" << win1.Get_TextWidth ( "test" ) << " text height=" << win1.Get_TextHeight ( "test" );
	cout << "\nChanging font to 4 point";
	win1.Set_Font ( win1.CreateFontObject ( 8 ) );
	cout << "\nText width=" << win1.Get_TextWidth ( "test" ) << " text height=" << win1.Get_TextHeight ( "test" );
	
	//handle = win1.CreateControl ( "BUTTON", 10, 10, 100, 20, "testing button", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, (HMENU) 3002, (WindowClass::Window::EventFunction) TestButtonCallback );
	//Button_Enable( handle, false );
	WindowClass::Button *b = new WindowClass::Button();
	b->Create_CmdButton ( &win1, 10, 10, 100, 20, "click here", 3001 );
	b->AddEvent ( NULL, TestButtonCallback );
	WindowClass::Button *c1 = new WindowClass::Button ();
	handle = c1->Create_CheckBox ( &win1, 10, 40, 100, 20, "check box1", 3002 );
	c1->AddEvent ( NULL, DefaultCheckBoxHandler );
	WindowClass::Button *c2 = new WindowClass::Button ();
	c2->Create_CheckBox ( &win1, 10, 70, 100, 20, "check box2", 3003 );
	c2->AddEvent ( NULL, DefaultCheckBoxHandler );
	c2->RemoveEvent ( NULL );

	WindowClass::ComboBox *cb1 = new WindowClass::ComboBox ();
	cb1->Create_DropDownList ( &win1, 10, 130, 100, 100, "", 3004 );
	cb1->AddItem ( "item1" );
	cb1->AddItem ( "item2" );
	
	WindowClass::Edit *e1 = new WindowClass::Edit ();
	e1->Create ( &win1, 10, 100, 100, 20, "edit box1", 3005 );
	
	WindowClass::Static *st1 = new WindowClass::Static ();
	st1->Create_Text ( &win1, 10, 160, 100, 20, "Testing", 3006 );
	
	
	// ideal width of "click here" button
	cout << "\nIdeal with of 'click here' button=" << b->GetIdealWidth () << " Width=" << b->Get_Width () << " Height=" << b->Get_Height() << " x=" << b->Get_X () << " y=" << b->Get_Y();
	

	
	
	

	cout << "\nCreating window #2.\n";
	win2.Create ( "test2", 50, 50, 640, 480 );

	cout << "\nCreating static image...";
	WindowClass::Static *st2 = new WindowClass::Static ();
	st2->Create_Bitmap ( &win2, 10, 10, 50, 50 );
	st2->SetImage ( WindowClass::Static::LoadPicture ( "test.bmp" ) );
	
	cout << "\nCreating list-view control...";
	WindowClass::ListView *lv1 = new WindowClass::ListView ();
	lv1->Create_wHeader ( &win2, 10, 70, 100, 100, "", IDC_LISTVIEW1 );
	
	cout << "\ninserting columns in list view control...";
	lv1->InsertColumn ( 0, WindowClass::ListView::CreateColumn ( 0, 20, "test" ) );
	lv1->InsertColumn ( 1, WindowClass::ListView::CreateColumn ( 1, 20, "col2" ) );
	//lv1->InsertItem ( WindowClass::ListView::CreateItem ( 0, 0, "data" ) );
	
	cout << "\ninserting rows in list view control...";
	lv1->InsertRow ( 0 );
	lv1->InsertRow ( 1 );

	cout << "\nsetting items in list view control...";
	//lv1->SetItemCount ( 5 );
	lv1->SetItemText ( 0, 0, "6" );
	lv1->SetItemText ( 0, 1, "8" );
	lv1->SetItemText ( 1, 1, "data" );
	
	cout << "\nAdding event to list-view control";
	lv1->AddEvent ( WM_NOTIFY, TestListViewHandler );
	
	cout << "Creating window #3.\n";
	win3.Create ( "test3", 30, 30, 640, 480 );
	
	
	cin.ignore ();
	
	return 0;
}



