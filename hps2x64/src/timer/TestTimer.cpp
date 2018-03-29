
#include "ps2_system.h"
#include <iostream>

using namespace Playstation2;
using namespace std;

void OutputTimerData ( System *s )
{
	// output data for the three ps2 timers
	for ( int i = 0; i < 3; i++ )
	{
		// update the timer
		cout << "\nTimer#" << i;
		cout << "\nValue=" << dec << s->_TIMERS.GetValue ( i );
		cout << "\nMode=" << hex << s->_TIMERS.GetMode ( i );
		cout << "\nComp=" << dec << s->_TIMERS.GetComp ( i );
		
		// get next event
		s->_TIMERS.Get_NextEvent ( i );
		cout << "\nNextEventCycle=" << dec << s->_TIMERS.TheTimers [ i ].NextEvent_Cycle;
		cout << "\n";
	}
}

int main ()
{

	cout << "\nTest: creating new ps2 system object";

	// create playstation 2 system object
	System *s = new System ();
	
	cout << "\nTest: starting up the system";
	
	// startup the system
	s->Start ();
	
	cout << "\nTest: running the system for a few cycles";
	
	// enable timer 1
	s->_TIMERS.SetMode ( 0, 0x83 );
	s->_TIMERS.SetMode ( 1, 0x81 );
	s->_TIMERS.SetMode ( 2, 0xc0 );
	
	s->_TIMERS.SetComp ( 2, 10 );
	
	OutputTimerData ( s );
	
	// run the system for a few cycles
	for ( int i = 0; i < 13; i++ ) s->Run ();
	
	cout << "\nTest: displaying timer data";
	
	OutputTimerData ( s );

	cin.ignore();
	delete s;
	return 0;
}


