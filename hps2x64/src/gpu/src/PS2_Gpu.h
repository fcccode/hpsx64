/*
	Copyright (C) 2012-2020

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


#ifndef _PS2_GPU_H_
#define _PS2_GPU_H_

#include "types.h"
#include "Debug.h"

#include "WinApiHandler.h"
//#include "GNUAsmUtility_x64.h"

#include "PS1_Gpu.h"


//#define DRAW_MULTIPLE_PIXELS

//#include "emmintrin.h"

//#define _ENABLE_SSE2_TRIANGLE_MONO
//#define _ENABLE_SSE2_TRIANGLE_GRADIENT
//#define _ENABLE_SSE2_RECTANGLE_MONO


#ifdef _ENABLE_SSE2

// need to include this file to use SSE2 intrinsics
//#include "emmintrin.h"
//#include "smmintrin.h"

#endif


//using namespace x64Asm::Utilities;



#define ENABLE_DATA_STRUCTURE


#define ENABLE_ALPHA_POINT
#define ENABLE_ALPHA_LINE_MONO
#define ENABLE_ALPHA_LINE_GRADIENT
#define ENABLE_ALPHA_RECTANGLE
#define ENABLE_ALPHA_SPRITE
#define ENABLE_ALPHA_TRIANGLE_MONO
#define ENABLE_ALPHA_TRIANGLE_GRADIENT
#define ENABLE_ALPHA_TRIANGLE_TEXTURE



//#define ENABLE_KEYCOLOR

#define ENABLE_DEPTH_TEST
#define ENABLE_DEST_ALPHA_TEST
#define ENABLE_SRC_ALPHA_TEST
#define ENABLE_FOG


//#define ENABLE_ZREAD_SHIFT
//#define ENABLE_ZSTORE_SHIFT


#define USE_CBP_SINGLEVALUE

//#define VERBOSE_ZBUFFER

// will comment this out as needed
//#define VERBOSE_RGBONLY


// the new way or the old way?
//#define OLD_PATH1_ARGS



namespace Playstation2
{

	class GPU
	{
	
		static Debug::Log debug;
		
		static WindowClass::Window *DisplayOutput_Window;
		static WindowClass::Window *FrameBuffer_DebugWindow;
	
	public:
	
		static GPU *_GPU;
		
		
		//////////////////////////
		//	General Parameters	//
		//////////////////////////
		
		// where the registers start at
		static const long Regs_Start = 0x10003000;
		
		// where the registers end at
		static const long Regs_End = 0x100037f0;
	
		// distance between groups of registers
		static const long Reg_Size = 0x10;
		
		// need to pack lots of info into the structure for debugging and read/write of hardware registers
		struct HW_Register
		{
			bool ReadOK;
			bool WriteOK;
			bool Unknown;
			char* Name;
			u32 Address;
			u32 SizeInBytes;
			u32* DataPtr;
		};
		
		HW_Register Registers [ Regs_End - Regs_Start + Reg_Size ];

		// the revision and id of the GPU
		static const u32 c_ulGS_Revision = 0x1b;
		static const u32 c_ulGS_ID = 0x55;
		
		static const char* GIFRegNames [ 11 ];
		static const char* GPUReg0Names [ 15 ];
		static const char* GPUReg1Names [ 9 ];
		static const char* GPURegsGp_Names [ 0x63 ];
		
		static const char* PixelFormat_Names [ 64 ];
		static const char* TransferDir_Names [ 4 ];
		
		// GPU Clock Speed in Hertz
		//static const long c_iClockSpeed = 53222400;
		//static const double c_dClockSpeed = 53222400.0L;
		static const unsigned long long c_llClockSpeed1 = 147456000;
		static const unsigned long long c_llClockSpeed2 = 149500000;

		static const double c_dClockSpeed1 = 147456000.0L;
		static const double c_dClockSpeed2 = 149500000.0L;
		
		
		// the number of gpu cycles for every cpu cycle or vice versa
		static const double c_dGPUPerCPU_Cycles = ( 1.0L / 2.0L );
		static const double c_dCPUPerGPU_Cycles = ( 2.0L / 1.0L );
		
		static const u64 c_BIAS = ( 1ULL << 31 );
		static const u64 c_BIAS24 = ( 1ULL << 23 );
		static const u64 c_BIAS16 = ( 1ULL << 15 );
		static inline s64 _Round ( s64 FixedPointValue )
		{
			return ( FixedPointValue + c_BIAS );
		}
		
		static inline s64 _Round24 ( s64 FixedPointValue )
		{
			return ( FixedPointValue + c_BIAS24 );
		}
		
		static inline s64 _Round16 ( s64 FixedPointValue )
		{
			return ( FixedPointValue + c_BIAS16 );
		}
		
		static inline u64 _Abs ( s64 Value )
		{
			return ( ( Value >> 63 ) ^ Value ) - ( Value >> 63 );
		}
		
		static inline u32 _Abs ( s32 Value )
		{
			return ( ( Value >> 31 ) ^ Value ) - ( Value >> 31 );
		}
		
		
		static inline u16 SwapEndian16 ( u16 Value )
		{
			return ( Value << 8 ) | ( Value >> 8 );
		}


		static inline u32 SwapEndian32 ( u32 Value )
		{
			return ( Value >> 24 ) | ( Value << 24 ) | ( ( Value << 8 ) & 0xff0000 ) | ( ( Value >> 8 ) & 0xff00 );
		}
		
		
		// appears that different pixels formats are stored differently
		// need to convert the word addresses offsets
		
		// convert word address offset for 32-bit pixels
		// returns word offset (offset into 32-bit values)
		// must use different function for Z-buffer, this one is just for pixels
		static inline u32 CvtAddrPix32 ( u32 x, u32 y, u32 width )
		{
			u32 Offset;
			u32 Temp, TempX, TempY;
			
			// pixels
			// 1 x-bit, 1 y-bit, 2 x-bits
			// columns
			// 2 y-bits
			// blocks
			// 1 x-bit, 1 y-bit, 1 x-bit, 1 y-bit, 1 x-bit 		//2 x-bits, 1 y-bit
			// then add in the rest times width plus x etc
			// xyxxyyxyxxy (11-bits total)
			// xyxyxyyxxyx
			
			// 64x32 pixels = 2048 pixels -> 11-bits per page, 6 x-bits and 5 y-bits -> 2048 pixels x 4 bytes/pixel = 8192 bytes
			
			//Offset = ( x & 0x1 ) | ( ( y & 0x1 ) << 1 ) | ( ( x & 0x6 ) << 1 ) | ( ( y & 0x6 ) << 3 ) | ( ( x & 0x8 ) << 3 ) | ( ( y & 0x8 ) << 4 ) | ( ( x & 0x10 ) << 4 ) | ( ( y & 0x10 ) << 5 ) | ( ( x & 0x20 ) << 5 );
			Offset = ( x & 0x1 ) | ( ( ( y & 0x1 ) | ( x & 0x6 ) ) << 1 ) | ( ( ( y & 0x6 ) | ( x & 0x8 ) ) << 3 ) | ( ( ( y & 0x8 ) | ( x & 0x10 ) ) << 4 ) | ( ( ( y & 0x10 ) | ( x & 0x20 ) ) << 5 );
			
			// add in the rest of the x-bits
			Offset |= ( ( x & ~0x3f ) << 5 );
			
			// add in the rest of the bits for the y
			// *** TODO *** make sure the shift value is correct
			// 2048 pixels per page, already used 5 y-bits, width_shift starts at 1 which means 1 page wide
			Offset += ( ( y & ~0x1f ) * width );
			
			return Offset;
		}


		// convert word address offset for 16-bit pixels
		// returns halfword offset (offset into 16-bit values)
		static inline u32 CvtAddrPix16 ( u32 x, u32 y, u32 width )
		{
			u32 Offset;
			
			// pixels
			// 1 x-bit (bit 3 of x), 1 x-bit (bit 0 of x), 1 y-bit, 2 x-bits (bits 1 and 2 of x)
			// columns
			// 2 y-bits
			// blocks
			// 1 y-bit, 1 x-bit, 1 y-bit, 1 x-bit, 1 y-bit
			// then add in the rest times width plus x etc
			// yxyxyyyxxyxx
			
			// 64x64 pixels = 4096 pixels -> 12-bits per page, 6 x-bits and 6 y-bits
			
			// first bit of y goes into second bit of x
			//Offset = ( ( x & 8 ) >> 3 ) | ( ( x & 1 ) << 1 ) | ( ( y & 1 ) << 2 ) | ( ( x & 6 ) << 2 ) | ( ( y & 0xe ) << 4 ) | ( ( x & 0x10 ) << 4 ) | ( ( y & 0x10 ) << 5 )
			//		| ( ( x & 0x20 ) << 5 ) | ( ( y & 0x20 ) << 6 );
			Offset = ( ( x & 8 ) >> 3 ) | ( ( x & 1 ) << 1 ) | ( ( ( y & 1 ) | ( x & 6 ) ) << 2 ) | ( ( ( y & 0xe ) | ( x & 0x10 ) ) << 4 ) | ( ( ( y & 0x10 ) | ( x & 0x20 ) ) << 5 ) | ( ( y & 0x20 ) << 6 );
			
			// add in the rest of the x-bits
			Offset |= ( ( x & ~0x3f ) << 6 );
			
			// add in the rest of the bits for the y
			// *** TODO *** make sure the shift value is correct
			// 4096 pixels per page, already used 6 y-bits, width_shift starts at 1 which means 1 page wide
			Offset += ( ( y & ~0x3f ) * width );
			
			return Offset;
		}


		// convert word address offset for 16-bit pixels
		// returns halfword offset (offset into 16-bit values)
		static inline u32 CvtAddrPix16S ( u32 x, u32 y, u32 width )
		{
			u32 Offset;
			
			// pixels
			// 1 x-bit (bit 3 of x), 1 x-bit (bit 0 of x), 1 y-bit, 2 x-bits (bits 1 and 2 of x)
			// columns
			// 2 y-bits
			// blocks
			// 1 y-bit, 1 x-bit, 1 y-bit (32's bit), 1 y-bit (16's bit), 1 x-bit
			// then add in the rest times width plus x etc
			// yxyxyyyxxyxx
			// xyyxyyyxxyxx
			
			// 64x64 pixels = 4096 pixels -> 12-bits per page, 6 x-bits and 6 y-bits
			
			// first bit of y goes into second bit of x
			//Offset = ( ( x & 8 ) >> 3 ) | ( ( x & 1 ) << 1 ) | ( ( y & 1 ) << 2 ) | ( ( x & 6 ) << 2 ) | ( ( y & 0xe ) << 4 ) | ( ( x & 0x10 ) << 4 ) | ( ( y & 0x20 ) << 4 )
			//		| ( ( y & 0x10 ) << 6 ) | ( ( x & 0x20 ) << 6 );
			Offset = ( ( x & 8 ) >> 3 ) | ( ( x & 1 ) << 1 ) | ( ( ( y & 1 ) | ( x & 6 ) ) << 2 ) | ( ( ( y & 0x2e ) | ( x & 0x10 ) ) << 4 ) | ( ( ( y & 0x10 ) | ( x & 0x20 ) ) << 6 );
			
			// add in the rest of the x-bits
			Offset |= ( ( x & ~0x3f ) << 6 );
			
			// add in the rest of the bits for the y
			// *** TODO *** make sure the shift value is correct
			// 4096 pixels per page, already used 6 y-bits, width_shift starts at 1 which means 1 page wide
			Offset += ( ( y & ~0x3f ) * width );
			
			return Offset;
		}


		// convert word address offset for 8-bit pixels
		// returns byte offset (offset into 8-bit values)
		static inline u32 CvtAddrPix8 ( u32 x, u32 y, u32 width )
		{
			u32 Offset;
			
			// pixels
			// 2's bit of y, 8's bit of x, 1 x-bit, 1 y-bit, 2 x-bits (where 4th x-bit here is xor'ed with 2's y-bit and 4's y-bit), 
			// columns
			// 2 y-bits
			// blocks
			// 1 x-bit, 1 y-bit, 1 x-bit, 1 y-bit, 1 x-bit		//2 x-bits, 1 y-bit
			// then add in the rest times width plus x etc
			// yxxyxyyxxyxxy
			// xyxyxyyxxyxxy
			
			// 128x64 pixels = 8192 pixels -> 13-bits per page, 7 x-bits and 6 y-bits
			
			// first bit of y goes into second bit of x
			//Offset = ( ( y & 2 ) >> 1 ) | ( ( x & 8 ) >> 2 ) | ( ( x & 1 ) << 2 ) | ( ( y & 1 ) << 3 ) | ( ( x & 6 ) << 3 ) | ( ( y & 0xc ) << 4 ) | ( ( x & 0x10 ) << 4 ) | ( ( y & 0x10 ) << 5 ) | ( ( x & 0x20 ) << 5 ) | ( ( y & 0x20 ) << 6 ) | ( ( x & 0x40 ) << 6 );
			Offset = ( ( y & 2 ) >> 1 ) | ( ( x & 8 ) >> 2 ) | ( ( x & 1 ) << 2 ) | ( ( ( y & 1 ) | ( x & 6 ) ) << 3 ) | ( ( ( y & 0xc ) | ( x & 0x10 ) ) << 4 ) | ( ( ( y & 0x10 ) | ( x & 0x20 ) ) << 5 ) | ( ( ( y & 0x20 ) | ( x & 0x40 ) ) << 6 );
			
			// xor
			Offset ^= ( ( y & 2 ) << 4 ) ^ ( ( y & 4 ) << 3 );
			
			// add in the rest of the x-bits
			Offset |= ( ( x & ~0x7f ) << 6 );
			
			// add in the rest of the bits for the y
			// *** TODO *** make sure the shift value is correct
			// 8192 pixels per page, already used 6 y-bits, width_shift starts at 1 which means 1 page wide
			// 4096 pixels per half page (64x64 pixels)
			Offset += ( ( y & ~0x3f ) * width );
			
			return Offset;
		}


		// convert word address offset for 4-bit pixels
		// returns nibble offset (offset into 4-bit values)
		static inline u32 CvtAddrPix4 ( u32 x, u32 y, u32 width )
		{
			u32 Offset;
			
			// pixels
			// 2's bit of y, 8's bit of x, 16's bit of x, 1 x-bit, 1 y-bit, 2 x-bits (where 5th x-bit is xor'd with 2's y-bit and 4's y-bit)
			// columns
			// 2 y-bits
			// blocks
			// 1 y-bit, 1 x-bit, 1 y-bit, 1 x-bit, 1 y-bit
			// then add in the rest times width plus x etc
			// yxyxyyyxxyxxxy (14-bits)
			
			// 128x128 pixels = 16384 pixels -> 14-bits per page, 7 x-bits and 7 y-bits
			
			// first bit of y goes into second bit of x
			//Offset = ( ( y & 2 ) >> 1 ) | ( ( x & 8 ) >> 2 ) | ( ( x & 0x10 ) >> 2 ) | ( ( x & 1 ) << 3 ) | ( ( y & 1 ) << 4 ) | ( ( x & 6 ) << 4 ) | ( ( y & 0x1c ) << 5 ) | ( ( x & 0x20 ) << 5 ) | ( ( y & 0x20 ) << 6 ) | ( ( x & 0x40 ) << 6 ) | ( ( y & 0x40 ) << 7 );
			Offset = ( ( y & 2 ) >> 1 ) | ( ( x & 0x18 ) >> 2 ) | ( ( x & 1 ) << 3 ) | ( ( ( y & 1 ) | ( x & 6 ) ) << 4 ) | ( ( ( y & 0x1c ) | ( x & 0x20 ) ) << 5 ) | ( ( ( y & 0x20 ) | ( x & 0x40 ) ) << 6 ) | ( ( y & 0x40 ) << 7 );
			
			// xor
			Offset ^= ( ( y & 2 ) << 5 ) ^ ( ( y & 4 ) << 4 );
			
			// add in the rest of the x-bits
			Offset |= ( ( x & ~0x7f ) << 7 );
			
			// add in the rest of the bits for the y
			// *** TODO *** make sure the shift value is correct
			// 16384 pixels per page, already used 7 y-bits, width_shift starts at 1 which means 1 page wide
			// 64x128 = 8192 pixels per half page
			Offset += ( ( y & ~0x7f ) * width );
			
			return Offset;
		}


		static inline u32 CvtAddrZBuf32 ( u32 x, u32 y, u32 width )
		{
			u32 Offset;
			u32 Temp, TempX, TempY;
			
			// pixels
			// 1 x-bit, 1 y-bit, 2 x-bits
			// columns
			// 2 y-bits
			// blocks
			// 1 x-bit, 1 y-bit, 1 x-bits, 1 y-bit, 1 x-bit (reverse last x-bit and y-bit here)
			// then add in the rest times width plus x etc
			// xyxxyyxyxxy (11-bits total)
			// yxxyxyyxxyx
			
			// 64x32 pixels = 2048 pixels -> 11-bits per page, 6 x-bits and 5 y-bits -> 2048 pixels x 4 bytes/pixel = 8192 bytes
			
			//Offset = ( x & 0x1 ) | ( ( y & 0x1 ) << 1 ) | ( ( x & 0x6 ) << 1 ) | ( ( y & 0x6 ) << 3 ) | ( ( x & 0x8 ) << 3 ) | ( ( y & 0x8 ) << 4 ) | ( ( x & 0x10 ) << 4 ) | ( ( y & 0x10 ) << 5 ) | ( ( x & 0x20 ) << 5 );
			Offset = ( x & 0x1 ) | ( ( ( y & 0x1 ) | ( x & 0x6 ) ) << 1 ) | ( ( ( y & 0x6 ) | ( x & 0x8 ) ) << 3 ) | ( ( ( y & 0x8 ) | ( x & 0x10 ) ) << 4 ) | ( ( ( y & 0x10 ) | ( x & 0x20 ) ) << 5 );
			
			// reverse last two bits for z-buffer
			Offset ^= 0x600;
			
			// add in the rest of the x-bits
			Offset |= ( ( x & ~0x3f ) << 5 );
			
			// add in the rest of the bits for the y
			// *** TODO *** make sure the shift value is correct
			// 2048 pixels per page, already used 5 y-bits, width_shift starts at 1 which means 1 page wide
			Offset += ( ( y & ~0x1f ) * width );
			
			return Offset;
		}


		static inline u32 CvtAddrZBuf16 ( u32 x, u32 y, u32 width )
		{
			u32 Offset;
			
			// pixels
			// 1 x-bit (bit 3 of x), 1 x-bit (bit 0 of x), 1 y-bit, 2 x-bits (bits 1 and 2 of x)
			// columns
			// 2 y-bits
			// blocks
			// 1 y-bit, 1 x-bit, 1 y-bit, 1 x-bit, 1 y-bit
			// then add in the rest times width plus x etc
			// yxyxyyyxxyxx
			
			// 64x64 pixels = 4096 pixels -> 12-bits per page, 6 x-bits and 6 y-bits
			
			// first bit of y goes into second bit of x
			//Offset = ( ( x & 8 ) >> 3 ) | ( ( x & 1 ) << 1 ) | ( ( y & 1 ) << 2 ) | ( ( x & 6 ) << 2 ) | ( ( y & 0xe ) << 4 ) | ( ( x & 0x10 ) << 4 ) | ( ( y & 0x10 ) << 5 )
			//		| ( ( x & 0x20 ) << 5 ) | ( ( y & 0x20 ) << 6 );
			Offset = ( ( x & 8 ) >> 3 ) | ( ( x & 1 ) << 1 ) | ( ( ( y & 1 ) | ( x & 6 ) ) << 2 ) | ( ( ( y & 0xe ) | ( x & 0x10 ) ) << 4 ) | ( ( ( y & 0x10 ) | ( x & 0x20 ) ) << 5 ) | ( ( y & 0x20 ) << 6 );
			
			// reverse last two bits for z-buffer
			Offset ^= 0xc00;
			
			// add in the rest of the x-bits
			Offset |= ( ( x & ~0x3f ) << 6 );
			
			// add in the rest of the bits for the y
			// *** TODO *** make sure the shift value is correct
			// 4096 pixels per page, already used 6 y-bits, width_shift starts at 1 which means 1 page wide
			Offset += ( ( y & ~0x3f ) * width );
			
			return Offset;
		}


		static inline u32 CvtAddrZBuf16S ( u32 x, u32 y, u32 width )
		{
			u32 Offset;
			
			// pixels
			// 1 x-bit (bit 3 of x), 1 x-bit (bit 0 of x), 1 y-bit, 2 x-bits (bits 1 and 2 of x)
			// columns
			// 2 y-bits
			// blocks
			// 1 y-bit, 1 x-bit, 1 y-bit (32's bit), 1 y-bit (16's bit), 1 x-bit
			// then add in the rest times width plus x etc
			// yxyxyyyxxyxx
			// xyyxyyyxxyxx
			
			// 64x64 pixels = 4096 pixels -> 12-bits per page, 6 x-bits and 6 y-bits
			
			// first bit of y goes into second bit of x
			//Offset = ( ( x & 8 ) >> 3 ) | ( ( x & 1 ) << 1 ) | ( ( y & 1 ) << 2 ) | ( ( x & 6 ) << 2 ) | ( ( y & 0xe ) << 4 ) | ( ( x & 0x10 ) << 4 ) | ( ( y & 0x20 ) << 4 )
			//		| ( ( y & 0x10 ) << 6 ) | ( ( x & 0x20 ) << 6 );
			Offset = ( ( x & 8 ) >> 3 ) | ( ( x & 1 ) << 1 ) | ( ( ( y & 1 ) | ( x & 6 ) ) << 2 ) | ( ( ( y & 0x2e ) | ( x & 0x10 ) ) << 4 ) | ( ( ( y & 0x10 ) | ( x & 0x20 ) ) << 6 );
			
			// add in the rest of the x-bits
			Offset |= ( ( x & ~0x3f ) << 6 );
			
			// xor
			Offset ^= 0xc00;
			
			// add in the rest of the bits for the y
			// *** TODO *** make sure the shift value is correct
			// 4096 pixels per page, already used 6 y-bits, width_shift starts at 1 which means 1 page wide
			Offset += ( ( y & ~0x3f ) * width );
			
			return Offset;
		}


		static inline u32 GetBlkNumPix32 ( u32 Offset )
		{
			// 64x32 pixels = 2048 pixels -> 11-bits per page, 6 x-bits and 5 y-bits -> 2048 pixels x 4 bytes/pixel = 8192 bytes
			// block is 8x8 = 64 pixels -> 6 bits per block = 64 pixels * 4 bytes/pixel = 256 bytes
			return ( Offset >> 6 );
		}

		static inline u32 GetBlkNumPix16 ( u32 Offset )
		{
			// block is 16x8 = 128 pixels -> 7 bits per block
			return ( Offset >> 7 );
		}

		static inline u32 GetBlkNumPix8 ( u32 Offset )
		{
			// block is 16x16 = 256 pixels -> 8 bits per block
			return ( Offset >> 8 );
		}

		static inline u32 GetBlkNumPix4 ( u32 Offset )
		{
			// block is 32x16 = 512 pixels -> 9 bits per block
			return ( Offset >> 9 );
		}


		
		// index for the next event
		u32 NextEvent_Idx;
		
		// cycle that the next event will happen at for this device
		u64 NextEvent_Cycle, NextEvent_Cycle_Vsync;

		// the cycle that device is busy until
		u64 BusyUntil_Cycle;
		
		// the last path/current path data is being transferred through
		u32 CurrentPath;
		
		// the amount of data in the fifo
		u32 FifoSize;

		static const int c_iFrameBuffer_DisplayWidth = 640;
		static const int c_iFrameBuffer_DisplayHeight = 960;
		
		static const int c_iScreen_MaxWidth = 1024;
		static const int c_iScreen_MaxHeight = 1024;
		
		// used to buffer pixels before drawing to the screen
		u32 PixelBuffer [ c_iScreen_MaxWidth * c_iScreen_MaxHeight ] __attribute__ ((aligned (32)));
		
		//static const int c_iRamSize = 4194304;
		//u64 RAM [ c_iRamSize >> 3 ] __attribute__ ((aligned (32)));

		
		static const unsigned long c_iRAM_Size = 4194304;
		static const unsigned long c_iRAM_Mask = c_iRAM_Size - 1;
		
		// PS2 VRAM is 4MB
		// ***todo*** show the VRAM when debugging as 1024x1024 if showing a graphical representation, with 32-bit pixels (rgba)
		// ***todo*** also show the frame buffers in 1024x1024 windows, but showing the frame buffers with correct size
		union
		{
			u8 RAM8 [ c_iRAM_Size ] __attribute__ ((aligned (32)));
			u16 RAM16 [ c_iRAM_Size >> 1 ] __attribute__ ((aligned (32)));
			u32 RAM32 [ c_iRAM_Size >> 2 ] __attribute__ ((aligned (32)));
			u64 RAM64 [ c_iRAM_Size >> 3 ] __attribute__ ((aligned (32)));
		};
		
		// temporary fix for disabled buffer updates
		//static u32 RAM32_Shadow [ c_iRAM_Size >> 2 ] __attribute__ ((aligned (32)));
		
		
		// size of the main program window
		static u32 MainProgramWindow_Width;
		static u32 MainProgramWindow_Height;
		
		// maximum width/height of a polygon allowed
		//static const s32 c_MaxPolygonWidth = 1023;
		//static const s32 c_MaxPolygonHeight = 511;

		static u64 Read ( u32 Address, u64 Mask );
		static void Write ( u32 Address, u64 Data, u64 Mask );
		
		void DMA_Read ( u32* Data, int ByteReadCount );
		void DMA_Write ( u32* Data, int ByteWriteCount );
		
		// right now, this says path 3 is not ready if it is masked
		static bool DMA_Write_Ready ();
		
		// these return the amound of data written/read
		static u32 DMA_WriteBlock ( u64* Data, u32 QuadwordCount );
		static u32 DMA_ReadBlock ( u64* Data, u32 QuadwordCount );
		
		// xgkick
#ifdef OLD_PATH1_ARGS
		static void Path1_WriteBlock ( u64* Data );
#else
		static void Path1_WriteBlock ( u64* pMemory64, u32 Address );
#endif
		
		// vif1
		//static void Path2_WriteBlock ( u64* Data, u32 QuadwordCount );
		static void Path2_WriteBlock ( u32* Data, u32 WordCount );
		static void Path2_ReadBlock ( u64* Data, u32 QuadwordCount );
		
		void Start ();

		// returns either vblank interrupt signal, gpu interrupt signal, or no interrupt signal
		void Run ();
		void Reset ();
		
		// need to specify what window to display graphical output to (this should be the main program window)
		// *** TODO *** probably also need to call this whenever the main program window size changes
		void SetDisplayOutputWindow ( u32 width, u32 height, WindowClass::Window* DisplayOutput );
		
		void Draw_Screen ();
		
		void Copy_Buffer ( u32* dstbuf, u32* srcbuf, u32 dstbuffer_width, u32 dstbuffer_height, u32 srcbuffer_width, u32 srcbuffer_height, u32 SrcPixelFormat );
		void Draw_FrameBuffers ();
		

		void GIF_FIFO_Execute ( u64 ull0, u64 ull1 );
		
		void TransferDataLocal ();
		void TransferDataOut32 ( u32* Data, u32 WordCount32 );
		void TransferDataIn32 ( u32* Data, u32 WordCount32 );
		//void TransferDataOut ();
		
		// path1 needs to know when to stop feeding data
		u32 EndOfPacket, Tag_Done;
		
		
		// Priveleged Regsiter structs //
		
		union DISPFB_t
		{
			struct
			{
				// Frame Base Address (Base Address/2048)
				// bits 0-8
				u64 FBP : 9;
				
				// Frame Buffer Width (Width in pixels/64)
				u64 FBW : 6;
				
				// Pixel Format
				// 00000: PSMCT32, 00001: PSMCT24, 00010: PSMCT16, 01010: PSMCT16S, 10010: PS-GPU24
				u64 PSM : 5;
				
				// unknown
				// bits 20-31
				u64 unk0 : 12;
				
				// X pixel position in Buffer
				// bits 32-42
				u64 DBX : 11;
				
				// Y pixel position in Buffer
				// bits 43-53
				u64 DBY : 11;
			};
			
			u64 Value;
		};
		
		
		// this must have to do with how it is displayed on the TV/Monitor
		union DISPLAY_t
		{
			struct
			{
				// X VCK/CLK position in display
				// bits 0-11
				u64 DX : 12;
				
				// Y position in display
				// bits 12-22
				u64 DY : 11;
				
				// Horizontal magnification
				// 0000: means times 1, 0001: means times 2, etc
				// bits 23-26
				u64 MAGH : 4;
				
				// Vertical magnification
				// 0000: means times 1, 0001: means times 2, etc
				// bits 27-28
				u64 MAGV : 2;
				
				// unknown
				// bits 29-31
				u64 unk0 : 3;
				
				// width of display minus one in VCK/CLK
				// bits 32-43
				u64 DW : 12;
				
				// height of display minus one in pixels
				u64 DH : 11;
			};
			
			u64 Value;
		};
		
		
		// Interrupt mask
		union IMR_t
		{
			struct
			{
				// unknown
				// bits 0-7
				u64 unk0 : 8;
				
				// SIGNAL interrupt mask
				// 0: enable, 1: disable; initial value=1
				// bit 8
				u64 SIGMSK : 1;
				
				// FINISH interrupt mask
				// bit 9
				u64 FINISHMSK : 1;
				
				// HSYNC interrupt mask
				// bit 10
				u64 HSMSK : 1;
				
				// VSYNC interrupt mask
				// bit 11
				u64 VSMSK : 1;
				
				// Rectangular Area Write interrupt mask
				// bit 12
				u64 EDWMSK : 1;
			};
			
			u64 Value;
		};
		
		
		union SMODE2_t
		{
			struct
			{
				// Interlace Mode
				// 0: disable, 1: enable
				// bit 0
				u64 INTER : 1;
				
				// field/frame mode
				// 0: Read every OTHER line, 1: Read EVERY line
				u64 FFMD : 1;
				
				// DPMS??
				// 0: on, 1: ready, 2: hold, 3: off
				u64 DPMS : 2;
			};
			
			u64 Value;
		};
		
		
		// GIF TAG
		
		// lower part
		union GIFTag0_t
		{
			struct
			{
				// bits 0-14 - NLOOP - Repeat Count
				u64 NLOOP : 15;
				
				// bit 15 - EOP - End of packet - 1: End of packet
				u64 EOP : 1;
				
				// bits 16-45 - not used
				u64 NotUsed0 : 30;
				
				// bit 46 - PRE - PRIM Field enable - 1: Output PRIM field to PRIM register
				u64 PRE : 1;
				
				// bits 47-57 PRIM
				u64 PRIM : 11;
				
				// bits 58-59 - FLG - 00: Packed, 01: Reglist, 10: Image, 11: Disabled/Image
				u64 FLG : 2;
				
				// bits 60-63 - NREG - Register descriptor - (where 0 means 16)
				u64 REGS : 4;
			};
			
			struct
			{
				u32 Lo;
				u32 Hi;
			};
			
			u64 Value;
		};
		
		// upper part
		union GIFTag1_t
		{
			u8 Regs [ 8 ];
			
			struct
			{
				u32 Lo;
				u32 Hi;
			};
			
			u64 Value;
		};
		
		// note: there is no path zero, so need to add plus one so that data does not get overwritten, even though there are only 3 paths to GPU
		static const int c_iNumPaths = 3 + 1;
		
		bool bFIFOTransfer_InProgress;
		GIFTag0_t GIFTag0 [ c_iNumPaths ];
		GIFTag1_t GIFTag1 [ c_iNumPaths ];
		
		// TransferCount is the current count of the transfer
		// TransferSize is the number of quadwords to be transferred
		u32 ulLoopCount [ c_iNumPaths ], ulRegCount [ c_iNumPaths ], ulNumRegs [ c_iNumPaths ], ulTransferCount [ c_iNumPaths ], ulTransferSize [ c_iNumPaths ];
		u32 PacketInProgress [ c_iNumPaths ];
		
		u32 TransferType;
		enum { TRANSFERTYPE_PACKED = 0, TRANSFERTYPE_REGLIST, TRANSFERTYPE_IMAGE, TRANSFERTYPE_DISABLED };

		// PS2 GIF Registers
		
		// 0x1000 3000 - GIF CTRL
		static const u32 GIF_CTRL = 0x10003000;
		// bit 0 - RST - write 1 for reset
		// bit 3 - PSE - 0: transfer restart, 1: transfer start
		
		union GIF_CTRL_t
		{
			struct
			{
				// bit 0 - RST - write 1 for reset
				u32 RST : 1;
				
				// bit 1-2
				u32 zero0 : 2;
				
				// bit 3 - PSE - 0: transfer restart, 1: transfer start
				u32 PSE : 1;
				
			};
			
			u32 Value;
		};
		
		// 0x1000 3010 - GIF MODE
		static const u32 GIF_MODE = 0x10003010;
		// bit 0 - M3R - Path3 Mask - 0: Mask cancel, 1: Mask
		// bit 2 - IMT - Path3 transfer mode - 0: continuous, 1: in every 8 qwords
		
		union GIF_MODE_t
		{
			struct
			{
				// bit 0 - M3R - Path3 Mask - 0: Mask cancel, 1: Mask
				u32 M3R : 1;
				
				// bit 1
				u32 zero0 : 1;
				
				// bit 2 - IMT - Path3 transfer mode - 0: continuous, 1: in every 8 qwords
				u32 IMT : 1;
			};
			
			u32 Value;
		};
		
		
		// 0x1000 3020 - GIF STAT - GIF Status
		static const u32 GIF_STAT = 0x10003020;
		
		union GIF_STAT_Format
		{
			struct
			{
				// bit 0 - M3R Status - 0: enable, 1: disable
				u32 M3R : 1;
				
				// bit 1 - M3P Status - 0: enable, 1: disable
				u32 M3P : 1;
				
				// bit 2 - IMT Status - 0: continuous, 1: every 8 qwords
				u32 IMT : 1;
				
				// bit 3 - PSE Status - 0: normal, 1: stopped
				u32 PSE : 1;
				
				// bit 4 - zero?
				u32 zero0 : 1;
				
				// bit 5 - IP3 - 0: not interrupted, 1: interrupted path3 transfer
				u32 IP3 : 1;
				
				// bit 6 - P3Q - 0: no request, 1: request to wait for processing in path3
				u32 P3Q : 1;
				
				// bit 7 - P2Q - 0: no request, 1: request to wait for processing in path2
				u32 P2Q : 1;
				
				// bit 8 - P1Q - 0: no request, 1: request to wait for processing in path1
				u32 P1Q : 1;
				
				// bit 9 - OPH - Output path - 0: Idle, 1: Outputting data
				u32 OPH : 1;
				
				// bit 10-11 - APATH - 00: Idle, 01: transferring data via path1, 10: transferring data via path2, 11: transferring data via path3
				u32 APATH : 2;
				
				// bit 12 - DIR - Transfer direction - 0: to GPU, 1: from GPU
				u32 DIR : 1;
				
				// bits 13-23 - zero?
				u32 zero1 : 11;
				
				// bit 24-28 - FQC - Count of items in GIF FIFO (0-16 in qwords)
				u32 FQC : 5;
				
			};
			
			u32 Value;
		};
		
		//GIF_STAT_Format GIF_STAT_Reg;
		
		
		// 0x1000 3040 - GIF TAG0
		// 0x1000 3050 - GIF TAG1
		// 0x1000 3060 - GIF TAG2
		// 0x1000 3070 - GIF TAG3
		// 0x1000 3080 - GIF CNT
		// 0x1000 3090 - GIF P3CNT
		// 0x1000 30a0 - GIF P3TAG


		// 0x100030X0
		union GIFRegs_t
		{
			u32 Regs [ 0x10 ];
			
			struct
			{
				GIF_CTRL_t CTRL;
				GIF_MODE_t MODE;
				GIF_STAT_Format STAT;
				u32 Reserved0;
				u32 TAG0;
				u32 TAG1;
				u32 TAG2;
				u32 TAG3;
				u32 CNT;
				u32 P3CNT;
				u32 P3TAG;
			};
		};
		
		GIFRegs_t GIFRegs;

		
		
		// 0x1000 6000 - GIF FIFO
		static const u32 GIF_FIFO = 0x10006000;
		
		
		
		// 0x120000X0
		//u64 GPURegs0 [ 0x10 ];
		
		// 0x120010X0
		//u64 GPURegs1 [ 0x10 ];
		
		// PS2 GPU Registers
		
		// 0x1200 0000 - PMODE
		static const u32 GPU_PMODE = 0x12000000;
		// 0x1200 0010 - SMODE1 - Sync
		static const u32 GPU_SMODE1 = 0x12000010;
		// 0x1200 0020 - SMODE2 - Sync
		static const u32 GPU_SMODE2 = 0x12000020;
		// 0x1200 0030 - SRFSH - DRAM refresh
		static const u32 GPU_SRFSH = 0x12000030;
		// 0x1200 0040 - SYNCH1 - Sync
		static const u32 GPU_SYNCH1 = 0x12000040;
		// 0x1200 0050 - SYNCH2 - Sync
		static const u32 GPU_SYNCH2 = 0x12000050;
		// 0x1200 0060 - SYNCV - Sync/Start
		static const u32 GPU_SYNCV = 0x12000060;
		
		// 0x120000X0
		union GPURegs0_t
		{
			u64 Regs [ 0x10 ];
			
			struct
			{
				u64 PMODE;
				u64 SMODE1;
				SMODE2_t SMODE2;
				u64 SRFSH;
				u64 SYNCH1;
				u64 SYNCH2;
				u64 SYNCV;
				DISPFB_t DISPFB1;
				DISPLAY_t DISPLAY1;
				DISPFB_t DISPFB2;
				DISPLAY_t DISPLAY2;
				u64 EXTBUF;
				u64 EXTDATA;
				u64 EXTWRITE;
				u64 BGCOLOR;
			};
		};
		
		// 0x120000X0
		GPURegs0_t GPURegs0;
		
		
		// 0x1200 1000 - CSR - GS status
		static const u32 GPU_CSR = 0x12001000;
		// when writing
		
		union CSR_t
		{
			struct
			{
				// bit 0 - SIGNAL - Signal event control - (write) 1: old event is cleared and event is enabled
				// (read) 0: signal event has not been generated, 1: signal event has been generated
				u64 SIGNAL : 1;
				
				// bit 1 - FINISH - Finsih event control - (write) 1: old event is cleared and event is enabled
				// (read) 0: event has not been generated, 1: event has been generated
				u64 FINISH : 1;
				
				// bit 2 - HSINT - Hsync interrupt control - (write) 1: old event is cleared and event is enabled
				// (read) 0: event has not been generated, 1: event has been generated
				u64 HSINT : 1;
				
				// bit 3 - VSINT - Vsync interrupt control - (write) 1: old event is cleared and event is enabled
				// (read) 0: event has not been generated, 1: event has been generated
				u64 VSINT : 1;
				
				// bit 4 - EDWINT - Rectangular area write termination interrupt control - (write) 1: old event is cleared and event is enabled
				// (read) 0: event has not been generated, 1: event has been generated
				u64 EDWINT : 1;
				
				// bits 5-7
				u64 res0 : 3;
				
				// bit 8 - FLUSH - Drawing suspend and FIFO clear
				// (read) 0: not flushed, 1: flushed
				u64 FLUSH : 1;
				
				// bit 9 - RESET - GS Reset - 0: not reset, 1: reset
				u64 RESET : 1;
				
				// bits 10-11
				u64 res1 : 2;
				
				// bit 12 - NFIELD - Output value of NFIELD
				u64 NFIELD : 1;
				
				// bit 13 - FIELD - The field that is being currently displayed
				// (interlace mode) 0: even, 1: odd
				// (non-interlace mode) 0: even, 1: odd
				u64 FIELD : 1;
				
				// bits 14-15 - FIFO - Host FIFO Status - 0: not empty not almost full, 1: empty, 2: almost full
				u64 FIFO : 2;
				
				// bits 16-23 - GS Revision Number
				u64 REV : 8;
				
				// bits 24-31 - GS ID
				u64 ID : 8;
			};
			
			u64 Value;
		};
		
		// 0x1200 1000 - SIGLBLID - GS status
		static const u32 GPU_SIGLBLID = 0x12001080;
		
		union SIGLBLID_t
		{
			struct
			{
				// the value stored into SIGNAL register
				// bits 0-31
				u32 SIGID;
				
				// the value stored into LABEL register
				// bits 32-63
				u32 LBLID;
			};
			
			u64 Value;
		};
		
		
		// 0x120010X0
		static const int c_iGPURegs1_Count = 0x10;
		union GPURegs1_t
		{
			u64 Regs [ c_iGPURegs1_Count ];
			
			struct
			{
				CSR_t CSR;
				IMR_t IMR;
				u64 Reserved0;
				u64 Reserved1;
				u64 BUSDIR;
				u64 Reserved2;
				u64 Reserved3;
				u64 Reserved4;
				SIGLBLID_t SIGLBLID;
			};
		};
		
		// 0x120010X0
		GPURegs1_t GPURegs1;
		
		
		// priveleged GPU registers
		static const int c_iGPURegsPr_Count = 20;
		union GPURegsPr_t
		{
			u64 Regs [ c_iGPURegsPr_Count ];
		
			struct
			{
				u64 PMODE;
				u64 SMODE1;
				u64 SMODE2;
				u64 SRFSH;
				u64 SYNCH1;
				u64 SYNCH2;
				u64 SYNCV;
				u64 DISPLAYFB1;
				u64 DISPLAY1;
				u64 DISPLAYFB2;
				u64 DISPLAY2;
				u64 EXTBUF;
				u64 EXTDATA;
				u64 EXTWRITE;
				u64 BGCOLOR;
				
				u64 CSR;
				u64 IMR;
				u64 BUSDIR;
				u64 SIGLBLID;
				
				u64 Reserved;
			};
		};
		
		GPURegsPr_t GPURegsPr;
		
		
		
		
		
		union ALPHA_t
		{
			struct
			{
				// input color A source (00: Cs [source]; 01: Cd [destination]; 10: 0; 11: Reserved)
				u64 A : 2;
				
				// input color B source (00: Cs [source]; 01: Cd [destination]; 10: 0; 11: Reserved)
				u64 B : 2;
				
				// input color A source (00: As [source]; 01: Ad [destination]; 10: FIX; 11: Reserved)
				u64 C : 2;
				
				// input color A source (00: Cs [source]; 01: Cd [destination]; 10: 0; 11: Reserved)
				u64 D : 2;
				
				// unknown
				u64 unk : 24;
				
				// FIX value
				u64 FIX : 8;
			};
			
			u64 Value;
		};


		union TEXA_t
		{
			struct
			{
				// As value for RGB24 or RGB16 with alpha as zero
				u64 TA0 : 8;
				
				// unknown
				u64 unk0 : 7;
				
				// texture alpha for RGB24 (0: RGB24 is opaque when pixel is zero, 1: RGB24 is transparent when pixel is zero)
				u64 AEM : 1;
				
				// unknown
				u64 unk1 : 16;
				
				// As value alpha is 1 in RGB16 format
				u64 TA1 : 8;
			};
			
			u64 Value;
		};
		
		union FRAME_t
		{
			struct
			{
				// frame buffer pointer (word address/2048) (word size is 32-bits)
				u64 FBP : 9;
				
				// unknown
				u64 unk0 : 7;
				
				// frame buffer width (number of pixels/64) (values go from 1 to 32 ONLY)
				u64 FBW : 6;
				
				// unknown
				u64 unk1 : 2;
				
				// buffer pixel format
				// 000000: PSMCT32, 000001: PSMCT24, 000010: PSMCT16, 001010: PSMCT16S
				// 110000: PSMZ32, 110001: PSMZ24, 110010: PSMZ16, 111010: PSMZ16S
				u64 PSM : 6;
				
				// unknown
				u64 unk2 : 2;
				
				// frame buffer drawing mask
				// 0: bit is updated in buffer, 1: bit is not updated in buffer
				u64 FBMSK : 32;
			};
			
			u64 Value;
		};



		union BITBLTBUF_t
		{
			struct
			{
				// source buffer ptr (word address/64) (where words are 32-bit words)
				u64 SBP : 14;
				
				// unknown
				u64 unk0 : 2;
				
				// source buffer width (number of pixels/64)
				u64 SBW : 6;
				
				// unknown
				u64 unk1 : 2;
				
				// source pixel format
				// 000000: PSMCT32, 000001: PSMCT24, 000010: PSMCT16, 001010: PSMCT16S, 010011: PSMT8, 010100: PSMT4, 011011: PSMT8H,
				// 100100: PSMT4HL, 101100: PSMT4HH, 110000: PSMZ32, 110001: PSMZ24, 110010: PSMZ16, 111010: PSMZ16S
				u64 SPSM : 6;
				
				// unknown
				u64 unk2 : 2;
				
				// destination buffer pointer (word address/64)
				u64 DBP : 14;
				
				// unknown
				u64 unk3 : 2;
				
				// destination buffer width (number of pixels/64)
				u64 DBW : 6;
				
				// unknown
				u64 unk4 : 2;
				
				// destination pixel format
				u64 DPSM : 6;
				
				// unknown
				u64 unk5 : 2;
			};
			
			u64 Value;
		};
		
		
		union TRXDIR_t
		{
			struct
			{
				// 00: Host->GPU, 01: GPU->Host, 10: GPU->GPU, 11: none
				u64 XDIR : 2;
			};

			u64 Value;
		};
		
		
		union TRXPOS_t
		{
			struct
			{
				// x-coord to start transfer at for source (upper-left corner)
				u64 SSAX : 11;
				
				// unknown
				u64 unk0 : 5;
				
				// y-coord to start transfer at for source (upper-left corner)
				u64 SSAY : 11;
				
				// unknown
				u64 unk1 : 5;
				
				// x-coord to start transfer at for dest (upper-left corner)
				u64 DSAX : 11;
				
				// unknown
				u64 unk2 : 5;
				
				// y-coord to start transfer at for dest (upper-left corner)
				u64 DSAY : 11;
				
				// transmission method
				// 00: upper-left->lower-right, 01: lower-left->upper-right, 10: upper-right->lower-left, 11: lower-right->upper-left
				u64 DIR : 2;
				
				// unknown
				u64 unk3 : 3;
			};
			
			u64 Value;
		};
		
		
		union TRXREG_t
		{
			struct
			{
				// width of data to transfer in pixels
				u64 RRW : 12;
				
				// unknown
				u64 unk0 : 20;
				
				// height of data to transfer in pixels
				u64 RRH : 12;
				
				// unknown
				u64 unk1 : 20;
			};
			
			u64 Value;
		};
		
		
		union ZBUF_t
		{
			struct
			{
				// z-buffer pointer (word address/2048)
				u64 ZBP : 9;
				
				// unknown
				u64 unk0 : 15;
				
				// z-value format
				// 0000: PSMZ32, 0001: PSMZ24, 0010: PSMZ16, 1010: PSMZ16S
				u64 PSM : 4;
				
				// unknown
				u64 unk1 : 4;
				
				// z-value drawing mask
				// 0: z-buffer is updated, 1: z-buffer is not updated, even if depth test is passed
				u64 ZMSK : 1;
			};
			
			u64 Value;
		};
		
		
		union XYOFFSET_t
		{
			struct
			{
				// x/y offsets are unsigned
				// x-offset (12.4 fixed point)
				u64 OFX : 16;
				
				// unknown
				u64 unk0 : 16;
				
				// y-offset (12.4 fixed point)
				u64 OFY : 16;
				
				// unknown
				u64 unk1 : 16;
			};
			
			u64 Value;
		};
		
		// these look to be positive-only values
		union SCISSOR_t
		{
			struct
			{
				// x-coord for upper-left corner of drawing window
				u64 SCAX0 : 11;
				
				// unknown
				u64 unk0 : 5;
				
				// x-coord for lower-right corner of drawing window
				u64 SCAX1 : 11;
				
				// unknown
				u64 unk1 : 5;
				
				// y-coord for upper-left corner of drawing window
				u64 SCAY0 : 11;
				
				// unknown
				u64 unk2 : 5;
				
				// y-coord for lower-right corner of drawing window
				u64 SCAY1 : 11;
				
				// unknown
				u64 unk3 : 5;
			};
			
			u64 Value;
		};
		
		
		union PRIM_t
		{
			struct
			{
				// type of primitive
				// 000: pixel, 001: line, 010: line strip, 011: triangle, 100: triangle strip, 101: triangle fan, 110: sprite, 111: reserved
				u64 PRIM : 3;
				
				// shading
				// 0: flat, 1: gouraud
				u64 IIP : 1;
				
				// texture mapping
				// 0: off, 1: on
				u64 TME : 1;
				
				// fogging
				// 0: off, 1: on
				u64 FGE : 1;
				
				// alpha blending
				// 0: off, 1: on
				u64 ABE : 1;
				
				// single-pass anti-aliasing
				// 0: off, 1: on
				u64 AA1 : 1;
				
				// texture coordinate specification
				// 0: STQ (enables perspective mapping), 1: UV (no perspective mapping like PS1)
				u64 FST : 1;
				
				// context
				// 0: using context 0, 1: using context 1
				u64 CTXT : 1;
				
				// fragment value control (RGBAFSTQ change via DDA) ??
				// 0: unfixed (normal), 1: fixed
				u64 FIX : 1;
			};
			
			u64 Value;
		};
		
		
		union TEST_t
		{
			struct
			{
				// Test Alpha - bit 0
				// 0: off, 1: on
				u64 ATE : 1;
				
				// Alpha test type - bits 1-3
				// 000: always fail, 001: always pass, 010: Alpha<AREF pass, 011: Alpha<=AREF pass,
				// 100: Alpha=AREF pass, 101: Alpha>=AREF pass, 110: Alpha>AREF pass, 111: Alpha!=AREF pass
				u64 ATST : 3;
				
				// Alpha compare value - bits 4-11
				u64 AREF : 8;
				
				// what to do when alpha test failed - bits 12-13
				// 00: neither frame buffer nor z-buffer updated, 01: only update frame buffer, 10: only update z-buffer, 11: only update frame buffer RGB
				u64 AFAIL : 2;
				
				// destination alpha test - bit 14
				// 0: off, 1: on
				u64 DATE : 1;
				
				// destination alpha test type - bit 15
				// 0: pixels with destination alpha of 0 pass, 1: pixels with destination alpha of 1 pass
				u64 DATM : 1;
				
				// z-buffer depth test - bit 16
				// 0: off, 1: on
				u64 ZTE : 1;
				
				// depth test type - bits 17-18
				// 00: all pixels fail, 01: all pixels pass, 10: Z>=ZBUFFER pass, 11: Z>ZBUFFER pass
				u64 ZTST : 2;
			};
			
			u64 Value;
		};
		
		
		union XYZ_t
		{
			struct
			{
				// x/y coords are unsigned
				// 12.4 fixed point
				u64 X : 16;
				
				// 12.4 fixed point
				u64 Y : 16;
				
				u64 Z : 32;
			};
			
			u64 Value;
		};
		
		union XYZF_t
		{
			struct
			{
				// x/y coords are unsigned
				// 12.4 fixed point
				u64 X : 16;
				
				// 12.4 fixed point
				u64 Y : 16;
				
				u64 Z : 24;
				
				u64 F : 8;
			};
			
			u64 Value;
		};
		
		union RGBAQ_t
		{
			struct
			{
				u64 R : 8;
				u64 G : 8;
				u64 B : 8;
				u64 A : 8;
				u64 Q : 32;
			};
			
			struct
			{
				u32 RGBAQ;
				float fQ;
			};
			
			u64 Value;
		};
		
		union ST_t
		{
			struct
			{
				u32 S;
				u32 T;
			};
			
			struct
			{
				float fS;
				float fT;
			};
			
			u64 Value;
		};
		
		union UV_t
		{
			struct
			{
				// 10.4 fixed point format
				u64 U : 14;
				
				// padding
				u64 padding0 : 2;
				
				// 10.4 fixed point format
				u64 V : 14;
			};
			
			u64 Value;
		};
		
		
		
		union TEXCLUT_t
		{
			struct
			{
				// width of color lookup table (units: pixels/64)
				u64 CBW : 6;
				
				// x position of color lookup table (units: pixels/64)
				u64 COU : 6;
				
				// x position of color lookup table (units: pixels)
				u64 COV : 10;
			};
			
			u64 Value;
		};
		
		
		union TEX0_t
		{
			struct
			{
				// texture base pointer (32-bit word address/64)
				u64 TBP0 : 14;
				
				// texture buffer width (texels/64)
				u64 TBW0 : 6;
				
				// texture format
				// 000000: PSMCT32, 000001: PSMCT24, 000010: PSMCT16, 001010: PSMCT16S, 010011: PSMT8, 010100: PSMT4, 011011: PSMT8H,
				// 100100: PSMT4HL, 101100: PSMT4HH, 110000: PSMZ32, 110001: PSMZ24, 110010: PSMZ16, 111010: PSMZ16S
				u64 PSM : 6;
				
				// texture width (2^x)
				u64 TW : 4;
				
				// texture height (2^x)
				u64 TH : 4;
				
				// texture color components type
				// 0: RGB, 1: RGBA
				u64 TCC : 1;
				
				// texture function
				// 00: modulate, 01: decal, 10: highlight1, 10: highlight2
				u64 TFX : 2;
				
				// color LUT buffer base pointer (32-bit word address/64)
				u64 CBP : 14;
				
				// color LUT pixel format
				// 0000: PSMCT32, 0010: PSMCT16, 1010: PSMCT16S
				u64 CPSM : 4;
				
				// color LUT mode
				// 0: CSM1, 1: CSM2
				u64 CSM : 1;
				
				// color LUT entry offset (offset/16)
				u64 CSA : 5;
				
				// color LUT buffer load mode
				u64 CLD : 3;
			};
			
			u64 Value;
		};
		
		// TEX1 register appears to be MIPMAP information
		
		// TEX2 register has a subset of the items in TEX0
		
		
		union CLAMP_t
		{
			struct
			{
				// horizontal wrap mode (00: repeat, 01: clamp, 10: region clamp, 11: region repeat)
				u64 WMS : 2;
				
				// vertical wrap mode (00: repeat, 01: clamp, 10: region clamp, 11: region repeat)
				u64 WMT : 2;
				
				// horizontal clamp lower-limit/umsk
				u64 MINU : 10;
				
				// horizontal clamp upper-limit/ufix
				u64 MAXU : 10;
				
				// vertical clamp lower-limit/vmsk
				u64 MINV : 10;
				
				// vertical clamp upper-limit/vfix
				u64 MAXV : 10;
			};
		
			u64 Value;
		};
		
		// general purpose GPU registers
		static const int c_iGPURegsGp_Count = 0x63;
		union GPURegsGp_t
		{
			u64 Regs [ c_iGPURegsGp_Count ];
		
			struct
			{
				PRIM_t PRIM;	// 0x00
				RGBAQ_t RGBAQ;
				ST_t ST;
				UV_t UV;
				XYZF_t XYZF2;
				XYZ_t XYZ2;
				TEX0_t TEX0_1;
				TEX0_t TEX0_2;
				CLAMP_t CLAMP_1;
				CLAMP_t CLAMP_2;
				u64 FOG;
				
				u64 res0;	// 0x0b
				
				XYZF_t XYZF3;
				XYZ_t XYZ3;	// 0x0d
				
				// 0xe-0x13 - 6 registers
				u64 res1 [ 6 ];
				
				u64 TEX1_1;	// 0x14
				u64 TEX1_2;
				TEX0_t TEX2_1;
				TEX0_t TEX2_2;
				XYOFFSET_t XYOFFSET_1;
				XYOFFSET_t XYOFFSET_2;
				u64 PRMODECONT;
				PRIM_t PRMODE;
				TEXCLUT_t TEXCLUT;	// 0x1c
				
				// 0x1d-0x21 - 5 registers
				u64 res2 [ 5 ];
				
				u64 SCANMSK;	// 0x22
				
				// 0x23-0x33 - 17 registers
				u64 res3 [ 17 ];
				
				u64 MIPTBP1_1;	// 0x34
				u64 MIPTBP1_2;
				u64 MIPTBP2_1;
				u64 MIPTBP2_2;	// 0x37
				
				// 0x38-0x3a - 3 registers
				u64 res4 [ 3 ];
				
				TEXA_t TEXA;	// 0x3b
				
				u64 res5;	// 0x3c
				
				u64 FOGCOL;	// 0x3d
				
				u64 res6;	// 0x3e
				
				u64 TEXFLUSH;	// 0x3f
				SCISSOR_t SCISSOR_1;
				SCISSOR_t SCISSOR_2;
				ALPHA_t ALPHA_1;
				ALPHA_t ALPHA_2;
				u64 DIMX;
				u64 DTHE;
				u64 COLCLAMP;
				TEST_t TEST_1;
				TEST_t TEST_2;
				u64 PABE;
				u64 FBA_1;
				u64 FBA_2;
				FRAME_t FRAME_1;
				FRAME_t FRAME_2;
				ZBUF_t ZBUF_1;
				ZBUF_t ZBUF_2;
				BITBLTBUF_t BITBLTBUF;
				TRXPOS_t TRXPOS;
				TRXREG_t TRXREG;
				TRXDIR_t TRXDIR;
				u64 HWREG;	//0x54
				
				// 0x55-0x5f - 11 registers
				u64 res7 [ 11 ];
				
				u64 SIGNAL;	// 0x60
				u64 FINISH;
				u64 LABEL;	// 0x62
			};
		};
		
		GPURegsGp_t GPURegsGp;
		
		
		// internal graphics registers/variables
		
		// window coords (these values are inclusive)
		// window coords are unsigned
		s32 iWindow_XLeft [ 2 ], iWindow_XRight [ 2 ], iWindow_YTop [ 2 ], iWindow_YBottom [ 2 ];
		
		// texture buffer
		u32 iTexBufStartOffset32 [ 2 ], iTexBufWidth_Pixels [ 2 ], iTexBuf_PixelFormat [ 2 ];
		
		// clut
		u32 iCLUTStartOffset32 [ 2 ], iCLUTWidth_Pixels [ 2 ], iCLUT_BitsPerPixel [ 2 ];
		
		u32 iTexBufWidth [ 2 ], iTexWidth [ 2 ], iTexHeight [ 2 ], iTexWidth_Mask [ 2 ], iTexHeight_Mask [ 2 ];
		
		u32 iShift1 [ 2 ], iShift2 [ 2 ], iAnd1 [ 2 ], iAnd2 [ 2 ];
		u32 iShift0 [ 2 ];
		u32 iTextureOffset [ 2 ];
		
		u32 iAnd3 [ 2 ], iShift3 [ 2 ], iShift4 [ 2 ];
		
		u32 iPixelFormat [ 2 ];
		u32 iCLUTBufBase32 [ 2 ], iCLUTPixelFormat [ 2 ], iCLUTStoreMode [ 2 ], iCLUTOffset [ 2 ];

		
		u32 iclut_width, iclut_x, iclut_y;
		
#ifdef USE_CBP_SINGLEVALUE
		u32 CBP0, CBP1;
#else
		u32 CBP0 [ 32 ], CBP1 [ 32 ];
#endif
		
		// not used
		//u32 iTexBPP [ 2 ];
		//u32 iTexWidth_Shift [ 2 ];

		
		// general purpose register indexes
		static const u32 PRIM = 0x0;	// 0x00
		static const u32 RGBAQ = 0x1;
		static const u32 ST = 0x2;
		static const u32 UV = 0x3;
		static const u32 XYZF2 = 0x4;
		static const u32 XYZ2 = 0x5;
		static const u32 TEX0_1 = 0x6;
		static const u32 TEX0_2 = 0x7;
		static const u32 CLAMP_1 = 0x8;
		static const u32 CLAMP_2 = 0x9;
		static const u32 FOG = 0xa;
		static const u32 XYZF3 = 0xc;
		static const u32 XYZ3 = 0xd;	// 0x0d
		static const u32 TEX1_1 = 0x14;	// 0x14
		static const u32 TEX1_2 = 0x15;
		static const u32 TEX2_1 = 0x16;
		static const u32 TEX2_2 = 0x17;
		static const u32 XYOFFSET_1 = 0x18;
		static const u32 XYOFFSET_2 = 0x19;
		static const u32 PRMODECONT = 0x1a;
		static const u32 PRMODE = 0x1b;
		static const u32 TEXCLUT = 0x1c;	// 0x1c
		static const u32 SCANMSK = 0x22;	// 0x22
		static const u32 MIPTBP1_1 = 0x34;	// 0x34
		static const u32 MIPTBP1_2 = 0x35;
		static const u32 MIPTBP2_1 = 0x36;
		static const u32 MIPTBP2_2 = 0x37;	// 0x37
		static const u32 TEXA = 0x3b;	// 0x3b
		static const u32 FOGCOL = 0x3d;	// 0x3d
		static const u32 TEXFLUSH = 0x3f;	// 0x3f
		static const u32 SCISSOR_1 = 0x40;
		static const u32 SCISSOR_2 = 0x41;
		static const u32 ALPHA_1 = 0x42;
		static const u32 ALPHA_2 = 0x43;
		static const u32 DIMX = 0x44;
		static const u32 DTHE = 0x45;
		static const u32 COLCLAMP = 0x46;
		static const u32 TEST_1 = 0x47;
		static const u32 TEST_2 = 0x48;
		static const u32 PABE = 0x49;
		static const u32 FBA_1 = 0x4a;
		static const u32 FBA_2 = 0x4b;
		static const u32 FRAME_1 = 0x4c;
		static const u32 FRAME_2 = 0x4d;
		static const u32 ZBUF_1 = 0x4e;
		static const u32 ZBUF_2 = 0x4f;
		static const u32 BITBLTBUF = 0x50;
		static const u32 TRXPOS = 0x51;
		static const u32 TRXREG = 0x52;
		static const u32 TRXDIR = 0x53;
		static const u32 HWREG = 0x54;	//0x54
		static const u32 SIGNAL = 0x60;	// 0x60
		static const u32 FINISH = 0x61;
		static const u32 LABEL = 0x62;	// 0x62
		
		
		
		
		double GetCycles_SinceLastPixel ();
		double GetCycles_SinceLastHBlank ();
		double GetCycles_SinceLastVBlank ();
		double GetCycles_ToNextPixel ();
		double GetCycles_ToNextHBlank ();
		double GetCycles_ToNextVBlank ();

		double GetCycles_SinceLastPixel ( double dAtCycle );
		double GetCycles_SinceLastHBlank ( double dAtCycle );
		double GetCycles_SinceLastVBlank ( double dAtCycle );
		double GetCycles_ToNextPixel ( double dAtCycle );
		double GetCycles_ToNextHBlank ( double dAtCycle );
		double GetCycles_ToNextVBlank ( double dAtCycle );
		
		double GetCycles_ToNextScanlineStart ();
		double GetCycles_ToNextFieldStart ();
		double GetCycles_SinceLastScanlineStart ();
		double GetCycles_SinceLastFieldStart ();
		
		double GetCycles_ToNextScanlineStart ( double dAtCycle );
		double GetCycles_ToNextFieldStart ( double dAtCycle );
		double GetCycles_SinceLastScanlineStart ( double dAtCycle );
		double GetCycles_SinceLastFieldStart ( double dAtCycle );
		
		
		// gets the total number of scanlines since start of program
		u64 GetScanline_Count ();
		
		// gets the scanline number you are on (for ntsc that would be 0-524, for pal 0-624)
		u64 GetScanline_Number ();
		
		// gets cycle @ start of scanline
		double GetScanline_Start ();
		
		bool isHBlank ();
		bool isVBlank ();
		
		bool isHBlank ( double dAtCycle );
		bool isVBlank ( double dAtCycle );
		
		// get the next event (vblank)
		void GetNextEvent ();
		void GetNextEvent_V ();
		void Update_NextEvent ();
		
		void SetNextEvent ( u64 CycleOffset );
		void SetNextEvent_Cycle ( u64 Cycle );
		void Update_NextEventCycle ();
		

		
		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////
		

																	
		
		// System data
		//static const double SystemBus_CyclesPerSec = 147456000.0L;
		
		// Raster data
		static const double NTSC_FieldsPerSec = 59.94005994L;
		static const double PAL_FieldsPerSec = 50.0L;
		static const double NTSC_FramesPerSec = ( 59.94005994L / 2.0L );	//NTSC_FieldsPerSec / 2;
		static const double PAL_FramesPerSec = ( 50.0L / 2.0L );	//PAL_FieldsPerSec / 2;
		static const double NTSC_CyclesPerFrame = ( 147456000.0L / ( 59.94005994L / 2.0L ) );
		static const double PAL_CyclesPerFrame = ( 147456000.0L / ( 50.0L / 2.0L ) );
		
		static const double NTSC_FramesPerCycle = 1.0L / ( 147456000.0L / ( 59.94005994L / 2.0L ) );
		static const double PAL_FramesPerCycle = 1.0L / ( 147456000.0L / ( 50.0L / 2.0L ) );
		
		// the EVEN field must be the field that starts at scanline number zero, which means that the even field has 263 scanlines
		// so, for now I just have these mixed up since the EVEN scanlines go from 0-524, making 263 scanlines, and the ODD go from 1-523, making 262
		
		// EVEN scanlines from 0-524 (263 out of 525 total)
		static const u32 NTSC_ScanlinesPerField_Even = 263;
		
		// ODD scanlines from 1-523 (262 out of 525 total)
		static const u32 NTSC_ScanlinesPerField_Odd = 262;
		
		// 525 total scanlines for NTSC
		static const u32 NTSC_ScanlinesPerFrame = 525;	//NTSC_ScanlinesPerField_Odd + NTSC_ScanlinesPerField_Even;
		
		// EVEN scanlines from 0-624 (313 out of 625 total)
		static const u32 PAL_ScanlinesPerField_Even = 313;
		
		// ODD scanlines from 1-623 (312 out of 625 total)
		static const u32 PAL_ScanlinesPerField_Odd = 312;
		
		// 625 total scanlines for PAL
		static const u32 PAL_ScanlinesPerFrame = 625;	//PAL_ScanlinesPerField_Odd + PAL_ScanlinesPerField_Even;

		static const double NTSC_FieldsPerScanline_Even = 1.0L / 263.0L;
		static const double NTSC_FieldsPerScanline_Odd = 1.0L / 262.0L;
		static const double NTSC_FramesPerScanline = 1.0L / 525.0L;
		
		static const double PAL_FieldsPerScanline_Even = 1.0L / 313.0L;
		static const double PAL_FieldsPerScanline_Odd = 1.0L / 312.0L;
		static const double PAL_FramesPerScanline = 1.0L / 625.0L;
		
		
		
		static const double NTSC_ScanlinesPerSec = ( 525.0L * ( 59.94005994L / 2.0L ) );	//NTSC_ScanlinesPerFrame * NTSC_FramesPerSec; //15734.26573425;
		static const double PAL_ScanlinesPerSec = ( 625.0L * ( 50.0L / 2.0L ) );	//PAL_ScanlinesPerFrame * PAL_FramesPerSec; //15625;
		static const double NTSC_CyclesPerScanline = ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) );	//SystemBus_CyclesPerSec / NTSC_ScanlinesPerSec; //2152.5504000022;
		static const double PAL_CyclesPerScanline = ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) );	//SystemBus_CyclesPerSec / PAL_ScanlinesPerSec; //2167.6032;
		
		static const double NTSC_ScanlinesPerCycle = 1.0L / ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) );
		static const double PAL_ScanlinesPerCycle = 1.0L / ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) );
		
		
		
		static const double NTSC_CyclesPerField_Even = 263.0L * ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) );
		static const double NTSC_CyclesPerField_Odd = 262.0L * ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) );
		static const double PAL_CyclesPerField_Even = 313.0L * ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) );
		static const double PAL_CyclesPerField_Odd = 312.0L * ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) );
		
		static const double NTSC_FieldsPerCycle_Even = 1.0L / ( 263.0L * ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) );
		static const double NTSC_FieldsPerCycle_Odd = 1.0L / ( 262.0L * ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) );
		static const double PAL_FieldsPerCycle_Even = 1.0L / ( 313.0L * ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) );
		static const double PAL_FieldsPerCycle_Odd = 1.0L / ( 312.0L * ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) );



		static const double NTSC_DisplayAreaCycles = 240.0L * ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) );
		static const double PAL_DisplayAreaCycles = 288.0L * ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) );
		
		static const double NTSC_CyclesPerVBlank_Even = ( 263.0L - 240.0L ) * ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) );
		static const double NTSC_CyclesPerVBlank_Odd = ( 262.0L - 240.0L ) * ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) );
		static const double PAL_CyclesPerVBlank_Even = ( 313.0L - 288.0L ) * ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) );
		static const double PAL_CyclesPerVBlank_Odd = ( 312.0L - 288.0L ) * ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) );
		
		static const double NTSC_VBlanksPerCycle_Even = 1.0L / ( ( 263.0L - 240.0L ) * ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) );
		static const double NTSC_VBlanksPerCycle_Odd = 1.0L / ( ( 262.0L - 240.0L ) * ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) );
		static const double PAL_VBlanksPerCycle_Even = 1.0L / ( ( 313.0L - 288.0L ) * ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) );
		static const double PAL_VBlanksPerCycle_Odd = 1.0L / ( ( 312.0L - 288.0L ) * ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) );



		// the viewable scanlines (inclusive)
		static const long PAL_Field0_Viewable_YStart = 23;
		static const long PAL_Field0_Viewable_YEnd = 310;
		static const long PAL_Field1_Viewable_YStart = 336;
		static const long PAL_Field1_Viewable_YEnd = 623;
		
		static const u32 NTSC_VBlank = 240;
		static const u32 PAL_VBlank = 288;
		u32 Display_Width, Display_Height, Raster_Width, Raster_Height;
		u32 Raster_X, Raster_Y, Raster_HBlank;
		u32 Raster_Pixel_INC;
		
		// cycle at which the raster settings last changed at
		u64 RasterChange_StartCycle;
		u64 RasterChange_StartPixelCount;
		u64 RasterChange_StartHBlankCount;
		
		double dCyclesPerFrame, dCyclesPerField0, dCyclesPerField1, dDisplayArea_Cycles, dVBlank0Area_Cycles, dVBlank1Area_Cycles, dHBlankArea_Cycles;
		double dFramesPerCycle, dFieldsPerCycle0, dFieldsPerCycle1; //, dDisplayArea_Cycles, dVBlank0Area_Cycles, dVBlank1Area_Cycles, dHBlankArea_Cycles;
		u64 CyclesPerHBlank, CyclesPerVBlank;

		u64 CyclesPerPixel_INC;
		double dCyclesPerPixel, dCyclesPerScanline;
		double dPixelsPerCycle, dScanlinesPerCycle;
		u64 PixelCycles;
		u32 X_Pixel, Y_Pixel;
		u64 Global_XPixel, Global_YPixel;
		u32 HBlank_X;
		u32 VBlank_Y;
		u32 ScanlinesPerField0, ScanlinesPerField1;
		u32 Raster_XMax;
		u32 Raster_YMax;	// either 525 or 625
		
		
		// there could be data waiting to be transferred by path2 (currently up to 64-bits)
		u32 ulPath2_DataWaiting;
		u32 ullPath2_Data [ 4 ];
		
		
		// the current start cycles for the current scanline (in both double and long long)
		// recalculate whenever the internal resolution gets changed, even slightly
		double dScanlineStart, dNextScanlineStart, dHBlankStart;
		unsigned long long llScanlineStart, llNextScanlineStart, llHBlankStart;
		unsigned long lScanline, lNextScanline, lVBlank, lMaxScanline;
		
		double dGPU_NextEventCycle;
		u64 iGPU_NextEventCycle, iCyclesPerScanline;
		
		
		//double dNextScanlineStart_Cycle, dNextVsyncStart_Cycle, dNextDrawStart_Cycle;
		
		// *** todo *** keeping track of these may be used to speed up timers
		//double dLastScanlineStart_Cycle, dNextScanlineStart_Cycle, dLastHBlank_Cycle, dNextHBlank_Cycle, dLastVBlank_Cycle, dNextVBlank_Cycle, dLastDrawStart_Cycle, dNextDrawStart_Cycle;
		
		
		static const u32 HBlank_X_LUT [ 8 ];
		static const u32 VBlank_Y_LUT [ 2 ];
		static const u32 Raster_XMax_LUT [ 2 ] [ 8 ];
		static const u32 Raster_YMax_LUT [ 2 ];
		u64 CyclesPerPixel_INC_Lookup [ 2 ] [ 8 ];
		double CyclesPerPixel_Lookup [ 2 ] [ 8 ];
		double PixelsPerCycle_Lookup [ 2 ] [ 8 ];
		
		static const int c_iGPUCyclesPerPixel_256 = 10;
		static const int c_iGPUCyclesPerPixel_320 = 8;
		static const int c_iGPUCyclesPerPixel_368 = 7;
		static const int c_iGPUCyclesPerPixel_512 = 5;
		static const int c_iGPUCyclesPerPixel_640 = 4;
		
		//static const u32 c_iGPUCyclesPerPixel [];
		
		static const double c_dGPUCyclesPerScanline_NTSC = 147456000.0L / ( 525.0L * 30.0L );
		static const double c_dGPUCyclesPerScanline_PAL = 147456000.0L / ( 625.0L * 30.0L );

		
		static const int c_iVisibleArea_StartX_Cycle = 584; //544;
		static const int c_iVisibleArea_EndX_Cycle = 3192;	//3232;
		static const int c_iVisibleArea_StartY_Pixel_NTSC = 15;
		static const int c_iVisibleArea_EndY_Pixel_NTSC = 257;
		static const int c_iVisibleArea_StartY_Pixel_PAL = 34;
		static const int c_iVisibleArea_EndY_Pixel_PAL = 292;
		
		static const int c_iScreenOutput_MaxWidth = ( c_iVisibleArea_EndX_Cycle >> 2 ) - ( c_iVisibleArea_StartX_Cycle >> 2 );
		static const int c_iScreenOutput_MaxHeight = ( c_iVisibleArea_EndY_Pixel_PAL - c_iVisibleArea_StartY_Pixel_PAL ) << 1;
		
		// raster functions
		void UpdateRaster_VARS ( void );
		
		// returns interrupt data
		//u32 UpdateRaster ( void );
		void UpdateRaster ( void );
		
		
		//static const u32 DisplayWidth_Values [] = { 256, 320, 368, 512, 640 };
		//static const u32 NTSC_RasterWidth_Values [] = { 341, 426, 487, 682, 853 };
		//static const u32 PAL_RasterWidth_Values [] = { 340, 426, 486, 681, 851 };
		
		//static const u32 NTSC_DisplayHeight_Values [] = { 240, 480 };
		//static const u32 PAL_DisplayHeight_Values [] = { 288, 576 };
		//static const u32 NTSC_RasterHeight_OddField = 263;
		//static const u32 NTSC_RasterHeight_EvenField = 262;
		//static const u32 NTSC_RasterHeight_Total = 525;
		//static const u32 PAL_RasterHeight_OddField = 313;
		//static const u32 PAL_RasterHeight_EvenField = 312;
		//static const u32 PAL_RasterHeight_Total = 625;
		
		static const double NTSC_CyclesPerPixel_256 = ( ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) / 341.0L );
		static const double NTSC_CyclesPerPixel_320 = ( ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) / 426.0L );
		static const double NTSC_CyclesPerPixel_368 = ( ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) / 487.0L );
		static const double NTSC_CyclesPerPixel_512 = ( ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) / 682.0L );
		static const double NTSC_CyclesPerPixel_640 = ( ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) / 853.0L );
		static const double PAL_CyclesPerPixel_256 = ( ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) / 340.0L );
		static const double PAL_CyclesPerPixel_320 = ( ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) / 426.0L );
		static const double PAL_CyclesPerPixel_368 = ( ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) / 486.0L );
		static const double PAL_CyclesPerPixel_512 = ( ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) / 681.0L );
		static const double PAL_CyclesPerPixel_640 = ( ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) / 851.0L );
		
		static const double NTSC_PixelsPerCycle_256 = 1.0L / ( ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) / 341.0L );
		static const double NTSC_PixelsPerCycle_320 = 1.0L / ( ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) / 426.0L );
		static const double NTSC_PixelsPerCycle_368 = 1.0L / ( ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) / 487.0L );
		static const double NTSC_PixelsPerCycle_512 = 1.0L / ( ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) / 682.0L );
		static const double NTSC_PixelsPerCycle_640 = 1.0L / ( ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) / 853.0L );
		static const double PAL_PixelsPerCycle_256 = 1.0L / ( ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) / 340.0L );
		static const double PAL_PixelsPerCycle_320 = 1.0L / ( ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) / 426.0L );
		static const double PAL_PixelsPerCycle_368 = 1.0L / ( ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) / 486.0L );
		static const double PAL_PixelsPerCycle_512 = 1.0L / ( ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) / 681.0L );
		static const double PAL_PixelsPerCycle_640 = 1.0L / ( ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) / 851.0L );
		
		
		static const u64 NTSC_CyclesPerPixelINC_256 = ((u64) (( (1ull) << 63 ) * ( 1.0L / ( ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) / 341.0L ) ))) + ( 1 << 8 );
		static const u64 NTSC_CyclesPerPixelINC_320 = ((u64) (( (1ull) << 63 ) * ( 1.0L / ( ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) / 426.0L ) ))) + ( 1 << 8 );
		static const u64 NTSC_CyclesPerPixelINC_368 = ((u64) (( (1ull) << 63 ) * ( 1.0L / ( ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) / 487.0L ) ))) + ( 1 << 8 );
		static const u64 NTSC_CyclesPerPixelINC_512 = ((u64) (( (1ull) << 63 ) * ( 1.0L / ( ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) / 682.0L ) ))) + ( 1 << 8 );
		static const u64 NTSC_CyclesPerPixelINC_640 = ((u64) (( (1ull) << 63 ) * ( 1.0L / ( ( 147456000.0L / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) / 853.0L ) ))) + ( 1 << 8 );
		static const u64 PAL_CyclesPerPixelINC_256 = ((u64) (( (1ull) << 63 ) * ( 1.0L / ( ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) / 340.0L ) ))) + ( 1 << 8 );
		static const u64 PAL_CyclesPerPixelINC_320 = ((u64) (( (1ull) << 63 ) * ( 1.0L / ( ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) / 426.0L ) ))) + ( 1 << 8 );
		static const u64 PAL_CyclesPerPixelINC_368 = ((u64) (( (1ull) << 63 ) * ( 1.0L / ( ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) / 486.0L ) ))) + ( 1 << 8 );
		static const u64 PAL_CyclesPerPixelINC_512 = ((u64) (( (1ull) << 63 ) * ( 1.0L / ( ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) / 681.0L ) ))) + ( 1 << 8 );
		static const u64 PAL_CyclesPerPixelINC_640 = ((u64) (( (1ull) << 63 ) * ( 1.0L / ( ( 147456000.0L / ( 625.0L * ( 50.0L / 2.0L ) ) ) / 851.0L ) ))) + ( 1 << 8 );
		
		//static const double NTSC_CyclesPerPixel_Values [] = { 6.3124645161, 5.0529352113, 4.4200213552, 3.1562322581, 2.5235057444 };
		//static const double PAL_CyclesPerPixel_Values [] = { 6.3753035294, 5.0882704225, 4.4600888889, 3.1829709251, 2.5471247944 };
		
		// the last command written to CTRL register
		//u32 CommandReady;
		//CTRL_Write_Format NextCommand;
		
		// we need to know if the GPU is busy or not
		s64 BusyCycles;
		
		// we need a "command buffer" for GPU
		// stuff written to DATA register or via dma goes into command buffer
		//u32 BufferMode;
		//enum { MODE_NORMAL, MODE_IMAGEIN, MODE_IMAGEOUT };
		//DATA_Write_Format Buffer [ 16 ];
		//u32 BufferSize;
		//u32 PixelsLeftToTransfer;
		
		// dither values array
		//static const s64 c_iDitherValues [];
		//static const s64 c_iDitherZero [];
		//static const s64 c_iDitherValues24 [];
		//static const s32 c_iDitherValues4 [];
		//static const s16 c_viDitherValues16_add [];
		//static const s16 c_viDitherValues16_sub [];

		// draw area data
		
		// upper left hand corner of frame buffer area being drawn to the tv
		u32 ScreenArea_TopLeftX;
		u32 ScreenArea_TopLeftY;
		
		// bounding rectangles of the area being drawn to - anything outside of this is not drawn
		//u32 DrawArea_TopLeftX;
		//u32 DrawArea_TopLeftY;
		//u32 DrawArea_BottomRightX;
		//u32 DrawArea_BottomRightY;
		
		// also need to maintain the internal coords to return when requested
		//u32 iREG_DrawArea_TopLeftX;
		//u32 iREG_DrawArea_TopLeftY;
		//u32 iREG_DrawArea_BottomRightX;
		//u32 iREG_DrawArea_BottomRightY;
		
		// the offset from the upper left hand corner of frame buffer that is being drawn to. Applies to all drawing
		// these are signed and go from -1024 to +1023
		//s32 DrawArea_OffsetX;
		//s32 DrawArea_OffsetY;
		
		// specifies amount of data being sent to the tv
		u32 DisplayRange_Horizontal;
		u32 DisplayRange_Vertical;
		u32 DisplayRange_X1;
		u32 DisplayRange_X2;
		u32 DisplayRange_Y1;
		u32 DisplayRange_Y2;
		
		// texture window
		u32 TWX, TWY, TWW, TWH;
		u32 TextureWindow_X;
		u32 TextureWindow_Y;
		u32 TextureWindow_Width;
		u32 TextureWindow_Height;
		
		// mask settings
		u32 SetMaskBitWhenDrawing;
		u32 DoNotDrawToMaskAreas;
		
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// the tx and ty in the GPU_CTRL register are actually texture offsets into where the textures start at
		// add these to the texture offsets supplied by the command to get the real offsets in the frame buffer
		//u32 Texture_OffsetX, Texture_OffsetY;

		// *note* these should be signed
		s32 Absolute_DrawX, Absolute_DrawY;
		
		// also need to count primitives for debugging (step primitive, etc.)
		u32 Primitive_Count;
		
		// also need to count frames for debugging
		u32 Frame_Count;
		
		/*
		static const u32 FrameBuffer_Width = 1024;
		static const u32 FrameBuffer_Height = 512;
		static const u32 FrameBuffer_XMask = FrameBuffer_Width - 1;
		static const u32 FrameBuffer_YMask = FrameBuffer_Height - 1;
		*/
		
		// constructor
		GPU ();
		
		// debug info
		static u32* DebugCpuPC;
		
		
		// write to a general purpose register
		void WriteReg ( u32 lIndex, u64 Value );
		void WriteReg_Packed ( u32 lIndex, u64 ull0, u64 ull1 );
		
		// perform drawing kick
		void DrawingKick ();
		
		// perform actual drawing of primitive (triangle, sprite, etc)
		void DrawPrimitive ();
		
		// reset vertex count for certain primitives
		void ResetPrimitive ();
		
		u32 lVertexCount;
		u32 lVertexQ_Index;
		static const u32 c_iVertexQ_Size = 4;
		static const u32 c_iVertexQ_Mask = c_iVertexQ_Size - 1;
		//u32 x [ 4 ], y [ 4 ], z [ 4 ], f [ 4 ], r [ 4 ], g [ 4 ], b [ 4 ], a [ 4 ], q [ 4 ], u [ 4 ], v [ 4 ], s [ 4 ], t [ 4 ];
		
		// adding one to the size of the vertex buffer so that the first triangle fan point can be saved on end of array
		XYZ_t xyz [ c_iVertexQ_Size + 1 ];
		RGBAQ_t rgbaq [ c_iVertexQ_Size + 1 ];
		UV_t uv [ c_iVertexQ_Size + 1 ];
		ST_t st [ c_iVertexQ_Size + 1 ];
		XYZF_t f [ c_iVertexQ_Size + 1 ];
		
		// need to know what the current color is? or last color specified?
		RGBAQ_t rgbaq_Current;
		
		// there is also an internal Q register for packed mode
		u64 Internal_Q;
		
		
		struct UV_temp_t
		{
			s32 U;
			s32 V;
		};
		
		// need a temporary uv storage for st->uv conversion for now
		UV_temp_t uv_temp [ c_iVertexQ_Size + 1 ];
		
		
		inline void StartPrimitive () { lVertexCount = 0; lVertexQ_Index = 0; }

		/*
		inline void SetX ( u32 Value ) { x [ lVertexQ_Index ] = Value; }
		inline void SetY ( u32 Value ) { y [ lVertexQ_Index ] = Value; }
		inline void SetZ ( u32 Value ) { z [ lVertexQ_Index ] = Value; }
		inline void SetF ( u32 Value ) { f [ lVertexQ_Index ] = Value; }
		inline void SetR ( u32 Value ) { r [ lVertexQ_Index ] = Value; }
		inline void SetG ( u32 Value ) { g [ lVertexQ_Index ] = Value; }
		inline void SetB ( u32 Value ) { b [ lVertexQ_Index ] = Value; }
		inline void SetA ( u32 Value ) { a [ lVertexQ_Index ] = Value; }
		inline void SetQ ( u32 Value ) { q [ lVertexQ_Index ] = Value; }
		inline void SetU ( u32 Value ) { u [ lVertexQ_Index ] = Value; }
		inline void SetV ( u32 Value ) { v [ lVertexQ_Index ] = Value; }
		inline void SetS ( u32 Value ) { s [ lVertexQ_Index ] = Value; }
		inline void SetT ( u32 Value ) { t [ lVertexQ_Index ] = Value; }
		*/
		
		inline void SetF ( u64 Value ) { f [ lVertexQ_Index & c_iVertexQ_Mask ].Value = Value; }
		inline void SetXYZ ( u64 Value ) { xyz [ lVertexQ_Index & c_iVertexQ_Mask ].Value = Value; }
		inline void SetRGBAQ ( u64 Value ) { rgbaq [ lVertexQ_Index & c_iVertexQ_Mask ].Value = Value; rgbaq_Current.Value = Value; }
		inline void SetUV ( u64 Value ) { uv [ lVertexQ_Index & c_iVertexQ_Mask ].Value = Value; }
		inline void SetST ( u64 Value ) { st [ lVertexQ_Index & c_iVertexQ_Mask ].Value = Value; }
		
		
		void VertexKick ();
		
		
		void ExecuteGPUBuffer ();
		
		void ProcessDataRegWrite ( u32 Data );
		u32 ProcessDataRegRead ();
		
		void TransferPixelPacketIn ( u32 Data );
		u32 TransferPixelPacketOut ();
		
		
		///////////////////////////////////////////////////////////////////////
		// Function to perform actual drawing of primitives on screen area


		void DrawPoint ( u32 Coord0 );
		void DrawLine ( u32 Coord0, u32 Coord1 );
		void DrawTriangle ( u32 Coord0, u32 Coord1, u32 Coord2 );
		void DrawTriangleFan ();
		void DrawSprite ( u32 Coord0, u32 Coord1 );

		
		// ???
		// number of bits at top of buffer pointer to use to store width (set 9 for per page, etc)
		static const u32 c_lBufCheckBits = 14;
		
		// there must also be 512 entries for buffer widths ???
		// using top 9 bits of base pointer as index ???
		u32 InternalBufferWidth [ 1 << c_lBufCheckBits ];
		
		// internal CLUT is 1KB
		u16 InternalCLUT [ 512 ];
		
		// need to function to write CLUT into internal CLUT
		void WriteInternalCLUT ( TEX0_t TEX02 );
		
		
		// global rendering variables //
		
		// must read texture info from here since there are multiple registers that write here
		// need this for both contexts
		TEX0_t TEXX [ 2 ];
		
		// context
		u32 Ctx;
		
		// frame buffer
		u32 FrameBufferStartOffset32, FrameBufferWidth_Pixels, FrameBuffer_PixelFormat;
		u32 ZBufferStartOffset32, ZBuffer_PixelFormat;
		
		// texture buffer
		u32 TexBufStartOffset32, TexBufWidth_Pixels, TexBuf_PixelFormat;
		
		// clut
		u32 CLUTStartOffset32, CLUTWidth_Pixels, CLUT_BitsPerPixel;
		
		// bit blt
		u32 XferSrcOffset32, XferDstOffset32, XferSrcBufWidth, XferDstBufWidth, XferSrcX, XferSrcY, XferDstX, XferDstY, XferSrcPixelSize, XferDstPixelSize, XferWidth, XferHeight;
		u32 XferDstBufWidth64, XferSrcBufWidth64;
		u32 XferX, XferY;
		u64 PixelCount;
		u64 PixelShift;
		
		// variables for local->local transfer
		s32 xStart, yStart, xInc, yInc;
		
		// window coords (these values are inclusive)
		// window coords are unsigned
		s32 Window_XLeft, Window_XRight, Window_YTop, Window_YBottom;
		
		// x,y offset (12.4 fixed point format -> would imagine this is signed)
		s32 Coord_OffsetX, Coord_OffsetY;
		
		// clamp
		u32 Clamp_ModeX, Clamp_ModeY, Clamp_MinU, Clamp_MaxU, Clamp_MinV, Clamp_MaxV;
		
		// object texture mapped?
		u32 TextureMapped;

		// gradient?
		u32 Gradient;
		
		// alpha blending enabled?
		u32 Alpha;
		
		// FST can also be selected by prmodecont
		u32 Fst;
		
		// set if fogging is enabled
		u32 FogEnable;
		
		// alpha correction value
		u32 FBA_X;
		
		// z-buffer data
		ZBUF_t ZBUF_X;
		
		// get the values to select for alpha
		ALPHA_t ALPHA_X;

		// get the values to select for test
		TEST_t TEST_X;
		
		
		// global drawing variables //
		
		// note: could make these static, but may want to set them later when registers get set?
		// or could allow for configuring both ways for testing?

		// alpha blending selection
		u32 AlphaSelect [ 4 ];
		u32 uA, uB, uC, uD;
		
		// pabe
		u32 AlphaXor32, AlphaXor16;
		
		// alpha correction value (FBA)
		u32 PixelOr32, PixelOr16;
		
		// destination alpha test
		// DA_Enable -> the value of DATE, to be and'ed with pixel
		// DA_Test -> the value of DATM, to be xor'ed with pixel
		u32 DA_Enable, DA_Test;
		
		// source alpha test
		//u64 LessThan_And, GreaterOrEqualTo_And, LessThan_Or;
		u32 Alpha_Fail;
		u32 SrcAlphaTest_Pass, SrcAlpha_ARef;

		// depth test
		s64 DepthTest_Offset;

		// in a 24-bit frame buffer, destination alpha is always 0x80
		u32 DestAlpha24, DestMask24;
		
		// rgb24 source pixel handling
		u32 RGB24_Alpha, RGB24_TAlpha, RGB24_DTestAlpha;
		u32 Pixel24_Mask;
		
		// mask for drawing to frame buffer
		u32 FrameBuffer_WriteMask32, FrameBuffer_WriteMask16;
		
		// pointer to end of draw buffer (on ps2 it is easy to draw outside of buffer)
		static void *PtrEnd;

		// need pointer into buffer and into z-buffer
		static u32 *buf32;
		static u32 *zbuf32;
		
		// need a null z-buffer to point into on error for now
		u32 zbuf_NULL;
		
		
		
		void SetDrawVars ();
		void RenderPoint ( u32 Coord0 );
		void RenderLine_Mono ( u32 Coord0, u32 Coord1 );
		void RenderLine_Gradient ( u32 Coord0, u32 Coord1 );
		//void RenderTriangleColor ();
		//void RenderTriangleTexture ();
		void RenderRectangle ( u32 Coord0, u32 Coord1 );
		void RenderSprite ( u32 Coord0, u32 Coord1 );
		
		void DrawTriangle_Mono32 ( u32 Coord0, u32 Coord1, u32 Coord2 );
		void DrawTriangle_Gradient32 ( u32 Coord0, u32 Coord1, u32 Coord2 );
		void DrawTriangle_Texture32 ( u32 Coord0, u32 Coord1, u32 Coord2 );
		
		
		void DrawTriangle_Texture ( u32 Coord0, u32 Coord1, u32 Coord2 );
		void DrawTriangle_TextureGradient ( u32 Coord0, u32 Coord1, u32 Coord2 );


		void RenderPoint_DS ( u32 Coord0 );
		void RenderLine_Mono_DS ( u32 Coord0, u32 Coord1 );
		void RenderLine_Gradient_DS ( u32 Coord0, u32 Coord1 );
		void RenderRectangle_DS ( u32 Coord0, u32 Coord1 );
		void RenderSprite_DS ( u32 Coord0, u32 Coord1 );
		void DrawTriangle_Mono32_DS ( u32 Coord0, u32 Coord1, u32 Coord2 );
		void DrawTriangle_Gradient32_DS ( u32 Coord0, u32 Coord1, u32 Coord2 );
		void DrawTriangle_Texture32_DS ( u32 Coord0, u32 Coord1, u32 Coord2 );
		//void DrawTriangle_Texture_DS ( u32 Coord0, u32 Coord1, u32 Coord2 );
		void DrawTriangle_GradientTexture32_DS ( u32 Coord0, u32 Coord1, u32 Coord2 );
		//void DrawTriangle_TextureGradient_DS ( u32 Coord0, u32 Coord1, u32 Coord2 );
		void TransferDataLocal_DS ();
		void TransferDataOut32_DS ( u32* Data, u32 WordCount32 );
		void TransferDataIn32_DS ( u32* Data, u32 WordCount32 );


		
		// sets the portion of frame buffer that is showing on screen
		void Set_ScreenSize ( int _width, int _height );

		
		// inline functions
		
		inline static s32 GetRed16 ( u16 Color ) { return ( ( Color >> 10 ) & 0x1f ); }
		inline static s32 GetGreen16 ( u16 Color ) { return ( ( Color >> 5 ) & 0x1f ); }
		inline static s32 GetBlue16 ( u16 Color ) { return ( Color & 0x1f ); }
		inline static s32 SetRed16 ( u16 Color ) { return ( ( Color & 0x1f ) << 10 ); }
		inline static s32 SetGreen16 ( u16 Color ) { return ( ( Color & 0x1f ) << 5 ); }
		inline static s32 SetBlue16 ( u16 Color ) { return ( Color & 0x1f ); }
		
		inline static s32 GetRed24 ( u32 Color ) { return ( ( Color >> 16 ) & 0xff ); }
		inline static s32 GetGreen24 ( u32 Color ) { return ( ( Color >> 8 ) & 0xff ); }
		inline static s32 GetBlue24 ( u32 Color ) { return ( Color & 0xff ); }
		inline static s32 SetRed24 ( u32 Color ) { return ( ( Color & 0xff ) << 16 ); }
		inline static s32 SetGreen24 ( u32 Color ) { return ( ( Color & 0xff ) << 8 ); }
		inline static s32 SetBlue24 ( u32 Color ) { return ( Color & 0xff ); }
		
		
		inline static u32 Clamp5 ( long n )
		{
			long a = 0x1f;
			a -= n;
			a >>= 31;
			a |= n;
			n >>= 31;
			n = ~n;
			n &= a;
			return n & 0x1f;
		}
		
		// signed clamp to use after an ADD operation
		// CLAMPTO - this is the number of bits to clamp to
		template<typename T, const int CLAMPTO>
		inline static T AddSignedClamp ( T n )
		{
			n &= ~( n >> ( ( sizeof ( T ) * 8 ) - 1 ) );
			return ( n | ( ( n << ( ( sizeof ( T ) * 8 ) - ( CLAMPTO + 1 ) ) ) >> ( ( sizeof ( T ) * 8 ) - 1 ) ) ) & ( ( 1 << CLAMPTO ) - 1 );
		}
		
		// unsigned clamp to use after an ADD operation
		// CLAMPTO - this is the number of bits to clamp to
		template<typename T, const int CLAMPTO>
		inline static T AddUnsignedClamp ( T n )
		{
			//n &= ~( n >> ( ( sizeof ( T ) * 8 ) - 1 ) );
			return ( n | ( ( n << ( ( sizeof ( T ) * 8 ) - ( CLAMPTO + 1 ) ) ) >> ( ( sizeof ( T ) * 8 ) - ( CLAMPTO + 1 ) ) ) ) & ( ( 1 << CLAMPTO ) - 1 );
		}
		
		// signed clamp to use after an ANY (MULTIPLY, etc) operation, but should take longer than the one above
		// CLAMPTO - this is the mask for number of bits to clamp to
		template<typename T, const int CLAMPTO>
		inline static T SignedClamp ( T n )
		{
			long a = ( ( 1 << CLAMPTO ) - 1 );	//0x1f;
			a -= n;
			a >>= ( ( sizeof ( T ) * 8 ) - 1 );	//31;
			a |= n;
			n >>= ( ( sizeof ( T ) * 8 ) - 1 );	//31;
			n = ~n;
			n &= a;
			return n & ( ( 1 << CLAMPTO ) - 1 );	//0x1f;
		}


		// signed clamp to use after an ANY (MULTIPLY, etc) operation, but should take longer than the one above
		// CLAMPTO - this is the mask for number of bits to clamp to
		template<typename T, const int CLAMPTO>
		inline static T UnsignedClamp ( T n )
		{
			long a = ( ( 1 << CLAMPTO ) - 1 );	//0x1f;
			a -= n;
			a >>= ( ( sizeof ( T ) * 8 ) - 1 );	//31;
			n |= a;
			//n >>= ( ( sizeof ( T ) * 8 ) - 1 );	//31;
			//n = ~n;
			//n &= a;
			return n & ( ( 1 << CLAMPTO ) - 1 );	//0x1f;
		}
		
		
		inline static u32 Clamp8 ( long n )
		{
			long a = 0xff;
			a -= n;
			a >>= 31;
			a |= n;
			n >>= 31;
			n = ~n;
			n &= a;
			return n & 0xff;
		}
		
		template<typename T>
		inline static void Swap ( T& Op1, T& Op2 )
		{
			//T Temp = Op1;
			//Op1 = Op2;
			//Op2 = Temp;
			Op1 ^= Op2;
			Op2 ^= Op1;
			Op1 ^= Op2;
		}
		

		inline static u32 Color16To24 ( u16 Color ) { return SetRed24(GetRed16(Color)<<3) | SetGreen24(GetGreen16(Color)<<3) | SetBlue24(GetBlue16(Color)<<3); }
		inline static u16 Color24To16 ( u32 Color ) { return SetRed16(GetRed24(Color)>>3) | SetGreen16(GetGreen24(Color)>>3) | SetBlue16(GetBlue24(Color)>>3); }
		
		inline static u16 MakeRGB16 ( u32 R, u32 G, u32 B ) { return ( ( R & 0x1f ) << 10 ) | ( ( G & 0x1f ) << 5 ) | ( B & 0x1f ); }
		inline static u32 MakeRGB24 ( u32 R, u32 G, u32 B ) { return ( ( R & 0xff ) << 16 ) | ( ( G & 0xff ) << 8 ) | ( B & 0xff ); }
		
		inline static u16 ColorMultiply16 ( u16 Color1, u16 Color2 )
		{
			return SetRed16 ( Clamp5 ( ( GetRed16 ( Color1 ) * GetRed16 ( Color2 ) ) >> 4 ) ) |
					SetGreen16 ( Clamp5 ( ( GetGreen16 ( Color1 ) * GetGreen16 ( Color2 ) ) >> 4 ) ) |
					SetBlue16 ( Clamp5 ( ( GetBlue16 ( Color1 ) * GetBlue16 ( Color2 ) ) >> 4 ) );
		}
		
		inline static u32 ColorMultiply24 ( u32 Color1, u32 Color2 )
		{
			return SetRed24 ( Clamp8 ( ( GetRed24 ( Color1 ) * GetRed24 ( Color2 ) ) >> 7 ) ) |
					SetGreen24 ( Clamp8 ( ( GetGreen24 ( Color1 ) * GetGreen24 ( Color2 ) ) >> 7 ) ) |
					SetBlue24 ( Clamp8 ( ( GetBlue24 ( Color1 ) * GetBlue24 ( Color2 ) ) >> 7 ) );
		}
		
		inline static u16 ColorMultiply1624 ( u64 Color16, u64 Color24 )
		{
			static const u32 c_iBitsPerPixel16 = 5;
			static const u32 c_iRedShift16 = c_iBitsPerPixel16 * 2;
			static const u32 c_iRedMask16 = ( 0x1f << c_iRedShift16 );
			static const u32 c_iGreenShift16 = c_iBitsPerPixel16 * 1;
			static const u32 c_iGreenMask16 = ( 0x1f << c_iGreenShift16 );
			static const u32 c_iBlueShift16 = 0;
			static const u32 c_iBlueMask16 = ( 0x1f << c_iBlueShift16 );
		
			static const u32 c_iBitsPerPixel24 = 8;
			static const u32 c_iRedShift24 = c_iBitsPerPixel24 * 2;
			static const u32 c_iRedMask24 = ( 0xff << ( c_iBitsPerPixel24 * 2 ) );
			static const u32 c_iGreenShift24 = c_iBitsPerPixel24 * 1;
			static const u32 c_iGreenMask24 = ( 0xff << ( c_iBitsPerPixel24 * 1 ) );
			static const u32 c_iBlueShift24 = 0;
			static const u32 c_iBlueMask24 = ( 0xff << ( c_iBitsPerPixel24 * 0 ) );
			
			s64 Red, Green, Blue;
			
			// the multiply should put it in 16.23 fixed point, but need it back in 8.8
			Red = ( ( Color16 & c_iRedMask16 ) * ( Color24 & c_iRedMask24 ) );
			Red |= ( ( Red << ( 64 - ( 16 + 23 ) ) ) >> 63 );
			
			// to get to original position, shift back ( 23 - 8 ) = 15, then shift right 7, for total of 7 + 15 = 22 shift right
			// top bit (38) needs to end up in bit 15, so that would actually shift right by 23
			Red >>= 23;
			
			// the multiply should put it in 16.10 fixed point, but need it back in 8.30
			Green = ( ( (u32) ( Color16 & c_iGreenMask16 ) ) * ( (u32) ( Color24 & c_iGreenMask24 ) ) );
			Green |= ( ( Green << ( 64 - ( 16 + 10 ) ) ) >> 63 );
			
			// top bit (25) needs to end up in bit (10)
			Green >>= 15;
			
			// the multiply should put it in 13.0 fixed point
			Blue = ( ( (u16) ( Color16 & c_iBlueMask16 ) ) * ( (u16) ( Color24 & c_iBlueMask24 ) ) );
			Blue |= ( ( Blue << ( 64 - ( 13 + 0 ) ) ) >> 63 );
			
			// top bit (12) needs to end up in bit 5
			Blue >>= 7;
			
			return ( Red & c_iRedMask16 ) | ( Green & c_iGreenMask16 ) | ( Blue & c_iBlueMask16 );
			
			
			//return SetRed24 ( Clamp8 ( ( GetRed24 ( Color1 ) * GetRed24 ( Color2 ) ) >> 7 ) ) |
			//		SetGreen24 ( Clamp8 ( ( GetGreen24 ( Color1 ) * GetGreen24 ( Color2 ) ) >> 7 ) ) |
			//		SetBlue24 ( Clamp8 ( ( GetBlue24 ( Color1 ) * GetBlue24 ( Color2 ) ) >> 7 ) );
		}


		inline static u16 ColorMultiply241624 ( u64 Color16, u64 Color24 )
		{
			static const u32 c_iBitsPerPixel16 = 5;
			static const u32 c_iRedShift16 = c_iBitsPerPixel16 * 2;
			static const u32 c_iRedMask16 = ( 0x1f << c_iRedShift16 );
			static const u32 c_iGreenShift16 = c_iBitsPerPixel16 * 1;
			static const u32 c_iGreenMask16 = ( 0x1f << c_iGreenShift16 );
			static const u32 c_iBlueShift16 = 0;
			static const u32 c_iBlueMask16 = ( 0x1f << c_iBlueShift16 );
		
			static const u32 c_iBitsPerPixel24 = 8;
			static const u32 c_iRedShift24 = c_iBitsPerPixel24 * 2;
			static const u32 c_iRedMask24 = ( 0xff << ( c_iBitsPerPixel24 * 2 ) );
			static const u32 c_iGreenShift24 = c_iBitsPerPixel24 * 1;
			static const u32 c_iGreenMask24 = ( 0xff << ( c_iBitsPerPixel24 * 1 ) );
			static const u32 c_iBlueShift24 = 0;
			static const u32 c_iBlueMask24 = ( 0xff << ( c_iBitsPerPixel24 * 0 ) );
			
			s64 Red, Green, Blue;
			
			// the multiply should put it in 16.23 fixed point, but need it back in 8.8
			Red = ( ( Color16 & c_iRedMask16 ) * ( Color24 & c_iRedMask24 ) );
			Red |= ( ( Red << ( 64 - ( 16 + 23 ) ) ) >> 63 );
			
			// to get to original position, shift back ( 23 - 8 ) = 15, then shift right 7, for total of 7 + 15 = 22 shift right
			// top bit (38) needs to end up in bit 23, so that would actually shift right by 15
			Red >>= 15;
			
			// the multiply should put it in 16.10 fixed point, but need it back in 8.30
			Green = ( ( Color16 & c_iGreenMask16 ) * ( Color24 & c_iGreenMask24 ) );
			Green |= ( ( Green << ( 64 - ( 16 + 10 ) ) ) >> 63 );
			
			// top bit (25) needs to end up in bit 15, so shift right by 10
			Green >>= 10;
			
			// the multiply should put it in 13.0 fixed point
			Blue = ( ( Color16 & c_iBlueMask16 ) * ( Color24 & c_iBlueMask24 ) );
			Blue |= ( ( Blue << ( 64 - ( 13 + 0 ) ) ) >> 63 );
			
			// top bit (12) needs to end up in bit 7
			Blue >>= 5;
			
			return ( Red & c_iRedMask24 ) | ( Green & c_iGreenMask24 ) | ( Blue & c_iBlueMask24 );
			
			
			//return SetRed24 ( Clamp8 ( ( GetRed24 ( Color1 ) * GetRed24 ( Color2 ) ) >> 7 ) ) |
			//		SetGreen24 ( Clamp8 ( ( GetGreen24 ( Color1 ) * GetGreen24 ( Color2 ) ) >> 7 ) ) |
			//		SetBlue24 ( Clamp8 ( ( GetBlue24 ( Color1 ) * GetBlue24 ( Color2 ) ) >> 7 ) );
		}
		
		
		inline static u32 ConvertPixel16To24 ( u32 Pixel16 )
		{
			u32 Pixel32;
			Pixel32 = ( Pixel16 & 0x1f ) << 3;
			Pixel32 |= ( Pixel16 & 0x3e0 ) << 6;
			Pixel32 |= ( Pixel16 & 0x7c00 ) << 9;
			return Pixel32;
		}

		
		// does ( A - B ) * C + D
		u32 AlphaABCD_32 ( u32 A, u32 B, u32 C, u32 D );
		u32 AlphaABCD_16 ( u32 A, u32 B, u32 C, u32 D );
		
		
		static TEST_t *pTest;



