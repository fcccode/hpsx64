
#include <iostream>
#include "R5900_Execute.h"

using namespace R5900;


int main ( void )
{
	cout << "\ntesting";
	
	Instruction::Format inst;
	
	// create a new R5900 CPU
	Cpu *c = new Cpu ();
	
	// create an Execute Unit for the CPU
	Execute *e = new Execute ( c );
	
	// make sure Execute Unit has started
	Execute::Start ();
	
	// output register values before
	cout << dec << "\nBefore: r0=" << c->GPR [ 0 ].u << " r1=" << c->GPR [ 1 ].u << " r2=" << c->GPR [ 2 ].u;
	
	cout << "\nExecuting test instruction #1 (SLL r0, r0, 0)...";
	
	// execute test instruction
	inst.Value = 0;
	Execute::ExecuteInstruction ( inst );
	
	// output register values after
	cout << dec << "\nAfter: r0=" << c->GPR [ 0 ].u << " r1=" << c->GPR [ 1 ].u << " r2=" << c->GPR [ 2 ].u;
	
	// output register values before
	cout << dec << "\nBefore: r0=" << c->GPR [ 0 ].u << " r1=" << c->GPR [ 1 ].u << " r2=" << c->GPR [ 2 ].u;
	
	cout << "\nExecuting test instruction #2 (ADDIU r1, r0, 3)...";
	
	// execute test instruction
	inst.Value = ( 9 << 26 ) + ( 1 << 16 ) + 3;
	Execute::ExecuteInstruction ( inst );
	
	// output register values after
	cout << dec << "\nAfter: r0=" << c->GPR [ 0 ].u << " r1=" << c->GPR [ 1 ].u << " r2=" << c->GPR [ 2 ].u;
	
	
	cin.ignore ();
	return 0;
}

