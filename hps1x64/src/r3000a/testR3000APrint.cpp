

#include <iostream>
#include "R3000ADebugPrint.h"

using namespace R3000A::Instruction;

Print p;

int main ()
{
	p.Start ();
	cout << p.PrintInstruction ( 0 ) << "\n";

	cin.ignore ();
	return 0;
}