// 32-bit draw helper functions //		
		
inline u32* GetZBufPtr32 ( u32* zbuf32, u32 x, u32 y, u32 FrameBufferWidthInPixels )
{
	switch ( ZBUF_X.PSM )
	{
		// PSMZ32
		case 0:
		
		// PSMZ24
		case 1:
			return (u32*) ( & ( zbuf32 [ CvtAddrZBuf32 ( x, y, FrameBufferWidthInPixels ) ] ) );
			break;
			
		// PSMZ16
		case 2:
			// not valid for a 32-bit frame buffer
			//return (u32*) ( & ( ((u16*) zbuf32) [ CvtAddrZBuf16 ( x, y, FrameBufferWidthInPixels ) ] ) );
			
#ifdef VERBOSE_ZBUFFER
			// need to alert, as this shouldn't happen
			cout << "\nhps2x64: GPU: ALERT: PSMZ16 z-buffer format on a 32-bit frame buffer!\n";
#endif
			break;
			
		// PSMZ16S
		case 0xa:
			return (u32*) ( & ( ((u16*) zbuf32) [ CvtAddrZBuf16S ( x, y, FrameBufferWidthInPixels ) ] ) );
			break;
			
		// OTHER
		default:
#ifdef VERBOSE_ZBUFFER
			cout << "\nhps2x64: GPU: ALERT: invalid z-buffer format on a 32-bit frame buffer! ZBUF PSM=" << hex << ZBUF_X.PSM << "\n";
#endif
			break;
	}
	
	return (u32*) &zbuf_NULL;
}


