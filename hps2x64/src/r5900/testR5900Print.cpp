
#include <iostream>
#include "R5900_Print.h"

using namespace R5900::Instruction;

int main ( void )
{
	static const long c_iSLL0 = 0;
	static const long c_iADD0 = 0x20;
	
	stringstream ss;
	
	// initialize lookup object
	Print::Start ();
	
	// get instruction string from instruction value
	Print::PrintInstruction ( ss, c_iSLL0 );
	
	// output the instruction
	cout << ss.str().c_str() << "\n";
	
	// clear string
	ss.str ("");
	
	// get instruction string from instruction value
	Print::PrintInstruction ( ss, c_iADD0 );
	
	// output the instruction
	cout << ss.str().c_str() << "\n";
	
	
	cout << "\ntesting";
	cin.ignore ();
	return 0;
}