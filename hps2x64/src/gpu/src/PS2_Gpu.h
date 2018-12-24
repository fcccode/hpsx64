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


#define EXTRACT_BITS(var,from,to)	( ( (var) >> (from) ) & ( (1 << ( (to)-(from)+1 )) - 1 ) )


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
#define VERBOSE_INVALID_COMBINE
#define VERBOSE_INVALID_CLD

// will comment this out as needed
//#define VERBOSE_RGBONLY


// the new way or the old way?
//#define OLD_PATH1_ARGS



#define USE_CVT_LUT


//#define USE_TEMPLATES_PS2_RECTANGLE
//#define USE_TEMPLATES_PS2_SPRITE


// user, rhymes with... view mode
#define ENABLE_PIXELBUF_INTERLACING	

// enables templates for PS2 GPU
// this is enabled/disabled via makefile
//#define USE_PS2_GPU_TEMPLATES


#ifdef USE_PS2_GPU_TEMPLATES


#define USE_TEMPLATES_PS2_POINT
#define USE_TEMPLATES_PS2_LINE
#define USE_TEMPLATES_PS2_RECTANGLE
#define USE_TEMPLATES_PS2_TRIANGLE
#define USE_TEMPLATES_PS2_COPYLOCAL
#define USE_TEMPLATES_PS2_DRAWSCREEN
#define USE_TEMPLATES_PS2_WRITECLUT
#define USE_TEMPLATES_PS2_TRANSFERIN


//#define OLD_GIF_TRANSFER
//#define USE_OLD_MULTI_THREADING

#endif





// functions for alpha test and ztest, and alpha fail
typedef u32 (*AlphaTest) ( u32, u32 );
typedef u32 (*ZTest) ( u32*, u32 );
typedef void (*AlphaFail) ( u32*, u32*, u32, u32, u32 );
typedef u32 (*TexturePixel) ( u32*, u32, u32, u32, u16*, u64 );
typedef u32 (*TextureFunc) ( u32, u32, u32, u32, u32 );


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
		
		

		
		
		static u32 LUT_CvtAddrPix32 [ 64 * 32 ];
		static u32 LUT_CvtAddrPix16 [ 64 * 64 ];
		static u32 LUT_CvtAddrPix16s [ 64 * 64 ];
		static u32 LUT_CvtAddrPix8 [ 128 * 64 ];
		static u32 LUT_CvtAddrPix4 [ 128 * 128 ];
		
		static u32 LUT_CvtAddrZBuf32 [ 64 * 32 ];
		static u32 LUT_CvtAddrZBuf16 [ 64 * 64 ];
		static u32 LUT_CvtAddrZBuf16s [ 64 * 64 ];
		
		void InitCvtLUTs ();
		
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
			
#ifdef USE_CVT_LUT
			Offset = LUT_CvtAddrPix32 [ ( x & 0x3f ) | ( ( y & 0x1f ) << 6 ) ];
#else
			//Offset = ( x & 0x1 ) | ( ( y & 0x1 ) << 1 ) | ( ( x & 0x6 ) << 1 ) | ( ( y & 0x6 ) << 3 ) | ( ( x & 0x8 ) << 3 ) | ( ( y & 0x8 ) << 4 ) | ( ( x & 0x10 ) << 4 ) | ( ( y & 0x10 ) << 5 ) | ( ( x & 0x20 ) << 5 );
			Offset = ( x & 0x1 ) | ( ( ( y & 0x1 ) | ( x & 0x6 ) ) << 1 ) | ( ( ( y & 0x6 ) | ( x & 0x8 ) ) << 3 ) | ( ( ( y & 0x8 ) | ( x & 0x10 ) ) << 4 ) | ( ( ( y & 0x10 ) | ( x & 0x20 ) ) << 5 );
#endif
			
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
			
#ifdef USE_CVT_LUT
			Offset = LUT_CvtAddrPix16 [ ( x & 0x3f ) | ( ( y & 0x3f ) << 6 ) ];
#else
			// first bit of y goes into second bit of x
			//Offset = ( ( x & 8 ) >> 3 ) | ( ( x & 1 ) << 1 ) | ( ( y & 1 ) << 2 ) | ( ( x & 6 ) << 2 ) | ( ( y & 0xe ) << 4 ) | ( ( x & 0x10 ) << 4 ) | ( ( y & 0x10 ) << 5 )
			//		| ( ( x & 0x20 ) << 5 ) | ( ( y & 0x20 ) << 6 );
			Offset = ( ( x & 8 ) >> 3 ) | ( ( x & 1 ) << 1 ) | ( ( ( y & 1 ) | ( x & 6 ) ) << 2 ) | ( ( ( y & 0xe ) | ( x & 0x10 ) ) << 4 ) | ( ( ( y & 0x10 ) | ( x & 0x20 ) ) << 5 ) | ( ( y & 0x20 ) << 6 );
#endif
			
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
			
#ifdef USE_CVT_LUT
			Offset = LUT_CvtAddrPix16s [ ( x & 0x3f ) | ( ( y & 0x3f ) << 6 ) ];
#else
			// first bit of y goes into second bit of x
			//Offset = ( ( x & 8 ) >> 3 ) | ( ( x & 1 ) << 1 ) | ( ( y & 1 ) << 2 ) | ( ( x & 6 ) << 2 ) | ( ( y & 0xe ) << 4 ) | ( ( x & 0x10 ) << 4 ) | ( ( y & 0x20 ) << 4 )
			//		| ( ( y & 0x10 ) << 6 ) | ( ( x & 0x20 ) << 6 );
			Offset = ( ( x & 8 ) >> 3 ) | ( ( x & 1 ) << 1 ) | ( ( ( y & 1 ) | ( x & 6 ) ) << 2 ) | ( ( ( y & 0x2e ) | ( x & 0x10 ) ) << 4 ) | ( ( ( y & 0x10 ) | ( x & 0x20 ) ) << 6 );
#endif
			
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
			
#ifdef USE_CVT_LUT
			Offset = LUT_CvtAddrPix8 [ ( x & 0x7f ) | ( ( y & 0x3f ) << 7 ) ];
#else
			// first bit of y goes into second bit of x
			//Offset = ( ( y & 2 ) >> 1 ) | ( ( x & 8 ) >> 2 ) | ( ( x & 1 ) << 2 ) | ( ( y & 1 ) << 3 ) | ( ( x & 6 ) << 3 ) | ( ( y & 0xc ) << 4 ) | ( ( x & 0x10 ) << 4 ) | ( ( y & 0x10 ) << 5 ) | ( ( x & 0x20 ) << 5 ) | ( ( y & 0x20 ) << 6 ) | ( ( x & 0x40 ) << 6 );
			Offset = ( ( y & 2 ) >> 1 ) | ( ( x & 8 ) >> 2 ) | ( ( x & 1 ) << 2 ) | ( ( ( y & 1 ) | ( x & 6 ) ) << 3 ) | ( ( ( y & 0xc ) | ( x & 0x10 ) ) << 4 ) | ( ( ( y & 0x10 ) | ( x & 0x20 ) ) << 5 ) | ( ( ( y & 0x20 ) | ( x & 0x40 ) ) << 6 );
			
			// xor
			Offset ^= ( ( y & 2 ) << 4 ) ^ ( ( y & 4 ) << 3 );
#endif
			
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
			
#ifdef USE_CVT_LUT
			Offset = LUT_CvtAddrPix4 [ ( x & 0x7f ) | ( ( y & 0x7f ) << 7 ) ];
#else
			// first bit of y goes into second bit of x
			//Offset = ( ( y & 2 ) >> 1 ) | ( ( x & 8 ) >> 2 ) | ( ( x & 0x10 ) >> 2 ) | ( ( x & 1 ) << 3 ) | ( ( y & 1 ) << 4 ) | ( ( x & 6 ) << 4 ) | ( ( y & 0x1c ) << 5 ) | ( ( x & 0x20 ) << 5 ) | ( ( y & 0x20 ) << 6 ) | ( ( x & 0x40 ) << 6 ) | ( ( y & 0x40 ) << 7 );
			Offset = ( ( y & 2 ) >> 1 ) | ( ( x & 0x18 ) >> 2 ) | ( ( x & 1 ) << 3 ) | ( ( ( y & 1 ) | ( x & 6 ) ) << 4 ) | ( ( ( y & 0x1c ) | ( x & 0x20 ) ) << 5 ) | ( ( ( y & 0x20 ) | ( x & 0x40 ) ) << 6 ) | ( ( y & 0x40 ) << 7 );
			
			// xor
			Offset ^= ( ( y & 2 ) << 5 ) ^ ( ( y & 4 ) << 4 );
#endif
			
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
			
#ifdef USE_CVT_LUT
			Offset = LUT_CvtAddrZBuf32 [ ( x & 0x3f ) | ( ( y & 0x1f ) << 6 ) ];
#else
			//Offset = ( x & 0x1 ) | ( ( y & 0x1 ) << 1 ) | ( ( x & 0x6 ) << 1 ) | ( ( y & 0x6 ) << 3 ) | ( ( x & 0x8 ) << 3 ) | ( ( y & 0x8 ) << 4 ) | ( ( x & 0x10 ) << 4 ) | ( ( y & 0x10 ) << 5 ) | ( ( x & 0x20 ) << 5 );
			Offset = ( x & 0x1 ) | ( ( ( y & 0x1 ) | ( x & 0x6 ) ) << 1 ) | ( ( ( y & 0x6 ) | ( x & 0x8 ) ) << 3 ) | ( ( ( y & 0x8 ) | ( x & 0x10 ) ) << 4 ) | ( ( ( y & 0x10 ) | ( x & 0x20 ) ) << 5 );
			
			// reverse last two bits for z-buffer
			Offset ^= 0x600;
#endif
			
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
			
#ifdef USE_CVT_LUT
			Offset = LUT_CvtAddrZBuf16 [ ( x & 0x3f ) | ( ( y & 0x3f ) << 6 ) ];
#else
			// first bit of y goes into second bit of x
			//Offset = ( ( x & 8 ) >> 3 ) | ( ( x & 1 ) << 1 ) | ( ( y & 1 ) << 2 ) | ( ( x & 6 ) << 2 ) | ( ( y & 0xe ) << 4 ) | ( ( x & 0x10 ) << 4 ) | ( ( y & 0x10 ) << 5 )
			//		| ( ( x & 0x20 ) << 5 ) | ( ( y & 0x20 ) << 6 );
			Offset = ( ( x & 8 ) >> 3 ) | ( ( x & 1 ) << 1 ) | ( ( ( y & 1 ) | ( x & 6 ) ) << 2 ) | ( ( ( y & 0xe ) | ( x & 0x10 ) ) << 4 ) | ( ( ( y & 0x10 ) | ( x & 0x20 ) ) << 5 ) | ( ( y & 0x20 ) << 6 );
			
			// reverse last two bits for z-buffer
			Offset ^= 0xc00;
#endif
			
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
			
#ifdef USE_CVT_LUT
			Offset = LUT_CvtAddrZBuf16s [ ( x & 0x3f ) | ( ( y & 0x3f ) << 6 ) ];
#else
			// first bit of y goes into second bit of x
			//Offset = ( ( x & 8 ) >> 3 ) | ( ( x & 1 ) << 1 ) | ( ( y & 1 ) << 2 ) | ( ( x & 6 ) << 2 ) | ( ( y & 0xe ) << 4 ) | ( ( x & 0x10 ) << 4 ) | ( ( y & 0x20 ) << 4 )
			//		| ( ( y & 0x10 ) << 6 ) | ( ( x & 0x20 ) << 6 );
			Offset = ( ( x & 8 ) >> 3 ) | ( ( x & 1 ) << 1 ) | ( ( ( y & 1 ) | ( x & 6 ) ) << 2 ) | ( ( ( y & 0x2e ) | ( x & 0x10 ) ) << 4 ) | ( ( ( y & 0x10 ) | ( x & 0x20 ) ) << 6 );
			
			// xor
			Offset ^= 0xc00;
#endif
			
			// add in the rest of the x-bits
			Offset |= ( ( x & ~0x3f ) << 6 );
			
			
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
		static const s32 c_MaxPolygonWidth = 2048;
		static const s32 c_MaxPolygonHeight = 2048;

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
		

#ifdef OLD_GIF_TRANSFER
		void GIF_FIFO_Execute ( u64 ull0, u64 ull1 );
#else
		u32 GIF_FIFO_Execute ( u64* pData64, u32 Count64 );
#endif
		
		void TransferDataLocal ();
		void TransferDataOut32 ( u32* Data, u32 WordCount32 );
		void TransferDataIn32 ( u32* Data, u32 WordCount32 );
		//void TransferDataOut ();
		
		// path1 needs to know when to stop feeding data
		u32 EndOfPacket [ 4 ], Tag_Done;
		
		
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
		union TEX1_t
		{
			struct
			{
				// bit 0 - LCM - LOD calculation method
				// 0: LOD = (Log2(1/|Q|)<<L)+K; 1: LOD=K
				u64 LCM : 1;
				
				// bit 1 - unused
				u64 unused0 : 1;
				
				// bits 2-4 - MXL - Maximum MIP Level (ranges from 0-6)
				u64 MXL : 3;
				
				// bit 5 - MMAG - filter to use when LOD<0
				// 0: nearest; 1: linear
				u64 MMAG : 1;
				
				// bits 6-8 - MMIN - filter to use when LOD>=0
				u64 MMIN : 3;
				
				// bit 9 - MTBA - base address specification
				u64 MTBA : 1;
				
				// bits 10-18 - unused
				u64 unused1 : 9;
				
				// bits 19-20 - L
				u64 L : 2;
				
				// bits 21-31 - unused
				u64 unused2 : 11;
				
				// bits 32-43 - K
				u64 K : 12;
			};
			
			u64 Value;
		};
		
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
				
				TEX1_t TEX1_1;	// 0x14
				TEX1_t TEX1_2;
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
		static const double NTSC_FramesPerSec = 59.94005994L;
		static const double PAL_FramesPerSec = 50.0L;
		static const double NTSC_FieldsPerSec = ( 59.94005994L / 2.0L );	//NTSC_FieldsPerSec / 2;
		static const double PAL_FieldsPerSec = ( 50.0L / 2.0L );	//PAL_FieldsPerSec / 2;
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
		
		
		// threads
		static const u32 c_iMaxThreads = 32;
		static u32 ulNumberOfThreads;
		static Api::Thread* GPUThreads [ c_iMaxThreads ];
		
		// free buffer space required for program to run while multi-threading
		static const u32 c_ulRequiredBuffer = 20000;
		
		static const u32 c_ulInputBuffer_Size = 1 << 16;
		static const u32 c_ulInputBuffer_Mask = c_ulInputBuffer_Size - 1;
		static const u32 c_ulInputBuffer_Shift = 5;
		//volatile u32 ulThreads_Idle;
		static volatile u64 ullInputBuffer_Index;
		static volatile u32 ulInputBuffer_Count;
		static volatile u64 inputdata [ ( 1 << c_ulInputBuffer_Shift ) * c_ulInputBuffer_Size ] __attribute__ ((aligned (32)));
		
		static volatile u32 ulInputBuffer_WriteIndex;
		static volatile u32 ulInputBuffer_TargetIndex;
		static volatile u32 ulInputBuffer_ReadIndex;

		static int Start_GPUThread( void* Param );
		
		static void Start_Frame ( void );
		static void End_Frame ( void );
		
		
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
		u32 XferSrcOffset32, XferDstOffset32, XferSrcBufWidth, XferDstBufWidth, XferSrcX, XferSrcY, XferDstX, XferDstY, XferWidth, XferHeight;
		//u32 XferDstBufWidth64, XferSrcBufWidth64, XferSrcPixelSize, XferDstPixelSize;
		u32 XferX, XferY;
		u64 PixelCount;
		u64 PixelShift;
		u64 BITBLTBUF_DPSM;
		u64 BITBLTBUF_SPSM;
		
		
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
		void SetDrawVars_Line ( u64 *inputdata_ptr, u32 Coord0, u32 Coord1, u32 Coord2 );

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

template<const long COLCLAMP>
inline static s32 ColClamp8_t ( s32 Component )
{
	// if COLCLAMP then clamp, otherwise just mask to 8-bits
	if ( COLCLAMP & 1 )
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

		
		

		
		
/*
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


template<const long TEST_AFAIL,const long ZBUF_PSM>
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
			if ( TEST_X.ZTE && !ZBUF_X.ZMSK )
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


template<const long TEST_AFAIL,const long ZBUF_PSM>
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
			if ( TEST_X.ZTE && !ZBUF_X.ZMSK )
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
			
	
	// otherwise, return zero
	return 0;
}


template<const long TEST_ZTST,const long ZBUF_PSM>
inline u32 PerformZDepthTest16_t ( u16* zptr16, u32 ZValue32 )
{
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
	
		
	return 0;
}


template<const long TEST_ZTST,const long ZBUF_PSM>
inline u32 PerformZDepthTest32_t ( u32* zptr32, u32 ZValue32 )
{

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
*/
		



#ifdef USE_TEMPLATES_PS2_SPRITE


template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_ZTST,const long ZBUF_PSM,const long FRAME_PSM>
inline void PlotPixel_Texture_t ( s32 x0, s32 y0, s64 z0, u32 bgr, u32 c1, u32 c2, u32 c3, u32 ca, u32 fv )
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
	if ( ! ( FRAME_PSM & 2 ) )
	{
		// 32-bit frame buffer //

		// get pointer into frame buffer //
		
		//ptr32 = & ( buf32 [ CvtAddrPix32 ( x0, y0, FrameBufferWidth_Pixels ) ] );
		
		switch ( FRAME_PSM )
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
			zptr32 = GetZBufPtr32_t<ZBUF_PSM> ( zbuf32, x0, y0, FrameBufferWidth_Pixels );
			
			
			// determine z-buffer format
			// and test against the value in the z-buffer
			//ZBufValue = GetZBufValue32 ( zptr32 );
			ZDepthTest_Pass = PerformZDepthTest32_t<TEST_ZTST,ZBUF_PSM> ( zptr32, z0 );
			
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

			SrcAlphaTest_Pass = TestSrcAlpha32_t<TEST_ATST> ( bgr_temp );

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
				
					WriteZBufValue32_t<ZBUF_PSM> ( zptr32, z0 );
					
				} // end if ( !ZBUF_X.ZMSK )
			
#endif

#ifdef ENABLE_SRC_ALPHA_TEST
			}
			else
			{
				// source alpha test failed //
				
				PerformAlphaFail32_t<TEST_AFAIL,ZBUF_PSM> ( zptr32, ptr32, bgr_temp, DestPixel, z0 );
				
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
		
		switch ( FRAME_PSM )
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
			zptr16 = GetZBufPtr16_t<ZBUF_PSM> ( (u16*) zbuf32, x0, y0, FrameBufferWidth_Pixels );
			
			// determine z-buffer format
			// and test against the value in the z-buffer
					
			// PSMZ16
			// PSMZ16S
			//ZBufValue = ( (u64) ( *zptr16 ) ) << 16;
			ZDepthTest_Pass = PerformZDepthTest16_t<TEST_ZTST,ZBUF_PSM> ( zptr16, z0 );
			
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
			SrcAlphaTest_Pass = TestSrcAlpha32_t<TEST_ATST> ( bgr_temp );
			
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
				WriteZBufValue32_t<ZBUF_PSM> ( (u32*) zptr16, z0 );
				
			} // end if ( !ZBUF_X.ZMSK )
#endif

			
#ifdef ENABLE_SRC_ALPHA_TEST
			}
			else
			{
				// source alpha test failed //
				
				PerformAlphaFail16_t<TEST_AFAIL,ZBUF_PSM> ( zptr16, ptr16, bgr_temp, DestPixel, z0 );
				
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


template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_ZTST,const long FRAME_PSM,const long ZBUF_PSM,const long TEX_PSM>
static void RenderSprite_t ( u32 Coord0, u32 Coord1 )
{
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << "; RenderSprite";
#endif

	// before sse2, was sending 1 pixels at a time
	static const int c_iVectorSize = 1;

	// notes: looks like sprite size is same as specified by w/h

	//u32 Pixel,
	
	u16 *CLUT_LUT;
	
	u32 TexelIndex;
	
	u32 color_add;
	
	u32 *ptr_texture;
	u8 *ptr_texture8;
	u16 *ptr_texture16;
	u32 *ptr_texture32;
	u32 *ptr_clut;
	u16 *ptr_clut16;
	u32 clut_width, clut_x, clut_y;

	// not used
	//u32 clut_xoffset, clut_yoffset;
	
	//u16 *ptr;
	s32 StartX, EndX, StartY, EndY;
	u32 NumberOfPixelsDrawn;
	
	
	u32 *ptr32;
	u16 *ptr16;
	
	u32 *zptr32;
	u16 *zptr16;
	
	u32 DestPixel, bgr, bgr_temp;
	
	// 12.4 fixed point
	s32 x0, y0, x1, y1;
	s32 u0, v0, u1, v1;

	// and the z coords
	s64 z0, z1;
	
	// interpolate the z ??
	s64 dzdx, dzdy;
	s64 iZ;
	
	// interpolate f for fog
	s64 f0, f1;
	s64 dfdx, dfdy;
	s64 iF;
	
	
	s32 iU, iV, dudx, dvdy;
	
	s32 Line, x_across;
	
	u32 c1, c2, c3, ca;
	
	s64 Temp;
	
	//u32 tge;
	
	//u32 DestPixel, PixelMask = 0, SetPixelMask = 0;
	s32 TexCoordX, TexCoordY;
	
	u32 Shift1 = 0, Shift2 = 0, And1 = 0, And2 = 0;
	u32 Shift0 = 0;
	u32 TextureOffset;
	
	u32 And3, Shift3, Shift4;
	
	u32 PixelsPerLine;
	
	u32 PixelFormat;
	u32 CLUTBufBase32, CLUTPixelFormat, CLUTStoreMode, CLUTOffset;
	
	u32 TexBufWidth, TexWidth, TexHeight, TexWidth_Mask, TexHeight_Mask;
	
	s32 TexX_Min, TexX_Max, TexY_Min, TexY_Max;
	u32 TexX_And, TexX_Or, TexY_And, TexY_Or;

	

	// set fixed alpha values
	AlphaSelect [ 2 ] = ALPHA_X.FIX << 24;
	AlphaSelect [ 3 ] = 0;
	
	// color lookup table width is stored in TEXCLUT register (in pixels/64)
	// get clut width and x in pixels
	clut_width = GPURegsGp.TEXCLUT.CBW << 6;
	clut_x = GPURegsGp.TEXCLUT.COU << 6;
	
	// get clut y in units of pixels
	clut_y = GPURegsGp.TEXCLUT.COV;
	

	//TEX0_t *TEX0 = &GPURegsGp.TEX0_1;
	TEX0_t *TEX0 = TEXX;

	TEX1_t *TEX1 = &GPURegsGp.TEX1_1;
	
	
	TexBufWidth = TEX0 [ Ctx ].TBW0 << 6;
	TexWidth = 1 << TEX0 [ Ctx ].TW;
	TexHeight = 1 << TEX0 [ Ctx ].TH;
	TexWidth_Mask = TexWidth - 1;
	TexHeight_Mask = TexHeight - 1;
	
	
	
	//TexWidth_Shift = TEX0 [ Ctx ].TW;
	
	/////////////////////////////////////////////////////////
	// Get offset into texture page
	//TextureOffset = ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 );
	TexBufStartOffset32 = TEX0 [ Ctx ].TBP0 << 6;
	
	// get the pixel format (16 bit, 32 bit, etc)
	PixelFormat = TEX0 [ Ctx ].PSM;
	CLUTPixelFormat = TEX0 [ Ctx ].CPSM;
	
	// get base pointer to color lookup table (32-bit word address divided by 64)
	CLUTBufBase32 = TEX0 [ Ctx ].CBP << 6;
	
	// storage mode ??
	CLUTStoreMode = TEX0 [ Ctx ].CSM;
	
	// clut offset ??
	// this is the offset / 16
	// note: this is actually the position in the temporary buffer, not in the local memory
	CLUTOffset = TEX0 [ Ctx ].CSA;

	

	
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << "; TexPixFmt=" << PixelFormat_Names [ PixelFormat ];
#endif

	//tge = command_tge;
	//if ( ( bgr & 0x00ffffff ) == 0x00808080 ) tge = 1;
	
	//////////////////////////////////////////////////////
	// Get offset into color lookup table
	//u32 ClutOffset = ( clut_x << 4 ) + ( clut_y << 10 );
	
	//clut_xoffset = clut_x << 4;
	//clut_xoffset = 0;
	//ptr_clut = & ( VRAM [ clut_y << 10 ] );
	ptr_clut = & ( RAM32 [ CLUTBufBase32 ] );
	//ptr_texture = & ( VRAM [ ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) ] );
	ptr_texture = & ( RAM32 [ TexBufStartOffset32 ] );
	
	// *** TODO *** use a union
	ptr_texture8 = (u8*) ptr_texture;
	ptr_texture16 = (u16*) ptr_texture;
	ptr_texture32 = (u32*) ptr_texture;
	
	// x offset for CLUT in units of pixels
	//clut_xoffset = CLUTOffset << 4;
	
	
	// texture pixel format variables //
	
	
	
	
	//if ( PixelFormat == 1 )
	if ( TEX_PSM == 1 )
	{
		// rgb24 //
		
		// TEXA.TA0 is the alpha of rgb24 value
		RGB24_Alpha = GPURegsGp.TEXA.TA0 << 24;
		
		// 24-bit pixel only
		Pixel24_Mask = 0xffffff;
		
		// r,g,b=0 is transparent for rgb24 when TEXA.AEM is one
		RGB24_TAlpha = ( GPURegsGp.TEXA.AEM ^ 1 ) << 31;
	}
	else
	{
		// NOT rgb24 //
		RGB24_Alpha = 0;
		Pixel24_Mask = 0xffffffff;
		RGB24_TAlpha = 0;
	}
	

	//switch ( PixelFormat )
	switch ( TEX_PSM )
	{
		// PSMCT32
		case 0:
			
		// PSMCT24
		case 1:
			
		// PSMCT16
		case 2:
			
		// PSMCT16S
		case 0xa:
			
		// PSMT8
		case 0x13:
			
		// PSMT4
		case 0x14:
			
		// PSMT8H
		case 0x1b:
			
		// PSMT4HL
		case 0x24:
			
		// PSMT4HH
		case 0x2c:
			
		// Z-buffer formats
		
		// PSMZ32
		case 0x30:
		
		// PSMZ24
		case 0x31:
		//case 0x35:
			
		// PSMZ16
		case 0x32:
			
		// PSMZ16S
		case 0x3a:
			break;
			
		// UNKNOWN PIXEL FORMAT
		default:
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << "; TexPixFmt=" << PixelFormat_Names [ TEX_PSM ];
#endif

			cout << "\nhps2x64: GPU: ERROR: Unknown Pixel Format: " << hex << TEX_PSM << " Cycle#" << dec << *_DebugCycleCount << "\n";
			return;
			break;
	}

	
	// color lookup table pixel format variables //
	
	// assume clut bpp = 24/32, 32-bit ptr
	And3 = 0; Shift3 = 0; Shift4 = 0;
	
	if ( ( CLUTPixelFormat == 0x2 ) || ( CLUTPixelFormat == 0xa ) )
	{
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << " CLUT16BPP";
#endif

		// bpp = 16, ptr = 32-bit
		And3 = 1;
		Shift3 = 1;
		Shift4 = 4;
		
	}

	
	if ( !CLUTStoreMode )
	{
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << " CSM1";
#endif

		// CSM1 //
		//CLUT_LUT = ucCLUT_Lookup_CSM01_4bpp;
		
		if ( CLUTPixelFormat & 0x2 )
		{
			// 4-bit pixels - 16 colors
			// 16-bit pixels in CLUT //
			CLUTOffset &= 0x1f;
		}
		else
		{
			// 32-bit pixels in CLUT //
			CLUTOffset &= 0xf;
		}
		
	}
	else
	{
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << " CSM2";
#endif

		// CSM2 //
		//CLUT_LUT = ucCLUT_Lookup_CSM02;
		
		// use clut_x, clut_y, clut_width
		//ptr_clut = & ( ptr_clut [ ( clut_x >> Shift3 ) + ( clut_y * ( clut_width >> Shift3 ) ) ] );
	}
	
	CLUTOffset <<= 4;
	ptr_clut16 = & ( InternalCLUT [ CLUTOffset ] );
	
	// important: reading from buffer in 32-bit units, so the texture buffer width should be divided accordingly
	//TexBufWidth >>= Shift1;
	
	// get the shade color
	//color_add = bgr;
	c1 = GPURegsGp.RGBAQ.R;
	c2 = GPURegsGp.RGBAQ.G;
	c3 = GPURegsGp.RGBAQ.B;
	ca = GPURegsGp.RGBAQ.A;

	// initialize number of pixels drawn
	NumberOfPixelsDrawn = 0;
	
	
	// get x,y
	x0 = xyz [ Coord0 ].X;
	x1 = xyz [ Coord1 ].X;
	y0 = xyz [ Coord0 ].Y;
	y1 = xyz [ Coord1 ].Y;
	
	// get z
	z0 = (u64) xyz [ Coord0 ].Z;
	z1 = (u64) xyz [ Coord1 ].Z;
	

	// z0 should be same as z1 ??
	z0 = z1;

	// get fog coefficient
	f0 = (u64) f [ Coord0 ].F;
	f1 = (u64) f [ Coord1 ].F;
	
	// get dzdx/dzdy for z
	// ***TODO*** this is incorrect since sometimes the x/y coords get switched. Need to fix
	if ( x1 - x0 )
	{
		//dzdx = ( ( (s64)z1 - (s64)z0 ) << 27 ) / ( (s64) ( x1 - x0 ) );
		dfdx = ( ( (s64)f1 - (s64)f0 ) << 28 ) / ( (s64) ( x1 - x0 ) );
	}
	
	if ( y1 - y0 )
	{
		//dzdy = ( ( (s64)z1 - (s64)z0 ) << 27 ) / ( (s64) ( y1 - y0 ) );
		dfdy = ( ( (s64)f1 - (s64)f0 ) << 28 ) / ( (s64) ( y1 - y0 ) );
	}
	
	// check if sprite should use UV coords or ST coords
	// on PS2, sprite can use the ST register to specify texture coords
	//if ( GPURegsGp.PRIM.FST )
	if ( Fst )
	{
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << " FST";
#endif

		// get u,v
		u0 = uv [ Coord0 ].U;
		u1 = uv [ Coord1 ].U;
		v0 = uv [ Coord0 ].V;
		v1 = uv [ Coord1 ].V;
		
		// if using u,v coords, then the larger point should be minus one?
		//if ( u1 > u0 ) u1 -= ( 1 << 4 ); else u0 -= ( 1 << 4 );
		//if ( v1 > v0 ) v1 -= ( 1 << 4 ); else v0 -= ( 1 << 4 );
	}
	else
	{
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << " !FST";
#endif

		// get s,t
		// this appears to be correct
		//u0 = ( st [ Coord0 ].S >> 16 ) & 0x3fff;
		//u1 = ( st [ Coord1 ].S >> 16 ) & 0x3fff;
		//v0 = ( st [ Coord0 ].T >> 16 ) & 0x3fff;
		//v1 = ( st [ Coord1 ].T >> 16 ) & 0x3fff;
		
		// put s,t coords into 10.4 fixed point
		// note: tex width/height should probably be minus one
		u0 = (s32) ( st [ Coord0 ].fS * ( (float) ( TexWidth ) ) * 16.0f );
		u1 = (s32) ( st [ Coord1 ].fS * ( (float) ( TexWidth ) ) * 16.0f );
		v0 = (s32) ( st [ Coord0 ].fT * ( (float) ( TexHeight ) ) * 16.0f );
		v1 = (s32) ( st [ Coord1 ].fT * ( (float) ( TexHeight ) ) * 16.0f );
	}
	
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << hex << " bgr=" << bgr;
	debug << dec << "; Coords: x0=" << ( x0 >> 4 ) << " y0=" << ( y0 >> 4 ) << " x1=" << ( x1 >> 4 ) << " y1=" << ( y1 >> 4 );
	debug << dec << " u0=" << ( u0 >> 4 ) << " v0=" << ( v0 >> 4 ) << " u1=" << ( u1 >> 4 ) << " v1=" << ( v1 >> 4 );
	debug << hex << " z0=" << z0 << " z1=" << z1;
#endif

	//////////////////////////////////////////
	// get coordinates on screen
	// *note* this is different from PS1, where you would add the offsets..
	x0 -= Coord_OffsetX;
	y0 -= Coord_OffsetY;
	x1 -= Coord_OffsetX;
	y1 -= Coord_OffsetY;
	
	// order coords so they go top to bottom and left to right
	if ( x1 < x0 )
	{
		// swap x,u coords
		Swap ( x0, x1 );
		Swap ( u0, u1 );
		
		// swap z,f coords ??
		// note: possibly need to update the z,f coords ??
		Swap ( z0, z1 );
		Swap ( f0, f1 );
	}
	
	if ( y1 < y0 )
	{
		// swap y,v coords
		Swap ( y0, y1 );
		Swap ( v0, v1 );
		
		// swap z,f coords ??
		// note: possibly need to update the z,f coords ??
		Swap ( z0, z1 );
		Swap ( f0, f1 );
	}

	
	// looks like some sprites have y1 < y0 and/or x1 < x0
	// didn't expect that so need to alert and figure out some other time
	if ( ( y1 < y0 ) || ( x1 < x0 ) )
	{
#if defined INLINE_DEBUG_SPRITE_TEST || defined INLINE_DEBUG_PRIMITIVE_TEST
		debug << dec << "\r\nERROR: y1 < y0 or x1 < x0 in sprite!!!";
		debug << dec << " FinalCoords: x0=" << x0 << " y0=" << y0 << " x1=" << x1 << " y1=" << y1;
#endif

#ifdef VERBOSE_SPRITE_ERROR
		cout << "\nhps2x64: GPU: ERROR: Sprite has x1 < x0 or y1 < y0!!!";
#endif

		return;
	}
	
	
	// tex coords are in 10.4 fixed point
	//u0 <<= 12;
	//v0 <<= 12;
	//u1 <<= 12;
	//v1 <<= 12;
	u0 <<= 16;
	v0 <<= 16;
	u1 <<= 16;
	v1 <<= 16;
	
	if ( x1 - x0 ) dudx = ( u1 - u0 ) / ( x1 - x0 );
	if ( y1 - y0 ) dvdy = ( v1 - v0 ) / ( y1 - y0 );

	// shift back down to get into 16.16 format
	u0 >>= 4;
	v0 >>= 4;
	u1 >>= 4;
	v1 >>= 4;

	// the coords on the left and top are kept when equal (12.4 fixed point)
	//x0 += 0xf;
	//y0 += 0xf;
	
	// the coords on the right and bottom are not kept when equal (12.4 fixed point)
	//x1 -= 1;
	//y1 -= 1;
	
	// coords are in 12.4 fixed point
	//x0 >>= 4;
	//y0 >>= 4;
	//x1 >>= 4;
	//y1 >>= 4;
	StartX = ( x0 + 0xf ) >> 4;
	EndX = ( x1 - 1 ) >> 4;
	StartY = ( y0 + 0xf ) >> 4;
	EndY = ( y1 - 1 ) >> 4;
	
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << dec << "; FinalCoords: x0=" << x0 << " y0=" << y0 << " x1=" << x1 << " y1=" << y1;
	debug << hex << " u0=" << u0 << " v0=" << v0 << " u1=" << u1 << " v1=" << v1;
#endif

	

	// check if sprite is within draw area
	if ( EndX < Window_XLeft || StartX > Window_XRight || EndY < Window_YTop || StartY > Window_YBottom ) return;
#if defined INLINE_DEBUG_SPRITE_TEST || defined INLINE_DEBUG_PRIMITIVE_TEST
	else
	{
		debug << dec << "\r\nDrawSprite" << " FinalCoords: x0=" << ( x0 >> 4 ) << " y0=" << ( y0 >> 4 ) << " x1=" << ( x1 >> 4 ) << " y1=" << ( y1 >> 4 );
		debug << dec << " u0=" << ( u0 >> 16 ) << " v0=" << ( v0 >> 16 ) << " u1=" << ( u1 >> 16 ) << " v1=" << ( v1 >> 16 );
		debug << hex << " z0=" << z0 << " z1=" << z1;
		debug << hex << "; TexBufPtr32/64=" << ( TexBufStartOffset32 >> 6 );
		debug << dec << "; TexWidth=" << TexWidth << " TexHeight=" << TexHeight;
		debug << dec << "; TexBufWidth=" << TexBufWidth;
		debug << "; TexPixFmt=" << PixelFormat_Names [ TEX_PSM ];
		debug << "; Clamp_ModeX=" << Clamp_ModeX << " Clamp_ModeY=" << Clamp_ModeY;
		debug << hex << "; CLUTBufPtr32/64=" << ( CLUTBufBase32 >> 6 );
		debug << "; CLUTPixFmt=" << PixelFormat_Names [ CLUTPixelFormat ];
		debug << hex << "; CLUTOffset/16=" << CLUTOffset;
		debug << "; CSM=" << CLUTStoreMode;
		debug << "; TEXCLUT=" << hex << GPURegsGp.TEXCLUT.Value;
		debug << hex << "; FrameBufPtr32/64=" << ( FrameBufferStartOffset32 >> 6 );
		debug << dec << " FrameBufWidth=" << FrameBufferWidth_Pixels;
		debug << " FrameBufPixFmt=" << PixelFormat_Names [ FrameBuffer_PixelFormat ];
		debug << " Shift1=" << hex << Shift1;
		debug << " And3=" << hex << And3;
		debug << " Alpha=" << Alpha;
		debug << " PABE=" << GPURegsGp.PABE;
		debug << " FBA=" << FBA_X;
		debug << hex << " ZBuf=" << (ZBufferStartOffset32 >> 11);
		debug << PixelFormat_Names [ ZBuffer_PixelFormat ];
		debug << " ZFlags=" << ZBUF_X.Value;
		debug << " TEST=" << TEST_X.Value;
		debug << " MXL=" << TEX1 [ Ctx ].MXL;
		debug << " LCM=" << TEX1 [ Ctx ].LCM;
		debug << " K=" << TEX1 [ Ctx ].K;
	}
#endif



	switch ( Clamp_ModeY )
	{
		case 0:
			// repeat //
			//TexCoordY &= TexHeight_Mask;
			TexY_And = TexHeight_Mask;
			
			TexY_Or = 0;
			
			// can only have coords in range -2047 to +2047
			TexY_Min = -2047;
			TexY_Max = 2047;
			break;
			
		case 1:
			// clamp //
			//TexCoordY = ( TexCoordY < 0 ) ? 0 : TexCoordY;
			//TexCoordY = ( TexCoordY > TexHeight ) ? TexHeight : TexCoordY;
			TexY_Min = 0;
			TexY_Max = TexHeight_Mask;
			
			TexY_And = TexHeight_Mask;
			TexY_Or = 0;
			break;
			
		case 2:
			// region clamp //
			//TexCoordY = ( TexCoordY < Clamp_MinV ) ? Clamp_MinV : TexCoordY;
			//TexCoordY = ( TexCoordY > Clamp_MaxV ) ? Clamp_MaxV : TexCoordY;
			TexY_Min = Clamp_MinV;
			TexY_Max = Clamp_MaxV;
			
			TexY_And = TexHeight_Mask;
			TexY_Or = 0;
			break;
			
		case 3:
			// region repeat //
			// this one is just like on the ps1
			//TexCoordY = ( TexCoordY & Clamp_MinV ) | Clamp_MaxV;
			TexY_And = Clamp_MinV & TexHeight_Mask;
			TexY_Or = Clamp_MaxV & TexHeight_Mask;
			
			// can only have coords in range -2047 to +2047
			TexY_Min = -2047;
			TexY_Max = 2047;
			break;
	}

	
	switch ( Clamp_ModeX )
	{
		case 0:
			// repeat //
			//TexCoordY &= TexHeight_Mask;
			TexX_And = TexWidth_Mask;
			
			TexX_Or = 0;
			
			// can only have coords in range -2047 to +2047
			TexX_Min = -2047;
			TexX_Max = 2047;
			break;
			
		case 1:
			// clamp //
			//TexCoordY = ( TexCoordY < 0 ) ? 0 : TexCoordY;
			//TexCoordY = ( TexCoordY > TexHeight ) ? TexHeight : TexCoordY;
			TexX_Min = 0;
			TexX_Max = TexWidth_Mask;
			
			TexX_And = TexWidth_Mask;
			TexX_Or = 0;
			break;
			
		case 2:
			// region clamp //
			//TexCoordY = ( TexCoordY < Clamp_MinV ) ? Clamp_MinV : TexCoordY;
			//TexCoordY = ( TexCoordY > Clamp_MaxV ) ? Clamp_MaxV : TexCoordY;
			TexX_Min = Clamp_MinU;
			TexX_Max = Clamp_MaxU;
			
			TexX_And = TexWidth_Mask;
			TexX_Or = 0;
			break;
			
		case 3:
			// region repeat //
			// this one is just like on the ps1
			//TexCoordY = ( TexCoordY & Clamp_MinV ) | Clamp_MaxV;
			TexX_And = Clamp_MinU & TexWidth_Mask;
			TexX_Or = Clamp_MaxU & TexWidth_Mask;
			
			// can only have coords in range -2047 to +2047
			TexX_Min = -2047;
			TexX_Max = 2047;
			break;
	}


	
	//StartX = x0;
	//EndX = x1;
	//StartY = y0;
	//EndY = y1;
	
	// give z and f a fractional part
	//z0 <<= 23;
	f0 <<= 24;
	
	
	Temp = ( StartY << 4 ) - y0;

	if ( StartY < Window_YTop )
	{
		//v0 += ( Window_YTop - StartY ) * dvdy;
		Temp += ( Window_YTop - StartY ) << 4;
		StartY = Window_YTop;
	}
	
	v0 += ( dvdy >> 4 ) * Temp;
	
	// also update z and f
	//z0 += ( dzdy >> 4 ) * Temp;
	f0 += ( dfdy >> 4 ) * Temp;
	
	if ( EndY > Window_YBottom )
	{
		EndY = Window_YBottom;
	}
	
	
	Temp = ( StartX << 4 ) - x0;
	
	if ( StartX < Window_XLeft )
	{
		//u0 += ( Window_XLeft - StartX ) * dudx;
		Temp += ( Window_XLeft - StartX ) << 4;
		StartX = Window_XLeft;
	}
	
	u0 += ( dudx >> 4 ) * Temp;
	
	// also update z and f
	//z0 += ( dzdx >> 4 ) * Temp;
	f0 += ( dfdx >> 4 ) * Temp;
	
	if ( EndX > Window_XRight )
	{
		EndX = Window_XRight;
	}
	

	// check that there is a pixel to draw
	if ( ( EndX < StartX ) || ( EndY < StartY ) )
	{
#if defined INLINE_DEBUG_SPRITE_TEST || defined INLINE_DEBUG_PRIMITIVE_TEST
		debug << dec << "\r\nERROR: EndY < StartY or EndX < StartX in sprite!!!";
		debug << dec << " FinalCoords: StartX=" << StartX << " StartY=" << StartY << " EndX=" << EndX << " EndY=" << EndY;
#endif

		return;
	}
	
	
	
	
	NumberOfPixelsDrawn = ( EndX - StartX + 1 ) * ( EndY - StartY + 1 );
		
	// there are probably multiple pixel pipelines, so  might need to divide by like 8 or 16 or something
	// multiplying by 2 since the pixels has to be read before drawing
	//BusyUntil_Cycle = *_DebugCycleCount + ( NumberOfPixelsDrawn << 1 );
	if ( BusyUntil_Cycle < *_DebugCycleCount )
	{
		BusyUntil_Cycle = *_DebugCycleCount + ( ( NumberOfPixelsDrawn << 1 ) >> 4 );
	}
	//else
	//{
	//	BusyUntil_Cycle += ( NumberOfPixelsDrawn << 1 );
	//}

	
	//iV = v;
	iV = v0;
	

		for ( Line = StartY; Line <= EndY; Line++ )
		{
//#if defined INLINE_DEBUG_SPRITE_PIXEL
//	debug << "\r\n";
//#endif
//#if defined INLINE_DEBUG_SPRITE_PIXEL
//	debug << " y=" << dec << Line;
//#endif

	
			// need to start texture coord from left again
			//iU = u;
			iU = u0;
			
			//TexCoordY = (u8) ( ( iV & ~( TWH << 3 ) ) | ( ( TWY & TWH ) << 3 ) );
			//TexCoordY = (u8) ( ( iV & Not_TWH ) | ( TWYTWH ) );
			//TexCoordY <<= 10;
			TexCoordY = ( iV >> 16 ) /* & TexHeight_Mask */;
			
			TexCoordY = ( ( TexCoordY < TexY_Min ) ? TexY_Min : TexCoordY );
			TexCoordY = ( ( TexCoordY > TexY_Max ) ? TexY_Max : TexCoordY );
			TexCoordY &= TexY_And;
			TexCoordY |= TexY_Or;
			
			//ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			//ptr32 = & ( buf [ StartX + ( Line * FrameBufferWidth_Pixels ) ] );
			
//#if defined INLINE_DEBUG_SPRITE_PIXEL
//	debug << " TY=" << dec << TexCoordY;
//#endif

			// initialize z for line
			//iZ = z0;
			
			// initialize f for line
			iF = f0;

			// draw horizontal line
			//for ( x_across = StartX; x_across <= EndX; x_across++ )
			for ( x_across = StartX; x_across <= EndX /* && ptr32 < PtrEnd */; x_across += c_iVectorSize )
			{
//#if defined INLINE_DEBUG_SPRITE_PIXEL
//	debug << " x=" << dec << x_across;
//#endif

				// get pointer into frame buffer
				//ptr32 = & ( buf32 [ CvtAddrPix32 ( x_across, Line, FrameBufferWidth_Pixels ) ] );


				//TexCoordX = (u8) ( ( iU & Not_TWW ) | ( TWXTWW ) );
				TexCoordX = ( iU >> 16 ) /* & TexWidth_Mask */;
				
				TexCoordX = ( ( TexCoordX < TexX_Min ) ? TexX_Min : TexCoordX );
				TexCoordX = ( ( TexCoordX > TexX_Max ) ? TexX_Max : TexCoordX );
				TexCoordX &= TexX_And;
				TexCoordX |= TexX_Or;


//#if defined INLINE_DEBUG_SPRITE_PIXEL
//	debug << " TX=" << dec << TexCoordX;
//#endif


//if ( !Line )
//{				
//	debug << "\r\nx_across=" << dec << x_across << " Offset=" << CvtAddrPix32 ( x_across, Line, FrameBufferWidth_Pixels ) << " TOffset=" << ;
//}
				
				// get texel from texture buffer (32-bit frame buffer) //
				
				//bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + ( TexCoordY * ( TexBufWidth >> Shift1 ) ) ];
				//switch ( PixelFormat )
				switch ( TEX_PSM )
				{
					// PSMCT32
					case 0:
						bgr = ptr_texture32 [ CvtAddrPix32 ( TexCoordX, TexCoordY, TexBufWidth ) ];
						break;
						
					// PSMCT24
					case 1:
						bgr = ptr_texture32 [ CvtAddrPix32 ( TexCoordX, TexCoordY, TexBufWidth ) ];
						break;
						
					// PSMCT16
					case 2:
						bgr = ptr_texture16 [ CvtAddrPix16 ( TexCoordX, TexCoordY, TexBufWidth ) ];
						break;
						
					// PSMCT16S
					case 0xa:
						bgr = ptr_texture16 [ CvtAddrPix16S ( TexCoordX, TexCoordY, TexBufWidth ) ];
						break;
						
					// PSMT8
					case 0x13:
						bgr_temp = ptr_texture8 [ CvtAddrPix8 ( TexCoordX, TexCoordY, TexBufWidth ) ];
						break;
						
					// PSMT4
					case 0x14:
						bgr = CvtAddrPix4 ( TexCoordX, TexCoordY, TexBufWidth );
						bgr_temp = ( ( ptr_texture8 [ bgr >> 1 ] ) >> ( ( bgr & 1 ) << 2 ) ) & 0xf;
						break;
						
					// PSMT8H
					case 0x1b:
						bgr_temp = ( ptr_texture32 [ CvtAddrPix32 ( TexCoordX, TexCoordY, TexBufWidth ) ] ) >> 24;
						break;
						
					// PSMT4HL
					case 0x24:
						bgr_temp = ( ptr_texture32 [ CvtAddrPix32 ( TexCoordX, TexCoordY, TexBufWidth ) ] >> 24 ) & 0xf;
						break;
						
					// PSMT4HH
					case 0x2c:
						bgr_temp = ptr_texture32 [ CvtAddrPix32 ( TexCoordX, TexCoordY, TexBufWidth ) ] >> 28;
						break;
						
					// Z-buffer formats
					
					// PSMZ32
					case 0x30:
					
					// PSMZ24
					case 0x31:
					//case 0x35:
						bgr = ptr_texture32 [ CvtAddrZBuf32 ( TexCoordX, TexCoordY, TexBufWidth ) ];
						break;
						
					// PSMZ16
					case 0x32:
						bgr = ptr_texture16 [ CvtAddrZBuf16 ( TexCoordX, TexCoordY, TexBufWidth ) ];
						break;
						
					// PSMZ16S
					case 0x3a:
						bgr = ptr_texture16 [ CvtAddrZBuf16S ( TexCoordX, TexCoordY, TexBufWidth ) ];
						break;
				}
				
				// look up color value in CLUT if needed //
				//if ( ( PixelFormat & 7 ) >= 3 )
				if ( ( TEX_PSM & 7 ) >= 3 )
				{
					// lookup color value in CLUT //
					
//#if defined INLINE_DEBUG_SPRITE_PIXEL
//	debug << " IDX=" << hex << bgr_temp;
//#endif

					bgr = ptr_clut16 [ bgr_temp ];
					
					if ( ! ( CLUTPixelFormat & 0x2 ) )
					{
						// 32-bit pixels in CLUT //
						bgr |= ( (u32) ptr_clut16 [ bgr_temp + 256 ] ) << 16;
					}
				}
				
				// convert 16-bit pixels to 32-bit (since 32-bit frame buffer here) //
				
				// check if pixel is 16-bit
				//if ( ( ( PixelFormat == 0x2 ) || ( PixelFormat == 0xa ) ) || ( ( PixelFormat > 0xa ) && ( CLUTPixelFormat & 0x2 ) ) )
				if ( ( ( TEX_PSM == 0x2 ) || ( TEX_PSM == 0xa ) ) || ( ( TEX_PSM > 0xa ) && ( CLUTPixelFormat & 0x2 ) ) )
				{
					// pixel is 16-bit //
					
					// get alpha component of 16-bit pixel
					//bgr_temp = bgr & 0x8000;
					
					// check if pixel is definitely not transparent, if transparent then just stays zero
					if ( ( !GPURegsGp.TEXA.AEM ) || bgr )
					{
						// pixel is probably not transparent ?? //
						
						bgr_temp = bgr;
						
						// convert to 32-bit
						bgr = ConvertPixel16To24 ( bgr );
						
						// set the texture alpha for 16 bit pixel
						bgr |= ( ( ( bgr_temp & 0x8000 ) ? GPURegsGp.TEXA.TA1 : GPURegsGp.TEXA.TA0 ) << 24 );
					}
					/*
					else if ( bgr )
					{
						// set the texture alpha for 16 bit pixel
						bgr |= GPURegsGp.TEXA.TA0 << 24;
					}
					*/
				}
				
				//if ( PixelFormat == 1 )
				if ( TEX_PSM == 1 )
				{
					// 24-bit pixel //
					bgr &= 0xffffff;
					
					if ( ( !GPURegsGp.TEXA.AEM ) || bgr )
					{
						// alpha is TA0
						bgr |= ( GPURegsGp.TEXA.TA0 << 24 );
					}
#ifdef ENABLE_TRANSPARENT24
					else
					{
						// if alpha is not TA0, then it is transparent??
						iU += dudx;
						
						//iZ += dzdx;
						iF += dfdx;
						continue;
					}
#endif
				}
				


//#if defined INLINE_DEBUG_SPRITE_PIXEL
//	debug << " BGR=" << hex << bgr;
//#endif

				// texture function ??
				
				
				// fog function ??
				
				
				// no need to interpolate Z-value for sprite? just use z1 value?
				//PlotPixel_Texture ( x_across, Line, iZ >> 23, bgr );
				//PlotPixel_Texture ( x_across, Line, iZ >> 23, bgr, c1, c2, c3, ca, iF >> 24 );
				PlotPixel_Texture_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,ZBUF_PSM,FRAME_PSM> ( x_across, Line, z1, bgr, c1, c2, c3, ca, iF >> 24 );
				
				// update z on line
				//iZ += dzdx;
				
				// update f on line
				iF += dfdx;
				
				/////////////////////////////////////////////////////////
				// interpolate texture coords across
				//iU += c_iVectorSize;
				iU += dudx;
				
				// update pointer for pixel out
				//ptr32 += c_iVectorSize;
					
			}
			
			/////////////////////////////////////////////////////////
			// interpolate texture coords down
			//iV++;
			iV += dvdy;
			
			//z0 += dzdy;
			
			f0 += dfdy;
		}
		
		
	
}




template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_ZTST,const long FRAME_PSM,const long ZBUF_PSM>
inline void Select_RenderSprite6_t ( u32 Coord0, u32 Coord1 )
{
	switch ( TEXX [ Ctx ].PSM )
	{
		case 0:
			RenderSprite_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,ZBUF_PSM,0>( Coord0, Coord1 );
			break;
			
		case 1:
			RenderSprite_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,ZBUF_PSM,1>( Coord0, Coord1 );
			break;
			
		case 2:
			RenderSprite_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,ZBUF_PSM,2>( Coord0, Coord1 );
			break;
			
		case 0xa:
			RenderSprite_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,ZBUF_PSM,0xa>( Coord0, Coord1 );
			break;
			
		case 0x13:
			RenderSprite_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,ZBUF_PSM,0x13>( Coord0, Coord1 );
			break;
			
		case 0x14:
			RenderSprite_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,ZBUF_PSM,0x14>( Coord0, Coord1 );
			break;
			
		case 0x1b:
			RenderSprite_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,ZBUF_PSM,0x1b>( Coord0, Coord1 );
			break;
			
		case 0x24:
			RenderSprite_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,ZBUF_PSM,0x24>( Coord0, Coord1 );
			break;
			
		case 0x2c:
			RenderSprite_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,ZBUF_PSM,0x2c>( Coord0, Coord1 );
			break;
			
		case 0x30:
			RenderSprite_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,ZBUF_PSM,0x30>( Coord0, Coord1 );
			break;
			
		case 0x31:
			RenderSprite_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,ZBUF_PSM,0x31>( Coord0, Coord1 );
			break;
			
		case 0x32:
			RenderSprite_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,ZBUF_PSM,0x32>( Coord0, Coord1 );
			break;
			
		case 0x3a:
			RenderSprite_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,ZBUF_PSM,0x3a>( Coord0, Coord1 );
			break;
	}
}


template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_ZTST,const long FRAME_PSM>
inline void Select_RenderSprite5_t ( u32 Coord0, u32 Coord1 )
{
	switch ( ZBUF_X.PSM )
	{
		case 0:
			Select_RenderSprite6_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,0>( Coord0, Coord1 );
			break;
			
		case 1:
			Select_RenderSprite6_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,1>( Coord0, Coord1 );
			break;
			
		case 2:
			Select_RenderSprite6_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,2>( Coord0, Coord1 );
			break;
			
		case 3:
			Select_RenderSprite6_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,FRAME_PSM,3>( Coord0, Coord1 );
			break;
	}
}


template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_ZTST>
inline void Select_RenderSprite4_t ( u32 Coord0, u32 Coord1 )
{
	switch ( FrameBuffer_PixelFormat )
	{
		case 0:
			Select_RenderSprite5_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,0>( Coord0, Coord1 );
			break;
			
		case 1:
			Select_RenderSprite5_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,1>( Coord0, Coord1 );
			break;
			
		case 2:
			Select_RenderSprite5_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,2>( Coord0, Coord1 );
			break;
			
		case 0xa:
			Select_RenderSprite5_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,0xa>( Coord0, Coord1 );
			break;

			
		case 0x13:
		case 0x14:
		case 0x1b:
		case 0x24:
		case 0x2c:
			cout << "\nhps2x64: GPU: ALERT: Reserved FrameBuffer pixel value: " << hex << FrameBuffer_PixelFormat;
			break;

			
		case 0x30:
			Select_RenderSprite5_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,0x30>( Coord0, Coord1 );
			break;
			
		case 0x31:
			Select_RenderSprite5_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,0x31>( Coord0, Coord1 );
			break;
			
		case 0x32:
			Select_RenderSprite5_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,0x32>( Coord0, Coord1 );
			break;
			
		case 0x3a:
			Select_RenderSprite5_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,0x3a>( Coord0, Coord1 );
			break;
	}
}



template<const long TEST_ATST,const long TEST_AFAIL>
inline void Select_RenderSprite3_t ( u32 Coord0, u32 Coord1 )
{
	switch ( TEST_X.ZTE )
	{
		case 0:
			Select_RenderSprite4_t<TEST_ATST,TEST_AFAIL,1>( Coord0, Coord1 );
			break;
			
		case 1:
			switch ( TEST_X.ZTST )
			{
				case 0:
					Select_RenderSprite4_t<TEST_ATST,TEST_AFAIL,0>( Coord0, Coord1 );
					break;
					
				case 1:
					Select_RenderSprite4_t<TEST_ATST,TEST_AFAIL,1>( Coord0, Coord1 );
					break;
					
				case 2:
					Select_RenderSprite4_t<TEST_ATST,TEST_AFAIL,2>( Coord0, Coord1 );
					break;
					
				case 3:
					Select_RenderSprite4_t<TEST_ATST,TEST_AFAIL,3>( Coord0, Coord1 );
					break;
			}
			break;
	}
}



// TME,FST,FGE,ABE,COLCLAMP,FBA,DATE,ZMSK,ZTST(2),ATST(3)
//template<const long TEST_ATST>
//inline void Select_RenderSprite2_t ( u32 Coord0, u32 Coord1 )
template<const long TME,const long FGE,const long FST,const long ABE,const long COLCLAMP>
inline void Select_RenderSprite2_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	u32 ATST, ATE, ZTST, ZTE, ZMSK, DATE, FBA;
	u32 Combine;
	
	ATE = p_inputbuffer [ 5 ] & 1;
	ZTE = ( p_inputbuffer [ 5 ] >> 16 ) & 1;
	
	ATST = ( p_inputbuffer [ 5 ] >> 1 ) & 3;
	DATE = ( p_inputbuffer [ 5 ] >> 14 ) & 1;
	ZTST = ( p_inputbuffer [ 5 ] >> 17 ) & 3;
	ZMSK = ( p_inputbuffer [ 3 ] >> 32 ) & 1;
	
	if ( !ATE ) ATST = 0;
	if ( !ZTE ) ZTST = 0;
	
	Combine = ( ATST << 4 ) | ( DATE << 3 ) | ( ZTST << 1 ) | ZMSK;
	
	switch( Combine )
	{
		case 0:
			//Render_Generic_Sprite_t<TME,FGE,FST,ABE,COLCLAMP,ATST,DATE,ZTST,ZMSK>( p_inputbuffer, ulThreadNum );
			Render_Generic_Sprite_t<TME,FGE,FST,ABE,COLCLAMP,ATST,DATE,ZTST,ZMSK>( p_inputbuffer, ulThreadNum );
			break;
		case
	}
	
	
	/*
	switch ( TEST_X.AFAIL )
	{
		case 0:
			Select_RenderSprite3_t<TEST_ATST,0>( Coord0, Coord1 );
			break;
			
		case 1:
			Select_RenderSprite3_t<TEST_ATST,1>( Coord0, Coord1 );
			break;
			
		case 2:
			Select_RenderSprite3_t<TEST_ATST,2>( Coord0, Coord1 );
			break;
			
		case 3:
			Select_RenderSprite3_t<TEST_ATST,3>( Coord0, Coord1 );
			break;
	}
	*/
}


// TME,FST,FGE,ABE,COLCLAMP,FBA,DATE,ZMSK,ZTST(2),ATST(3)
// template needed: PIXELFORMAT,CLUT_PIXELFORMAT,FBUF_PIXELFORMAT,TEXA_AEM,ZBUF_PIXELFORMAT,TEST_ATE,TEST_ATST,TEST_ZTE,TEST_ZTST,TEST_AFAIL,ZBUF_ZMSK,COLCLAMP,TEX_TFX,TEX_TCC
//template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_ZTST,const long FRAME_PSM,const long ZBUF_PSM,const long TEX_PSM>
//void Select_RenderSprite_t ( u32 Coord0, u32 Coord1 )
void Select_RenderSprite_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	u32 TME, FST, FGE, ABE, COLCLAMP, FBA;	//DATE, ZMSK, ZTST, ATST;
	u32 Combine;
	
				
	// inputbuffer
	// 0: SCISSOR
	// 1: XYOFFFSET
	// 2: FRAME
	// 3: ZBUF
	// 4: FBA
	// 5: TEST
	// 6: ALPHA
	// 7: PABE
	// 8: CLAMP
	// 9: TEX0
	// 10: DTHE
	// 11: DIMX
	// 12: FOGCOL
	// 13: COLCLAMP
	// 14: ----------
	// -------------
	// 15: PRIM (COMMAND)
	// 16: RGBA
	// 17: XYZ
	// 18: UV
	// 19: FOG
	// 20: RGBA
	// 21: XYZ
	// 22: UV
	// 23: FOG
	// 24: RGBA
	// 25: XYZ
	// 26: UV
	// 27: FOG
				
	TME = ( p_inputbuffer [ 15 ] >> 4 ) & 1;
	FGE = ( p_inputbuffer [ 15 ] >> 5 ) & 1;
	FST = ( p_inputbuffer [ 15 ] >> 8 ) & 1;
	
	ABE = ( p_inputbuffer [ 15 ] >> 6 ) & 1;
	COLCLAMP = ( p_inputbuffer [ 13 ] ) & 1;
	//FBA = ( p_inputbuffer [ 4 ] ) & 1;
	
	Combine = FST | ( FGE << 1 ) | ( TME << 2 ) | ( ABE << 3 ) | ( COLCLAMP << 4 );	// | ( FBA << 5 );
	
	switch ( Combine )
	{
		case 0:
		case 1:
		case 2:
		case 3:
			//Select_RenderSprite2_t<TME,FGE,FST,ABE,COLCLAMP,FBA> ( p_inputbuffer, ulThreadNum )
			Select_RenderSprite2_t<0,0,0,0,0> ( p_inputbuffer, ulThreadNum );
			break;

		case 4:
			Select_RenderSprite2_t<1,0,0,0,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 5:
			Select_RenderSprite2_t<1,0,1,0,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 6:
			Select_RenderSprite2_t<1,1,0,0,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 7:
			Select_RenderSprite2_t<1,1,1,0,0> ( p_inputbuffer, ulThreadNum );
			break;
		
		case 8:
		case 9:
		case 10:
		case 11:
			Select_RenderSprite2_t<0,0,0,1,0> ( p_inputbuffer, ulThreadNum );
			break;
			
		case 12:
			Select_RenderSprite2_t<1,0,0,1,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 13:
			Select_RenderSprite2_t<1,0,1,1,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 14:
			Select_RenderSprite2_t<1,1,0,1,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 15:
			Select_RenderSprite2_t<1,1,1,1,0> ( p_inputbuffer, ulThreadNum );
			break;
			
			
			
			
		case 16:
		case 17:
		case 18:
		case 19:
			//Select_RenderSprite2_t<TME,FGE,FST,ABE,COLCLAMP,FBA> ( p_inputbuffer, ulThreadNum )
			Select_RenderSprite2_t<0,0,0,0,0> ( p_inputbuffer, ulThreadNum );
			break;

		case 20:
			Select_RenderSprite2_t<1,0,0,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 21:
			Select_RenderSprite2_t<1,0,1,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 22:
			Select_RenderSprite2_t<1,1,0,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 23:
			Select_RenderSprite2_t<1,1,1,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		
		case 24:
		case 25:
		case 26:
		case 27:
			Select_RenderSprite2_t<0,0,0,1,1> ( p_inputbuffer, ulThreadNum );
			break;
			
		case 28:
			Select_RenderSprite2_t<1,0,0,1,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 29:
			Select_RenderSprite2_t<1,0,1,1,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 30:
			Select_RenderSprite2_t<1,1,0,1,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 31:
			Select_RenderSprite2_t<1,1,1,1,1> ( p_inputbuffer, ulThreadNum );
			break;


		/*
		case 0 + 32:
		case 1 + 32:
		case 2 + 32:
		case 3 + 32:
			//Select_RenderSprite2_t<TME,FGE,FST,ABE,COLCLAMP,FBA> ( p_inputbuffer, ulThreadNum )
			Select_RenderSprite2_t<0,0,0,0,0,1> ( p_inputbuffer, ulThreadNum );
			break;

		case 4 + 32:
			Select_RenderSprite2_t<1,0,0,0,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 5 + 32:
			Select_RenderSprite2_t<1,0,1,0,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 6 + 32:
			Select_RenderSprite2_t<1,1,0,0,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 7 + 32:
			Select_RenderSprite2_t<1,1,1,0,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		
		case 8 + 32:
		case 9 + 32:
		case 10 + 32:
		case 11 + 32:
			Select_RenderSprite2_t<0,0,0,1,0,1> ( p_inputbuffer, ulThreadNum );
			break;
			
		case 12 + 32:
			Select_RenderSprite2_t<1,0,0,1,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 13 + 32:
			Select_RenderSprite2_t<1,0,1,1,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 14 + 32:
			Select_RenderSprite2_t<1,1,0,1,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 15 + 32:
			Select_RenderSprite2_t<1,1,1,1,0,1> ( p_inputbuffer, ulThreadNum );
			break;
			
			
			
			
		case 16 + 32:
		case 17 + 32:
		case 18 + 32:
		case 19 + 32:
			//Select_RenderSprite2_t<TME,FGE,FST,ABE,COLCLAMP,FBA> ( p_inputbuffer, ulThreadNum )
			Select_RenderSprite2_t<0,0,0,0,0,1> ( p_inputbuffer, ulThreadNum );
			break;

		case 20 + 32:
			Select_RenderSprite2_t<1,0,0,0,1,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 21 + 32:
			Select_RenderSprite2_t<1,0,1,0,1,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 22 + 32:
			Select_RenderSprite2_t<1,1,0,0,1,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 23 + 32:
			Select_RenderSprite2_t<1,1,1,0,1,1> ( p_inputbuffer, ulThreadNum );
			break;
		
		case 24 + 32:
		case 25 + 32:
		case 26 + 32:
		case 27 + 32:
			Select_RenderSprite2_t<0,0,0,1,1,1> ( p_inputbuffer, ulThreadNum );
			break;
			
		case 28 + 32:
			Select_RenderSprite2_t<1,0,0,1,1,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 29 + 32:
			Select_RenderSprite2_t<1,0,1,1,1,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 30 + 32:
			Select_RenderSprite2_t<1,1,0,1,1,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 31 + 32:
			Select_RenderSprite2_t<1,1,1,1,1,1> ( p_inputbuffer, ulThreadNum );
			break;
		*/

			
	}
	
	/*
	switch ( TEST_X.ATE )
	{
		case 0:
		
			Select_RenderSprite3_t<1,0>( Coord0, Coord1 );
			break;
			
		case 1:
			switch ( TEST_X.ATST )
			{
				case 0:
					Select_RenderSprite2_t<0>( Coord0, Coord1 );
					break;
					
				case 1:
					Select_RenderSprite3_t<1,0>( Coord0, Coord1 );
					break;
					
				case 2:
					Select_RenderSprite2_t<2>( Coord0, Coord1 );
					break;
					
				case 3:
					Select_RenderSprite2_t<3>( Coord0, Coord1 );
					break;
					
				case 4:
					Select_RenderSprite2_t<4>( Coord0, Coord1 );
					break;
					
				case 5:
					Select_RenderSprite2_t<5>( Coord0, Coord1 );
					break;
					
				case 6:
					Select_RenderSprite2_t<6>( Coord0, Coord1 );
					break;
					
				case 7:
					Select_RenderSprite2_t<7>( Coord0, Coord1 );
					break;
			}
			
			break;
	}
	*/
}











#endif



// does ( A - B ) * C + D
template<const long COLCLAMP>
inline static u32 GPU::AlphaABCD_32_t ( u32 A, u32 B, u32 C, u32 D )
{
	u32 uResult;
	s32 cA, cB, cC, cD, sC1, sC2, sC3;
	
	// get alpha value
	cC = ( C >> 24 ) & 0xff;
	
	// get component 1
	cA = ( A >> 0 ) & 0xff;
	cB = ( B >> 0 ) & 0xff;
	cD = ( D >> 0 ) & 0xff;
	
	// the blending does not0 clamp anything actually, and the result here actually does not get clamped until after dithering
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
static u32 TestSrcAlpha32_t ( u32 bgr, u32 SrcAlpha_ARef )
{
	// needs local variable: bgr_temp
	// returns: SrcAlphaTest_Pass
			
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
			
	
	// otherwise, return zero
	return 0;
}



template<const long TEST_ZTST,const long ZBUF_PSM>
static u32 PerformZDepthTest32_t ( u32 *zptr32, u32 ZValue32 )
{
	u32 z1;
	
	switch ( ZBUF_PSM )
	{
		case 0:
			z1 = *zptr32;
			break;
			
		case 1:
			z1 = ( *zptr32 ) & 0x00ffffff;
			break;
			
		case 2:
		case 0xa:
			z1 = *((u16*)zptr32);
			break;
	}

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
			if ( ZValue32 >= z1 ) return 1;
			break;
			
		// GREATER
		case 3:
			if ( ZValue32 > z1 ) return 1;
			break;
	} // switch ( TEST_X.ZTST )
	
	
	return 0;
}



template<const long DTHE,const long COLCLAMP,const long ABE,const long ATST,const long ZTST,const long DATE,const long ZMSK,const long FBPSM,const long ZBPSM>
inline static void PlotPixel_Gradient_t ( u32 *buf32, u32 *zbuf32, s32 x0, s32 y0, s64 z0, u32 bgr, u32 SetPixelMask, u32 FrameBufferWidthInPixels, u32 DA_Test, u32 AlphaXor32, u32 FrameBuffer_WriteMask32, u32 AREF, AlphaTest at, ZTest zt, AlphaFail af, u32 *AlphaSelect, u32 uA, u32 uB, u32 uC, u32 uD )
{
	u32 DestPixel;
	u32 bgr_temp;
	//u32 SrcAlphaTest_Pass;
	//u32 ZDepthTest_Pass;
	u32 salpha;

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
	
//cout << "\ndrawing pixel" << " x0=" << dec << x0 << " y0=" << y0 << " CvtAddrPix32=" << CvtAddrPix32 ( x0, y0, FrameBufferWidthInPixels ) ];

	switch ( FBPSM )
	{
		// PSMCT32
		case 0:
		
		// PSMCT24
		case 1:
			ptr32 = & ( buf32 [ CvtAddrPix32 ( x0, y0, FrameBufferWidthInPixels ) ] );
			break;
			
		// PSMZ32
		case 0x30:
		
		// PSMZ24
		case 0x31:
			ptr32 = & ( buf32 [ CvtAddrZBuf32 ( x0, y0, FrameBufferWidthInPixels ) ] );
			break;
			
		// PSMCT16
		case 2:
			//ptr16 = & ( buf16 [ x0 + ( y0 * FrameBufferWidth_Pixels ) ] );
			ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrPix16 ( x0, y0, FrameBufferWidthInPixels ) ] );
			break;
			
		// PSMCT16S
		case 0xa:
			ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrPix16S ( x0, y0, FrameBufferWidthInPixels ) ] );
			break;
			
		// PSMZ16
		case 0x32:
			ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrZBuf16 ( x0, y0, FrameBufferWidthInPixels ) ] );
			break;
			
		// PSMZ16S
		case 0x3a:
			ptr16 = & ( ( (u16*) buf32 ) [ CvtAddrZBuf16S ( x0, y0, FrameBufferWidthInPixels ) ] );
			break;
			
		default:
			cout << "\nhps2x64: GPU: ALERT: invalid frame-buffer format! FBBUF PSM=" << hex << FBPSM << "\n";
			break;
	}

	
	switch ( ZBPSM )
	{
		// PSMZ32
		case 0:
		
		// PSMZ24
		case 1:
			zptr32 = (u32*) ( & ( zbuf32 [ CvtAddrZBuf32 ( x0, y0, FrameBufferWidthInPixels ) ] ) );
			break;
			
		// PSMZ16
		case 2:
			
			// not valid for a 32-bit frame buffer
			zptr16 = & ( ( (u16*) zbuf32 ) [ CvtAddrZBuf16 ( x0, y0, FrameBufferWidthInPixels ) ] );
			break;
			
		// PSMZ16S
		case 0xa:
			zptr32 = (u32*) ( & ( ( (u16*) zbuf32 ) [ CvtAddrZBuf16S ( x0, y0, FrameBufferWidthInPixels ) ] ) );
			break;
			
		// OTHER
		default:
//#ifdef VERBOSE_ZBUFFER_H
			cout << "\nhps2x64: GPU: ALERT: invalid z-buffer format! ZBUF PSM=" << hex << ZBPSM << "\n";
//#endif
			break;
	}
	

//cout << "\ngot pointers";
//cout << "\nptr32=" << hex << ptr32 << " zptr32=" << zptr32 << " PtrEnd=" << PtrEnd << " buf32=" << buf32 << " zbuf32=" << zbuf32 << " FrameBufferWidthInPixels=" << dec << FrameBufferWidthInPixels;
	
	if ( ptr32 < PtrEnd )
	{
//cout << "\nwithin PtrEnd";
		
		// perform destination alpha test second //
		//DestPixel = ReadFrameBuffer_t<FBPSM> ( ptr );
		switch ( FBPSM )
		{
			// PSMCT32
			case 0:
			
			// PSMZ32
			case 0x30:
				DestPixel = *ptr32;
				break;
			
			// PSMCT24
			case 1:
				DestPixel = *ptr32;
				DestPixel &= 0x00ffffff;
				//DestPixel |= 0x80000000;
				break;
			
			// PSMZ24
			case 0x31:
				DestPixel = *ptr32;
				DestPixel &= 0x00ffffff;
				break;
				
			// PSMCT16
			case 2:
				
			// PSMCT16S
			case 0xa:
				bgr_temp = *ptr16;
				DestPixel = ( ( bgr_temp << 3 ) & 0xf8 ) | ( ( bgr_temp << 6 ) & 0xf800 ) | ( ( bgr_temp << 9 ) & 0xf80000 ) | ( ( bgr_temp << 16 ) & 0x80000000 );
				break;
				
			// PSMZ16
			case 0x32:
				
			// PSMZ16S
			case 0x3a:
				/*
				DestPixel = *ptr16;
				//DestPixel = ( ( bgr_temp << 3 ) & 0xf8 ) | ( ( bgr_temp << 6 ) & 0xf800 ) | ( ( bgr_temp << 9 ) & 0xf80000 ) | ( ( bgr_temp << 16 ) & 0x80000000 );
				*/
				
				bgr_temp = *ptr16;
				DestPixel = ( ( bgr_temp << 3 ) & 0xf8 ) | ( ( bgr_temp << 6 ) & 0xf800 ) | ( ( bgr_temp << 9 ) & 0xf80000 ) | ( ( bgr_temp << 16 ) & 0x80000000 );
				break;
				
			default:
				cout << "\nhps2x64: GPU: ALERT: invalid frame-buffer format! FBBUF PSM=" << hex << FBPSM << "\n";
				break;
		}
		
		
		
		if ( DATE )
		{
			// perform destination alpha test
			if ( ( ( DestPixel ^ DA_Test ) & 0x80000000ull ) )
			{
				// destination alpha test failed //
				return;
			}
		}
		
		
		// destination alpha test passed //
//cout << "\ndate passed";
		
		
		// perform z-test //
		if ( ZTST != 1 )
		{
			// perform z-test //
			//if ( !PerformZTest_fn ( zptr32, z0 ) )
			if ( !zt ( zptr32, z0 ) )
			{
				// z-test failed //
				return;
			}
		}
		
		
		// z-test passed //
//cout << "\nztst passed";
		
		
		// get the source alpha value for source alpha test
		salpha = bgr;
		
		
		// perform alpha //
		if ( ABE )
		{
			if ( !( ( bgr ^ AlphaXor32 ) & AlphaXor32 ) )
			{
				// save pixel
				bgr_temp = bgr;
				
				AlphaSelect [ 0 ] = bgr;
				
				if ( FBPSM == 1 )
				{
					// 24-bit pixel //
				AlphaSelect [ 1 ] = ( DestPixel & 0x00ffffff ) | 0x80000000;
				}
				else
				{
				AlphaSelect [ 1 ] = DestPixel;
				}
				
				// need to keep the source alpha
				// *todo* use COLCLAMP here
				bgr = AlphaABCD_32_t<COLCLAMP> ( AlphaSelect [ uA ], AlphaSelect [ uB ], AlphaSelect [ uC ], AlphaSelect [ uD ] );
				
				// re-add source alpha
				bgr |= ( bgr_temp & 0xff000000 );
			}
		}
		

		// source alpha test passed //
//cout << "\nabe passed";
		
		// set pixel mask //
		bgr |= SetPixelMask;
		
		// frame buffer write mask //
		bgr = ( bgr & FrameBuffer_WriteMask32 ) | ( DestPixel & ~FrameBuffer_WriteMask32 );

		// perform source alpha test first //
		if ( ATST != 1 )
		{
			// perform source alpha test
			//if ( !PerformSrcAlphaTest_fn ( bgr, AREF ) )
			if ( !at ( salpha, AREF ) )
			{
				// source alpha test failed //
				//AlphaTestFail_fn ( ptr32, zptr32, salpha, bgr, z0 );
				af ( ptr32, zptr32, bgr, DestPixel, z0 );
				return;
			}
		}
		
		// pixel writer //
		//WriteFrameBuffer_t<FBPSM> ( ptr32, bgr );
//cout << "\natst passed";
//cout << "\nbgr=" << hex << bgr;

		switch ( FBPSM )
		{
			// PSMCT32
			case 0:
			
			// PSMZ32
			case 0x30:
				*ptr32 = bgr;
				break;
			
			// PSMCT24
			case 1:
			
			// PSMZ24
			case 0x31:
				DestPixel = *ptr32;
				*ptr32 = ( DestPixel & 0xff000000 ) | ( bgr & 0x00ffffff );
				break;
				
			// PSMCT16
			case 2:
				
			// PSMCT16S
			case 0xa:
				*ptr16 = ( ( bgr >> 3 ) & 0x001f ) | ( ( bgr >> 6 ) & 0x03e0 ) | ( ( bgr >> 9 ) & 0x7c00 ) | ( ( bgr >> 16 ) & 0x8000 );
				break;
				
			// PSMZ16
			case 0x32:
				
			// PSMZ16S
			case 0x3a:
				/*
				//bgr_temp = *ptr16;
				*ptr16 = bgr;
				*/
				
				*ptr16 = ( ( bgr >> 3 ) & 0x001f ) | ( ( bgr >> 6 ) & 0x03e0 ) | ( ( bgr >> 9 ) & 0x7c00 ) | ( ( bgr >> 16 ) & 0x8000 );
				break;
				
			default:
//#ifdef VERBOSE_ZBUFFER_H
					cout << "\nhps2x64: GPU: ALERT: invalid frame-buffer format! FBBUF PSM=" << hex << FBPSM << "\n";
//#endif
				break;
		}
		
		// z-buffer writer //
		if ( !ZMSK )
		{
			//WriteZBuffer_t<ZBPSM> ( zptr32, z0 );
			
			switch ( ZBPSM )
			{
				// PSMZ32
				case 0:
					*zptr32 = z0;
					break;
				
				// PSMZ24
				case 1:
					DestPixel = *zptr32;
					*zptr32 = ( DestPixel & 0xff000000 ) | ( z0 & 0x00ffffff );
					break;
					
				// PSMZ16
				// not valid for a 32-bit frame buffer
				case 2:
					
				// PSMZ16S
				case 0xa:
					*zptr16 = z0;
					break;
					
				// OTHER
				default:
//#ifdef VERBOSE_ZBUFFER_H
					cout << "\nhps2x64: GPU: ALERT: invalid z-buffer format! ZBUF PSM=" << hex << ZBPSM << "\n";
//#endif
					break;
			}
		}
		
		
		
	} // end if ( ptr32 < PtrEnd )

}


template<const long TEST_AFAIL,const long FBUF_PSM,const long ZBUF_PSM,const long TEST_ZMSK>
static void PerformAlphaFail32_t ( u32* ptr32, u32* zptr32, u32 bgr, u32 DstPixel, u32 z0 )
{
	// source alpha test failed //
	
	switch ( TEST_AFAIL )
	{
		case 0:
			break;
		
		// update frame buffer ONLY
		case 1:
			// set MSB of pixel if specified before drawing
			//bgr |= PixelOr32;
			//bgr = ( bgr & FrameBuffer_WriteMask32 ) | ( DstPixel & ~FrameBuffer_WriteMask32 );
			switch ( FBUF_PSM )
			{
				// 32-bit frame buffer
				case 0x0:
				case 0x30:
					*ptr32 = bgr;
					break;
					
				// 24-bit frame buffer
				case 0x1:
				case 0x31:
					DstPixel = *ptr32;
					*ptr32 = ( DstPixel & 0xff000000 ) | ( bgr & 0x00ffffff );
					break;
					
				// 16-bit frame buffer
				case 0x2:
				case 0xa:
				case 0x32:
				case 0x3a:
					
					// convert pixel to 16-bit first then store
					*((u16*)ptr32) = ( ( bgr >> 3 ) & 0x001f ) | ( ( bgr >> 6 ) & 0x03e0 ) | ( ( bgr >> 9 ) & 0x7c00 ) | ( ( bgr >> 16 ) & 0x8000 );
					break;
			}
			break;
			
		// *todo* update z-buffer ONLY
		case 2:
			// store 32-bit depth value //
			
			// only store depth value if ZMSK is zero, which means to update z-buffer
			if ( !TEST_ZMSK )
			{
			
				//WriteZBufValue32_t<ZBUF_PSM> ( zptr32, z0 );
				switch ( ZBUF_PSM )
				{
					// PSMZ32
					case 0:
						*zptr32 = z0;
						break;
					
					// PSMZ24
					case 1:
						DstPixel = *zptr32;
						*zptr32 = ( DstPixel & 0xff000000 ) | ( z0 & 0x00ffffff );
						break;
						
					// PSMZ16
					// not valid for a 32-bit frame buffer
					case 2:
						
					// PSMZ16S
					case 0xa:
						*((u16*)zptr32) = z0;
						break;
						
					// OTHER
					default:
#ifdef VERBOSE_ZBUFFER_H
						cout << "\nhps2x64: GPU: ALERT: invalid z-buffer format on a 32-bit frame buffer! ZBUF PSM=" << hex << ZBUF_X.PSM << "\n";
#endif
						break;
				}
			
			} // end if ( !ZBUF_X.ZMSK )
			
			break;
			
		// update RGB ONLY (only available for 32-bit pixels)
		case 3:
			//bgr = ( bgr & FrameBuffer_WriteMask32 ) | ( DstPixel & ~FrameBuffer_WriteMask32 );
			
			// pixel must be 32-bit RGBA, then store as if 24-bit pixel
			if ( !( FBUF_PSM & 2 ) )
			{
				DstPixel = *ptr32;
				*ptr32 = ( DstPixel & 0xff000000 ) | ( bgr & ~0xff000000 );
			}
			else
			{
				
				// convert pixel to 16-bit first then store
				//bgr = ( DstPixel & 0xff000000 ) | ( bgr & ~0xff000000 );
				*((u16*)ptr32) = ( ( bgr >> 3 ) & 0x001f ) | ( ( bgr >> 6 ) & 0x03e0 ) | ( ( bgr >> 9 ) & 0x7c00 ) | ( ( bgr >> 16 ) & 0x8000 );
				
#ifdef VERBOSE_ALPHAFAIL3
				// error ??
				cout << "\nhps2x64: ALERT: GPU: Alpha fail #3 requires RGBA format but FBPSM=" << hex << FBUF_PSM;
#endif
			}
			break;
	}
}



//-----------------------------------------------------------------------------------------------


#ifdef USE_TEMPLATES_PS2_POINT

template<const long DTHE,const long COLCLAMP,const long ABE,const long ATST,const long ZTST,const long DATE,const long ZMSK,const long FBPSM, const long ZBPSM>
static u64 Render_Generic_Point_t ( u64 *p_inputbuffer, u32 ulThreadNum )
{
#if defined INLINE_DEBUG_POINT || defined INLINE_DEBUG_PRIMITIVE
	debug << "; RenderPoint";
#endif

	// render in 8bits per pixel, then convert down when done
	
	static const int c_iVectorSize = 1;

	u32 NumberOfPixelsDrawn;
	
	// if writing 16-bit pixels, then this could change to a u16 pointer
	//u32 *ptr, *buf;
	
	u32 bgr;
	//u32 DestPixel, bgr_temp;
	
	// 12.4 fixed point
	s32 x0, y0;
	
	// must be 64-bit signed
	s64 z0;
	
	//u32 *buf32;
	//u16 *buf16;
	
	//u32 *ptr32;
	//u16 *ptr16;
	
	// array for alpha selection
	//u32 AlphaSelect [ 4 ];

	// alpha value for rgb24 pixels
	// RGB24_Alpha -> this will be or'ed with pixel when writing to add-in alpha value for RGB24 format when writing
	// Pixel24_Mask -> this will be and'ed with pixel after reading from source
	//u32 RGB24_Alpha, Pixel24_Mask;
	
	// determine if rgb24 value is transparent (a=0)
	// this will be or'ed with pixel to determine if pixels is transparent
	//u32 RGB24_TAlpha;
	
	// alpha blending selection
	//u32 uA, uB, uC, uD;
	
	// pabe
	//u32 AlphaXor32, AlphaXor16;
	
	// alpha correction value (FBA)
	//u32 PixelOr32, PixelOr16;
	
	// destination alpha test
	// DA_Enable -> the value of DATE, to be and'ed with pixel
	// DA_Test -> the value of DATM, to be xor'ed with pixel
	//u32 DA_Enable, DA_Test;
	
	// source alpha test
	//u64 LessThan_And, GreaterOrEqualTo_And, LessThan_Or;
	//u32 Alpha_Fail;
	//u32 SrcAlphaTest_Pass, SrcAlpha_ARef;

	// depth test
	//s64 DepthTest_Offset;

	// in a 24-bit frame buffer, destination alpha is always 0x80
	//u32 DestAlpha24, DestMask24;

	
	// z-buffer
	//u32 *zbuf32;
	//u16 *zbuf16;
	//u32 *zptr32;
	//u16 *zptr16;
	
	//u32 ZBuffer_Shift, ZBuffer_32bit;
	//s64 ZBufValue;
	
	
	u32 FrameBufferWidthInPixels;
	u32 SetPixelMask;
	u32 Window_YTop, Window_YBottom, Window_XLeft, Window_XRight;
	s32 Coord_OffsetX, Coord_OffsetY;
	u32 FrameBufferStartOffset32, ZBufferStartOffset32;
	
	u32 DA_Test, AlphaXor32, FrameBuffer_WriteMask32;
	u32 AlphaSelect [ 4 ];
	u32 uA, uB, uC, uD;
	
	AlphaTest at;
	ZTest zt;
	AlphaFail af;
	
	u32 TEST_ATST;
	u32 TEST_ZTST;
	u32 TEST_AFAIL;
	u32 aref;
	
//cout << "\n->Render_Generic_Line_t" << hex << " SHADED=" << SHADED << " ABE=" << ABE << " ATST=" << ATST << " COLCLAMP=" << COLCLAMP << " ZTST=" << ZTST << " DATE=" << DATE << " FBPSM=" << FBPSM << " ZBPSM=" << ZBPSM;
	
	if ( !ATST )
	{
	TEST_ATST = ( p_inputbuffer [ 5 ] >> 1 ) & 7;
	TEST_AFAIL = ( p_inputbuffer [ 5 ] >> 12 ) & 3;
	
	aref = ( p_inputbuffer [ 5 ] >> 4 ) & 0xff;
	
	
	switch ( TEST_ATST )
	{
		case 0:
			at = & Playstation2::GPU::TestSrcAlpha32_t<0>;
			break;
		case 1:
			at = & Playstation2::GPU::TestSrcAlpha32_t<1>;
			break;
		case 2:
			aref = ( aref << 24 );
			at = & Playstation2::GPU::TestSrcAlpha32_t<2>;
			break;
		case 3:
			aref = ( aref << 24 ) | 0xffffff;
			at = & Playstation2::GPU::TestSrcAlpha32_t<3>;
			break;
		case 4:
			//aref = ( (u32) TEST_X.AREF );
			at = & Playstation2::GPU::TestSrcAlpha32_t<4>;
			break;
		case 5:
			aref = ( aref << 24 );
			at = & Playstation2::GPU::TestSrcAlpha32_t<5>;
			break;
		case 6:
			aref = ( aref << 24 ) | 0xffffff;
			at = & Playstation2::GPU::TestSrcAlpha32_t<6>;
			break;
		case 7:
			at = & Playstation2::GPU::TestSrcAlpha32_t<7>;
			break;
	}
	
	switch ( TEST_AFAIL )
	{
		case 0:
			af = & Playstation2::GPU::PerformAlphaFail32_t<0,FBPSM,ZBPSM,ZMSK>;
			break;
		case 1:
			af = & Playstation2::GPU::PerformAlphaFail32_t<1,FBPSM,ZBPSM,ZMSK>;
			break;
		case 2:
			af = & Playstation2::GPU::PerformAlphaFail32_t<2,FBPSM,ZBPSM,ZMSK>;
			break;
		case 3:
			af = & Playstation2::GPU::PerformAlphaFail32_t<3,FBPSM,ZBPSM,ZMSK>;
			break;
	}
	
	}	// end if ( !ATST )
	

	if ( !ZTST )
	{
	TEST_ZTST = ( p_inputbuffer [ 5 ] >> 17 ) & 3;
	
	switch ( TEST_ZTST )
	{
		case 0:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<0,ZBPSM>;
			break;
		case 1:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<1,ZBPSM>;
			break;
		case 2:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<2,ZBPSM>;
			break;
		case 3:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<3,ZBPSM>;
			break;
	}
	
	}	// end if ( !ZTST )
	
	
	if ( DATE )
	{
	DA_Test = ( p_inputbuffer [ 5 ] << 16 ) & 0x80000000ull;
	}	// end if ( DATE )
	
	
	SetPixelMask = p_inputbuffer [ 4 ] << 31;
	
	
	FrameBuffer_WriteMask32 = 0xffffffffull & ~( p_inputbuffer [ 2 ] >> 32 );
	
	//FrameBufferStartOffset32 = Frame [ Ctx ].FBP << 11;
	FrameBufferStartOffset32 = ( p_inputbuffer [ 2 ] & 0x1ff ) << 11;
	FrameBufferWidthInPixels = ( ( p_inputbuffer [ 2 ] >> 16 ) & 0x3f ) << 6;

	ZBufferStartOffset32 = ( p_inputbuffer [ 3 ] & 0x1ff ) << 11;
	
	
	buf32 = & ( _GPU->RAM32 [ FrameBufferStartOffset32 ] );
	zbuf32 = & ( _GPU->RAM32 [ ZBufferStartOffset32 ] );
	
//cout << "\nbuf32=" << hex << buf32 << " zbuf32=" << zbuf32;
	
	
	if ( ABE )
	{
	AlphaXor32 = p_inputbuffer [ 8 ] << 31;

	uA = ( p_inputbuffer [ 7 ] >> 0 ) & 3;
	uB = ( p_inputbuffer [ 7 ] >> 2 ) & 3;
	uC = ( p_inputbuffer [ 7 ] >> 4 ) & 3;
	uD = ( p_inputbuffer [ 7 ] >> 6 ) & 3;
	

	// set fixed alpha values
	
	// current RGBA value
	//AlphaSelect [ 0 ] = rgbaq_Current.Value & 0xffffffffL;
	AlphaSelect [ 0 ] = p_inputbuffer [ 16 ] & 0xffffffffULL;
	
	// FIX value
	//AlphaSelect [ 2 ] = ALPHA_X.FIX << 24;
	AlphaSelect [ 2 ] = ( p_inputbuffer [ 7 ] >> 8 ) & 0xff000000ULL;
	
	// ZERO
	AlphaSelect [ 3 ] = 0;
	}	// end if ( ABE )
	
	
	
	Window_YTop = ( p_inputbuffer [ 0 ] >> 32 ) & 0x7ff;
	Window_YBottom = ( p_inputbuffer [ 0 ] >> 48 ) & 0x7ff;
	Window_XLeft = ( p_inputbuffer [ 0 ] ) & 0x7ff;
	Window_XRight = ( p_inputbuffer [ 0 ] >> 16 ) & 0x7ff;
	
	Coord_OffsetX = p_inputbuffer [ 1 ] & 0xffff;
	Coord_OffsetY = ( p_inputbuffer [ 1 ] >> 32 ) & 0xffff;
	
	// get x,y
	//x0 = xyz [ Coord0 ].X;
	//y0 = xyz [ Coord0 ].Y;
	//x1 = xyz [ Coord1 ].X;
	//y1 = xyz [ Coord1 ].Y;
	x0 = p_inputbuffer [ 16 + 1 ] & 0xffff;
	y0 = ( p_inputbuffer [ 16 + 1 ] >> 16 ) & 0xffff;

	//////////////////////////////////////////
	// get coordinates on screen
	// *note* this is different from PS1, where you would add the offsets..
	x0 -= Coord_OffsetX;
	y0 -= Coord_OffsetY;

	
	// get z
	z0 = ( p_inputbuffer [ 16 + 1 ] >> 32 );
	
	
	
	// get fill color
	//bgr = rgbaq_Current.Value & 0xffffffffL;
	
	//bgr = rgbaq [ Coord0 ].Value & 0xffffffffL;
	bgr = p_inputbuffer [ 16 + 0 ] & 0xffffffffULL;

	
#if defined INLINE_DEBUG_LINE || defined INLINE_DEBUG_PRIMITIVE
	debug << hex << " bgr=" << bgr;
	debug << dec << "; Coords: x0=" << ( x0 >> 4 ) << " y0=" << ( y0 >> 4 ) << " x1=" << ( x1 >> 4 ) << " y1=" << ( y1 >> 4 );
#endif

	
	
#if defined INLINE_DEBUG_LINE || defined INLINE_DEBUG_PRIMITIVE
	debug << dec << "; FinalCoords: x0=" << x0 << " y0=" << y0 << " x1=" << x1 << " y1=" << y1;
#endif




	// check if line is within draw area
//	if ( x1 < Window_XLeft || x0 > Window_XRight || y1 < Window_YTop || y0 > Window_YBottom ) return;
#if defined INLINE_DEBUG_LINE_TEST || defined INLINE_DEBUG_PRIMITIVE_TEST
//	else
//	{
		debug << dec << "\r\nDrawLine_Gradient" << " FinalCoords: x0=" << x0 << " y0=" << y0 << " x1=" << x1 << " y1=" << y1;
		debug << " FrameBufPixFmt=" << PixelFormat_Names [ FrameBuffer_PixelFormat ];
		debug << " Alpha=" << Alpha;
//	}
#endif

	
	

	//StartX = x0 >> 4;
	//StartY = y0 >> 4;
	x0 = ( x0 + 0x8 ) >> 4;
	y0 = ( y0 + 0x8 ) >> 4;
	


	// check for some important conditions
	if ( Window_XRight < Window_XLeft )
	{
		//cout << "\nhps2x64 ALERT: GPU: Window_XRight < Window_XLeft.\n";
		return 0;
	}
	
	if ( Window_YBottom < Window_YTop )
	{
		//cout << "\nhps2x64 ALERT: GPU: Window_YBottom < Window_YTop.\n";
		return 0;
	}

	// check if sprite is within draw area
	if ( x0 < ((s32)Window_XLeft) || x0 > ((s32)Window_XRight) || y0 < ((s32)Window_YTop) || y0 > ((s32)Window_YBottom) ) return 0;
	
	
	PlotPixel_Gradient_t<DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( buf32, zbuf32, x0, y0, z0, bgr, SetPixelMask, FrameBufferWidthInPixels, DA_Test, AlphaXor32, FrameBuffer_WriteMask32, aref, at, zt, af, AlphaSelect, uA, uB, uC, uD );
}


template<const long DTHE,const long COLCLAMP,const long ABE,const long ATST,const long ZTST,const long DATE,const long ZMSK>
inline static u64 Select_RenderPoint3_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	u32 FBPSM, ZBPSM;
	u32 Combine;
	
//cout << "\n->Select_RenderLine3_t";
	
	// 000000: PSMCT32, 000001: PSMCT24, 000010: PSMCT16, 001010: PSMCT16S
	// 110000: PSMZ32, 110001: PSMZ24, 110010: PSMZ16, 111010: PSMZ16S
	FBPSM = ( p_inputbuffer [ 2 ] >> 24 ) & 0x3f;
	
	// 0000: PSMZ32, 0001: PSMZ24, 0010: PSMZ16, 1010: PSMZ16S
	ZBPSM = ( p_inputbuffer [ 3 ] >> 24 ) & 0xf;
	
	Combine = ( FBPSM << 4 ) | ( ZBPSM );
	
	switch ( Combine )
	{
		// 16-bit combinations
		
		// PSMCT16
		case 0x022:
			//template<DTHE,COLCLAMP,ABE,ATST,DATE,ZTST,ZMSK,FBPSM,ZBPSM>
			return Render_Generic_Point_t<DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x02,0x02>( p_inputbuffer, ulThreadNum );
			break;

		case 0x020:
		case 0x021:
		case 0x02a:
			//template<SHADED,DTHE,COLCLAMP,ABE,ATST,DATE,ZTST,ZMSK,FBPSM,ZBPSM>
			return Render_Generic_Point_t<DTHE,COLCLAMP,ABE,ATST,1,DATE,1,0x02,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x322:
			return Render_Generic_Point_t<DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x32,0x02>( p_inputbuffer, ulThreadNum );
			break;
			
		// 32-bit combinations
		
		// PSMCT32
		case 0x000:
			return Render_Generic_Point_t<0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x00,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x001:
			return Render_Generic_Point_t<0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x00,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		// 16-bit zbuf
		case 0x002:
			return Render_Generic_Point_t<0,COLCLAMP,ABE,ATST,1,DATE,1,0x00,0x02>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x00a:
			return Render_Generic_Point_t<0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x00,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMCT24
		case 0x010:
			return Render_Generic_Point_t<0,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x01,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x011:
			return Render_Generic_Point_t<0,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x01,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		// 16-bit zbuf
		case 0x012:
			return Render_Generic_Point_t<0,COLCLAMP,ABE,ATST,1,DATE,1,0x01,0x02>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x01a:
			return Render_Generic_Point_t<0,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x01,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMCT16S
		case 0x0a0:
			return Render_Generic_Point_t<DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x0a,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x0a1:
			return Render_Generic_Point_t<DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x0a,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		// 16-bit zbuf
		case 0x0a2:
			return Render_Generic_Point_t<DTHE,COLCLAMP,ABE,ATST,1,DATE,1,0x0a,0x02>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x0aa:
			return Render_Generic_Point_t<DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x0a,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// drawing pixels to z-buffers ?? need to investigate this
		/*
		// PSMZ32
		case 0x300:
			return Render_Generic_Point_t<0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x30,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x301:
			return Render_Generic_Point_t<0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x30,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x30a:
			return Render_Generic_Point_t<0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x30,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMZ24
		case 0x310:
			return Render_Generic_Point_t<0,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x31,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x311:
			return Render_Generic_Point_t<0,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x31,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x31a:
			return Render_Generic_Point_t<0,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x31,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMZ16S
		case 0x3a0:
			return Render_Generic_Point_t<DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x3a,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x3a1:
			return Render_Generic_Point_t<DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x3a,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x3aa:
			return Render_Generic_Point_t<DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x3a,0x0a>( p_inputbuffer, ulThreadNum );
			break;
		*/
			
		default:
			cout << "\nhps2x64: GPU: Invalid buffer combination. FBUF=" << hex << FBPSM << " ZBUF=" << ZBPSM;
			return 0;
			break;
	}
}


template<const long DTHE,const long COLCLAMP,const long ABE>
inline static u64 Select_RenderPoint2_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	u32 ATST, ATE, ZTST, ZTE, ZMSK, DATE;
	u32 Combine;
	
//cout << "\n->Select_RenderLine2_t";
	
	ATE = p_inputbuffer [ 5 ] & 1;
	ZTE = ( p_inputbuffer [ 5 ] >> 16 ) & 1;
	
	ATST = ( p_inputbuffer [ 5 ] >> 1 ) & 7;
	DATE = ( p_inputbuffer [ 5 ] >> 14 ) & 1;
	ZTST = ( p_inputbuffer [ 5 ] >> 17 ) & 3;
	ZMSK = ( p_inputbuffer [ 3 ] >> 32 ) & 1;
	
	//if ( ATST > 1 ) ATST = 2;
	if ( !ATE ) ATST = 1;
	
	if ( ZTST != 1 ) ZTST = 0;
	if ( ATST != 1 ) ATST = 0;
	
	// if z-test is disabled, then ztest always passes and zbuf is never written to
	if ( !ZTE )
	{
		ZTST = 1;
		ZMSK = 1;
	}
	
	// ***TODO*** if Frame buffer format is PSMCT24, then disable destination alpha test (DATE=0)

	//Combine = ( ATST << 4 ) | ( AFAIL << 3 ) | ( ZTST << 2 ) | ( DATE << 1 ) | ZMSK;
	Combine = ( ATST << 3 ) | ( ZTST << 2 ) | ( DATE << 1 ) | ZMSK;
	
	
	switch ( Combine )
	{
		case 0:
			// <DTHE,COLCLAMP,ABE,ATST,AFAIL,ZTST,DATE,ZMSK>
			return Select_RenderPoint3_t<DTHE,COLCLAMP,ABE,0,0,0,0>( p_inputbuffer, ulThreadNum );
			break;
		case 1:
			return Select_RenderPoint3_t<DTHE,COLCLAMP,ABE,0,0,0,1>( p_inputbuffer, ulThreadNum );
			break;
		case 2:
			return Select_RenderPoint3_t<DTHE,COLCLAMP,ABE,0,0,1,0>( p_inputbuffer, ulThreadNum );
			break;
		case 3:
			return Select_RenderPoint3_t<DTHE,COLCLAMP,ABE,0,0,1,1>( p_inputbuffer, ulThreadNum );
			break;
		case 4:
			return Select_RenderPoint3_t<DTHE,COLCLAMP,ABE,0,1,0,0>( p_inputbuffer, ulThreadNum );
			break;
		case 5:
			return Select_RenderPoint3_t<DTHE,COLCLAMP,ABE,0,1,0,1>( p_inputbuffer, ulThreadNum );
			break;
		case 6:
			return Select_RenderPoint3_t<DTHE,COLCLAMP,ABE,0,1,1,0>( p_inputbuffer, ulThreadNum );
			break;
		case 7:
			return Select_RenderPoint3_t<DTHE,COLCLAMP,ABE,0,1,1,1>( p_inputbuffer, ulThreadNum );
			break;
		case 8:
			return Select_RenderPoint3_t<DTHE,COLCLAMP,ABE,1,0,0,0>( p_inputbuffer, ulThreadNum );
			break;
		case 9:
			return Select_RenderPoint3_t<DTHE,COLCLAMP,ABE,1,0,0,1>( p_inputbuffer, ulThreadNum );
			break;
		case 10:
			return Select_RenderPoint3_t<DTHE,COLCLAMP,ABE,1,0,1,0>( p_inputbuffer, ulThreadNum );
			break;
		case 11:
			return Select_RenderPoint3_t<DTHE,COLCLAMP,ABE,1,0,1,1>( p_inputbuffer, ulThreadNum );
			break;
		case 12:
			return Select_RenderPoint3_t<DTHE,COLCLAMP,ABE,1,1,0,0>( p_inputbuffer, ulThreadNum );
			break;
		case 13:
			return Select_RenderPoint3_t<DTHE,COLCLAMP,ABE,1,1,0,1>( p_inputbuffer, ulThreadNum );
			break;
		case 14:
			return Select_RenderPoint3_t<DTHE,COLCLAMP,ABE,1,1,1,0>( p_inputbuffer, ulThreadNum );
			break;
		case 15:
			return Select_RenderPoint3_t<DTHE,COLCLAMP,ABE,1,1,1,1>( p_inputbuffer, ulThreadNum );
			break;
			
	}
	
}

static u64 Select_RenderPoint_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	u32 *p_inputbuffer32;
	u32 DTHE, ABE, AA1, COLCLAMP, FBA, DATE, ZMSK, ZTST, ATST;
	u32 Combine;

	
	// now that the data is set, if this is multi-threaded, then we are done if we are not on the GPU thread
	if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
	{
		ulInputBuffer_WriteIndex++;
		return;
	}
	

	Combine = DTHE | ( COLCLAMP << 1 ) | ( ABE << 2 );	// | ( FBA << 4 );
	
	switch ( Combine )
	{
		case 0:
		case 1:
		case 2:
		case 3:
			//Select_RenderLine_t<SHADED,DTHE,COLCLAMP,ABE,FBA>( p_inputbuffer, ulThreadNum );
			return Select_RenderPoint2_t<0,0,0>( p_inputbuffer, ulThreadNum );
			break;
			
		case 4:
		case 5:
			return Select_RenderPoint2_t<0,0,1>( p_inputbuffer, ulThreadNum );
			break;
			
		case 6:
		case 7:
			return Select_RenderPoint2_t<0,1,1>( p_inputbuffer, ulThreadNum );
			break;
	}
}

#endif


//-----------------------------------------------------------------------------------------------

#ifdef USE_TEMPLATES_PS2_LINE

template<const long SHADED,const long DTHE,const long COLCLAMP,const long ABE,const long ATST,const long ZTST,const long DATE,const long ZMSK,const long FBPSM, const long ZBPSM>
static u64 Render_Generic_Line_t( u64* p_inputbuffer, u32 ulThreadNum )
{
	s32 StartX, EndX, StartY, EndY;
	//u32 PixelsPerLine;
	u32 NumberOfPixelsDrawn;
	
	// if writing 16-bit pixels, then this could change to a u16 pointer
	//u32 *ptr, *buf32;
	//u16 *buf16;
	
	u32 bgr;
	//u32 DestPixel, bgr, bgr_temp;
	//u32 bgr2;
	
	// 12.4 fixed point
	s32 x0, y0, x1, y1;
	u32 bgr0, bgr1;
	
	s64 z0, z1;
	
	s32 Line, x_across;
	
	
	s32 distance, x_distance, y_distance;
	
	s32 r0, g0, b0, r1, g1, b1;
	s32 a0, a1;
	
	
	//s64 dxdc;
	//s64 dydc;
	//s64 drdc, dgdc, dbdc;
	//s64 line_x, line_y;
	//s64 line_r, line_g, line_b;
	
	// ***TODO*** interpolate alpha
	//s64 dadc;
	//s64 line_a;

	// and for the z
	//s64 dzdc;
	//s64 line_z;
	
	
	//u32 DestPixel, PixelMask = 0;
	
	//s32 iX, iY;
	s32 ix, iy;
	s64 iz;
	s32 dx, dy, dr, dg, db, da;
	s64 dz;
	s32 incdec;
	s32 Temp;
	s32 line_length;
	u64 NumPixels;
	
	s32 iR, iG, iB, iA;
	
	s32 Window_YTop, Window_YBottom, Window_XLeft, Window_XRight;
	
	s32 Coord_OffsetX, Coord_OffsetY;
	
	s32 RightMostX, LeftMostX, TopMostY, BottomMostY;
	
	u32 *buf32, *zbuf32;
	u32 SetPixelMask, FrameBufferWidthInPixels;
	
	u32 FrameBufferStartOffset32, ZBufferStartOffset32;
	
	u32 DA_Test, AlphaXor32, FrameBuffer_WriteMask32;
	u32 AlphaSelect [ 4 ];
	u32 uA, uB, uC, uD;
	
	AlphaTest at;
	ZTest zt;
	AlphaFail af;
	
	u32 TEST_ATST;
	u32 TEST_ZTST;
	u32 TEST_AFAIL;
	u32 aref;
	
//cout << "\n->Render_Generic_Line_t" << hex << " SHADED=" << SHADED << " ABE=" << ABE << " ATST=" << ATST << " COLCLAMP=" << COLCLAMP << " ZTST=" << ZTST << " DATE=" << DATE << " FBPSM=" << FBPSM << " ZBPSM=" << ZBPSM;
	
	if ( !ATST )
	{
	TEST_ATST = ( p_inputbuffer [ 5 ] >> 1 ) & 7;
	TEST_AFAIL = ( p_inputbuffer [ 5 ] >> 12 ) & 3;
	
	aref = ( p_inputbuffer [ 5 ] >> 4 ) & 0xff;
	
	
	switch ( TEST_ATST )
	{
		case 0:
			at = & Playstation2::GPU::TestSrcAlpha32_t<0>;
			break;
		case 1:
			at = & Playstation2::GPU::TestSrcAlpha32_t<1>;
			break;
		case 2:
			aref = ( aref << 24 );
			at = & Playstation2::GPU::TestSrcAlpha32_t<2>;
			break;
		case 3:
			aref = ( aref << 24 ) | 0xffffff;
			at = & Playstation2::GPU::TestSrcAlpha32_t<3>;
			break;
		case 4:
			//aref = ( (u32) TEST_X.AREF );
			at = & Playstation2::GPU::TestSrcAlpha32_t<4>;
			break;
		case 5:
			aref = ( aref << 24 );
			at = & Playstation2::GPU::TestSrcAlpha32_t<5>;
			break;
		case 6:
			aref = ( aref << 24 ) | 0xffffff;
			at = & Playstation2::GPU::TestSrcAlpha32_t<6>;
			break;
		case 7:
			at = & Playstation2::GPU::TestSrcAlpha32_t<7>;
			break;
	}
	
	switch ( TEST_AFAIL )
	{
		case 0:
			af = & Playstation2::GPU::PerformAlphaFail32_t<0,FBPSM,ZBPSM,ZMSK>;
			break;
		case 1:
			af = & Playstation2::GPU::PerformAlphaFail32_t<1,FBPSM,ZBPSM,ZMSK>;
			break;
		case 2:
			af = & Playstation2::GPU::PerformAlphaFail32_t<2,FBPSM,ZBPSM,ZMSK>;
			break;
		case 3:
			af = & Playstation2::GPU::PerformAlphaFail32_t<3,FBPSM,ZBPSM,ZMSK>;
			break;
	}
	
	}	// end if ( !ATST )
	

	if ( !ZTST )
	{
	TEST_ZTST = ( p_inputbuffer [ 5 ] >> 17 ) & 3;
	
	switch ( TEST_ZTST )
	{
		case 0:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<0,ZBPSM>;
			break;
		case 1:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<1,ZBPSM>;
			break;
		case 2:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<2,ZBPSM>;
			break;
		case 3:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<3,ZBPSM>;
			break;
	}
	
	}	// end if ( !ZTST )
	
	
	if ( DATE )
	{
	DA_Test = ( p_inputbuffer [ 5 ] << 16 ) & 0x80000000ull;
	}	// end if ( DATE )
	
	
	SetPixelMask = p_inputbuffer [ 4 ] << 31;
	
	
	FrameBuffer_WriteMask32 = 0xffffffffull & ~( p_inputbuffer [ 2 ] >> 32 );
	
	//FrameBufferStartOffset32 = Frame [ Ctx ].FBP << 11;
	FrameBufferStartOffset32 = ( p_inputbuffer [ 2 ] & 0x1ff ) << 11;
	FrameBufferWidthInPixels = ( ( p_inputbuffer [ 2 ] >> 16 ) & 0x3f ) << 6;

	ZBufferStartOffset32 = ( p_inputbuffer [ 3 ] & 0x1ff ) << 11;
	
	
	buf32 = & ( _GPU->RAM32 [ FrameBufferStartOffset32 ] );
	zbuf32 = & ( _GPU->RAM32 [ ZBufferStartOffset32 ] );
	
//cout << "\nbuf32=" << hex << buf32 << " zbuf32=" << zbuf32;
	
	
	if ( ABE )
	{
	AlphaXor32 = p_inputbuffer [ 8 ] << 31;

	uA = ( p_inputbuffer [ 7 ] >> 0 ) & 3;
	uB = ( p_inputbuffer [ 7 ] >> 2 ) & 3;
	uC = ( p_inputbuffer [ 7 ] >> 4 ) & 3;
	uD = ( p_inputbuffer [ 7 ] >> 6 ) & 3;
	

	// set fixed alpha values
	
	// current RGBA value
	//AlphaSelect [ 0 ] = rgbaq_Current.Value & 0xffffffffL;
	AlphaSelect [ 0 ] = p_inputbuffer [ 16 ] & 0xffffffffULL;
	
	// FIX value
	//AlphaSelect [ 2 ] = ALPHA_X.FIX << 24;
	AlphaSelect [ 2 ] = ( p_inputbuffer [ 7 ] >> 8 ) & 0xff000000ULL;
	
	// ZERO
	AlphaSelect [ 3 ] = 0;
	}	// end if ( ABE )
	
	
	
	Window_YTop = ( p_inputbuffer [ 0 ] >> 32 ) & 0x7ff;
	Window_YBottom = ( p_inputbuffer [ 0 ] >> 48 ) & 0x7ff;
	Window_XLeft = ( p_inputbuffer [ 0 ] ) & 0x7ff;
	Window_XRight = ( p_inputbuffer [ 0 ] >> 16 ) & 0x7ff;
	
	Coord_OffsetX = p_inputbuffer [ 1 ] & 0xffff;
	Coord_OffsetY = ( p_inputbuffer [ 1 ] >> 32 ) & 0xffff;
	
	// get x,y
	//x0 = xyz [ Coord0 ].X;
	//y0 = xyz [ Coord0 ].Y;
	//x1 = xyz [ Coord1 ].X;
	//y1 = xyz [ Coord1 ].Y;
	x0 = p_inputbuffer [ 16 + 1 ] & 0xffff;
	y0 = ( p_inputbuffer [ 16 + 1 ] >> 16 ) & 0xffff;
	x1 = p_inputbuffer [ 16 + 5 ] & 0xffff;
	y1 = ( p_inputbuffer [ 16 + 5 ] >> 16 ) & 0xffff;

	//////////////////////////////////////////
	// get coordinates on screen
	// *note* this is different from PS1, where you would add the offsets..
	x0 -= Coord_OffsetX;
	y0 -= Coord_OffsetY;
	x1 -= Coord_OffsetX;
	y1 -= Coord_OffsetY;



	
	// get z
	//z0 = (u64) xyz [ Coord0 ].Z;
	//z1 = (u64) xyz [ Coord1 ].Z;
	z0 = ( p_inputbuffer [ 16 + 1 ] >> 32 );
	z1 = ( p_inputbuffer [ 16 + 5 ] >> 32 );
	
	iz = z0 << 16;
	
	
	LeftMostX = ( x1 > x0 ) ? ( x0 >> 4 ) : ( x1 >> 4 );
	RightMostX = ( x0 > x1 ) ? ( x0 >> 4 ) : ( x1 >> 4 );
	TopMostY = ( y1 > y0 ) ? ( y0 >> 4 ) : ( y1 >> 4 );
	BottomMostY = ( y0 > y1 ) ? ( y0 >> 4 ) : ( y1 >> 4 );
	
	// get fill color
	//bgr = rgbaq_Current.Value & 0xffffffffL;
	
	if ( SHADED )
	{
	//bgr0 = rgbaq [ Coord0 ].Value & 0xffffffffL;
	//bgr1 = rgbaq [ Coord1 ].Value & 0xffffffffL;
	bgr0 = p_inputbuffer [ 16 + 0 ] & 0xffffffffULL;
	bgr1 = p_inputbuffer [ 16 + 4 ] & 0xffffffffULL;


	// get color components
	r0 = bgr0 & 0xff;
	r1 = bgr1 & 0xff;
	g0 = ( bgr0 >> 8 ) & 0xff;
	g1 = ( bgr1 >> 8 ) & 0xff;
	b0 = ( bgr0 >> 16 ) & 0xff;
	b1 = ( bgr1 >> 16 ) & 0xff;

	// get alpha
	a0 = ( bgr0 >> 24 ) & 0xff;
	a1 = ( bgr1 >> 24 ) & 0xff;
	
	iR = ( r0 << 16 ) + 0x8000;
	iG = ( g0 << 16 ) + 0x8000;
	iB = ( b0 << 16 ) + 0x8000;
	iA = ( a0 << 16 ) + 0x8000;
	}
	else
	{
	//bgr = rgbaq [ Coord0 ].Value & 0xffffffffL;
	bgr = p_inputbuffer [ 16 + 4 ] & 0xffffffffULL;
	}
	
#if defined INLINE_DEBUG_LINE || defined INLINE_DEBUG_PRIMITIVE
	debug << hex << " bgr=" << bgr;
	debug << dec << "; Coords: x0=" << ( x0 >> 4 ) << " y0=" << ( y0 >> 4 ) << " x1=" << ( x1 >> 4 ) << " y1=" << ( y1 >> 4 );
#endif

	
	
#if defined INLINE_DEBUG_LINE || defined INLINE_DEBUG_PRIMITIVE
	debug << dec << "; FinalCoords: x0=" << x0 << " y0=" << y0 << " x1=" << x1 << " y1=" << y1;
#endif




	// check if line is within draw area
//	if ( x1 < Window_XLeft || x0 > Window_XRight || y1 < Window_YTop || y0 > Window_YBottom ) return;
#if defined INLINE_DEBUG_LINE_TEST || defined INLINE_DEBUG_PRIMITIVE_TEST
//	else
//	{
		debug << dec << "\r\nDrawLine_Gradient" << " FinalCoords: x0=" << x0 << " y0=" << y0 << " x1=" << x1 << " y1=" << y1;
		debug << " FrameBufPixFmt=" << PixelFormat_Names [ FrameBuffer_PixelFormat ];
		debug << " Alpha=" << Alpha;
//	}
#endif

	
	

	//StartX = ( x0 + 0xf ) >> 4;
	//EndX = ( x1 - 1 ) >> 4;
	//StartY = ( y0 + 0xf ) >> 4;
	//EndY = ( y1 - 1 ) >> 4;
	StartX = x0 >> 4;
	EndX = x1 >> 4;
	StartY = y0 >> 4;
	EndY = y1 >> 4;
	
	x_distance = _Abs( EndX - StartX );
	y_distance = _Abs( EndY - StartY );
	//if ( x_distance > y_distance ) distance = x_distance; else distance = y_distance;


		// check for some important conditions
		if ( Window_XRight < Window_XLeft )
		{
			//cout << "\nhps2x64 ALERT: GPU: Window_XRight < Window_XLeft.\n";
			return 0;
		}
		
		if ( Window_YBottom < Window_YTop )
		{
			//cout << "\nhps2x64 ALERT: GPU: Window_YBottom < Window_YTop.\n";
			return 0;
		}

		// check if sprite is within draw area
		if ( RightMostX < ((s32)Window_XLeft) || LeftMostX > ((s32)Window_XRight) || BottomMostY < ((s32)Window_YTop) || TopMostY > ((s32)Window_YBottom) ) return 0;
		
		// skip drawing if distance between vertices is greater than max allowed by GPU
		if ( ( _Abs( EndX - StartX ) > c_MaxPolygonWidth ) || ( _Abs( EndY - StartY ) > c_MaxPolygonHeight ) )
		{
			// skip drawing polygon
			return 0;
		}
		
		//x_distance = _Abs( x1 - x0 );
		//y_distance = _Abs( y1 - y0 );
		
//cout << "\ncheckpoint1";

		if ( x_distance > y_distance )
		{
			NumPixels = x_distance;
			
			if ( LeftMostX < ((s32)Window_XLeft) )
			{
				NumPixels -= ( Window_XLeft - LeftMostX );
			}
			
			if ( RightMostX > ((s32)Window_XRight) )
			{
				NumPixels -= ( RightMostX - Window_XRight );
			}
		}
		else
		{
			NumPixels = y_distance;
			
			if ( y0 < ((s32)Window_YTop) )
			{
				NumPixels -= ( Window_YTop - y0 );
			}
			
			if ( y1 > ((s32)Window_YBottom) )
			{
				NumPixels -= ( y1 - Window_YBottom );
			}
		}
		
		
		
		if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
		{
			ulInputBuffer_WriteIndex++;
			return NumPixels;
		}
	
	if ( x_distance > y_distance )
	{
		
		// get the largest length
		line_length = x_distance;
		
		//if ( denominator < 0 )
		//{
			// x1 is on the left and x0 is on the right //
			
			////////////////////////////////////
			// get slopes
			
		//ix = x0;
		iy = ( y0 << 12 ) + 0x8000;
		//iy = ( y0 << 12 ) + 0xffff;
		//x_right = x_left;
		
		
		//if ( y1 - y0 )
		if ( line_length )
		{
			/////////////////////////////////////////////
			// init x on the left and right
			
			//dx_left = ( ( x1 - x0 ) << 16 ) / ( ( y1 - y0 ) + 1 );
			//dx = ( ( x1 - x0 ) << 16 ) / line_length;
			dy = ( ( y1 - y0 ) << 12 ) / line_length;
			dz = ( ( (s64) ( z1 - z0 ) ) << 16 ) / ( (s64) line_length );
			
			if ( SHADED )
			{
			dr = ( ( r1 - r0 ) << 16 ) / line_length;
			dg = ( ( g1 - g0 ) << 16 ) / line_length;
			db = ( ( b1 - b0 ) << 16 ) / line_length;
			da = ( ( a1 - a0 ) << 16 ) / line_length;
			}
		}

//cout << "\ndy=" << hex << dy << " dz=" << dz << " dr=" << dr << " dg=" << dg << " db=" << db << " da=" << da;
		
		// check if line is going left or right
		if ( x1 > x0 )
		{
			// line is going to the right
			incdec = 1;
			
			// clip against edge of screen
			if ( StartX < ((s32)Window_XLeft) )
			{
				Temp = Window_XLeft - StartX;
				StartX = Window_XLeft;
				
				iy += dy * Temp;
				iz += dz * Temp;
				
				if ( SHADED )
				{
				iR += dr * Temp;
				iG += dg * Temp;
				iB += db * Temp;
				iA += da * Temp;
				}
			}
			
			if ( EndX > ((s32)Window_XRight) )
			{
				EndX = Window_XRight + 1;
			}
		}
		else
		{
			// line is going to the left from the right
			incdec = -1;
			
			// clip against edge of screen
			if ( StartX > ((s32)Window_XRight) )
			{
				Temp = StartX - Window_XRight;
				StartX = Window_XRight;
				
				iy += dy * Temp;
				iz += dz * Temp;
				
				if ( SHADED )
				{
				iR += dr * Temp;
				iG += dg * Temp;
				iB += db * Temp;
				iA += da * Temp;
				}
			}
			
			if ( EndX < ((s32)Window_XLeft) )
			{
				EndX = Window_XLeft - 1;
			}
		}
		
		
		if ( dy <= 0 )
		{
	
			if ( ( iy >> 16 ) < ((s32)Window_YTop) )
			{
				return NumPixels;
			}
			//else
			//{
			//	// line is veering onto screen
			//	
			//	// get y value it hits screen at
			//	ix = ( ( ( y0 << 16 ) + 0x8000 ) - ( ((s32)DrawArea_TopLeftY) << 16 ) ) / ( dy >> 8 );
			//	ix -= ( x0 << 8 ) + 0xff;
			//	
			//}
			
			if ( EndY < ((s32)Window_YTop) )
			{
				// line is going down, so End Y would
				EndY = Window_YTop - 1;
			}
		}
		
		if ( dy >= 0 )
		{
			if ( ( iy >> 16 ) > ((s32)Window_YBottom) )
			{
				// line is veering off screen
				return NumPixels;
			}
			
			if ( EndY > ((s32)Window_YBottom) )
			{
				// line is going down, so End Y would
				EndY = Window_YBottom + 1;
			}
		}
		
		
		////////////////
		// *** TODO *** at this point area of full triangle can be calculated and the rest of the drawing can be put on another thread *** //
		
		// include the first point?
		//EndX += incdec;
		
		// draw the line horizontally
		for ( ix = StartX; ix != EndX; ix += incdec )
		{
			Line = iy >> 16;
			
//cout << "\nLine=" << dec << Line << " Window_YTop=" << Window_YTop << " Window_YBottom=" << Window_YBottom;
			if ( Line >= ((s32)Window_YTop) && Line <= ((s32)Window_YBottom) )
			{
				if ( SHADED )
				{
					bgr = ( iR >> 16 ) | ( ( iG >> 16 ) << 8 ) | ( ( iB >> 16 ) << 16 ) | ( ( iA >> 16 ) << 24 );
					
//cout << "\niy=" << dec << Line << " ix=" << ix << " iz=" << ( iz >> 16 ) << " EndX=" << EndX << hex << " bgr=" << bgr << " ir=" << iR << " ig=" << iG << " ib=" << iB << " ia=" << iA;
					PlotPixel_Gradient_t<DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( buf32, zbuf32, ix, Line, iz >> 16, bgr, SetPixelMask, FrameBufferWidthInPixels, DA_Test, AlphaXor32, FrameBuffer_WriteMask32, aref, at, zt, af, AlphaSelect, uA, uB, uC, uD );
				}
				else
				{
					//PlotPixel_Mono_t<COLCLAMP,ABE,ATST,AFAIL,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( Line, iy, iz, bgr );
					PlotPixel_Gradient_t<0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( buf32, zbuf32, ix, Line, iz >> 16, bgr, SetPixelMask, FrameBufferWidthInPixels, DA_Test, AlphaXor32, FrameBuffer_WriteMask32, aref, at, zt, af, AlphaSelect, uA, uB, uC, uD );
				}
			}
			
			iy += dy;
			iz += dz;
			
			if ( SHADED )
			{
			iR += dr;
			iG += dg;
			iB += db;
			iA += da;
			}
		}
		
	}
	else
	{
		// line is vertical //

		// get the largest length
		line_length = y_distance;
		
		//if ( denominator < 0 )
		//{
			// x1 is on the left and x0 is on the right //
			
			////////////////////////////////////
			// get slopes
			
		ix = ( x0 << 12 ) + 0x8000;
		//ix = ( x0 << 12 ) + 0xffff;
		//iy = y0;
		//x_right = x_left;
		
		//if ( y1 - y0 )
		if ( line_length )
		{
			/////////////////////////////////////////////
			// init x on the left and right
			
			//dx_left = ( ( x1 - x0 ) << 16 ) / ( ( y1 - y0 ) + 1 );
			//dy = ( ( y1 - y0 ) << 16 ) / line_length;,
			dx = ( ( x1 - x0 ) << 12 ) / line_length;
			dz = ( ( (s64) ( z1 - z0 ) ) << 16 ) / ( (s64) line_length );
			
			if ( SHADED )
			{
			dr = ( ( r1 - r0 ) << 16 ) / line_length;
			dg = ( ( g1 - g0 ) << 16 ) / line_length;
			db = ( ( b1 - b0 ) << 16 ) / line_length;
			da = ( ( a1 - a0 ) << 16 ) / line_length;
			}
		}
		
//cout << "\ndy=" << hex << dy << " dz=" << dz << " dr=" << dr << " dg=" << dg << " db=" << db << " da=" << da;
		
		
		// check if line is going up or down
		if ( y1 > y0 )
		{
			// line is going to the down
			incdec = 1;
			
			// clip against edge of screen
			if ( StartY < ((s32)Window_YTop) )
			{
				Temp = Window_YTop - StartY;
				StartY = Window_YTop;
				
				ix += dx * Temp;
				iz += dz * Temp;
				
				if ( SHADED )
				{
				iR += dr * Temp;
				iG += dg * Temp;
				iB += db * Temp;
				iA += da * Temp;
				}
			}
			
			if ( EndY > ((s32)Window_YBottom) )
			{
				EndY = Window_YBottom + 1;
			}
		}
		else
		{
			// line is going to the left from the up
			incdec = -1;
			
			// clip against edge of screen
			if ( StartY > ((s32)Window_YBottom) )
			{
				Temp = StartY - Window_YBottom;
				StartY = Window_YBottom;
				
				ix += dx * Temp;
				iz += dz * Temp;
				
				if ( SHADED )
				{
				iR += dr * Temp;
				iG += dg * Temp;
				iB += db * Temp;
				iA += da * Temp;
				}
			}
			
			if ( EndY < ((s32)Window_YTop) )
			{
				EndY = Window_YTop - 1;
			}
		}
	
		if ( dx <= 0 )
		{
			if ( ( ix >> 16 ) < ((s32)Window_XLeft) )
			{
				// line is veering off screen
				return NumPixels;
			}
			
			if ( EndX < ((s32)Window_XLeft) )
			{
				EndX = Window_XLeft - 1;
			}
		}
		
		if ( dx >= 0 )
		{
			if ( ( ix >> 16 ) > ((s32)Window_XRight) )
			{
				// line is veering off screen
				return NumPixels;
			}
			
			if ( EndX > ((s32)Window_XRight) )
			{
				EndX = Window_XRight + 1;
			}
		}
		
		
		// offset to get to this compute unit's scanline
		//group_yoffset = group_id - ( StartY % num_global_groups );
		//if ( group_yoffset < 0 )
		//{
		//	group_yoffset += num_global_groups;
		//}

	//}	// end if ( !local_id )
	
	

	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

		// include the first point?
		//EndY += incdec;
	
		// draw the line vertically
		for ( iy = StartY; iy != EndY; iy += incdec )
		{
			Line = ix >> 16;

//cout << "\nLine=" << dec << Line << " Window_XLeft=" << Window_XLeft << " Window_XRight=" << Window_XRight;
			if ( Line >= ((s32)Window_XLeft) && Line <= ((s32)Window_XRight) )
			{
				if ( SHADED )
				{
					bgr = ( iR >> 16 ) | ( ( iG >> 16 ) << 8 ) | ( ( iB >> 16 ) << 16 ) | ( ( iA >> 16 ) << 24 );
					
//cout << "\nix=" << dec << Line << " iy=" << iy << " iz=" << ( iz >> 16 ) << " EndY=" << EndY << hex << " bgr=" << bgr << " EndY=" << EndY << " ir=" << iR << " ig=" << iG << " ib=" << iB << " ia=" << iA;
					PlotPixel_Gradient_t<DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( buf32, zbuf32, Line, iy, iz >> 16, bgr, SetPixelMask, FrameBufferWidthInPixels, DA_Test, AlphaXor32, FrameBuffer_WriteMask32, aref, at, zt, af, AlphaSelect, uA, uB, uC, uD );
				}
				else
				{
					//PlotPixel_Mono_t<COLCLAMP,ABE,ATST,AFAIL,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( Line, iy, iz, bgr );
					PlotPixel_Gradient_t<0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( buf32, zbuf32, Line, iy, iz >> 16, bgr, SetPixelMask, FrameBufferWidthInPixels, DA_Test, AlphaXor32, FrameBuffer_WriteMask32, aref, at, zt, af, AlphaSelect, uA, uB, uC, uD );
				}
			}
			
			ix += dx;
			iz += dz;
			
			if ( SHADED )
			{
			iR += dr;
			iG += dg;
			iB += db;
			iA += da;
			}
		}


	}
	
	return NumPixels;

}



template<const long SHADED,const long DTHE,const long COLCLAMP,const long ABE,const long ATST,const long ZTST,const long DATE,const long ZMSK>
inline static u64 Select_RenderLine3_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	u32 FBPSM, ZBPSM;
	u32 Combine;
	
//cout << "\n->Select_RenderLine3_t";
	
	// 000000: PSMCT32, 000001: PSMCT24, 000010: PSMCT16, 001010: PSMCT16S
	// 110000: PSMZ32, 110001: PSMZ24, 110010: PSMZ16, 111010: PSMZ16S
	FBPSM = ( p_inputbuffer [ 2 ] >> 24 ) & 0x3f;
	
	// 0000: PSMZ32, 0001: PSMZ24, 0010: PSMZ16, 1010: PSMZ16S
	ZBPSM = ( p_inputbuffer [ 3 ] >> 24 ) & 0xf;
	
	Combine = ( FBPSM << 4 ) | ( ZBPSM );
	
	switch ( Combine )
	{
		// 16-bit combinations
		
		// PSMCT16
		case 0x022:
			//template<SHADED,DTHE,COLCLAMP,ABE,ATST,DATE,ZTST,ZMSK,FBPSM,ZBPSM>
			return Render_Generic_Line_t<SHADED,DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x02,0x02>( p_inputbuffer, ulThreadNum );
			break;

		case 0x020:
		case 0x021:
		case 0x02a:
			//template<SHADED,DTHE,COLCLAMP,ABE,ATST,DATE,ZTST,ZMSK,FBPSM,ZBPSM>
			return Render_Generic_Line_t<SHADED,DTHE,COLCLAMP,ABE,ATST,1,DATE,1,0x02,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x322:
			return Render_Generic_Line_t<SHADED,DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x32,0x02>( p_inputbuffer, ulThreadNum );
			break;
			
		// 32-bit combinations
		
		// PSMCT32
		case 0x000:
			return Render_Generic_Line_t<SHADED,0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x00,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x001:
			return Render_Generic_Line_t<SHADED,0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x00,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		// 16-bit zbuf
		case 0x002:
			return Render_Generic_Line_t<SHADED,0,COLCLAMP,ABE,ATST,1,DATE,1,0x00,0x02>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x00a:
			return Render_Generic_Line_t<SHADED,0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x00,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMCT24
		case 0x010:
			return Render_Generic_Line_t<SHADED,0,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x01,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x011:
			return Render_Generic_Line_t<SHADED,0,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x01,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		// 16-bit zbuf
		case 0x012:
			return Render_Generic_Line_t<SHADED,0,COLCLAMP,ABE,ATST,1,DATE,1,0x01,0x02>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x01a:
			return Render_Generic_Line_t<SHADED,0,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x01,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMCT16S
		case 0x0a0:
			return Render_Generic_Line_t<SHADED,DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x0a,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x0a1:
			return Render_Generic_Line_t<SHADED,DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x0a,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		// 16-bit zbuf
		case 0x0a2:
			return Render_Generic_Line_t<SHADED,DTHE,COLCLAMP,ABE,ATST,1,DATE,1,0x0a,0x02>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x0aa:
			return Render_Generic_Line_t<SHADED,DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x0a,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// drawing pixels to z-buffers ?? need to investigate this
		/*
		// PSMZ32
		case 0x300:
			return Render_Generic_Line_t<SHADED,0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x30,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x301:
			return Render_Generic_Line_t<SHADED,0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x30,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x30a:
			return Render_Generic_Line_t<SHADED,0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x30,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMZ24
		case 0x310:
			return Render_Generic_Line_t<SHADED,0,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x31,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x311:
			return Render_Generic_Line_t<SHADED,0,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x31,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x31a:
			return Render_Generic_Line_t<SHADED,0,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x31,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMZ16S
		case 0x3a0:
			return Render_Generic_Line_t<SHADED,DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x3a,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x3a1:
			return Render_Generic_Line_t<SHADED,DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x3a,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x3aa:
			return Render_Generic_Line_t<SHADED,DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x3a,0x0a>( p_inputbuffer, ulThreadNum );
			break;
		*/
			
		default:
			cout << "\nhps2x64: GPU: Invalid buffer combination. FBUF=" << hex << FBPSM << " ZBUF=" << ZBPSM;
			return 0;
			break;
	}
}


template<const long SHADED,const long DTHE,const long COLCLAMP,const long ABE>
inline static u64 Select_RenderLine2_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	u32 ATST, ATE, ZTST, ZTE, ZMSK, DATE;
	u32 Combine;
	
//cout << "\n->Select_RenderLine2_t";
	
	ATE = p_inputbuffer [ 5 ] & 1;
	ZTE = ( p_inputbuffer [ 5 ] >> 16 ) & 1;
	
	ATST = ( p_inputbuffer [ 5 ] >> 1 ) & 7;
	DATE = ( p_inputbuffer [ 5 ] >> 14 ) & 1;
	ZTST = ( p_inputbuffer [ 5 ] >> 17 ) & 3;
	ZMSK = ( p_inputbuffer [ 3 ] >> 32 ) & 1;
	
	//if ( ATST > 1 ) ATST = 2;
	if ( !ATE ) ATST = 1;
	
	if ( ZTST != 1 ) ZTST = 0;
	if ( ATST != 1 ) ATST = 0;
	
	// if z-test is disabled, then ztest always passes and zbuf is never written to
	if ( !ZTE )
	{
		ZTST = 1;
		ZMSK = 1;
	}
	
	// ***TODO*** if Frame buffer format is PSMCT24, then disable destination alpha test (DATE=0)

	//Combine = ( ATST << 4 ) | ( AFAIL << 3 ) | ( ZTST << 2 ) | ( DATE << 1 ) | ZMSK;
	Combine = ( ATST << 3 ) | ( ZTST << 2 ) | ( DATE << 1 ) | ZMSK;
	
	
	switch ( Combine )
	{
		case 0:
			// <SHADED,DTHE,COLCLAMP,ABE,ATST,AFAIL,ZTST,DATE,ZMSK>
			return Select_RenderLine3_t<SHADED,DTHE,COLCLAMP,ABE,0,0,0,0>( p_inputbuffer, ulThreadNum );
			break;
		case 1:
			return Select_RenderLine3_t<SHADED,DTHE,COLCLAMP,ABE,0,0,0,1>( p_inputbuffer, ulThreadNum );
			break;
		case 2:
			return Select_RenderLine3_t<SHADED,DTHE,COLCLAMP,ABE,0,0,1,0>( p_inputbuffer, ulThreadNum );
			break;
		case 3:
			return Select_RenderLine3_t<SHADED,DTHE,COLCLAMP,ABE,0,0,1,1>( p_inputbuffer, ulThreadNum );
			break;
		case 4:
			return Select_RenderLine3_t<SHADED,DTHE,COLCLAMP,ABE,0,1,0,0>( p_inputbuffer, ulThreadNum );
			break;
		case 5:
			return Select_RenderLine3_t<SHADED,DTHE,COLCLAMP,ABE,0,1,0,1>( p_inputbuffer, ulThreadNum );
			break;
		case 6:
			return Select_RenderLine3_t<SHADED,DTHE,COLCLAMP,ABE,0,1,1,0>( p_inputbuffer, ulThreadNum );
			break;
		case 7:
			return Select_RenderLine3_t<SHADED,DTHE,COLCLAMP,ABE,0,1,1,1>( p_inputbuffer, ulThreadNum );
			break;
		case 8:
			return Select_RenderLine3_t<SHADED,DTHE,COLCLAMP,ABE,1,0,0,0>( p_inputbuffer, ulThreadNum );
			break;
		case 9:
			return Select_RenderLine3_t<SHADED,DTHE,COLCLAMP,ABE,1,0,0,1>( p_inputbuffer, ulThreadNum );
			break;
		case 10:
			return Select_RenderLine3_t<SHADED,DTHE,COLCLAMP,ABE,1,0,1,0>( p_inputbuffer, ulThreadNum );
			break;
		case 11:
			return Select_RenderLine3_t<SHADED,DTHE,COLCLAMP,ABE,1,0,1,1>( p_inputbuffer, ulThreadNum );
			break;
		case 12:
			return Select_RenderLine3_t<SHADED,DTHE,COLCLAMP,ABE,1,1,0,0>( p_inputbuffer, ulThreadNum );
			break;
		case 13:
			return Select_RenderLine3_t<SHADED,DTHE,COLCLAMP,ABE,1,1,0,1>( p_inputbuffer, ulThreadNum );
			break;
		case 14:
			return Select_RenderLine3_t<SHADED,DTHE,COLCLAMP,ABE,1,1,1,0>( p_inputbuffer, ulThreadNum );
			break;
		case 15:
			return Select_RenderLine3_t<SHADED,DTHE,COLCLAMP,ABE,1,1,1,1>( p_inputbuffer, ulThreadNum );
			break;
			
	}
	
}



// TME,FST,FGE,ABE,COLCLAMP,FBA,DATE,ZMSK,ZTST(2),ATST(3)
// template needed: PIXELFORMAT,CLUT_PIXELFORMAT,FBUF_PIXELFORMAT,TEXA_AEM,ZBUF_PIXELFORMAT,TEST_ATE,TEST_ATST,TEST_ZTE,TEST_ZTST,TEST_AFAIL,ZBUF_ZMSK,COLCLAMP,TEX_TFX,TEX_TCC
//template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_ZTST,const long FRAME_PSM,const long ZBUF_PSM,const long TEX_PSM>
//void Select_RenderSprite_t ( u32 Coord0, u32 Coord1 )
static u64 Select_RenderLine_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	u32 *p_inputbuffer32;
	u32 SHADED, DTHE, ABE, AA1, COLCLAMP, FBA, DATE, ZMSK, ZTST, ATST;
	u32 Combine;
	
	/*
	s32 StartX, EndX, StartY, EndY;
	u32 NumberOfPixelsDrawn;
	
	u32 bgr;
	
	// 12.4 fixed point
	s32 x0, y0, x1, y1;
	u32 bgr0, bgr1;
	
	s64 z0, z1;
	
	s32 Line, x_across;
	
	
	s32 distance, x_distance, y_distance;
	
	s32 r0, g0, b0, r1, g1, b1;
	s32 a0, a1;
	
	
	
	s32 ix, iy;
	s64 iz;
	s32 dx, dy, dr, dg, db, da;
	s64 dz;
	s32 incdec;
	s32 Temp;
	s32 line_length;
	u64 NumPixels;
	
	s32 iR, iG, iB, iA;
	
	s32 Window_YTop, Window_YBottom, Window_XLeft, Window_XRight;
	
	s32 Coord_OffsetX, Coord_OffsetY;
	
	s32 RightMostX, LeftMostX, TopMostY, BottomMostY;
	
	u32 *buf32, *zbuf32;
	u32 SetPixelMask, FrameBufferWidthInPixels;
	
	u32 FrameBufferStartOffset32, ZBufferStartOffset32;
	
	u32 DA_Test, AlphaXor32, FrameBuffer_WriteMask32;
	u32 AlphaSelect [ 4 ];
	u32 uA, uB, uC, uD;
	
	AlphaTest at;
	ZTest zt;
	AlphaFail af;
	
	u32 TEST_ATST;
	u32 TEST_ZTST;
	u32 TEST_AFAIL;
	u32 aref;
	*/
				
	// inputbuffer
	// 0: SCISSOR
	// 1: XYOFFSET
	// 2: FRAME
	// 3: ZBUF
	// 4: FBA
	// 5: TEST
	// 6: COLCLAMP
	// 7: ALPHA
	// 8: PABE
	// 9: DTHE
	// 10: DIMX
	// 11: FOGCOL
	// 12: CLAMP
	// 13: TEX0
	// 14: TEXCLUT
	// 31: TEXA
	// -------------
	// 15: PRIM (COMMAND)
	// 16: RGBAQ
	// 17: XYZ
	// 18: UV/ST
	// 19: FOG
	// 20: RGBAQ
	// 21: XYZ
	// 22: UV/ST
	// 23: FOG
	// 24: RGBAQ
	// 25: XYZ
	// 26: UV/ST
	// 27: FOG
	
//cout << "\n->Select_RenderLine_t";

	SHADED = ( p_inputbuffer [ 15 ] >> 3 ) & 1;
	//TME = ( p_inputbuffer [ 15 ] >> 4 ) & 1;
	//FGE = ( p_inputbuffer [ 15 ] >> 5 ) & 1;
	//FST = ( p_inputbuffer [ 15 ] >> 8 ) & 1;0
	
	ABE = ( p_inputbuffer [ 15 ] >> 6 ) & 1;
	AA1 = ( p_inputbuffer [ 15 ] >> 7 ) & 1;
	COLCLAMP = ( p_inputbuffer [ 6 ] ) & 1;
	//FBA = ( p_inputbuffer [ 4 ] ) & 1;
	DTHE = ( p_inputbuffer [ 10 ] ) & 1;
	
	// for now do alpha blending also if aa1 is enabled
	if ( AA1 )
	{
		ABE = 1;
	}

	
//cout << "\n->Render_Generic_Line_t" << hex << " SHADED=" << SHADED << " ABE=" << ABE << " ATST=" << ATST << " COLCLAMP=" << COLCLAMP << " ZTST=" << ZTST << " DATE=" << DATE << " FBPSM=" << FBPSM << " ZBPSM=" << ZBPSM;


	/*
	if ( !ATST )
	{
	TEST_ATST = ( p_inputbuffer [ 5 ] >> 1 ) & 7;
	TEST_AFAIL = ( p_inputbuffer [ 5 ] >> 12 ) & 3;
	
	aref = ( p_inputbuffer [ 5 ] >> 4 ) & 0xff;
	
	
	switch ( TEST_ATST )
	{
		case 0:
			at = & Playstation2::GPU::TestSrcAlpha32_t<0>;
			break;
		case 1:
			at = & Playstation2::GPU::TestSrcAlpha32_t<1>;
			break;
		case 2:
			aref = ( aref << 24 );
			at = & Playstation2::GPU::TestSrcAlpha32_t<2>;
			break;
		case 3:
			aref = ( aref << 24 ) | 0xffffff;
			at = & Playstation2::GPU::TestSrcAlpha32_t<3>;
			break;
		case 4:
			//aref = ( (u32) TEST_X.AREF );
			at = & Playstation2::GPU::TestSrcAlpha32_t<4>;
			break;
		case 5:
			aref = ( aref << 24 );
			at = & Playstation2::GPU::TestSrcAlpha32_t<5>;
			break;
		case 6:
			aref = ( aref << 24 ) | 0xffffff;
			at = & Playstation2::GPU::TestSrcAlpha32_t<6>;
			break;
		case 7:
			at = & Playstation2::GPU::TestSrcAlpha32_t<7>;
			break;
	}
	
	switch ( TEST_AFAIL )
	{
		case 0:
			af = & Playstation2::GPU::PerformAlphaFail32_t<0,FBPSM,ZBPSM,ZMSK>;
			break;
		case 1:
			af = & Playstation2::GPU::PerformAlphaFail32_t<1,FBPSM,ZBPSM,ZMSK>;
			break;
		case 2:
			af = & Playstation2::GPU::PerformAlphaFail32_t<2,FBPSM,ZBPSM,ZMSK>;
			break;
		case 3:
			af = & Playstation2::GPU::PerformAlphaFail32_t<3,FBPSM,ZBPSM,ZMSK>;
			break;
	}
	
	}	// end if ( !ATST )
	

	if ( !ZTST )
	{
	TEST_ZTST = ( p_inputbuffer [ 5 ] >> 17 ) & 3;
	
	switch ( TEST_ZTST )
	{
		case 0:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<0,ZBPSM>;
			break;
		case 1:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<1,ZBPSM>;
			break;
		case 2:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<2,ZBPSM>;
			break;
		case 3:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<3,ZBPSM>;
			break;
	}
	
	}	// end if ( !ZTST )
	
	
	if ( DATE )
	{
	DA_Test = ( p_inputbuffer [ 5 ] << 16 ) & 0x80000000ull;
	}	// end if ( DATE )
	
	
	SetPixelMask = p_inputbuffer [ 4 ] << 31;
	
	
	FrameBuffer_WriteMask32 = 0xffffffffull & ~( p_inputbuffer [ 2 ] >> 32 );
	
	//FrameBufferStartOffset32 = Frame [ Ctx ].FBP << 11;
	FrameBufferStartOffset32 = ( p_inputbuffer [ 2 ] & 0x1ff ) << 11;
	FrameBufferWidthInPixels = ( ( p_inputbuffer [ 2 ] >> 16 ) & 0x3f ) << 6;

	ZBufferStartOffset32 = ( p_inputbuffer [ 3 ] & 0x1ff ) << 11;
	
	
	buf32 = & ( _GPU->RAM32 [ FrameBufferStartOffset32 ] );
	zbuf32 = & ( _GPU->RAM32 [ ZBufferStartOffset32 ] );
	
//cout << "\nbuf32=" << hex << buf32 << " zbuf32=" << zbuf32;
	
	
	if ( ABE )
	{
	AlphaXor32 = p_inputbuffer [ 8 ] << 31;

	uA = ( p_inputbuffer [ 7 ] >> 0 ) & 3;
	uB = ( p_inputbuffer [ 7 ] >> 2 ) & 3;
	uC = ( p_inputbuffer [ 7 ] >> 4 ) & 3;
	uD = ( p_inputbuffer [ 7 ] >> 6 ) & 3;
	

	// set fixed alpha values
	
	// current RGBA value
	//AlphaSelect [ 0 ] = rgbaq_Current.Value & 0xffffffffL;
	AlphaSelect [ 0 ] = p_inputbuffer [ 16 ] & 0xffffffffULL;
	
	// FIX value
	//AlphaSelect [ 2 ] = ALPHA_X.FIX << 24;
	AlphaSelect [ 2 ] = ( p_inputbuffer [ 7 ] >> 8 ) & 0xff000000ULL;
	
	// ZERO
	AlphaSelect [ 3 ] = 0;
	}	// end if ( ABE )
	
	
	Window_YTop = ( p_inputbuffer [ 0 ] >> 32 ) & 0x7ff;
	Window_YBottom = ( p_inputbuffer [ 0 ] >> 48 ) & 0x7ff;
	Window_XLeft = ( p_inputbuffer [ 0 ] ) & 0x7ff;
	Window_XRight = ( p_inputbuffer [ 0 ] >> 16 ) & 0x7ff;
	
	Coord_OffsetX = p_inputbuffer [ 1 ] & 0xffff;
	Coord_OffsetY = ( p_inputbuffer [ 1 ] >> 32 ) & 0xffff;
	
	// get x,y
	//x0 = xyz [ Coord0 ].X;
	//y0 = xyz [ Coord0 ].Y;
	//x1 = xyz [ Coord1 ].X;
	//y1 = xyz [ Coord1 ].Y;
	x0 = p_inputbuffer [ 16 + 1 ] & 0xffff;
	y0 = ( p_inputbuffer [ 16 + 1 ] >> 16 ) & 0xffff;
	x1 = p_inputbuffer [ 16 + 5 ] & 0xffff;
	y1 = ( p_inputbuffer [ 16 + 5 ] >> 16 ) & 0xffff;

	//////////////////////////////////////////
	// get coordinates on screen
	// *note* this is different from PS1, where you would add the offsets..
	x0 -= Coord_OffsetX;
	y0 -= Coord_OffsetY;
	x1 -= Coord_OffsetX;
	y1 -= Coord_OffsetY;



	
	// get z
	//z0 = (u64) xyz [ Coord0 ].Z;
	//z1 = (u64) xyz [ Coord1 ].Z;
	z0 = ( p_inputbuffer [ 16 + 1 ] >> 32 );
	z1 = ( p_inputbuffer [ 16 + 5 ] >> 32 );
	
	iz = z0 << 16;
	
	
	LeftMostX = ( x1 > x0 ) ? ( x0 >> 4 ) : ( x1 >> 4 );
	RightMostX = ( x0 > x1 ) ? ( x0 >> 4 ) : ( x1 >> 4 );
	TopMostY = ( y1 > y0 ) ? ( y0 >> 4 ) : ( y1 >> 4 );
	BottomMostY = ( y0 > y1 ) ? ( y0 >> 4 ) : ( y1 >> 4 );
	
	// get fill color
	//bgr = rgbaq_Current.Value & 0xffffffffL;
	
	if ( SHADED )
	{
	//bgr0 = rgbaq [ Coord0 ].Value & 0xffffffffL;
	//bgr1 = rgbaq [ Coord1 ].Value & 0xffffffffL;
	bgr0 = p_inputbuffer [ 16 + 0 ] & 0xffffffffULL;
	bgr1 = p_inputbuffer [ 16 + 4 ] & 0xffffffffULL;


	// get color components
	r0 = bgr0 & 0xff;
	r1 = bgr1 & 0xff;
	g0 = ( bgr0 >> 8 ) & 0xff;
	g1 = ( bgr1 >> 8 ) & 0xff;
	b0 = ( bgr0 >> 16 ) & 0xff;
	b1 = ( bgr1 >> 16 ) & 0xff;

	// get alpha
	a0 = ( bgr0 >> 24 ) & 0xff;
	a1 = ( bgr1 >> 24 ) & 0xff;
	
	iR = ( r0 << 16 ) + 0x8000;
	iG = ( g0 << 16 ) + 0x8000;
	iB = ( b0 << 16 ) + 0x8000;
	iA = ( a0 << 16 ) + 0x8000;
	}
	else
	{
	//bgr = rgbaq [ Coord0 ].Value & 0xffffffffL;
	bgr = p_inputbuffer [ 16 + 4 ] & 0xffffffffULL;
	}
	
#if defined INLINE_DEBUG_LINE || defined INLINE_DEBUG_PRIMITIVE
	debug << hex << " bgr=" << bgr;
	debug << dec << "; Coords: x0=" << ( x0 >> 4 ) << " y0=" << ( y0 >> 4 ) << " x1=" << ( x1 >> 4 ) << " y1=" << ( y1 >> 4 );
#endif

	
	
#if defined INLINE_DEBUG_LINE || defined INLINE_DEBUG_PRIMITIVE
	debug << dec << "; FinalCoords: x0=" << x0 << " y0=" << y0 << " x1=" << x1 << " y1=" << y1;
#endif




	// check if line is within draw area
//	if ( x1 < Window_XLeft || x0 > Window_XRight || y1 < Window_YTop || y0 > Window_YBottom ) return;
#if defined INLINE_DEBUG_LINE_TEST || defined INLINE_DEBUG_PRIMITIVE_TEST
//	else
//	{
		debug << dec << "\r\nDrawLine_Gradient" << " FinalCoords: x0=" << x0 << " y0=" << y0 << " x1=" << x1 << " y1=" << y1;
		debug << " FrameBufPixFmt=" << PixelFormat_Names [ FrameBuffer_PixelFormat ];
		debug << " Alpha=" << Alpha;
//	}
#endif

	
	

	//StartX = ( x0 + 0xf ) >> 4;
	//EndX = ( x1 - 1 ) >> 4;
	//StartY = ( y0 + 0xf ) >> 4;
	//EndY = ( y1 - 1 ) >> 4;
	StartX = x0 >> 4;
	EndX = x1 >> 4;
	StartY = y0 >> 4;
	EndY = y1 >> 4;
	
	x_distance = _Abs( EndX - StartX );
	y_distance = _Abs( EndY - StartY );
	//if ( x_distance > y_distance ) distance = x_distance; else distance = y_distance;


		// check for some important conditions
		if ( Window_XRight < Window_XLeft )
		{
			//cout << "\nhps2x64 ALERT: GPU: Window_XRight < Window_XLeft.\n";
			return 0;
		}
		
		if ( Window_YBottom < Window_YTop )
		{
			//cout << "\nhps2x64 ALERT: GPU: Window_YBottom < Window_YTop.\n";
			return 0;
		}

		// check if sprite is within draw area
		if ( RightMostX < ((s32)Window_XLeft) || LeftMostX > ((s32)Window_XRight) || BottomMostY < ((s32)Window_YTop) || TopMostY > ((s32)Window_YBottom) ) return 0;
		
		// skip drawing if distance between vertices is greater than max allowed by GPU
		if ( ( _Abs( EndX - StartX ) > c_MaxPolygonWidth ) || ( _Abs( EndY - StartY ) > c_MaxPolygonHeight ) )
		{
			// skip drawing polygon
			return 0;
		}
		
		//x_distance = _Abs( x1 - x0 );
		//y_distance = _Abs( y1 - y0 );
		
//cout << "\ncheckpoint1";

		if ( x_distance > y_distance )
		{
			NumPixels = x_distance;
			
			if ( LeftMostX < ((s32)Window_XLeft) )
			{
				NumPixels -= ( Window_XLeft - LeftMostX );
			}
			
			if ( RightMostX > ((s32)Window_XRight) )
			{
				NumPixels -= ( RightMostX - Window_XRight );
			}
		}
		else
		{
			NumPixels = y_distance;
			
			if ( y0 < ((s32)Window_YTop) )
			{
				NumPixels -= ( Window_YTop - y0 );
			}
			
			if ( y1 > ((s32)Window_YBottom) )
			{
				NumPixels -= ( y1 - Window_YBottom );
			}
		}
		
		
		
		if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
		{
			return NumPixels;
		}
	
	if ( x_distance > y_distance )
	{
		
		// get the largest length
		line_length = x_distance;
		
		//if ( denominator < 0 )
		//{
			// x1 is on the left and x0 is on the right //
			
			////////////////////////////////////
			// get slopes
			
		//ix = x0;
		iy = ( y0 << 12 ) + 0x8000;
		//iy = ( y0 << 12 ) + 0xffff;
		//x_right = x_left;
		
		
		//if ( y1 - y0 )
		if ( line_length )
		{
			/////////////////////////////////////////////
			// init x on the left and right
			
			//dx_left = ( ( x1 - x0 ) << 16 ) / ( ( y1 - y0 ) + 1 );
			//dx = ( ( x1 - x0 ) << 16 ) / line_length;
			dy = ( ( y1 - y0 ) << 12 ) / line_length;
			dz = ( ( (s64) ( z1 - z0 ) ) << 16 ) / ( (s64) line_length );
			
			if ( SHADED )
			{
			dr = ( ( r1 - r0 ) << 16 ) / line_length;
			dg = ( ( g1 - g0 ) << 16 ) / line_length;
			db = ( ( b1 - b0 ) << 16 ) / line_length;
			da = ( ( a1 - a0 ) << 16 ) / line_length;
			}
		}

//cout << "\ndy=" << hex << dy << " dz=" << dz << " dr=" << dr << " dg=" << dg << " db=" << db << " da=" << da;
		
		// check if line is going left or right
		if ( x1 > x0 )
		{
			// line is going to the right
			incdec = 1;
			
			// clip against edge of screen
			if ( StartX < ((s32)Window_XLeft) )
			{
				Temp = Window_XLeft - StartX;
				StartX = Window_XLeft;
				
				iy += dy * Temp;
				iz += dz * Temp;
				
				if ( SHADED )
				{
				iR += dr * Temp;
				iG += dg * Temp;
				iB += db * Temp;
				iA += da * Temp;
				}
			}
			
			if ( EndX > ((s32)Window_XRight) )
			{
				EndX = Window_XRight + 1;
			}
		}
		else
		{
			// line is going to the left from the right
			incdec = -1;
			
			// clip against edge of screen
			if ( StartX > ((s32)Window_XRight) )
			{
				Temp = StartX - Window_XRight;
				StartX = Window_XRight;
				
				iy += dy * Temp;
				iz += dz * Temp;
				
				if ( SHADED )
				{
				iR += dr * Temp;
				iG += dg * Temp;
				iB += db * Temp;
				iA += da * Temp;
				}
			}
			
			if ( EndX < ((s32)Window_XLeft) )
			{
				EndX = Window_XLeft - 1;
			}
		}
		
		
		if ( dy <= 0 )
		{
	
			if ( ( iy >> 16 ) < ((s32)Window_YTop) )
			{
				return NumPixels;
			}
			//else
			//{
			//	// line is veering onto screen
			//	
			//	// get y value it hits screen at
			//	ix = ( ( ( y0 << 16 ) + 0x8000 ) - ( ((s32)DrawArea_TopLeftY) << 16 ) ) / ( dy >> 8 );
			//	ix -= ( x0 << 8 ) + 0xff;
			//	
			//}
			
			if ( EndY < ((s32)Window_YTop) )
			{
				// line is going down, so End Y would
				EndY = Window_YTop - 1;
			}
		}
		
		if ( dy >= 0 )
		{
			if ( ( iy >> 16 ) > ((s32)Window_YBottom) )
			{
				// line is veering off screen
				return NumPixels;
			}
			
			if ( EndY > ((s32)Window_YBottom) )
			{
				// line is going down, so End Y would
				EndY = Window_YBottom + 1;
			}
		}
		
		
		////////////////
		// *** TODO *** at this point area of full triangle can be calculated and the rest of the drawing can be put on another thread *** //
		
		// include the first point?
		//EndX += incdec;
		
		// need ix, iy, StartX, EndX, incdec, Window_YTop, Window_YBottom, iR, iG, iB, iA, SetPixelMask, FrameBufferWidthInPixels, DA_Test, AlphaXor32, FrameBuffer_WriteMask32, aref, AlphaSelect, uA, uB, uC, uD, dy, dr, dg, db, da
		// need iz, dz, buf32, zbuf32, at, zt, af
		p_inputbuffer32 = (u32*) p_inputbuffer;
		
		p_inputbuffer32 [ 0 ] = ix;
		p_inputbuffer32 [ 1 ] = iy;
		p_inputbuffer32 [ 2 ] = StartX;
		p_inputbuffer32 [ 3 ] = EndX;
		p_inputbuffer32 [ 4 ] = incdec;
		p_inputbuffer32 [ 5 ] = Window_YTop;
		p_inputbuffer32 [ 6 ] = Window_YBottom;
		p_inputbuffer32 [ 7 ] = iR;
		p_inputbuffer32 [ 8 ] = iG;
		p_inputbuffer32 [ 9 ] = iB;
		p_inputbuffer32 [ 10 ] = iA;
		p_inputbuffer32 [ 11 ] = SetPixelMask;
		p_inputbuffer32 [ 12 ] = FrameBufferWidthInPixels;
		p_inputbuffer32 [ 13 ] = DA_Test;
		p_inputbuffer32 [ 14 ] = AlphaXor32;
		p_inputbuffer32 [ 15 ] = FrameBuffer_WriteMask32;
		p_inputbuffer32 [ 16 ] = AlphaSelect;
		p_inputbuffer32 [ 17 ] = uA;
		p_inputbuffer32 [ 18 ] = uB;
		p_inputbuffer32 [ 19 ] = uC;
		p_inputbuffer32 [ 20 ] = uD;
		p_inputbuffer32 [ 21 ] = dy;
		p_inputbuffer32 [ 22 ] = dr;
		p_inputbuffer32 [ 23 ] = dg;
		p_inputbuffer32 [ 24 ] = db;
		p_inputbuffer32 [ 25 ] = da;
		
		p_inputbuffer [ 14 ] = (u64) iz;
		p_inputbuffer [ 15 ] = (u64) dz;
		p_inputbuffer [ 16 ] = (u64) buf32;
		p_inputbuffer [ 17 ] = (u64) zbuf32;
		p_inputbuffer [ 18 ] = (u64) at;
		p_inputbuffer [ 19 ] = (u64) zt;
		p_inputbuffer [ 20 ] = (u64) af;
		
		
		// draw the line horizontally
		for ( ix = StartX; ix != EndX; ix += incdec )
		{
			Line = iy >> 16;
			
//cout << "\nLine=" << dec << Line << " Window_YTop=" << Window_YTop << " Window_YBottom=" << Window_YBottom;
			if ( Line >= ((s32)Window_YTop) && Line <= ((s32)Window_YBottom) )
			{
				if ( SHADED )
				{
					bgr = ( iR >> 16 ) | ( ( iG >> 16 ) << 8 ) | ( ( iB >> 16 ) << 16 ) | ( ( iA >> 16 ) << 24 );
					
//cout << "\niy=" << dec << Line << " ix=" << ix << " iz=" << ( iz >> 16 ) << " EndX=" << EndX << hex << " bgr=" << bgr << " ir=" << iR << " ig=" << iG << " ib=" << iB << " ia=" << iA;
					PlotPixel_Gradient_t<DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( buf32, zbuf32, ix, Line, iz >> 16, bgr, SetPixelMask, FrameBufferWidthInPixels, DA_Test, AlphaXor32, FrameBuffer_WriteMask32, aref, at, zt, af, AlphaSelect, uA, uB, uC, uD );
				}
				else
				{
					//PlotPixel_Mono_t<COLCLAMP,ABE,ATST,AFAIL,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( Line, iy, iz, bgr );
					PlotPixel_Gradient_t<0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( buf32, zbuf32, ix, Line, iz >> 16, bgr, SetPixelMask, FrameBufferWidthInPixels, DA_Test, AlphaXor32, FrameBuffer_WriteMask32, aref, at, zt, af, AlphaSelect, uA, uB, uC, uD );
				}
			}
			
			iy += dy;
			iz += dz;
			
			if ( SHADED )
			{
			iR += dr;
			iG += dg;
			iB += db;
			iA += da;
			}
		}
		
	}
	else
	{
		// line is vertical //

		// get the largest length
		line_length = y_distance;
		
		//if ( denominator < 0 )
		//{
			// x1 is on the left and x0 is on the right //
			
			////////////////////////////////////
			// get slopes
			
		ix = ( x0 << 12 ) + 0x8000;
		//ix = ( x0 << 12 ) + 0xffff;
		//iy = y0;
		//x_right = x_left;
		
		//if ( y1 - y0 )
		if ( line_length )
		{
			/////////////////////////////////////////////
			// init x on the left and right
			
			//dx_left = ( ( x1 - x0 ) << 16 ) / ( ( y1 - y0 ) + 1 );
			//dy = ( ( y1 - y0 ) << 16 ) / line_length;,
			dx = ( ( x1 - x0 ) << 12 ) / line_length;
			dz = ( ( (s64) ( z1 - z0 ) ) << 16 ) / ( (s64) line_length );
			
			if ( SHADED )
			{
			dr = ( ( r1 - r0 ) << 16 ) / line_length;
			dg = ( ( g1 - g0 ) << 16 ) / line_length;
			db = ( ( b1 - b0 ) << 16 ) / line_length;
			da = ( ( a1 - a0 ) << 16 ) / line_length;
			}
		}
		
//cout << "\ndy=" << hex << dy << " dz=" << dz << " dr=" << dr << " dg=" << dg << " db=" << db << " da=" << da;
		
		
		// check if line is going up or down
		if ( y1 > y0 )
		{
			// line is going to the down
			incdec = 1;
			
			// clip against edge of screen
			if ( StartY < ((s32)Window_YTop) )
			{
				Temp = Window_YTop - StartY;
				StartY = Window_YTop;
				
				ix += dx * Temp;
				iz += dz * Temp;
				
				if ( SHADED )
				{
				iR += dr * Temp;
				iG += dg * Temp;
				iB += db * Temp;
				iA += da * Temp;
				}
			}
			
			if ( EndY > ((s32)Window_YBottom) )
			{
				EndY = Window_YBottom + 1;
			}
		}
		else
		{
			// line is going to the left from the up
			incdec = -1;
			
			// clip against edge of screen
			if ( StartY > ((s32)Window_YBottom) )
			{
				Temp = StartY - Window_YBottom;
				StartY = Window_YBottom;
				
				ix += dx * Temp;
				iz += dz * Temp;
				
				if ( SHADED )
				{
				iR += dr * Temp;
				iG += dg * Temp;
				iB += db * Temp;
				iA += da * Temp;
				}
			}
			
			if ( EndY < ((s32)Window_YTop) )
			{
				EndY = Window_YTop - 1;
			}
		}
	
		if ( dx <= 0 )
		{
			if ( ( ix >> 16 ) < ((s32)Window_XLeft) )
			{
				// line is veering off screen
				return NumPixels;
			}
			
			if ( EndX < ((s32)Window_XLeft) )
			{
				EndX = Window_XLeft - 1;
			}
		}
		
		if ( dx >= 0 )
		{
			if ( ( ix >> 16 ) > ((s32)Window_XRight) )
			{
				// line is veering off screen
				return NumPixels;
			}
			
			if ( EndX > ((s32)Window_XRight) )
			{
				EndX = Window_XRight + 1;
			}
		}
		
		
		// offset to get to this compute unit's scanline
		//group_yoffset = group_id - ( StartY % num_global_groups );
		//if ( group_yoffset < 0 )
		//{
		//	group_yoffset += num_global_groups;
		//}

	//}	// end if ( !local_id )
	
	

	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

		// include the first point?
		//EndY += incdec;
	
		// draw the line vertically
		for ( iy = StartY; iy != EndY; iy += incdec )
		{
			Line = ix >> 16;

//cout << "\nLine=" << dec << Line << " Window_XLeft=" << Window_XLeft << " Window_XRight=" << Window_XRight;
			if ( Line >= ((s32)Window_XLeft) && Line <= ((s32)Window_XRight) )
			{
				if ( SHADED )
				{
					bgr = ( iR >> 16 ) | ( ( iG >> 16 ) << 8 ) | ( ( iB >> 16 ) << 16 ) | ( ( iA >> 16 ) << 24 );
					
//cout << "\nix=" << dec << Line << " iy=" << iy << " iz=" << ( iz >> 16 ) << " EndY=" << EndY << hex << " bgr=" << bgr << " EndY=" << EndY << " ir=" << iR << " ig=" << iG << " ib=" << iB << " ia=" << iA;
					PlotPixel_Gradient_t<DTHE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( buf32, zbuf32, Line, iy, iz >> 16, bgr, SetPixelMask, FrameBufferWidthInPixels, DA_Test, AlphaXor32, FrameBuffer_WriteMask32, aref, at, zt, af, AlphaSelect, uA, uB, uC, uD );
				}
				else
				{
					//PlotPixel_Mono_t<COLCLAMP,ABE,ATST,AFAIL,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( Line, iy, iz, bgr );
					PlotPixel_Gradient_t<0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( buf32, zbuf32, Line, iy, iz >> 16, bgr, SetPixelMask, FrameBufferWidthInPixels, DA_Test, AlphaXor32, FrameBuffer_WriteMask32, aref, at, zt, af, AlphaSelect, uA, uB, uC, uD );
				}
			}
			
			ix += dx;
			iz += dz;
			
			if ( SHADED )
			{
			iR += dr;
			iG += dg;
			iB += db;
			iA += da;
			}
		}


	}
	
	return NumPixels;
	*/




	
	Combine = DTHE | ( COLCLAMP << 1 ) | ( ABE << 2 ) | ( SHADED << 3 );	// | ( FBA << 4 );
	
	switch ( Combine )
	{
		case 0:
		case 1:
		case 2:
		case 3:
			//Select_RenderLine_t<SHADED,DTHE,COLCLAMP,ABE,FBA>( p_inputbuffer, ulThreadNum );
			return Select_RenderLine2_t<0,0,0,0>( p_inputbuffer, ulThreadNum );
			break;
			
		case 4:
		case 5:
			return Select_RenderLine2_t<0,0,0,1>( p_inputbuffer, ulThreadNum );
			break;
			
		case 6:
		case 7:
			return Select_RenderLine2_t<0,0,1,1>( p_inputbuffer, ulThreadNum );
			break;
			
		case 8:
		case 10:
			return Select_RenderLine2_t<1,0,0,0>( p_inputbuffer, ulThreadNum );
			break;
			
		case 9:
		case 11:
			return Select_RenderLine2_t<1,1,0,0>( p_inputbuffer, ulThreadNum );
			break;
			
		case 12:
			return Select_RenderLine2_t<1,0,0,1>( p_inputbuffer, ulThreadNum );
			break;
			
		case 13:
			return Select_RenderLine2_t<1,1,0,1>( p_inputbuffer, ulThreadNum );
			break;
			
		case 14:
			return Select_RenderLine2_t<1,0,1,1>( p_inputbuffer, ulThreadNum );
			break;
			
		case 15:
			return Select_RenderLine2_t<1,1,1,1>( p_inputbuffer, ulThreadNum );
			break;

	}
}



#endif


//---------------------------------------------------------------------------------------


template<const long COLCLAMP>
inline static u32 FogFunc32_t ( u32 bgr, u32 FogCoef, u32 FOGCOL )
{
	u32 cr, cg, cb, ca, rf, fogr, fogg, fogb;
	
	rf = 0xff - FogCoef;
	
	// get components from pixel
	ca = ( bgr >> 24 ) & 0xff;
	cr = ( bgr >> 0 ) & 0xff;
	cg = ( bgr >> 8 ) & 0xff;
	cb = ( bgr >> 16 ) & 0xff;
	
	// get components from fog color
	fogr = ( FOGCOL >> 0 ) & 0xff;
	fogg = ( FOGCOL >> 8 ) & 0xff;
	fogb = ( FOGCOL >> 16 ) & 0xff;
	
	cr = ColClamp8_t<COLCLAMP> ( ( ( FogCoef * cr ) >> 8 ) + ( ( rf * fogr ) >> 8 ) );
	cg = ColClamp8_t<COLCLAMP> ( ( ( FogCoef * cg ) >> 8 ) + ( ( rf * fogg ) >> 8 ) );
	cb = ColClamp8_t<COLCLAMP> ( ( ( FogCoef * cb ) >> 8 ) + ( ( rf * fogb ) >> 8 ) );
	
	// return the pixel
	return ( cr ) | ( cg << 8 ) | ( cb << 16 ) | ( ca << 24 );
}


template<const long COLCLAMP,const long TFX,const long TCC>
static u32 TextureFunc32_t ( u32 bgr, u32 shade1, u32 shade2, u32 shade3, u32 shade_a )
{
	if ( TFX == 1 )
	{
		if ( TCC )
		{
			return bgr;
		}
		else
		{
			return ( ( bgr & 0x00ffffff ) | ( shade_a << 24 ) );
		}
	}
	else
	{
	u32 c1, c2, c3, ca;
	
	// get components from pixel
	ca = ( bgr >> 24 ) & 0xff;
	c3 = ( bgr >> 16 ) & 0xff;
	c2 = ( bgr >> 8 ) & 0xff;
	c1 = ( bgr >> 0 ) & 0xff;
	
	// get pixel color
	switch ( TFX )
	{
		// MODULATE
		case 0:
			//c1 = UnsignedClamp<u32,8> ( ( c1 * shade1 ) >> 7 );
			//c2 = UnsignedClamp<u32,8> ( ( c2 * shade2 ) >> 7 );
			//c3 = UnsignedClamp<u32,8> ( ( c3 * shade3 ) >> 7 );
			c1 = ( ( c1 * shade1 ) >> 7 );
			c2 = ( ( c2 * shade2 ) >> 7 );
			c3 = ( ( c3 * shade3 ) >> 7 );
			
			c1 = ColClamp8_t<COLCLAMP> ( c1 );
			c2 = ColClamp8_t<COLCLAMP> ( c2 );
			c3 = ColClamp8_t<COLCLAMP> ( c3 );
		
			break;
			
			
		// HIGHLIGHT
		case 2:
		
		// HIGHLIGHT2
		case 3:
		
			//c1 = UnsignedClamp<u32,8> ( ( c1 * shade1 ) >> 7 );
			//c2 = UnsignedClamp<u32,8> ( ( c2 * shade2 ) >> 7 );
			//c3 = UnsignedClamp<u32,8> ( ( c3 * shade3 ) >> 7 );
			c1 = ( ( c1 * shade1 ) >> 7 );
			c2 = ( ( c2 * shade2 ) >> 7 );
			c3 = ( ( c3 * shade3 ) >> 7 );
			
			c1 += shade_a;
			c2 += shade_a;
			c3 += shade_a;
			
			c1 = ColClamp8_t<COLCLAMP> ( c1 );
			c2 = ColClamp8_t<COLCLAMP> ( c2 );
			c3 = ColClamp8_t<COLCLAMP> ( c3 );
			break;
	}
	
	
	// determine alpha
	switch ( TCC )
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
			switch ( TFX )
			{
				// MODULATE
				case 0:
					//ca = UnsignedClamp<u32,8> ( ( ca * shade_a ) >> 7 );
					ca = ColClamp8_t<COLCLAMP> ( ( ca * shade_a ) >> 7 );
					break;
				
				// HIGHLIGHT
				case 2:
					ca = ColClamp8_t<COLCLAMP> ( ca + shade_a );
					break;
			}
			
			break;
	}
	
	// return the pixel
	return ( c1 ) | ( c2 << 8 ) | ( c3 << 16 ) | ( ca << 24 );
	}
}


// requirement: TEXA64 must be pre-shifted to the left by 24
template<const long TEX_PSM,const long CLUT_PSM,const long TEXA_AEM>
static u32 Render_Texture_Pixel_t ( u32* ptr_texture32, u32 TexCoordX, u32 TexCoordY, u32 TexBufWidth, u16* ptr_clut16, u64 TEXA64 )
{
	u32 bgr, bgr_temp;
	
	switch ( TEX_PSM )
	{
		// PSMCT32
		case 0:
			//bgr = ptr_texture32 [ CvtAddrPix32 ( TexCoordX, TexCoordY, TexBufWidth ) ];
			return ptr_texture32 [ CvtAddrPix32 ( TexCoordX, TexCoordY, TexBufWidth ) ];
			break;
			
		// PSMCT24
		case 1:
			bgr = ptr_texture32 [ CvtAddrPix32 ( TexCoordX, TexCoordY, TexBufWidth ) ];
			break;
			
		// PSMCT16
		case 2:
			bgr = ((u16*)ptr_texture32) [ CvtAddrPix16 ( TexCoordX, TexCoordY, TexBufWidth ) ];
			break;
			
		// PSMCT16S
		case 0xa:
			bgr = ((u16*)ptr_texture32) [ CvtAddrPix16S ( TexCoordX, TexCoordY, TexBufWidth ) ];
			break;
			
		// PSMT8
		case 0x13:
			bgr_temp = ((u8*)ptr_texture32) [ CvtAddrPix8 ( TexCoordX, TexCoordY, TexBufWidth ) ];
			break;
			
		// PSMT4
		case 0x14:
			bgr = CvtAddrPix4 ( TexCoordX, TexCoordY, TexBufWidth );
			bgr_temp = ( ( ((u8*)ptr_texture32) [ bgr >> 1 ] ) >> ( ( bgr & 1 ) << 2 ) ) & 0xf;
			break;
			
		// PSMT8H
		case 0x1b:
			bgr_temp = ( ptr_texture32 [ CvtAddrPix32 ( TexCoordX, TexCoordY, TexBufWidth ) ] ) >> 24;
			break;
			
		// PSMT4HL
		case 0x24:
			bgr_temp = ( ptr_texture32 [ CvtAddrPix32 ( TexCoordX, TexCoordY, TexBufWidth ) ] >> 24 ) & 0xf;
			break;
			
		// PSMT4HH
		case 0x2c:
			bgr_temp = ptr_texture32 [ CvtAddrPix32 ( TexCoordX, TexCoordY, TexBufWidth ) ] >> 28;
			break;
			
		// Z-buffer formats
		
		// PSMZ32
		case 0x30:
		
		// PSMZ24
		case 0x31:
		//case 0x35:
			//bgr = ptr_texture32 [ CvtAddrZBuf32 ( TexCoordX, TexCoordY, TexBufWidth ) ];
			return ptr_texture32 [ CvtAddrZBuf32 ( TexCoordX, TexCoordY, TexBufWidth ) ];
			break;
			
		// PSMZ16
		case 0x32:
			bgr = ((u16*)ptr_texture32) [ CvtAddrZBuf16 ( TexCoordX, TexCoordY, TexBufWidth ) ];
			break;
			
		// PSMZ16S
		case 0x3a:
			bgr = ((u16*)ptr_texture32) [ CvtAddrZBuf16S ( TexCoordX, TexCoordY, TexBufWidth ) ];
			break;
	}

	
	// look up color value in CLUT if needed //
	//if ( ( PixelFormat & 7 ) >= 3 )
	if ( ( ( TEX_PSM & 7 ) >= 3 ) && ( TEX_PSM < 0x30 ) )
	{
		// lookup color value in CLUT //
		
//#if defined INLINE_DEBUG_SPRITE_PIXEL
//	debug << " IDX=" << hex << bgr_temp;
//#endif

		bgr = ptr_clut16 [ bgr_temp ];
		
		if ( ! ( CLUT_PSM & 0x2 ) )
		{
			// 32-bit pixels in CLUT //
			bgr |= ( (u32) ptr_clut16 [ bgr_temp + 256 ] ) << 16;
		}
	}
	
	// convert 16-bit pixels to 32-bit (since 32-bit frame buffer here) //
	
	// check if pixel is 16-bit
	//if ( ( ( PixelFormat == 0x2 ) || ( PixelFormat == 0xa ) ) || ( ( PixelFormat > 0xa ) && ( CLUTPixelFormat & 0x2 ) ) )
	if ( ( ( TEX_PSM == 0x2 ) || ( TEX_PSM == 0xa ) ) || ( ( TEX_PSM > 0xa ) && ( TEX_PSM < 0x30 ) && ( CLUT_PSM & 0x2 ) ) )
	{
		// pixel is 16-bit //
		
		// get alpha component of 16-bit pixel
		//bgr_temp = bgr & 0x8000;
		
		// check if pixel is definitely not transparent, if transparent then just stays zero
		if ( ( !TEXA_AEM ) || bgr )
		{
			// pixel is probably not transparent ?? //
			
			bgr_temp = bgr;
			
			// convert to 32-bit
			//bgr = ConvertPixel16To24 ( bgr );
			bgr = ( ( bgr & 0x7c00 ) << 9 ) | ( ( bgr & 0x3e0 ) << 6 ) | ( ( bgr & 0x1f ) << 3 );
			
			// set the texture alpha for 16 bit pixel
			//bgr |= ( ( ( bgr_temp & 0x8000 ) ? GPURegsGp.TEXA.TA1 : GPURegsGp.TEXA.TA0 ) << 24 );
			//bgr |= ( ( TEXA << 24 ) >> ( ( bgr_temp & 0x8000 ) >> 10 ) ) & 0xff000000;
			bgr |= ( TEXA64 >> ( ( bgr_temp & 0x8000 ) >> 10 ) ) & 0xff000000ull;
			
			// always draw pixel
			//bgr |= 0x100000000ull;
		}
		/*
		else
		{
			// we'll test out for below
			//return -1ULL;
			return 0;
		}
		*/
	}
	
	//if ( PixelFormat == 1 )
	if ( TEX_PSM == 1 )
	{
		// 24-bit pixel //
		bgr &= 0xffffff;
		
		if ( ( !TEXA_AEM ) || bgr )
		{
			// alpha is TA0
			//bgr |= ( TEXA << 24 );
			bgr |= TEXA64;
			
			// always draw pixel
			//bgr |= 0x100000000ull;
		}
		/*
		else
		{
			// we'll test out for below
			//return -1ULL;
			return 0;
		}
		*/
		
	}
	
	return bgr;
}

// returns texture function to use
static TexturePixel Select_TexturePixel_t ( u32 TEX_PSM, u32 CLUT_PSM, u32 TEXA_AEM )
{
	u32 Combine;
	
	
	switch ( TEX_PSM )
	{
		//PSMCT32
		case 0x00:
			//return &Render_Texture_Pixel_t<TEX_PSM,CLUT_PSM,TEXA_AEM>;
			return &Render_Texture_Pixel_t<0,0,0>;
			break;
			
		//PSMCT24
		case 0x01:
			switch( TEXA_AEM )
			{
			//PSMCT24 AEM=0
			case 0x0:
				return &Render_Texture_Pixel_t<1,0,0>;
				break;
				
			//PSMCT24 AEM=1
			case 0x1:
				return &Render_Texture_Pixel_t<1,0,1>;
				break;
			}
			
			break;
			
		//PSMCT16
		case 0x2:
			switch ( TEXA_AEM )
			{
			//PSMCT16 AEM=0
			case 0x0:
				return &Render_Texture_Pixel_t<2,0,0>;
				break;
		
			//PSMCT16 AEM=1
			case 0x1:
				return &Render_Texture_Pixel_t<2,0,1>;
				break;
			}
			
			break;
		
		//PSMCT16S
		case 0xa:
			switch ( TEXA_AEM )
			{
			//PSMCT16S AEM=0
			case 0x0:
				return &Render_Texture_Pixel_t<0xa,0,0>;
				break;
		
			//PSMCT16S AEM=1
			case 0x1:
				return &Render_Texture_Pixel_t<0xa,0,1>;
				break;
			}
			
			break;

		//PSMZ32
		case 0x30:
			//return &Render_Texture_Pixel_t<TEX_PSM,CLUT_PSM,TEXA_AEM>;
			return &Render_Texture_Pixel_t<0x30,0,0>;
			break;
			
		//PSMZ24 AEM=0
		case 0x31:
			return &Render_Texture_Pixel_t<0x31,0,0>;
			break;
			
		//PSMZ24 AEM=1
		//case 0x1031:
		//case 0x1231:
		//case 0x1a31:
		//	return &Render_Texture_Pixel_t<0x31,0,1>;
		//	break;
			
		//PSMZ16 AEM=0
		case 0x32:
			return &Render_Texture_Pixel_t<0x32,0,0>;
			break;
		
		//PSMZ16 AEM=1
		//case 0x1032:
		//case 0x1232:
		//case 0x1a32:
		//	return &Render_Texture_Pixel_t<0x32,0,1>;
		//	break;
		
		//PSMZ16S AEM=0
		case 0x3a:
			return &Render_Texture_Pixel_t<0x3a,0,0>;
			break;
		
		//PSMZ16S AEM=1
		//case 0x103a:
		//case 0x123a:
		//case 0x1a3a:
		//	return &Render_Texture_Pixel_t<0x3a,0,1>;
		//	break;

			
			
		default:
			Combine = ( TEXA_AEM << 12 ) | ( ( CLUT_PSM & 0xa ) << 8 ) | TEX_PSM;
			
			switch( Combine )
			{
			//PSMT8 CLUT=PSMCT32 AEM=0
			//PSMT8 CLUT=PSMCT32 AEM=1
			case 0x0013:
			case 0x1013:
				return &Render_Texture_Pixel_t<0x13,0,0>;
				break;
			//PSMT8 CLUT=PSMCT16 AEM=0
			case 0x0213:
				return &Render_Texture_Pixel_t<0x13,2,0>;
				break;
			//PSMT8 CLUT=PSMCT16S AEM=0
			case 0x0a13:
				return &Render_Texture_Pixel_t<0x13,0xa,0>;
				break;
				
			//PSMT8 CLUT=PSMCT16 AEM=1
			case 0x1213:
				return &Render_Texture_Pixel_t<0x13,2,1>;
				break;
			//PSMT8 CLUT=PSMCT16S AEM=1
			case 0x1a13:
				return &Render_Texture_Pixel_t<0x13,0xa,1>;
				break;

				
			//PSMT4 CLUT=PSMCT32 AEM=0
			//PSMT4 CLUT=PSMCT32 AEM=1
			case 0x0014:
			case 0x1014:
				return &Render_Texture_Pixel_t<0x14,0,0>;
				break;
			//PSMT4 CLUT=PSMCT16 AEM=0
			case 0x0214:
				return &Render_Texture_Pixel_t<0x14,2,0>;
				break;
			//PSMT4 CLUT=PSMCT16S AEM=0
			case 0x0a14:
				return &Render_Texture_Pixel_t<0x14,0xa,0>;
				break;
				
			//PSMT4 CLUT=PSMCT16 AEM=1
			case 0x1214:
				return &Render_Texture_Pixel_t<0x14,2,1>;
				break;
			//PSMT4 CLUT=PSMCT16S AEM=1
			case 0x1a14:
				return &Render_Texture_Pixel_t<0x14,0xa,1>;
				break;
				
			//PSMT8H CLUT=PSMCT32 AEM=0
			//PSMT8H CLUT=PSMCT32 AEM=1
			case 0x001b:
			case 0x101b:
				return &Render_Texture_Pixel_t<0x1b,0,0>;
				break;
			//PSMT8H CLUT=PSMCT16 AEM=0
			case 0x021b:
				return &Render_Texture_Pixel_t<0x1b,2,0>;
				break;
			//PSMT8H CLUT=PSMCT16S AEM=0
			case 0x0a1b:
				return &Render_Texture_Pixel_t<0x1b,0xa,0>;
				break;
				
			//PSMT8H CLUT=PSMCT16 AEM=1
			case 0x121b:
				return &Render_Texture_Pixel_t<0x1b,2,1>;
				break;
			//PSMT8H CLUT=PSMCT16S AEM=1
			case 0x1a1b:
				return &Render_Texture_Pixel_t<0x1b,0xa,1>;
				break;
				

			//PSMT4HL CLUT=PSMCT32 AEM=0
			//PSMT4HL CLUT=PSMCT32 AEM=1
			case 0x0024:
			case 0x1024:
				return &Render_Texture_Pixel_t<0x24,0,0>;
				break;
			//PSMT4HL CLUT=PSMCT16 AEM=0
			case 0x0224:
				return &Render_Texture_Pixel_t<0x24,2,0>;
				break;
			//PSMT4HL CLUT=PSMCT16S AEM=0
			case 0x0a24:
				return &Render_Texture_Pixel_t<0x24,0xa,0>;
				break;
				
			//PSMT4HL CLUT=PSMCT16 AEM=1
			case 0x1224:
				return &Render_Texture_Pixel_t<0x24,2,1>;
				break;
			//PSMT4HL CLUT=PSMCT16S AEM=1
			case 0x1a24:
				return &Render_Texture_Pixel_t<0x24,0xa,1>;
				break;

				
			//PSMT4HH CLUT=PSMCT32 AEM=0
			//PSMT4HH CLUT=PSMCT32 AEM=1
			case 0x002c:
			case 0x102c:
				return &Render_Texture_Pixel_t<0x2c,0,0>;
				break;
			//PSMT4HH CLUT=PSMCT16 AEM=0
			case 0x022c:
				return &Render_Texture_Pixel_t<0x2c,2,0>;
				break;
			//PSMT4HH CLUT=PSMCT16S AEM=0
			case 0x0a2c:
				return &Render_Texture_Pixel_t<0x2c,0xa,0>;
				break;
				
			//PSMT4HH CLUT=PSMCT16 AEM=1
			case 0x122c:
				return &Render_Texture_Pixel_t<0x2c,2,1>;
				break;
			//PSMT4HH CLUT=PSMCT16S AEM=1
			case 0x1a2c:
				return &Render_Texture_Pixel_t<0x2c,0xa,1>;
				break;
				
			default:
				cout << "\nhps2x64: GPU: ERROR: problem selecting texture pixel renderer. TEX_PSM=" << hex << TEX_PSM << " CLUT_PSM=" << CLUT_PSM << " TEXA_AEM=" << TEXA_AEM;
				return NULL;
				break;
			}
			
			break;
	}
}


#ifdef USE_TEMPLATES_PS2_RECTANGLE

template<const long TME,const long FGE,const long COLCLAMP,const long ABE,const long ATST,const long ZTST,const long DATE,const long ZMSK,const long FBPSM,const long ZBPSM>
static u64 Render_Generic_Rectangle_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << "; RenderSprite";
#endif

	// before sse2, was sending 1 pixels at a time
	static const int c_iVectorSize = 1;

	// notes: looks like sprite size is same as specified by w/h

	//u32 Pixel,
	
	u16 *CLUT_LUT;
	
	u32 TexelIndex;
	
	u32 color_add;
	
	u32 *ptr_texture;
	u8 *ptr_texture8;
	u16 *ptr_texture16;
	u32 *ptr_texture32;
	u32 *ptr_clut;
	u16 *ptr_clut16;
	u32 clut_width, clut_x, clut_y;

	// not used
	//u32 clut_xoffset, clut_yoffset;
	
	//u16 *ptr;
	s32 StartX, EndX, StartY, EndY;
	u32 NumberOfPixelsDrawn;
	
	
	u32 *ptr32;
	u16 *ptr16;
	
	u32 *zptr32;
	u16 *zptr16;
	
	u32 DestPixel, bgr, bgr_temp;
	u32 bgr0, bgr1;
	
	// 12.4 fixed point
	s32 x0, y0, x1, y1;
	s32 u0, v0, u1, v1;

	// and the z coords
	s64 z0, z1;
	
	// interpolate the z ??
	s64 dzdx, dzdy;
	s64 iZ;
	
	// interpolate f for fog
	s64 f0, f1;
	s64 dfdx, dfdy;
	s64 iF;
	
	
	s32 iU, iV, dudx, dvdy;
	
	s32 Line, x_across;
	
	u32 c1, c2, c3, ca;
	
	s64 Temp;
	s64 TempX, TempY;
	
	//u32 tge;
	
	//u32 DestPixel, PixelMask = 0, SetPixelMask = 0;
	s32 TexCoordX, TexCoordY;
	
	//u32 Shift1 = 0, Shift2 = 0, And1 = 0, And2 = 0;
	//u32 Shift0 = 0;
	u32 TextureOffset;
	
	u32 And3, Shift3, Shift4;
	
	u32 PixelsPerLine;
	
	u32 PixelFormat;
	u32 CLUTBufBase32, CLUTPixelFormat, CLUTStoreMode, CLUTOffset;
	
	u32 TexBufWidth, TexWidth, TexHeight, TexWidth_Mask, TexHeight_Mask;
	
	s32 TexX_Min, TexX_Max, TexY_Min, TexY_Max;
	u32 TexX_And, TexX_Or, TexY_And, TexY_Or;

	u64 NumPixels;
	
	s32 iR, iG, iB, iA;
	s64 iz;
	
	s32 Window_YTop, Window_YBottom, Window_XLeft, Window_XRight;
	
	s32 Coord_OffsetX, Coord_OffsetY;
	
	s32 RightMostX, LeftMostX, TopMostY, BottomMostY;
	u32 TexBufStartOffset32;
	u32 Clamp_ModeX, Clamp_ModeY;
	s32 Clamp_MinU, Clamp_MaxU, Clamp_MinV, Clamp_MaxV;
	u32 Fst;
	u32 TEX_PSM;
	u32 RGB24_Alpha, Pixel24_Mask, RGB24_TAlpha;
	FloatLong fTemp1, fTemp2;
	//long lTemp1, lTemp2;
	
	
	u64 TEXA64;
	u32 Combine;
	
	u32 TEX_TFX, TEX_TCC;
	u32 FOGCOL;
	
	u32 *buf32, *zbuf32;
	u32 SetPixelMask, FrameBufferWidthInPixels;
	
	u32 FrameBufferStartOffset32, ZBufferStartOffset32;
	
	u32 DA_Test, AlphaXor32, FrameBuffer_WriteMask32;
	u32 AlphaSelect [ 4 ];
	u32 uA, uB, uC, uD;
	
	u32 TEXA_AEM;
	
	AlphaTest at;
	ZTest zt;
	AlphaFail af;
	TexturePixel pd;
	TextureFunc tf;
	
	u32 TEST_ATST;
	u32 TEST_ZTST;
	u32 TEST_AFAIL;
	u32 aref;
	
	u32 Coord0x = 16, Coord0y = 16, Coord1x = 20, Coord1y = 20;
	
	
	Window_YTop = ( p_inputbuffer [ 0 ] >> 32 ) & 0x7ff;
	Window_YBottom = ( p_inputbuffer [ 0 ] >> 48 ) & 0x7ff;
	Window_XLeft = ( p_inputbuffer [ 0 ] ) & 0x7ff;
	Window_XRight = ( p_inputbuffer [ 0 ] >> 16 ) & 0x7ff;
	
	Coord_OffsetX = p_inputbuffer [ 1 ] & 0xffff;
	Coord_OffsetY = ( p_inputbuffer [ 1 ] >> 32 ) & 0xffff;
	
	// get x,y
	x0 = p_inputbuffer [ Coord0x + 1 ] & 0xffff;
	y0 = ( p_inputbuffer [ Coord0y + 1 ] >> 16 ) & 0xffff;
	x1 = p_inputbuffer [ Coord1x + 1 ] & 0xffff;
	y1 = ( p_inputbuffer [ Coord1y + 1 ] >> 16 ) & 0xffff;

	//////////////////////////////////////////
	// get coordinates on screen
	// *note* this is different from PS1, where you would add the offsets..
	x0 -= Coord_OffsetX;
	y0 -= Coord_OffsetY;
	x1 -= Coord_OffsetX;
	y1 -= Coord_OffsetY;

	LeftMostX = ( x1 > x0 ) ? ( ( x0 + 0xf ) >> 4 ) : ( ( x1 + 0xf ) >> 4 );
	RightMostX = ( x0 > x1 ) ? ( ( x0 - 1 ) >> 4 ) : ( ( x1 - 1 ) >> 4 );
	TopMostY = ( y1 > y0 ) ? ( ( y0 + 0xf ) >> 4 ) : ( ( y1 + 0xf ) >> 4 );
	BottomMostY = ( y0 > y1 ) ? ( ( y0 - 1 ) >> 4 ) : ( ( y1 - 1 ) >> 4 );
	
	// order coords so they go top to bottom and left to right
	if ( x1 < x0 )
	{
		// swap x,u coords
		Swap ( x0, x1 );
		
		// swap z,f coords ??
		// note: possibly need to update the z,f coords ??
		//Swap ( z0, z1 );
		
		
		if ( TME )
		{
		//Swap ( u0, u1 );
		Swap ( Coord0x, Coord1x );
		}
		
		//if ( FGE )
		//{
		//Swap ( f0, f1 );
		//}
	}
	
	if ( y1 < y0 )
	{
		// swap y,v coords
		Swap ( y0, y1 );
		
		// swap z,f coords ??
		// note: possibly need to update the z,f coords ??
		//Swap ( z0, z1 );
		
		if ( TME )
		{
		//Swap ( v0, v1 );
		Swap ( Coord0y, Coord1y );
		}
		
		//if ( FGE )
		//{
		//Swap ( f0, f1 );
		//}
	}

	
	// looks like some sprites have y1 < y0 and/or x1 < x0
	// didn't expect that so need to alert and figure out some other time
	if ( ( y1 < y0 ) || ( x1 < x0 ) )
	{
#if defined INLINE_DEBUG_SPRITE_TEST || defined INLINE_DEBUG_PRIMITIVE_TEST
		debug << dec << "\r\nERROR: y1 < y0 or x1 < x0 in sprite!!!";
		debug << dec << " FinalCoords: x0=" << x0 << " y0=" << y0 << " x1=" << x1 << " y1=" << y1;
#endif

#ifdef VERBOSE_SPRITE_ERROR
		cout << "\nhps2x64: GPU: ERROR: Sprite has x1 < x0 or y1 < y0!!!";
#endif

		return 0;
	}
	
	// coords are in 12.4 fixed point
	//x0 >>= 4;
	//y0 >>= 4;
	//x1 >>= 4;
	//y1 >>= 4;
	StartX = ( x0 + 0xf ) >> 4;
	EndX = ( x1 - 1 ) >> 4;
	StartY = ( y0 + 0xf ) >> 4;
	EndY = ( y1 - 1 ) >> 4;


	TempY = ( StartY << 4 ) - y0;

	if ( StartY < Window_YTop )
	{
		//v0 += ( Window_YTop - StartY ) * dvdy;
		TempY += ( Window_YTop - StartY ) << 4;
		StartY = Window_YTop;
	}
	
	
	
	if ( EndY > Window_YBottom )
	{
		EndY = Window_YBottom;
	}
	
	
	TempX = ( StartX << 4 ) - x0;
	
	if ( StartX < Window_XLeft )
	{
		//u0 += ( Window_XLeft - StartX ) * dudx;
		TempX += ( Window_XLeft - StartX ) << 4;
		StartX = Window_XLeft;
	}
	
	
	
	if ( EndX > Window_XRight )
	{
		EndX = Window_XRight;
	}
	

	// check that there is a pixel to draw
	if ( ( EndX < StartX ) || ( EndY < StartY ) )
	{
#if defined INLINE_DEBUG_SPRITE_TEST || defined INLINE_DEBUG_PRIMITIVE_TEST
		debug << dec << "\r\nERROR: EndY < StartY or EndX < StartX in sprite!!!";
		debug << dec << " FinalCoords: StartX=" << StartX << " StartY=" << StartY << " EndX=" << EndX << " EndY=" << EndY;
#endif

		return 0;
	}
	
	
	
	
	NumberOfPixelsDrawn = ( EndX - StartX + 1 ) * ( EndY - StartY + 1 );
		
	if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
	{
		ulInputBuffer_WriteIndex++;
		return NumberOfPixelsDrawn;
	}
	

	// check if sprite is within draw area
	if ( EndX < Window_XLeft || StartX > Window_XRight || EndY < Window_YTop || StartY > Window_YBottom ) return 0;
#if defined INLINE_DEBUG_SPRITE_TEST || defined INLINE_DEBUG_PRIMITIVE_TEST
	else
	{
		debug << dec << "\r\nDrawSprite" << " FinalCoords: x0=" << ( x0 >> 4 ) << " y0=" << ( y0 >> 4 ) << " x1=" << ( x1 >> 4 ) << " y1=" << ( y1 >> 4 );
		debug << dec << " u0=" << ( u0 >> 16 ) << " v0=" << ( v0 >> 16 ) << " u1=" << ( u1 >> 16 ) << " v1=" << ( v1 >> 16 );
		debug << hex << " z0=" << z0 << " z1=" << z1;
		debug << " bgr=" << bgr;
		debug << hex << "; TexBufPtr32/64=" << ( TexBufStartOffset32 >> 6 );
		debug << dec << "; TexWidth=" << TexWidth << " TexHeight=" << TexHeight;
		debug << dec << "; TexBufWidth=" << TexBufWidth;
		debug << "; TexPixFmt=" << PixelFormat_Names [ PixelFormat ];
		debug << "; Clamp_ModeX=" << Clamp_ModeX << " Clamp_ModeY=" << Clamp_ModeY;
		debug << hex << "; CLUTBufPtr32/64=" << ( CLUTBufBase32 >> 6 );
		debug << "; CLUTPixFmt=" << PixelFormat_Names [ CLUTPixelFormat ];
		debug << hex << "; CLUTOffset/16=" << CLUTOffset;
		debug << "; CSM=" << CLUTStoreMode;
		debug << "; TME=" << hex << TME;
		debug << hex << "; FrameBufPtr32/64=" << ( FrameBufferStartOffset32 >> 6 );
		debug << dec << " FrameBufWidth=" << FrameBufferWidthInPixels;
		debug << " FrameBufPixFmt=" << PixelFormat_Names [ FBPSM ];
		debug << " ATST=" << hex << TEST_ATST;
		debug << " AFAIL=" << hex << TEST_AFAIL;
		debug << " Alpha=" << ABE;
		debug << " SetPixelMask=" << SetPixelMask;
		debug << " AlphaXor32=" << AlphaXor32;
		debug << " DA_Test=" << DA_Test;
		debug << hex << " ZBuf=" << (ZBufferStartOffset32 >> 11);
		debug << PixelFormat_Names [ ZBPSM ];
		//debug << " TEST=" << TEST_X.Value;
		//debug << " LCM=" << TEX1 [ Ctx ].LCM;
		//debug << " K=" << TEX1 [ Ctx ].K;
	}
#endif
	

	
	
	
	
	
	// get z
	//z0 = ( p_inputbuffer [ Coord0 + 1 ] >> 32 );
	z1 = ( p_inputbuffer [ 16 + 5 ] >> 32 );
	
	// constant z-value for rectangles/sprites (using z1 here)
	//iz = z1 << 16;
	
	
	
	
	// get bgr color value
	//bgr0 = p_inputbuffer [ Coord0 + 0 ] & 0xffffffffULL;
	bgr1 = p_inputbuffer [ 16 + 4 ] & 0xffffffffULL;
	
	bgr = bgr1;
	
//cout << "\nx0=" << dec << x0 << " y0=" << y0 << " x1=" << x1 << " y1=" << y1 << " z0=" << z0 << " z1=" << z1 << hex << " bgr0=" << bgr0 << " bgr1=" << bgr1;
//cout << "\nABE=" << hex << ABE << " TEST_ATST=" << TEST_ATST << " TEST_AFAIL=" << TEST_AFAIL << " TEST_ZTST=" << TEST_ZTST << " A=" << uA << " B=" << uB << " C=" << uC << " D=" << uD << " PRIM=" << p_inputbuffer [ 15 ];
	
	/*
	// set fixed alpha values
	AlphaSelect [ 2 ] = ALPHA_X.FIX << 24;
	AlphaSelect [ 3 ] = 0;
	*/
	
	


	if ( TME )
	{
	// get color components
	c1 = bgr & 0xff;
	c2 = ( bgr >> 8 ) & 0xff;
	c3 = ( bgr >> 16 ) & 0xff;
	ca = ( bgr >> 24 ) & 0xff;
		
	
	

	//TEX0_t *TEX0 = TEXX;
	//TEX1_t *TEX1 = &GPURegsGp.TEX1_1;
	
	
	//TexBufWidth = TEX0 [ Ctx ].TBW0 << 6;
	TexBufWidth = ( ( p_inputbuffer [ 13 ] >> 14 ) & 0x3f ) << 6;
	//TexWidth = 1 << TEX0 [ Ctx ].TW;
	TexWidth = 1 << ( ( p_inputbuffer [ 13 ] >> 26 ) & 0xf );
	//TexHeight = 1 << TEX0 [ Ctx ].TH;
	TexHeight = 1 << ( ( p_inputbuffer [ 13 ] >> 30 ) & 0xf );
	TexWidth_Mask = TexWidth - 1;
	TexHeight_Mask = TexHeight - 1;
	
	
	
	
	/////////////////////////////////////////////////////////
	// Get offset into texture page
	//TexBufStartOffset32 = TEX0 [ Ctx ].TBP0 << 6;
	TexBufStartOffset32 = ( p_inputbuffer [ 13 ] & 0x3fff ) << 6;
	
	// get the pixel format (16 bit, 32 bit, etc)
	//PixelFormat = TEX0 [ Ctx ].PSM;
	PixelFormat = ( ( p_inputbuffer [ 13 ] >> 20 ) & 0x3f );
	
	

	// color lookup table width is stored in TEXCLUT register (in pixels/64)
	// get clut width and x in pixels
	//clut_width = GPURegsGp.TEXCLUT.CBW << 6;
	clut_width = ( p_inputbuffer [ 14 ] & 0x3f ) << 6;
	
	
	//clut_x = GPURegsGp.TEXCLUT.COU << 6;
	clut_x = p_inputbuffer [ 14 ] & 0xfc0;
	
	// get clut y in units of pixels
	//clut_y = GPURegsGp.TEXCLUT.COV;
	clut_y = ( p_inputbuffer [ 14 ] & 0x3ff000 ) >> 12;
	
	
	//CLUTPixelFormat = TEX0 [ Ctx ].CPSM;
	CLUTPixelFormat = ( ( p_inputbuffer [ 13 ] >> 51 ) & 0xf );
	
	// get base pointer to color lookup table (32-bit word address divided by 64)
	//CLUTBufBase32 = TEX0 [ Ctx ].CBP << 6;
	CLUTBufBase32 = ( ( p_inputbuffer [ 13 ] >> 37 ) & 0x3fff ) << 6;
	
	// storage mode ??
	//CLUTStoreMode = TEX0 [ Ctx ].CSM;
	CLUTStoreMode = ( ( p_inputbuffer [ 13 ] >> 55 ) & 0x1 );
	
	// clut offset ??
	// this is the offset / 16
	// note: this is actually the position in the temporary buffer, not in the local memory
	//CLUTOffset = TEX0 [ Ctx ].CSA;
	CLUTOffset = ( ( p_inputbuffer [ 13 ] >> 56 ) & 0x1f );

	//clut_xoffset = clut_x << 4;
	//clut_xoffset = 0;
	//ptr_clut = & ( VRAM [ clut_y << 10 ] );
	ptr_clut = & ( _GPU->RAM32 [ CLUTBufBase32 ] );

	//////////////////////////////////////////////////////
	// Get offset into color lookup table
	//u32 ClutOffset = ( clut_x << 4 ) + ( clut_y << 10 );
	
	if ( !CLUTStoreMode )
	{
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << " CSM1";
#endif

		// CSM1 //
		//CLUT_LUT = ucCLUT_Lookup_CSM01_4bpp;
		
		if ( CLUTPixelFormat & 0x2 )
		{
			// 4-bit pixels - 16 colors
			// 16-bit pixels in CLUT //
			CLUTOffset &= 0x1f;
		}
		else
		{
			// 32-bit pixels in CLUT //
			CLUTOffset &= 0xf;
		}
		
	}
	else
	{
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << " CSM2";
#endif

		// CSM2 //
		//CLUT_LUT = ucCLUT_Lookup_CSM02;
		
		// use clut_x, clut_y, clut_width
		//ptr_clut = & ( ptr_clut [ ( clut_x >> Shift3 ) + ( clut_y * ( clut_width >> Shift3 ) ) ] );
	}
	
	
	CLUTOffset <<= 4;
	ptr_clut16 = & ( _GPU->InternalCLUT [ CLUTOffset ] );

	
	

	TEX_TCC = ( p_inputbuffer [ 13 ] >> 34 ) & 1;
	TEX_TFX = ( p_inputbuffer [ 13 ] >> 35 ) & 3;

	TEXA_AEM = ( p_inputbuffer [ 31 ] >> 15 ) & 1;
	
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << "; TexPixFmt=" << PixelFormat_Names [ PixelFormat ];
#endif

	//tge = command_tge;
	//if ( ( bgr & 0x00ffffff ) == 0x00808080 ) tge = 1;
	
	
	//ptr_texture = & ( VRAM [ ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) ] );
	ptr_texture = & ( _GPU->RAM32 [ TexBufStartOffset32 ] );
	
	// *** TODO *** use a union
	ptr_texture8 = (u8*) ptr_texture;
	ptr_texture16 = (u16*) ptr_texture;
	ptr_texture32 = (u32*) ptr_texture;
	
	
	
	// texture pixel format variables //
	
	
	// get texture drawing function
	pd = Select_TexturePixel_t ( PixelFormat, CLUTPixelFormat, TEXA_AEM );
	
	if ( !pd ) return 0;
	
	Combine = TEX_TFX | ( TEX_TCC << 2 );
	
	switch ( Combine )
	{
		case 0:
			tf = & TextureFunc32_t<COLCLAMP,0,0>;
			break;
		case 1:
			tf = & TextureFunc32_t<COLCLAMP,1,0>;
			break;
		case 2:
			tf = & TextureFunc32_t<COLCLAMP,2,0>;
			break;
		case 3:
			tf = & TextureFunc32_t<COLCLAMP,3,0>;
			break;
		case 4:
			tf = & TextureFunc32_t<COLCLAMP,0,1>;
			break;
		case 5:
			tf = & TextureFunc32_t<COLCLAMP,1,1>;
			break;
		case 6:
			tf = & TextureFunc32_t<COLCLAMP,2,1>;
			break;
		case 7:
			tf = & TextureFunc32_t<COLCLAMP,3,1>;
			break;
	}
	
	// TEXA64 is TEXA pre-shifted to the left by 24
	//TEXA64 = ( p_inputbuffer [ 31 ] >> 8 ) & 0xff000000ull;
	TEXA64 = p_inputbuffer [ 31 ] << 24;
	



	
	// important: reading from buffer in 32-bit units, so the texture buffer width should be divided accordingly
	//TexBufWidth >>= Shift1;
	


	if ( FGE )
	{
	FOGCOL = p_inputbuffer [ 11 ];
	
	// get fog coefficient
	//f0 = (u64) f [ Coord0 ].F;
	//f1 = (u64) f [ Coord1 ].F;
	//f0 = ( p_inputbuffer [ 19 ] >> 56 ) & 0xff;
	f1 = ( p_inputbuffer [ 23 ] >> 56 ) & 0xff;
	}


	
	Fst = p_inputbuffer [ 15 ] & 0x100;
	
	// check if sprite should use UV coords or ST coords
	// on PS2, sprite can use the ST register to specify texture coords
	//if ( GPURegsGp.PRIM.FST )
	if ( Fst )
	{
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << " FST";
#endif

		// get u,v
		//u0 = p_inputbuffer [ 18 ] & 0x3fff;
		u0 = p_inputbuffer [ Coord0x + 2 ] & 0x3fff;
		//v0 = ( p_inputbuffer [ 18 ] >> 16 ) & 0x3fff;
		v0 = ( p_inputbuffer [ Coord0y + 2 ] >> 16 ) & 0x3fff;
		//u1 = p_inputbuffer [ 22 ] & 0x3fff;
		u1 = p_inputbuffer [ Coord1x + 2 ] & 0x3fff;
		//v1 = ( p_inputbuffer [ 22 ] >> 16 ) & 0x3fff;
		v1 = ( p_inputbuffer [ Coord1y + 2 ] >> 16 ) & 0x3fff;
	}
	else
	{
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << " !FST";
#endif
		
		// put s,t coords into 10.4 fixed point
		// note: tex width/height should probably be minus one
		
		//fTemp1.l = p_inputbuffer [ 18 ] & 0xffffffffull;
		fTemp1.l = p_inputbuffer [ Coord0x + 2 ] & 0xffffffffull;
		//fTemp2.l = p_inputbuffer [ 18 ] >> 32;
		fTemp2.l = p_inputbuffer [ Coord0y + 2 ] >> 32;
		u0 = (s32) ( fTemp1.f * ( (float) ( TexWidth ) ) * 16.0f );
		v0 = (s32) ( fTemp2.f * ( (float) ( TexHeight ) ) * 16.0f );
		//fTemp1.l = p_inputbuffer [ 22 ] & 0xffffffffull;
		fTemp1.l = p_inputbuffer [ Coord1x + 2 ] & 0xffffffffull;
		//fTemp2.l = p_inputbuffer [ 22 ] >> 32;
		fTemp2.l = p_inputbuffer [ Coord1y + 2 ] >> 32;
		u1 = (s32) ( fTemp1.f * ( (float) ( TexWidth ) ) * 16.0f );
		v1 = (s32) ( fTemp2.f * ( (float) ( TexHeight ) ) * 16.0f );
	}



	// *** testing ***
	/*
	if ( PixelFormat == 1 )
	{
		// rgb24 //
		
		// TEXA.TA0 is the alpha of rgb24 value
		_GPU->RGB24_Alpha = _GPU->GPURegsGp.TEXA.TA0 << 24;
		
		// 24-bit pixel only
		_GPU->Pixel24_Mask = 0xffffff;
		
		// r,g,b=0 is transparent for rgb24 when TEXA.AEM is one
		_GPU->RGB24_TAlpha = ( _GPU->GPURegsGp.TEXA.AEM ^ 1 ) << 31;
	}
	else
	{
		// NOT rgb24 //
		_GPU->RGB24_Alpha = 0;
		_GPU->Pixel24_Mask = 0xffffffff;
		_GPU->RGB24_TAlpha = 0;
	}
	*/

	
	}	// end if ( TME )
		


	// initialize number of pixels drawn
	//NumberOfPixelsDrawn = 0;
	
	

	
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << hex << " bgr=" << bgr;
	debug << dec << "; Coords: x0=" << ( x0 >> 4 ) << " y0=" << ( y0 >> 4 ) << " x1=" << ( x1 >> 4 ) << " y1=" << ( y1 >> 4 );
	debug << dec << " u0=" << ( u0 >> 4 ) << " v0=" << ( v0 >> 4 ) << " u1=" << ( u1 >> 4 ) << " v1=" << ( v1 >> 4 );
	debug << hex << " z0=" << z0 << " z1=" << z1;
#endif

	
	
	
	
	if ( TME )
	{
	// tex coords are in 10.4 fixed point
	u0 <<= 16;
	v0 <<= 16;
	u1 <<= 16;
	v1 <<= 16;
	
	if ( x1 - x0 ) dudx = ( u1 - u0 ) / ( x1 - x0 );
	if ( y1 - y0 ) dvdy = ( v1 - v0 ) / ( y1 - y0 );

	//if ( FGE )
	//{
	//if ( x1 - x0 ) dfdx = ( ( f1 - f0 ) << 16 ) / ( x1 - x0 );
	//if ( y1 - y0 ) dfdy = ( ( f1 - f0 ) << 16 ) / ( y1 - y0 );
	//}

	// shift back down to get into 16.16 format
	u0 >>= 4;
	v0 >>= 4;
	u1 >>= 4;
	v1 >>= 4;
	}	// end if ( TME )

	
	
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << dec << "; FinalCoords: x0=" << x0 << " y0=" << y0 << " x1=" << x1 << " y1=" << y1;
	debug << hex << " u0=" << u0 << " v0=" << v0 << " u1=" << u1 << " v1=" << v1;
#endif



	if ( TME )
	{
		
	Clamp_ModeX = p_inputbuffer [ 12 ] & 0x3;
	Clamp_ModeY = ( p_inputbuffer [ 12 ] >> 2 ) & 0x3;
	
	Clamp_MinU = ( p_inputbuffer [ 12 ] >> 4 ) & 0x3ff;
	Clamp_MaxU = ( p_inputbuffer [ 12 ] >> 14 ) & 0x3ff;
	Clamp_MinV = ( p_inputbuffer [ 12 ] >> 24 ) & 0x3ff;
	Clamp_MaxV = ( p_inputbuffer [ 12 ] >> 34 ) & 0x3ff;
	
	switch ( Clamp_ModeY )
	{
		case 0:
			// repeat //
			//TexCoordY &= TexHeight_Mask;
			TexY_And = TexHeight_Mask;
			
			TexY_Or = 0;
			
			// can only have coords in range -2047 to +2047
			TexY_Min = -2047;
			TexY_Max = 2047;
			break;
			
		case 1:
			// clamp //
			//TexCoordY = ( TexCoordY < 0 ) ? 0 : TexCoordY;
			//TexCoordY = ( TexCoordY > TexHeight ) ? TexHeight : TexCoordY;
			TexY_Min = 0;
			TexY_Max = TexHeight_Mask;
			
			TexY_And = TexHeight_Mask;
			TexY_Or = 0;
			break;
			
		case 2:
			// region clamp //
			//TexCoordY = ( TexCoordY < Clamp_MinV ) ? Clamp_MinV : TexCoordY;
			//TexCoordY = ( TexCoordY > Clamp_MaxV ) ? Clamp_MaxV : TexCoordY;
			TexY_Min = Clamp_MinV;
			TexY_Max = Clamp_MaxV;
			
			TexY_And = TexHeight_Mask;
			TexY_Or = 0;
			break;
			
		case 3:
			// region repeat //
			// this one is just like on the ps1
			//TexCoordY = ( TexCoordY & Clamp_MinV ) | Clamp_MaxV;
			TexY_And = Clamp_MinV & TexHeight_Mask;
			TexY_Or = Clamp_MaxV & TexHeight_Mask;
			
			// can only have coords in range -2047 to +2047
			TexY_Min = -2047;
			TexY_Max = 2047;
			break;
	}

	
	switch ( Clamp_ModeX )
	{
		case 0:
			// repeat //
			//TexCoordY &= TexHeight_Mask;
			TexX_And = TexWidth_Mask;
			
			TexX_Or = 0;
			
			// can only have coords in range -2047 to +2047
			TexX_Min = -2047;
			TexX_Max = 2047;
			break;
			
		case 1:
			// clamp //
			//TexCoordY = ( TexCoordY < 0 ) ? 0 : TexCoordY;
			//TexCoordY = ( TexCoordY > TexHeight ) ? TexHeight : TexCoordY;
			TexX_Min = 0;
			TexX_Max = TexWidth_Mask;
			
			TexX_And = TexWidth_Mask;
			TexX_Or = 0;
			break;
			
		case 2:
			// region clamp //
			//TexCoordY = ( TexCoordY < Clamp_MinV ) ? Clamp_MinV : TexCoordY;
			//TexCoordY = ( TexCoordY > Clamp_MaxV ) ? Clamp_MaxV : TexCoordY;
			TexX_Min = Clamp_MinU;
			TexX_Max = Clamp_MaxU;
			
			TexX_And = TexWidth_Mask;
			TexX_Or = 0;
			break;
			
		case 3:
			// region repeat //
			// this one is just like on the ps1
			//TexCoordY = ( TexCoordY & Clamp_MinV ) | Clamp_MaxV;
			TexX_And = Clamp_MinU & TexWidth_Mask;
			TexX_Or = Clamp_MaxU & TexWidth_Mask;
			
			// can only have coords in range -2047 to +2047
			TexX_Min = -2047;
			TexX_Max = 2047;
			break;
	}
	
	}	// end if ( TME )

	if ( !ATST )
	{
	TEST_ATST = ( p_inputbuffer [ 5 ] >> 1 ) & 7;
	TEST_AFAIL = ( p_inputbuffer [ 5 ] >> 12 ) & 3;
	
	aref = ( p_inputbuffer [ 5 ] >> 4 ) & 0xff;
	
	switch ( TEST_ATST )
	{
		case 0:
			at = & Playstation2::GPU::TestSrcAlpha32_t<0>;
			break;
		case 1:
			at = & Playstation2::GPU::TestSrcAlpha32_t<1>;
			break;
		case 2:
			aref = ( aref << 24 );
			at = & Playstation2::GPU::TestSrcAlpha32_t<2>;
			break;
		case 3:
			aref = ( aref << 24 ) | 0xffffff;
			at = & Playstation2::GPU::TestSrcAlpha32_t<3>;
			break;
		case 4:
			//aref = ( (u32) TEST_X.AREF );
			at = & Playstation2::GPU::TestSrcAlpha32_t<4>;
			break;
		case 5:
			aref = ( aref << 24 );
			at = & Playstation2::GPU::TestSrcAlpha32_t<5>;
			break;
		case 6:
			aref = ( aref << 24 ) | 0xffffff;
			at = & Playstation2::GPU::TestSrcAlpha32_t<6>;
			break;
		case 7:
			at = & Playstation2::GPU::TestSrcAlpha32_t<7>;
			break;
	}
	
	switch ( TEST_AFAIL )
	{
		case 0:
			af = & Playstation2::GPU::PerformAlphaFail32_t<0,FBPSM,ZBPSM,ZMSK>;
			break;
		case 1:
			af = & Playstation2::GPU::PerformAlphaFail32_t<1,FBPSM,ZBPSM,ZMSK>;
			break;
		case 2:
			af = & Playstation2::GPU::PerformAlphaFail32_t<2,FBPSM,ZBPSM,ZMSK>;
			break;
		case 3:
			af = & Playstation2::GPU::PerformAlphaFail32_t<3,FBPSM,ZBPSM,ZMSK>;
			break;
	}
	
	}	// end if ( !ATST )
	

	if ( !ZTST )
	{
	TEST_ZTST = ( p_inputbuffer [ 5 ] >> 17 ) & 3;
	
	switch ( TEST_ZTST )
	{
		case 0:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<0,ZBPSM>;
			break;
		case 1:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<1,ZBPSM>;
			break;
		case 2:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<2,ZBPSM>;
			break;
		case 3:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<3,ZBPSM>;
			break;
	}
	
	}	// end if ZTST
	
	
	
	SetPixelMask = p_inputbuffer [ 4 ] << 31;
	
	
	FrameBuffer_WriteMask32 = 0xffffffffull & ~( p_inputbuffer [ 2 ] >> 32 );
	
	//FrameBufferStartOffset32 = Frame [ Ctx ].FBP << 11;
	FrameBufferStartOffset32 = ( p_inputbuffer [ 2 ] & 0x1ff ) << 11;
	FrameBufferWidthInPixels = ( ( p_inputbuffer [ 2 ] >> 16 ) & 0x3f ) << 6;

	ZBufferStartOffset32 = ( p_inputbuffer [ 3 ] & 0x1ff ) << 11;
	
	
	buf32 = & ( _GPU->RAM32 [ FrameBufferStartOffset32 ] );
	zbuf32 = & ( _GPU->RAM32 [ ZBufferStartOffset32 ] );
	
//cout << "\nbuf32=" << hex << buf32 << " zbuf32=" << zbuf32;
	
	if ( DATE )
	{
	DA_Test = ( p_inputbuffer [ 5 ] << 16 ) & 0x80000000ull;
	}	//end if ( DATE )
	
	if ( ABE )
	{
	AlphaXor32 = p_inputbuffer [ 8 ] << 31;

	uA = ( p_inputbuffer [ 7 ] >> 0 ) & 3;
	uB = ( p_inputbuffer [ 7 ] >> 2 ) & 3;
	uC = ( p_inputbuffer [ 7 ] >> 4 ) & 3;
	uD = ( p_inputbuffer [ 7 ] >> 6 ) & 3;
	

	// set fixed alpha values
	
	// current RGBA value
	//AlphaSelect [ 0 ] = rgbaq_Current.Value & 0xffffffffL;
	AlphaSelect [ 0 ] = p_inputbuffer [ 16 + 4 ] & 0xffffffffULL;
	
	// FIX value
	//AlphaSelect [ 2 ] = ALPHA_X.FIX << 24;
	AlphaSelect [ 2 ] = ( p_inputbuffer [ 7 ] >> 8 ) & 0xff000000ULL;
	
	// ZERO
	AlphaSelect [ 3 ] = 0;
	
	
	// *** TESTING ***
	/*
	_GPU->AlphaSelect [ 0 ] = p_inputbuffer [ 16 + 4 ] & 0xffffffffULL;
	_GPU->AlphaSelect [ 2 ] = _GPU->ALPHA_X.FIX << 24;
	_GPU->AlphaSelect [ 3 ] = 0;
	*/
	
	}	// end if ( ABE )
	
	
	
	
	if ( TME )
	{
	v0 += ( dvdy >> 4 ) * TempY;
	}
	
	if ( TME )
	{
	u0 += ( dudx >> 4 ) * TempX;
	}

	if ( TME )
	{
	//iV = v;
	iV = v0;
	}
	
	//if ( FGE )
	//{
	//	f0 <<= 16;
	//}

		for ( Line = StartY; Line <= EndY; Line++ )
		{
//#if defined INLINE_DEBUG_SPRITE_PIXEL
//	debug << "\r\n";
//#endif
//#if defined INLINE_DEBUG_SPRITE_PIXEL
//	debug << " y=" << dec << Line;
//#endif

			if ( TME )
			{
			// need to start texture coord from left again
			//iU = u;
			iU = u0;
			
			//TexCoordY = (u8) ( ( iV & ~( TWH << 3 ) ) | ( ( TWY & TWH ) << 3 ) );
			//TexCoordY = (u8) ( ( iV & Not_TWH ) | ( TWYTWH ) );
			//TexCoordY <<= 10;
			TexCoordY = ( iV >> 16 ) /* & TexHeight_Mask */;
			
			TexCoordY = ( ( TexCoordY < TexY_Min ) ? TexY_Min : TexCoordY );
			TexCoordY = ( ( TexCoordY > TexY_Max ) ? TexY_Max : TexCoordY );
			TexCoordY &= TexY_And;
			TexCoordY |= TexY_Or;
			}
			
			//ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			//ptr32 = & ( buf [ StartX + ( Line * FrameBufferWidth_Pixels ) ] );
			
//#if defined INLINE_DEBUG_SPRITE_PIXEL
//	debug << " TY=" << dec << TexCoordY;
//#endif

			// initialize z for line
			//iZ = z0;
			
			//if ( FGE )
			//{
			// initialize f for line
			//iF = f0;
			//iF = f1;
			//}

			// draw horizontal line
			//for ( x_across = StartX; x_across <= EndX; x_across++ )
			for ( x_across = StartX; x_across <= EndX /* && ptr32 < PtrEnd */; x_across += c_iVectorSize )
			{
//#if defined INLINE_DEBUG_SPRITE_PIXEL
//	debug << " x=" << dec << x_across;
//#endif

				// get pointer into frame buffer
				//ptr32 = & ( buf32 [ CvtAddrPix32 ( x_across, Line, FrameBufferWidth_Pixels ) ] );

				if ( TME )
				{
				//TexCoordX = (u8) ( ( iU & Not_TWW ) | ( TWXTWW ) );
				TexCoordX = ( iU >> 16 ) /* & TexWidth_Mask */;
				
				TexCoordX = ( ( TexCoordX < TexX_Min ) ? TexX_Min : TexCoordX );
				TexCoordX = ( ( TexCoordX > TexX_Max ) ? TexX_Max : TexCoordX );
				TexCoordX &= TexX_And;
				TexCoordX |= TexX_Or;

//#if defined INLINE_DEBUG_SPRITE_PIXEL
//	debug << " TX=" << dec << TexCoordX;
//#endif


//if ( !Line )
//{				
//	debug << "\r\nx_across=" << dec << x_across << " Offset=" << CvtAddrPix32 ( x_across, Line, FrameBufferWidth_Pixels ) << " TOffset=" << ;
//}
				
				// get texel from texture buffer (32-bit frame buffer) //
				bgr = pd ( ptr_texture32, TexCoordX, TexCoordY, TexBufWidth, ptr_clut16, TEXA64 );
				
				/*
				switch ( PixelFormat )
				{
					// PSMCT32
					case 0:
						bgr = ptr_texture32 [ CvtAddrPix32 ( TexCoordX, TexCoordY, TexBufWidth ) ];
						break;
						
					// PSMCT24
					case 1:
						bgr = ptr_texture32 [ CvtAddrPix32 ( TexCoordX, TexCoordY, TexBufWidth ) ];
						break;
						
					// PSMCT16
					case 2:
						bgr = ptr_texture16 [ CvtAddrPix16 ( TexCoordX, TexCoordY, TexBufWidth ) ];
						break;
						
					// PSMCT16S
					case 0xa:
						bgr = ptr_texture16 [ CvtAddrPix16S ( TexCoordX, TexCoordY, TexBufWidth ) ];
						break;
						
					// PSMT8
					case 0x13:
						bgr_temp = ptr_texture8 [ CvtAddrPix8 ( TexCoordX, TexCoordY, TexBufWidth ) ];
						break;
						
					// PSMT4
					case 0x14:
						bgr = CvtAddrPix4 ( TexCoordX, TexCoordY, TexBufWidth );
						bgr_temp = ( ( ptr_texture8 [ bgr >> 1 ] ) >> ( ( bgr & 1 ) << 2 ) ) & 0xf;
						break;
						
					// PSMT8H
					case 0x1b:
						bgr_temp = ( ptr_texture32 [ CvtAddrPix32 ( TexCoordX, TexCoordY, TexBufWidth ) ] ) >> 24;
						break;
						
					// PSMT4HL
					case 0x24:
						bgr_temp = ( ptr_texture32 [ CvtAddrPix32 ( TexCoordX, TexCoordY, TexBufWidth ) ] >> 24 ) & 0xf;
						break;
						
					// PSMT4HH
					case 0x2c:
						bgr_temp = ptr_texture32 [ CvtAddrPix32 ( TexCoordX, TexCoordY, TexBufWidth ) ] >> 28;
						break;
						
					// Z-buffer formats
					
					// PSMZ32
					case 0x30:
					
					// PSMZ24
					case 0x31:
					//case 0x35:
						bgr = ptr_texture32 [ CvtAddrZBuf32 ( TexCoordX, TexCoordY, TexBufWidth ) ];
						break;
						
					// PSMZ16
					case 0x32:
						bgr = ptr_texture16 [ CvtAddrZBuf16 ( TexCoordX, TexCoordY, TexBufWidth ) ];
						break;
						
					// PSMZ16S
					case 0x3a:
						bgr = ptr_texture16 [ CvtAddrZBuf16S ( TexCoordX, TexCoordY, TexBufWidth ) ];
						break;
				}
				
				// look up color value in CLUT if needed //
				//if ( PixelFormat >> 4 )
				if ( ( PixelFormat & 7 ) >= 3 )
				{
					// lookup color value in CLUT //
					
//#if defined INLINE_DEBUG_SPRITE_PIXEL
//	debug << " IDX=" << hex << bgr_temp;
//#endif

					bgr = ptr_clut16 [ bgr_temp ];
					
					if ( ! ( CLUTPixelFormat & 0x2 ) )
					{
						// 32-bit pixels in CLUT //
						bgr |= ( (u32) ptr_clut16 [ bgr_temp + 256 ] ) << 16;
					}
				}
				
				// convert 16-bit pixels to 32-bit (since 32-bit frame buffer here) //
				
				// check if pixel is 16-bit
				if ( ( ( PixelFormat == 0x2 ) || ( PixelFormat == 0xa ) ) || ( ( PixelFormat > 0xa ) && ( CLUTPixelFormat & 0x2 ) ) )
				{
					// pixel is 16-bit //
					
					// get alpha component of 16-bit pixel
					//bgr_temp = bgr & 0x8000;
					
					// check if pixel is definitely not transparent, if transparent then just stays zero
					if ( ( !_GPU->GPURegsGp.TEXA.AEM ) || bgr )
					{
						// pixel is probably not transparent ?? //
						
						bgr_temp = bgr;
						
						// convert to 32-bit
						bgr = ConvertPixel16To24 ( bgr );
						
						// set the texture alpha for 16 bit pixel
						bgr |= ( ( ( bgr_temp & 0x8000 ) ? _GPU->GPURegsGp.TEXA.TA1 : _GPU->GPURegsGp.TEXA.TA0 ) << 24 );
					}
				}
				
				if ( PixelFormat == 1 )
				{
					// 24-bit pixel //
					bgr &= 0xffffff;
					
					if ( ( !_GPU->GPURegsGp.TEXA.AEM ) || bgr )
					{
						// alpha is TA0
						bgr |= ( _GPU->GPURegsGp.TEXA.TA0 << 24 );
					}
				}
				*/
				
				
				

				// texture function ??
				
				
				// fog function ??
				
				//if ( bgr != -1ULL )
				//if ( bgr )
				//{
					
					bgr = tf ( bgr, c1, c2, c3, ca );
					//bgr = _GPU->TextureFunc32 ( bgr, c1, c2, c3, ca );

					if ( FGE )
					{
						bgr = FogFunc32_t<COLCLAMP> ( bgr, f1, FOGCOL );
						//bgr = _GPU->FogFunc32 ( bgr, f1 );
						//bgr = _GPU->FogFunc32 ( bgr, iF >> 16 );
					}
					

					
					// no need to interpolate Z-value for sprite? just use z1 value?
					PlotPixel_Gradient_t<0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( buf32, zbuf32, x_across, Line, z1, bgr, SetPixelMask, FrameBufferWidthInPixels, DA_Test, AlphaXor32, FrameBuffer_WriteMask32, aref, at, zt, af, AlphaSelect, uA, uB, uC, uD );
					//_GPU->PlotPixel_Gradient ( x_across, Line, z1, bgr );
					//_GPU->PlotPixel_Texture ( x_across, Line, z1, bgr, c1, c2, c3, ca, f1 /* iF >> 16 */ );
				//}
				
				}
				else
				{
					PlotPixel_Gradient_t<0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( buf32, zbuf32, x_across, Line, z1, bgr, SetPixelMask, FrameBufferWidthInPixels, DA_Test, AlphaXor32, FrameBuffer_WriteMask32, aref, at, zt, af, AlphaSelect, uA, uB, uC, uD );
					//_GPU->PlotPixel_Gradient ( x_across, Line, z1, bgr );
				}
				
				// update z on line
				//iZ += dzdx;
				
				//if ( FGE )
				//{
				// update f on line
				//iF += dfdx;
				//}
				
				if ( TME )
				{
				/////////////////////////////////////////////////////////
				// interpolate texture coords across
				//iU += c_iVectorSize;
				iU += dudx;
				}
				
				// update pointer for pixel out
				//ptr32 += c_iVectorSize;
					
			}
			
			if ( TME )
			{
			/////////////////////////////////////////////////////////
			// interpolate texture coords down
			//iV++;
			iV += dvdy;
			}
			
			//z0 += dzdy;
			
			//if ( FGE )
			//{
			//f0 += dfdy;
			//}
		}
		
	return NumberOfPixelsDrawn;
}



template<const long TME,const long FGE,const long COLCLAMP,const long ABE,const long ATST,const long ZTST,const long DATE,const long ZMSK>
inline static u64 Select_RenderSprite3_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	u32 FBPSM, ZBPSM;
	u32 Combine;
	
//cout << "\n->Select_RenderSprite3_t";
	
	// 000000: PSMCT32, 000001: PSMCT24, 000010: PSMCT16, 001010: PSMCT16S
	// 110000: PSMZ32, 110001: PSMZ24, 110010: PSMZ16, 111010: PSMZ16S
	FBPSM = ( p_inputbuffer [ 2 ] >> 24 ) & 0x3f;
	
	// 0000: PSMZ32, 0001: PSMZ24, 0010: PSMZ16, 1010: PSMZ16S
	ZBPSM = ( p_inputbuffer [ 3 ] >> 24 ) & 0xf;
	
	Combine = ( FBPSM << 4 ) | ( ZBPSM );
	
	switch ( Combine )
	{
		// 16-bit combinations

		// PSMCT16
		case 0x022:
			//template<TME,FGE,FST,COLCLAMP,ABE,ATST,DATE,ZTST,ZMSK,FBPSM,ZBPSM>
			return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x02,0x02>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x020:
		case 0x021:
		case 0x02a:
			return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,1,DATE,1,0x02,0x00>( p_inputbuffer, ulThreadNum );
			break;
		
		// PSMZ16
		case 0x322:
			//return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x32,0x02>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Rectangle_t<TME,0,0,0,0,ZTST,0,ZMSK,0x32,0x02>( p_inputbuffer, ulThreadNum );
			break;
			
		// 32-bit combinations
		
		// PSMCT32
		case 0x000:
			return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x00,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x001:
			return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x00,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x002:
			return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,1,DATE,1,0x00,0x02>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x00a:
			return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x00,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMCT24
		case 0x010:
			return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x01,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x011:
			return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x01,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x012:
			return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,1,0,1,0x01,0x02>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x01a:
			return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x01,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMCT16S
		case 0x0a0:
			return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x0a,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x0a1:
			return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x0a,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x0a2:
			return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,1,DATE,1,0x0a,0x02>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x0aa:
			return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x0a,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMZ32
		case 0x300:
			//return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x30,0x00>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Rectangle_t<TME,0,0,0,0,ZTST,0,ZMSK,0x30,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x301:
			//return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x30,0x01>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Rectangle_t<TME,0,0,0,0,ZTST,0,ZMSK,0x30,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x30a:
			//return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x30,0x0a>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Rectangle_t<TME,0,0,0,0,ZTST,0,ZMSK,0x30,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMZ24
		case 0x310:
			//return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x31,0x00>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Rectangle_t<TME,0,0,0,0,ZTST,0,ZMSK,0x31,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x311:
			//return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x31,0x01>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Rectangle_t<TME,0,0,0,0,ZTST,0,ZMSK,0x31,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x31a:
			//return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x31,0x0a>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Rectangle_t<TME,0,0,0,0,ZTST,0,ZMSK,0x31,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMZ16S
		case 0x3a0:
			//return Render_Generic_Rectangle_t<TME,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x3a,0x00>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Rectangle_t<TME,0,0,0,0,ZTST,0,ZMSK,0x3a,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x3a1:
			return Render_Generic_Rectangle_t<TME,0,0,0,0,ZTST,0,ZMSK,0x3a,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x3aa:
			return Render_Generic_Rectangle_t<TME,0,0,0,0,ZTST,0,ZMSK,0x3a,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		default:
			cout << "\nhps2x64: GPU: Invalid buffer combination. FBUF=" << hex << FBPSM << " ZBUF=" << ZBPSM;
			return 0;
			break;
	}
}


template<const long TME,const long FGE,const long COLCLAMP,const long ABE>
inline static u64 Select_RenderSprite2_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	u32 ATST, ATE, ZTST, ZTE, ZMSK, DATE;
	u32 Combine;
	
//cout << "\n->Select_RenderSprite2_t";
	
	ATE = p_inputbuffer [ 5 ] & 1;
	ZTE = ( p_inputbuffer [ 5 ] >> 16 ) & 1;
	
	ATST = ( p_inputbuffer [ 5 ] >> 1 ) & 7;
	DATE = ( p_inputbuffer [ 5 ] >> 14 ) & 1;
	ZTST = ( p_inputbuffer [ 5 ] >> 17 ) & 3;
	ZMSK = ( p_inputbuffer [ 3 ] >> 32 ) & 1;
	
	//if ( ATST > 1 ) ATST = 2;
	if ( !ATE ) ATST = 1;
	
	if ( ZTST != 1 ) ZTST = 0;
	if ( ATST != 1 ) ATST = 0;
	
	// if z-test is disabled, then ztest always passes and zbuf is never written to
	if ( !ZTE )
	{
		ZTST = 1;
		ZMSK = 1;
	}
	
	// ***TODO*** if Frame buffer format is PSMCT24, then disable destination alpha test (DATE=0)

	//Combine = ( ATST << 4 ) | ( AFAIL << 3 ) | ( ZTST << 2 ) | ( DATE << 1 ) | ZMSK;
	Combine = ( ATST << 3 ) | ( ZTST << 2 ) | ( DATE << 1 ) | ZMSK;
	
	
	switch ( Combine )
	{
		case 0:
			// <TME,FGE,FST,ABE,COLCLAMP,ATST,ZTST,DATE,ZMSK>
			return Select_RenderSprite3_t<TME,FGE,COLCLAMP,ABE,0,0,0,0>( p_inputbuffer, ulThreadNum );
			break;
		case 1:
			return Select_RenderSprite3_t<TME,FGE,COLCLAMP,ABE,0,0,0,1>( p_inputbuffer, ulThreadNum );
			break;
		case 2:
			return Select_RenderSprite3_t<TME,FGE,COLCLAMP,ABE,0,0,1,0>( p_inputbuffer, ulThreadNum );
			break;
		case 3:
			return Select_RenderSprite3_t<TME,FGE,COLCLAMP,ABE,0,0,1,1>( p_inputbuffer, ulThreadNum );
			break;
		case 4:
			return Select_RenderSprite3_t<TME,FGE,COLCLAMP,ABE,0,1,0,0>( p_inputbuffer, ulThreadNum );
			break;
		case 5:
			return Select_RenderSprite3_t<TME,FGE,COLCLAMP,ABE,0,1,0,1>( p_inputbuffer, ulThreadNum );
			break;
		case 6:
			return Select_RenderSprite3_t<TME,FGE,COLCLAMP,ABE,0,1,1,0>( p_inputbuffer, ulThreadNum );
			break;
		case 7:
			return Select_RenderSprite3_t<TME,FGE,COLCLAMP,ABE,0,1,1,1>( p_inputbuffer, ulThreadNum );
			break;
		case 8:
			return Select_RenderSprite3_t<TME,FGE,COLCLAMP,ABE,1,0,0,0>( p_inputbuffer, ulThreadNum );
			break;
		case 9:
			return Select_RenderSprite3_t<TME,FGE,COLCLAMP,ABE,1,0,0,1>( p_inputbuffer, ulThreadNum );
			break;
		case 10:
			return Select_RenderSprite3_t<TME,FGE,COLCLAMP,ABE,1,0,1,0>( p_inputbuffer, ulThreadNum );
			break;
		case 11:
			return Select_RenderSprite3_t<TME,FGE,COLCLAMP,ABE,1,0,1,1>( p_inputbuffer, ulThreadNum );
			break;
		case 12:
			return Select_RenderSprite3_t<TME,FGE,COLCLAMP,ABE,1,1,0,0>( p_inputbuffer, ulThreadNum );
			break;
		case 13:
			return Select_RenderSprite3_t<TME,FGE,COLCLAMP,ABE,1,1,0,1>( p_inputbuffer, ulThreadNum );
			break;
		case 14:
			return Select_RenderSprite3_t<TME,FGE,COLCLAMP,ABE,1,1,1,0>( p_inputbuffer, ulThreadNum );
			break;
		case 15:
			return Select_RenderSprite3_t<TME,FGE,COLCLAMP,ABE,1,1,1,1>( p_inputbuffer, ulThreadNum );
			break;
			
	}
	
}



// TME,FST,FGE,ABE,COLCLAMP,FBA,DATE,ZMSK,ZTST(2),ATST(3)
// template needed: PIXELFORMAT,CLUT_PIXELFORMAT,FBUF_PIXELFORMAT,TEXA_AEM,ZBUF_PIXELFORMAT,TEST_ATE,TEST_ATST,TEST_ZTE,TEST_ZTST,TEST_AFAIL,ZBUF_ZMSK,COLCLAMP,TEX_TFX,TEX_TCC
//template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_ZTST,const long FRAME_PSM,const long ZBUF_PSM,const long TEX_PSM>
//void Select_RenderSprite_t ( u32 Coord0, u32 Coord1 )
static u64 Select_RenderSprite_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	u32 TME, FST, FGE, ABE, COLCLAMP, FBA;	//DATE, ZMSK, ZTST, ATST;
	u32 Combine;
	
				
	// inputbuffer
	// 0: SCISSOR
	// 1: XYOFFSET
	// 2: FRAME
	// 3: ZBUF
	// 4: FBA
	// 5: TEST
	// 6: COLCLAMP
	// 7: ALPHA
	// 8: PABE
	// 9: DTHE
	// 10: DIMX
	// 11: FOGCOL
	// 12: CLAMP
	// 13: TEX0
	// 14: TEXCLUT
	// 31: TEXA
	// -------------
	// 15: PRIM (COMMAND)
	// 16: RGBAQ
	// 17: XYZ
	// 18: UV/ST
	// 19: FOG
	// 20: RGBAQ
	// 21: XYZ
	// 22: UV/ST
	// 23: FOG
	// 24: RGBAQ
	// 25: XYZ
	// 26: UV/ST
	// 27: FOG
				
	TME = ( p_inputbuffer [ 15 ] >> 4 ) & 1;
	FGE = ( p_inputbuffer [ 15 ] >> 5 ) & 1;
	//FST = ( p_inputbuffer [ 15 ] >> 8 ) & 1;
	
	ABE = ( p_inputbuffer [ 15 ] >> 6 ) & 1;
	COLCLAMP = ( p_inputbuffer [ 6 ] ) & 1;
	//FBA = ( p_inputbuffer [ 4 ] ) & 1;
	
	Combine = COLCLAMP | ( FGE << 1 ) | ( ABE << 2 ) | ( TME << 3 );	// | ( FBA << 5 );
	
	switch ( Combine )
	{
		case 0:
		case 1:
		case 2:
		case 3:
			//Select_RenderSprite2_t<TME,FGE,COLCLAMP,ABE> ( p_inputbuffer, ulThreadNum )
			return Select_RenderSprite2_t<0,0,0,0> ( p_inputbuffer, ulThreadNum );
			break;

		case 4:
		case 6:
			return Select_RenderSprite2_t<0,0,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 5:
		case 7:
			return Select_RenderSprite2_t<0,0,1,1> ( p_inputbuffer, ulThreadNum );
			break;
		
		case 8:
			return Select_RenderSprite2_t<1,0,0,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 9:
			return Select_RenderSprite2_t<1,0,1,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 10:
			return Select_RenderSprite2_t<1,1,0,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 11:
			return Select_RenderSprite2_t<1,1,1,0> ( p_inputbuffer, ulThreadNum );
			break;
		
		case 12:
			return Select_RenderSprite2_t<1,0,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 13:
			return Select_RenderSprite2_t<1,0,1,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 14:
			return Select_RenderSprite2_t<1,1,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 15:
			return Select_RenderSprite2_t<1,1,1,1> ( p_inputbuffer, ulThreadNum );
			break;
			
			
	}
}

#endif


//-----------------------------------------------------------------------------------------


#ifdef USE_TEMPLATES_PS2_TRIANGLE

template<const long SHADED,const long TME,const long FST,const long FGE,const long COLCLAMP,const long ABE,const long ATST,const long ZTST,const long DATE,const long ZMSK,const long FBPSM,const long ZBPSM>
static u64 Render_Generic_Triangle_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	// before sse2, was sending 1 pixels at a time
	static const int c_iVectorSize = 1;

	//u32 Pixel, TexelIndex,
	
	//u32 Y1_OnLeft;
	
	u16 *CLUT_LUT;
	
	u32 color_add;
	
	u32 *ptr_texture;
	u32 *ptr_clut;
	u16 *ptr_clut16;
	u32 clut_width, clut_x, clut_y;
	
	u32 *ptr_texture32;
	u16 *ptr_texture16;
	u8 *ptr_texture8;
	
	//s64 denominator;
	
	// not used
	//u32 clut_xoffset, clut_yoffset;
	
	//u16 *ptr;
	
	s64 Temp;
	s64 LeftMostX, RightMostX, TopMostY, BottomMostY;
	
	s32 StartX, EndX, StartY, EndY;

	//s64 Error_Left;
	
	//s64* DitherArray;
	//s64* DitherLine;
	//s64 DitherValue;
	
	u32 uy0, uy1, uy2;
	s32 x0, y0, x1, y1, x2, y2;
	s64 x [ 2 ], dxdy [ 2 ];

	float fS0, fS1, fS2, fT0, fT1, fT2, fQ0, fQ1, fQ2;
	float fS [ 2 ], fT [ 2 ], fQ [ 2 ], dSdy [ 2 ], dTdy [ 2 ], dQdy [ 2 ];
	float iS, iT, iQ, dSdx, dTdx, dQdx;
	
	s64 iU, iV;
	
	s64 iR, iG, iB;
	
	s64 u0, u1, u2, v0, v1, v2;
	s64 u [ 2 ], v [ 2 ];
	s64 dudy [ 2 ], dvdy [ 2 ];
	
	s64 r0, r1, r2, g0, g1, g2, b0, b1, b2, a0, a1, a2;
	u32 bgr0, bgr1, bgr2;
	s64 r [ 2 ], g [ 2 ], b [ 2 ];
	s64 drdy [ 2 ], dgdy [ 2 ], dbdy [ 2 ];
	
	
	s64 dudx, dvdx;
	s64 drdx, dgdx, dbdx;
	
	s64 a [ 2 ];
	s64 dady [ 2 ];
	s64 dadx;
	s64 iA;
	
	u64 bgr, bgr_temp;
	u32 bgr16;
	
	// z coords ?
	s64 z0, z1, z2;
	s64 z [ 2 ], dzdy [ 2 ];
	s64 dzdx;
	s64 iZ;

	// fog coords ?
	s64 f0, f1, f2;
	s64 fogc [ 2 ], dfdy [ 2 ];
	s64 dfdx;
	s64 iF;
	
	u32 c1, c2, c3, ca;
	
	
	u32 NumberOfPixelsDrawn;
	s32 Line, x_across;

	/*
	s32 x_left, x_right;
	s32 R_left, G_left, B_left, A_left;
	s32 U_left, V_left;
	s32 F_left;
	s64 Z_left;
	float fS_left, fT_left, fQ_left;


	s32 dx_left, dx_right;
	s32 dR_left, dG_left, dB_left, dA_left;
	s32 dU_left, dV_left;
	s32 dF_left;
	u64 dZ_left;
	float dfS_left, dfT_left, dfQ_left;
	*/
	
	
	u32 Coord0 = 16 + 0, Coord1 = 16 + 4, Coord2 = 16 + 8;
	u32 X1Index = 0, X0Index = 1;
	
	s64 Red, Green, Blue;


	s32 TexCoordX, TexCoordY;
	//u32 Shift1 = 0, Shift2 = 0, And1 = 0, And2 = 0;
	//u32 Shift0 = 0;
	u32 TextureOffset;
	
	//u32 And3, Shift3, Shift4;
	
	u32 PixelsPerLine;
	
	u32 PixelFormat;
	u32 CLUTBufBase32, CLUTPixelFormat, CLUTStoreMode, CLUTOffset;
	u32 TexBufWidth, TexWidth, TexHeight, TexWidth_Mask, TexHeight_Mask;
	
	s32 TexX_Min, TexX_Max, TexY_Min, TexY_Max;
	u32 TexX_And, TexX_Or, TexY_And, TexY_Or;
	
	u64 NumPixels;
	
	
	s32 Window_YTop, Window_YBottom, Window_XLeft, Window_XRight;
	
	s32 Coord_OffsetX, Coord_OffsetY;
	
	u32 TexBufStartOffset32;
	u32 Clamp_ModeX, Clamp_ModeY;
	s32 Clamp_MinU, Clamp_MaxU, Clamp_MinV, Clamp_MaxV;
	u32 Fst;
	u32 TEX_PSM;
	u32 RGB24_Alpha, Pixel24_Mask, RGB24_TAlpha;
	FloatLong fTemp1, fTemp2;
	//long lTemp1, lTemp2;
	
	u64 TEXA64;
	u32 Combine;
	
	u32 TEX_TFX, TEX_TCC;
	u32 FOGCOL;
	
	u32 *buf32, *zbuf32;
	u32 SetPixelMask, FrameBufferWidthInPixels;
	
	u32 FrameBufferStartOffset32, ZBufferStartOffset32;
	
	u32 DA_Test, AlphaXor32, FrameBuffer_WriteMask32;
	u32 AlphaSelect [ 4 ];
	u32 uA, uB, uC, uD;
	
	AlphaTest at;
	ZTest zt;
	AlphaFail af;
	TexturePixel pd;
	TextureFunc tf;
	
	u32 TEST_ATST;
	u32 TEST_ZTST;
	u32 TEST_AFAIL;
	u32 aref;

	
	Window_YTop = ( p_inputbuffer [ 0 ] >> 32 ) & 0x7ff;
	Window_YBottom = ( p_inputbuffer [ 0 ] >> 48 ) & 0x7ff;
	Window_XLeft = ( p_inputbuffer [ 0 ] ) & 0x7ff;
	Window_XRight = ( p_inputbuffer [ 0 ] >> 16 ) & 0x7ff;
	
	Coord_OffsetX = p_inputbuffer [ 1 ] & 0xffff;
	Coord_OffsetY = ( p_inputbuffer [ 1 ] >> 32 ) & 0xffff;
	
	
	// get y
	//y0 = ( p_inputbuffer [ Coord0 + 1 ] >> 16 ) & 0xffff;
	//y1 = ( p_inputbuffer [ Coord1 + 1 ] >> 16 ) & 0xffff;
	//y2 = ( p_inputbuffer [ Coord2 + 1 ] >> 16 ) & 0xffff;
	uy0 = p_inputbuffer [ Coord0 + 1 ];
	uy1 = p_inputbuffer [ Coord1 + 1 ];
	uy2 = p_inputbuffer [ Coord2 + 1 ];
	
	if ( uy1 < uy0 )
	{
		Swap( uy1, uy0 );
		Swap( Coord1, Coord0 );
	}
	
	if ( uy2 < uy0 )
	{
		Swap( uy2, uy0 );
		Swap( Coord2, Coord0 );
	}
	
	if ( uy2 < uy1 )
	{
		Swap( uy2, uy1 );
		Swap( Coord2, Coord1 );
	}
	
	
	// get x,y
	//x0 = p_inputbuffer [ Coord0 + 1 ] & 0xffff;
	//y0 = ( p_inputbuffer [ Coord0 + 1 ] >> 16 ) & 0xffff;
	//x1 = p_inputbuffer [ Coord1 + 1 ] & 0xffff;
	//y1 = ( p_inputbuffer [ Coord1 + 1 ] >> 16 ) & 0xffff;
	//x2 = p_inputbuffer [ Coord2 + 1 ] & 0xffff;
	//y2 = ( p_inputbuffer [ Coord2 + 1 ] >> 16 ) & 0xffff;
	x0 = uy0 & 0xffff;
	y0 = uy0 >> 16;
	x1 = uy1 & 0xffff;
	y1 = uy1 >> 16;
	x2 = uy2 & 0xffff;
	y2 = uy2 >> 16;

	//////////////////////////////////////////
	// get coordinates on screen
	// *note* this is different from PS1, where you would add the offsets..
	x0 -= Coord_OffsetX;
	y0 -= Coord_OffsetY;
	x1 -= Coord_OffsetX;
	y1 -= Coord_OffsetY;
	x2 -= Coord_OffsetX;
	y2 -= Coord_OffsetY;

	
	
	
	// get the left/right most x
	LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
	LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
	RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
	RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );
	
	LeftMostX += 0xf;
	RightMostX -= 1;
	
	LeftMostX >>= 4;
	RightMostX >>= 4;
	TopMostY = ( y0 + 0xf ) >> 4;
	BottomMostY = ( y2 - 1 ) >> 4;
	

	// check if triangle is within draw area
	//if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	if ( RightMostX <= Window_XLeft || LeftMostX > Window_XRight || BottomMostY <= Window_YTop || TopMostY > Window_YBottom ) return 0;
#if defined INLINE_DEBUG_TRIANGLE_TEST || defined INLINE_DEBUG_PRIMITIVE_TEST
	else
	{
		debug << dec << "\r\nDrawTriangleGradientTexture";
		debug << dec << " FinalCoords: x0=" << ( x0 >> 4 ) << " y0=" << ( y0 >> 4 ) << " x1=" << ( x1 >> 4 ) << " y1=" << ( y1 >> 4 ) << " x2=" << ( x2 >> 4 ) << " y2=" << ( y2 >> 4 );
		debug << hex << " u0=" << uv_temp [ Coord0 ].U << " v0=" << uv_temp [ Coord0 ].V << " u1=" << uv_temp [ Coord1 ].U << " v1=" << uv_temp [ Coord1 ].V << " u2=" << uv_temp [ Coord2 ].U << " v2=" << uv_temp [ Coord2 ].V;
		debug << "; Clamp_ModeX=" << Clamp_ModeX << " Clamp_ModeY=" << Clamp_ModeY;
		debug << hex << "; CLUTBufPtr32/64=" << ( CLUTBufBase32 >> 6 );
		debug << "; CLUTPixFmt=" << PixelFormat_Names [ CLUTPixelFormat ];
		debug << hex << "; CLUTOffset/16=" << CLUTOffset;
		debug << "; CSM=" << CLUTStoreMode;
		debug << "; TEXCLUT=" << hex << GPURegsGp.TEXCLUT.Value;
		debug << hex << "; TexBufPtr32/64=" << ( TexBufStartOffset32 >> 6 );
		debug << dec << "; TexWidth=" << TexWidth << " TexHeight=" << TexHeight;
		debug << "; TexPixFmt=" << PixelFormat_Names [ PixelFormat ];
		debug << hex << "; FrameBufPtr32/64=" << ( FrameBufferStartOffset32 >> 6 );
		debug << " FrameBufPixFmt=" << PixelFormat_Names [ FrameBuffer_PixelFormat ];
		debug << " Alpha=" << Alpha;
		debug << " ZBUF=" << ZBUF_X.Value;
		debug << " MXL=" << TEX1 [ Ctx ].MXL;
		debug << " LCM=" << TEX1 [ Ctx ].LCM;
		debug << " K=" << TEX1 [ Ctx ].K;
	}
#endif

	// check if polygon is too large
	if ( ( RightMostX - LeftMostX ) > c_MaxPolygonWidth ) return 0;
	if ( ( BottomMostY - TopMostY ) > c_MaxPolygonHeight ) return 0;

	// denominator is negative when x1 is on the left, positive when x1 is on the right
	s64 t0 = y1 - y2;
	s64 t1 = y0 - y2;
	s64 denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );
	
	
	NumPixels = _Abs( denominator ) >> ( 8 + 1 );

	if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
	{
		ulInputBuffer_WriteIndex++;
		return NumPixels;
	}
	
	// check if x1 is on left or right //
	if ( denominator > 0 )
	{
		// x1 is on the right //
		X1Index = 1;
		X0Index = 0;
	}

	/*
	denominator = p_inputbuffer [ 0 ];
	
	Coord0 = ((u8*)p_inputbuffer) [ 8 ];
	Coord1 = ((u8*)p_inputbuffer) [ 9 ];
	Coord2 = ((u8*)p_inputbuffer) [ 10 ];
	X1Index = ((u8*)p_inputbuffer) [ 11 ];
	X0Index = X1Index ^ 1;
	
	x0 = ((u32*)p_inputbuffer) [ ( 28 << 1 ) + 0 ];
	y0 = ((u32*)p_inputbuffer) [ ( 28 << 1 ) + 1 ];
	x1 = ((u32*)p_inputbuffer) [ ( 29 << 1 ) + 0 ];
	y1 = ((u32*)p_inputbuffer) [ ( 29 << 1 ) + 1 ];
	x2 = ((u32*)p_inputbuffer) [ ( 30 << 1 ) + 0 ];
	y2 = ((u32*)p_inputbuffer) [ ( 30 << 1 ) + 1 ];
	
	s64 t0 = y1 - y2;
	s64 t1 = y0 - y2;
	*/

	// get z
	z0 = ( p_inputbuffer [ Coord0 + 1 ] >> 32 );
	z1 = ( p_inputbuffer [ Coord1 + 1 ] >> 32 );
	z2 = ( p_inputbuffer [ Coord2 + 1 ] >> 32 );


	SetPixelMask = p_inputbuffer [ 4 ] << 31;
	
	
	FrameBuffer_WriteMask32 = 0xffffffffull & ~( p_inputbuffer [ 2 ] >> 32 );
	
	//FrameBufferStartOffset32 = Frame [ Ctx ].FBP << 11;
	FrameBufferStartOffset32 = ( p_inputbuffer [ 2 ] & 0x1ff ) << 11;
	FrameBufferWidthInPixels = ( ( p_inputbuffer [ 2 ] >> 16 ) & 0x3f ) << 6;

	ZBufferStartOffset32 = ( p_inputbuffer [ 3 ] & 0x1ff ) << 11;
	
	
	buf32 = & ( _GPU->RAM32 [ FrameBufferStartOffset32 ] );
	zbuf32 = & ( _GPU->RAM32 [ ZBufferStartOffset32 ] );
	
	
	if ( !ATST )
	{
	TEST_ATST = ( p_inputbuffer [ 5 ] >> 1 ) & 7;
	TEST_AFAIL = ( p_inputbuffer [ 5 ] >> 12 ) & 3;
	
	aref = ( p_inputbuffer [ 5 ] >> 4 ) & 0xff;
	
	switch ( TEST_ATST )
	{
		case 0:
			at = & Playstation2::GPU::TestSrcAlpha32_t<0>;
			break;
		case 1:
			at = & Playstation2::GPU::TestSrcAlpha32_t<1>;
			break;
		case 2:
			aref = ( aref << 24 );
			at = & Playstation2::GPU::TestSrcAlpha32_t<2>;
			break;
		case 3:
			aref = ( aref << 24 ) | 0xffffff;
			at = & Playstation2::GPU::TestSrcAlpha32_t<3>;
			break;
		case 4:
			//aref = ( (u32) TEST_X.AREF );
			at = & Playstation2::GPU::TestSrcAlpha32_t<4>;
			break;
		case 5:
			aref = ( aref << 24 );
			at = & Playstation2::GPU::TestSrcAlpha32_t<5>;
			break;
		case 6:
			aref = ( aref << 24 ) | 0xffffff;
			at = & Playstation2::GPU::TestSrcAlpha32_t<6>;
			break;
		case 7:
			at = & Playstation2::GPU::TestSrcAlpha32_t<7>;
			break;
	}

	switch ( TEST_AFAIL )
	{
		case 0:
			af = & Playstation2::GPU::PerformAlphaFail32_t<0,FBPSM,ZBPSM,ZMSK>;
			break;
		case 1:
			af = & Playstation2::GPU::PerformAlphaFail32_t<1,FBPSM,ZBPSM,ZMSK>;
			break;
		case 2:
			af = & Playstation2::GPU::PerformAlphaFail32_t<2,FBPSM,ZBPSM,ZMSK>;
			break;
		case 3:
			af = & Playstation2::GPU::PerformAlphaFail32_t<3,FBPSM,ZBPSM,ZMSK>;
			break;
	}

	}	// end if ( !ATST )
	
	
	if ( !ZTST )
	{
	TEST_ZTST = ( p_inputbuffer [ 5 ] >> 17 ) & 3;
	
	switch ( TEST_ZTST )
	{
		case 0:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<0,ZBPSM>;
			break;
		case 1:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<1,ZBPSM>;
			break;
		case 2:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<2,ZBPSM>;
			break;
		case 3:
			zt = & Playstation2::GPU::PerformZDepthTest32_t<3,ZBPSM>;
			break;
	}
	
	}	// end if ( !ZTST )
	
	
	if ( DATE )
	{
	DA_Test = ( p_inputbuffer [ 5 ] << 16 ) & 0x80000000ull;
	}	// end if ( DATE )
	
	
	
//cout << "\nbuf32=" << hex << buf32 << " zbuf32=" << zbuf32;
	
	if ( ABE )
	{
	AlphaXor32 = p_inputbuffer [ 8 ] << 31;

	uA = ( p_inputbuffer [ 7 ] >> 0 ) & 3;
	uB = ( p_inputbuffer [ 7 ] >> 2 ) & 3;
	uC = ( p_inputbuffer [ 7 ] >> 4 ) & 3;
	uD = ( p_inputbuffer [ 7 ] >> 6 ) & 3;
	

	// set fixed alpha values
	
	// current RGBA value
	//AlphaSelect [ 0 ] = rgbaq_Current.Value & 0xffffffffL;
	AlphaSelect [ 0 ] = p_inputbuffer [ 16 + 8 ] & 0xffffffffULL;
	
	// FIX value
	//AlphaSelect [ 2 ] = ALPHA_X.FIX << 24;
	AlphaSelect [ 2 ] = ( p_inputbuffer [ 7 ] >> 8 ) & 0xff000000ULL;
	
	// ZERO
	AlphaSelect [ 3 ] = 0;
	
	// *** testing ***
	//_GPU->AlphaSelect [ 0 ] = p_inputbuffer [ 16 + 8 ] & 0xffffffffULL;
	//_GPU->AlphaSelect [ 2 ] = _GPU->ALPHA_X.FIX << 24;
	//_GPU->AlphaSelect [ 3 ] = 0;
	
	
	}	// end if ( ABE )
	
	
	
	
	// skip drawing if distance between vertices is greater than max allowed by GPU
	//if ( ( _Abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( _Abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
	//{
	//	// skip drawing polygon
	//	return;
	//}
	
	
	
	if ( SHADED )
	{
	// get bgr color value
	//bgr0 = p_inputbuffer [ Coord0 + 0 ] & 0xffffffffULL;
	//bgr1 = p_inputbuffer [ Coord1 + 0 ] & 0xffffffffULL;
	//bgr2 = p_inputbuffer [ Coord2 + 0 ] & 0xffffffffULL;
	bgr0 = p_inputbuffer [ Coord0 + 0 ];
	bgr1 = p_inputbuffer [ Coord1 + 0 ];
	bgr2 = p_inputbuffer [ Coord2 + 0 ];
	
	r0 = ( bgr0 >> 0 ) & 0xff;
	r1 = ( bgr1 >> 0 ) & 0xff;
	r2 = ( bgr2 >> 0 ) & 0xff;
	
	g0 = ( bgr0 >> 8 ) & 0xff;
	g1 = ( bgr1 >> 8 ) & 0xff;
	g2 = ( bgr2 >> 8 ) & 0xff;
	
	b0 = ( bgr0 >> 16 ) & 0xff;
	b1 = ( bgr1 >> 16 ) & 0xff;
	b2 = ( bgr2 >> 16 ) & 0xff;
	
	//a0 = ( bgr0 >> 24 ) & 0xff;
	//a1 = ( bgr1 >> 24 ) & 0xff;
	//a2 = ( bgr2 >> 24 ) & 0xff;
	a0 = ( bgr0 >> 24 );
	a1 = ( bgr1 >> 24 );
	a2 = ( bgr2 >> 24 );
	}
	else
	{
	bgr = p_inputbuffer [ 16 + 8 ] & 0xffffffffULL;
	}
	
	
	
	if ( TME )
	{
		
	if ( !SHADED )
	{
	// get color components (in case not shaded)
	c1 = bgr & 0xff;
	c2 = ( bgr >> 8 ) & 0xff;
	c3 = ( bgr >> 16 ) & 0xff;
	ca = ( bgr >> 24 ) & 0xff;
	}
	
	// color lookup table width is stored in TEXCLUT register (in pixels/64)
	// get clut width and x in pixels
	//clut_width = GPURegsGp.TEXCLUT.CBW << 6;
	clut_width = ( p_inputbuffer [ 14 ] & 0x3f ) << 6;
	
	
	//clut_x = GPURegsGp.TEXCLUT.COU << 6;
	clut_x = p_inputbuffer [ 14 ] & 0xfc0;
	
	// get clut y in units of pixels
	//clut_y = GPURegsGp.TEXCLUT.COV;
	clut_y = ( p_inputbuffer [ 14 ] & 0x3ff000 ) >> 12;
	

	//TEX0_t *TEX0 = TEXX;
	//TEX1_t *TEX1 = &GPURegsGp.TEX1_1;
	
	
	//TexBufWidth = TEX0 [ Ctx ].TBW0 << 6;
	TexBufWidth = ( ( p_inputbuffer [ 13 ] >> 14 ) & 0x3f ) << 6;
	//TexWidth = 1 << TEX0 [ Ctx ].TW;
	TexWidth = 1 << ( ( p_inputbuffer [ 13 ] >> 26 ) & 0xf );
	//TexHeight = 1 << TEX0 [ Ctx ].TH;
	TexHeight = 1 << ( ( p_inputbuffer [ 13 ] >> 30 ) & 0xf );
	TexWidth_Mask = TexWidth - 1;
	TexHeight_Mask = TexHeight - 1;
	
	
	
	
	/////////////////////////////////////////////////////////
	// Get offset into texture page
	//TexBufStartOffset32 = TEX0 [ Ctx ].TBP0 << 6;
	TexBufStartOffset32 = ( p_inputbuffer [ 13 ] & 0x3fff ) << 6;
	
	// get the pixel format (16 bit, 32 bit, etc)
	//PixelFormat = TEX0 [ Ctx ].PSM;
	PixelFormat = ( ( p_inputbuffer [ 13 ] >> 20 ) & 0x3f );
	//CLUTPixelFormat = TEX0 [ Ctx ].CPSM;
	CLUTPixelFormat = ( ( p_inputbuffer [ 13 ] >> 51 ) & 0xf );
	
	// get base pointer to color lookup table (32-bit word address divided by 64)
	//CLUTBufBase32 = TEX0 [ Ctx ].CBP << 6;
	CLUTBufBase32 = ( ( p_inputbuffer [ 13 ] >> 37 ) & 0x3fff ) << 6;
	
	// storage mode ??
	//CLUTStoreMode = TEX0 [ Ctx ].CSM;
	CLUTStoreMode = ( ( p_inputbuffer [ 13 ] >> 55 ) & 0x1 );
	
	// clut offset ??
	// this is the offset / 16
	// note: this is actually the position in the temporary buffer, not in the local memory
	//CLUTOffset = TEX0 [ Ctx ].CSA;
	CLUTOffset = ( ( p_inputbuffer [ 13 ] >> 56 ) & 0x1f );

	
	// *** testing ***
	/*
	if ( PixelFormat == 1 )
	{
		// rgb24 //
		
		// TEXA.TA0 is the alpha of rgb24 value
		_GPU->RGB24_Alpha = _GPU->GPURegsGp.TEXA.TA0 << 24;
		
		// 24-bit pixel only
		_GPU->Pixel24_Mask = 0xffffff;
		
		// r,g,b=0 is transparent for rgb24 when TEXA.AEM is one
		_GPU->RGB24_TAlpha = ( _GPU->GPURegsGp.TEXA.AEM ^ 1 ) << 31;
	}
	else
	{
		// NOT rgb24 //
		_GPU->RGB24_Alpha = 0;
		_GPU->Pixel24_Mask = 0xffffffff;
		_GPU->RGB24_TAlpha = 0;
	}
	*/
	

	TEX_TFX = ( p_inputbuffer [ 13 ] >> 35 ) & 3;
	TEX_TCC = ( p_inputbuffer [ 13 ] >> 34 ) & 1;


	
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << "; TexPixFmt=" << PixelFormat_Names [ PixelFormat ];
#endif

	//tge = command_tge;
	//if ( ( bgr & 0x00ffffff ) == 0x00808080 ) tge = 1;
	
	//////////////////////////////////////////////////////
	// Get offset into color lookup table
	//u32 ClutOffset = ( clut_x << 4 ) + ( clut_y << 10 );
	
	//clut_xoffset = clut_x << 4;
	//clut_xoffset = 0;
	//ptr_clut = & ( VRAM [ clut_y << 10 ] );
	ptr_clut = & ( _GPU->RAM32 [ CLUTBufBase32 ] );
	//ptr_texture = & ( VRAM [ ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) ] );
	ptr_texture = & ( _GPU->RAM32 [ TexBufStartOffset32 ] );
	
	// *** TODO *** use a union
	ptr_texture8 = (u8*) ptr_texture;
	ptr_texture16 = (u16*) ptr_texture;
	ptr_texture32 = (u32*) ptr_texture;
	
	
	
	// texture pixel format variables //
	
	// TEXA64 is TEXA pre-shifted to the left by 24
	//TEXA64 = ( p_inputbuffer [ 31 ] >> 8 ) & 0xff000000ull;
	TEXA64 = p_inputbuffer [ 31 ] << 24;
	
	// get texture drawing function
	pd = Select_TexturePixel_t ( PixelFormat, CLUTPixelFormat, ( TEXA64 >> ( 15 + 24 ) ) & 1 );
	
	if ( !pd ) return 0;
	
	Combine = TEX_TFX | ( TEX_TCC << 2 );
	
	switch ( Combine )
	{
		case 0:
			tf = & TextureFunc32_t<COLCLAMP,0,0>;
			break;
		case 1:
			tf = & TextureFunc32_t<COLCLAMP,1,0>;
			break;
		case 2:
			tf = & TextureFunc32_t<COLCLAMP,2,0>;
			break;
		case 3:
			tf = & TextureFunc32_t<COLCLAMP,3,0>;
			break;
		case 4:
			tf = & TextureFunc32_t<COLCLAMP,0,1>;
			break;
		case 5:
			tf = & TextureFunc32_t<COLCLAMP,1,1>;
			break;
		case 6:
			tf = & TextureFunc32_t<COLCLAMP,2,1>;
			break;
		case 7:
			tf = & TextureFunc32_t<COLCLAMP,3,1>;
			break;
	}
	
	

	
	if ( !CLUTStoreMode )
	{
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << " CSM1";
#endif

		// CSM1 //
		//CLUT_LUT = ucCLUT_Lookup_CSM01_4bpp;
		
		if ( CLUTPixelFormat & 0x2 )
		{
			// 4-bit pixels - 16 colors
			// 16-bit pixels in CLUT //
			CLUTOffset &= 0x1f;
		}
		else
		{
			// 32-bit pixels in CLUT //
			CLUTOffset &= 0xf;
		}
		
	}
	else
	{
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << " CSM2";
#endif

		// CSM2 //
		//CLUT_LUT = ucCLUT_Lookup_CSM02;
		
		// use clut_x, clut_y, clut_width
		//ptr_clut = & ( ptr_clut [ ( clut_x >> Shift3 ) + ( clut_y * ( clut_width >> Shift3 ) ) ] );
	}
	
	
	CLUTOffset <<= 4;
	ptr_clut16 = & ( _GPU->InternalCLUT [ CLUTOffset ] );
	


	if ( FGE )
	{
	FOGCOL = p_inputbuffer [ 11 ];
	
	// get fog coefficient
	//f0 = (u64) f [ Coord0 ].F;
	//f1 = (u64) f [ Coord1 ].F;
	f0 = ( p_inputbuffer [ Coord0 + 3 ] >> 56 ) & 0xff;
	f1 = ( p_inputbuffer [ Coord1 + 3 ] >> 56 ) & 0xff;
	f2 = ( p_inputbuffer [ Coord2 + 3 ] >> 56 ) & 0xff;
	}
	
	//Fst = p_inputbuffer [ 15 ] & 0x100;
	
	// check if sprite should use UV coords or ST coords
	// on PS2, sprite can use the ST register to specify texture coords
	//if ( GPURegsGp.PRIM.FST )
	if ( FST )
	{
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << " FST";
#endif

		// get u,v
		//u0 = uv [ Coord0 ].U;
		//u1 = uv [ Coord1 ].U;
		//v0 = uv [ Coord0 ].V;
		//v1 = uv [ Coord1 ].V;
		u0 = p_inputbuffer [ Coord0 + 2 ] & 0x3fff;
		v0 = ( p_inputbuffer [ Coord0 + 2 ] >> 16 ) & 0x3fff;
		u1 = p_inputbuffer [ Coord1 + 2 ] & 0x3fff;
		v1 = ( p_inputbuffer [ Coord1 + 2 ] >> 16 ) & 0x3fff;
		u2 = p_inputbuffer [ Coord2 + 2 ] & 0x3fff;
		v2 = ( p_inputbuffer [ Coord2 + 2 ] >> 16 ) & 0x3fff;
	}
	else
	{
#if defined INLINE_DEBUG_SPRITE || defined INLINE_DEBUG_PRIMITIVE
	debug << " !FST";
#endif
		
		// put s,t coords into 10.4 fixed point
		// note: tex width/height should probably be minus one
		//u0 = (s32) ( st [ Coord0 ].fS * ( (float) ( TexWidth ) ) * 16.0f );
		//u1 = (s32) ( st [ Coord1 ].fS * ( (float) ( TexWidth ) ) * 16.0f );
		//v0 = (s32) ( st [ Coord0 ].fT * ( (float) ( TexHeight ) ) * 16.0f );
		//v1 = (s32) ( st [ Coord1 ].fT * ( (float) ( TexHeight ) ) * 16.0f );
		
		fTemp1.l = p_inputbuffer [ Coord0 + 2 ] & 0xffffffffull;
		fTemp2.l = p_inputbuffer [ Coord0 + 2 ] >> 32;
		fS0 = fTemp1.f;
		fT0 = fTemp2.f;
		fTemp1.l = p_inputbuffer [ Coord1 + 2 ] & 0xffffffffull;
		fTemp2.l = p_inputbuffer [ Coord1 + 2 ] >> 32;
		fS1 = fTemp1.f;
		fT1 = fTemp2.f;
		fTemp1.l = p_inputbuffer [ Coord2 + 2 ] & 0xffffffffull;
		fTemp2.l = p_inputbuffer [ Coord2 + 2 ] >> 32;
		fS2 = fTemp1.f;
		fT2 = fTemp2.f;
		
		fTemp1.l = p_inputbuffer [ Coord0 + 0 ] >> 32;
		fQ0 = fTemp1.f;
		fTemp1.l = p_inputbuffer [ Coord1 + 0 ] >> 32;
		fQ1 = fTemp1.f;
		fTemp1.l = p_inputbuffer [ Coord2 + 0 ] >> 32;
		fQ2 = fTemp1.f;
		
		fS0 *= TexWidth;
		fS1 *= TexWidth;
		fS2 *= TexWidth;
		fT0 *= TexHeight;
		fT1 *= TexHeight;
		fT2 *= TexHeight;
	}
	
	
	
	Clamp_ModeX = p_inputbuffer [ 12 ] & 0x3;
	Clamp_ModeY = ( p_inputbuffer [ 12 ] >> 2 ) & 0x3;
	
	Clamp_MinU = ( p_inputbuffer [ 12 ] >> 4 ) & 0x3ff;
	Clamp_MaxU = ( p_inputbuffer [ 12 ] >> 14 ) & 0x3ff;
	Clamp_MinV = ( p_inputbuffer [ 12 ] >> 24 ) & 0x3ff;
	Clamp_MaxV = ( p_inputbuffer [ 12 ] >> 34 ) & 0x3ff;
	
	switch ( Clamp_ModeY )
	{
		case 0:
			// repeat //
			//TexCoordY &= TexHeight_Mask;
			TexY_And = TexHeight_Mask;
			
			TexY_Or = 0;
			
			// can only have coords in range -2047 to +2047
			TexY_Min = -2047;
			TexY_Max = 2047;
			break;
			
		case 1:
			// clamp //
			//TexCoordY = ( TexCoordY < 0 ) ? 0 : TexCoordY;
			//TexCoordY = ( TexCoordY > TexHeight ) ? TexHeight : TexCoordY;
			TexY_Min = 0;
			TexY_Max = TexHeight_Mask;
			
			TexY_And = TexHeight_Mask;
			TexY_Or = 0;
			break;
			
		case 2:
			// region clamp //
			//TexCoordY = ( TexCoordY < Clamp_MinV ) ? Clamp_MinV : TexCoordY;
			//TexCoordY = ( TexCoordY > Clamp_MaxV ) ? Clamp_MaxV : TexCoordY;
			TexY_Min = Clamp_MinV;
			TexY_Max = Clamp_MaxV;
			
			TexY_And = TexHeight_Mask;
			TexY_Or = 0;
			break;
			
		case 3:
			// region repeat //
			// this one is just like on the ps1
			//TexCoordY = ( TexCoordY & Clamp_MinV ) | Clamp_MaxV;
			TexY_And = Clamp_MinV & TexHeight_Mask;
			TexY_Or = Clamp_MaxV & TexHeight_Mask;
			
			// can only have coords in range -2047 to +2047
			TexY_Min = -2047;
			TexY_Max = 2047;
			break;
	}

	
	switch ( Clamp_ModeX )
	{
		case 0:
			// repeat //
			//TexCoordY &= TexHeight_Mask;
			TexX_And = TexWidth_Mask;
			
			TexX_Or = 0;
			
			// can only have coords in range -2047 to +2047
			TexX_Min = -2047;
			TexX_Max = 2047;
			break;
			
		case 1:
			// clamp //
			//TexCoordY = ( TexCoordY < 0 ) ? 0 : TexCoordY;
			//TexCoordY = ( TexCoordY > TexHeight ) ? TexHeight : TexCoordY;
			TexX_Min = 0;
			TexX_Max = TexWidth_Mask;
			
			TexX_And = TexWidth_Mask;
			TexX_Or = 0;
			break;
			
		case 2:
			// region clamp //
			//TexCoordY = ( TexCoordY < Clamp_MinV ) ? Clamp_MinV : TexCoordY;
			//TexCoordY = ( TexCoordY > Clamp_MaxV ) ? Clamp_MaxV : TexCoordY;
			TexX_Min = Clamp_MinU;
			TexX_Max = Clamp_MaxU;
			
			TexX_And = TexWidth_Mask;
			TexX_Or = 0;
			break;
			
		case 3:
			// region repeat //
			// this one is just like on the ps1
			//TexCoordY = ( TexCoordY & Clamp_MinV ) | Clamp_MaxV;
			TexX_And = Clamp_MinU & TexWidth_Mask;
			TexX_Or = Clamp_MaxU & TexWidth_Mask;
			
			// can only have coords in range -2047 to +2047
			TexX_Min = -2047;
			TexX_Max = 2047;
			break;
	}

	}	// end if ( TME )

		
#if defined INLINE_DEBUG_TRIANGLE || defined INLINE_DEBUG_PRIMITIVE
	debug << hex << " bgr=" << bgr;
	debug << dec << "; Coords: x0=" << ( x0 >> 4 ) << " y0=" << ( y0 >> 4 ) << " x1=" << ( x1 >> 4 ) << " y1=" << ( y1 >> 4 ) << " x2=" << ( x2 >> 4 ) << " y2=" << ( y2 >> 4 );
	debug << hex << " u0=" << uv_temp [ Coord0 ].U << " v0=" << uv_temp [ Coord0 ].V << " u1=" << uv_temp [ Coord1 ].U << " v1=" << uv_temp [ Coord1 ].V << " u2=" << uv_temp [ Coord2 ].U << " v2=" << uv_temp [ Coord2 ].V;
#endif

	
	
	
	
	/////////////////////////////////////////////////
	// draw top part of triangle
	
	
	
	// calculate across
	if ( denominator )
	{
		dzdx = ( ( ( ( (s64) ( z0 - z2 ) ) * t0 ) - ( ( (s64) ( z1 - z2 ) ) * t1 ) ) << 20 ) / denominator;
		
		if ( TME )
		{
			
		if ( FST )
		{
		//dudx = ( ( ( ( (s64) ( uv [ Coord0 ].U - uv [ Coord2 ].U ) ) * t0 ) - ( ( (s64) ( uv [ Coord1 ].U - uv [ Coord2 ].U ) ) * t1 ) ) << 12 ) / denominator;
		//dvdx = ( ( ( ( (s64) ( uv [ Coord0 ].V - uv [ Coord2 ].V ) ) * t0 ) - ( ( (s64) ( uv [ Coord1 ].V - uv [ Coord2 ].V ) ) * t1 ) ) << 12 ) / denominator;
		dudx = ( ( ( ( (s64) ( u0 - u2 ) ) * t0 ) - ( ( (s64) ( u1 - u2 ) ) * t1 ) ) << 16 ) / denominator;
		dvdx = ( ( ( ( (s64) ( v0 - v2 ) ) * t0 ) - ( ( (s64) ( v1 - v2 ) ) * t1 ) ) << 16 ) / denominator;
		}
		else
		{
		dSdx = ( ( ( fS0 - fS2 ) * ( t0 / 16.0f ) ) - ( ( fS1 - fS2 ) * ( t1 / 16.0f ) ) ) / ( denominator / 256.0f );
		dTdx = ( ( ( fT0 - fT2 ) * ( t0 / 16.0f ) ) - ( ( fT1 - fT2 ) * ( t1 / 16.0f ) ) ) / ( denominator / 256.0f );
		dQdx = ( ( ( fQ0 - fQ2 ) * ( t0 / 16.0f ) ) - ( ( fQ1 - fQ2 ) * ( t1 / 16.0f ) ) ) / ( denominator / 256.0f );
		}
		
		if ( FGE )
		{
		dfdx = ( ( ( ( (s64) ( f0 - f2 ) ) * t0 ) - ( ( (s64) ( f1 - f2 ) ) * t1 ) ) << 20 ) / denominator;
		}
		
		}	// end if ( TME )
	

		if ( SHADED )
		{
		drdx = ( ( ( ( (s64) ( r0 - r2 ) ) * t0 ) - ( ( (s64) ( r1 - r2 ) ) * t1 ) ) << 20 ) / denominator;
		dgdx = ( ( ( ( (s64) ( g0 - g2 ) ) * t0 ) - ( ( (s64) ( g1 - g2 ) ) * t1 ) ) << 20 ) / denominator;
		dbdx = ( ( ( ( (s64) ( b0 - b2 ) ) * t0 ) - ( ( (s64) ( b1 - b2 ) ) * t1 ) ) << 20 ) / denominator;
		dadx = ( ( ( ( (s64) ( a0 - a2 ) ) * t0 ) - ( ( (s64) ( a1 - a2 ) ) * t1 ) ) << 20 ) / denominator;
		}
	}
	
	//debug << dec << "\r\nx0=" << x0 << " y0=" << y0 << " x1=" << x1 << " y1=" << y1 << " x2=" << x2 << " y2=" << y2;
	//debug << "\r\nfixed: denominator=" << denominator;



	//StartY = y0;
	//EndY = y1;

	// left point is included if points are equal
	StartY = ( y0 + 0xf ) >> 4;
	EndY = ( y1 - 1 ) >> 4;


	
	/////////////////////////////////////////////////
	// draw top part of triangle
	
	
	
	// need to set the x0 index unconditionally
	//x [ X0Index ] = ( ((s64)x0) << 32 );
	x [ X0Index ] = ( ((s64)x0) << 12 );
	
	z [ X0Index ] = ( ((s64)z0) << 16 );
	
	// offset
	z [ X0Index ] += 0x8000;
	
	if ( TME )
	{
		
	if ( FST )
	{
	//u [ X0Index ] = ( uv [ Coord0 ].U << 12 );
	//v [ X0Index ] = ( uv [ Coord0 ].V << 12 );
	u [ X0Index ] = ( u0 << 12 );
	v [ X0Index ] = ( v0 << 12 );
	}
	else
	{
	fS [ X0Index ] = fS0;
	fT [ X0Index ] = fT0;
	fQ [ X0Index ] = fQ0;
	}
	
	if ( FGE )
	{
	fogc [ X0Index ] = ( ((s64)f0) << 16 );
	
	// offset
	fogc [ X0Index ] += 0x8000;
	}
	
	}
	
	if ( SHADED )
	{
	r [ X0Index ] = ( r0 << 16 );
	g [ X0Index ] = ( g0 << 16 );
	b [ X0Index ] = ( b0 << 16 );
	a [ X0Index ] = ( a0 << 16 );
	
	// offset
	r [ X0Index ] += 0x8000;
	g [ X0Index ] += 0x8000;
	b [ X0Index ] += 0x8000;
	a [ X0Index ] += 0x8000;
	}
	
	if ( y1 - y0 )
	{
		// triangle is pointed on top //
		
		/////////////////////////////////////////////
		// init x on the left and right
		//x_left = ( ((s64)x0) << 32 );
		//x_right = x_left;
		x [ X1Index ] = x [ X0Index ];
		
		z [ X1Index ] = z [ X0Index ];
		
		if ( TME )
		{
			
		if ( FST )
		{
		u [ X1Index ] = u [ X0Index ];
		v [ X1Index ] = v [ X0Index ];
		}
		else
		{
		fS [ X1Index ] = fS [ X0Index ];
		fT [ X1Index ] = fT [ X0Index ];
		fQ [ X1Index ] = fQ [ X0Index ];
		}
		
		if ( FGE )
		{
		fogc [ X1Index ] = fogc [ X0Index ];
		}
		
		}
		
		if ( SHADED )
		{
		r [ X1Index ] = r [ X0Index ];
		g [ X1Index ] = g [ X0Index ];
		b [ X1Index ] = b [ X0Index ];
		a [ X1Index ] = a [ X0Index ];
		}
		
		//dx_left = (((s64)( x1 - x0 )) << 32 ) / ((s64)( y1 - y0 ));
		//dx_right = (((s64)( x2 - x0 )) << 32 ) / ((s64)( y2 - y0 ));
		dxdy [ X1Index ] = (((s64)( x1 - x0 )) << 16 ) / ((s64)( y1 - y0 ));
		dxdy [ X0Index ] = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
		
		dzdy [ X1Index ] = (((s64)( z1 - z0 )) << 20 ) / ((s64)( y1 - y0 ));
		
		if ( TME )
		{
			
		if ( FST )
		{
		//dudy [ X1Index ] = (((s32)( uv [ Coord1 ].U - uv [ Coord0 ].U )) << 12 ) / ((s32)( y1 - y0 ));
		//dvdy [ X1Index ] = (((s32)( uv [ Coord1 ].V - uv [ Coord0 ].V )) << 12 ) / ((s32)( y1 - y0 ));
		dudy [ X1Index ] = (((s64)( u1 - u0 )) << 16 ) / ((s64)( y1 - y0 ));
		dvdy [ X1Index ] = (((s64)( v1 - v0 )) << 16 ) / ((s64)( y1 - y0 ));
		}
		else
		{
		dSdy [ X1Index ] = ( fS1 - fS0 ) / ( ( y1 - y0 ) / 16.0f );
		dTdy [ X1Index ] = ( fT1 - fT0 ) / ( ( y1 - y0 ) / 16.0f );
		dQdy [ X1Index ] = ( fQ1 - fQ0 ) / ( ( y1 - y0 ) / 16.0f );
		}
		
		if ( FGE )
		{
		dfdy [ X1Index ] = (((s64)( f1 - f0 )) << 20 ) / ((s64)( y1 - y0 ));
		}
		
		}
		
		if ( SHADED )
		{
		drdy [ X1Index ] = (((s64)( r1 - r0 )) << 20 ) / ((s64)( y1 - y0 ));
		dgdy [ X1Index ] = (((s64)( g1 - g0 )) << 20 ) / ((s64)( y1 - y0 ));
		dbdy [ X1Index ] = (((s64)( b1 - b0 )) << 20 ) / ((s64)( y1 - y0 ));
		dady [ X1Index ] = (((s64)( a1 - a0 )) << 20 ) / ((s64)( y1 - y0 ));
		}
		
		
		dzdy [ X0Index ] = (((s64)( z2 - z0 )) << 20 ) / ((s64)( y2 - y0 ));
		
		if ( TME )
		{
			
		if ( FST )
		{
		//dudy [ X0Index ] = (((s32)( uv [ Coord2 ].U - uv [ Coord0 ].U )) << 12 ) / ((s32)( y2 - y0 ));
		//dvdy [ X0Index ] = (((s32)( uv [ Coord2 ].V - uv [ Coord0 ].V )) << 12 ) / ((s32)( y2 - y0 ));
		dudy [ X0Index ] = (((s64)( u2 - u0 )) << 16 ) / ((s64)( y2 - y0 ));
		dvdy [ X0Index ] = (((s64)( v2 - v0 )) << 16 ) / ((s64)( y2 - y0 ));
		}
		else
		{
		dSdy [ X0Index ] = ( fS2 - fS0 ) / ( ( y2 - y0 ) / 16.0f );
		dTdy [ X0Index ] = ( fT2 - fT0 ) / ( ( y2 - y0 ) / 16.0f );
		dQdy [ X0Index ] = ( fQ2 - fQ0 ) / ( ( y2 - y0 ) / 16.0f );
		}
		
		if ( FGE )
		{
		dfdy [ X0Index ] = (((s64)( f2 - f0 )) << 20 ) / ((s64)( y2 - y0 ));
		}
		
		}
		
		if ( SHADED )
		{
		drdy [ X0Index ] = (((s64)( r2 - r0 )) << 20 ) / ((s64)( y2 - y0 ));
		dgdy [ X0Index ] = (((s64)( g2 - g0 )) << 20 ) / ((s64)( y2 - y0 ));
		dbdy [ X0Index ] = (((s64)( b2 - b0 )) << 20 ) / ((s64)( y2 - y0 ));
		dady [ X0Index ] = (((s64)( a2 - a0 )) << 20 ) / ((s64)( y2 - y0 ));
		}
	}
	else
	{
		// Triangle is flat on top //
		
		// change x_left and x_right where y1 is on left
		//x [ X1Index ] = ( ((s64)x1) << 32 );
		x [ X1Index ] = ( ((s64)x1) << 12 );
		
		z [ X1Index ] = ( ((s64)z1) << 16 );
		
		// offset
		z [ X1Index ] += 0x8000;
		
		if ( TME )
		{
			
		if ( FST )
		{
		//u [ X1Index ] = ( uv [ Coord1 ].U << 12 );
		//v [ X1Index ] = ( uv [ Coord1 ].V << 12 );
		u [ X1Index ] = ( u1 << 12 );
		v [ X1Index ] = ( v1 << 12 );
		}
		else
		{
		fS [ X1Index ] = fS1;
		fT [ X1Index ] = fT1;
		fQ [ X1Index ] = fQ1;
		}
		
		if ( FGE )
		{
		fogc [ X1Index ] = ( ((s64)f1) << 16 );
		
		// offset
		fogc [ X1Index ] += 0x8000;
		}
		
		}
		
		if ( SHADED )
		{
		r [ X1Index ] = ( r1 << 16 );
		g [ X1Index ] = ( g1 << 16 );
		b [ X1Index ] = ( b1 << 16 );
		a [ X1Index ] = ( a1 << 16 );
		
		r [ X1Index ] += 0x8000;
		g [ X1Index ] += 0x8000;
		b [ X1Index ] += 0x8000;
		a [ X1Index ] += 0x8000;
		}
		
		// this means that x0 and x1 are on the same line
		// so if the height of the entire polygon is zero, then we are done
		if ( y2 - y1 )
		{
			//dx_left = (((s64)( x2 - x1 )) << 32 ) / ((s64)( y2 - y1 ));
			//dx_right = (((s64)( x2 - x0 )) << 32 ) / ((s64)( y2 - y0 ));
			dxdy [ X1Index ] = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dxdy [ X0Index ] = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			
			dzdy [ X0Index ] = (((s64)( z2 - z0 )) << 20 ) / ((s64)( y2 - y0 ));
			
			if ( TME )
			{
				
			if ( FST )
			{
			//dudy [ X0Index ] = (((s32)( uv [ Coord2 ].U - uv [ Coord0 ].U )) << 12 ) / ((s32)( y2 - y0 ));
			//dvdy [ X0Index ] = (((s32)( uv [ Coord2 ].V - uv [ Coord0 ].V )) << 12 ) / ((s32)( y2 - y0 ));
			dudy [ X0Index ] = (((s32)( u2 - u0 )) << 16 ) / ((s32)( y2 - y0 ));
			dvdy [ X0Index ] = (((s32)( v2 - v0 )) << 16 ) / ((s32)( y2 - y0 ));
			}
			else
			{
			dSdy [ X0Index ] = ( fS2 - fS0 ) / ( ( y2 - y0 ) / 16.0f );
			dTdy [ X0Index ] = ( fT2 - fT0 ) / ( ( y2 - y0 ) / 16.0f );
			dQdy [ X0Index ] = ( fQ2 - fQ0 ) / ( ( y2 - y0 ) / 16.0f );
			}
			
			if ( FGE )
			{
			dfdy [ X0Index ] = (((s64)( f2 - f0 )) << 20 ) / ((s64)( y2 - y0 ));
			}
			
			}
			
			if ( SHADED )
			{
			// only need to set dr,dg,db for the x0/x2 side here
			drdy [ X0Index ] = (((s64)( r2 - r0 )) << 20 ) / ((s64)( y2 - y0 ));
			dgdy [ X0Index ] = (((s64)( g2 - g0 )) << 20 ) / ((s64)( y2 - y0 ));
			dbdy [ X0Index ] = (((s64)( b2 - b0 )) << 20 ) / ((s64)( y2 - y0 ));
			dady [ X0Index ] = (((s64)( a2 - a0 )) << 20 ) / ((s64)( y2 - y0 ));
			}
		}
	}
	
	
	
	Temp = ( StartY << 4 ) - y0;
	
	
	

	if ( StartY < Window_YTop )
	{
		if ( EndY < Window_YTop )
		{
			//Temp = EndY - StartY;
			Temp += ( EndY - StartY + 1 ) << 4;
			//StartY = EndY;
			StartY = EndY + 1;
		}
		else
		{
			Temp += ( Window_YTop - StartY ) << 4;
			StartY = Window_YTop;
		}
	}

	

	x [ 0 ] += ( dxdy [ 0 ] >> 4 ) * Temp;
	x [ 1 ] += ( dxdy [ 1 ] >> 4 ) * Temp;
	
	z [ 0 ] += ( dzdy [ 0 ] >> 4 ) * Temp;
	
	if ( TME )
	{
		
	if ( FST )
	{
	u [ 0 ] += ( dudy [ 0 ] >> 4 ) * Temp;
	v [ 0 ] += ( dvdy [ 0 ] >> 4 ) * Temp;
	}
	else
	{
	fS [ 0 ] += ( dSdy [ 0 ] ) * ( Temp / 16.0f );
	fT [ 0 ] += ( dTdy [ 0 ] ) * ( Temp / 16.0f );
	fQ [ 0 ] += ( dQdy [ 0 ] ) * ( Temp / 16.0f );
	}
	
	if ( FGE )
	{
	fogc [ 0 ] += ( dfdy [ 0 ] >> 4 ) * Temp;
	}
	
	}
	
	if ( SHADED )
	{
	r [ 0 ] += ( drdy [ 0 ] >> 4 ) * Temp;
	g [ 0 ] += ( dgdy [ 0 ] >> 4 ) * Temp;
	b [ 0 ] += ( dbdy [ 0 ] >> 4 ) * Temp;
	a [ 0 ] += ( dady [ 0 ] >> 4 ) * Temp;
	}
	
	
	if ( EndY > Window_YBottom )
	{
		//EndY = Window_YBottom + 1;
		EndY = Window_YBottom;
	}
		
	
	if ( EndY >= StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y1
	for ( Line = StartY; Line <= EndY; Line++ )
	{
		// left point is included if points are equal
		StartX = ( x [ 0 ] + 0xffffLL ) >> 16;
		EndX = ( x [ 1 ] - 1 ) >> 16;
		//StartX = ( x_left + 0xffffLL ) >> 16;
		//EndX = ( x_right - 1 ) >> 16;
		

		if ( StartX <= Window_XRight && EndX >= Window_XLeft && EndX >= StartX )
		{
			iZ = z [ 0 ];
			
			if ( TME )
			{
				
			if ( FST )
			{
			iU = u [ 0 ];
			iV = v [ 0 ];
			}
			else
			{
			iS = fS [ 0 ];
			iT = fT [ 0 ];
			iQ = fQ [ 0 ];
			}
			
			if ( FGE )
			{
			iF = fogc [ 0 ];
			}
			
			}

			
			if ( SHADED )
			{
			iR = r [ 0 ];
			iG = g [ 0 ];
			iB = b [ 0 ];
			iA = a [ 0 ];
			}
			
			
			
			//Temp = ( StartX << 16 ) - x_left;
			Temp = ( StartX << 16 ) - x [ 0 ];
			
			if ( StartX < Window_XLeft )
			{
				Temp += ( Window_XLeft - StartX ) << 16;
				StartX = Window_XLeft;
			}
			
			iZ += ( dzdx >> 8 ) * ( Temp >> 8 );
			
			if ( TME )
			{
				
			if ( FST )
			{
			iU += ( dudx >> 8 ) * ( Temp >> 8 );
			iV += ( dvdx >> 8 ) * ( Temp >> 8 );
			}
			else
			{
			iS += ( dSdx ) * ( Temp / 65536.0f );
			iT += ( dTdx ) * ( Temp / 65536.0f );
			iQ += ( dQdx ) * ( Temp / 65536.0f );
			}
			
			if ( FGE )
			{
			iF += ( dfdx >> 8 ) * ( Temp >> 8 );
			}
			
			}
			
			if ( SHADED )
			{
			iR += ( drdx >> 8 ) * ( Temp >> 8 );
			iG += ( dgdx >> 8 ) * ( Temp >> 8 );
			iB += ( dbdx >> 8 ) * ( Temp >> 8 );
			iA += ( dadx >> 8 ) * ( Temp >> 8 );
			}
			
			
			if ( EndX > Window_XRight )
			{
				//EndX = Window_XRight + 1;
				EndX = Window_XRight;
			}
			
			
			
			
			
#ifdef DEBUG_TEST
	debug << "\r\n";
#endif

//#define INLINE_DEBUG_SHOW_RASTER
#ifdef INLINE_DEBUG_SHOW_RASTER
	debug << "\r\n";
	debug << "top y=" << dec << Line << " sx=" << StartX << " ex=" << EndX << hex << " dadx=" << dadx << " dady=" << dady [ 0 ];
	debug << hex << " S=" << iS << " T=" << iT << " Q=" << iQ << " u=" << iU << " v=" << iV << " f=" << iF << " r=" << iR << " g=" << iG << " b=" << iB << " a=" << iA;
#endif


			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across < EndX; x_across++ )
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
				if ( TME )
				{
					
				if ( FST )
				{
				TexCoordX = ( iU >> 16 ) /* & TexWidth_Mask */;
				TexCoordY = ( iV >> 16 ) /* & TexHeight_Mask */;
				}
				else
				{
					TexCoordX = (long) (iS / iQ);
					TexCoordY = (long) (iT / iQ);
				}
				
				TexCoordY = ( ( TexCoordY < TexY_Min ) ? TexY_Min : TexCoordY );
				TexCoordY = ( ( TexCoordY > TexY_Max ) ? TexY_Max : TexCoordY );
				TexCoordY &= TexY_And;
				TexCoordY |= TexY_Or;

				TexCoordX = ( ( TexCoordX < TexX_Min ) ? TexX_Min : TexCoordX );
				TexCoordX = ( ( TexCoordX > TexX_Max ) ? TexX_Max : TexCoordX );
				TexCoordX &= TexX_And;
				TexCoordX |= TexX_Or;

//if ( Clamp_ModeX == 1 && TexWidth == 64 )
//{
//	debug << "\r\nTexCoordX=" << dec << TexCoordX;
//	debug << " TexCoordY=" << dec << TexCoordY;
//}
				


				// get texel from texture buffer (32-bit frame buffer) //
				bgr = pd ( ptr_texture32, TexCoordX, TexCoordY, TexBufWidth, ptr_clut16, TEXA64 );
				



				// texture function ??
				
				
				// fog function ??
				
				//if ( bgr )
				//if ( bgr != -1ULL )
				//{
					
					if ( SHADED )
					{
					bgr = tf ( bgr, iR >> 16, iG >> 16, iB >> 16, iA >> 16 );
					//bgr = _GPU->TextureFunc32 ( bgr, iR >> 16, iG >> 16, iB >> 16, iA >> 16 );
					}
					else
					{
					bgr = tf ( bgr, c1, c2, c3, ca );
					//bgr = _GPU->TextureFunc32 ( bgr, c1, c2, c3, ca );
					}

					if ( FGE )
					{
						//bgr = FogFunc32_t<COLCLAMP> ( bgr, iF, FOGCOL );
						bgr = FogFunc32_t<COLCLAMP> ( bgr, iF >> 16, FOGCOL );
						//bgr = _GPU->FogFunc32 ( bgr, iF >> 16 );
					}


					
					// no need to interpolate Z-value for sprite? just use z1 value?
					//PlotPixel_Texture ( x_across, Line, iZ >> 23, bgr );
					//PlotPixel_Texture ( x_across, Line, iZ >> 23, bgr, c1, c2, c3, ca, iF >> 24 );
					//PlotPixel_Texture_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,ZBUF_PSM,FRAME_PSM> ( x_across, Line, z1, bgr, c1, c2, c3, ca, iF >> 24 );
					PlotPixel_Gradient_t<0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( buf32, zbuf32, x_across, Line, iZ >> 16, bgr, SetPixelMask, FrameBufferWidthInPixels, DA_Test, AlphaXor32, FrameBuffer_WriteMask32, aref, at, zt, af, AlphaSelect, uA, uB, uC, uD );
					
					//_GPU->PlotPixel_Gradient ( x_across, Line, iZ >> 16, bgr );
					
				//}	// end if ( bgr )
				
				}
				else
				{
					if ( SHADED )
					{
						bgr = ( iR >> 16 ) | ( ( iG >> 16 ) << 8 ) | ( ( iB >> 16 ) << 16 ) | ( ( iA >> 16 ) << 24 );
					}
					
					PlotPixel_Gradient_t<0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( buf32, zbuf32, x_across, Line, iZ >> 16, bgr, SetPixelMask, FrameBufferWidthInPixels, DA_Test, AlphaXor32, FrameBuffer_WriteMask32, aref, at, zt, af, AlphaSelect, uA, uB, uC, uD );
					
				}	// end if ( TME )

					
				iZ += dzdx;
						
				if ( TME )
				{

				if ( FST )
				{
				iU += dudx;
				iV += dvdx;
				}
				else
				{
				iS += dSdx;
				iT += dTdx;
				iQ += dQdx;
				}
				
				if ( FGE )
				{
				iF += dfdx;
				}
				
				}	// end if ( TME )
				
				if ( SHADED )
				{
				iR += drdx;
				iG += dgdx;
				iB += dbdx;
				iA += dadx;
				}
				
			}
			
		}
		
		//////////////////////////////////
		// draw next line
		//Line++;
		
		/////////////////////////////////////
		// update x on left and right
		//x_left += dx_left;
		//x_right += dx_right;
		x [ 0 ] += dxdy [ 0 ];
		x [ 1 ] += dxdy [ 1 ];
		
		z [ 0 ] += dzdy [ 0 ];
		
		if ( TME )
		{
			
		if ( FST )
		{
		u [ 0 ] += dudy [ 0 ];
		v [ 0 ] += dvdy [ 0 ];
		}
		else
		{
		fS [ 0 ] += dSdy [ 0 ];
		fT [ 0 ] += dTdy [ 0 ];
		fQ [ 0 ] += dQdy [ 0 ];
		}
		
		if ( FGE )
		{
		fogc [ 0 ] += dfdy [ 0 ];
		}
		
		}
		
		if ( SHADED )
		{
		r [ 0 ] += drdy [ 0 ];
		g [ 0 ] += dgdy [ 0 ];
		b [ 0 ] += dbdy [ 0 ];
		a [ 0 ] += dady [ 0 ];
		}
	}
	
	} // if ( EndY >= StartY )

	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	
		
	// left point is included if points are equal
	StartY = ( y1 + 0xf ) >> 4;
	EndY = ( y2 - 1 ) >> 4;
	

	/////////////////////////////////////////////
	// init x on the left and right


	x [ X1Index ] = ( ((s64)x1) << 12 );
	
	z [ X1Index ] = ( ((s64)z1) << 16 );
	
	// offset
	z [ X1Index ] += 0x8000;
	
	if ( TME )
	{
		
	if ( FST )
	{
	//u [ X1Index ] = ( uv [ Coord1 ].U << 12 );
	//v [ X1Index ] = ( uv [ Coord1 ].V << 12 );
	u [ X1Index ] = ( u1 << 12 );
	v [ X1Index ] = ( v1 << 12 );
	}
	else
	{
	fS [ X1Index ] = fS1;
	fT [ X1Index ] = fT1;
	fQ [ X1Index ] = fQ1;
	}
	
	if ( FGE )
	{
	fogc [ X1Index ] = ( ((s64)f1) << 16 );
	
	// offset
	fogc [ X1Index ] += 0x8000;
	}
	
	}
	
	if ( SHADED )
	{
	r [ X1Index ] = ( r1 << 16 );
	g [ X1Index ] = ( g1 << 16 );
	b [ X1Index ] = ( b1 << 16 );
	a [ X1Index ] = ( a1 << 16 );
	
	// offset 
	r [ X1Index ] += 0x8000;
	g [ X1Index ] += 0x8000;
	b [ X1Index ] += 0x8000;
	a [ X1Index ] += 0x8000;
	}
	
	if ( y2 - y1 )
	{
		// triangle is pointed on the bottom //
		dxdy [ X1Index ] = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
		
		dzdy [ X1Index ] = (((s64)( z2 - z1 )) << 20 ) / ((s64)( y2 - y1 ));
		
		if ( TME )
		{
			
		if ( FST )
		{
		//dudy [ X1Index ] = (((s64)( uv [ Coord2 ].U - uv [ Coord1 ].U )) << 12 ) / ((s64)( y2 - y1 ));
		//dvdy [ X1Index ] = (((s64)( uv [ Coord2 ].V - uv [ Coord1 ].V )) << 12 ) / ((s64)( y2 - y1 ));
		dudy [ X1Index ] = (((s64)( u2 - u1 )) << 16 ) / ((s64)( y2 - y1 ));
		dvdy [ X1Index ] = (((s64)( v2 - v1 )) << 16 ) / ((s64)( y2 - y1 ));
		}
		else
		{
		dSdy [ X1Index ] = ( fS2 - fS1 ) / ( ( y2 - y1 ) / 16.0f );
		dTdy [ X1Index ] = ( fT2 - fT1 ) / ( ( y2 - y1 ) / 16.0f );
		dQdy [ X1Index ] = ( fQ2 - fQ1 ) / ( ( y2 - y1 ) / 16.0f );
		}
		
		if ( FGE )
		{
		dfdy [ X1Index ] = (((s64)( f2 - f1 )) << 20 ) / ((s64)( y2 - y1 ));
		}
		
		}
		
		if ( SHADED )
		{
		drdy [ X1Index ] = (((s64)( r2 - r1 )) << 20 ) / ((s64)( y2 - y1 ));
		dgdy [ X1Index ] = (((s64)( g2 - g1 )) << 20 ) / ((s64)( y2 - y1 ));
		dbdy [ X1Index ] = (((s64)( b2 - b1 )) << 20 ) / ((s64)( y2 - y1 ));
		dady [ X1Index ] = (((s64)( a2 - a1 )) << 20 ) / ((s64)( y2 - y1 ));
		}
	}

	
	
	// *** testing ***
	//debug << "\r\nfixed: dR_across=" << dR_across << " dG_across=" << dG_across << " dB_across=" << dB_across;
	
	// the line starts at y1 from here
	//Line = y1;

	//StartY = y1;
	//EndY = y2;

	
	Temp = ( StartY << 4 ) - y1;
	

	x [ X1Index ] += ( dxdy [ X1Index ] >> 4 ) * Temp;
	
	z [ X1Index ] += ( dzdy [ X1Index ] >> 4 ) * Temp;
	
	if ( TME )
	{
		
	if ( FST )
	{
	u [ X1Index ] += ( dudy [ X1Index ] >> 4 ) * Temp;
	v [ X1Index ] += ( dvdy [ X1Index ] >> 4 ) * Temp;
	}
	else
	{
	fS [ X1Index ] += ( dSdy [ X1Index ] ) * ( Temp / 16.0f );
	fT [ X1Index ] += ( dTdy [ X1Index ] ) * ( Temp / 16.0f );
	fQ [ X1Index ] += ( dQdy [ X1Index ] ) * ( Temp / 16.0f );
	}
	
	if ( FGE )
	{
	fogc [ X1Index ] += ( dfdy [ X1Index ] >> 4 ) * Temp;
	}
	
	}
	
	if ( SHADED )
	{
	r [ X1Index ] += ( drdy [ X1Index ] >> 4 ) * Temp;
	g [ X1Index ] += ( dgdy [ X1Index ] >> 4 ) * Temp;
	b [ X1Index ] += ( dbdy [ X1Index ] >> 4 ) * Temp;
	a [ X1Index ] += ( dady [ X1Index ] >> 4 ) * Temp;
	}
	

		
	if ( StartY < Window_YTop )
	{
		if ( EndY < Window_YTop )
		{
			//Temp = EndY - StartY;
			Temp = ( EndY - StartY + 1 ) << 4;
			//StartY = EndY;
			StartY = EndY + 1;
		}
		else
		{
			Temp = ( Window_YTop - StartY ) << 4;
			StartY = Window_YTop;
		}
		
		// dxdy is in .16, Temp is in .4, and x is in .16
		x [ 0 ] += ( dxdy [ 0 ] >> 4 ) * Temp;
		x [ 1 ] += ( dxdy [ 1 ] >> 4 ) * Temp;
		
		z [ 0 ] += ( dzdy [ 0 ] >> 4 ) * Temp;
		
		if ( TME )
		{
			
		if ( FST )
		{
		u [ 0 ] += ( dudy [ 0 ] >> 4 ) * Temp;
		v [ 0 ] += ( dvdy [ 0 ] >> 4 ) * Temp;
		}
		else
		{
		fS [ 0 ] += ( dSdy [ 0 ] ) * ( Temp / 16.0f );
		fT [ 0 ] += ( dTdy [ 0 ] ) * ( Temp / 16.0f );
		fQ [ 0 ] += ( dQdy [ 0 ] ) * ( Temp / 16.0f );
		}
		
		if ( FGE )
		{
		fogc [ 0 ] += ( dfdy [ 0 ] >> 4 ) * Temp;
		}
		
		}
		
		if ( SHADED )
		{
		r [ 0 ] += ( drdy [ 0 ] >> 4 ) * Temp;
		g [ 0 ] += ( dgdy [ 0 ] >> 4 ) * Temp;
		b [ 0 ] += ( dbdy [ 0 ] >> 4 ) * Temp;
		a [ 0 ] += ( dady [ 0 ] >> 4 ) * Temp;
		}
		
		//u [ 1 ] += ( dudy [ 1 ] >> 4 ) * Temp;
		//v [ 1 ] += ( dvdy [ 1 ] >> 4 ) * Temp;
	}

	
	if ( EndY > Window_YBottom )
	{
		//EndY = Window_YBottom + 1;
		EndY = Window_YBottom;
	}

	
	
	
	if ( EndY >= StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y1
	for ( Line = StartY; Line <= EndY; Line++ )
	{
		// left point is included if points are equal
		StartX = ( x [ 0 ] + 0xffffLL ) >> 16;
		EndX = ( x [ 1 ] - 1 ) >> 16;
		//StartX = ( x_left + 0xffffLL ) >> 16;
		//EndX = ( x_right - 1 ) >> 16;
		

		if ( StartX <= Window_XRight && EndX >= Window_XLeft && EndX >= StartX )
		{
			iZ = z [ 0 ];
			
			if ( TME )
			{
				
			if ( FST )
			{
			iU = u [ 0 ];
			iV = v [ 0 ];
			}
			else
			{
			iS = fS [ 0 ];
			iT = fT [ 0 ];
			iQ = fQ [ 0 ];
			}
			
			if ( FGE )
			{
			iF = fogc [ 0 ];
			}
			
			}

			
			if ( SHADED )
			{
			iR = r [ 0 ];
			iG = g [ 0 ];
			iB = b [ 0 ];
			iA = a [ 0 ];
			}
			
			
			
			//Temp = ( StartX << 16 ) - x_left;
			Temp = ( StartX << 16 ) - x [ 0 ];
			
			if ( StartX < Window_XLeft )
			{
				Temp += ( Window_XLeft - StartX ) << 16;
				StartX = Window_XLeft;
			}
			
			iZ += ( dzdx >> 8 ) * ( Temp >> 8 );
			
			if ( TME )
			{
				
			if ( FST )
			{
			iU += ( dudx >> 8 ) * ( Temp >> 8 );
			iV += ( dvdx >> 8 ) * ( Temp >> 8 );
			}
			else
			{
			iS += ( dSdx ) * ( Temp / 65536.0f );
			iT += ( dTdx ) * ( Temp / 65536.0f );
			iQ += ( dQdx ) * ( Temp / 65536.0f );
			}
			
			if ( FGE )
			{
			iF += ( dfdx >> 8 ) * ( Temp >> 8 );
			}
			
			}
			
			if ( SHADED )
			{
			iR += ( drdx >> 8 ) * ( Temp >> 8 );
			iG += ( dgdx >> 8 ) * ( Temp >> 8 );
			iB += ( dbdx >> 8 ) * ( Temp >> 8 );
			iA += ( dadx >> 8 ) * ( Temp >> 8 );
			}
			
			
			if ( EndX > Window_XRight )
			{
				//EndX = Window_XRight + 1;
				EndX = Window_XRight;
			}
			
			
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			//NumberOfPixelsDrawn += EndX - StartX + 1;
			
#ifdef DEBUG_TEST
	debug << "\r\n";
#endif

//#define INLINE_DEBUG_SHOW_RASTER
#ifdef INLINE_DEBUG_SHOW_RASTER
	debug << "\r\n";
	debug << "top y=" << dec << Line << " sx=" << StartX << " ex=" << EndX << hex << " dadx=" << dadx << " dady=" << dady [ 0 ];
	debug << hex << " S=" << iS << " T=" << iT << " Q=" << iQ << " u=" << iU << " v=" << iV << " f=" << iF << " r=" << iR << " g=" << iG << " b=" << iB << " a=" << iA;
#endif


			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across < EndX; x_across++ )
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
				if ( TME )
				{
					
				if ( FST )
				{
				TexCoordX = ( iU >> 16 );
				TexCoordY = ( iV >> 16 );
				}
				else
				{
					TexCoordX = (long) (iS / iQ);
					TexCoordY = (long) (iT / iQ);
				}
				
				TexCoordY = ( ( TexCoordY < TexY_Min ) ? TexY_Min : TexCoordY );
				TexCoordY = ( ( TexCoordY > TexY_Max ) ? TexY_Max : TexCoordY );
				TexCoordY &= TexY_And;
				TexCoordY |= TexY_Or;

				TexCoordX = ( ( TexCoordX < TexX_Min ) ? TexX_Min : TexCoordX );
				TexCoordX = ( ( TexCoordX > TexX_Max ) ? TexX_Max : TexCoordX );
				TexCoordX &= TexX_And;
				TexCoordX |= TexX_Or;

//if ( Clamp_ModeX == 1 && TexWidth == 64 )
//{
//	debug << "\r\nTexCoordX=" << dec << TexCoordX;
//	debug << " TexCoordY=" << dec << TexCoordY;
//}
				


				// get texel from texture buffer (32-bit frame buffer) //
				bgr = pd ( ptr_texture32, TexCoordX, TexCoordY, TexBufWidth, ptr_clut16, TEXA64 );
				



				// texture function ??
				
				
				// fog function ??
				
				//if ( bgr )
				//if ( bgr != -1ULL )
				//{
					
					if ( SHADED )
					{
					bgr = tf ( bgr, iR >> 16, iG >> 16, iB >> 16, iA >> 16 );
					//bgr = _GPU->TextureFunc32 ( bgr, iR >> 16, iG >> 16, iB >> 16, iA >> 16 );
					}
					else
					{
					bgr = tf ( bgr, c1, c2, c3, ca );
					//bgr = _GPU->TextureFunc32 ( bgr, c1, c2, c3, ca );
					}

					if ( FGE )
					{
						//bgr = FogFunc32_t<COLCLAMP> ( bgr, iF, FOGCOL );
						bgr = FogFunc32_t<COLCLAMP> ( bgr, iF >> 16, FOGCOL );
						//bgr = _GPU->FogFunc32 ( bgr, iF >> 16 );
					}


					
					// no need to interpolate Z-value for sprite? just use z1 value?
					//PlotPixel_Texture ( x_across, Line, iZ >> 23, bgr );
					//PlotPixel_Texture ( x_across, Line, iZ >> 23, bgr, c1, c2, c3, ca, iF >> 24 );
					//PlotPixel_Texture_t<TEST_ATST,TEST_AFAIL,TEST_ZTST,ZBUF_PSM,FRAME_PSM> ( x_across, Line, z1, bgr, c1, c2, c3, ca, iF >> 24 );
					PlotPixel_Gradient_t<0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( buf32, zbuf32, x_across, Line, iZ >> 16, bgr, SetPixelMask, FrameBufferWidthInPixels, DA_Test, AlphaXor32, FrameBuffer_WriteMask32, aref, at, zt, af, AlphaSelect, uA, uB, uC, uD );
					
					//_GPU->PlotPixel_Gradient ( x_across, Line, iZ >> 16, bgr );
					
				//}	// end if ( bgr )
				
				}
				else
				{
					if ( SHADED )
					{
						bgr = ( iR >> 16 ) | ( ( iG >> 16 ) << 8 ) | ( ( iB >> 16 ) << 16 ) | ( ( iA >> 16 ) << 24 );
					}
					
					PlotPixel_Gradient_t<0,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,FBPSM,ZBPSM> ( buf32, zbuf32, x_across, Line, iZ >> 16, bgr, SetPixelMask, FrameBufferWidthInPixels, DA_Test, AlphaXor32, FrameBuffer_WriteMask32, aref, at, zt, af, AlphaSelect, uA, uB, uC, uD );
					
				}	// end if ( TME )

					
				iZ += dzdx;
						
				if ( TME )
				{

				if ( FST )
				{
				iU += dudx;
				iV += dvdx;
				}
				else
				{
				iS += dSdx;
				iT += dTdx;
				iQ += dQdx;
				}
				
				if ( FGE )
				{
				iF += dfdx;
				}
				
				}	// end if ( TME )
				
				if ( SHADED )
				{
				iR += drdx;
				iG += dgdx;
				iB += dbdx;
				iA += dadx;
				}
				
			}
			
		}
		
		//////////////////////////////////
		// draw next line
		//Line++;
		
		/////////////////////////////////////
		// update x on left and right
		//x_left += dx_left;
		//x_right += dx_right;
		x [ 0 ] += dxdy [ 0 ];
		x [ 1 ] += dxdy [ 1 ];
		
		z [ 0 ] += dzdy [ 0 ];
		
		if ( TME )
		{
			
		if ( FST )
		{
		u [ 0 ] += dudy [ 0 ];
		v [ 0 ] += dvdy [ 0 ];
		}
		else
		{
		fS [ 0 ] += dSdy [ 0 ];
		fT [ 0 ] += dTdy [ 0 ];
		fQ [ 0 ] += dQdy [ 0 ];
		}
		
		if ( FGE )
		{
		fogc [ 0 ] += dfdy [ 0 ];
		}
		
		}
		
		if ( SHADED )
		{
		r [ 0 ] += drdy [ 0 ];
		g [ 0 ] += dgdy [ 0 ];
		b [ 0 ] += dbdy [ 0 ];
		a [ 0 ] += dady [ 0 ];
		}
	}
	
	} // if ( EndY >= StartY )

		
	return NumPixels;
}



template<const long TME,const long FST,const long FGE,const long COLCLAMP,const long ABE,const long ATST,const long ZTST,const long DATE,const long ZMSK>
inline static u64 Select_RenderTriangle3_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	u32 SHADED, DTHE;
	u32 FBPSM, ZBPSM;
	u32 Combine;
	
//cout << "\n->Select_RenderSprite3_t";
	
	// 000000: PSMCT32, 000001: PSMCT24, 000010: PSMCT16, 001010: PSMCT16S
	// 110000: PSMZ32, 110001: PSMZ24, 110010: PSMZ16, 111010: PSMZ16S
	FBPSM = ( p_inputbuffer [ 2 ] >> 24 ) & 0x3f;
	
	// 0000: PSMZ32, 0001: PSMZ24, 0010: PSMZ16, 1010: PSMZ16S
	ZBPSM = ( p_inputbuffer [ 3 ] >> 24 ) & 0xf;
	
	SHADED = ( p_inputbuffer [ 15 ] >> 3 ) & 1;
	
	Combine = ( SHADED << 12 ) | ( FBPSM << 4 ) | ( ZBPSM );
	
	switch ( Combine )
	{
		// 16-bit combinations
		
		// PSMCT16
		case 0x022:
			//template<SHADED,TME,FST,FGE,COLCLAMP,ABE,ATST,DATE,ZTST,ZMSK,FBPSM,ZBPSM>
			return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x02,0x02>( p_inputbuffer, ulThreadNum );
			break;

		case 0x020:
		case 0x021:
		case 0x02a:
			//template<SHADED,TME,FST,FGE,COLCLAMP,ABE,ATST,DATE,ZTST,ZMSK,FBPSM,ZBPSM>
			return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,1,DATE,1,0x02,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x322:
			//return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x32,0x02>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Triangle_t<0,TME,FST,0,0,0,0,ZTST,0,ZMSK,0x32,0x02>( p_inputbuffer, ulThreadNum );
			break;
			
		// 32-bit combinations
		
		// PSMCT32
		case 0x000:
			return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x00,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x001:
			return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x00,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x00a:
			return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x00,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMCT24
		case 0x010:
			return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x01,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x011:
			return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x01,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x01a:
			return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x01,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMCT16S
		case 0x0a0:
			return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x0a,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x0a1:
			return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x0a,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x0aa:
			return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x0a,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
			
		// write to z-buffer as frame buffer ??
		// PSMZ32
		case 0x300:
			//return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x30,0x00>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Triangle_t<0,TME,FST,0,0,0,0,ZTST,0,ZMSK,0x30,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x301:
			//return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x30,0x01>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Triangle_t<0,TME,FST,0,0,0,0,ZTST,0,ZMSK,0x30,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x30a:
			//return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x30,0x0a>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Triangle_t<0,TME,FST,0,0,0,0,ZTST,0,ZMSK,0x30,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMZ24
		case 0x310:
			//return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x31,0x00>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Triangle_t<0,TME,FST,0,0,0,0,ZTST,0,ZMSK,0x31,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x311:
			//return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x31,0x01>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Triangle_t<0,TME,FST,0,0,0,0,ZTST,0,ZMSK,0x31,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x31a:
			//return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x31,0x0a>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Triangle_t<0,TME,FST,0,0,0,0,ZTST,0,ZMSK,0x31,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMZ16S
		case 0x3a0:
			//return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x3a,0x00>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Triangle_t<0,TME,FST,0,0,0,0,ZTST,0,ZMSK,0x3a,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x3a1:
			//return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x3a,0x01>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Triangle_t<0,TME,FST,0,0,0,0,ZTST,0,ZMSK,0x3a,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x3aa:
			//return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x3a,0x0a>( p_inputbuffer, ulThreadNum );
			return Render_Generic_Triangle_t<0,TME,FST,0,0,0,0,ZTST,0,ZMSK,0x3a,0x0a>( p_inputbuffer, ulThreadNum );
			break;



		// SHADED=1	
		// 16-bit combinations
		
		// PSMCT16
		case 0x1022:
			//template<SHADED,TME,FST,FGE,COLCLAMP,ABE,ATST,DATE,ZTST,ZMSK,FBPSM,ZBPSM>
			return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x02,0x02>( p_inputbuffer, ulThreadNum );
			break;

		case 0x1020:
		case 0x1021:
		case 0x102a:
			//template<SHADED,TME,FST,FGE,COLCLAMP,ABE,ATST,DATE,ZTST,ZMSK,FBPSM,ZBPSM>
			return Render_Generic_Triangle_t<0,TME,FST,FGE,COLCLAMP,ABE,ATST,1,DATE,1,0x02,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
			
		// 32-bit combinations
		
		// PSMCT32
		case 0x1000:
			return Render_Generic_Triangle_t<1,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x00,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x1001:
			return Render_Generic_Triangle_t<1,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x00,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x100a:
			return Render_Generic_Triangle_t<1,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x00,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMCT24
		case 0x1010:
			return Render_Generic_Triangle_t<1,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x01,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x1011:
			return Render_Generic_Triangle_t<1,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x01,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x101a:
			return Render_Generic_Triangle_t<1,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,0,ZMSK,0x01,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		// PSMCT16S
		case 0x10a0:
			return Render_Generic_Triangle_t<1,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x0a,0x00>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x10a1:
			return Render_Generic_Triangle_t<1,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x0a,0x01>( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x10aa:
			return Render_Generic_Triangle_t<1,TME,FST,FGE,COLCLAMP,ABE,ATST,ZTST,DATE,ZMSK,0x0a,0x0a>( p_inputbuffer, ulThreadNum );
			break;
			
		default:
			cout << "\nhps2x64: GPU: Invalid buffer combination. FBUF=" << hex << FBPSM << " ZBUF=" << ZBPSM << " SHADED=" << SHADED;
			return 0;
			break;
	}
}


template<const long TME,const long FST,const long FGE,const long COLCLAMP,const long ABE>
inline static u64 Select_RenderTriangle2_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	u32 ATST, ATE, ZTST, ZTE, ZMSK, DATE;
	u32 Combine;
	
//cout << "\n->Select_RenderSprite2_t";
	
	ATE = p_inputbuffer [ 5 ] & 1;
	ZTE = ( p_inputbuffer [ 5 ] >> 16 ) & 1;
	
	ATST = ( p_inputbuffer [ 5 ] >> 1 ) & 7;
	DATE = ( p_inputbuffer [ 5 ] >> 14 ) & 1;
	ZTST = ( p_inputbuffer [ 5 ] >> 17 ) & 3;
	ZMSK = ( p_inputbuffer [ 3 ] >> 32 ) & 1;
	
	//if ( ATST > 1 ) ATST = 2;
	if ( !ATE ) ATST = 1;
	
	if ( ZTST != 1 ) ZTST = 0;
	if ( ATST != 1 ) ATST = 0;
	
	// if z-test is disabled, then ztest always passes and zbuf is never written to
	if ( !ZTE )
	{
		ZTST = 1;
		ZMSK = 1;
	}
	
	// ***TODO*** if Frame buffer format is PSMCT24, then disable destination alpha test (DATE=0)

	//Combine = ( ATST << 4 ) | ( AFAIL << 3 ) | ( ZTST << 2 ) | ( DATE << 1 ) | ZMSK;
	Combine = ( ATST << 3 ) | ( ZTST << 2 ) | ( DATE << 1 ) | ZMSK;
	
	
	switch ( Combine )
	{
		case 0:
			// <TME,FST,FGE,ABE,COLCLAMP,ATST,ZTST,DATE,ZMSK>
			return Select_RenderTriangle3_t<TME,FST,FGE,COLCLAMP,ABE,0,0,0,0>( p_inputbuffer, ulThreadNum );
			break;
		case 1:
			return Select_RenderTriangle3_t<TME,FST,FGE,COLCLAMP,ABE,0,0,0,1>( p_inputbuffer, ulThreadNum );
			break;
		case 2:
			return Select_RenderTriangle3_t<TME,FST,FGE,COLCLAMP,ABE,0,0,1,0>( p_inputbuffer, ulThreadNum );
			break;
		case 3:
			return Select_RenderTriangle3_t<TME,FST,FGE,COLCLAMP,ABE,0,0,1,1>( p_inputbuffer, ulThreadNum );
			break;
		case 4:
			return Select_RenderTriangle3_t<TME,FST,FGE,COLCLAMP,ABE,0,1,0,0>( p_inputbuffer, ulThreadNum );
			break;
		case 5:
			return Select_RenderTriangle3_t<TME,FST,FGE,COLCLAMP,ABE,0,1,0,1>( p_inputbuffer, ulThreadNum );
			break;
		case 6:
			return Select_RenderTriangle3_t<TME,FST,FGE,COLCLAMP,ABE,0,1,1,0>( p_inputbuffer, ulThreadNum );
			break;
		case 7:
			return Select_RenderTriangle3_t<TME,FST,FGE,COLCLAMP,ABE,0,1,1,1>( p_inputbuffer, ulThreadNum );
			break;
		case 8:
			return Select_RenderTriangle3_t<TME,FST,FGE,COLCLAMP,ABE,1,0,0,0>( p_inputbuffer, ulThreadNum );
			break;
		case 9:
			return Select_RenderTriangle3_t<TME,FST,FGE,COLCLAMP,ABE,1,0,0,1>( p_inputbuffer, ulThreadNum );
			break;
		case 10:
			return Select_RenderTriangle3_t<TME,FST,FGE,COLCLAMP,ABE,1,0,1,0>( p_inputbuffer, ulThreadNum );
			break;
		case 11:
			return Select_RenderTriangle3_t<TME,FST,FGE,COLCLAMP,ABE,1,0,1,1>( p_inputbuffer, ulThreadNum );
			break;
		case 12:
			return Select_RenderTriangle3_t<TME,FST,FGE,COLCLAMP,ABE,1,1,0,0>( p_inputbuffer, ulThreadNum );
			break;
		case 13:
			return Select_RenderTriangle3_t<TME,FST,FGE,COLCLAMP,ABE,1,1,0,1>( p_inputbuffer, ulThreadNum );
			break;
		case 14:
			return Select_RenderTriangle3_t<TME,FST,FGE,COLCLAMP,ABE,1,1,1,0>( p_inputbuffer, ulThreadNum );
			break;
		case 15:
			return Select_RenderTriangle3_t<TME,FST,FGE,COLCLAMP,ABE,1,1,1,1>( p_inputbuffer, ulThreadNum );
			break;
			
	}
	
}



// TME,FST,FGE,ABE,COLCLAMP,FBA,DATE,ZMSK,ZTST(2),ATST(3)
// template needed: PIXELFORMAT,CLUT_PIXELFORMAT,FBUF_PIXELFORMAT,TEXA_AEM,ZBUF_PIXELFORMAT,TEST_ATE,TEST_ATST,TEST_ZTE,TEST_ZTST,TEST_AFAIL,ZBUF_ZMSK,COLCLAMP,TEX_TFX,TEX_TCC
//template<const long TEST_ATST,const long TEST_AFAIL,const long TEST_ZTST,const long FRAME_PSM,const long ZBUF_PSM,const long TEX_PSM>
//void Select_RenderSprite_t ( u32 Coord0, u32 Coord1 )
static u64 Select_RenderTriangle_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	u32 TME, FST, FGE, ABE, COLCLAMP, FBA;	//DATE, ZMSK, ZTST, ATST;
	u32 Combine;
	
	/*
	u32 Coord0 = 16 + 0, Coord1 = 16 + 4, Coord2 = 16 + 8;
	u32 X1Index = 0, X0Index = 1;

	s32 LeftMostX, RightMostX, TopMostY, BottomMostY;
	
	u32 uy0, uy1, uy2;
	s32 x0, y0, x1, y1, x2, y2;
	
	s32 Window_YTop, Window_YBottom, Window_XLeft, Window_XRight;
	
	s32 Coord_OffsetX, Coord_OffsetY;
	u64 NumPixels;
	*/
	
				
	// inputbuffer
	// 0: SCISSOR
	// 1: XYOFFSET
	// 2: FRAME
	// 3: ZBUF
	// 4: FBA
	// 5: TEST
	// 6: COLCLAMP
	// 7: ALPHA
	// 8: PABE
	// 9: DTHE
	// 10: DIMX
	// 11: FOGCOL
	// 12: CLAMP
	// 13: TEX0
	// 14: TEXCLUT
	// 31: TEXA
	// -------------
	// 15: PRIM (COMMAND)
	// 16: RGBAQ
	// 17: XYZ
	// 18: UV/ST
	// 19: FOG
	// 20: RGBAQ
	// 21: XYZ
	// 22: UV/ST
	// 23: FOG
	// 24: RGBAQ
	// 25: XYZ
	// 26: UV/ST
	// 27: FOG


	/*
	Window_YTop = ( p_inputbuffer [ 0 ] >> 32 ) & 0x7ff;
	Window_YBottom = ( p_inputbuffer [ 0 ] >> 48 ) & 0x7ff;
	Window_XLeft = ( p_inputbuffer [ 0 ] ) & 0x7ff;
	Window_XRight = ( p_inputbuffer [ 0 ] >> 16 ) & 0x7ff;
	
	Coord_OffsetX = p_inputbuffer [ 1 ] & 0xffff;
	Coord_OffsetY = ( p_inputbuffer [ 1 ] >> 32 ) & 0xffff;
	
	
	// get y
	uy0 = p_inputbuffer [ Coord0 + 1 ];
	uy1 = p_inputbuffer [ Coord1 + 1 ];
	uy2 = p_inputbuffer [ Coord2 + 1 ];
	
	if ( uy1 < uy0 )
	{
		Swap( uy1, uy0 );
		Swap( Coord1, Coord0 );
	}
	
	if ( uy2 < uy0 )
	{
		Swap( uy2, uy0 );
		Swap( Coord2, Coord0 );
	}
	
	if ( uy2 < uy1 )
	{
		Swap( uy2, uy1 );
		Swap( Coord2, Coord1 );
	}
	
	
	// get x,y
	x0 = uy0 & 0xffff;
	y0 = uy0 >> 16;
	x1 = uy1 & 0xffff;
	y1 = uy1 >> 16;
	x2 = uy2 & 0xffff;
	y2 = uy2 >> 16;

	//////////////////////////////////////////
	// get coordinates on screen
	// *note* this is different from PS1, where you would add the offsets..
	x0 -= Coord_OffsetX;
	y0 -= Coord_OffsetY;
	x1 -= Coord_OffsetX;
	y1 -= Coord_OffsetY;
	x2 -= Coord_OffsetX;
	y2 -= Coord_OffsetY;

	
	
	
	// get the left/right most x
	LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
	LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
	RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
	RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );
	
	LeftMostX += 0xf;
	RightMostX -= 1;
	
	LeftMostX >>= 4;
	RightMostX >>= 4;
	TopMostY = ( y0 + 0xf ) >> 4;
	BottomMostY = ( y2 - 1 ) >> 4;

	// check if triangle is within draw area
	//if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	if ( RightMostX <= Window_XLeft || LeftMostX > Window_XRight || BottomMostY <= Window_YTop || TopMostY > Window_YBottom ) return 0;
#if defined INLINE_DEBUG_TRIANGLE_TEST || defined INLINE_DEBUG_PRIMITIVE_TEST
	else
	{
		debug << dec << "\r\nDrawTriangleGradientTexture";
		debug << dec << " FinalCoords: x0=" << ( x0 >> 4 ) << " y0=" << ( y0 >> 4 ) << " x1=" << ( x1 >> 4 ) << " y1=" << ( y1 >> 4 ) << " x2=" << ( x2 >> 4 ) << " y2=" << ( y2 >> 4 );
		debug << hex << " u0=" << uv_temp [ Coord0 ].U << " v0=" << uv_temp [ Coord0 ].V << " u1=" << uv_temp [ Coord1 ].U << " v1=" << uv_temp [ Coord1 ].V << " u2=" << uv_temp [ Coord2 ].U << " v2=" << uv_temp [ Coord2 ].V;
		debug << "; Clamp_ModeX=" << Clamp_ModeX << " Clamp_ModeY=" << Clamp_ModeY;
		debug << hex << "; CLUTBufPtr32/64=" << ( CLUTBufBase32 >> 6 );
		debug << "; CLUTPixFmt=" << PixelFormat_Names [ CLUTPixelFormat ];
		debug << hex << "; CLUTOffset/16=" << CLUTOffset;
		debug << "; CSM=" << CLUTStoreMode;
		debug << "; TEXCLUT=" << hex << GPURegsGp.TEXCLUT.Value;
		debug << hex << "; TexBufPtr32/64=" << ( TexBufStartOffset32 >> 6 );
		debug << dec << "; TexWidth=" << TexWidth << " TexHeight=" << TexHeight;
		debug << "; TexPixFmt=" << PixelFormat_Names [ PixelFormat ];
		debug << hex << "; FrameBufPtr32/64=" << ( FrameBufferStartOffset32 >> 6 );
		debug << " FrameBufPixFmt=" << PixelFormat_Names [ FrameBuffer_PixelFormat ];
		debug << " Alpha=" << Alpha;
		debug << " ZBUF=" << ZBUF_X.Value;
		debug << " MXL=" << TEX1 [ Ctx ].MXL;
		debug << " LCM=" << TEX1 [ Ctx ].LCM;
		debug << " K=" << TEX1 [ Ctx ].K;
	}
#endif


	// denominator is negative when x1 is on the left, positive when x1 is on the right
	s64 t0 = y1 - y2;
	s64 t1 = y0 - y2;
	s64 denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );
	
	
	NumPixels = _Abs( denominator ) >> ( 8 + 1 );

	if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
	{
		ulInputBuffer_WriteIndex++;
		return NumPixels;
	}

	
	// check if x1 is on left or right //
	if ( denominator > 0 )
	{
		// x1 is on the right //
		X1Index = 1;
		X0Index = 0;
	}


	// get z
	//z0 = ( p_inputbuffer [ Coord0 + 1 ] >> 32 );
	//z1 = ( p_inputbuffer [ Coord1 + 1 ] >> 32 );
	//z2 = ( p_inputbuffer [ Coord2 + 1 ] >> 32 );


	p_inputbuffer [ 0 ] = denominator;
	
	((u8*)p_inputbuffer) [ 8 ] = Coord0;
	((u8*)p_inputbuffer) [ 9 ] = Coord1;
	((u8*)p_inputbuffer) [ 10 ] = Coord2;
	((u8*)p_inputbuffer) [ 11 ] = X1Index;
	
	((u32*)p_inputbuffer) [ ( 28 << 1 ) + 0 ] = x0;
	((u32*)p_inputbuffer) [ ( 28 << 1 ) + 1 ] = y0;
	((u32*)p_inputbuffer) [ ( 29 << 1 ) + 0 ] = x1;
	((u32*)p_inputbuffer) [ ( 29 << 1 ) + 1 ] = y1;
	((u32*)p_inputbuffer) [ ( 30 << 1 ) + 0 ] = x2;
	((u32*)p_inputbuffer) [ ( 30 << 1 ) + 1 ] = y2;
	*/
	
		
	TME = ( p_inputbuffer [ 15 ] >> 4 ) & 1;
	//SHADED = ( p_inputbuffer [ 15 ] >> 3 ) & 1;
	FGE = ( p_inputbuffer [ 15 ] >> 5 ) & 1;
	FST = ( p_inputbuffer [ 15 ] >> 8 ) & 1;
	
	ABE = ( p_inputbuffer [ 15 ] >> 6 ) & 1;
	COLCLAMP = ( p_inputbuffer [ 6 ] ) & 1;
	//FBA = ( p_inputbuffer [ 4 ] ) & 1;

	
	Combine = FST | ( FGE << 1 ) | ( COLCLAMP << 2 ) | ( ABE << 3 ) | ( TME << 4 );
	
	switch ( Combine )
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			//Select_RenderTriangle2_t<TME,FST,FGE,COLCLAMP,ABE> ( p_inputbuffer, ulThreadNum )
			return Select_RenderTriangle2_t<0,0,0,0,0> ( p_inputbuffer, ulThreadNum );
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			return Select_RenderTriangle2_t<0,0,0,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		
		case 12:
		case 13:
		case 14:
		case 15:
			return Select_RenderTriangle2_t<0,0,0,1,1> ( p_inputbuffer, ulThreadNum );
			break;
			
		case 16:
			return Select_RenderTriangle2_t<1,0,0,0,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 17:
			return Select_RenderTriangle2_t<1,1,0,0,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 18:
			return Select_RenderTriangle2_t<1,0,1,0,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 19:
			return Select_RenderTriangle2_t<1,1,1,0,0> ( p_inputbuffer, ulThreadNum );
			break;

		case 20:
			return Select_RenderTriangle2_t<1,0,0,1,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 21:
			return Select_RenderTriangle2_t<1,1,0,1,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 22:
			return Select_RenderTriangle2_t<1,0,1,1,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 23:
			return Select_RenderTriangle2_t<1,1,1,1,0> ( p_inputbuffer, ulThreadNum );
			break;
			
		case 24:
			return Select_RenderTriangle2_t<1,0,0,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 25:
			return Select_RenderTriangle2_t<1,1,0,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 26:
			return Select_RenderTriangle2_t<1,0,1,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 27:
			return Select_RenderTriangle2_t<1,1,1,0,1> ( p_inputbuffer, ulThreadNum );
			break;

		case 28:
			return Select_RenderTriangle2_t<1,0,0,1,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 29:
			return Select_RenderTriangle2_t<1,1,0,1,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 30:
			return Select_RenderTriangle2_t<1,0,1,1,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 31:
			return Select_RenderTriangle2_t<1,1,1,1,1> ( p_inputbuffer, ulThreadNum );
			break;

	}
	
	//return NumPixels;
}

#endif


//-------------------------------------------------------------------------------------------------------------


#ifdef USE_TEMPLATES_PS2_COPYLOCAL

template<const long BITBLTBUF_SPSM, const long BITBLTBUF_DPSM>
static void Render_Generic_CopyLocal_t( u64* p_inputbuffer, u32 ulThreadNum )
{
	u32 XferSrcOffset32, XferDstOffset32, XferSrcBufWidth, XferDstBufWidth, XferSrcX, XferSrcY, XferDstX, XferDstY, XferSrcPixelSize, XferDstPixelSize, XferWidth, XferHeight;
	u32 XferDstBufWidth64, XferSrcBufWidth64;
	u32 XferX, XferY;
	u64 PixelCount;
	u64 PixelShift;
	
	// variables for local->local transfer
	s32 xStart, yStart, xInc, yInc;
	
	u32 BITBLTBUF_SBP, BITBLTBUF_SBW, BITBLTBUF_DBP, BITBLTBUF_DBW;
	u32 TRXPOS_SSAX, TRXPOS_SSAY, TRXPOS_DSAX, TRXPOS_DSAY, TRXPOS_DIR;
	u32 TRXREG_RRW, TRXREG_RRH;

	u64 PixelLoad;
	u32 Pixel0;
	u32 Pixel1;

	// get pointer to dest buffer
	//u32 *DestBuffer;
	
	u32 *Data32;
	u16 *Data16;
	u8 *Data8;
	
	u32 *srcbuf32;
	u16 *srcbuf16;
	u8 *srcbuf8;

	u32 *dstbuf32;
	u16 *dstbuf16;
	u8 *dstbuf8;
	
	u32 *SrcBuffer32;
	u16 *SrcBuffer16;
	u8 *SrcBuffer8;

	u32 *DstBuffer32;
	u16 *DstBuffer16;
	u8 *DstBuffer8;
	
	u32 Count;
	
	u32 OffsetSrc, OffsetDst;
	


	
	BITBLTBUF_SBP = ( p_inputbuffer [ 0 ] >> 0 ) & 0x3fff;
	BITBLTBUF_SBW = ( p_inputbuffer [ 0 ] >> 16 ) & 0x3f;
	BITBLTBUF_DBP = ( p_inputbuffer [ 0 ] >> 32 ) & 0x3fff;
	BITBLTBUF_DBW = ( p_inputbuffer [ 0 ] >> 48 ) & 0x3f;

	TRXPOS_SSAX = ( p_inputbuffer [ 1 ] >> 0 ) & 0x7ff;
	TRXPOS_SSAY = ( p_inputbuffer [ 1 ] >> 16 ) & 0x7ff;
	TRXPOS_DSAX = ( p_inputbuffer [ 1 ] >> 32 ) & 0x7ff;
	TRXPOS_DSAY = ( p_inputbuffer [ 1 ] >> 48 ) & 0x7ff;
	
	TRXPOS_DIR = ( p_inputbuffer [ 1 ] >> 59 ) & 0x3;
	
	TRXREG_RRW = ( p_inputbuffer [ 2 ] >> 0 ) & 0xfff;
	TRXREG_RRH = ( p_inputbuffer [ 2 ] >> 32 ) & 0xfff;

	// get source buffer offset (SBP : word address/64)
	//XferSrcOffset32 = GPURegsGp.BITBLTBUF.SBP << 6;
	XferSrcOffset32 = BITBLTBUF_SBP << 6;

	//if ( GPURegsGp.BITBLTBUF.SBW >= 1 && GPURegsGp.BITBLTBUF.SBW <= 32 )
	if ( BITBLTBUF_SBW >= 1 && BITBLTBUF_SBW <= 32 )
	{
		// get source buffer width (in pixels) (SBW : number of pixels/64)
		XferSrcBufWidth = BITBLTBUF_SBW << 6;
	}
	else
	{
#ifdef INLINE_DEBUG_INVALID_SBW
	debug << dec << "\r\nhps2x64: ALERT: Src Buffer Width value illegal. BITBLTBUF.SBW=" << dec << (u32) BITBLTBUF_SBW;
#endif

#ifdef VERBOSE_ALERTS_SBW
	cout << "\nhps2x64: ALERT: Src Buffer Width value illegal. BITBLTBUF.SBW=" << dec << (u32) BITBLTBUF_SBW;
#endif
	}

	// get dest buffer offset (DBP : word address/64)
	XferDstOffset32 = BITBLTBUF_DBP << 6;

	//if ( GPURegsGp.BITBLTBUF.DBW >= 1 && GPURegsGp.BITBLTBUF.DBW <= 32 )
	if ( BITBLTBUF_DBW >= 1 && BITBLTBUF_DBW <= 32 )
	{
		// get dest buffer width (in pixels) (DBW : pixels/64)
		XferDstBufWidth = BITBLTBUF_DBW << 6;
	}
	else
	{
#ifdef INLINE_DEBUG_INVALID_DBW
	debug << dec << "\r\nhps2x64: ALERT: Dst Buffer Width value illegal. BITBLTBUF.DBW=" << dec << (u32) BITBLTBUF_DBW;
#endif

#ifdef VERBOSE_ALERTS_DBW
if ( BITBLTBUF_DBW )
{
				cout << "\nhps2x64: ALERT: Dst Buffer Width value illegal. BITBLTBUF.DBW=" << dec << (u32) BITBLTBUF_DBW;
}
#endif
	}

			
	// TRXPOS //
	
	// transfer x,y for source
	//XferSrcX = GPURegsGp.TRXPOS.SSAX;
	//XferSrcY = GPURegsGp.TRXPOS.SSAY;
	XferSrcX = TRXPOS_SSAX;
	XferSrcY = TRXPOS_SSAY;
	
	
	// transfer x,y for dest
	//XferDstX = GPURegsGp.TRXPOS.DSAX;
	//XferDstY = GPURegsGp.TRXPOS.DSAY;
	XferDstX = TRXPOS_DSAX;
	XferDstY = TRXPOS_DSAY;

	// set current transfer x position
	XferX = 0;
	
	// set current transfer y position
	XferY = 0;
	
	// set pixel shift and count for 24-bit pixels
	PixelShift = 0;
	PixelCount = 0;

	// TRXREG //
	
	// get transfer width (in pixels)
	XferWidth = TRXREG_RRW;
	
	// get transfer height (in pixels)
	XferHeight = TRXREG_RRH;
			
			
	// ***todo*** get transfer direction, and start transfer immediately if GPU->GPU transfer
	//if ( ( Value & 3 ) == 2 )
	//{
#ifdef VERBOSE_LOCAL_TRANSFER
		cout << "\nhps2x64: ALERT: GPU: Local->Local transfer started!!!\n";
#endif
				
			// set local->local transfer variables //
			if ( TRXPOS_DIR & 2 )
			{
				// transfer is starting on the right //
				xStart = XferWidth - 1;
				xInc = -1;
			}
			else
			{
				// transfer is starting on the left //
				xStart = 0;
				xInc = 1;
			}
			
			if ( TRXPOS_DIR & 1 )
			{
				// transfer is starting on the bottom //
				yStart = XferHeight - 1;
				yInc = -1;
			}
			else
			{
				// transfer is starting on the top //
				yStart = 0;
				yInc = 1;
			}
		
		// set the local transfer variables
		XferX = xStart;
		XferY = yStart;
		
	//}


#ifdef INLINE_DEBUG_TRANSFER_LOCAL
	if ( !XferX && !XferY )
	{
		debug << "\r\nTransferLocal: ";
		//debug << dec << " WC=" << WordCount32;
		debug << dec << " Width=" << XferWidth << " Height=" << XferHeight;
		debug << hex << " DSTPTR32/64=" << BITBLTBUF_DBP;
		debug << hex << " SRCPTR32/64=" << BITBLTBUF_SBP;
		debug << dec << " DstBufWidth=" << XferDstBufWidth;
		debug << dec << " SrcBufWidth=" << XferSrcBufWidth;
		debug << " OutPixFmt=" << PixelFormat_Names [ BITBLTBUF_DPSM ];
		debug << " InPixFmt=" << PixelFormat_Names [ BITBLTBUF_SPSM ];
		debug << " TransferDir=" << TransferDir_Names [ TRXPOS_DIR ];
		debug << dec << " XferX=" << XferX << " XferY=" << XferY;
		debug << dec << " XferDstX=" << XferDstX << " XferDstY=" << XferDstY;
		debug << dec << " XferSrcX=" << XferSrcX << " XferSrcY=" << XferSrcY;
		debug << " @Cycle#" << dec << *_DebugCycleCount;
#ifdef VERBOSE_TRANSFER_LOCAL
		cout << "\nhps2x64: ALERT: GPU: Transferring data TO gpu FROM gpu";
#endif
	}
#endif

#ifdef INLINE_DEBUG_TRANSFER_LOCAL_2
	debug << "\r\n";
	debug << " XferX=" << dec << XferX << " XferY=" << XferY;
	//debug << " WC=" << WordCount32;
	debug << " Offset=" << ( ( XferSrcOffset32 + XferX + ( XferY * XferSrcBufWidth ) ) << 2 );
	//debug << " XferDstX=" << XferDstX << " XferDstY=" << XferDstY;
#endif

	
	// make sure transfer method is set to local->local
	/*
	if ( GPURegsGp.TRXDIR.XDIR != 2 )
	{
		cout << "\nhps2x64: ALERT: GPU: Performing local->local transmission while not activated";
		
#ifdef INLINE_DEBUG_TRANSFER_LOCAL
		cout << "\r\nhps2x64: ALERT: GPU: Performing local->local transmission while not activated";
#endif
	}
	*/
	
	// get count of pixels to transfer
	Count = XferWidth * XferHeight;
	
	if ( !XferSrcBufWidth )
	{
		if ( !XferX && !XferY )
		{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL
	debug << "\r\nGPU: ERROR: Transfer Src Buf Width is ZERO!!!\r\n";
#endif

			cout << "\nhps2x64: GPU: ERROR: Transfer Src Buf Width is ZERO!!!\n";
		}
		
		return;
	}
	
	// check that there is data to transfer
	if ( !XferWidth || !XferHeight ) return;
	
	
	
	srcbuf32 = & ( _GPU->RAM32 [ XferSrcOffset32 ] );
	srcbuf16 = (u16*) srcbuf32;
	srcbuf8 = (u8*) srcbuf32;

	dstbuf32 = & ( _GPU->RAM32 [ XferDstOffset32 ] );
	dstbuf16 = (u16*) dstbuf32;
	dstbuf8 = (u8*) dstbuf32;
	
		if ( ( BITBLTBUF_SPSM & 7 ) < 2 )
		{
			// 32 or 24 bit pixels //
			
			if ( !XferX && !XferY )
			{
				// first transfer for PSMCT32, PSMCT24, PSMZ32, PSMZ24 //
				
				if ( BITBLTBUF_SPSM & 1 )
				{
					// PSMCT24, PSMZ24 //
					
					// Limitations for PSMCT24, PSMZ24
					// no limitation on start x-coord
					// width must be a multiple of 8
					if ( XferWidth & 7 )
					{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL_LIMIT
						debug << "\r\n***PSMCT24/PSMZ24 Transfer width not multiple of 8. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL_LIMIT
						cout << "\nhps2x64: ALERT: GPU: PSMCT24/PSMZ24 Transfer width not multiple of 8. Cycle#" << dec << *_DebugCycleCount;
#endif
					} // end if ( XferWidth & 7 )
				}
				else
				{
					// PSMCT32, PSMZ32 //
					
					// Limitations for PSMCT32, PSMZ32
					// no limitation on start x-coord
					// width must be a multiple of 2
					if ( XferWidth & 1 )
					{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL_LIMIT
						debug << "\r\n***PSMCT32/PSMZ32 Transfer width not multiple of 2. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL_LIMIT
						cout << "\nhps2x64: ALERT: GPU: PSMCT32/PSMZ32 Transfer width not multiple of 2. Cycle#" << dec << *_DebugCycleCount;
#endif
					} // end if ( XferWidth & 1 )
					
				} // end if ( GPURegsGp.BITBLTBUF.SPSM & 1 )
				
				// check if this transfers outside of GPU memory device
				if ( ( ( XferSrcOffset32 + XferWidth + ( XferHeight * XferSrcBufWidth ) ) << 2 ) > c_iRAM_Size )
				{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL
				debug << "\r\n***Transfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL
					cout << "\nTransfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif
				}
				
				if ( ( ( XferDstOffset32 + XferWidth + ( XferHeight * XferDstBufWidth ) ) << 2 ) > c_iRAM_Size )
				{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL
				debug << "\r\n***Transfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL
					cout << "\nTransfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif
				}
			}
			
			
			// 32-bit pixels
			
			while ( ( XferY < XferHeight ) && Count )
			{
				// have to keep pixels in GPU buffer for now
				//SrcBuffer32 = & ( srcbuf32 [ CvtAddrPix32( XferX + XferSrcX, XferY + XferSrcY, XferSrcBufWidth ) ] );
				//DstBuffer32 = & ( dstbuf32 [ CvtAddrPix32( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
				switch ( BITBLTBUF_SPSM )
				{
					case 0:
					case 1:
						SrcBuffer32 = & ( srcbuf32 [ CvtAddrPix32( XferX + XferSrcX, XferY + XferSrcY, XferSrcBufWidth ) ] );
						break;
						
					case 0x30:
					case 0x31:
						SrcBuffer32 = & ( srcbuf32 [ CvtAddrZBuf32( XferX + XferSrcX, XferY + XferSrcY, XferSrcBufWidth ) ] );
						break;
				}
				switch ( BITBLTBUF_DPSM )
				{
					case 0:
					case 1:
						DstBuffer32 = & ( dstbuf32 [ CvtAddrPix32( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						break;
						
					case 0x30:
					case 0x31:
						DstBuffer32 = & ( dstbuf32 [ CvtAddrZBuf32( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						break;
				}
			
				if ( ( DstBuffer32 < PtrEnd ) && ( SrcBuffer32 < PtrEnd ) )
				{
					// transfer a pixel
					// *DstBuffer32++ = *SrcBuffer32++;
					// *DstBuffer32 = *SrcBuffer32;
					switch ( BITBLTBUF_DPSM & 1 )
					{
						case 0:
						case 0x30:
							// 32-bit pixel //
							*DstBuffer32 = *SrcBuffer32;
							break;
							
						case 1:
						case 0x31:
							// 24-bit pixel //
							*DstBuffer32 = ( ( *SrcBuffer32 ) & 0xffffff ) | ( ( *DstBuffer32 ) & ~0xffffff );
							break;
						
					} // end switch ( GPURegsGp.BITBLTBUF.DPSM & 1 )
				}
				
				
				// update x
				//XferX++;
				XferX += xInc;
				
				// check if greater than width
				if ( XferX >= XferWidth )
				{
					// go to next line
					//XferX = 0;
					//XferY++;
					XferX = xStart;
					XferY += yInc;
					
					// set buffer pointer
					//SrcBuffer32 = & ( srcbuf32 [ ( ( XferX + XferSrcX ) + ( ( XferY + XferSrcY ) * XferSrcBufWidth ) ) ] );
					//DstBuffer32 = & ( dstbuf32 [ ( ( XferX + XferDstX ) + ( ( XferY + XferDstY ) * XferDstBufWidth ) ) ] );
				}
				
				
				// don't go past the number of pixels available to read
				Count--;
			}
		}
		else if ( ( BITBLTBUF_SPSM & 7 ) == 2 )
		{
			// 16-bit pixels //
			
			// PSMCT16, PSMZ16 //
			
			if ( !XferX && !XferY )
			{
					
					// Limitations for PSMCT16, PSMZ16
					// no limitation on start x-coord
					// width must be a multiple of 4
					if ( XferWidth & 3 )
					{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL_LIMIT
						debug << "\r\n***PSMCT16/PSMZ16 Transfer width not multiple of 4. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL_LIMIT
						cout << "\nhps2x64: ALERT: GPU: PSMCT16/PSMZ16 Transfer width not multiple of 4. Cycle#" << dec << *_DebugCycleCount;
#endif
					} // end if ( XferWidth & 3 )
					
					
					
				// check if this transfers outside of GPU memory device
				if ( ( ( XferSrcOffset32 + ( ( XferWidth + ( XferHeight * XferSrcBufWidth ) ) >> 1 ) ) << 2 ) > c_iRAM_Size )
				{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL
				debug << "\r\n***Transfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL
					cout << "\nTransfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif
				}
				
				if ( ( ( XferDstOffset32 + ( ( XferWidth + ( XferHeight * XferDstBufWidth ) ) >> 1 ) ) << 2 ) > c_iRAM_Size )
				{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL
				debug << "\r\n***Transfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL
					cout << "\nTransfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif
				}
			}
			
			
			// 16-bit pixels //
			
			while ( ( XferY < XferHeight ) && Count )
			{
			
				// have to keep pixels in GPU buffer for now
				//SrcBuffer16 = & ( srcbuf16 [ ( ( XferX + XferSrcX ) + ( ( XferY + XferSrcY ) * XferSrcBufWidth ) ) ] );
				//DstBuffer16 = & ( dstbuf16 [ ( ( XferX + XferDstX ) + ( ( XferY + XferDstY ) * XferDstBufWidth ) ) ] );
				switch ( BITBLTBUF_SPSM )
				{
					case 2:
						SrcBuffer16 = & ( srcbuf16 [ CvtAddrPix16 ( XferX + XferSrcX, XferY + XferSrcY, XferSrcBufWidth ) ] );
						//DstBuffer16 = & ( dstbuf16 [ CvtAddrPix16 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						break;
						
					case 0xa:
						SrcBuffer16 = & ( srcbuf16 [ CvtAddrPix16S ( XferX + XferSrcX, XferY + XferSrcY, XferSrcBufWidth ) ] );
						//DstBuffer16 = & ( dstbuf16 [ CvtAddrPix16S ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						break;
						
					case 0x32:
						SrcBuffer16 = & ( srcbuf16 [ CvtAddrZBuf16 ( XferX + XferSrcX, XferY + XferSrcY, XferSrcBufWidth ) ] );
						break;
						
					case 0x3a:
						SrcBuffer16 = & ( srcbuf16 [ CvtAddrZBuf16S ( XferX + XferSrcX, XferY + XferSrcY, XferSrcBufWidth ) ] );
						break;
				}
				switch ( BITBLTBUF_DPSM )
				{
					case 2:
						//SrcBuffer16 = & ( srcbuf16 [ CvtAddrPix16 ( XferX + XferSrcX, XferY + XferSrcY, XferSrcBufWidth ) ] );
						DstBuffer16 = & ( dstbuf16 [ CvtAddrPix16 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						break;
						
					case 0xa:
						//SrcBuffer16 = & ( srcbuf16 [ CvtAddrPix16S ( XferX + XferSrcX, XferY + XferSrcY, XferSrcBufWidth ) ] );
						DstBuffer16 = & ( dstbuf16 [ CvtAddrPix16S ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						break;
						
					case 0x32:
						DstBuffer16 = & ( dstbuf16 [ CvtAddrZBuf16 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						break;
						
					case 0x3a:
						DstBuffer16 = & ( dstbuf16 [ CvtAddrZBuf16S ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						break;
				}
				
				if ( ( DstBuffer16 < PtrEnd ) && ( SrcBuffer16 < PtrEnd ) )
				{
					// transfer a pixel
					// *DstBuffer16++ = *SrcBuffer16++;
					*DstBuffer16 = *SrcBuffer16;
				}
				
				
				// update x
				//XferX++;
				XferX += xInc;
				
				// check if greater than width
				if ( XferX >= XferWidth )
				{
					// go to next line
					//XferX = 0;
					//XferY++;
					XferX = xStart;
					XferY += yInc;
					
				}
				
				
				// don't go past the number of pixels available to read
				Count--;
			}
		}
		else if ( ( BITBLTBUF_SPSM & 7 ) == 3 )
		{
			// 8-bit pixels //
			
			// PSMT8, PSMT8H //
			
			if ( !XferX && !XferY )
			{
					
					// Limitations for PSMT8, PSMT8H
					// start x must be a multiple of 2
					if ( XferSrcX & 1 )
					{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL_LIMIT
						debug << "\r\n***PSMT8/PSMT8H Transfer SrcX not multiple of 2. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL_LIMIT
						cout << "\nhps2x64: ALERT: GPU: PSMT8/PSMT8H Transfer SrcX not multiple of 2. Cycle#" << dec << *_DebugCycleCount;
#endif
					}
					if ( XferDstX & 1 )
					{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL_LIMIT
						debug << "\r\n***PSMT8/PSMT8H Transfer DstX not multiple of 2. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL_LIMIT
						cout << "\nhps2x64: ALERT: GPU: PSMT8/PSMT8H Transfer DstX not multiple of 2. Cycle#" << dec << *_DebugCycleCount;
#endif
					}
					
					// width must be a multiple of 8
					if ( XferWidth & 7 )
					{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL_LIMIT
						debug << "\r\n***PSMT8/PSMT8H Transfer width not multiple of 8. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL_LIMIT
						cout << "\nhps2x64: ALERT: GPU: PSMT8/PSMT8H Transfer width not multiple of 8. Cycle#" << dec << *_DebugCycleCount;
#endif
					} // end if ( XferWidth & 7 )
					
				// check if this transfers outside of GPU memory device
				if ( ( ( XferSrcOffset32 + ( ( XferWidth + ( XferHeight * XferSrcBufWidth ) ) >> 2 ) ) << 2 ) > c_iRAM_Size )
				{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL
				debug << "\r\n***Transfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL
					cout << "\nTransfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif
				}
				
				if ( ( ( XferDstOffset32 + ( ( XferWidth + ( XferHeight * XferDstBufWidth ) ) >> 2 ) ) << 2 ) > c_iRAM_Size )
				{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL
				debug << "\r\n***Transfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL
					cout << "\nTransfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif
				}
			}
			
			
			// 8-bit pixels //
			
			while ( ( XferY < XferHeight ) && Count )
			{
				// determine format pixel is coming from
				switch ( BITBLTBUF_SPSM )
				{
					// PSMT8
					case 0x13:
						// have to keep pixels in GPU buffer for now
						//SrcBuffer8 = & ( srcbuf8 [ ( ( XferX + XferSrcX ) + ( ( XferY + XferSrcY ) * XferSrcBufWidth ) ) ] );
						//DstBuffer8 = & ( dstbuf8 [ ( ( XferX + XferDstX ) + ( ( XferY + XferDstY ) * XferDstBufWidth ) ) ] );
						SrcBuffer8 = & ( srcbuf8 [ CvtAddrPix8( XferX + XferSrcX, XferY + XferSrcY, XferSrcBufWidth ) ] );
						//DstBuffer8 = & ( dstbuf8 [ CvtAddrPix8( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						
						// get the pixel
						Pixel0 = *SrcBuffer8;
						break;
						
					// PSMT8H
					case 0x1b:
						SrcBuffer32 = & ( srcbuf32 [ CvtAddrPix32( XferX + XferSrcX, XferY + XferSrcY, XferSrcBufWidth ) ] );
						Pixel0 = ( *SrcBuffer32 ) >> 24;
						break;
				}
			
				// determine format pixel is going into
				switch ( BITBLTBUF_DPSM )
				{
					// PSMT8
					case 0x13:
						DstBuffer8 = & ( dstbuf8 [ CvtAddrPix8( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						*DstBuffer8 = Pixel0;
						break;
						
					// PSMT8H
					case 0x1b:
						DstBuffer32 = & ( dstbuf32 [ CvtAddrPix32( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						*DstBuffer32 = ( ( *DstBuffer32 ) & ~( 0xff << 24 ) ) | ( Pixel0 << 24 );
						break;
				}
					
				
				// update x
				//XferX++;
				XferX += xInc;
				
				// check if greater than width
				// ***TODO*** this would not work
				if ( XferX >= XferWidth )
				{
					// go to next line
					//XferX = 0;
					//XferY++;
					XferX = xStart;
					XferY += yInc;
					
				}
				
				
				// don't go past the number of pixels available to read
				Count--;
			}
		}
		else if ( ( BITBLTBUF_SPSM & 7 ) == 4 )
		{
			// 4-bit pixels //
			
			if ( !XferX && !XferY )
			{
					// Limitations for PSMT4, PSMT4HL, PSMT4HH
					// start x must be a multiple of 4
					if ( XferSrcX & 3 )
					{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL_LIMIT
						debug << "\r\n***PSMT4/PSMT4HL/PSMT4HH Transfer SrcX not multiple of 2. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL_LIMIT
						cout << "\nhps2x64: ALERT: GPU: PSMT4/PSMT4HL/PSMT4HH Transfer SrcX not multiple of 2. Cycle#" << dec << *_DebugCycleCount;
#endif
					}
					if ( XferDstX & 3 )
					{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL_LIMIT
						debug << "\r\n***PSMT4/PSMT4HL/PSMT4HH Transfer DstX not multiple of 2. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL_LIMIT
						cout << "\nhps2x64: ALERT: GPU: PSMT4/PSMT4HL/PSMT4HH Transfer DstX not multiple of 2. Cycle#" << dec << *_DebugCycleCount;
#endif
					}
					
					// width must be a multiple of 8
					if ( XferWidth & 7 )
					{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL_LIMIT
						debug << "\r\n***PSMT4/PSMT4HL/PSMT4HH Transfer width not multiple of 8. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL_LIMIT
						cout << "\nhps2x64: ALERT: GPU: PSMT4/PSMT4HL/PSMT4HH Transfer width not multiple of 8. Cycle#" << dec << *_DebugCycleCount;
#endif
					} // end if ( XferWidth & 7 )
					
				// check if this transfers outside of GPU memory device
				if ( ( ( XferSrcOffset32 + ( ( XferWidth + ( XferHeight * XferSrcBufWidth ) ) >> 3 ) ) << 2 ) > c_iRAM_Size )
				{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL
				debug << "\r\n***Transfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL
					cout << "\nTransfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif
				}
				
				if ( ( ( XferDstOffset32 + ( ( XferWidth + ( XferHeight * XferDstBufWidth ) ) >> 3 ) ) << 2 ) > c_iRAM_Size )
				{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL
				debug << "\r\n***Transfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL
					cout << "\nTransfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif
				}
			}
			
			
			
			// 8 times the pixels, but transferring 2 at a time, so times 4
			//WordCount32 <<= 2;
			Count >>= 1;
			
			while ( ( XferY < XferHeight ) && Count )
			{
				// determine format pixel is coming from
				switch ( BITBLTBUF_SPSM )
				{
					// PSMT4
					case 0x14:
						// have to keep pixels in GPU buffer for now
						//SrcBuffer8 = & ( srcbuf8 [ ( ( ( XferX + XferSrcX ) >> 1 ) + ( ( XferY + XferSrcY ) * ( XferSrcBufWidth >> 1 ) ) ) ] );
						//DstBuffer8 = & ( dstbuf8 [ ( ( ( XferX + XferDstX ) >> 1 ) + ( ( XferY + XferDstY ) * ( XferDstBufWidth >> 1 ) ) ) ] );
						OffsetSrc = CvtAddrPix4( XferX + XferSrcX, XferY + XferSrcY, XferSrcBufWidth );
						//OffsetDst = CvtAddrPix4( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth );
						SrcBuffer8 = & ( srcbuf8 [ OffsetSrc >> 1 ] );
						//DstBuffer8 = & ( dstbuf8 [ OffsetDst >> 1 ] );
						
						OffsetSrc = ( OffsetSrc & 1 ) << 2;
						Pixel0 = ( ( *SrcBuffer8 ) >> OffsetSrc ) & 0xf;
						break;
						
					// PSMT4HL
					case 0x24:
						SrcBuffer32 = & ( srcbuf32 [ CvtAddrPix32( XferX + XferSrcX, XferY + XferSrcY, XferSrcBufWidth ) ] );
						Pixel0 = ( ( *SrcBuffer32 ) >> 24 ) & 0xf;
						break;
						
					// PSMT4HH
					case 0x2c:
						SrcBuffer32 = & ( srcbuf32 [ CvtAddrPix32( XferX + XferSrcX, XferY + XferSrcY, XferSrcBufWidth ) ] );
						Pixel0 = ( ( *SrcBuffer32 ) >> 28 ) & 0xf;
						break;
				}
			
				// determine format pixel is going into
				switch ( BITBLTBUF_SPSM )
				{
					// PSMT4
					case 0x14:
						OffsetDst = CvtAddrPix4( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth );
						DstBuffer8 = & ( dstbuf8 [ OffsetDst >> 1 ] );
						
						OffsetDst = ( OffsetDst & 1 ) << 2;
						*DstBuffer8 = ( ( *DstBuffer8 ) & ~( 0xf << OffsetDst ) ) | ( Pixel0 << OffsetDst );
						break;
						
					// PSMT4HL
					case 0x24:
						DstBuffer32 = & ( dstbuf32 [ CvtAddrPix32( XferX + XferSrcX, XferY + XferSrcY, XferSrcBufWidth ) ] );
						*DstBuffer32 = ( ( *DstBuffer32 ) & ~( 0xf << 24 ) ) | ( Pixel0 << 24 );
						break;
						
					// PSMT4HH
					case 0x2c:
						DstBuffer32 = & ( dstbuf32 [ CvtAddrPix32( XferX + XferSrcX, XferY + XferSrcY, XferSrcBufWidth ) ] );
						*DstBuffer32 = ( ( *DstBuffer32 ) & ~( 0xf << 28 ) ) | ( Pixel0 << 28 );
						break;
				}
				
					// update x
					XferX += xInc;
					
					// check if greater than width
					if ( XferX >= XferWidth )
					{
						// go to next line
						//XferX = 0;
						//XferY++;
						XferX = xStart;
						XferY += yInc;
					}
				
				
				// don't go past the number of pixels available to read
				Count--;
			}
		}
	//}

}


static u64 Select_CopyLocal_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	u32 SPSM, DPSM;	//DATE, ZMSK, ZTST, ATST;
	u32 Combine;
	
	u64 NumPixels;
				
	// inputbuffer
	// 0: BITBLTBUF
	// 1: TRXPOS
	// 2: TRXREG
	// 3: 
	// 4: 
	// 5: 
	// 6: 
	// 7: 
	// 8: 
	// 9: 
	// 10: 
	// 11: 
	// 12: 
	// 13: 
	// 14: 
	// -------------
	// 15: PRIM (COMMAND)
	// 16: 
	// 17: 
	// 18: 
	// 19: 
	// 20: 
	// 21: 
	// 22: 
	// 23:
	// 24: 
	// 25: 
	// 26: 
	// 27: 
	
//cout << "\nhps2x64: ALERT: BUFFER COPY";
	
	if ( !ulThreadNum )
	{
		p_inputbuffer [ 0 ] = _GPU->GPURegsGp.BITBLTBUF.Value;
		p_inputbuffer [ 1 ] = _GPU->GPURegsGp.TRXPOS.Value;
		p_inputbuffer [ 2 ] = _GPU->GPURegsGp.TRXREG.Value;
		
		NumPixels = _GPU->GPURegsGp.TRXREG.RRW * _GPU->GPURegsGp.TRXREG.RRH;
	
		// now that the data is set, if this is multi-threaded, then we are done if we are not on the GPU thread
		if ( _GPU->ulNumberOfThreads )
		{
			// set the command
			p_inputbuffer [ 15 ] = 0x7;
			p_inputbuffer [ 14 ] = 0x1;
	
			ulInputBuffer_WriteIndex++;
			return NumPixels;
		}
	}
				
	
	// 000000: PSMCT32, 000001: PSMCT24, 000010: PSMCT16, 001010: PSMCT16S
	// 110000: PSMZ32, 110001: PSMZ24, 110010: PSMZ16, 111010: PSMZ16S
	SPSM = ( p_inputbuffer [ 0 ] >> 24 ) & 0x3f;
	DPSM = ( p_inputbuffer [ 0 ] >> 56 ) & 0x3f;

	
	Combine = ( SPSM << 8 ) | DPSM;
	
	switch ( Combine )
	{
		// 32-bit transfers
		// 24-bit transfers
		
		case 0x0000:
			//Render_Generic_CopyLocal_t<SPSM,DPSM> ( p_inputbuffer, ulThreadNum )
			Render_Generic_CopyLocal_t<0x00,0x00> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x3030:
			Render_Generic_CopyLocal_t<0x30,0x30> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x0101:
			Render_Generic_CopyLocal_t<0x01,0x01> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x3131:
			Render_Generic_CopyLocal_t<0x31,0x31> ( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x0030:
			Render_Generic_CopyLocal_t<0x00,0x30> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x0001:
			Render_Generic_CopyLocal_t<0x00,0x01> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x0031:
			Render_Generic_CopyLocal_t<0x00,0x31> ( p_inputbuffer, ulThreadNum );
			break;

		case 0x3000:
			Render_Generic_CopyLocal_t<0x30,0x00> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x3001:
			Render_Generic_CopyLocal_t<0x30,0x01> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x3031:
			Render_Generic_CopyLocal_t<0x30,0x31> ( p_inputbuffer, ulThreadNum );
			break;

		case 0x0100:
			Render_Generic_CopyLocal_t<0x01,0x00> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x0130:
			Render_Generic_CopyLocal_t<0x01,0x30> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x0131:
			Render_Generic_CopyLocal_t<0x01,0x31> ( p_inputbuffer, ulThreadNum );
			break;

		case 0x3100:
			Render_Generic_CopyLocal_t<0x31,0x00> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x3101:
			Render_Generic_CopyLocal_t<0x31,0x01> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x3130:
			Render_Generic_CopyLocal_t<0x31,0x30> ( p_inputbuffer, ulThreadNum );
			break;
			
			
		// 16-bit transfers
		case 0x0202:
			Render_Generic_CopyLocal_t<0x02,0x02> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x0a0a:
			Render_Generic_CopyLocal_t<0x0a,0x0a> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x3232:
			Render_Generic_CopyLocal_t<0x32,0x32> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x3a3a:
			Render_Generic_CopyLocal_t<0x3a,0x3a> ( p_inputbuffer, ulThreadNum );
			break;

		case 0x020a:
			Render_Generic_CopyLocal_t<0x02,0x0a> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x0232:
			Render_Generic_CopyLocal_t<0x02,0x32> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x023a:
			Render_Generic_CopyLocal_t<0x02,0x3a> ( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x0a02:
			Render_Generic_CopyLocal_t<0x0a,0x02> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x0a32:
			Render_Generic_CopyLocal_t<0x0a,0x32> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x0a3a:
			Render_Generic_CopyLocal_t<0x0a,0x3a> ( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x3202:
			Render_Generic_CopyLocal_t<0x32,0x02> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x320a:
			Render_Generic_CopyLocal_t<0x32,0x0a> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x323a:
			Render_Generic_CopyLocal_t<0x32,0x3a> ( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x3a02:
			Render_Generic_CopyLocal_t<0x3a,0x02> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x3a0a:
			Render_Generic_CopyLocal_t<0x3a,0x0a> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x3a32:
			Render_Generic_CopyLocal_t<0x3a,0x32> ( p_inputbuffer, ulThreadNum );
			break;
			
			
		// 8-bit transfers
		case 0x1313:
			Render_Generic_CopyLocal_t<0x13,0x13> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x1b1b:
			Render_Generic_CopyLocal_t<0x1b,0x1b> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x131b:
			Render_Generic_CopyLocal_t<0x13,0x1b> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x1b13:
			Render_Generic_CopyLocal_t<0x1b,0x13> ( p_inputbuffer, ulThreadNum );
			break;
			
			
		// 4-bit transfers
		case 0x1414:
			Render_Generic_CopyLocal_t<0x14,0x14> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x2424:
			Render_Generic_CopyLocal_t<0x24,0x24> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x2c2c:
			Render_Generic_CopyLocal_t<0x2c,0x2c> ( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x1424:
			Render_Generic_CopyLocal_t<0x14,0x24> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x142c:
			Render_Generic_CopyLocal_t<0x14,0x2c> ( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x2414:
			Render_Generic_CopyLocal_t<0x24,0x14> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x242c:
			Render_Generic_CopyLocal_t<0x24,0x2c> ( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x2c14:
			Render_Generic_CopyLocal_t<0x2c,0x14> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x2c24:
			Render_Generic_CopyLocal_t<0x2c,0x24> ( p_inputbuffer, ulThreadNum );
			break;
			
		default:
			cout << "\nhps2x64: GPU ALERT: Bad local buffer copy combination: SPSM=" << hex << SPSM << " DPSM=" << DPSM;
			break;
	}
	
	return NumPixels;
}

#endif



#ifdef USE_TEMPLATES_PS2_DRAWSCREEN

template<const long DISPFBX_PSM,const long SMODE2_FFMD,const long SMODE2_INTER>
static void Draw_Screen_th ( u64* p_inputbuffer, u32 ulThreadNum )
{
	static const int c_iVisibleArea_StartY [] = { c_iVisibleArea_StartY_Pixel_NTSC, c_iVisibleArea_StartY_Pixel_PAL };
	static const int c_iVisibleArea_EndY [] = { c_iVisibleArea_EndY_Pixel_NTSC, c_iVisibleArea_EndY_Pixel_PAL };
	
	
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\n***Frame Draw***";
	debug << " PMode=" << hex << GPURegs0.PMODE;
	debug << " FBOffset1/2048=" << hex << GPURegs0.DISPFB1.FBP;
	debug << " FBOffset2/2048=" << hex << GPURegs0.DISPFB2.FBP;
	debug << " FBWidth1/64 (for screen output/in pixels)=" << dec << GPURegs0.DISPFB1.FBW;
	debug << " FBWidth2/64 (for screen output/in pixels)=" << dec << GPURegs0.DISPFB2.FBW;
	debug << " FBHeight1-1 (for screen output/in pixels)=" << dec << GPURegs0.DISPLAY1.DH;
	debug << " FBHeight2-1 (for screen output/in pixels)=" << dec << GPURegs0.DISPLAY2.DH;
	debug << " XPixel1=" << dec << GPURegs0.DISPFB1.DBX;
	debug << " YPixel1=" << dec << GPURegs0.DISPFB1.DBY;
	debug << " XPixel2=" << dec << GPURegs0.DISPFB2.DBX;
	debug << " YPixel2=" << dec << GPURegs0.DISPFB2.DBY;
	debug << " PixelFormat=" << PixelFormat_Names [ GPURegs0.DISPFB2.PSM ];
#endif

	int x, y;
	
	u32* buf_ptr;
	u32 *buf_ptr32;
	u16 *buf_ptr16;
	s32 draw_buffer_offset;
	s32 draw_width, draw_height;
	s32 start_x, start_y;
	s32 Index = 0;
	
	u32 Pixel16, Pixel32;
	//u32 PixelFormat;
	
	u64 DISPFBX, DISPLAYX;
	
	u32 DISPFBX_FBP, DISPFBX_FBW, DISPLAYX_DH, DISPFBX_DBX, DISPFBX_DBY;
	u32 lScanline;
	
	lScanline = p_inputbuffer [ 13 ];

	DISPFBX = p_inputbuffer [ 11 ];
	DISPLAYX = p_inputbuffer [ 12 ];
	
	DISPFBX_FBP = ( DISPFBX >> 0 ) & 0x1ff;
	DISPFBX_FBW = ( DISPFBX >> 9 ) & 0x3f;
	DISPFBX_DBX = ( DISPFBX >> 32 ) & 0x7ff;
	DISPFBX_DBY = ( DISPFBX >> 43 ) & 0x7ff;
	
	DISPLAYX_DH = ( DISPLAYX >> 44 ) & 0x7ff;
	
	
	draw_buffer_offset = DISPFBX_FBP << 11;
	
	buf_ptr = & ( _GPU->RAM32 [ draw_buffer_offset ] );
	
	draw_width = DISPFBX_FBW << 6;
	draw_height = DISPLAYX_DH + 1;
	start_x = DISPFBX_DBX;
	start_y = DISPFBX_DBY;
	
	buf_ptr32 = buf_ptr;
	buf_ptr16 = (u16*) buf_ptr;
	
	/*
	// make sure that framebuffer has some width and height to it before drawing it
	if ( ( PMODE & 1 ) && DISPFB1_FBW && DISPLAY1_DH )
	{
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << " DISPLAY1";
#endif

		// display 1 //
		
		draw_buffer_offset = GPURegs0.DISPFB1.FBP << 11;
		
		//buf_ptr = & ( RAM32 [ GPURegs0.DISPFB1.FBP >> 4 ] );
		buf_ptr = & ( RAM32 [ draw_buffer_offset ] );
		
		draw_width = GPURegs0.DISPFB1.FBW << 6;
		draw_height = GPURegs0.DISPLAY1.DH + 1;
		start_x = GPURegs0.DISPFB1.DBX;
		start_y = GPURegs0.DISPFB1.DBY;
		
		buf_ptr32 = buf_ptr;
		buf_ptr16 = (u16*) buf_ptr;
		
		PixelFormat = GPURegs0.DISPFB1.PSM;
	}
	else if ( GPURegs0.PMODE & 2 )
	{
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << " DISPLAY2";
#endif

		// display 2 //
		
		draw_buffer_offset = GPURegs0.DISPFB2.FBP << 11;
		
		//buf_ptr = & ( RAM32 [ GPURegs0.DISPFB2.FBP >> 4 ] );
		buf_ptr = & ( RAM32 [ draw_buffer_offset ] );
		
		draw_width = GPURegs0.DISPFB2.FBW << 6;
		draw_height = GPURegs0.DISPLAY2.DH + 1;
		start_x = GPURegs0.DISPFB2.DBX;
		start_y = GPURegs0.DISPFB2.DBY;
		
		buf_ptr32 = buf_ptr;
		buf_ptr16 = (u16*) buf_ptr;
		
		PixelFormat = GPURegs0.DISPFB2.PSM;
	}
	else
	{
		// ???
		// todo: probably should black out screen in this case ??
		return;
	}
	*/
	
	// draw height can only be a maximum of 1024
	draw_height = ( ( draw_height > c_iScreen_MaxHeight ) ? c_iFrameBuffer_DisplayHeight : draw_height );
	
	// draw width can only be a maximum of 1024
	draw_width = ( ( draw_width > c_iScreen_MaxWidth ) ? c_iFrameBuffer_DisplayWidth : draw_width );

	
#ifdef ENABLE_PIXELBUF_INTERLACING
	// if set to read every line, then half draw height for now
	// *todo* need to take into account whether interlaced or not
	// comment this out to show correct screen at top for now
	//if ( GPURegs0.SMODE2.FFMD ) draw_height >>= 1;
	// check if this is set to read every line
	if ( SMODE2_FFMD && SMODE2_INTER )
	{
		// only draw half the draw height for now
		draw_height >>= 1;
		
		// set to read every line, so need to skip lines when writing to pixel buffer for interlacing
		if ( ( lScanline & 1 ) )
		{
			Index += draw_width;
		}
	}
#endif

	
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << " draw_width=" << dec << draw_width;
	debug << " draw_height=" << dec << draw_height;
	debug << " PixelFormat=" << dec << PixelFormat;
#endif
	
	// draw starting from correct position
	//buf_ptr = buf_ptr [ start_x + ( start_y * draw_width ) ];
	
	// testing
	//start_x = 0;
	//start_y = 0;



	
	if ( DISPFBX_PSM < 2 )
	{
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << " PIXEL32";
#endif

		// 24/32-bit pixels in frame buffer //
		
		//buf_ptr32 = & ( RAM32 [ GPURegs0.DISPFB2.FBP >> 4 ] );
		
		
		// pad on the bottom with zeros if needed for now
		y = 0;
		while ( y < start_y )
		{
			for ( x = 0; x < draw_width; x++ )
			{
				_GPU->PixelBuffer [ Index++ ] = 0;
			}
			
			y++;
		}
		
		// copy the pixels into pixel buffer
		//for ( y = start_y + ( draw_height - 1 ); y >= start_y; y-- )
		for ( y = draw_height - 1; y >= start_y; y-- )
		{
			//for ( int x = start_x; x < ( start_x + draw_width ); x++ )
			for ( x = start_x; x < draw_width; x++ )
			{
				Pixel32 = buf_ptr32 [ CvtAddrPix32( x, y, draw_width ) ];
				
				_GPU->PixelBuffer [ Index++ ] = Pixel32;
			}
			
			// pad on the right with zeros if needed for now
			x = 0;
			while ( x < start_x )
			{
				_GPU->PixelBuffer [ Index++ ] = 0;
				x++;
			}
			
#ifdef ENABLE_PIXELBUF_INTERLACING	
			// if reading every other line, only copy every other line to pixel buffer
			if ( SMODE2_FFMD && SMODE2_INTER )
			{
				Index += draw_width;
			}
#endif

		}
	}
	else
	{
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << " PIXEL16";
#endif

		// assume 16-bit pixels for frame buffer for now //
		
		
		// pad on the bottom with zeros if needed for now
		y = 0;
		while ( y < start_y )
		{
			for ( x = 0; x < draw_width; x++ )
			{
				_GPU->PixelBuffer [ Index++ ] = 0;
			}
			
			y++;
		}
		
		//for ( y = start_y + ( draw_height - 1 ); y >= start_y; y-- )
		for ( y = draw_height - 1; y >= start_y; y-- )
		{
			//for ( x = start_x; x < ( start_x + draw_width ); x++ )
			for ( x = start_x; x < draw_width; x++ )
			{
				switch ( DISPFBX_PSM )
				{
					case 2:
						Pixel16 = buf_ptr16 [ CvtAddrPix16( x, y, draw_width ) ];
						break;
						
					case 0xa:
						Pixel16 = buf_ptr16 [ CvtAddrPix16S( x, y, draw_width ) ];
						break;
				}

				Pixel32 = ( ( Pixel16 & 0x1f ) << 3 ) | ( ( ( Pixel16 >> 5 ) & 0x1f ) << ( 8 + 3 ) ) | ( ( ( Pixel16 >> 10 ) & 0x1f ) << ( 16 + 3 ) );
				_GPU->PixelBuffer [ Index++ ] = Pixel32;
			}
			
			// pad on the right with zeros if needed for now
			x = 0;
			while ( x < start_x )
			{
				_GPU->PixelBuffer [ Index++ ] = 0;
				x++;
			}
			
#ifdef ENABLE_PIXELBUF_INTERLACING	
			// if reading every other line, only copy every other line to pixel buffer
			if ( SMODE2_FFMD && SMODE2_INTER )
			{
				Index += draw_width;
			}
#endif

		}
	}
	
	
#ifdef ENABLE_PIXELBUF_INTERLACING	
	// for now, if interlaced, need to put the draw_height back
	if ( SMODE2_FFMD && SMODE2_INTER )
	{
		// only draw half the draw height for now
		draw_height <<= 1;
		
	}
#endif
	
	
		
	// *** output of pixel buffer to screen *** //

	// make this the current window we are drawing to
	DisplayOutput_Window->OpenGL_MakeCurrentWindow ();
	
	//glPixelZoom ( (float)MainProgramWindow_Width / (float)VisibleArea_Width, (float)MainProgramWindow_Height / (float)VisibleArea_Height );
	glPixelZoom ( (float)MainProgramWindow_Width / (float)draw_width, (float)MainProgramWindow_Height / (float)draw_height );
	//glDrawPixels ( VisibleArea_Width, VisibleArea_Height, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*) PixelBuffer );
	glDrawPixels ( draw_width, draw_height, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*) _GPU->PixelBuffer );
	
	// update screen
	DisplayOutput_Window->FlipScreen ();
	
	// this is no longer the current window we are drawing to
	DisplayOutput_Window->OpenGL_ReleaseWindow ();
	
	
}


static void Select_DrawScreen_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	// inputbuffer
	// 0: PMODE
	// 1: DISPFB1
	// 2: DISPFB2
	// 3: DISPLAY1
	// 4: DISPLAY2
	// 5: SMODE2
	// 6: SMODE1 (?)
	// 7: BGCOLOR
	// 8: EXTBUF
	// 9: EXTDATA
	// 10: EXTWRITE
	// 11: DISPFBX -> set here
	// 12: DISPLAYX -> set here
	// 13: lScanline
	// 14: ADDITIONAL COMMAND
	// -------------
	// 15: PRIM (COMMAND)
	
	u32 PMODE;
	u32 SMODE2;
	u64 DISPFB1, DISPFB2;
	u32 DISPFB1_FBW;
	u32 DISPLAY1_DH;
	u32 DISPFB1_PSM, DISPFB2_PSM, DISPFBX_PSM;
	u32 SMODE2_FFMD, SMODE2_INTER;
	u32 Combine;
	
	if ( !ulThreadNum )
	{
		// set data
		p_inputbuffer [ 0 ] = _GPU->GPURegs0.PMODE;
		p_inputbuffer [ 1 ] = _GPU->GPURegs0.DISPFB1.Value;
		p_inputbuffer [ 2 ] = _GPU->GPURegs0.DISPFB2.Value;
		p_inputbuffer [ 3 ] = _GPU->GPURegs0.DISPLAY1.Value;
		p_inputbuffer [ 4 ] = _GPU->GPURegs0.DISPLAY2.Value;
		p_inputbuffer [ 5 ] = _GPU->GPURegs0.SMODE2.Value;
		p_inputbuffer [ 13 ] = _GPU->lScanline;
	
		// now that the data is set, if this is multi-threaded, then we are done if we are not on the GPU thread
		if ( _GPU->ulNumberOfThreads )
		{
			// set command
			p_inputbuffer [ 15 ] = 7;
			p_inputbuffer [ 14 ] = 0;
			
			ulInputBuffer_WriteIndex++;
			return;
		}
	}
	

	
	SMODE2 = ( p_inputbuffer [ 5 ] >> 0 ) & 0x3;
	
	PMODE = p_inputbuffer [ 0 ];
	
	DISPFB1 = p_inputbuffer [ 1 ];
	
	DISPLAY1_DH = ( p_inputbuffer [ 3 ] >> 44 ) & 0x7ff;
	DISPFB1_FBW = ( DISPFB1 >> 9 ) & 0x3f;
	
	if ( ( PMODE & 1 ) && DISPFB1_FBW && DISPLAY1_DH )
	{
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << " DISPLAY1";
#endif

		// display 1 //
		
		// set DISPFBX
		p_inputbuffer [ 11 ] = DISPFB1;
		
		// set DISPLAYX
		p_inputbuffer [ 12 ] = p_inputbuffer [ 3 ];
		
		DISPFBX_PSM = ( DISPFB1 >> 15 ) & 0x1f;
	}
	else if ( PMODE & 2 )
	{
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << " DISPLAY2";
#endif

		// display 2 //
		
		DISPFB2 = p_inputbuffer [ 2 ];
		
		// set DISPFBX
		p_inputbuffer [ 11 ] = DISPFB2;
		
		// set DISPLAYX
		p_inputbuffer [ 12 ] = p_inputbuffer [ 4 ];
		
		DISPFBX_PSM = ( DISPFB2 >> 15 ) & 0x1f;
	}
	else
	{
		// ***TODO*** should probably make screen all black here or use BGCOLOR register?
		return;
	}


	Combine = ( DISPFBX_PSM << 4 ) | SMODE2;
	
	switch ( Combine )
	{
		case 0x000:
			//template<const long DISPFB_PSM,const long SMODE2_FFMD,const long SMODE2_INTER>
			Draw_Screen_th<0x00,0,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x001:
			Draw_Screen_th<0x00,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x002:
			Draw_Screen_th<0x00,1,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x003:
			Draw_Screen_th<0x00,1,1> ( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x010:
			Draw_Screen_th<0x01,0,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x011:
			Draw_Screen_th<0x01,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x012:
			Draw_Screen_th<0x01,1,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x013:
			Draw_Screen_th<0x01,1,1> ( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x020:
			Draw_Screen_th<0x02,0,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x021:
			Draw_Screen_th<0x02,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x022:
			Draw_Screen_th<0x02,1,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x023:
			Draw_Screen_th<0x02,1,1> ( p_inputbuffer, ulThreadNum );
			break;
			
		case 0x0a0:
			Draw_Screen_th<0x0a,0,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x0a1:
			Draw_Screen_th<0x0a,0,1> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x0a2:
			Draw_Screen_th<0x0a,1,0> ( p_inputbuffer, ulThreadNum );
			break;
		case 0x0a3:
			Draw_Screen_th<0x0a,1,1> ( p_inputbuffer, ulThreadNum );
			break;
			
		default:
			// check for PSGPU-24 format here //
			
			cout << "\nhps2x64: ALERT: GPU: DRAWSCREEN_TH: UNKNOWN FORMAT COMBO: " << hex << Combine << " or PSGPU-24 format?";
			break;
	}
}

#endif


//---------------------------------------------------------------------------------------


#ifdef USE_TEMPLATES_PS2_WRITECLUT

template<const long CLUT_CSM, const long CLUT_PSM, const long PIX_COUNT>
static void WriteInternalCLUT_t ( u64* p_inputbuffer )
{
	// 0: CLUTBufBase32
	// 1: CLUTOffset
	// 2: TEXCLUT
	
	const u32 c_ulPixelCount = ( ( PIX_COUNT == 0x4 ) ? 16 : 256 );
	
	u32 lIndex;
	u32 CLUTBufBase32, CLUTOffset;
	u32 *ptr_clut32;
	u16 *ptr_clut16;
	
	u32 TEXCLUT, TEXCLUT_CBW, TEXCLUT_COU;
	u32 clut_x, clut_y, clut_width;
	u32 bgr;
	
	CLUTBufBase32 = p_inputbuffer [ 0 ];
	CLUTOffset = p_inputbuffer [ 1 ];
	
	// get pointer into CLUT in local memory
	ptr_clut32 = & ( _GPU->RAM32 [ CLUTBufBase32 ] );
	ptr_clut16 = (u16*)ptr_clut32;
	
	if ( !CLUT_CSM )
	{
		// CSM1 //
		//CLUT_LUT = ucCLUT_Lookup_CSM01;
		
		// need to determine size of pixels to transfer into clut
		// need to know if pixels are 16 or 32 bit
		if ( CLUT_PSM & 0x2 )
		{
			// 16-bit pixels //
			
			for ( lIndex = 0; lIndex < c_ulPixelCount; lIndex++ )
			{
				switch ( c_ulPixelCount )
				{
					case 16:
						switch ( CLUT_PSM )
						{
							case 2:
								bgr = ptr_clut16 [ CvtAddrPix16 ( lIndex & 7, lIndex >> 3, 0 ) ];
								break;
								
							case 0xa:
								bgr = ptr_clut16 [ CvtAddrPix16S ( lIndex & 7, lIndex >> 3, 0 ) ];
								break;
						}
							
						break;
						
					case 256:
						switch ( CLUT_PSM )
						{
							case 2:
								bgr = ptr_clut16 [ CvtAddrPix16 ( ( lIndex & 0x7 ) | ( ( lIndex & 0x10 ) >> 1 ), ( ( lIndex >> 4 ) & 0xe ) | ( ( lIndex >> 3 ) & 1 ), 0 ) ];
								break;
								
							case 0xa:
								bgr = ptr_clut16 [ CvtAddrPix16S ( ( lIndex & 0x7 ) | ( ( lIndex & 0x10 ) >> 1 ), ( ( lIndex >> 4 ) & 0xe ) | ( ( lIndex >> 3 ) & 1 ), 0 ) ];
								break;
						}
						
						break;
				}
				
				// 512-entry 16-bit pixels
				_GPU->InternalCLUT [ ( CLUTOffset + lIndex ) & 511 ] = bgr;
			}

		}
		else
		{
//cout << " CLUT_CSM=" << hex << CLUT_CSM << " CLUT_PSM=" << CLUT_PSM << " PIX_COUNT=" << PIX_COUNT << " c_ulPixelCount=" << c_ulPixelCount << " CLUTOffset=" << CLUTOffset << " CLUTBufBase32=" << CLUTBufBase32;
			// 32-bit pixels //
			
			// only 4-bits of Offset are valid
			CLUTOffset &= 255;
			
			// transfer pixels
			for ( lIndex = 0; lIndex < c_ulPixelCount; lIndex++ )
			{
				switch ( c_ulPixelCount )
				{
					case 16:
						bgr = ptr_clut32 [ CvtAddrPix32 ( lIndex & 7, lIndex >> 3, 0 ) ];
						break;
						
					case 256:
						bgr = ptr_clut32 [ CvtAddrPix32 ( ( lIndex & 0x7 ) | ( ( lIndex & 0x10 ) >> 1 ), ( ( lIndex >> 4 ) & 0xe ) | ( ( lIndex >> 3 ) & 1 ), 0 ) ];
						break;
				}
//cout << "\nlIndex#" << dec << lIndex << " bgr=" << hex << bgr;
				// 512-entry 16-bit pixels
				_GPU->InternalCLUT [ ( CLUTOffset + lIndex ) & 511 ] = bgr;
				
				// the upper part of buffer gets the high bits
				_GPU->InternalCLUT [ ( CLUTOffset + lIndex + 256 ) & 511 ] = ( bgr >> 16 );
			}
		}
	}
	else
	{
		// CSM2 //
		//CLUT_LUT = ucCLUT_Lookup_CSM02;
		
		TEXCLUT = p_inputbuffer [ 2 ];
		
		// CBW is in units of pixels/64
		//clut_width = GPURegsGp.TEXCLUT.CBW << 6;
		TEXCLUT_CBW = ( TEXCLUT >> 0 ) & 0x3f;
		clut_width = TEXCLUT_CBW << 6;
		
		// COU is in units of pixels/16
		//clut_x = GPURegsGp.TEXCLUT.COU << 4;
		TEXCLUT_COU = ( TEXCLUT >> 6 ) & 0x3f;
		clut_x = TEXCLUT_COU << 4;
		
		// get clut y in units of pixels
		//clut_y = GPURegsGp.TEXCLUT.COV;
		clut_y = ( TEXCLUT >> 12 ) & 0x3ff;
		
		
		// in CSM2 mode, the size of the pixels is always 16-bit and can only specify PSMCT16 //
		
		
		// 16-bit pixels //
		
		for ( lIndex = 0; lIndex < c_ulPixelCount; lIndex++ )
		{
			bgr = ptr_clut16 [ CvtAddrPix16 ( clut_x + lIndex, clut_y, clut_width ) ];
			
			// 512-entry 16-bit pixels
			_GPU->InternalCLUT [ ( CLUTOffset + lIndex ) & 511 ] = bgr;
		}
	}
}


static void Select_WriteInternalCLUT_t ( u64* p_inputbuffer, u32 ulThreadNum, u32 lContext = 0 )
{
	u32 lCLD, CLUTBufBase32, PixelFormat, CLUTPixelFormat, CLUTOffset, CLUTStoreMode;
	u32 clut_width, clut_x, clut_y;
	u32 lPixelCount, lPixelSize;
	
	u32 x, y;
	
	u32 lIndex, lConvertedIndex;
	u32 bgr;
	
	u16 *ptr_clut16;
	u32 *ptr_clut32;
	
	u16 *CLUT_LUT;
	
	u32 ClutBufWidth;

	u32 TEX_PSM, CLUT_PSM, TEX_CSA, TEX_CLD, TEX_CBP, TEX_CSM;
	
	u32 Combine;
	


	if ( !ulThreadNum )
	{
		
		// set data
		p_inputbuffer [ 0 ] = _GPU->TEXX [ lContext ].Value;
		p_inputbuffer [ 2 ] = _GPU->GPURegsGp.TEXCLUT.Value;
	
		// now that the data is set, if this is multi-threaded, then we are done if we are not on the GPU thread
		if ( _GPU->ulNumberOfThreads )
		{
			// set command
			p_inputbuffer [ 15 ] = 7;
			p_inputbuffer [ 14 ] = 3;
			
			ulInputBuffer_WriteIndex++;
			
			
			return;
		}
	}
	
	
	TEX_PSM = ( p_inputbuffer [ 0 ] >> 20 ) & 0x3f;
	
	// check if psm is an indexed color format that requires CLUT
	if ( ( TEX_PSM & 7 ) <= 2 )
	{
		// this is not an indexed pixel format so has nothing to do with CLUT
		return;
	}
	
	TEX_CSA = ( p_inputbuffer [ 0 ] >> 56 ) & 0x1f;
	TEX_CLD = ( p_inputbuffer [ 0 ] >> 61 ) & 0x7;
	TEX_CBP = ( p_inputbuffer [ 0 ] >> 37 ) & 0x3fff;
	

	// ***TODO*** send palette to internal CLUT
	
	//TEX0_t *TEX0 = &GPURegsGp.TEX0_1;
	

	// clut offset ??
	CLUTOffset = TEX_CSA;
	
//cout << "\nTEX_CLD=" << TEX_CLD;
	// check cld
	switch ( TEX_CLD )
	{
		case 0:
			// do not load into temp CLUT //
			return;
			break;
			
		case 1:
			// always load //
			break;
			
		case 2:
			// load and copy CBP to CBP0 //
			_GPU->CBP0 = TEX_CBP;
			break;
			
		case 3:
			// load and copy CBP to CBP1 //
			_GPU->CBP1 = TEX_CBP;
			break;
			
		case 4:
			// load and copy CBP to CBP0 only if CBP<>CBP0 //
			if ( TEX_CBP == _GPU->CBP0 ) return;
			_GPU->CBP0 = TEX_CBP;
			break;
			
		case 5:
			// load and copy CBP to CBP1 only if CBP<>CBP1 //
			if ( TEX_CBP == _GPU->CBP1 ) return;
			_GPU->CBP1 = TEX_CBP;
			break;
			
		default:
		
#ifdef VERBOSE_INVALID_CLD
			cout << "\nhps2x64: GPU: Invalid CLD value=" << dec << TEX_CLD << " TEX_PSM=" << hex << TEX_PSM;
#endif
			break;
	}
	
	
	// the clut offset is actually CSA times 16 pixels
	CLUTOffset <<= 4;
	
	
	// get base pointer to color lookup table (32-bit word address divided by 64)
	CLUTBufBase32 = TEX_CBP << 6;
	
	CLUT_PSM = ( p_inputbuffer [ 0 ] >> 51 ) & 0xf;
	TEX_CSM = ( p_inputbuffer [ 0 ] >> 55 ) & 0x1;
	
	// get pointer into CLUT in local memory
	//ptr_clut32 = & ( RAM32 [ CLUTBufBase32 ] );
	//ptr_clut16 = (u16*)ptr_clut32;
	p_inputbuffer [ 0 ] = CLUTBufBase32;
	p_inputbuffer [ 1 ] = CLUTOffset;
	
	
	
	// transfer pixels
	
	// need to know if pixels are 4/8-bit
	//if ( ( TEX_PSM & 7 ) > 2 )
	//{
		// will need to write into the internal CLUT
		
		/*
		// get the number of pixels to transfer
		if ( TEX_PSM & 0x4 )
		{
			// 4-bit pixels - 16 colors
			lPixelCount = 16;
			
		}
		else
		{
			// 8-bit pixels - 256 colors
			lPixelCount = 256;
			
		}
		*/
	
		if ( !TEX_CSM )
		{
			//Combine = ( ( CLUT_PSM & 0xf ) << 4 ) | ( TEX_PSM & 0x4 );
			Combine = ( ( CLUT_PSM & 0xa ) << 4 ) | ( TEX_PSM & 0x4 );
			
			switch ( Combine )
			{
				case 0x00:
					// CSM=0, CLUT_PSM=PSMCT32, TEX_PSM=8-bit
					WriteInternalCLUT_t<0x0,0x0,0x0> ( p_inputbuffer );
					break;
				case 0x04:
					// CSM=0, CLUT_PSM=PSMCT32, TEX_PSM=4-bit
					WriteInternalCLUT_t<0x0,0x0,0x4> ( p_inputbuffer );
					break;
					
				case 0x20:
					// CSM=0, CLUT_PSM=PSMCT16, TEX_PSM=8-bit
					WriteInternalCLUT_t<0x0,0x2,0x0> ( p_inputbuffer );
					break;
				case 0x24:
					// CSM=0, CLUT_PSM=PSMCT16, TEX_PSM=4-bit
					WriteInternalCLUT_t<0x0,0x2,0x4> ( p_inputbuffer );
					break;
					
				case 0xa0:
					// CSM=0, CLUT_PSM=PSMCT16S, TEX_PSM=8-bit
					WriteInternalCLUT_t<0x0,0xa,0x0> ( p_inputbuffer );
					break;
				case 0xa4:
					// CSM=0, CLUT_PSM=PSMCT16S, TEX_PSM=4-bit
					WriteInternalCLUT_t<0x0,0xa,0x4> ( p_inputbuffer );
					break;
					
				default:
				
#ifdef VERBOSE_INVALID_COMBINE
					cout << "\nhps2x64: GPU: Invalid CLUT !TEX_CSM Combine=" << hex << Combine << " TEX_PSM=" << TEX_PSM;
#endif
					break;

			}
		}
		else
		{
			Combine = ( TEX_PSM & 0x4 );
			
			switch ( Combine )
			{
				case 0x00:
					// CSM=1, CLUT_PSM=PSMCT16, TEX_PSM=8-bit
					WriteInternalCLUT_t<0x1,0x0,0x0> ( p_inputbuffer );
					break;
				case 0x04:
					// CSM=1, CLUT_PSM=PSMCT16S, TEX_PSM=4-bit
					WriteInternalCLUT_t<0x1,0x0,0x4> ( p_inputbuffer );
					break;
					
				default:
				
#ifdef VERBOSE_INVALID_COMBINE
					cout << "\nhps2x64: GPU: Invalid CLUT TEX_CSM Combine=" << hex << Combine << " TEX_PSM=" << TEX_PSM;
#endif
					break;
			}
		}
		
	//}


}

#endif

//---------------------------------------------------------------------------------------


#ifdef USE_TEMPLATES_PS2_TRANSFERIN


template<const long tBITBLTBUF_DPSM>
void GPU::TransferDataIn32_DS_t ( u32* Data, u32 WordCount32 )
{
#ifdef INLINE_DEBUG_TRANSFER_IN
	if ( !XferX && !XferY )
	{
		debug << "\r\nTransferIn: ";
		debug << dec << " WC=" << WordCount32;
		debug << dec << " Width=" << XferWidth << " Height=" << XferHeight;
		debug << hex << " DESTPTR32/64=" << GPURegsGp.BITBLTBUF.DBP;
		//debug << " InPixFmt=" << PixelFormat_Names [ GPURegsGp.BITBLTBUF.SPSM ];
		debug << dec << " DestBufWidth=" << XferDstBufWidth;
		debug << " OutPixFmt=" << PixelFormat_Names [ GPURegsGp.BITBLTBUF.DPSM ];
		debug << " TransferDir=" << TransferDir_Names [ GPURegsGp.TRXPOS.DIR ];
		debug << dec << " XferX=" << XferX << " XferY=" << XferY;
		debug << dec << " XferDstX=" << XferDstX << " XferDstY=" << XferDstY;
		debug << " @Cycle#" << dec << *_DebugCycleCount;
		
#ifdef VERBOSE_TRANSFER_IN
		cout << "\nhps2x64: ALERT: GPU: Transferring data TO gpu FROM memory";
#endif
	}
#endif

#ifdef INLINE_DEBUG_TRANSFER_IN_2
	debug << "\r\n";
	debug << " XferX=" << dec << XferX << " XferY=" << XferY;
	debug << " WC=" << WordCount32 << " Offset=" << ( ( XferDstOffset32 + XferX + ( XferY * XferDstBufWidth ) ) << 2 );
	//debug << " XferDstX=" << XferDstX << " XferDstY=" << XferDstY;
#endif

	
	//u64 PixelShift
	u64 PixelLoad;
	u32 Pixel0;
	//u64 PixelCount;

	// get pointer to dest buffer
	//u32 *DestBuffer;
	
	u32 *Data32;
	u16 *Data16;
	u8 *Data8;
	
	u32 *buf32;
	u16 *buf16;
	u8 *buf8;
	
	u32 *DestBuffer32;
	u16 *DestBuffer16;
	u8 *DestBuffer8;
	
	u32 Offset;
	
	void *PtrEnd;
	PtrEnd = RAM8 + c_iRAM_Size;
	
	
	
	
	
	// make sure transfer method is set to cpu->gpu
	/*
	if ( GPURegsGp.TRXDIR.XDIR != 0 )
	{
		cout << "\nhps2x64: ALERT: GPU: Performing mem->gpu transmission while not activated";
		
#ifdef INLINE_DEBUG_TRANSFER_IN
		debug << "\r\nhps2x64: ALERT: GPU: Performing mem->gpu transmission while not activated";
#endif
	}
	*/

	
	if ( !XferDstBufWidth )
	{
		if ( !XferX && !XferY )
		{
#ifdef INLINE_DEBUG_TRANSFER_IN
	debug << "\r\nGPU: ERROR: Transfer Dest Buf Width is ZERO!!!\r\n";
#endif

			cout << "\nhps2x64: GPU: ERROR: Transfer Dest Buf Width is ZERO!!!\n";
		}
		
		return;
	}
	
	// check that there is data to transfer
	if ( !XferWidth || !XferHeight ) return;
	
	
	// if the X specified is greater than buffer width, then modulo
	//if ( XferDstX >= XferDstBufWidth ) XferDstX %= XferDstBufWidth;
	
	buf32 = & ( RAM32 [ XferDstOffset32 ] );
	buf16 = (u16*) buf32;
	buf8 = (u8*) buf32;
	
	// check if source pixel format is 24-bit
	// this is transferring TO GPU, so must check the destination for the correct value
	if ( ( tBITBLTBUF_DPSM & 7 ) == 1 )
	{
		// 24-bit pixels //
		
		if ( !XferX && !XferY )
		{
			// first transfer for PSMCT24, PSMZ24 //
				
			// PSMCT24, PSMZ24 //
			
			// Limitations for PSMCT24, PSMZ24
			// no limitation on start x-coord
			// width must be a multiple of 8
			if ( XferWidth & 7 )
			{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL_LIMIT
				debug << "\r\n***PSMCT24/PSMZ24 Transfer width not multiple of 8. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL_LIMIT
				cout << "\nhps2x64: ALERT: GPU: PSMCT24/PSMZ24 Transfer width not multiple of 8. Cycle#" << dec << *_DebugCycleCount;
#endif
			} // end if ( XferWidth & 7 )
			
					
				
			// check if this transfers outside of GPU memory device
			if ( ( ( XferDstOffset32 + XferWidth + ( XferHeight * XferDstBufWidth ) ) << 2 ) > c_iRAM_Size )
			{
#ifdef INLINE_DEBUG_TRANSFER_IN
				debug << "\r\n***Transfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_IN
				cout << "\nTransfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif
			}
		}
		
		
		DestBuffer32 = & ( buf32 [ ( ( XferX + XferDstX ) + ( ( XferY + XferDstY ) * XferDstBufWidth ) ) ] );
		Data32 = Data;
		
		//PixelCount = 0;
		//PixelShift = 0;
		
		while ( ( XferY < XferHeight ) && ( WordCount32 || ( PixelCount >= 3 ) ) )
		{
			// check if you need to load
			if ( PixelCount < 3 )
			{
				// load next data
				PixelLoad = *Data32++;
				
				WordCount32--;
				
				// put into pixel
				PixelShift |= ( PixelLoad << ( PixelCount << 3 ) );
				
				PixelCount += 4;
			}
				
			// get pixel
			Pixel0 = PixelShift & 0xffffff;
			
			// have to keep pixels in GPU buffer for now
			//DestBuffer32 = & ( buf32 [ CvtAddrPix32 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
			switch ( tBITBLTBUF_DPSM )
			{
				// PSMCT24
				case 1:
					DestBuffer32 = & ( buf32 [ CvtAddrPix32 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
					break;
					
				// PSMZ24
				case 0x31:
					DestBuffer32 = & ( buf32 [ CvtAddrZBuf32 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
					break;
			}
			
			if ( DestBuffer32 < PtrEnd )
			{
			
				// transfer pixel
				// *DestBuffer32++ = ( Pixel0 & 0xffffff ) | ( ( *DestBuffer32 ) & ~0xffffff );
				*DestBuffer32 = ( Pixel0 & 0xffffff ) | ( ( *DestBuffer32 ) & ~0xffffff );
			}
			
			// shift (24-bit pixels)
			PixelShift >>= 24;
			
			// update pixel count
			PixelCount -= 3;
			
			// update x
			XferX++;
			
			// check if greater than width
			if ( XferX >= XferWidth )
			{
				// go to next line
				XferX = 0;
				XferY++;
				
				// set buffer pointer
				//DestBuffer32 = & ( buf32 [ ( ( XferX + XferDstX ) + ( ( XferY + XferDstY ) * XferDstBufWidth ) ) ] );
			}
			
		}
	}
	else
	{
		if ( ( tBITBLTBUF_DPSM & 7 ) == 0 )
		{
			// 32-bit pixels //
			
			if ( !XferX && !XferY )
			{
				// PSMCT32, PSMZ32 //
				
				// Limitations for PSMCT32, PSMZ32
				// no limitation on start x-coord
				// width must be a multiple of 2
				if ( XferWidth & 1 )
				{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL_LIMIT
					debug << "\r\n***PSMCT32/PSMZ32 Transfer width not multiple of 2. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL_LIMIT
					cout << "\nhps2x64: ALERT: GPU: PSMCT32/PSMZ32 Transfer width not multiple of 2. Cycle#" << dec << *_DebugCycleCount;
#endif
				} // end if ( XferWidth & 1 )
				
				// check if this transfers outside of GPU memory device
				if ( ( ( XferDstOffset32 + XferWidth + ( XferHeight * XferDstBufWidth ) ) << 2 ) > c_iRAM_Size )
				{
#ifdef INLINE_DEBUG_TRANSFER_IN
				debug << "\r\n***Transfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_IN
				cout << "\nTransfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif
				}
			}
			
			
			// 32-bit pixels
			//DestBuffer32 = & ( buf32 [ ( ( XferX + XferDstX ) + ( ( XferY + XferDstY ) * XferDstBufWidth ) ) /* & ( c_iRAM_Mask >> 2 ) */ ] );
			Data32 = Data;
			
			while ( ( XferY < XferHeight ) && WordCount32 )
			{
				// have to keep pixels in GPU buffer for now
				//DestBuffer32 = & ( buf32 [ CvtAddrPix32 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
				switch ( tBITBLTBUF_DPSM )
				{
					// PSMCT32
					case 0:
						DestBuffer32 = & ( buf32 [ CvtAddrPix32 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						break;
						
					// PSMZ32
					case 0x30:
						DestBuffer32 = & ( buf32 [ CvtAddrZBuf32 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						break;
				}
			
				if ( DestBuffer32 < PtrEnd )
				{
					Pixel0 = *Data32++;
					

					// transfer a pixel
					// *DestBuffer32++ = *Data32++;
					*DestBuffer32++ = Pixel0;
				}
				
				// update x
				XferX++;
				
				
				// check if greater than width
				if ( XferX >= XferWidth )
				{
					// go to next line
					XferX = 0;
					XferY++;
					
					// set buffer pointer
					//DestBuffer32 = & ( buf32 [ ( ( XferX + XferDstX ) + ( ( XferY + XferDstY ) * XferDstBufWidth ) ) ] );
				}
				
				
				// don't go past the number of pixels available to read
				WordCount32--;
			}
		}
		else if ( ( tBITBLTBUF_DPSM & 7 ) == 2 )
		{
			// 16-bit pixels //
			
			if ( !XferX && !XferY )
			{
					// Limitations for PSMCT16, PSMZ16
					// no limitation on start x-coord
					// width must be a multiple of 4
					if ( XferWidth & 3 )
					{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL_LIMIT
						debug << "\r\n***PSMCT16/PSMZ16 Transfer width not multiple of 4. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL_LIMIT
						cout << "\nhps2x64: ALERT: GPU: PSMCT16/PSMZ16 Transfer width not multiple of 4. Cycle#" << dec << *_DebugCycleCount;
#endif
					} // end if ( XferWidth & 3 )
					
				// check if this transfers outside of GPU memory device
				if ( ( ( XferDstOffset32 + ( ( XferWidth + ( XferHeight * XferDstBufWidth ) ) >> 1 ) ) << 2 ) > c_iRAM_Size )
				{
#ifdef INLINE_DEBUG_TRANSFER_IN
				debug << "\r\n***Transfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_IN
				cout << "\nTransfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif
				}
			}
			
			
			// 16-bit pixels //
			DestBuffer16 = & ( buf16 [ ( ( XferX + XferDstX ) + ( ( XferY + XferDstY ) * XferDstBufWidth ) ) ] );
			Data16 = (u16*) Data;
			
			// 2 times the pixels
			WordCount32 <<= 1;
			
			while ( ( XferY < XferHeight ) && WordCount32 )
			{
			
				// have to keep pixels in GPU buffer for now
				//DestBuffer16 = & ( buf16 [ ( ( XferX + XferDstX ) + ( ( XferY + XferDstY ) * XferDstBufWidth ) ) ] );
				switch ( tBITBLTBUF_DPSM )
				{
					// PSMCT16
					case 2:
						DestBuffer16 = & ( buf16 [ CvtAddrPix16 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						break;
					
					// PSMCT16S
					case 0xa:
						DestBuffer16 = & ( buf16 [ CvtAddrPix16S ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						break;
						
					// PSMZ16
					case 0x32:
						DestBuffer16 = & ( buf16 [ CvtAddrZBuf16 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						break;
						
					// PSMZ16S
					case 0x3a:
						DestBuffer16 = & ( buf16 [ CvtAddrZBuf16S ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						break;
				}
				
				if ( DestBuffer16 < PtrEnd )
				{
					Pixel0 = *Data16++;
					

					// transfer a pixel
					// *DestBuffer16++ = *Data16++;
					*DestBuffer16++ = Pixel0;
				}
				
				// update x
				XferX++;
				
				
				// check if greater than width
				if ( XferX >= XferWidth )
				{
					// go to next line
					XferX = 0;
					XferY++;
					
					// set buffer pointer
					//DestBuffer16 = & ( buf16 [ ( ( XferX + XferDstX ) + ( ( XferY + XferDstY ) * XferDstBufWidth ) ) ] );
				}
				
				
				// don't go past the number of pixels available to read
				WordCount32--;
			}
		}
		else if ( ( tBITBLTBUF_DPSM & 7 ) == 3 )
		{
			// 8-bits per pixel //
			
			if ( !XferX && !XferY )
			{
					// Limitations for PSMT8, PSMT8H
					// start x must be a multiple of 2
					if ( XferDstX & 1 )
					{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL_LIMIT
						debug << "\r\n***PSMT8/PSMT8H Transfer DstX not multiple of 2. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL_LIMIT
						cout << "\nhps2x64: ALERT: GPU: PSMT8/PSMT8H Transfer DstX not multiple of 2. Cycle#" << dec << *_DebugCycleCount;
#endif
					}
					
					// width must be a multiple of 8
					if ( XferWidth & 7 )
					{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL_LIMIT
						debug << "\r\n***PSMT8/PSMT8H Transfer width not multiple of 8. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL_LIMIT
						cout << "\nhps2x64: ALERT: GPU: PSMT8/PSMT8H Transfer width not multiple of 8. Cycle#" << dec << *_DebugCycleCount;
#endif
					} // end if ( XferWidth & 7 )
					
				// check if this transfers outside of GPU memory device
				if ( ( ( XferDstOffset32 + ( ( XferWidth + ( XferHeight * XferDstBufWidth ) ) >> 2 ) ) << 2 ) > c_iRAM_Size )
				{
#ifdef INLINE_DEBUG_TRANSFER_IN
				debug << "\r\n***Transfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_IN
				cout << "\nTransfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif
				}
			}
			
			
			// 8-bit pixels //
			DestBuffer8 = & ( buf8 [ ( ( XferX + XferDstX ) + ( ( XferY + XferDstY ) * XferDstBufWidth ) ) ] );
			Data8 = (u8*) Data;
			
			// 4 times the pixels
			WordCount32 <<= 2;
			
			while ( ( XferY < XferHeight ) && WordCount32 )
			{
				switch ( tBITBLTBUF_DPSM )
				{
					// PSMT8
					case 0x13:
						// have to keep pixels in GPU buffer for now
						//DestBuffer8 = & ( buf8 [ ( ( XferX + XferDstX ) + ( ( XferY + XferDstY ) * XferDstBufWidth ) ) ] );
						DestBuffer8 = & ( buf8 [ CvtAddrPix8 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						
						// transfer a pixel
						if ( DestBuffer8 < PtrEnd ) *DestBuffer8++ = *Data8++;
						break;
						
					// PSMT8H
					case 0x1b:
						DestBuffer32 = & ( buf32 [ CvtAddrPix32 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						
						PixelLoad = *DestBuffer32;
						PixelLoad = ( PixelLoad & ~( 0xff << 24 ) ) | ( ( (u32) ( *Data8 ) ) << 24 );
						
						// transfer a pixel
						if ( DestBuffer32 < PtrEnd ) *DestBuffer32 = PixelLoad;
						
						// go to next source pixel
						Data8++;
						
						break;
				}
			
				
				// update x
				XferX++;
				
				
				// check if greater than width
				if ( XferX >= XferWidth )
				{
					// go to next line
					XferX = 0;
					XferY++;
					
					// set buffer pointer
					//DestBuffer8 = & ( buf8 [ ( ( XferX + XferDstX ) + ( ( XferY + XferDstY ) * XferDstBufWidth ) ) ] );
				}
				
				
				// don't go past the number of pixels available to read
				WordCount32--;
			}
		}
		else if ( ( tBITBLTBUF_DPSM & 7 ) == 4 )
		{
			// 4-bits per pixel //
			
			if ( !XferX && !XferY )
			{
					// Limitations for PSMT4, PSMT4HL, PSMT4HH
					// start x must be a multiple of 4
					if ( XferDstX & 3 )
					{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL_LIMIT
						debug << "\r\n***PSMT4/PSMT4HL/PSMT4HH Transfer DstX not multiple of 2. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL_LIMIT
						cout << "\nhps2x64: ALERT: GPU: PSMT4/PSMT4HL/PSMT4HH Transfer DstX not multiple of 2. Cycle#" << dec << *_DebugCycleCount;
#endif
					}
					
					// width must be a multiple of 8
					if ( XferWidth & 7 )
					{
#ifdef INLINE_DEBUG_TRANSFER_LOCAL_LIMIT
						debug << "\r\n***PSMT4/PSMT4HL/PSMT4HH Transfer width not multiple of 8. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_LOCAL_LIMIT
						cout << "\nhps2x64: ALERT: GPU: PSMT4/PSMT4HL/PSMT4HH Transfer width not multiple of 8. Cycle#" << dec << *_DebugCycleCount;
#endif
					} // end if ( XferWidth & 7 )
					
				// check if this transfers outside of GPU memory device
				if ( ( ( XferDstOffset32 + ( ( XferWidth + ( XferHeight * XferDstBufWidth ) ) >> 3 ) ) << 2 ) > c_iRAM_Size )
				{
#ifdef INLINE_DEBUG_TRANSFER_IN
				debug << "\r\n***Transfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif

#ifdef VERBOSE_TRANSFER_IN
				cout << "\nTransfer will extend outside GPU RAM. Cycle#" << dec << *_DebugCycleCount;
#endif
				}
			}
			
			
			// 4-bit pixels //
			DestBuffer8 = & ( buf8 [ ( ( ( XferX + XferDstX ) >> 1 ) + ( ( XferY + XferDstY ) * ( XferDstBufWidth >> 1 ) ) ) ] );
			Data8 = (u8*) Data;
			
			// 8 times the pixels, but transferring 2 at a time, so times 4
			WordCount32 <<= 2;
			
			while ( ( XferY < XferHeight ) && WordCount32 )
			{
				
				//if ( DestBuffer8 < PtrEnd )
				//{
					// get the first pixel
					PixelShift = ( *Data8 ) & 0xf;

				switch ( tBITBLTBUF_DPSM )
				{
					// PSMT4
					case 0x14:
						// have to keep pixels in GPU buffer for now
						//DestBuffer8 = & ( buf8 [ ( ( ( XferX + XferDstX ) >> 1 ) + ( ( XferY + XferDstY ) * ( XferDstBufWidth >> 1 ) ) ) ] );
						Offset = CvtAddrPix4 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth );
						DestBuffer8 = & ( buf8 [ Offset >> 1 ] );
						
						Offset = ( Offset & 1 ) << 2;
						PixelLoad = *DestBuffer8;
						PixelLoad = ( PixelLoad & ~( 0xf << Offset ) ) | ( PixelShift << Offset );
						
						// transfer a pixel
						// *DestBuffer8++ = *Data8++;
						if ( DestBuffer8 < PtrEnd ) *DestBuffer8 = PixelLoad;
						break;
						
					// PSMT4HL
					case 0x24:
						DestBuffer32 = & ( buf32 [ CvtAddrPix32 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						
						PixelLoad = *DestBuffer32;
						PixelLoad = ( PixelLoad & ~( 0xf << 24 ) ) | ( PixelShift << 24 );
						if ( DestBuffer32 < PtrEnd ) *DestBuffer32 = PixelLoad;
						break;
						
					// PSMT4HH
					case 0x2c:
						DestBuffer32 = & ( buf32 [ CvtAddrPix32 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						
						PixelLoad = *DestBuffer32;
						PixelLoad = ( PixelLoad & ~( 0xf << 28 ) ) | ( PixelShift << 28 );
						if ( DestBuffer32 < PtrEnd ) *DestBuffer32 = PixelLoad;
						break;
				}
					
					
					// update x
					XferX++;
					
				// check if greater than width
				if ( XferX >= XferWidth )
				{
					// go to next line
					XferX = 0;
					XferY++;
					
					// set buffer pointer
					//DestBuffer8 = & ( buf8 [ ( ( ( XferX + XferDstX ) >> 1 ) + ( ( XferY + XferDstY ) * ( XferDstBufWidth >> 1 ) ) ) ] );
				}
				
					// get the second pixel
					PixelShift = ( ( *Data8 ) >> 4 ) & 0xf;
					
					// do the second pixel
				switch ( tBITBLTBUF_DPSM )
				{
					// PSMT4
					case 0x14:
						// have to keep pixels in GPU buffer for now
						//DestBuffer8 = & ( buf8 [ ( ( ( XferX + XferDstX ) >> 1 ) + ( ( XferY + XferDstY ) * ( XferDstBufWidth >> 1 ) ) ) ] );
						Offset = CvtAddrPix4 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth );
						DestBuffer8 = & ( buf8 [ Offset >> 1 ] );
						
						Offset = ( Offset & 1 ) << 2;
						PixelLoad = *DestBuffer8;
						PixelLoad = ( PixelLoad & ~( 0xf << Offset ) ) | ( PixelShift << Offset );
						
						// transfer a pixel
						// *DestBuffer8++ = *Data8++;
						if ( DestBuffer8 < PtrEnd ) *DestBuffer8 = PixelLoad;
						break;
						
					// PSMT4HL
					case 0x24:
						DestBuffer32 = & ( buf32 [ CvtAddrPix32 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						
						PixelLoad = *DestBuffer32;
						PixelLoad = ( PixelLoad & ~( 0xf << 24 ) ) | ( PixelShift << 24 );
						if ( DestBuffer32 < PtrEnd ) *DestBuffer32 = PixelLoad;
						break;
						
					// PSMT4HH
					case 0x2c:
						DestBuffer32 = & ( buf32 [ CvtAddrPix32 ( XferX + XferDstX, XferY + XferDstY, XferDstBufWidth ) ] );
						
						PixelLoad = *DestBuffer32;
						PixelLoad = ( PixelLoad & ~( 0xf << 28 ) ) | ( PixelShift << 28 );
						if ( DestBuffer32 < PtrEnd ) *DestBuffer32 = PixelLoad;
						break;
				}
					
					// update source pointer to next byte
					Data8++;
				//}
				
				// update x
				//XferX++;
				XferX++;
				
				// check if greater than width
				if ( XferX >= XferWidth )
				{
					// go to next line
					XferX = 0;
					XferY++;
					
					// set buffer pointer
					//DestBuffer8 = & ( buf8 [ ( ( ( XferX + XferDstX ) >> 1 ) + ( ( XferY + XferDstY ) * ( XferDstBufWidth >> 1 ) ) ) ] );
				}
				
				
				// don't go past the number of pixels available to read
				WordCount32--;
			}
		}
	}
}




void Select_Start_Transfer_t ( u64* p_inputbuffer, u32 ulThreadNum, u32 TRXDIR = 0 )
{

	u32 BITBLTBUF_SBP, BITBLTBUF_SBW, BITBLTBUF_DBP, BITBLTBUF_DBW;
	u32 TRXPOS_SSAX, TRXPOS_SSAY, TRXPOS_DSAX, TRXPOS_DSAY, TRXPOS_DIR;
	u32 TRXREG_RRW, TRXREG_RRH;
	
	
	// inputbuffer
	// 0: BITBLTBUF
	// 1: TRXPOS
	// 2: TRXREG
	// 3: CurrentPath
	// 4: Count64
	// 5: XferX
	// 6: XferY
	// 7: XferDstX
	// 8: XferDstY
	// 9: XferWidth
	// 10: XferHeight
	// 11: XferDstBufWidth
	// 12: XferDstOffset32
	// 13: 
	// 14: ADDITIONAL COMMAND
	// -------------
	// 15: PRIM (COMMAND)
	// 16: Data0
	// ...
	// 31: Data15


	if ( !ulThreadNum )
	{
		// TRXDIR might not have been set yet when starting transfer, so it is passed as an argument
		p_inputbuffer [ 0 ] = _GPU->GPURegsGp.BITBLTBUF.Value;
		p_inputbuffer [ 1 ] = _GPU->GPURegsGp.TRXPOS.Value;
		p_inputbuffer [ 2 ] = _GPU->GPURegsGp.TRXREG.Value;
		
		
		// now that the data is set, if this is multi-threaded, then we are done if we are not on the GPU thread
		if ( _GPU->ulNumberOfThreads )
		{
			p_inputbuffer [ 3 ] = TRXDIR;
			
			// set command
			p_inputbuffer [ 15 ] = 7;
			p_inputbuffer [ 14 ] = 2;
			
			ulInputBuffer_WriteIndex++;
			return;
		}
	}
	else
	{
		TRXDIR = p_inputbuffer [ 3 ];
	}	// end if ( !ulThreadNum )
	

	//TRXPOS_DIR = ( p_inputbuffer [ 1 ] >> 59 ) & 0x3;
	
	
	// set current transfer x position
	XferX = 0;
	
	// set current transfer y position
	XferY = 0;
	
	// set pixel shift and count for 24-bit pixels
	PixelShift = 0;
	PixelCount = 0;

	// TRXREG //
	
	TRXREG_RRW = ( p_inputbuffer [ 2 ] >> 0 ) & 0xfff;
	TRXREG_RRH = ( p_inputbuffer [ 2 ] >> 32 ) & 0xfff;
	
	// get transfer width (in pixels)
	XferWidth = TRXREG_RRW;
	
	// get transfer height (in pixels)
	XferHeight = TRXREG_RRH;
	

	if ( TRXDIR & 1 )
	{
		// Local->Host (GPU->CPU) transfer //
	
	TRXPOS_SSAX = ( p_inputbuffer [ 1 ] >> 0 ) & 0x7ff;
	TRXPOS_SSAY = ( p_inputbuffer [ 1 ] >> 16 ) & 0x7ff;
	
	BITBLTBUF_SBP = ( p_inputbuffer [ 0 ] >> 0 ) & 0x3fff;
	BITBLTBUF_SBW = ( p_inputbuffer [ 0 ] >> 16 ) & 0x3f;
	
	// get source buffer offset (SBP : word address/64)
	//XferSrcOffset32 = GPURegsGp.BITBLTBUF.SBP << 6;
	XferSrcOffset32 = BITBLTBUF_SBP << 6;

	//if ( GPURegsGp.BITBLTBUF.SBW >= 1 && GPURegsGp.BITBLTBUF.SBW <= 32 )
	if ( BITBLTBUF_SBW >= 1 && BITBLTBUF_SBW <= 32 )
	{
		// get source buffer width (in pixels) (SBW : number of pixels/64)
		XferSrcBufWidth = BITBLTBUF_SBW << 6;
	}
	else
	{
#ifdef INLINE_DEBUG_INVALID_SBW
	debug << dec << "\r\nhps2x64: ALERT: Src Buffer Width value illegal. BITBLTBUF.SBW=" << dec << (u32) BITBLTBUF_SBW;
#endif

#ifdef VERBOSE_ALERTS_SBW
	cout << "\nhps2x64: ALERT: Src Buffer Width value illegal. BITBLTBUF.SBW=" << dec << (u32) BITBLTBUF_SBW;
#endif
	}
	
	// transfer x,y for source
	//XferSrcX = GPURegsGp.TRXPOS.SSAX;
	//XferSrcY = GPURegsGp.TRXPOS.SSAY;
	XferSrcX = TRXPOS_SSAX;
	XferSrcY = TRXPOS_SSAY;
	
	BITBLTBUF_SPSM = ( p_inputbuffer [ 0 ] >> 24 ) & 0x3f;
	
	}
	else
	{
		// Host->Local (CPU->GPU) transfer //

	
	TRXPOS_DSAX = ( p_inputbuffer [ 1 ] >> 32 ) & 0x7ff;
	TRXPOS_DSAY = ( p_inputbuffer [ 1 ] >> 48 ) & 0x7ff;
	
	BITBLTBUF_DBP = ( p_inputbuffer [ 0 ] >> 32 ) & 0x3fff;
	BITBLTBUF_DBW = ( p_inputbuffer [ 0 ] >> 48 ) & 0x3f;
	
	// get dest buffer offset (DBP : word address/64)
	XferDstOffset32 = BITBLTBUF_DBP << 6;

	//if ( GPURegsGp.BITBLTBUF.DBW >= 1 && GPURegsGp.BITBLTBUF.DBW <= 32 )
	if ( BITBLTBUF_DBW >= 1 && BITBLTBUF_DBW <= 32 )
	{
		// get dest buffer width (in pixels) (DBW : pixels/64)
		XferDstBufWidth = BITBLTBUF_DBW << 6;
	}
	else
	{
#ifdef INLINE_DEBUG_INVALID_DBW
	debug << dec << "\r\nhps2x64: ALERT: Dst Buffer Width value illegal. BITBLTBUF.DBW=" << dec << (u32) BITBLTBUF_DBW;
#endif

#ifdef VERBOSE_ALERTS_DBW
if ( BITBLTBUF_DBW )
{
				cout << "\nhps2x64: ALERT: Dst Buffer Width value illegal. BITBLTBUF.DBW=" << dec << (u32) BITBLTBUF_DBW;
}
#endif
	}

	// transfer x,y for dest
	//XferDstX = GPURegsGp.TRXPOS.DSAX;
	//XferDstY = GPURegsGp.TRXPOS.DSAY;
	XferDstX = TRXPOS_DSAX;
	XferDstY = TRXPOS_DSAY;

	BITBLTBUF_DPSM = ( p_inputbuffer [ 0 ] >> 56 ) & 0x3f;
	
	}

}



// if multi-threading, Count64 (count of 64-bit values) has a max value of 16
static void GPU::Select_TransferIn_t ( u64* p_inputbuffer, u32 ulThreadNum, u64* pData64 = NULL, u32 Count64 = 0 )
{
	
	if ( !ulThreadNum )
	{
		
		// now that the data is set, if this is multi-threaded, then we are done if we are not on the GPU thread
		if ( _GPU->ulNumberOfThreads )
		{
			
			// TRXDIR might not have been set yet when starting transfer, so it is passed as an argument
			p_inputbuffer [ 4 ] = Count64;
			
			// copy in the data
			for ( int i = 0; i < Count64; i++ )
			{
				p_inputbuffer [ 16 + i ] = *pData64++;
			}
			
			// set command
			p_inputbuffer [ 15 ] = 7;
			p_inputbuffer [ 14 ] = 4;
			
			ulInputBuffer_WriteIndex++;
			return;
		}
	}
	else
	{
		
		pData64 = & ( p_inputbuffer [ 16 ] );
		Count64 = p_inputbuffer [ 4 ];
		
	}	// end if ( !ulThreadNum )
	
	switch ( _GPU->BITBLTBUF_DPSM )
	{
		case 0:
			_GPU->TransferDataIn32_DS_t<0> ( (u32*) pData64, Count64 << 1 );
			break;
			
		case 1:
			_GPU->TransferDataIn32_DS_t<1> ( (u32*) pData64, Count64 << 1 );
			break;
			
		case 2:
			_GPU->TransferDataIn32_DS_t<2> ( (u32*) pData64, Count64 << 1 );
			break;
			
		case 0xa:
			_GPU->TransferDataIn32_DS_t<0xa> ( (u32*) pData64, Count64 << 1 );
			break;
			
		case 0x13:
			_GPU->TransferDataIn32_DS_t<0x13> ( (u32*) pData64, Count64 << 1 );
			break;
			
		case 0x14:
			_GPU->TransferDataIn32_DS_t<0x14> ( (u32*) pData64, Count64 << 1 );
			break;
			
		case 0x1b:
			_GPU->TransferDataIn32_DS_t<0x1b> ( (u32*) pData64, Count64 << 1 );
			break;
			
		case 0x24:
			_GPU->TransferDataIn32_DS_t<0x24> ( (u32*) pData64, Count64 << 1 );
			break;
			
		case 0x2c:
			_GPU->TransferDataIn32_DS_t<0x2c> ( (u32*) pData64, Count64 << 1 );
			break;
			
		case 0x30:
			_GPU->TransferDataIn32_DS_t<0x30> ( (u32*) pData64, Count64 << 1 );
			break;
			
		case 0x31:
			_GPU->TransferDataIn32_DS_t<0x31> ( (u32*) pData64, Count64 << 1 );
			break;
			
		case 0x32:
			_GPU->TransferDataIn32_DS_t<0x32> ( (u32*) pData64, Count64 << 1 );
			break;
			
		case 0x3a:
			_GPU->TransferDataIn32_DS_t<0x3a> ( (u32*) pData64, Count64 << 1 );
			break;
	}
}

#endif

static void GPU::Select_KillGpuThreads_t ( u64* p_inputbuffer, u32 ulThreadNum )
{
	if ( _GPU->ulNumberOfThreads )
	{
		// set command
		p_inputbuffer [ 15 ] = 7;
		p_inputbuffer [ 14 ] = 5;

		ulInputBuffer_WriteIndex++;
	}
}


static void GPU::Flush ()
{
	if ( _GPU->ulNumberOfThreads )
	{
		if ( ulInputBuffer_WriteIndex != ulInputBuffer_TargetIndex )
		{
			// send the command to the other thread
			//Lock_ExchangeAdd32 ( (long&) ulInputBuffer_Count, ulNumberOfThreads );
			x64ThreadSafe::Utilities::Lock_Exchange32 ( (long&) ulInputBuffer_TargetIndex, ulInputBuffer_WriteIndex );
			
			while ( ( ulInputBuffer_WriteIndex - ulInputBuffer_ReadIndex ) > ( c_ulInputBuffer_Size - c_ulRequiredBuffer ) );
		}
	}
}

static void GPU::Finish ()
{
	if ( _GPU->ulNumberOfThreads )
	{
		if ( ulInputBuffer_WriteIndex != ulInputBuffer_ReadIndex )
		{
			if ( ulInputBuffer_WriteIndex != ulInputBuffer_TargetIndex )
			{
				// send the command to the other thread
				//Lock_ExchangeAdd32 ( (long&) ulInputBuffer_Count, ulNumberOfThreads );
				x64ThreadSafe::Utilities::Lock_Exchange32 ( (long&) ulInputBuffer_TargetIndex, ulInputBuffer_WriteIndex );
			}
			
			// wait for the other thread to complete
			while ( ulInputBuffer_WriteIndex != ulInputBuffer_ReadIndex );
		}
	}
}


	};
}


#endif