inline u32 GetZBufValue32 ( u32* zptr32 )
{
	switch ( ZBUF_X.PSM )
	{
		// PSMZ32
		case 0:
			return ( *zptr32 );
			break;
		
		// PSMZ24
		case 1:
#ifdef ENABLE_ZREAD_SHIFT
			return ( ( *zptr32 ) << 8 );
#else
			return ( ( *zptr32 ) & 0xffffff );
#endif
			break;
			
		// PSMZ16
		case 2:
			// need to alert, as this shouldn't happen
			//cout << "\nhps2x64: GPU: ALERT: PSMZ16 z-buffer format on a 32-bit frame buffer!\n";
			
#ifdef ENABLE_ZREAD_SHIFT
			return ( ( (u32) ( *((u16*)zptr32) ) ) << 16 );
#else
			return ( ( (u32) ( *((u16*)zptr32) ) ) );
#endif
			break;
			
		// PSMZ16S
		case 0xa:
#ifdef ENABLE_ZREAD_SHIFT
			return ( ( (u32) ( *((u16*)zptr32) ) ) << 16 );
#else
			return ( ( (u32) ( *((u16*)zptr32) ) ) );
#endif
			break;
			
		default:
			cout << "\nhps2x64: GPU: ALERT: Invalid z-buffer pixel format: " << hex << ZBUF_X.PSM << "\n";
			break;
	}
}


