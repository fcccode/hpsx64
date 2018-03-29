
// enable inline debugging
//#define INLINE_DEBUG


#include <iostream>


#include "Debug.h"


#include "R3000A.h"
#include "MipsOpcode.h"

#include "PS1_Timer.h"
#include "PS1_CD.h"
#include "PS1_Intc.h"
#include "PS1_SPU.h"


using namespace std;
using namespace Playstation1;

string MipsCodeFile;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
	// create inline debugger
	Debug::Log debug;
	debug.Create ( "MainLog.txt" );
	
	// set priority for process
	//SetPriorityClass ( GetCurrentProcess (), REALTIME_PRIORITY_CLASS );

#ifdef INLINE_DEBUG
	debug << "calling WindowClass::Register\r\n";
#endif

	cout << "Registering window class.\n";
	WindowClass::Register ( hInstance, "testR3000A" );
	
	long Counter = 0;
	

#ifdef INLINE_DEBUG
	debug << "Creating CD.\r\n";
#endif

	CD *thecd = new CD ();

#ifdef INLINE_DEBUG
	debug << "Creating DMA.\r\n";
#endif

	Dma *thedma = new Dma ();

#ifdef INLINE_DEBUG
	debug << "Creating INTC.\r\n";
#endif

	Intc *theintc = new Intc ();
	
#ifdef INLINE_DEBUG
	debug << "Creating SPU.\r\n";
#endif

	SPU *thespu = new SPU ();

#ifdef INLINE_DEBUG
	debug << "Creating GPU.\r\n";
#endif

	GPU *thegpu = new GPU ();

#ifdef INLINE_DEBUG
	debug << "Creating timers.\r\n";
#endif

	Timers *thetimers = new Timers ( thegpu );

#ifdef INLINE_DEBUG
	debug << "Creating new data bus object.\r\n";
#endif

	Playstation1::DataBus *db = new Playstation1::DataBus ( thedma, thecd, thegpu, theintc, NULL, NULL, NULL, thespu, thetimers );

	//R3000A::Assembler* a = new R3000A::Assembler ();
	
#ifdef INLINE_DEBUG
	debug << "Creating new R3000A CPU object\r\n";
#endif

	cout << "Creating R3000A CPU object.\n";
	R3000A::Cpu* c = new R3000A::Cpu ( db );
	
	// set pointer in databus for debugging
	db->DebugPC = &(c->PC);

	// must enable debugging to show the debug window
	
#ifdef INLINE_DEBUG
	debug << "Enabling debugging\r\n";
#endif

	cout << "Enabling debugging.\n";
	c->EnableDebugging ();

	// Prompt for R3000A code to test
#ifdef INLINE_DEBUG
	debug << "Prompting for code file to test.\r\n";
#endif

	cout << "Prompting for code file...\n";
	MipsCodeFile = c->DebugWindow->ShowFileOpenDialog ();
	

	

	
	cout << "Enabled Debugging.\n";
	
	c->UpdateDebugWindow ();
	
	cout << "Updated debug window. Will execute instruction next.\n";

	// load the code into processor
	cout << "Loading test code.\n";
	if ( c->LoadTestProgram ( MipsCodeFile.c_str() ) )
	{
		// run the test code
		
#ifdef INLINE_DEBUG
		debug << "Running test code.\r\n";
#endif

		cout << "Running test code.\n";
		c->RunTestProgram ();
	}

	c->UpdateDebugWindow ();

#ifdef INLINE_DEBUG
	debug << "Updated debug window.\r\n";
#endif

	cout << "Updated debug window.\n";
	

	//while ( WindowClass::DoEvents () );
	cin.ignore();
	
	//delete a;
	delete c;
	
	return 0;
}

