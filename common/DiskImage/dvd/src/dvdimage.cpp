


#include "cdimage.h"


using namespace DiskImage;


bool DVDImage::OpenDiskImage ( char* DiskImagePath )
{
	image = fopen64 ( DiskImagePath, "r" );
	
	if ( !image ) return false;
	
	return true;
}

bool DVDImage::CloseDiskImage ()
{
	if ( !fclose ( image ) ) return true;
	
	return false;
}


void DVDImage::SeekSector ( u64 LogicalSectorNumber )
{
	fseeko64 ( image, ( LogicalSectorNumber ) * c_SectorSize, SEEK_SET );
}

int DVDImage::ReadData ( u8* Data )
{
	fread ( Data, c_SectorSize, 1, image );
	return 2048;
}