inline u32 TestSrcAlpha32 ( u32 bgr )
{
	// needs local variable: bgr_temp
	// returns: SrcAlphaTest_Pass
	switch ( TEST_X.ATE )
	{
		case 0:
			return 1;
			break;
			
		case 1:
		
			//SrcAlphaTest_Pass = 0;
			
			switch ( TEST_X.ATST )
			{
				// NEVER
				case 0:
					return 0;
					break;
					
				// ALWAYS
				case 1:
					return 1;
					break;
					
				// LESS
				case 2:
					// bottom 24 bits of SrcAlpha_ARef need to be cleared out first
					if ( bgr < SrcAlpha_ARef ) return 1;
					break;
					
				// LESS OR EQUAL
				case 3:
					// bottom 24 bits of SrcAlpha_ARef need to be set first
					if ( bgr <= SrcAlpha_ARef ) return 1;
					break;
					
				// EQUAL
				case 4:
					// SrcAlpha_ARef need to be shifted right 24 first
					if ( ( bgr >> 24 ) == SrcAlpha_ARef ) return 1;
					break;
					
				// GREATER OR EQUAL
				case 5:
					// bottom 24 bits of SrcAlpha_ARef need to be cleared out first
					if ( bgr >= SrcAlpha_ARef ) return 1;
					break;
					
				// GREATER
				case 6:
					// bottom 24 bits of SrcAlpha_ARef need to be set first
					if ( bgr > SrcAlpha_ARef ) return 1;
					break;
					
				// NOT EQUAL
				case 7:
					// SrcAlpha_ARef need to be shifted right 24 first
					if ( ( bgr >> 24 ) != SrcAlpha_ARef ) return 1;
					break;
			}
			
			break;
	}
	
	// otherwise, return zero
	return 0;
}


