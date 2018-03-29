


#ifndef _DVDIMAGE_H_
#define _DVDIMAGE_H_


#include <stdio.h>

#include "types.h"

namespace DiskImage
{

	class DVDImage
	{
	public:
	
		// each DVD sector is 2418 bytes
		
		// an iso file ALWAYS has 2048 bytes per sector
		// or that might be 2064
		// or that might be 2366
		
		
		static const u32 c_SectorSize = 2048;
		
		
		FILE *image;
		
		
		// open a disk image for reading
		// returns true if opened successfully, returns false otherwise
		bool OpenDiskImage ( char* DiskImagePath );
		
		// close the currently open disk image
		// returns true if closed successfully, returns false otherwise
		bool CloseDiskImage ();
		
		// seeks to time and sector offset
		//bool SeekTime ( s32 Minutes, s32 Seconds, s32 Sector );
		
		// seeks to the absolute sector number on dvd
		void SeekSector ( u64 LogicalSectorNumber );
		
		// reads just the data in sector and returns the number of bytes read, for a dvd this is always 2048 bytes
		int ReadData ( u8* Data );
		
		// reads entire 2418-byte sector
		// can't read the ENTIRE DVD sector from an iso file (meaning including the ID and error correction codes)
		//void ReadSector ( u8* Sector );
		
		//static u32 ConvertBCDToDec ( u8 BCD );
		
	};

}

#endif


