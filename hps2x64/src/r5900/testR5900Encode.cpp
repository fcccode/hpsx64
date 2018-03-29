
#include <iostream>
#include "R5900_Encoder.h"

using namespace R5900;
using namespace std;

int main ( void )
{
	cout << "\ntesting";
	
	long inst;
	
	Encoder e;
	
	inst = e.ADDU ( 1, 2, 3 );
	
	cout << "\nTest 1:\n";
	cout << "\nADDU r1, r2, r3= " << hex << inst;
	
	cin.ignore ();
	return 0;
}