inline void WriteZBufValue32 ( u32* zptr32, u32 ZValue32 )
{
	// first need to make sure we are not writing outside GPU RAM
	if ( zptr32 < PtrEnd )
	{
		// determine z-buffer format
		// and then store value back to z-buffer
		switch ( ZBUF_X.PSM )
		{
			// PSMZ32
			case 0:
				*zptr32 = ZValue32;
				break;
			
			// PSMZ24
			case 1:
#ifdef ENABLE_ZSTORE_SHIFT
				// note: probably don't want to destroy the upper 8-bits here
				*zptr32 = ( *zptr32 & 0xff000000 ) | ( ( ZValue32 >> 8 ) & 0xffffff );
#else
				// note: probably don't want to destroy the upper 8-bits here
				*zptr32 = ( *zptr32 & 0xff000000 ) | ( ( ZValue32 ) & 0xffffff );
#endif
				break;
				
			// PSMZ16
			case 2:
				// need to alert, as this shouldn't happen
				//cout << "\nhps2x64: GPU: ALERT: PSMZ16 z-buffer format on a 32-bit frame buffer!\n";
#ifdef ENABLE_ZSTORE_SHIFT
				*((u16*)zptr32) = ZValue32 >> 16;
#else
				*((u16*)zptr32) = (u16) ZValue32;
#endif
				break;
				
			// PSMZ16S
			case 0xa:
#ifdef ENABLE_ZSTORE_SHIFT
				*((u16*)zptr32) = ZValue32 >> 16;
#else
				*((u16*)zptr32) = (u16) ZValue32;
#endif
				break;
				
			// OTHER
			default:
				cout << "\nhps2x64: GPU: ALERT: invalid z-buffer format on a 32-bit frame buffer! ZBUF PSM=" << hex << ZBUF_X.PSM << "\n";
				break;
				
		} // end switch ( ZBUF_X.PSM )
	} // end if ( zptr32 < PtrEnd )
}


inline u32 PerformZDepthTest32 ( u32* zptr32, u32 ZValue32 )
{
	if ( !TEST_X.ZTE )
	{
		// depth test is OFF //
		
		// this is supposedly not allowed??
		return 1;
	}
	else
	{
//#define DEBUG_TEST22
#ifdef DEBUG_TEST22
	debug << "\r\n" << hex << ZValue32 << " vs " << GetZBufValue32 ( zptr32 );
#endif
		// initialize offset for z-buffer test //
		switch ( TEST_X.ZTST )
		{
			// NEVER pass
			case 0:
				return 0;
				break;
				
			// ALWAYS pass
			case 1:
				return 1;
				break;
				
			// GREATER OR EQUAL pass
			case 2:
				if ( ZValue32 >= GetZBufValue32 ( zptr32 ) ) return 1;
				return 0;
				break;
				
			// GREATER
			case 3:
				if ( ZValue32 > GetZBufValue32 ( zptr32 ) ) return 1;
				return 0;
				break;
		} // switch ( TEST_X.ZTST )
		
	} // if ( !TEST_X.ZTE )
	
	return 0;
}


inline u32 PerformZDepthTest16 ( u16* zptr16, u32 ZValue32 )
{
	if ( !TEST_X.ZTE )
	{
		// depth test is OFF //
		
		// this is supposedly not allowed??
		return 1;
	}
	else
	{
		// convert from 32-bit to 16-bit z-value //
		
		// the z-value is always specified as an unsigned 32-bit value
		//ZValue32 >>= 16;
		
		// perform the z-test //
		
		switch ( TEST_X.ZTST )
		{
			// NEVER pass
			case 0:
				return 0;
				break;
				
			// ALWAYS pass
			case 1:
				return 1;
				break;
				
			// GREATER OR EQUAL pass
			case 2:
				//if ( ZValue32 >= ( (u32) ( *zptr16 ) ) ) return 1;
				if ( ZValue32 >= GetZBufValue32 ( (u32*) zptr16 ) ) return 1;
				return 0;
				break;
				
			// GREATER
			case 3:
				//if ( ZValue32 > ( (u32) ( *zptr16 ) ) ) return 1;
				if ( ZValue32 > GetZBufValue32 ( (u32*) zptr16 ) ) return 1;
				return 0;
				break;
		} // end switch ( TEST_X.ZTST )
		
	} // if ( !TEST_X.ZTE )
		
	return 0;
}


inline void PerformAlphaFail32 ( u32* zptr32, u32* ptr32, u32 bgr, u32 DstPixel, s64 z0 )
{
	// source alpha test failed //
	
	switch ( TEST_X.AFAIL )
	{
		case 0:
			break;
		
		// update frame buffer ONLY
		case 1:
			// set MSB of pixel if specified before drawing
			bgr |= PixelOr32;
			
			bgr = ( bgr & FrameBuffer_WriteMask32 ) | ( DstPixel & ~FrameBuffer_WriteMask32 );
			*ptr32 = bgr;
			break;
			
		// *todo* update z-buffer ONLY
		case 2:
#ifdef ENABLE_DEPTH_TEST
			// store 32-bit depth value //
			
			// only store depth value if ZMSK is zero, which means to update z-buffer
			if ( TEST_X.ZTE && !ZBUF_X.ZMSK )
			{
			
				WriteZBufValue32 ( zptr32, z0 );
			
			} // end if ( !ZBUF_X.ZMSK )
#endif
			
			break;
			
		// update RGB ONLY (only available for 32-bit pixels)
		case 3:
			bgr = ( bgr & FrameBuffer_WriteMask32 ) | ( DstPixel & ~FrameBuffer_WriteMask32 );
			*ptr32 = ( DstPixel & 0xff000000 ) | ( bgr & ~0xff000000 );
			break;
	}
}


// 16-bit draw helper functions //		

inline u16* GetZBufPtr16 ( u16* zbuf16, u32 x, u32 y, u32 FrameBufferWidthInPixels )
{
	switch ( ZBUF_X.PSM )
	{
		// PSMZ32
		case 0:
		
		// PSMZ24
		case 1:
			return (u16*) ( & ( ( (u32*) zbuf16 ) [ CvtAddrZBuf32 ( x, y, FrameBufferWidthInPixels ) ] ) );
			break;
			
		// PSMZ16
		case 2:
#ifdef VERBOSE_ZBUFFER
			// need to alert, as this shouldn't happen
			if ( FrameBuffer_PixelFormat == 0xa ) cout << "\nhps2x64: GPU: ALERT: PSMZ16 z-buffer format on a PSMCT16S frame buffer!\n";
#endif
			
			// not valid for a 32-bit frame buffer
			return & ( zbuf16 [ CvtAddrZBuf16 ( x, y, FrameBufferWidthInPixels ) ] );
			break;
			
		// PSMZ16S
		case 0xa:
#ifdef VERBOSE_ZBUFFER
			// need to alert, as this shouldn't happen
			if ( FrameBuffer_PixelFormat == 0x2 ) cout << "\nhps2x64: GPU: ALERT: PSMZ16S z-buffer format on a PSMCT16 frame buffer!\n";
#endif
			
			return & ( zbuf16 [ CvtAddrZBuf16S ( x, y, FrameBufferWidthInPixels ) ] );
			break;
			
		// OTHER
		default:
#ifdef VERBOSE_ZBUFFER
			cout << "\nhps2x64: GPU: ALERT: invalid z-buffer format on a 16-bit frame buffer! ZBUF PSM=" << hex << ZBUF_X.PSM << "\n";
#endif
			break;
	}
	
	return (u16*) &zbuf_NULL;
}


inline void PerformAlphaFail16 ( u16* zptr16, u16* ptr16, u32 bgr, u32 DstPixel, s64 z0 )
{
	// source alpha test failed //
	
	switch ( TEST_X.AFAIL )
	{
		case 0:
			break;
			
		// update frame buffer ONLY
		case 1:
			// set MSB if specified before drawing
			bgr |= PixelOr16;
			
			bgr = ( bgr & FrameBuffer_WriteMask16 ) | ( DstPixel & ~FrameBuffer_WriteMask16 );
			*ptr16 = bgr;
			break;
			
		// *todo* update z-buffer ONLY
		case 2:
#ifdef ENABLE_DEPTH_TEST
			// only store 16-bit depth value if ZMSK is zero, which means to update z-buffer
			if ( TEST_X.ZTE && !ZBUF_X.ZMSK )
			{
				// here, z-buffer must be 16-bit
				// *zptr16 = z0 >> 16;
				WriteZBufValue32 ( (u32*) zptr16, z0 );
				
			} // end if ( !ZBUF_X.ZMSK )
#endif

			break;
			
		// update RGB ONLY (only available for 32-bit pixels)
		// probably available if the source pixel is 32-bits??
		case 3:
			bgr = ( bgr & FrameBuffer_WriteMask16 ) | ( DstPixel & ~FrameBuffer_WriteMask16 );
			*ptr16 = ( DstPixel & 0x8000 ) | ( bgr & ~0x8000 );
			
#ifdef VERBOSE_RGBONLY
			// probably available if source pixel is 32-bits??
			cout << "\nhps2x64: ERROR?: GPU: RGB ONLY for 16-bit frame buffer.\n";
#endif
			break;
	}
}


// color clamping on PS2 using COLCLAMP register //

inline s32 ColClamp8 ( s32 Component )
{
	// if COLCLAMP then clamp, otherwise just mask to 8-bits
	if ( GPURegsGp.COLCLAMP & 1 )
	{
		Component = SignedClamp<s32,8> ( Component );
	}
	else
	{
		Component &= 0xff;
	}
	
	return Component;
}


inline s32 ColClamp5 ( s32 Component )
{
	// if COLCLAMP then clamp, otherwise just mask to 8-bits
	if ( GPURegsGp.COLCLAMP & 1 )
	{
		Component = SignedClamp<s32,5> ( Component );
	}
	else
	{
		Component &= 0x1f;
	}
	
	return Component;
}


// texture function //

inline u32 TextureFunc32 ( u32 bgr, u32 shade1, u32 shade2, u32 shade3, u32 shade_a )
{
	u32 c1, c2, c3, ca;
	
	// get components from pixel
	ca = ( bgr >> 24 ) & 0xff;
	c3 = ( bgr >> 16 ) & 0xff;
	c2 = ( bgr >> 8 ) & 0xff;
	c1 = ( bgr >> 0 ) & 0xff;
	
	// get pixel color
	switch ( TEXX [ Ctx ].TFX )
	{
		// MODULATE
		case 0:
			c1 = UnsignedClamp<u32,8> ( ( c1 * shade1 ) >> 7 );
			c2 = UnsignedClamp<u32,8> ( ( c2 * shade2 ) >> 7 );
			c3 = UnsignedClamp<u32,8> ( ( c3 * shade3 ) >> 7 );
		
			break;
			
			
		// HIGHLIGHT
		case 2:
		
		// HIGHLIGHT2
		case 3:
		
			c1 = UnsignedClamp<u32,8> ( ( c1 * shade1 ) >> 7 );
			c2 = UnsignedClamp<u32,8> ( ( c2 * shade2 ) >> 7 );
			c3 = UnsignedClamp<u32,8> ( ( c3 * shade3 ) >> 7 );
			
			c1 += shade_a;
			c2 += shade_a;
			c3 += shade_a;
			
			c1 = ColClamp8 ( c1 );
			c2 = ColClamp8 ( c2 );
			c3 = ColClamp8 ( c3 );
			break;
	}
	
	
	// determine alpha
	switch ( TEXX [ Ctx ].TCC )
	{
		// RGB
		case 0:
		
			// alpha value is from shading color
			ca = shade_a;
			break;
			
		// RGBA
		case 1:
		
			// use TEXA register if pixel format is not 32-bit to get alpha value
			
			// calculate alpha
			switch ( TEXX [ Ctx ].TFX )
			{
				// MODULATE
				case 0:
					ca = UnsignedClamp<u32,8> ( ( ca * shade_a ) >> 7 );
					break;
				
				// HIGHLIGHT
				case 2:
					ca = ColClamp8 ( ca + shade_a );
					break;
			}
			
			break;
	}
	
	// return the pixel
	return ( c1 ) | ( c2 << 8 ) | ( c3 << 16 ) | ( ca << 24 );
}


// only the bgr is 16-bit here with 8-bit alpha included
inline u32 TextureFunc16 ( u32 bgr, u32 shade1, u32 shade2, u32 shade3, u32 shade_a )
{
	u32 c1, c2, c3, ca;
	
	// get components from pixel
	ca = ( bgr >> 24 ) & 0xff;
	c3 = ( bgr >> 10 ) & 0x1f;
	c2 = ( bgr >> 5 ) & 0x1f;
	c1 = ( bgr >> 0 ) & 0x1f;
	
	// get pixel color
	switch ( TEXX [ Ctx ].TFX )
	{
		// MODULATE
		case 0:
			c1 = UnsignedClamp<u32,5> ( ( c1 * shade1 ) >> 7 );
			c2 = UnsignedClamp<u32,5> ( ( c2 * shade2 ) >> 7 );
			c3 = UnsignedClamp<u32,5> ( ( c3 * shade3 ) >> 7 );
		
			break;
			
			
		// HIGHLIGHT
		case 2:
		
		// HIGHLIGHT2
		case 3:
		
			c1 = UnsignedClamp<u32,8> ( ( c1 * shade1 ) >> 4 );
			c2 = UnsignedClamp<u32,8> ( ( c2 * shade2 ) >> 4 );
			c3 = UnsignedClamp<u32,8> ( ( c3 * shade3 ) >> 4 );
			
			c1 += shade_a;
			c2 += shade_a;
			c3 += shade_a;
			
			c1 = ColClamp5 ( c1 >> 3 );
			c2 = ColClamp5 ( c2 >> 3 );
			c3 = ColClamp5 ( c3 >> 3 );
			break;
	}
	
	
	// determine alpha
	switch ( TEXX [ Ctx ].TCC )
	{
		// RGB
		case 0:
		
			// alpha value is from shading color
			ca = shade_a;
			break;
			
		// RGBA
		case 1:
		
			// use TEXA register if pixel format is not 32-bit to get alpha value
			
			// calculate alpha
			switch ( TEXX [ Ctx ].TFX )
			{
				// MODULATE
				case 0:
					ca = UnsignedClamp<u32,8> ( ( ca * shade_a ) >> 7 );
					break;
				
				// HIGHLIGHT
				case 2:
					ca = ColClamp8 ( ca + shade_a );
					break;
			}
			
			break;
	}
	
	// return the pixel
	return ( c1 ) | ( c2 << 5 ) | ( c3 << 10 ) | ( ca << 24 ) | ( ( ca & 0x80 ) << 8 );
}



// fog function //

inline u32 FogFunc32 ( u32 bgr, u32 FogCoef )
{
	u32 cr, cg, cb, ca, rf, fogr, fogg, fogb;
	
	rf = 0xff - FogCoef;
	
	// get components from pixel
	ca = ( bgr >> 24 ) & 0xff;
	cr = ( bgr >> 0 ) & 0xff;
	cg = ( bgr >> 8 ) & 0xff;
	cb = ( bgr >> 16 ) & 0xff;
	
	// get components from fog color
	fogr = ( GPURegsGp.FOGCOL >> 0 ) & 0xff;
	fogg = ( GPURegsGp.FOGCOL >> 8 ) & 0xff;
	fogb = ( GPURegsGp.FOGCOL >> 16 ) & 0xff;
	
	cr = ColClamp8 ( ( ( FogCoef * cr ) >> 8 ) + ( ( rf * fogr ) >> 8 ) );
	cg = ColClamp8 ( ( ( FogCoef * cg ) >> 8 ) + ( ( rf * fogg ) >> 8 ) );
	cb = ColClamp8 ( ( ( FogCoef * cb ) >> 8 ) + ( ( rf * fogb ) >> 8 ) );
	
	// return the pixel
	return ( cr ) | ( cg << 8 ) | ( cb << 16 ) | ( ca << 24 );
}


inline u32 FogFunc16 ( u32 bgr, u32 FogCoef )
{
	u32 cr, cg, cb, ca, rf, fogr, fogg, fogb;
	
	rf = 0xff - FogCoef;
	
	// get components from pixel
	ca = ( bgr >> 24 ) & 0xff;
	cr = ( bgr >> 0 ) & 0x1f;
	cg = ( bgr >> 5 ) & 0x1f;
	cb = ( bgr >> 10 ) & 0x1f;
	
	// get components from fog color
	fogr = ( GPURegsGp.FOGCOL >> 0 ) & 0xff;
	fogg = ( GPURegsGp.FOGCOL >> 8 ) & 0xff;
	fogb = ( GPURegsGp.FOGCOL >> 16 ) & 0xff;
	
	cr = ColClamp5 ( ( ( ( FogCoef * cr ) >> 5 ) + ( ( rf * fogr ) >> 8 ) ) >> 3 );
	cg = ColClamp5 ( ( ( ( FogCoef * cg ) >> 5 ) + ( ( rf * fogg ) >> 8 ) ) >> 3 );
	cb = ColClamp5 ( ( ( ( FogCoef * cb ) >> 5 ) + ( ( rf * fogb ) >> 8 ) ) >> 3 );
	
	// return the pixel
	return ( cr ) | ( cg << 5 ) | ( cb << 10 ) | ( ca << 24 ) | ( ( ca & 0x80 ) << 8 );
}


// pixel plotting functions (for simplification) //


