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
using namespace std;

#include "PS1DataBus.h"

//Playstation1::DataBus(NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL) d;

int main ()
{
	Playstation1::DataBus* d = new Playstation1::DataBus (NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);

	u32 ReadValue1;
	
	const u32 ValueToWrite1 = 11;
	const u32 AddressToWrite1 = 8;

	cout << "Testing Data Bus.\n";
	
	cout << "Writing value: " << ValueToWrite1 << " to address: " << AddressToWrite1 << "\n";
	
	d->Write ( AddressToWrite1, Playstation1::DataBus::RW_32, ValueToWrite1 );
	
	ReadValue1 = d->Read ( AddressToWrite1, Playstation1::DataBus::RW_32 );
	
	cout << "Read value: " << ReadValue1 << " from address: " << AddressToWrite1 << "\n";
	
	cin.ignore();

	return 0;
}

