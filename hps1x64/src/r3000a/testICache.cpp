
// enable inline debugging
//#define INLINE_DEBUG


#include <iostream>


#include "Debug.h"



#include "R3000A_ICache.h"

using namespace std;
//using namespace Playstation1;

//string MipsCodeFile;

unsigned long Data [ 4 ] = { 1, 2, 3, 4 };

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
	// create inline debugger
	//Debug::Log debug;
	//debug.Create ( "MainLog.txt" );
	
	cout << "Initializing ICache\n";
	
	R3000A::ICache_Device ICache;
	
	cout << "Is address 0 a cache hit? " << ICache.isCacheHit ( 0 ) << "\n";
	
	cout << "Writing data into cache\n";
	ICache.GetCacheLinePtr ( 0 ) [ 0 ] = Data [ 0 ];
	ICache.GetCacheLinePtr ( 0 ) [ 1 ] = Data [ 1 ];
	ICache.GetCacheLinePtr ( 0 ) [ 2 ] = Data [ 2 ];
	ICache.GetCacheLinePtr ( 0 ) [ 3 ] = Data [ 3 ];
	
	cout << "Validating cache line\n";
	ICache.ValidateCacheLine ( 0 );

	cout << "Is address 0 a cache hit? " << ICache.isCacheHit ( 0 ) << "\n";
	cout << "Is address 4 a cache hit? " << ICache.isCacheHit ( 4 ) << "\n";
	cout << "Is address 16 a cache hit? " << ICache.isCacheHit ( 16 ) << "\n";
	
	cout << "Address 0 = " << ICache.Read ( 0 ) << "\n";
	cout << "Address 8 = " << ICache.Read ( 8 ) << "\n";
	
	//while ( WindowClass::DoEvents () );
	cin.ignore();
	
	
	return 0;
}