// this one expects the pixel to be already 16/32-bit depending on whether it is a 16 or 32 bit framebuffer
inline void PlotPixel_Mono ( s32 x0, s32 y0, s64 z0, u32 bgr )
{
	u32 DestPixel;
	u32 bgr_temp;
	u32 SrcAlphaTest_Pass;
	u32 ZDepthTest_Pass;

	union
	{
		u32 *ptr32;
		u16 *ptr16;
	};
	
	union
	{
		u32 *zptr32;
		u16 *zptr16;
	};
	

	//if ( FrameBuffer_PixelFormat < 2 )
	if ( ! ( FrameBuffer_PixelFormat & 2 ) )
	{
		// 32-bit frame buffer //

		// get pointer into frame buffer //
		
		//ptr32 = & ( buf32 [ CvtAddrPix32 ( x0, y0, FrameBufferWidth_Pixels ) ] );
		
		switch ( FrameBuffer_PixelFormat )
		{
			// PSMCT32
			case 0:
			
			// PSMCT24
			case 1:
				ptr32 = & ( buf32 [ CvtAddrPix32 ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
				
			// PSMZ32
			case 0x30:
			
			// PSMZ24
			case 0x31:
				ptr32 = & ( buf32 [ CvtAddrZBuf32 ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
		}

		
		if ( ptr32 < PtrEnd )
		{

#ifdef ENABLE_DEPTH_TEST
			// perform depth test //
			
			// get pointer into depth buffer //
			zptr32 = GetZBufPtr32 ( zbuf32, x0, y0, FrameBufferWidth_Pixels );
			
			// determine z-buffer format
			// and test against the value in the z-buffer
			//ZBufValue = GetZBufValue32 ( zptr32 );
			ZDepthTest_Pass = PerformZDepthTest32 ( zptr32, z0 );
			
			// z-buffer test
			//if ( ( z0 + DepthTest_Offset ) > ZBufValue )
			if ( ZDepthTest_Pass )
			{
#endif

			
			DestPixel = *ptr32;

			
#ifdef ENABLE_DEST_ALPHA_TEST
			// destination alpha test
			if ( !( ( DestPixel ^ DA_Test ) & DA_Enable ) )
			{
				// passed destination alpha test //
#endif


			bgr_temp = bgr;

			
#ifdef ENABLE_ALPHA_POINT
			//if ( Alpha /* && !( ( bgr ^ AlphaXor32 ) >> 31 ) */ )
			if ( Alpha && !( ( bgr ^ AlphaXor32 ) & AlphaXor32 ) )
			{
				//AlphaSelect [ 0 ] = bgr_temp;
				AlphaSelect [ 1 ] = ( DestPixel & DestMask24 ) | DestAlpha24;
				
				// need to keep the source alpha
				bgr_temp = AlphaABCD_32 ( AlphaSelect [ uA ], AlphaSelect [ uB ], AlphaSelect [ uC ], AlphaSelect [ uD ] );
				
				// re-add source alpha
				bgr_temp |= ( bgr & 0xff000000 );
			}
#endif
			
						

			
#ifdef ENABLE_SRC_ALPHA_TEST

			SrcAlphaTest_Pass = TestSrcAlpha32 ( bgr_temp );

			// source alpha test
			if ( SrcAlphaTest_Pass )
			{
				// source alpha test passed //
#endif

				// store pixel and z-value //

				// this should go AFTER the source alpha test?
				bgr_temp |= PixelOr32;
				
				// draw pixel if we can draw to mask pixels or mask bit not set
				// ***todo*** PS2 pixel mask
				// *ptr32 = ( bgr_temp | PixelOr32 );
				
				// draw pixel //
				
				bgr_temp = ( bgr_temp & FrameBuffer_WriteMask32 ) | ( DestPixel & ~FrameBuffer_WriteMask32 );
				*ptr32 = bgr_temp;

			
#ifdef ENABLE_DEPTH_TEST
				// store depth value //
				// local variable input: zbuf ptr, zvalue
				
				// only store depth value if ZMSK is zero, which means to update z-buffer
				if ( TEST_X.ZTE && !ZBUF_X.ZMSK )
				{
				
					WriteZBufValue32 ( zptr32, z0 );
					
				} // end if ( !ZBUF_X.ZMSK )
			
#endif

#ifdef ENABLE_SRC_ALPHA_TEST
			}
			else
			{
				// source alpha test failed //
				
				PerformAlphaFail32 ( zptr32, ptr32, bgr_temp, DestPixel, z0 );
				
			} // end if ( ( bgr_temp > GreaterOrEqualTo_And && bgr_temp < LessThan_And ) || ( bgr_temp < LessThan_Or ) )
#endif

			
#ifdef ENABLE_DEST_ALPHA_TEST			
			} // end if ( !( ( DestPixel ^ DA_Test ) & DA_Enable ) )
#endif

#ifdef ENABLE_DEPTH_TEST
			} // end if ( ( z0 + DepthTest_Offset ) > ZBufValue )
#endif

		} // end if ( ptr32 < PtrEnd )
			
	}
	else
	{
		// 16-bit frame buffer //

		// note: this actually needs to be done ahead of time, the conversion from 32-bit pixel to 16-bit pixel
		// convert to 16-bit pixel
		// must also convert alpha
		//bgr = Color24To16 ( bgr ) | ( ( bgr >> 16 ) & 0x8000 ) | ( bgr & 0xff000000 );
		
		// set the alpha
		//AlphaSelect [ 0 ] = bgr | ( AlphaSelect [ 0 ] & 0xff000000 );
		//AlphaSelect [ 0 ] = bgr;
		
		// get pointer into frame buffer //
		
		switch ( FrameBuffer_PixelFormat )
		{
			// PSMCT32
			case 0:
			
			// PSMCT24
			case 1:
				break;
				
			// PSMCT16
			case 2:
				//ptr16 = & ( buf16 [ x0 + ( y0 * FrameBufferWidth_Pixels ) ] );
				ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrPix16 ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
				
			// PSMCT16S
			case 0xa:
				ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrPix16S ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
				
			// PSMZ16
			case 0x32:
				ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrZBuf16 ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
				
			// PSMZ16S
			case 0x3a:
				ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrZBuf16S ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
		}


		
		
		if ( ptr16 < PtrEnd )
		{
		
#ifdef ENABLE_DEPTH_TEST
			// perform depth test //
			
			// get pointer into depth buffer //
			zptr16 = GetZBufPtr16 ( (u16*) zbuf32, x0, y0, FrameBufferWidth_Pixels );
			
			// determine z-buffer format
			// and test against the value in the z-buffer
					
			// PSMZ16
			// PSMZ16S
			//ZBufValue = ( (u64) ( *zptr16 ) ) << 16;
			ZDepthTest_Pass = PerformZDepthTest16 ( zptr16, z0 );
			
			// z-buffer test
			//if ( ( z0 + DepthTest_Offset ) > ZBufValue )
			if ( ZDepthTest_Pass )
			{
#endif

			
			DestPixel = *ptr16;
			
			
#ifdef ENABLE_DEST_ALPHA_TEST			
			// destination alpha test
			if ( !( ( DestPixel ^ DA_Test ) & DA_Enable ) )
			{
				// passed destination alpha test //
#endif


			bgr_temp = bgr;

			
#ifdef ENABLE_ALPHA_POINT
			//if ( Alpha /* && !( ( bgr ^ AlphaXor32 ) >> 31 ) */ )
			if ( Alpha && !( ( bgr ^ AlphaXor32 ) & AlphaXor32 ) )
			{
				//AlphaSelect [ 0 ] = bgr_temp;
				
				// for 16-bit frame buffer, destination alpha is A shifted left 7
				AlphaSelect [ 1 ] = ( (u32) ( DestPixel ) ) | ( ( (u32) ( DestPixel & 0x8000 ) ) << 16 );
				
				// need to keep the source alpha
				bgr_temp = AlphaABCD_16 ( AlphaSelect [ uA ], AlphaSelect [ uB ], AlphaSelect [ uC ], AlphaSelect [ uD ] );
				
				// re-add source alpha
				// ***TODO*** ***IMPORTANT*** look out for the source alpha, probably should be getting top 8-bits also here?
				//bgr_temp |= ( AlphaSelect [ 0 ] & 0x8000 );
				bgr_temp |= ( bgr & 0xff008000 );
				
			} // end if ( Alpha /* && !( ( bgr ^ AlphaXor32 ) >> 31 ) */ )
#endif
			
						

#ifdef ENABLE_SRC_ALPHA_TEST

			// same 32-bit code should apply to 16-bit pixels theoretically
			SrcAlphaTest_Pass = TestSrcAlpha32 ( bgr_temp );
			
			// source alpha test
			if ( SrcAlphaTest_Pass )
			{
				// source alpha test passed //
#endif

			// set MSB if specified AFTER alpha test?
			bgr_temp |= PixelOr16;

			// draw pixel //
			
			bgr_temp = ( bgr_temp & FrameBuffer_WriteMask16 ) | ( DestPixel & ~FrameBuffer_WriteMask16 );
			*ptr16 = bgr_temp;

#ifdef ENABLE_DEPTH_TEST
			// only store depth value if ZMSK is zero, which means to update z-buffer
			if ( TEST_X.ZTE && !ZBUF_X.ZMSK )
			{
				// here, z-buffer must be 16-bit
				// *zptr16 = z0 >> 16;
				WriteZBufValue32 ( (u32*) zptr16, z0 );
				
			} // end if ( !ZBUF_X.ZMSK )
#endif

			
#ifdef ENABLE_SRC_ALPHA_TEST
			}
			else
			{
				// source alpha test failed //
				
				PerformAlphaFail16 ( zptr16, ptr16, bgr_temp, DestPixel, z0 );
				
			} // end if ( ( bgr_temp > GreaterOrEqualTo_And && bgr_temp < LessThan_And ) || ( bgr_temp < LessThan_Or ) )
#endif
			
#ifdef ENABLE_DEST_ALPHA_TEST			
			} // end if ( !( ( DestPixel ^ DA_Test ) & DA_Enable ) )
#endif

#ifdef ENABLE_DEPTH_TEST
			} // end if ( ( z0 + DepthTest_Offset ) > ZBufValue )
#endif

		} // end if ( ptr16 < PtrEnd )
	}

}



// this one always takes in a 32-bit pixel and will convert it as needed
inline void PlotPixel_Gradient ( s32 x0, s32 y0, s64 z0, u32 bgr )
{
	u32 DestPixel;
	u32 bgr_temp;
	u32 SrcAlphaTest_Pass;
	u32 ZDepthTest_Pass;

	union
	{
		u32 *ptr32;
		u16 *ptr16;
	};
	
	union
	{
		u32 *zptr32;
		u16 *zptr16;
	};
	

	//if ( FrameBuffer_PixelFormat < 2 )
	if ( ! ( FrameBuffer_PixelFormat & 2 ) )
	{
		// 32-bit frame buffer //

		// get pointer into frame buffer //
		
		//ptr32 = & ( buf32 [ CvtAddrPix32 ( x0, y0, FrameBufferWidth_Pixels ) ] );
		
		switch ( FrameBuffer_PixelFormat )
		{
			// PSMCT32
			case 0:
			
			// PSMCT24
			case 1:
				ptr32 = & ( buf32 [ CvtAddrPix32 ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
				
			// PSMZ32
			case 0x30:
			
			// PSMZ24
			case 0x31:
				ptr32 = & ( buf32 [ CvtAddrZBuf32 ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
		}

		
		if ( ptr32 < PtrEnd )
		{

#ifdef ENABLE_DEPTH_TEST
			// perform depth test //
			
			// get pointer into depth buffer //
			zptr32 = GetZBufPtr32 ( zbuf32, x0, y0, FrameBufferWidth_Pixels );
			
			// determine z-buffer format
			// and test against the value in the z-buffer
			//ZBufValue = GetZBufValue32 ( zptr32 );
			ZDepthTest_Pass = PerformZDepthTest32 ( zptr32, z0 );
			
			// z-buffer test
			//if ( ( z0 + DepthTest_Offset ) > ZBufValue )
			if ( ZDepthTest_Pass )
			{
#endif

			
			DestPixel = *ptr32;

			
#ifdef ENABLE_DEST_ALPHA_TEST			
			// destination alpha test
			if ( !( ( DestPixel ^ DA_Test ) & DA_Enable ) )
			{
				// passed destination alpha test //
#endif


			bgr_temp = bgr;

			
#ifdef ENABLE_ALPHA_POINT
			//if ( Alpha /* && !( ( bgr ^ AlphaXor32 ) >> 31 ) */ )
			if ( Alpha && !( ( bgr ^ AlphaXor32 ) & AlphaXor32 ) )
			{
				AlphaSelect [ 0 ] = bgr;
				AlphaSelect [ 1 ] = ( DestPixel & DestMask24 ) | DestAlpha24;
				
				// need to keep the source alpha
				bgr_temp = AlphaABCD_32 ( AlphaSelect [ uA ], AlphaSelect [ uB ], AlphaSelect [ uC ], AlphaSelect [ uD ] );
				
				// re-add source alpha
				bgr_temp |= ( bgr & 0xff000000 );
			}
#endif
			
						

			
#ifdef ENABLE_SRC_ALPHA_TEST

			SrcAlphaTest_Pass = TestSrcAlpha32 ( bgr_temp );

			// source alpha test
			if ( SrcAlphaTest_Pass )
			{
				// source alpha test passed //
#endif

				// store pixel and z-value //

				// this should go AFTER the source alpha test?
				bgr_temp |= PixelOr32;
				
				// draw pixel if we can draw to mask pixels or mask bit not set
				// ***todo*** PS2 pixel mask
				// *ptr32 = ( bgr_temp | PixelOr32 );
				
				// draw pixel //
				
				bgr_temp = ( bgr_temp & FrameBuffer_WriteMask32 ) | ( DestPixel & ~FrameBuffer_WriteMask32 );
				*ptr32 = bgr_temp;

//debug << " pixel=" << ( *ptr32 );
			
#ifdef ENABLE_DEPTH_TEST
				// store depth value //
				// local variable input: zbuf ptr, zvalue
				
				// only store depth value if ZMSK is zero, which means to update z-buffer
				if ( TEST_X.ZTE && !ZBUF_X.ZMSK )
				{
				
					WriteZBufValue32 ( zptr32, z0 );
					
				} // end if ( !ZBUF_X.ZMSK )
			
#endif

#ifdef ENABLE_SRC_ALPHA_TEST
			}
			else
			{
				// source alpha test failed //
				
				PerformAlphaFail32 ( zptr32, ptr32, bgr_temp, DestPixel, z0 );
				
			} // end if ( ( bgr_temp > GreaterOrEqualTo_And && bgr_temp < LessThan_And ) || ( bgr_temp < LessThan_Or ) )
#endif

			
#ifdef ENABLE_DEST_ALPHA_TEST			
			} // end if ( !( ( DestPixel ^ DA_Test ) & DA_Enable ) )
#endif

#ifdef ENABLE_DEPTH_TEST
			} // end if ( ( z0 + DepthTest_Offset ) > ZBufValue )
#endif

		} // end if ( ptr32 < PtrEnd )
			
	}
	else
	{
		// 16-bit frame buffer //

		// note: this actually needs to be done ahead of time, the conversion from 32-bit pixel to 16-bit pixel
		// convert to 16-bit pixel
		// must also convert alpha
		bgr = Color24To16 ( bgr ) | ( ( bgr >> 16 ) & 0x8000 ) | ( bgr & 0xff000000 );
		
		// set the alpha
		//AlphaSelect [ 0 ] = bgr | ( AlphaSelect [ 0 ] & 0xff000000 );
		//AlphaSelect [ 0 ] = bgr;
		
		// get pointer into frame buffer //
		
		switch ( FrameBuffer_PixelFormat )
		{
			// PSMCT32
			case 0:
			
			// PSMCT24
			case 1:
				break;
				
			// PSMCT16
			case 2:
				//ptr16 = & ( buf16 [ x0 + ( y0 * FrameBufferWidth_Pixels ) ] );
				ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrPix16 ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
				
			// PSMCT16S
			case 0xa:
				ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrPix16S ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
				
			// PSMZ16
			case 0x32:
				ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrZBuf16 ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
				
			// PSMZ16S
			case 0x3a:
				ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrZBuf16S ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
		}


		
		
		if ( ptr16 < PtrEnd )
		{
		
#ifdef ENABLE_DEPTH_TEST
			// perform depth test //
			
			// get pointer into depth buffer //
			zptr16 = GetZBufPtr16 ( (u16*) zbuf32, x0, y0, FrameBufferWidth_Pixels );
			
			// determine z-buffer format
			// and test against the value in the z-buffer
					
			// PSMZ16
			// PSMZ16S
			//ZBufValue = ( (u64) ( *zptr16 ) ) << 16;
			ZDepthTest_Pass = PerformZDepthTest16 ( zptr16, z0 );
			
			// z-buffer test
			//if ( ( z0 + DepthTest_Offset ) > ZBufValue )
			if ( ZDepthTest_Pass )
			{
#endif

			
			DestPixel = *ptr16;
			
			
#ifdef ENABLE_DEST_ALPHA_TEST			
			// destination alpha test
			if ( !( ( DestPixel ^ DA_Test ) & DA_Enable ) )
			{
				// passed destination alpha test //
#endif


			bgr_temp = bgr;

			
#ifdef ENABLE_ALPHA_POINT
			//if ( Alpha /* && !( ( bgr ^ AlphaXor32 ) >> 31 ) */ )
			if ( Alpha && !( ( bgr ^ AlphaXor32 ) & AlphaXor32 ) )
			{
				//AlphaSelect [ 0 ] = bgr_temp;
				AlphaSelect [ 0 ] = bgr;
				
				// for 16-bit frame buffer, destination alpha is A shifted left 7
				AlphaSelect [ 1 ] = ( (u32) ( DestPixel ) ) | ( ( (u32) ( DestPixel & 0x8000 ) ) << 16 );
				
				// need to keep the source alpha
				bgr_temp = AlphaABCD_16 ( AlphaSelect [ uA ], AlphaSelect [ uB ], AlphaSelect [ uC ], AlphaSelect [ uD ] );
				
				// re-add source alpha
				// ***TODO*** ***IMPORTANT*** look out for the source alpha, probably should be getting top 8-bits also here?
				//bgr_temp |= ( AlphaSelect [ 0 ] & 0x8000 );
				bgr_temp |= ( bgr & 0xff008000 );
				
			} // end if ( Alpha /* && !( ( bgr ^ AlphaXor32 ) >> 31 ) */ )
#endif
			
						

#ifdef ENABLE_SRC_ALPHA_TEST

			// same 32-bit code should apply to 16-bit pixels theoretically
			SrcAlphaTest_Pass = TestSrcAlpha32 ( bgr_temp );
			
			// source alpha test
			if ( SrcAlphaTest_Pass )
			{
				// source alpha test passed //
#endif

			// set MSB if specified AFTER alpha test?
			bgr_temp |= PixelOr16;

			// draw pixel //
			
			bgr_temp = ( bgr_temp & FrameBuffer_WriteMask16 ) | ( DestPixel & ~FrameBuffer_WriteMask16 );
			*ptr16 = bgr_temp;

#ifdef ENABLE_DEPTH_TEST
			// only store depth value if ZMSK is zero, which means to update z-buffer
			if ( TEST_X.ZTE && !ZBUF_X.ZMSK )
			{
				// here, z-buffer must be 16-bit
				// *zptr16 = z0 >> 16;
				WriteZBufValue32 ( (u32*) zptr16, z0 );
				
			} // end if ( !ZBUF_X.ZMSK )
#endif

			
#ifdef ENABLE_SRC_ALPHA_TEST
			}
			else
			{
				// source alpha test failed //
				
				PerformAlphaFail16 ( zptr16, ptr16, bgr_temp, DestPixel, z0 );
				
			} // end if ( ( bgr_temp > GreaterOrEqualTo_And && bgr_temp < LessThan_And ) || ( bgr_temp < LessThan_Or ) )
#endif
			
#ifdef ENABLE_DEST_ALPHA_TEST			
			} // end if ( !( ( DestPixel ^ DA_Test ) & DA_Enable ) )
#endif

#ifdef ENABLE_DEPTH_TEST
			} // end if ( ( z0 + DepthTest_Offset ) > ZBufValue )
#endif

		} // end if ( ptr16 < PtrEnd )
	}

}


// this one always takes in a 32-bit pixel and will convert it as needed
inline void PlotPixel_Texture ( s32 x0, s32 y0, s64 z0, u32 bgr, u32 c1, u32 c2, u32 c3, u32 ca, u32 fv )
{
	u32 DestPixel;
	u32 bgr_temp;
	u32 SrcAlphaTest_Pass;
	u32 ZDepthTest_Pass;

	union
	{
		u32 *ptr32;
		u16 *ptr16;
	};
	
	union
	{
		u32 *zptr32;
		u16 *zptr16;
	};
	

	//if ( FrameBuffer_PixelFormat < 2 )
	if ( ! ( FrameBuffer_PixelFormat & 2 ) )
	{
		// 32-bit frame buffer //

		// get pointer into frame buffer //
		
		//ptr32 = & ( buf32 [ CvtAddrPix32 ( x0, y0, FrameBufferWidth_Pixels ) ] );
		
		switch ( FrameBuffer_PixelFormat )
		{
			// PSMCT32
			case 0:
			
			// PSMCT24
			case 1:
				ptr32 = & ( buf32 [ CvtAddrPix32 ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
				
			// PSMZ32
			case 0x30:
			
			// PSMZ24
			case 0x31:
				ptr32 = & ( buf32 [ CvtAddrZBuf32 ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
		}

		
		if ( ptr32 < PtrEnd )
		{
		
		
#ifdef ENABLE_KEYCOLOR
			// if 24-bit pixel, then should only need bottom 24-bits of the pixel
			bgr &= Pixel24_Mask;
		
			// check for transparent pixel ??
			if ( bgr | RGB24_TAlpha )
			{
#endif

#ifdef ENABLE_DEPTH_TEST
			// perform depth test //
			
			// get pointer into depth buffer //
			zptr32 = GetZBufPtr32 ( zbuf32, x0, y0, FrameBufferWidth_Pixels );
			
			
			// determine z-buffer format
			// and test against the value in the z-buffer
			//ZBufValue = GetZBufValue32 ( zptr32 );
			ZDepthTest_Pass = PerformZDepthTest32 ( zptr32, z0 );
			
			// z-buffer test
			//if ( ( z0 + DepthTest_Offset ) > ZBufValue )
			if ( ZDepthTest_Pass )
			{
#endif

//cout << " Ztest";
			
			DestPixel = *ptr32;


#ifdef ENABLE_DEST_ALPHA_TEST			
			// destination alpha test
			// if frame buffer is only 24-bits, then this always passes
			if ( !( ( DestPixel ^ DA_Test ) & DA_Enable ) )
			{
				// passed destination alpha test //
#endif

//cout << " DA Test";

			// should probably add alpha into the source bgr pixel here
			bgr |= RGB24_Alpha;
			
			// texture function ??
			bgr = TextureFunc32 ( bgr, c1, c2, c3, ca );
			
			// fogging ??
#ifdef ENABLE_FOG
			if ( FogEnable )
			{
				bgr = FogFunc32 ( bgr, fv );
			}
#endif

			bgr_temp = bgr;

			// add alpha value into pixel in case of RGB24
			//bgr_temp |= RGB24_Alpha;
			
#ifdef ENABLE_ALPHA_POINT
			//if ( Alpha && !( ( bgr ^ AlphaXor32 ) >> 31 ) )
			if ( Alpha && !( ( bgr ^ AlphaXor32 ) & AlphaXor32 ) )
			{
				AlphaSelect [ 0 ] = bgr_temp;
				AlphaSelect [ 1 ] = ( DestPixel & DestMask24 ) | DestAlpha24;
				
//cout << " A=" << hex << AlphaSelect [ uA ];
//cout << " B=" << hex << AlphaSelect [ uB ];
//cout << " C=" << hex << AlphaSelect [ uC ];
//cout << " D=" << hex << AlphaSelect [ uD ];

				// need to keep the source alpha
				bgr_temp = AlphaABCD_32 ( AlphaSelect [ uA ], AlphaSelect [ uB ], AlphaSelect [ uC ], AlphaSelect [ uD ] );
				
				// re-add source alpha
				bgr_temp |= ( bgr & 0xff000000 );
			} // if ( Alpha && !( ( bgr ^ AlphaXor32 ) >> 31 ) )
#endif
			
						

			
#ifdef ENABLE_SRC_ALPHA_TEST

			SrcAlphaTest_Pass = TestSrcAlpha32 ( bgr_temp );

			// source alpha test
			if ( SrcAlphaTest_Pass )
			{
				// source alpha test passed //
#endif

				// store pixel and z-value //

				// this should go AFTER the source alpha test?
				bgr_temp |= PixelOr32;
				
				// draw pixel if we can draw to mask pixels or mask bit not set
				// ***todo*** PS2 pixel mask
				// *ptr32 = ( bgr_temp | PixelOr32 );
				
				// draw pixel //
				
				bgr_temp = ( bgr_temp & FrameBuffer_WriteMask32 ) | ( DestPixel & ~FrameBuffer_WriteMask32 );
				*ptr32 = bgr_temp;

			
#ifdef ENABLE_DEPTH_TEST
				// store depth value //
				// local variable input: zbuf ptr, zvalue
				
				// only store depth value if ZMSK is zero, which means to update z-buffer
				if ( TEST_X.ZTE && !ZBUF_X.ZMSK )
				{
				
					WriteZBufValue32 ( zptr32, z0 );
					
				} // end if ( !ZBUF_X.ZMSK )
			
#endif

#ifdef ENABLE_SRC_ALPHA_TEST
			}
			else
			{
				// source alpha test failed //
				
				PerformAlphaFail32 ( zptr32, ptr32, bgr_temp, DestPixel, z0 );
				
			} // end if ( ( bgr_temp > GreaterOrEqualTo_And && bgr_temp < LessThan_And ) || ( bgr_temp < LessThan_Or ) )
#endif

			
#ifdef ENABLE_DEST_ALPHA_TEST			
			} // end if ( !( ( DestPixel ^ DA_Test ) & DA_Enable ) )
#endif

#ifdef ENABLE_DEPTH_TEST
			} // end if ( ( z0 + DepthTest_Offset ) > ZBufValue )
#endif

#ifdef ENABLE_KEYCOLOR
			} // end if ( bgr | RGB24_TAlpha )
#endif
			
		} // end if ( ptr32 < PtrEnd )
		
	}
	else
	{
		// 16-bit frame buffer //

		// note: this actually needs to be done ahead of time, the conversion from 32-bit pixel to 16-bit pixel
		// convert to 16-bit pixel
		// must also convert alpha
		bgr = Color24To16 ( bgr ) | ( ( bgr >> 16 ) & 0x8000 ) | ( bgr & 0xff000000 );
		
		// set the alpha
		//AlphaSelect [ 0 ] = bgr | ( AlphaSelect [ 0 ] & 0xff000000 );
		//AlphaSelect [ 0 ] = bgr;
		
		// get pointer into frame buffer //
		
		switch ( FrameBuffer_PixelFormat )
		{
			// PSMCT32
			case 0:
			
			// PSMCT24
			case 1:
				break;
				
			// PSMCT16
			case 2:
				//ptr16 = & ( buf16 [ x0 + ( y0 * FrameBufferWidth_Pixels ) ] );
				ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrPix16 ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
				
			// PSMCT16S
			case 0xa:
				ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrPix16S ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
				
			// PSMZ16
			case 0x32:
				ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrZBuf16 ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
				
			// PSMZ16S
			case 0x3a:
				ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrZBuf16S ( x0, y0, FrameBufferWidth_Pixels ) ] );
				break;
		}


		
		
		if ( ptr16 < PtrEnd )
		{
		
#ifdef ENABLE_KEYCOLOR
			// check if pixel is transparent //
			// ***TODO*** should probably check the 32-bit version of the pixel ??
			// need to OR in RGB24_TAlpha for now since it could be a 24-bit pixel that was converted to 16-bit??
			//if ( bgr )
			if ( bgr | RGB24_TAlpha )
			{
#endif
		
#ifdef ENABLE_DEPTH_TEST
			// perform depth test //
			
			// get pointer into depth buffer //
			zptr16 = GetZBufPtr16 ( (u16*) zbuf32, x0, y0, FrameBufferWidth_Pixels );
			
			// determine z-buffer format
			// and test against the value in the z-buffer
					
			// PSMZ16
			// PSMZ16S
			//ZBufValue = ( (u64) ( *zptr16 ) ) << 16;
			ZDepthTest_Pass = PerformZDepthTest16 ( zptr16, z0 );
			
			// z-buffer test
			//if ( ( z0 + DepthTest_Offset ) > ZBufValue )
			if ( ZDepthTest_Pass )
			{
#endif

			
			DestPixel = *ptr16;
			
			
#ifdef ENABLE_DEST_ALPHA_TEST			
			// destination alpha test
			if ( !( ( DestPixel ^ DA_Test ) & DA_Enable ) )
			{
				// passed destination alpha test //
#endif

			// texture function ??
			bgr = TextureFunc16 ( bgr, c1, c2, c3, ca );
			
			// fogging ??
			// check if fogging is enabled for primitive
#ifdef ENABLE_FOG
			if ( FogEnable )
			{
				bgr = FogFunc16 ( bgr, fv );
			}
#endif
			

			bgr_temp = bgr;

			
#ifdef ENABLE_ALPHA_POINT
			//if ( Alpha && !( ( bgr ^ AlphaXor16 ) & 0x8000 ) )
			if ( Alpha && !( ( bgr ^ AlphaXor32 ) & AlphaXor32 ) )
			{
				AlphaSelect [ 0 ] = bgr_temp;
				//AlphaSelect [ 0 ] = bgr;
				
				// for 16-bit frame buffer, destination alpha is A shifted left 7
				AlphaSelect [ 1 ] = ( (u32) ( DestPixel ) ) | ( ( (u32) ( DestPixel & 0x8000 ) ) << 16 );
				
				// need to keep the source alpha
				bgr_temp = AlphaABCD_16 ( AlphaSelect [ uA ], AlphaSelect [ uB ], AlphaSelect [ uC ], AlphaSelect [ uD ] );
				
				// re-add source alpha
				// ***TODO*** ***IMPORTANT*** look out for the source alpha, probably should be getting top 8-bits also here?
				//bgr_temp |= ( AlphaSelect [ 0 ] & 0x8000 );
				bgr_temp |= ( bgr & 0xff008000 );
				
			} // end if ( Alpha && !( ( bgr ^ AlphaXor16 ) & 0x8000 ) )
#endif
			
						

#ifdef ENABLE_SRC_ALPHA_TEST

			// same 32-bit code should apply to 16-bit pixels theoretically
			SrcAlphaTest_Pass = TestSrcAlpha32 ( bgr_temp );
			
			// source alpha test
			if ( SrcAlphaTest_Pass )
			{
				// source alpha test passed //
#endif

			// set MSB if specified AFTER alpha test?
			bgr_temp |= PixelOr16;

			// draw pixel //
			
			bgr_temp = ( bgr_temp & FrameBuffer_WriteMask16 ) | ( DestPixel & ~FrameBuffer_WriteMask16 );
			*ptr16 = bgr_temp;

#ifdef ENABLE_DEPTH_TEST
			// only store depth value if ZMSK is zero, which means to update z-buffer
			if ( TEST_X.ZTE && !ZBUF_X.ZMSK )
			{
				// here, z-buffer must be 16-bit
				// *zptr16 = z0 >> 16;
				WriteZBufValue32 ( (u32*) zptr16, z0 );
				
			} // end if ( !ZBUF_X.ZMSK )
#endif

			
#ifdef ENABLE_SRC_ALPHA_TEST
			}
			else
			{
				// source alpha test failed //
				
				PerformAlphaFail16 ( zptr16, ptr16, bgr_temp, DestPixel, z0 );
				
			} // end if ( ( bgr_temp > GreaterOrEqualTo_And && bgr_temp < LessThan_And ) || ( bgr_temp < LessThan_Or ) )
#endif
			
#ifdef ENABLE_DEST_ALPHA_TEST			
			} // end if ( !( ( DestPixel ^ DA_Test ) & DA_Enable ) )
#endif

#ifdef ENABLE_DEPTH_TEST
			} // end if ( ( z0 + DepthTest_Offset ) > ZBufValue )
#endif

#ifdef ENABLE_KEYCOLOR
			} // if ( bgr )
#endif
			
		} // end if ( ptr16 < PtrEnd )

	}

}


		static void sRun () { _GPU->Run (); }
		static void Set_EventCallback ( funcVoid1 UpdateEvent_CB ) { _GPU->NextEvent_Idx = UpdateEvent_CB ( sRun ); };


		static const u32 c_InterruptCpuNotifyBit = 0;
		static const u32 c_InterruptBit = 0;
		static const u32 c_InterruptBit_Vsync_Start = 2;
		static const u32 c_InterruptBit_Vsync_End = 3;

		static u32* _Intc_Stat;
		static u32* _Intc_Mask;
		static u32* _R5900_Status_12;
		static u32* _R5900_Cause_13;
		static u64* _ProcStatus;
		
		inline void ConnectInterrupt ( u32* _IStat, u32* _IMask, u32* _R5900_Status, u32* _R5900_Cause, u64* _ProcStat )
		{
			_Intc_Stat = _IStat;
			_Intc_Mask = _IMask;
			_R5900_Cause_13 = _R5900_Cause;
			_R5900_Status_12 = _R5900_Status;
			_ProcStatus = _ProcStat;
		}
		
		
		inline void SetInterrupt_Vsync_Start ()
		{
			*_Intc_Stat |= ( 1 << c_InterruptBit_Vsync_Start );
			if ( *_Intc_Stat & *_Intc_Mask ) *_R5900_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0xff00 ) && ( *_R5900_Status_12 & 1 ) )
			{
				*_ProcStatus |= ( 1 << c_InterruptCpuNotifyBit );
			}
			else
			{
				*_ProcStatus &= ~( 1 << c_InterruptCpuNotifyBit );
			}
			
			// also set for IOP ??
			Playstation1::GPU::SetInterrupt_Vsync ();
		}
		
		inline void SetInterrupt_Vsync_End ()
		{
			*_Intc_Stat |= ( 1 << c_InterruptBit_Vsync_End );
			if ( *_Intc_Stat & *_Intc_Mask ) *_R5900_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0xff00 ) && ( *_R5900_Status_12 & 1 ) )
			{
				*_ProcStatus |= ( 1 << c_InterruptCpuNotifyBit );
			}
			else
			{
				*_ProcStatus &= ~( 1 << c_InterruptCpuNotifyBit );
			}
			
			// also set for IOP ??
			Playstation1::GPU::SetInterrupt_EVsync ();
		}
		
		static inline void SetInterrupt ()
		{
			*_Intc_Stat |= ( 1 << c_InterruptBit );
			if ( *_Intc_Stat & *_Intc_Mask ) *_R5900_Cause_13 |= ( 1 << 10 );
			
			// ***TODO*** more stuff needs to probably be checked on an R5900 to determine if interrupts are enabled or not
			// it probably doesn't matter, but should probably correct that
			if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0xff00 ) && ( *_R5900_Status_12 & 1 ) )
			{
				*_ProcStatus |= ( 1 << c_InterruptCpuNotifyBit );
			}
			else
			{
				*_ProcStatus &= ~( 1 << c_InterruptCpuNotifyBit );
			}
		}
		
		
		/*
		inline void ClearInterrupt ()
		{
			//*_Intc_Master &= ~( 1 << c_InterruptBit );
			*_Intc_Stat &= ~( 1 << c_InterruptBit );
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 ); else *_ProcStatus &= ~( 1 << 20 );
		}

		inline void ClearInterrupt_Vsync ()
		{
			//*_Intc_Master &= ~( 1 << c_InterruptBit_Vsync );
			*_Intc_Stat &= ~( 1 << c_InterruptBit_Vsync );
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 ); else *_ProcStatus &= ~( 1 << 20 );
		}
		*/

		
		static u64* _NextSystemEvent;

		
		////////////////////////////////
		// Debug Info
		static u32* _DebugPC;
		static u64* _DebugCycleCount;
		static u32* _NextEventIdx;
		
		static bool DebugWindow_Enabled;
		static void DebugWindow_Enable ();
		static void DebugWindow_Disable ();
		static void DebugWindow_Update ();

		
		
#ifdef USE_TEMPLATES_PS2_RECTANGLE

template<const long ZBUF_PSM>
inline u32 GetZBufValue32_t ( u32* zptr32 )
{
	switch ( ZBUF_PSM )
	{
		// PSMZ32
		case 0:
			return ( *zptr32 );
			break;
		
		// PSMZ24
		case 1:
#ifdef ENABLE_ZREAD_SHIFT
			return ( ( *zptr32 ) << 8 );
#else
			return ( ( *zptr32 ) & 0xffffff );
#endif
			break;
			
		// PSMZ16
		case 2:
			// need to alert, as this shouldn't happen
			//cout << "\nhps2x64: GPU: ALERT: PSMZ16 z-buffer format on a 32-bit frame buffer!\n";
			
#ifdef ENABLE_ZREAD_SHIFT
			return ( ( (u32) ( *((u16*)zptr32) ) ) << 16 );
#else
			return ( ( (u32) ( *((u16*)zptr32) ) ) );
#endif
			break;
			
		// PSMZ16S
		case 0xa:
#ifdef ENABLE_ZREAD_SHIFT
			return ( ( (u32) ( *((u16*)zptr32) ) ) << 16 );
#else
			return ( ( (u32) ( *((u16*)zptr32) ) ) );
#endif
			break;
			
		default:
#ifdef VERBOSE_HEADER
			cout << "\nhps2x64: GPU: ALERT: Invalid z-buffer pixel format: " << hex << ZBUF_X.PSM << "\n";
#endif
			break;
	}
}


template<const long ZBUF_PSM>
inline void WriteZBufValue32_t ( u32* zptr32, u32 ZValue32 )
{
	// first need to make sure we are not writing outside GPU RAM
	if ( zptr32 < PtrEnd )
	{
		// determine z-buffer format
		// and then store value back to z-buffer
		switch ( ZBUF_PSM )
		{
			// PSMZ32
			case 0:
				*zptr32 = ZValue32;
				break;
			
			// PSMZ24
			case 1:
#ifdef ENABLE_ZSTORE_SHIFT
				// note: probably don't want to destroy the upper 8-bits here
				*zptr32 = ( *zptr32 & 0xff000000 ) | ( ( ZValue32 >> 8 ) & 0xffffff );
#else
				// note: probably don't want to destroy the upper 8-bits here
				*zptr32 = ( *zptr32 & 0xff000000 ) | ( ( ZValue32 ) & 0xffffff );
#endif
				break;
				
			// PSMZ16
			case 2:
				// need to alert, as this shouldn't happen
				//cout << "\nhps2x64: GPU: ALERT: PSMZ16 z-buffer format on a 32-bit frame buffer!\n";
#ifdef ENABLE_ZSTORE_SHIFT
				*((u16*)zptr32) = ZValue32 >> 16;
#else
				*((u16*)zptr32) = (u16) ZValue32;
#endif
				break;
				
			// PSMZ16S
			case 0xa:
#ifdef ENABLE_ZSTORE_SHIFT
				*((u16*)zptr32) = ZValue32 >> 16;
#else
				*((u16*)zptr32) = (u16) ZValue32;
#endif
				break;
				
			// OTHER
			default:
#ifdef VERBOSE_HEADER
				cout << "\nhps2x64: GPU: ALERT: invalid z-buffer format on a 32-bit frame buffer! ZBUF PSM=" << hex << ZBUF_PSM << "\n";
#endif
				break;
				
		} // end switch ( ZBUF_X.PSM )
	} // end if ( zptr32 < PtrEnd )
}


template<const long TEST_AFAIL,const long ZBUF_ZMSK,const long TEST_ZTE,const long ZBUF_PSM>
inline void PerformAlphaFail16_t ( u16* zptr16, u16* ptr16, u32 bgr, u32 DstPixel, s64 z0 )
{
	// source alpha test failed //
	
	switch ( TEST_AFAIL )
	{
		case 0:
			break;
			
		// update frame buffer ONLY
		case 1:
			// set MSB if specified before drawing
			bgr |= PixelOr16;
			
			bgr = ( bgr & FrameBuffer_WriteMask16 ) | ( DstPixel & ~FrameBuffer_WriteMask16 );
			*ptr16 = bgr;
			break;
			
		// *todo* update z-buffer ONLY
		case 2:
#ifdef ENABLE_DEPTH_TEST
			// only store 16-bit depth value if ZMSK is zero, which means to update z-buffer
			if ( TEST_ZTE && !ZBUF_ZMSK )
			{
				// here, z-buffer must be 16-bit
				// *zptr16 = z0 >> 16;
				WriteZBufValue32_t<ZBUF_PSM> ( (u32*) zptr16, z0 );
				
			} // end if ( !ZBUF_X.ZMSK )
#endif

			break;
			
		// update RGB ONLY (only available for 32-bit pixels)
		// probably available if the source pixel is 32-bits??
		case 3:
			bgr = ( bgr & FrameBuffer_WriteMask16 ) | ( DstPixel & ~FrameBuffer_WriteMask16 );
			*ptr16 = ( DstPixel & 0x8000 ) | ( bgr & ~0x8000 );
			
#ifdef VERBOSE_RGBONLY_H
			// probably available if source pixel is 32-bits??
			cout << "\nhps2x64: ERROR?: GPU: RGB ONLY for 16-bit frame buffer.\n";
#endif
			break;
	}
}


template<const long TEST_AFAIL,const long ZBUF_ZMSK,const long TEST_ZTE,const long ZBUF_PSM>
inline void PerformAlphaFail32_t ( u32* zptr32, u32* ptr32, u32 bgr, u32 DstPixel, s64 z0 )
{
	// source alpha test failed //
	
	switch ( TEST_AFAIL )
	{
		case 0:
			break;
		
		// update frame buffer ONLY
		case 1:
			// set MSB of pixel if specified before drawing
			bgr |= PixelOr32;
			
			bgr = ( bgr & FrameBuffer_WriteMask32 ) | ( DstPixel & ~FrameBuffer_WriteMask32 );
			*ptr32 = bgr;
			break;
			
		// *todo* update z-buffer ONLY
		case 2:
#ifdef ENABLE_DEPTH_TEST
			// store 32-bit depth value //
			
			// only store depth value if ZMSK is zero, which means to update z-buffer
			if ( TEST_ZTE && !ZBUF_ZMSK )
			{
			
				WriteZBufValue32_t<ZBUF_PSM> ( zptr32, z0 );
			
			} // end if ( !ZBUF_X.ZMSK )
#endif
			
			break;
			
		// update RGB ONLY (only available for 32-bit pixels)
		case 3:
			bgr = ( bgr & FrameBuffer_WriteMask32 ) | ( DstPixel & ~FrameBuffer_WriteMask32 );
			*ptr32 = ( DstPixel & 0xff000000 ) | ( bgr & ~0xff000000 );
			break;
	}
}


template<const long COLCLAMP>
inline u32 GPU::AlphaABCD_16_t ( u32 A, u32 B, u32 C, u32 D )
{
	u32 uResult;
	s32 cA, cB, cC, cD, sC1, sC2, sC3;
	
	// get alpha
	cC = ( C >> 24 ) & 0xff;
	
	// get component 1
	cA = ( A >> 0 ) & 0x1f;
	cB = ( B >> 0 ) & 0x1f;
	cD = ( D >> 0 ) & 0x1f;
	
	// the blending does not clamp anything actually, and the result here actually does not get clamped until after dithering
	//sC1 = SignedClamp<s32,5>( ( ( cA - cB ) * cC ) >> 7 );
	sC1 = ( ( ( cA - cB ) * cC ) >> 7 );
	
	// add with D color
	sC1 += cD;
	
	// if COLCLAMP then clamp, otherwise just mask to 8-bits
	if ( COLCLAMP )
	{
		sC1 = SignedClamp<s32,5> ( sC1 );
	}
	else
	{
		sC1 &= 0x1f;
	}
	
	// get component 2
	cA = ( A >> 5 ) & 0x1f;
	cB = ( B >> 5 ) & 0x1f;
	cD = ( D >> 5 ) & 0x1f;
	
	//sC2 = SignedClamp<s32,5>( ( ( cA - cB ) * cC ) >> 7 );
	sC2 = ( ( ( cA - cB ) * cC ) >> 7 );
	
	// add with D color
	sC2 += cD;
	
	// if COLCLAMP then clamp, otherwise just mask to 8-bits
	if ( COLCLAMP )
	{
		sC2 = SignedClamp<s32,5> ( sC2 );
	}
	else
	{
		sC2 &= 0x1f;
	}
	
	
	// get component 3
	cA = ( A >> 10 ) & 0x1f;
	cB = ( B >> 10 ) & 0x1f;
	cD = ( D >> 10 ) & 0x1f;
	
	//sC3 = SignedClamp<s32,5>( ( ( cA - cB ) * cC ) >> 7 );
	sC3 = ( ( ( cA - cB ) * cC ) >> 7 );
	
	// add with D color
	sC3 += cD;
	
	// if COLCLAMP then clamp, otherwise just mask to 8-bits
	if ( COLCLAMP )
	{
		sC3 = SignedClamp<s32,5> ( sC3 );
	}
	else
	{
		sC3 &= 0x1f;
	}
	
	
	return sC1 | ( sC2 << 5 ) | ( sC3 << 10 );
}


// does ( A - B ) * C + D
template<const long COLCLAMP>
inline u32 GPU::AlphaABCD_32_t ( u32 A, u32 B, u32 C, u32 D )
{
	u32 uResult;
	s32 cA, cB, cC, cD, sC1, sC2, sC3;
	
	// get alpha value
	cC = ( C >> 24 ) & 0xff;
	
	// get component 1
	cA = ( A >> 0 ) & 0xff;
	cB = ( B >> 0 ) & 0xff;
	cD = ( D >> 0 ) & 0xff;
	
	// the blending does not clamp anything actually, and the result here actually does not get clamped until after dithering
	//sC1 = SignedClamp<s32,8>( ( ( cA - cB ) * cC ) >> 7 );
	sC1 = ( ( ( cA - cB ) * cC ) >> 7 );
	
	// add with D color
	sC1 += cD;
	
	// if COLCLAMP then clamp, otherwise just mask to 8-bits
	//if ( GPURegsGp.COLCLAMP & 1 )
	if ( COLCLAMP )
	{
		sC1 = SignedClamp<s32,8> ( sC1 );
	}
	else
	{
		sC1 &= 0xff;
	}
	
	
	// get component 2
	cA = ( A >> 8 ) & 0xff;
	cB = ( B >> 8 ) & 0xff;
	cD = ( D >> 8 ) & 0xff;
	
	//sC2 = SignedClamp<s32,8>( ( ( cA - cB ) * cC ) >> 7 );
	sC2 = ( ( ( cA - cB ) * cC ) >> 7 );
	
	// add with D color
	sC2 += cD;
	
	// if COLCLAMP then clamp, otherwise just mask to 8-bits
	//if ( GPURegsGp.COLCLAMP & 1 )
	if ( COLCLAMP )
	{
		sC2 = SignedClamp<s32,8> ( sC2 );
	}
	else
	{
		sC2 &= 0xff;
	}
	
	
	// get component 3
	cA = ( A >> 16 ) & 0xff;
	cB = ( B >> 16 ) & 0xff;
	cD = ( D >> 16 ) & 0xff;
	
	//sC3 = SignedClamp<s32,8>( ( ( cA - cB ) * cC ) >> 7 );
	sC3 = ( ( ( cA - cB ) * cC ) >> 7 );

	// add with D color
	sC3 += cD;
	
	// if COLCLAMP then clamp, otherwise just mask to 8-bits
	//if ( GPURegsGp.COLCLAMP & 1 )
	if ( COLCLAMP )
	{
		sC3 = SignedClamp<s32,8> ( sC3 );
	}
	else
	{
		sC3 &= 0xff;
	}
	
	
	return sC1 | ( sC2 << 8 ) | ( sC3 << 16 );
}


template<const long TEST_ATST>
inline u32 TestSrcAlpha32_t ( u32 bgr )
{
	// needs local variable: bgr_temp
	// returns: SrcAlphaTest_Pass
/*
	switch ( TEST_X.ATE )
	{
		case 0:
			return 1;
			break;
			
		case 1:
		
			//SrcAlphaTest_Pass = 0;
*/
			
			//switch ( TEST_X.ATST )
			switch ( TEST_ATST )
			{
				// NEVER
				case 0:
					return 0;
					break;
					
				// ALWAYS
				case 1:
					return 1;
					break;
					
				// LESS
				case 2:
					// bottom 24 bits of SrcAlpha_ARef need to be cleared out first
					if ( bgr < SrcAlpha_ARef ) return 1;
					break;
					
				// LESS OR EQUAL
				case 3:
					// bottom 24 bits of SrcAlpha_ARef need to be set first
					if ( bgr <= SrcAlpha_ARef ) return 1;
					break;
					
				// EQUAL
				case 4:
					// SrcAlpha_ARef need to be shifted right 24 first
					if ( ( bgr >> 24 ) == SrcAlpha_ARef ) return 1;
					break;
					
				// GREATER OR EQUAL
				case 5:
					// bottom 24 bits of SrcAlpha_ARef need to be cleared out first
					if ( bgr >= SrcAlpha_ARef ) return 1;
					break;
					
				// GREATER
				case 6:
					// bottom 24 bits of SrcAlpha_ARef need to be set first
					if ( bgr > SrcAlpha_ARef ) return 1;
					break;
					
				// NOT EQUAL
				case 7:
					// SrcAlpha_ARef need to be shifted right 24 first
					if ( ( bgr >> 24 ) != SrcAlpha_ARef ) return 1;
					break;
			}
			
/*
			break;
	}
*/
	
	// otherwise, return zero
	return 0;
}


template<const long TEST_ZTST,const long ZBUF_PSM>
inline u32 PerformZDepthTest16_t ( u16* zptr16, u32 ZValue32 )
{
	/*
	if ( !TEST_X.ZTE )
	{
		// depth test is OFF //
		
		// this is supposedly not allowed??
		return 1;
	}
	else
	{
	*/
		// convert from 32-bit to 16-bit z-value //
		
		// the z-value is always specified as an unsigned 32-bit value
		//ZValue32 >>= 16;
		
		// perform the z-test //
		
		switch ( TEST_ZTST )
		{
			// NEVER pass
			case 0:
				return 0;
				break;
				
			// ALWAYS pass
			case 1:
				return 1;
				break;
				
			// GREATER OR EQUAL pass
			case 2:
				//if ( ZValue32 >= ( (u32) ( *zptr16 ) ) ) return 1;
				if ( ZValue32 >= GetZBufValue32_t<ZBUF_PSM> ( (u32*) zptr16 ) ) return 1;
				return 0;
				break;
				
			// GREATER
			case 3:
				//if ( ZValue32 > ( (u32) ( *zptr16 ) ) ) return 1;
				if ( ZValue32 > GetZBufValue32_t<ZBUF_PSM> ( (u32*) zptr16 ) ) return 1;
				return 0;
				break;
		} // end switch ( TEST_X.ZTST )
	
	/*
	} // if ( !TEST_X.ZTE )
	*/
		
	return 0;
}


template<const long TEST_ZTST,const long ZBUF_PSM>
inline u32 PerformZDepthTest32_t ( u32* zptr32, u32 ZValue32 )
{
/*
	if ( !TEST_X.ZTE )
	{
		// depth test is OFF //
		
		// this is supposedly not allowed??
		return 1;
	}
	else
	{
//#define DEBUG_TEST22
#ifdef DEBUG_TEST22
	debug << "\r\n" << hex << ZValue32 << " vs " << GetZBufValue32 ( zptr32 );
#endif
*/

		// initialize offset for z-buffer test //
		//switch ( TEST_X.ZTST )
		switch ( TEST_ZTST )
		{
			// NEVER pass
			case 0:
				return 0;
				break;
				
			// ALWAYS pass
			case 1:
				return 1;
				break;
				
			// GREATER OR EQUAL pass
			case 2:
				if ( ZValue32 >= GetZBufValue32_t<ZBUF_PSM> ( zptr32 ) ) return 1;
				return 0;
				break;
				
			// GREATER
			case 3:
				if ( ZValue32 > GetZBufValue32_t<ZBUF_PSM> ( zptr32 ) ) return 1;
				return 0;
				break;
		} // switch ( TEST_X.ZTST )
		
	//} // if ( !TEST_X.ZTE )
	
	return 0;
}


template<const long ZBUF_PSM>
inline u16* GetZBufPtr16_t ( u16* zbuf16, u32 x, u32 y, u32 FrameBufferWidthInPixels )
{
	switch ( ZBUF_PSM )
	{
		// PSMZ32
		case 0:
		
		// PSMZ24
		case 1:
			return (u16*) ( & ( ( (u32*) zbuf16 ) [ CvtAddrZBuf32 ( x, y, FrameBufferWidthInPixels ) ] ) );
			break;
			
		// PSMZ16
		case 2:
#ifdef VERBOSE_ZBUFFER_H
			// need to alert, as this shouldn't happen
			if ( FrameBuffer_PixelFormat == 0xa ) cout << "\nhps2x64: GPU: ALERT: PSMZ16 z-buffer format on a PSMCT16S frame buffer!\n";
#endif
			
			// not valid for a 32-bit frame buffer
			return & ( zbuf16 [ CvtAddrZBuf16 ( x, y, FrameBufferWidthInPixels ) ] );
			break;
			
		// PSMZ16S
		case 0xa:
#ifdef VERBOSE_ZBUFFER_H
			// need to alert, as this shouldn't happen
			if ( FrameBuffer_PixelFormat == 0x2 ) cout << "\nhps2x64: GPU: ALERT: PSMZ16S z-buffer format on a PSMCT16 frame buffer!\n";
#endif
			
			return & ( zbuf16 [ CvtAddrZBuf16S ( x, y, FrameBufferWidthInPixels ) ] );
			break;
			
		// OTHER
		default:
#ifdef VERBOSE_ZBUFFER_H
			cout << "\nhps2x64: GPU: ALERT: invalid z-buffer format on a 16-bit frame buffer! ZBUF PSM=" << hex << ZBUF_PSM << "\n";
#endif
			break;
	}
	
	return (u16*) &zbuf_NULL;
}


template<const long ZBUF_PSM>
inline u32* GetZBufPtr32_t ( u32* zbuf32, u32 x, u32 y, u32 FrameBufferWidthInPixels )
{
	//switch ( ZBUF_X.PSM )
	switch ( ZBUF_PSM )
	{
		// PSMZ32
		case 0:
		
		// PSMZ24
		case 1:
			return (u32*) ( & ( zbuf32 [ CvtAddrZBuf32 ( x, y, FrameBufferWidthInPixels ) ] ) );
			break;
			
		// PSMZ16
		case 2:
			// not valid for a 32-bit frame buffer
			//return (u32*) ( & ( ((u16*) zbuf32) [ CvtAddrZBuf16 ( x, y, FrameBufferWidthInPixels ) ] ) );
			
#ifdef VERBOSE_ZBUFFER_H
			// need to alert, as this shouldn't happen
			cout << "\nhps2x64: GPU: ALERT: PSMZ16 z-buffer format on a 32-bit frame buffer!\n";
#endif
			break;
			
		// PSMZ16S
		case 0xa:
			return (u32*) ( & ( ((u16*) zbuf32) [ CvtAddrZBuf16S ( x, y, FrameBufferWidthInPixels ) ] ) );
			break;
			
		// OTHER
		default:
#ifdef VERBOSE_ZBUFFER_H
			cout << "\nhps2x64: GPU: ALERT: invalid z-buffer format on a 32-bit frame buffer! ZBUF PSM=" << hex << ZBUF_X.PSM << "\n";
#endif
			break;
	}
	
	return (u32*) &zbuf_NULL;
}

		

//template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_ZTE,const long ZBUF_ZMSK,const long TEST_ZTST,const long ZBUF_PSM,const long FRAME_PSM,const long ABE,const long FGE,const long COLCLAMP>
//inline void PlotPixel_Mono_t ( s32 x0, s32 y0, s64 z0, u32 bgr )
#define PlotPixel_Mono_t(TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,FRAME_PSM,ABE,FGE,COLCLAMP)\
{\
	u32 DestPixel;\
	u32 bgr_temp;\
	u32 SrcAlphaTest_Pass;\
	u32 ZDepthTest_Pass;\
\
	union\
	{\
		u32 *ptr32;\
		u16 *ptr16;\
	};\
\
	union\
	{\
		u32 *zptr32;\
		u16 *zptr16;\
	};\
\
\
	if ( ! ( FRAME_PSM & 2 ) )\
	{\
		/* 32-bit frame buffer */\
\
		/* get pointer into frame buffer */\
\
		switch ( FRAME_PSM )\
		{\
			/* PSMCT32 */\
			case 0:\
\
			/* PSMCT24 */\
			case 1:\
				ptr32 = & ( buf32 [ CvtAddrPix32 ( x0, y0, FrameBufferWidth_Pixels ) ] );\
				break;\
\
			/* PSMZ32 */\
			case 0x30:\
\
			/* PSMZ24 */\
			case 0x31:\
				ptr32 = & ( buf32 [ CvtAddrZBuf32 ( x0, y0, FrameBufferWidth_Pixels ) ] );\
				break;\
		}\
\
		if ( ptr32 < PtrEnd )\
		{\
			/* perform depth test */\
			\
			/* get pointer into depth buffer */\
			zptr32 = GetZBufPtr32_t<ZBUF_PSM> ( zbuf32, x0, y0, FrameBufferWidth_Pixels );\
			\
			/* determine z-buffer format */\
			/* and test against the value in the z-buffer */\
			ZDepthTest_Pass = PerformZDepthTest32_t<TEST_ZTST,ZBUF_PSM> ( zptr32, z0 );\
			\
			/* z-buffer test */\
			if ( ZDepthTest_Pass )\
			{\
			DestPixel = *ptr32;\
\
			/* destination alpha test */\
			if ( !( ( DestPixel ^ DA_Test ) & DA_Enable ) )\
			{\
				/* passed destination alpha test */\
\
			bgr_temp = bgr;\
\
			if ( ABE )\
			{\
				\
			if ( !( ( bgr ^ AlphaXor32 ) & AlphaXor32 ) )\
			{\
				AlphaSelect [ 1 ] = ( DestPixel & DestMask24 ) | DestAlpha24;\
				\
				/* need to keep the source alpha */\
				bgr_temp = AlphaABCD_32_t<COLCLAMP> ( AlphaSelect [ uA ], AlphaSelect [ uB ], AlphaSelect [ uC ], AlphaSelect [ uD ] );\
				\
				/* re-add source alpha */\
				bgr_temp |= ( bgr & 0xff000000 );\
			}\
			\
			} /* end if ( ABE ) */\
			\
			SrcAlphaTest_Pass = TestSrcAlpha32_t<TEST_ATST> ( bgr_temp );\
\
			/* source alpha test */\
			if ( SrcAlphaTest_Pass )\
			{\
				/* source alpha test passed */\
\
				/* store pixel and z-value */\
\
				/* this should go AFTER the source alpha test? */\
				bgr_temp |= PixelOr32;\
				\
				/* draw pixel if we can draw to mask pixels or mask bit not set */\
				\
				/* draw pixel */\
				\
				bgr_temp = ( bgr_temp & FrameBuffer_WriteMask32 ) | ( DestPixel & ~FrameBuffer_WriteMask32 );\
				*ptr32 = bgr_temp;\
\
				/* store depth value */\
				/* local variable input: zbuf ptr, zvalue */\
				\
				/* only store depth value if ZMSK is zero, which means to update z-buffer */\
				if ( TEST_ZTE && !ZBUF_ZMSK )\
				{\
				\
					WriteZBufValue32_t<ZBUF_PSM> ( zptr32, z0 );\
					\
				} /* end if ( !ZBUF_X.ZMSK ) */\
\
			}\
			else\
			{\
				/* source alpha test failed */\
				\
				PerformAlphaFail32_t<TEST_AFAIL,ZBUF_ZMSK,TEST_ZTE,ZBUF_PSM> ( zptr32, ptr32, bgr_temp, DestPixel, z0 );\
				\
			} /* end if ( ( bgr_temp > GreaterOrEqualTo_And && bgr_temp < LessThan_And ) || ( bgr_temp < LessThan_Or ) ) */\
\
			\
			} /* end if ( !( ( DestPixel ^ DA_Test ) & DA_Enable ) ) */\
\
			} /* end if ( ( z0 + DepthTest_Offset ) > ZBufValue ) */\
\
		} /* end if ( ptr32 < PtrEnd ) */\
			\
	}\
	else\
	{\
		/* 16-bit frame buffer */\
\
		/* get pointer into frame buffer */\
		\
		switch ( FRAME_PSM )\
		{\
			/* PSMCT32 */\
			case 0:\
			\
			/* PSMCT24 */\
			case 1:\
				break;\
				\
			/* PSMCT16 */\
			case 2:\
				ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrPix16 ( x0, y0, FrameBufferWidth_Pixels ) ] );\
				break;\
				\
			/* PSMCT16S */\
			case 0xa:\
				ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrPix16S ( x0, y0, FrameBufferWidth_Pixels ) ] );\
				break;\
				\
			/* PSMZ16 */\
			case 0x32:\
				ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrZBuf16 ( x0, y0, FrameBufferWidth_Pixels ) ] );\
				break;\
				\
			/* PSMZ16S */\
			case 0x3a:\
				ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrZBuf16S ( x0, y0, FrameBufferWidth_Pixels ) ] );\
				break;\
		}\
\
		if ( ptr16 < PtrEnd )\
		{\
			/* perform depth test */\
			\
			/* get pointer into depth buffer */\
			zptr16 = GetZBufPtr16 ( (u16*) zbuf32, x0, y0, FrameBufferWidth_Pixels );\
			\
			/* determine z-buffer format */\
			/* and test against the value in the z-buffer */\
					\
			/* PSMZ16 */\
			/* PSMZ16S */\
			ZDepthTest_Pass = PerformZDepthTest16_t<TEST_ZTST,ZBUF_PSM> ( zptr16, z0 );\
			\
			/* z-buffer test */\
			if ( ZDepthTest_Pass )\
			{\
			\
			DestPixel = *ptr16;\
			\
			/* destination alpha test */\
			if ( !( ( DestPixel ^ DA_Test ) & DA_Enable ) )\
			{\
				/* passed destination alpha test */\
\
			bgr_temp = bgr;\
\
			if ( ABE )\
			{\
				\
			if ( !( ( bgr ^ AlphaXor32 ) & AlphaXor32 ) )\
			{\
				/* for 16-bit frame buffer, destination alpha is A shifted left 7 */\
				AlphaSelect [ 1 ] = ( (u32) ( DestPixel ) ) | ( ( (u32) ( DestPixel & 0x8000 ) ) << 16 );\
				\
				/* need to keep the source alpha */\
				bgr_temp = AlphaABCD_16_t<COLCLAMP> ( AlphaSelect [ uA ], AlphaSelect [ uB ], AlphaSelect [ uC ], AlphaSelect [ uD ] );\
				\
				/* re-add source alpha */\
				/* ***TODO*** ***IMPORTANT*** look out for the source alpha, probably should be getting top 8-bits also here? */\
				bgr_temp |= ( bgr & 0xff008000 );\
				\
			} /* end if ( Alpha && !( ( bgr ^ AlphaXor32 ) & AlphaXor32 ) ) */\
			\
			}\
						\
			/* same 32-bit code should apply to 16-bit pixels theoretically */\
			SrcAlphaTest_Pass = TestSrcAlpha32_t<TEST_ATST> ( bgr_temp );\
			\
			/* source alpha test */\
			if ( SrcAlphaTest_Pass )\
			{\
				/* source alpha test passed */\
\
			/* set MSB if specified AFTER alpha test? */\
			bgr_temp |= PixelOr16;\
\
			/* draw pixel */\
			\
			bgr_temp = ( bgr_temp & FrameBuffer_WriteMask16 ) | ( DestPixel & ~FrameBuffer_WriteMask16 );\
			*ptr16 = bgr_temp;\
\
			/* only store depth value if ZMSK is zero, which means to update z-buffer */\
			if ( TEST_ZTE && !ZBUF_ZMSK )\
			{\
				/* here, z-buffer must be 16-bit */\
				WriteZBufValue32_t<ZBUF_PSM> ( (u32*) zptr16, z0 );\
				\
			} /* end if ( !ZBUF_X.ZMSK ) */\
\
\
			}\
			else\
			{\
				/* source alpha test failed */\
				\
				PerformAlphaFail16_t<TEST_AFAIL,ZBUF_ZMSK,TEST_ZTE,ZBUF_PSM> ( zptr16, ptr16, bgr_temp, DestPixel, z0 );\
				\
			} /* end if ( ( bgr_temp > GreaterOrEqualTo_And && bgr_temp < LessThan_And ) || ( bgr_temp < LessThan_Or ) ) */\
			\
			} /* end if ( !( ( DestPixel ^ DA_Test ) & DA_Enable ) ) */\
\
			} /* end if ( ( z0 + DepthTest_Offset ) > ZBufValue ) */\
\
		} /* end if ( ptr16 < PtrEnd ) */\
	}\
}


//template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_ZTE,const long ZBUF_ZMSK,const long TEST_ZTST,const long ZBUF_PSM,const long FRAME_PSM,const long ABE,const long FGE,const long COLCLAMP>
//inline void GPU::Render_Rectangle_t ( u32 Coord0, u32 Coord1 )
#define Render_Rectangle_t(TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,FRAME_PSM,ABE,FGE,COLCLAMP)\
{\
/*#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE*/\
	/*debug << "; RenderRectangle";*/\
/*#endif*/\
\
	/* render in 8bits per pixel, then convert down when done */\
\
	static const int c_iVectorSize = 1;\
\
	s32 StartX, EndX, StartY, EndY;\
	u32 PixelsPerLine;\
	u32 NumberOfPixelsDrawn;\
\
\
	u32 DestPixel, bgr, bgr_temp;\
\
	/* 12.4 fixed point */\
	s32 x0, y0, x1, y1;\
\
	/* and the z coords */\
	s64 z0, z1;\
\
	s32 Line, x_across;\
\
	/* interpolate the z ?? */\
	s64 dzdx, dzdy;\
	s64 iZ;\
\
	s64 Temp;\
\
\
	/* set fixed alpha values */\
	AlphaSelect [ 0 ] = rgbaq_Current.Value & 0xffffffffL;\
	AlphaSelect [ 2 ] = ALPHA_X.FIX << 24;\
	AlphaSelect [ 3 ] = 0;\
\
\
	/* get x,y */\
	x0 = xyz [ Coord0 ].X;\
	x1 = xyz [ Coord1 ].X;\
	y0 = xyz [ Coord0 ].Y;\
	y1 = xyz [ Coord1 ].Y;\
\
	/* get z */\
	z0 = (u64) xyz [ Coord0 ].Z;\
	z1 = (u64) xyz [ Coord1 ].Z;\
\
\
	/* z0 should be same as z1 ?? */\
	z0 = z1;\
\
\
\
	/* get fill color */\
	bgr = rgbaq_Current.Value & 0xffffffffL;\
\
\
\
	/* //////////////////////////////////////// */\
	/* get coordinates on screen */\
	/* *note* this is different from PS1, where you would add the offsets.. */\
	x0 -= Coord_OffsetX;\
	y0 -= Coord_OffsetY;\
	x1 -= Coord_OffsetX;\
	y1 -= Coord_OffsetY;\
\
\
\
	/* looks like some sprites have y1 < y0 and/or x1 < x0 */\
	/* didn't expect that so need to alert and figure out some other time */\
	if ( ( y1 < y0 ) || ( x1 < x0 ) )\
	{\
\
		StartX = ( x0 <= x1 ) ? x0 : x1;\
		EndX = ( x0 <= x1 ) ? x1 : x0;\
		StartY = ( y0 <= y1 ) ? y0 : y1;\
		EndY = ( y0 <= y1 ) ? y1 : y0;\
\
		x0 = StartX;\
		x1 = EndX;\
		y0 = StartY;\
		y1 = EndY;\
\
	}\
\
\
\
\
	/* coords are in 12.4 fixed point */\
	StartX = ( x0 + 0xf ) >> 4;\
	EndX = ( x1 - 1 ) >> 4;\
	StartY = ( y0 + 0xf ) >> 4;\
	EndY = ( y1 - 1 ) >> 4;\
\
\
	/* check if sprite is within draw area */\
	if ( EndX < Window_XLeft || StartX > Window_XRight || EndY < Window_YTop || StartY > Window_YBottom ) return;\
/*#if defined INLINE_DEBUG_SPRITE_TEST || defined INLINE_DEBUG_PRIMITIVE_TEST*/\
	else\
	{\
		debug << dec << "\r\nDrawRectangle" << " FinalCoords: x0=" << x0 << " y0=" << y0 << " x1=" << x1 << " y1=" << y1;\
		debug << hex << " bgr=" << bgr;\
		debug << " FrameBufPixFmt=" << PixelFormat_Names [ FrameBuffer_PixelFormat ];\
		debug << hex << "; FrameBufPtr32/64=" << ( FrameBufferStartOffset32 >> 6 );\
		debug << " FrameBufPixFmt=" << PixelFormat_Names [ FrameBuffer_PixelFormat ];\
		debug << " Alpha=" << Alpha;\
		debug << " PABE=" << GPURegsGp.PABE;\
		debug << " FBA=" << FBA_X;\
		debug << hex << " ZBuf=" << (ZBufferStartOffset32 >> 11);\
		debug << PixelFormat_Names [ ZBuffer_PixelFormat ];\
		debug << " ZFlags=" << ZBUF_X.Value;\
		debug << " TEST=" << TEST_X.Value;\
	}\
/*#endif*/\
\
\
	Temp = ( StartY << 4 ) - y0;\
\
	if ( StartY < Window_YTop )\
	{\
		Temp += ( Window_YTop - StartY ) << 4;\
		StartY = Window_YTop;\
	}\
\
\
	if ( EndY > Window_YBottom )\
	{\
		EndY = Window_YBottom;\
	}\
\
	Temp = ( StartX << 4 ) - x0;\
\
	if ( StartX < Window_XLeft )\
	{\
		Temp += ( Window_XLeft - StartX ) << 4;\
		StartX = Window_XLeft;\
	}\
\
\
\
	if ( EndX > Window_XRight )\
	{\
		EndX = Window_XRight;\
	}\
\
\
	/* check that there is a pixel to draw */\
	if ( ( EndX < StartX ) || ( EndY < StartY ) )\
	{\
/*#if defined INLINE_DEBUG_SPRITE_TEST || defined INLINE_DEBUG_PRIMITIVE_TEST*/\
		/*debug << dec << "\r\nERROR: EndY < StartY or EndX < StartX in rectangle!!!";*/\
		/*debug << dec << " FinalCoords: StartX=" << StartX << " StartY=" << StartY << " EndX=" << EndX << " EndY=" << EndY;*/\
/*#endif*/\
\
		return;\
	}\
\
	NumberOfPixelsDrawn = ( EndX - StartX + 1 ) * ( EndY - StartY + 1 );\
\
	/* there are probably multiple pixel pipelines, so  might need to divide by like 8 or 16 or something */\
	if ( BusyUntil_Cycle < *_DebugCycleCount )\
	{\
		BusyUntil_Cycle = *_DebugCycleCount + ( NumberOfPixelsDrawn >> 4 );\
	}\
\
	/* need to convert bgr to 16-bit pixel if it is a 16-bit frame buffer */\
	if ( FrameBuffer_PixelFormat > 1 )\
	{\
		/* 16-bit frame buffer */\
\
		/* note: this actually needs to be done ahead of time, the conversion from 32-bit pixel to 16-bit pixel */\
		/* convert to 16-bit pixel */\
		/* must also convert alpha */\
		bgr = Color24To16 ( bgr ) | ( ( bgr >> 16 ) & 0x8000 ) | ( bgr & 0xff000000 );\
\
		/* set the alpha */\
		AlphaSelect [ 0 ] = bgr;\
	}\
\
\
	/* looks like the endy value is not included */\
	for ( Line = StartY; Line <= EndY; Line++ )\
	{\
		for ( x_across = StartX; x_across <= EndX /* && ptr32 < PtrEnd */; x_across += c_iVectorSize )\
		{\
			/* ***TODO*** ***PROBLEM*** need to interpolate z ?? */\
			/* plot the pixel */\
			PlotPixel_Mono_t(TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,FRAME_PSM,ABE,FGE,COLCLAMP);\
		}\
	}\
}


//template<const long TEST_ATE,const long TEST_ATST,const long TEST_AFAIL,const long TEST_DATE,const long TEST_DATM,const long TEST_ZTE,const long TEST_ZTST,const long ZBUF_ZMSK,const long ZBUF_PSM,const long FRAME_PSM,const long COLCLAMP,const long FGE>
//inline void GPU::Select_RenderRectangle13_t ( u32 Coord0, u32 Coord1 )
//{
//}


//template<const long TEST_ATE,const long TEST_ATST,const long TEST_AFAIL,const long TEST_DATE,const long TEST_DATM,const long TEST_ZTE,const long TEST_ZTST,const long ZBUF_ZMSK,const long ZBUF_PSM,const long FRAME_PSM,const long COLCLAMP>
//inline void GPU::Select_RenderRectangle12_t ( u32 Coord0, u32 Coord1 )
//{
//}

/*		
template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_DATE,const long TEST_ZTE,const long ZBUF_ZMSK,const long TEST_ZTST,const long ZBUF_PSM,const long FRAME_PSM,const long ABE,const long FGE>
inline void GPU::Select_RenderRectangle11_t ( u32 Coord0, u32 Coord1 )
{
	// color clamp or not //
	switch ( GPURegsGp.COLCLAMP & 1 )
	{
		case 0:
			// wrap around calculation //
			//Render_Rectangle_t<TEST_ATST,TEST_AFAIL,TEST_DATE,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,FRAME_PSM,ABE,FGE,0> ( Coord0, Coord1 );
			RenderRectangle_DS ( Coord0, Coord1 );
			break;
			
		case 1:
			// clamp to 0-255 range //
			//Render_Rectangle_t<TEST_ATST,TEST_AFAIL,TEST_DATE,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,FRAME_PSM,ABE,FGE,1> ( Coord0, Coord1 );
			RenderRectangle_DS ( Coord1, Coord0 );
			break;
	}
}


template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_DATE,const long TEST_ZTE,const long ZBUF_ZMSK,const long TEST_ZTST,const long ZBUF_PSM,const long FRAME_PSM,const long ABE>
inline void GPU::Select_RenderRectangle10_t ( u32 Coord0, u32 Coord1 )
{
	// fog enable //
	switch ( FogEnable )
	{
		case 0:
			Select_RenderRectangle11_t<TEST_ATST,TEST_AFAIL,TEST_DATE,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,FRAME_PSM,ABE,0> ( Coord0, Coord1 );
			break;
			
		case 1:
			Select_RenderRectangle11_t<TEST_ATST,TEST_AFAIL,TEST_DATE,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,FRAME_PSM,ABE,1> ( Coord0, Coord1 );
			break;
	}
}
*/


//template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_ZTE,const long ZBUF_ZMSK,const long TEST_ZTST,const long ZBUF_PSM,const long FRAME_PSM>
//inline void GPU::Select_RenderRectangle9_t ( u32 Coord0, u32 Coord1, const int TEST_ATST,const int TEST_AFAIL,const int TEST_ZTE,const int ZBUF_ZMSK,const int TEST_ZTST,const int ZBUF_PSM,const int FRAME_PSM )
#define Select_RenderRectangle9_t(TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,FRAME_PSM)\
{\
	/* alpha-blending enable */\
	switch ( Alpha )\
	{\
		case 0:\
\
			switch ( FogEnable )\
			{\
				case 0:\
					switch ( GPURegsGp.COLCLAMP & 1 )\
					{\
						case 0:\
							Render_Rectangle_t(TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,FRAME_PSM,0,0,0);\
							break;\
\
						case 1:\
							Render_Rectangle_t(TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,FRAME_PSM,0,0,1);\
							break;\
					}\
\
					break;\
\
				case 1:\
					switch ( GPURegsGp.COLCLAMP & 1 )\
					{\
						case 0:\
							Render_Rectangle_t(TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,FRAME_PSM,0,1,0);\
							break;\
\
						case 1:\
							Render_Rectangle_t(TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,FRAME_PSM,0,1,1);\
							break;\
					}\
					break;\
			}\
\
			break;\
\
		case 1:\
\
			switch ( FogEnable )\
			{\
				case 0:\
					switch ( GPURegsGp.COLCLAMP & 1 )\
					{\
						case 0:\
							Render_Rectangle_t(TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,FRAME_PSM,1,0,0);\
							break;\
\
						case 1:\
							Render_Rectangle_t(TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,FRAME_PSM,1,0,1);\
							break;\
					}\
\
					break;\
\
				case 1:\
					switch ( GPURegsGp.COLCLAMP & 1 )\
					{\
						case 0:\
							Render_Rectangle_t(TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,FRAME_PSM,1,1,0);\
							break;\
\
						case 1:\
							Render_Rectangle_t(TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,FRAME_PSM,1,1,1);\
							break;\
					}\
					break;\
			}\
\
			break;\
	}\
}


//template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_ZTE,const long ZBUF_ZMSK,const long TEST_ZTST,const long ZBUF_PSM>
//inline void GPU::Select_RenderRectangle8_t ( u32 Coord0, u32 Coord1, const int TEST_ATST,const int TEST_AFAIL,const int TEST_ZTE,const int ZBUF_ZMSK,const int TEST_ZTST,const int ZBUF_PSM )
#define Select_RenderRectangle8_t(TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM)\
{\
	/* frame buffer pixel format */\
	switch ( FrameBuffer_PixelFormat )\
	{\
		case 0:\
			/* PSMCT32 */\
			Select_RenderRectangle9_t ( TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,0 );\
			break;\
\
		case 1:\
			/* PSMCT24 */\
			Select_RenderRectangle9_t ( TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,1 );\
			break;\
\
		/*case 2:*/\
			/* PSMCT16 */\
\
			/* this one can ONLY go with PSMZ16 */\
			/*Select_RenderRectangle11_t<TEST_ATE,TEST_ATST,TEST_AFAIL,TEST_DATE,TEST_DATM,TEST_ZTE,TEST_ZTST,ZBUF_ZMSK,ZBUF_PSM,2> ( Coord0, Coord1 );*/\
			/*break;*/\
\
		case 0xa:\
			/* PSMCT16S */\
			Select_RenderRectangle9_t ( TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,0xa );\
			break;\
\
		/*case 0x30:*/\
			/* PSMZ32 */\
			/*Select_RenderRectangle11_t<TEST_ATE,TEST_ATST,TEST_AFAIL,TEST_DATE,TEST_DATM,TEST_ZTE,TEST_ZTST,ZBUF_ZMSK,ZBUF_PSM,0x30> ( Coord0, Coord1 );*/\
			/*break;*/\
\
		case 0x31:\
			/* PSMZ24 */\
			Select_RenderRectangle9_t ( TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,0x31 );\
			break;\
\
		/*case 0x32:*/\
			/* PSMZ16 */\
\
			/* this one can ONLY go with PSMZ16 */\
			/*Select_RenderRectangle11_t<TEST_ATE,TEST_ATST,TEST_AFAIL,TEST_DATE,TEST_DATM,TEST_ZTE,TEST_ZTST,ZBUF_ZMSK,ZBUF_PSM,0x32> ( Coord0, Coord1 );*/\
			/*break;*/\
\
		case 0x3a:\
			/* PSMZ16S */\
			Select_RenderRectangle9_t ( TEST_ATST,TEST_AFAIL,TEST_ZTE,ZBUF_ZMSK,TEST_ZTST,ZBUF_PSM,0x3a );\
			break;\
\
		default:\
			break;\
	}\
}


//template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_ZMSK,const long TEST_ZTST>
//inline void GPU::Select_RenderRectangle7_t ( u32 Coord0, u32 Coord1, const int TEST_ATST,const int TEST_AFAIL,const int TEST_ZMSK,const int TEST_ZTST )
#define Select_RenderRectangle7_t(TEST_ATST,TEST_AFAIL,TEST_ZMSK,TEST_ZTST)\
{\
	/* z buffer pixel format */\
	switch ( ZBUF_X.PSM )\
	{\
		case 0:\
			/* PSMZ32 */\
			Select_RenderRectangle8_t( TEST_ATST,TEST_AFAIL,1,TEST_ZMSK,TEST_ZTST,0 );\
			break;\
\
		case 1:\
			/* PSMZ24 */\
			Select_RenderRectangle8_t ( TEST_ATST,TEST_AFAIL,1,TEST_ZMSK,TEST_ZTST,1 );\
			break;\
\
		/* this can ONLY go with PSMCT16/PSMZ16 */\
		case 2:\
			/* PSMZ16 */\
\
			/* check if frame buffer is PSMCT16 format */\
			if ( FrameBuffer_PixelFormat == 0x2 )\
			{\
				Select_RenderRectangle9_t ( TEST_ATST,TEST_AFAIL,1,TEST_ZMSK,TEST_ZTST,2,2 );\
			}\
			else\
			{\
				cout << "\nhps2x64: ALERT: GPU: Invalid Frame Buffer PSM for 16-bit z-buffer. ZBUF pixel format: " << hex << ZBUF_X.PSM;\
			}\
			/*Select_RenderRectangle10_t<TEST_ATE,TEST_ATST,TEST_AFAIL,TEST_DATE,TEST_DATM,TEST_ZTE,TEST_ZTST,ZBUF_ZMSK,2> ( Coord0, Coord1 );*/\
			break;\
\
		case 0xa:\
			/* PSMZ16S */\
			Select_RenderRectangle8_t ( TEST_ATST,TEST_AFAIL,1,TEST_ZMSK,TEST_ZTST,0xa );\
			break;\
\
		default:\
			cout << "\nhps2x64: GPU: Unknown ZBuffer pixel format: " << hex << ZBUF_X.PSM;\
			break;\
	}\
}


//template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_ZMSK>
//inline void GPU::Select_RenderRectangle6_t ( u32 Coord0, u32 Coord1, const int TEST_ATST, const int TEST_AFAIL, const int TEST_ZMSK )
#define Select_RenderRectangle6_t(TEST_ATST,TEST_AFAIL,TEST_ZMSK)\
{\
	/* z depth test type */\
	switch ( TEST_X.ZTST )\
	{\
		case 0:\
			/* NEVER */\
			Select_RenderRectangle7_t ( TEST_ATST, TEST_AFAIL, TEST_ZMSK, 0 );\
			break;\
\
		case 1:\
			/* ALWAYS */\
			Select_RenderRectangle7_t ( TEST_ATST, TEST_AFAIL, TEST_ZMSK, 1 );\
			break;\
\
		case 2:\
			/* GREATER OR EQUAL */\
			Select_RenderRectangle7_t ( TEST_ATST, TEST_AFAIL, TEST_ZMSK, 2 );\
			break;\
\
		case 3:\
			/* GREATER */\
			Select_RenderRectangle7_t ( TEST_ATST, TEST_AFAIL, TEST_ZMSK, 3 );\
			break;\
	}\
}


//template<const long TEST_ATST,const long TEST_AFAIL>
//inline void GPU::Select_RenderRectangle5_t ( u32 Coord0, u32 Coord1, const int TEST_ATST, const int TEST_AFAIL )
#define Select_RenderRectangle5_t(TEST_ATST,TEST_AFAIL)\
{\
	/* write to zbuf enable */\
	switch ( ZBUF_X.ZMSK )\
	{\
		case 0:\
			/* z buffer is written back to */\
			Select_RenderRectangle6_t ( TEST_ATST, TEST_AFAIL, 0 );\
			break;\
\
		case 1:\
			/* z buffer is NOT written to */\
			Select_RenderRectangle6_t ( TEST_ATST, TEST_AFAIL, 1 );\
			break;\
	}\
}


//template<const long TEST_ATST,const long TEST_AFAIL>
//inline void GPU::Select_RenderRectangle4_t ( u32 Coord0, u32 Coord1, const int TEST_ATST, const int TEST_AFAIL )
#define Select_RenderRectangle4_t(TEST_ATST,TEST_AFAIL)\
{\
	/* z-test enable */\
	switch ( TEST_X.ZTE )\
	{\
		case 0:\
			/* z depth test DISABLED */\
			Select_RenderRectangle8_t ( TEST_ATST, TEST_AFAIL, 0, 1, 0, 0 );\
			break;\
\
		case 1:\
			/* z depth test ENABLED */\
			Select_RenderRectangle5_t ( TEST_ATST, TEST_AFAIL );\
			break;\
	}\
}

/*
template<const long TEST_ATST,const long TEST_AFAIL>
inline void GPU::Select_RenderRectangle3_t ( u32 Coord0, u32 Coord1 )
{
	// destination alpha test enabled //
	switch ( TEST_X.DATE )
	{
		case 0:
			Select_RenderRectangle4_t<TEST_ATST,TEST_AFAIL,0> ( Coord0, Coord1 );
			break;
			
		case 1:
			Select_RenderRectangle4_t<TEST_ATST,TEST_AFAIL,1> ( Coord0, Coord1 );
			break;
	}
}
*/


//template<const long TEST_ATST>
//inline void GPU::Select_RenderRectangle2_t ( u32 Coord0, u32 Coord1, const int TEST_ATST )
#define Select_RenderRectangle2_t(TEST_ATST)\
{\
	/* what to do when alpha test failed */\
	switch ( TEST_X.AFAIL )\
	{\
		case 0:\
			/* KEEP */\
			Select_RenderRectangle4_t ( TEST_ATST, 0 );\
			break;\
\
		case 1:\
			/* FBUF ONLY */\
			Select_RenderRectangle4_t ( TEST_ATST, 1 );\
			break;\
\
		case 2:\
			/* ZBUF ONLY */\
			Select_RenderRectangle4_t ( TEST_ATST, 2 );\
			break;\
\
		case 3:\
			/* RGB ONLY */\
			Select_RenderRectangle4_t ( TEST_ATST, 3 );\
			break;\
	}\
}


// template needed: DA_TEST,DA_ENABLE,FBUF_PIXELFORMAT,TEXA_AEM,ZBUF_PIXELFORMAT,TEST_ATE,TEST_ATST,TEST_ZTE,TEST_ZTST,TEST_AFAIL,ZBUF_ZMSK,COLCLAMP
void GPU::Select_RenderRectangle_t ( u32 Coord0, u32 Coord1 )
{
	// source alpha test //
	// source alpha test type //
	switch ( TEST_X.ATE )
	{
		case 0:
			// NO alpha test //
			Select_RenderRectangle4_t ( 1, 0 );
			break;
			
		case 1:
			// alpha test //
			switch ( TEST_X.ATST )
			{
				case 0:
					// NEVER //
					Select_RenderRectangle2_t ( 0 );
					break;
					
				case 1:
					// ALWAYS //
					Select_RenderRectangle4_t ( 1, 0 );
					break;
					
				case 2:
					// LESS //
					Select_RenderRectangle2_t ( 2 );
					break;
					
				case 3:
					// LESS OR EQUAL //
					Select_RenderRectangle2_t ( 3 );
					break;
					
				case 4:
					// EQUAL //
					Select_RenderRectangle2_t ( 4 );
					break;
					
				case 5:
					// GREATER OR EQUAL //
					Select_RenderRectangle2_t ( 5 );
					break;
					
				case 6:
					// GREATER //
					Select_RenderRectangle2_t ( 6 );
					break;
					
				case 7:
					// NOT EQUAL //
					Select_RenderRectangle2_t ( 7 );
					break;
			}
			
			break;
	}
}

// template needed: PIXELFORMAT,CLUT_PIXELFORMAT,FBUF_PIXELFORMAT,TEXA_AEM,ZBUF_PIXELFORMAT,TEST_ATE,TEST_ATST,TEST_ZTE,TEST_ZTST,TEST_AFAIL,ZBUF_ZMSK,COLCLAMP,TEX_TFX,TEX_TCC
void GPU::Select_RenderSprite_t ( u32 Coord0, u32 Coord1 )
{
	
}


#endif



	};
}


#endif

