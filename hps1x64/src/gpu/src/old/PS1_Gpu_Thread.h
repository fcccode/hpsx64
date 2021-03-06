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


#ifndef _PS1_GPU_H_
#define _PS1_GPU_H_

#include "types.h"
#include "Debug.h"

#include "WinApiHandler.h"
#include "GNUAsmUtility_x64.h"


#include "opencl_compute.h"


//#define DRAW_MULTIPLE_PIXELS

#include "emmintrin.h"

//#define _ENABLE_SSE2_NONTEMPLATE

//#define _ENABLE_SSE2_TRIANGLE_MONO
//#define _ENABLE_SSE2_TRIANGLE_GRADIENT
//#define _ENABLE_SSE2_RECTANGLE_MONO


#ifdef _ENABLE_SSE2

// need to include this file to use SSE2 intrinsics
#include "emmintrin.h"
//#include "smmintrin.h"

#endif


using namespace x64Asm::Utilities;


//#define ENABLE_DRAW_OVERHEAD


namespace Playstation1
{
	namespace GPUThread
	{
		
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
		
		//inline static u32 Clamp5 ( s32 Component )
		//{
		//	if ( Component < 0 ) Component = 0;
		//	if ( Component > 0x1f ) Component = 0x1f;
		//	return Component;
		//}
		
		//inline static u32 Clamp8 ( s32 Component )
		//{
		//	if ( Component < 0 ) Component = 0;
		//	if ( Component > 0xff ) Component = 0xff;
		//	return Component;
		//}
		
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

		
		inline static u16 SemiTransparency16 ( u16 B, u16 F, u32 abrCode )
		{
			static const u32 ShiftSame = 0;
			static const u32 ShiftHalf = 1;
			static const u32 ShiftQuarter = 2;
			
			static const u32 c_iBitsPerPixel = 5;
			//static const u32 c_iShiftHalf_Mask = ~( ( 1 << 4 ) + ( 1 << 9 ) );
			static const u32 c_iShiftHalf_Mask = ~( ( 1 << ( c_iBitsPerPixel - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 2 ) - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 3 ) - 1 ) ) );
			//static const u32 c_iShiftQuarter_Mask = ~( ( 3 << 3 ) + ( 3 << 8 ) );
			static const u32 c_iShiftQuarter_Mask = ~( ( 3 << ( c_iBitsPerPixel - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 2 ) - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 3 ) - 2 ) ) );
			//static const u32 c_iClamp_Mask = ( ( 1 << 5 ) + ( 1 << 10 ) + ( 1 << 15 ) );
			static const u32 c_iClampMask = ( ( 1 << ( c_iBitsPerPixel ) ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) + ( 1 << ( c_iBitsPerPixel * 3 ) ) );
			static const u32 c_iLoBitMask = ( ( 1 ) + ( 1 << c_iBitsPerPixel ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) );
			static const u32 c_iPixelMask = 0x7fff;
			
			u32 Red, Green, Blue;
			
			u32 Color, Actual, Mask;
			
			switch ( abrCode )
			{
				// 0.5xB+0.5 xF
				case 0:
					//Color = SetRed16 ( Clamp5 ( ( GetRed16 ( B ) >> ShiftHalf ) + ( GetRed16( F ) >> ShiftHalf ) ) ) |
					//		SetGreen16 ( Clamp5 ( ( GetGreen16 ( B ) >> ShiftHalf ) + ( GetGreen16( F ) >> ShiftHalf ) ) ) |
					//		SetBlue16 ( Clamp5 ( ( GetBlue16 ( B ) >> ShiftHalf ) + ( GetBlue16( F ) >> ShiftHalf ) ) );
					
					Mask = B & F & c_iLoBitMask;
					Color = ( ( B >> 1 ) & c_iShiftHalf_Mask ) + ( ( F >> 1 ) & c_iShiftHalf_Mask ) + Mask;
					return Color;
					
					break;
				
				// 1.0xB+1.0 xF
				case 1:
					//Color = SetRed16 ( Clamp5 ( ( GetRed16 ( B ) >> ShiftSame ) + ( GetRed16( F ) >> ShiftSame ) ) ) |
					//		SetGreen16 ( Clamp5 ( ( GetGreen16 ( B ) >> ShiftSame ) + ( GetGreen16( F ) >> ShiftSame ) ) ) |
					//		SetBlue16 ( Clamp5 ( ( GetBlue16 ( B ) >> ShiftSame ) + ( GetBlue16( F ) >> ShiftSame ) ) );
					
					B &= c_iPixelMask;
					F &= c_iPixelMask;
					Actual = B + F;
					Mask = ( B ^ F ^ Actual ) & c_iClampMask;
					Color = Actual - Mask;
					Mask -= ( Mask >> c_iBitsPerPixel );
					Color |= Mask;
					return Color;
					
					break;
					
				// 1.0xB-1.0 xF
				case 2:
					//Color = SetRed16 ( Clamp5 ( (s32) ( GetRed16 ( B ) >> ShiftSame ) - (s32) ( GetRed16( F ) >> ShiftSame ) ) ) |
					//		SetGreen16 ( Clamp5 ( (s32) ( GetGreen16 ( B ) >> ShiftSame ) - (s32) ( GetGreen16( F ) >> ShiftSame ) ) ) |
					//		SetBlue16 ( Clamp5 ( (s32) ( GetBlue16 ( B ) >> ShiftSame ) - (s32) ( GetBlue16( F ) >> ShiftSame ) ) );
					
					B &= c_iPixelMask;
					F &= c_iPixelMask;
					Actual = B - F;
					Mask = ( B ^ F ^ Actual ) & c_iClampMask;
					Color = Actual + Mask;
					Mask -= ( Mask >> c_iBitsPerPixel );
					Color &= ~Mask;
					return Color;
					
					break;
					
				// 1.0xB+0.25xF
				case 3:
					//Color = SetRed16 ( Clamp5 ( ( GetRed16 ( B ) >> ShiftSame ) + ( GetRed16( F ) >> ShiftQuarter ) ) ) |
					//		SetGreen16 ( Clamp5 ( ( GetGreen16 ( B ) >> ShiftSame ) + ( GetGreen16( F ) >> ShiftQuarter ) ) ) |
					//		SetBlue16 ( Clamp5 ( ( GetBlue16 ( B ) >> ShiftSame ) + ( GetBlue16( F ) >> ShiftQuarter ) ) );
					
					B &= c_iPixelMask;
					F = ( F >> 2 ) & c_iShiftQuarter_Mask;
					Actual = B + F;
					Mask = ( B ^ F ^ Actual ) & c_iClampMask;
					Color = Actual - Mask;
					Mask -= ( Mask >> c_iBitsPerPixel );
					Color |= Mask;
					return Color;
					
					break;
			}
			
			return Color;
		}


#ifdef _ENABLE_SSE2_NONTEMPLATE
		inline static __m128i vSemiTransparency16 ( __m128i B, __m128i F, u32 abrCode )
		{
			static const u32 ShiftSame = 0;
			static const u32 ShiftHalf = 1;
			static const u32 ShiftQuarter = 2;
			
			static const u32 c_iBitsPerPixel = 5;
			//static const u32 c_iShiftHalf_Mask = ~( ( 1 << 4 ) + ( 1 << 9 ) );
			static const u32 c_iShiftHalf_Mask = ~( ( 1 << ( c_iBitsPerPixel - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 2 ) - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 3 ) - 1 ) ) );
			//static const u32 c_iShiftQuarter_Mask = ~( ( 3 << 3 ) + ( 3 << 8 ) );
			static const u32 c_iShiftQuarter_Mask = ~( ( 3 << ( c_iBitsPerPixel - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 2 ) - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 3 ) - 2 ) ) );
			//static const u32 c_iClamp_Mask = ( ( 1 << 5 ) + ( 1 << 10 ) + ( 1 << 15 ) );
			static const u32 c_iClampMask = ( ( 1 << ( c_iBitsPerPixel ) ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) + ( 1 << ( c_iBitsPerPixel * 3 ) ) );
			static const u32 c_iLoBitMask = ( ( 1 ) + ( 1 << c_iBitsPerPixel ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) );
			static const u32 c_iPixelMask = 0x7fff;
			
			//u32 Red, Green, Blue;
			
			//u32 Color, Actual, Mask;
			__m128i Color, Actual, Mask;
			__m128i vTemp;
			
			switch ( abrCode )
			{
				// 0.5xB+0.5 xF
				case 0:
					//Mask = B & F & c_iLoBitMask;
					Mask = _mm_and_si128 ( _mm_and_si128 ( B, F ), _mm_set1_epi16 ( c_iLoBitMask ) );
					
					//Color = ( ( B >> 1 ) & c_iShiftHalf_Mask ) + ( ( F >> 1 ) & c_iShiftHalf_Mask ) + Mask;
					vTemp = _mm_set1_epi16 ( c_iShiftHalf_Mask );
					Color = _mm_add_epi16 ( _mm_add_epi16 ( _mm_and_si128( _mm_srli_epi16 ( B, 1 ), vTemp ), _mm_and_si128( _mm_srli_epi16 ( F, 1 ), vTemp ) ), Mask );
					
					return Color;
					
					break;
				
				// 1.0xB+1.0 xF
				case 1:
					//B &= c_iPixelMask;
					//F &= c_iPixelMask;
					vTemp = _mm_set1_epi16 ( c_iPixelMask );
					B = _mm_and_si128 ( B, vTemp );
					F = _mm_and_si128 ( F, vTemp );
					
					//Actual = B + F;
					Actual = _mm_add_epi16 ( B, F );
					
					//Mask = ( B ^ F ^ Actual ) & c_iClampMask;
					Mask = _mm_and_si128 ( _mm_xor_si128 ( _mm_xor_si128 ( B, F ), Actual ), _mm_set1_epi16 ( c_iClampMask ) );
					
					//Color = Actual - Mask;
					Color = _mm_sub_epi16 ( Actual, Mask );
					
					//Mask -= ( Mask >> c_iBitsPerPixel );
					Mask = _mm_sub_epi16 ( Mask, _mm_srli_epi16 ( Mask, c_iBitsPerPixel ) );
					
					//Color |= Mask;
					Color = _mm_or_si128 ( Color, Mask );
					
					return Color;
					
					break;
					
				// 1.0xB-1.0 xF
				case 2:
					//B &= c_iPixelMask;
					//F &= c_iPixelMask;
					vTemp = _mm_set1_epi16 ( c_iPixelMask );
					B = _mm_and_si128 ( B, vTemp );
					F = _mm_and_si128 ( F, vTemp );
					
					//Actual = B - F;
					Actual = _mm_sub_epi16 ( B, F );
					
					//Mask = ( B ^ F ^ Actual ) & c_iClampMask;
					Mask = _mm_and_si128 ( _mm_xor_si128 ( _mm_xor_si128 ( B, F ), Actual ), _mm_set1_epi16 ( c_iClampMask ) );
					
					//Color = Actual + Mask;
					Color = _mm_add_epi16 ( Actual, Mask );

					//Mask -= ( Mask >> c_iBitsPerPixel );
					Mask = _mm_sub_epi16 ( Mask, _mm_srli_epi16 ( Mask, c_iBitsPerPixel ) );

					//Color &= ~Mask;
					Color = _mm_andnot_si128 ( Mask, Color );

					return Color;
					
					break;
					
				// 1.0xB+0.25xF
				case 3:
					//B &= c_iPixelMask;
					vTemp = _mm_set1_epi16 ( c_iPixelMask );
					B = _mm_and_si128 ( B, vTemp );
					
					//F = ( F >> 2 ) & c_iShiftQuarter_Mask;
					vTemp = _mm_set1_epi16 ( c_iShiftQuarter_Mask );
					F = _mm_and_si128 ( _mm_srli_epi16 ( F, 2 ), vTemp );
					
					//Actual = B + F;
					Actual = _mm_add_epi16 ( B, F );
					
					//Mask = ( B ^ F ^ Actual ) & c_iClampMask;
					Mask = _mm_and_si128 ( _mm_xor_si128 ( _mm_xor_si128 ( B, F ), Actual ), _mm_set1_epi16 ( c_iClampMask ) );
					
					//Color = Actual - Mask;
					Color = _mm_sub_epi16 ( Actual, Mask );
					
					//Mask -= ( Mask >> c_iBitsPerPixel );
					Mask = _mm_sub_epi16 ( Mask, _mm_srli_epi16 ( Mask, c_iBitsPerPixel ) );
					
					//Color |= Mask;
					Color = _mm_or_si128 ( Color, Mask );
					
					return Color;
					
					break;
			}
			
			return Color;
		}


		
		
		// code from stack overflow @ http://stackoverflow.com/questions/10500766/sse-multiplication-of-4-32-bit-integers
		static inline __m128i _custom_mul_32(const __m128i &a, const __m128i &b)
		{
#ifdef _ENABLE_SSE41  // modern CPU - use SSE 4.1
			return _mm_mullo_epi32(a, b);
#else               // old CPU - use SSE 2
			__m128i tmp1 = _mm_mul_epu32(a,b); /* mul 2,0*/
			__m128i tmp2 = _mm_mul_epu32( _mm_srli_si128(a,4), _mm_srli_si128(b,4)); /* mul 3,1 */
			return _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE (0,0,2,0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE (0,0,2,0))); /* shuffle results to [63..0] and pack */
#endif
		}

		
		// code from stack overflow @ http://stackoverflow.com/questions/4360920/whats-the-most-efficient-way-to-load-and-extract-32-bit-integer-values-from-a-1
		inline s32 _custom_get0(const __m128i& vec){return _mm_cvtsi128_si32 (vec);}
		inline s32 _custom_get1(const __m128i& vec){return _mm_cvtsi128_si32 (_mm_shuffle_epi32(vec,0x55));}
		inline s32 _custom_get2(const __m128i& vec){return _mm_cvtsi128_si32 (_mm_shuffle_epi32(vec,0xAA));}
		inline s32 _custom_get3(const __m128i& vec){return _mm_cvtsi128_si32 (_mm_shuffle_epi32(vec,0xFF));}
		
		// all inputs should be 16-bit vector values
		inline static __m128i vColorMultiply1624 ( __m128i& Color16, __m128i& vr_Color24, __m128i& vg_Color24, __m128i& vb_Color24 )
		{
			/*
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
			*/
			
			//s64 Red, Green, Blue;
			__m128i Red, Green, Blue, vMask;
			
			Red = _mm_slli_epi16 ( Color16, 11 );
			Green = _mm_slli_epi16 ( _mm_srli_epi16 ( Color16, 5 ), 11 );
			Blue = _mm_slli_epi16 ( _mm_srli_epi16 ( Color16, 10 ), 11 );
			
			// multiply
			Red = _mm_mulhi_epu16 ( Red, vr_Color24 );
			Green = _mm_mulhi_epu16 ( Green, vg_Color24 );
			Blue = _mm_mulhi_epu16 ( Blue, vb_Color24 );
			
			vMask = _mm_set1_epi16 ( 0x1f << 2 );
			
			// saturate
			Red = _mm_and_si128 ( _mm_or_si128 ( Red, _mm_cmpgt_epi16 ( Red, vMask ) ), vMask );
			Green = _mm_and_si128 ( _mm_or_si128 ( Green, _mm_cmpgt_epi16 ( Green, vMask ) ), vMask );
			Blue = _mm_and_si128 ( _mm_or_si128 ( Blue, _mm_cmpgt_epi16 ( Blue, vMask ) ), vMask );
			
			// combine
			return _mm_or_si128 ( _mm_or_si128 ( _mm_srli_epi16 ( Red, 2 ), _mm_slli_epi16 ( Green, 3 ) ), _mm_slli_epi16 ( Blue, 8 ) );
			
			/*
			// ------
			
			// get components
			Red = _mm_srli_epi16 ( _mm_slli_epi16 ( Color16, 11 ), 11 );
			Green = _mm_srli_epi16 ( _mm_slli_epi16 ( Color16, 6 ), 11 );
			Blue = _mm_srli_epi16 ( _mm_slli_epi16 ( Color16, 1 ), 11 );
			
			// multiply
			Red = _mm_mullo_epi16 ( Red, vr_Color24 );
			Green = _mm_mullo_epi16 ( Green, vg_Color24 );
			Blue = _mm_mullo_epi16 ( Blue, vb_Color24 );
			
			// saturate
			Red = _mm_or_si128 ( Red, _mm_srai_epi16 ( _mm_slli_epi16 ( Red, 3 ), 15 ) );
			Green = _mm_or_si128 ( Green, _mm_srai_epi16 ( _mm_slli_epi16 ( Green, 3 ), 15 ) );
			Blue = _mm_or_si128 ( Blue, _mm_srai_epi16 ( _mm_slli_epi16 ( Blue, 3 ), 15 ) );
			
			// mask
			Red = _mm_srli_epi16 ( _mm_slli_epi16 ( Red, 4 ), 11 );
			Green = _mm_srli_epi16 ( _mm_slli_epi16 ( Green, 4 ), 11 );
			Blue = _mm_srli_epi16 ( _mm_slli_epi16 ( Blue, 4 ), 11 );
			
			// combine
			return _mm_or_si128 ( Red, _mm_or_si128 ( _mm_slli_epi16 ( Green, 5 ), _mm_slli_epi16 ( Blue, 10 ) ) );
			*/
		}
		
#endif



#ifdef _ENABLE_SSE2_TRIANGLE_MONO
		template<const int ABRCODE>
		inline static __m128i vSemiTransparency16_t ( __m128i B, __m128i F, const __m128i c_vLoBitMask, const __m128i c_vPixelMask, const __m128i c_vClampMask, const __m128i c_vShiftHalf_Mask, const __m128i c_vShiftQuarter_Mask )
		{
			static const u32 ShiftSame = 0;
			static const u32 ShiftHalf = 1;
			static const u32 ShiftQuarter = 2;
			
			static const u32 c_iBitsPerPixel = 5;
			static const u32 c_iShiftHalf_Mask = ~( ( 1 << ( c_iBitsPerPixel - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 2 ) - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 3 ) - 1 ) ) );
			static const u32 c_iShiftQuarter_Mask = ~( ( 3 << ( c_iBitsPerPixel - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 2 ) - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 3 ) - 2 ) ) );
			static const u32 c_iClampMask = ( ( 1 << ( c_iBitsPerPixel ) ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) + ( 1 << ( c_iBitsPerPixel * 3 ) ) );
			static const u32 c_iLoBitMask = ( ( 1 ) + ( 1 << c_iBitsPerPixel ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) );
			static const u32 c_iPixelMask = 0x7fff;
			
			//u32 Color, Actual, Mask;
			__m128i Color, Actual, Mask;
			//__m128i vTemp;
			
			// constants needed c_vLoBitMask, c_vPixelMask, c_vClampMask, c_vShiftHalfMask, c_vShiftQuarterMask
			switch ( ABRCODE )
			{
				// 0.5xB+0.5 xF
				case 0:
					//Mask = B & F & c_iLoBitMask;
					Mask = _mm_and_si128 ( _mm_and_si128 ( B, F ), c_vLoBitMask );
					
					//Color = ( ( B >> 1 ) & c_iShiftHalf_Mask ) + ( ( F >> 1 ) & c_iShiftHalf_Mask ) + Mask;
					//vTemp = _mm_set1_epi16 ( c_vShiftHalf_Mask );
					Color = _mm_add_epi16 ( _mm_add_epi16 ( _mm_and_si128( _mm_srli_epi16 ( B, 1 ), c_vShiftHalf_Mask ), _mm_and_si128( _mm_srli_epi16 ( F, 1 ), c_vShiftHalf_Mask ) ), Mask );
					
					return Color;
					
					break;
				
				// 1.0xB+1.0 xF
				case 1:
					//B &= c_iPixelMask;
					//F &= c_iPixelMask;
					//vTemp = _mm_set1_epi16 ( c_vPixelMask );
					B = _mm_and_si128 ( B, c_vPixelMask );
					F = _mm_and_si128 ( F, c_vPixelMask );
					
					//Actual = B + F;
					Actual = _mm_add_epi16 ( B, F );
					
					//Mask = ( B ^ F ^ Actual ) & c_iClampMask;
					Mask = _mm_and_si128 ( _mm_xor_si128 ( _mm_xor_si128 ( B, F ), Actual ), c_vClampMask );
					
					//Color = Actual - Mask;
					Color = _mm_sub_epi16 ( Actual, Mask );
					
					//Mask -= ( Mask >> c_iBitsPerPixel );
					Mask = _mm_sub_epi16 ( Mask, _mm_srli_epi16 ( Mask, c_iBitsPerPixel ) );
					
					//Color |= Mask;
					Color = _mm_or_si128 ( Color, Mask );
					
					return Color;
					
					break;
					
				// 1.0xB-1.0 xF
				case 2:
					//B &= c_iPixelMask;
					//F &= c_iPixelMask;
					//vTemp = _mm_set1_epi16 ( c_vPixelMask );
					B = _mm_and_si128 ( B, c_vPixelMask );
					F = _mm_and_si128 ( F, c_vPixelMask );
					
					//Actual = B - F;
					Actual = _mm_sub_epi16 ( B, F );
					
					//Mask = ( B ^ F ^ Actual ) & c_iClampMask;
					Mask = _mm_and_si128 ( _mm_xor_si128 ( _mm_xor_si128 ( B, F ), Actual ), c_vClampMask );
					
					//Color = Actual + Mask;
					Color = _mm_add_epi16 ( Actual, Mask );

					//Mask -= ( Mask >> c_iBitsPerPixel );
					Mask = _mm_sub_epi16 ( Mask, _mm_srli_epi16 ( Mask, c_iBitsPerPixel ) );

					//Color &= ~Mask;
					Color = _mm_andnot_si128 ( Mask, Color );

					return Color;
					
					break;
					
				// 1.0xB+0.25xF
				case 3:
					//B &= c_iPixelMask;
					//vTemp = _mm_set1_epi16 ( c_vPixelMask );
					B = _mm_and_si128 ( B, c_vPixelMask );
					
					//F = ( F >> 2 ) & c_iShiftQuarter_Mask;
					//vTemp = _mm_set1_epi16 ( c_vShiftQuarter_Mask );
					F = _mm_and_si128 ( _mm_srli_epi16 ( F, 2 ), c_vShiftQuarter_Mask );
					
					//Actual = B + F;
					Actual = _mm_add_epi16 ( B, F );
					
					//Mask = ( B ^ F ^ Actual ) & c_iClampMask;
					Mask = _mm_and_si128 ( _mm_xor_si128 ( _mm_xor_si128 ( B, F ), Actual ), c_vClampMask );
					
					//Color = Actual - Mask;
					Color = _mm_sub_epi16 ( Actual, Mask );
					
					//Mask -= ( Mask >> c_iBitsPerPixel );
					Mask = _mm_sub_epi16 ( Mask, _mm_srli_epi16 ( Mask, c_iBitsPerPixel ) );
					
					//Color |= Mask;
					Color = _mm_or_si128 ( Color, Mask );
					
					return Color;
					
					break;
			}
			
			//return Color;
		}
#endif
		
		
		inline u32 SemiTransparency24 ( u32 B, u32 F, u32 abrCode )
		{
			static const u32 ShiftSame = 0;
			static const u32 ShiftHalf = 1;
			static const u32 ShiftQuarter = 2;
			
			static const u32 c_iBitsPerPixel = 8;
			//static const u32 c_iShiftHalf_Mask = ~( ( 1 << 4 ) + ( 1 << 9 ) );
			static const u32 c_iShiftHalf_Mask = ~( ( 1 << ( c_iBitsPerPixel - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 2 ) - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 3 ) - 1 ) ) );
			//static const u32 c_iShiftQuarter_Mask = ~( ( 3 << 3 ) + ( 3 << 8 ) );
			static const u32 c_iShiftQuarter_Mask = ~( ( 3 << ( c_iBitsPerPixel - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 2 ) - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 3 ) - 2 ) ) );
			//static const u32 c_iClamp_Mask = ( ( 1 << 5 ) + ( 1 << 10 ) + ( 1 << 15 ) );
			static const u32 c_iClampMask = ( ( 1 << ( c_iBitsPerPixel ) ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) + ( 1 << ( c_iBitsPerPixel * 3 ) ) );
			static const u32 c_iLoBitMask = ( ( 1 ) + ( 1 << c_iBitsPerPixel ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) );
			
			u32 Color, Actual, Mask;
			
			//u32 Red, Green, Blue, Color;
			
			switch ( abrCode )
			{
				// 0.5xB+0.5 xF
				case 0:
					//Color = SetRed24 ( Clamp8 ( ( GetRed24 ( B ) >> ShiftHalf ) + ( GetRed24( F ) >> ShiftHalf ) ) ) |
					//		SetGreen24 ( Clamp8 ( ( GetGreen24 ( B ) >> ShiftHalf ) + ( GetGreen24( F ) >> ShiftHalf ) ) ) |
					//		SetBlue24 ( Clamp8 ( ( GetBlue24 ( B ) >> ShiftHalf ) + ( GetBlue24( F ) >> ShiftHalf ) ) );
					
					Mask = B & F & c_iLoBitMask;
					Color = ( ( B >> 1 ) & c_iShiftHalf_Mask ) + ( ( F >> 1 ) & c_iShiftHalf_Mask ) + Mask;
					return Color;
					
					break;
				
				// 1.0xB+1.0 xF
				case 1:
					//Color = SetRed24 ( Clamp8 ( ( GetRed24 ( B ) >> ShiftSame ) + ( GetRed24( F ) >> ShiftSame ) ) ) |
					//		SetGreen24 ( Clamp8 ( ( GetGreen24 ( B ) >> ShiftSame ) + ( GetGreen24( F ) >> ShiftSame ) ) ) |
					//		SetBlue24 ( Clamp8 ( ( GetBlue24 ( B ) >> ShiftSame ) + ( GetBlue24( F ) >> ShiftSame ) ) );
					
					Actual = B + F;
					Mask = ( B ^ F ^ Actual ) & c_iClampMask;
					Color = Actual - Mask;
					Mask -= ( Mask >> c_iBitsPerPixel );
					Color |= Mask;
					return Color;
					
					break;
					
				// 1.0xB-1.0 xF
				case 2:
					//Color = SetRed24 ( Clamp8 ( (s32) ( GetRed24 ( B ) >> ShiftSame ) - (s32) ( GetRed24( F ) >> ShiftSame ) ) ) |
					//		SetGreen24 ( Clamp8 ( (s32) ( GetGreen24 ( B ) >> ShiftSame ) - (s32) ( GetGreen24( F ) >> ShiftSame ) ) ) |
					//		SetBlue24 ( Clamp8 ( (s32) ( GetBlue24 ( B ) >> ShiftSame ) - (s32) ( GetBlue24( F ) >> ShiftSame ) ) );
					
					Actual = B - F;
					Mask = ( B ^ F ^ Actual ) & c_iClampMask;
					Color = Actual + Mask;
					Mask -= ( Mask >> c_iBitsPerPixel );
					Color &= ~Mask;
					return Color;
					
					break;
					
				// 1.0xB+0.28xF
				case 3:
					//Color = SetRed24 ( Clamp8 ( ( GetRed24 ( B ) >> ShiftSame ) + ( GetRed24( F ) >> ShiftQuarter ) ) ) |
					//		SetGreen24 ( Clamp8 ( ( GetGreen24 ( B ) >> ShiftSame ) + ( GetGreen24( F ) >> ShiftQuarter ) ) ) |
					//		SetBlue24 ( Clamp8 ( ( GetBlue24 ( B ) >> ShiftSame ) + ( GetBlue24( F ) >> ShiftQuarter ) ) );
					
					F = ( F >> 2 ) & c_iShiftQuarter_Mask;
					Actual = B + F;
					Mask = ( B ^ F ^ Actual ) & c_iClampMask;
					Color = Actual - Mask;
					Mask -= ( Mask >> c_iBitsPerPixel );
					Color |= Mask;
					return Color;
					
					break;
			}
			
			return Color;
		}

		
		// checks if it is ok to draw to ps1 frame buffer or not by looking at DFE and LCF
		inline bool isDrawOk ()
		{
			if ( GPU_CTRL_Read.DFE || ( !GPU_CTRL_Read.DFE && !GPU_CTRL_Read.LCF ) )
			{
				return true;
			}
			else
			{
				//BusyCycles = 0;
				return false;
				//return true;
			}
		}


		static void sRun () { _GPU->Run (); }
		static void Set_EventCallback ( funcVoid1 UpdateEvent_CB ) { _GPU->NextEvent_Idx = UpdateEvent_CB ( sRun ); };
		
		
		// for interrupt call back
		static funcVoid UpdateInterrupts;
		static void Set_IntCallback ( funcVoid UpdateInt_CB ) { UpdateInterrupts = UpdateInt_CB; };
		

		static const u32 c_InterruptBit = 1;
		static const u32 c_InterruptBit_Vsync = 0;
		
#ifdef PS2_COMPILE
		static const u32 c_InterruptBit_EVsync = 11;
#endif

		//static u32* _Intc_Master;
		static u32* _Intc_Stat;
		static u32* _Intc_Mask;
		static u32* _R3000A_Status_12;
		static u32* _R3000A_Cause_13;
		static u64* _ProcStatus;
		
		inline void ConnectInterrupt ( u32* _IStat, u32* _IMask, u32* _R3000A_Status, u32* _R3000A_Cause, u64* _ProcStat )
		{
			_Intc_Stat = _IStat;
			_Intc_Mask = _IMask;
			//_Intc_Master = _IMaster;
			_R3000A_Cause_13 = _R3000A_Cause;
			_R3000A_Status_12 = _R3000A_Status;
			_ProcStatus = _ProcStat;
		}
		
		
		inline static void SetInterrupt ()
		{
			//*_Intc_Master |= ( 1 << c_InterruptBit );
			*_Intc_Stat |= ( 1 << c_InterruptBit );
			
			UpdateInterrupts ();
			
			/*
			if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 ); else *_ProcStatus &= ~( 1 << 20 );
			*/
		}
		
		inline static void ClearInterrupt ()
		{
			//*_Intc_Master &= ~( 1 << c_InterruptBit );
			*_Intc_Stat &= ~( 1 << c_InterruptBit );

			UpdateInterrupts ();

			/*
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 ); else *_ProcStatus &= ~( 1 << 20 );
			*/
		}

		
		
		inline static void SetInterrupt_Vsync ()
		{
			//*_Intc_Master |= ( 1 << c_InterruptBit_Vsync );
			*_Intc_Stat |= ( 1 << c_InterruptBit_Vsync );
			
			UpdateInterrupts ();
			
			/*
			if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 ); else *_ProcStatus &= ~( 1 << 20 );
			*/
		}
		
		inline static void ClearInterrupt_Vsync ()
		{
			//*_Intc_Master &= ~( 1 << c_InterruptBit_Vsync );
			*_Intc_Stat &= ~( 1 << c_InterruptBit_Vsync );
			
			UpdateInterrupts ();
			
			/*
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 ); else *_ProcStatus &= ~( 1 << 20 );
			*/
		}


#ifdef PS2_COMPILE
		inline static void SetInterrupt_EVsync ()
		{
			*_Intc_Stat |= ( 1 << c_InterruptBit_EVsync );
			
			UpdateInterrupts ();
			
			/*
			if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 ); else *_ProcStatus &= ~( 1 << 20 );
			*/
		}
#endif

		
		static u64* _NextSystemEvent;

		
		////////////////////////////////
		// Debug Info
		static u32* _DebugPC;
		static u64* _DebugCycleCount;
		static u64* _SystemCycleCount;
		static u32 *_NextEventIdx;
		
		static bool DebugWindow_Enabled;
		static void DebugWindow_Enable ();
		static void DebugWindow_Disable ();
		static void DebugWindow_Update ();


/*
template<const long ABE, const long TGE>
void SelectSprite_t ();

template<const long ABE, const long TGE>
void Draw_Sprite_64_t ();
template<const long ABE, const long TGE>
void Draw_Sprite8x8_74_t ();
template<const long ABE, const long TGE>
void Draw_Sprite16x16_7c_t ();
*/


template <const long ABRCODE>		
inline static u16 SemiTransparency16_t ( u16 B, u16 F )
{
	static const u32 ShiftSame = 0;
	static const u32 ShiftHalf = 1;
	static const u32 ShiftQuarter = 2;
	
	static const u32 c_iBitsPerPixel = 5;
	//static const u32 c_iShiftHalf_Mask = ~( ( 1 << 4 ) + ( 1 << 9 ) );
	static const u32 c_iShiftHalf_Mask = ~( ( 1 << ( c_iBitsPerPixel - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 2 ) - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 3 ) - 1 ) ) );
	//static const u32 c_iShiftQuarter_Mask = ~( ( 3 << 3 ) + ( 3 << 8 ) );
	static const u32 c_iShiftQuarter_Mask = ~( ( 3 << ( c_iBitsPerPixel - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 2 ) - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 3 ) - 2 ) ) );
	//static const u32 c_iClamp_Mask = ( ( 1 << 5 ) + ( 1 << 10 ) + ( 1 << 15 ) );
	static const u32 c_iClampMask = ( ( 1 << ( c_iBitsPerPixel ) ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) + ( 1 << ( c_iBitsPerPixel * 3 ) ) );
	static const u32 c_iLoBitMask = ( ( 1 ) + ( 1 << c_iBitsPerPixel ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) );
	static const u32 c_iPixelMask = 0x7fff;
	
	//u32 Red, Green, Blue;
	
	u32 Color, Actual, Mask;
	
	switch ( ABRCODE )
	{
		// 0.5xB+0.5 xF
		case 0:
			//Color = SetRed16 ( Clamp5 ( ( GetRed16 ( B ) >> ShiftHalf ) + ( GetRed16( F ) >> ShiftHalf ) ) ) |
			//		SetGreen16 ( Clamp5 ( ( GetGreen16 ( B ) >> ShiftHalf ) + ( GetGreen16( F ) >> ShiftHalf ) ) ) |
			//		SetBlue16 ( Clamp5 ( ( GetBlue16 ( B ) >> ShiftHalf ) + ( GetBlue16( F ) >> ShiftHalf ) ) );
			
			Mask = B & F & c_iLoBitMask;
			Color = ( ( B >> 1 ) & c_iShiftHalf_Mask ) + ( ( F >> 1 ) & c_iShiftHalf_Mask ) + Mask;
			return Color;
			
			break;
		
		// 1.0xB+1.0 xF
		case 1:
			//Color = SetRed16 ( Clamp5 ( ( GetRed16 ( B ) >> ShiftSame ) + ( GetRed16( F ) >> ShiftSame ) ) ) |
			//		SetGreen16 ( Clamp5 ( ( GetGreen16 ( B ) >> ShiftSame ) + ( GetGreen16( F ) >> ShiftSame ) ) ) |
			//		SetBlue16 ( Clamp5 ( ( GetBlue16 ( B ) >> ShiftSame ) + ( GetBlue16( F ) >> ShiftSame ) ) );
			
			B &= c_iPixelMask;
			F &= c_iPixelMask;
			Actual = B + F;
			Mask = ( B ^ F ^ Actual ) & c_iClampMask;
			Color = Actual - Mask;
			Mask -= ( Mask >> c_iBitsPerPixel );
			Color |= Mask;
			return Color;
			
			break;
			
		// 1.0xB-1.0 xF
		case 2:
			//Color = SetRed16 ( Clamp5 ( (s32) ( GetRed16 ( B ) >> ShiftSame ) - (s32) ( GetRed16( F ) >> ShiftSame ) ) ) |
			//		SetGreen16 ( Clamp5 ( (s32) ( GetGreen16 ( B ) >> ShiftSame ) - (s32) ( GetGreen16( F ) >> ShiftSame ) ) ) |
			//		SetBlue16 ( Clamp5 ( (s32) ( GetBlue16 ( B ) >> ShiftSame ) - (s32) ( GetBlue16( F ) >> ShiftSame ) ) );
			
			B &= c_iPixelMask;
			F &= c_iPixelMask;
			Actual = B - F;
			Mask = ( B ^ F ^ Actual ) & c_iClampMask;
			Color = Actual + Mask;
			Mask -= ( Mask >> c_iBitsPerPixel );
			Color &= ~Mask;
			return Color;
			
			break;
			
		// 1.0xB+0.25xF
		case 3:
			//Color = SetRed16 ( Clamp5 ( ( GetRed16 ( B ) >> ShiftSame ) + ( GetRed16( F ) >> ShiftQuarter ) ) ) |
			//		SetGreen16 ( Clamp5 ( ( GetGreen16 ( B ) >> ShiftSame ) + ( GetGreen16( F ) >> ShiftQuarter ) ) ) |
			//		SetBlue16 ( Clamp5 ( ( GetBlue16 ( B ) >> ShiftSame ) + ( GetBlue16( F ) >> ShiftQuarter ) ) );
			
			B &= c_iPixelMask;
			F = ( F >> 2 ) & c_iShiftQuarter_Mask;
			Actual = B + F;
			Mask = ( B ^ F ^ Actual ) & c_iClampMask;
			Color = Actual - Mask;
			Mask -= ( Mask >> c_iBitsPerPixel );
			Color |= Mask;
			return Color;
			
			break;
	}
	
	//return Color;
}






template <const long ABRCODE>		
inline static u64 SemiTransparency16_64t ( u64 B, u64 F )
{
	static const u32 ShiftSame = 0;
	static const u32 ShiftHalf = 1;
	static const u32 ShiftQuarter = 2;
	
	static const u32 c_iBitsPerPixel = 5;
	
	static const u64 c_iShiftHalf_Mask64 = ( ~( ( 1 << ( c_iBitsPerPixel - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 2 ) - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 3 ) - 1 ) ) ) ) & 0xffff;
	static const u64 c_iShiftQuarter_Mask64 = ( ~( ( 3 << ( c_iBitsPerPixel - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 2 ) - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 3 ) - 2 ) ) ) ) & 0xffff;
	
	static const u64 c_iClampMask64 = ( ( 1 << ( c_iBitsPerPixel ) ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) + ( 1 << ( c_iBitsPerPixel * 3 ) ) );
	static const u64 c_iLoBitMask64 = ( ( 1 ) + ( 1 << c_iBitsPerPixel ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) );
	static const u64 c_iPixelMask64 = 0x7fff;
	
	static const u64 c_iShiftHalf_Mask = ( c_iShiftHalf_Mask64 ) + ( c_iShiftHalf_Mask64 << 16 ) + ( c_iShiftHalf_Mask64 << 32 ) + ( c_iShiftHalf_Mask64 << 48 );
	static const u64 c_iShiftQuarter_Mask = ( c_iShiftQuarter_Mask64 ) + ( c_iShiftQuarter_Mask64 << 16 ) + ( c_iShiftQuarter_Mask64 << 32 ) + ( c_iShiftQuarter_Mask64 << 48 );
	
	static const u64 c_iClampMask = ( c_iClampMask64 ) + ( c_iClampMask64 << 16 ) + ( c_iClampMask64 << 32 ) + ( c_iClampMask64 << 48 );
	static const u64 c_iLoBitMask = ( c_iLoBitMask64 ) + ( c_iLoBitMask64 << 16 ) + ( c_iLoBitMask64 << 32 ) + ( c_iLoBitMask64 << 48 );
	static const u64 c_iPixelMask = ( c_iPixelMask64 ) + ( c_iPixelMask64 << 16 ) + ( c_iPixelMask64 << 32 ) + ( c_iPixelMask64 << 48 );
	
	u64 Color, Actual, Mask;
	
	switch ( ABRCODE )
	{
		// 0.5xB+0.5 xF
		case 0:
			//Color = SetRed16 ( Clamp5 ( ( GetRed16 ( B ) >> ShiftHalf ) + ( GetRed16( F ) >> ShiftHalf ) ) ) |
			//		SetGreen16 ( Clamp5 ( ( GetGreen16 ( B ) >> ShiftHalf ) + ( GetGreen16( F ) >> ShiftHalf ) ) ) |
			//		SetBlue16 ( Clamp5 ( ( GetBlue16 ( B ) >> ShiftHalf ) + ( GetBlue16( F ) >> ShiftHalf ) ) );
			
			Mask = B & F & c_iLoBitMask;
			Color = ( ( B >> 1 ) & c_iShiftHalf_Mask ) + ( ( F >> 1 ) & c_iShiftHalf_Mask ) + Mask;
			return Color;
			
			break;
		
		// 1.0xB+1.0 xF
		case 1:
			//Color = SetRed16 ( Clamp5 ( ( GetRed16 ( B ) >> ShiftSame ) + ( GetRed16( F ) >> ShiftSame ) ) ) |
			//		SetGreen16 ( Clamp5 ( ( GetGreen16 ( B ) >> ShiftSame ) + ( GetGreen16( F ) >> ShiftSame ) ) ) |
			//		SetBlue16 ( Clamp5 ( ( GetBlue16 ( B ) >> ShiftSame ) + ( GetBlue16( F ) >> ShiftSame ) ) );
			
			B &= c_iPixelMask;
			F &= c_iPixelMask;
			Actual = B + F;
			Mask = ( B ^ F ^ Actual ) & c_iClampMask;
			Color = Actual - Mask;
			Mask -= ( Mask >> c_iBitsPerPixel );
			Color |= Mask;
			return Color;
			
			break;
			
		// 1.0xB-1.0 xF
		case 2:
			//Color = SetRed16 ( Clamp5 ( (s32) ( GetRed16 ( B ) >> ShiftSame ) - (s32) ( GetRed16( F ) >> ShiftSame ) ) ) |
			//		SetGreen16 ( Clamp5 ( (s32) ( GetGreen16 ( B ) >> ShiftSame ) - (s32) ( GetGreen16( F ) >> ShiftSame ) ) ) |
			//		SetBlue16 ( Clamp5 ( (s32) ( GetBlue16 ( B ) >> ShiftSame ) - (s32) ( GetBlue16( F ) >> ShiftSame ) ) );
			
			B &= c_iPixelMask;
			F &= c_iPixelMask;
			Actual = B - F;
			Mask = ( B ^ F ^ Actual ) & c_iClampMask;
			Color = Actual + Mask;
			Mask -= ( Mask >> c_iBitsPerPixel );
			Color &= ~Mask;
			return Color;
			
			break;
			
		// 1.0xB+0.25xF
		case 3:
			//Color = SetRed16 ( Clamp5 ( ( GetRed16 ( B ) >> ShiftSame ) + ( GetRed16( F ) >> ShiftQuarter ) ) ) |
			//		SetGreen16 ( Clamp5 ( ( GetGreen16 ( B ) >> ShiftSame ) + ( GetGreen16( F ) >> ShiftQuarter ) ) ) |
			//		SetBlue16 ( Clamp5 ( ( GetBlue16 ( B ) >> ShiftSame ) + ( GetBlue16( F ) >> ShiftQuarter ) ) );
			
			B &= c_iPixelMask;
			F = ( F >> 2 ) & c_iShiftQuarter_Mask;
			Actual = B + F;
			Mask = ( B ^ F ^ Actual ) & c_iClampMask;
			Color = Actual - Mask;
			Mask -= ( Mask >> c_iBitsPerPixel );
			Color |= Mask;
			return Color;
			
			break;
	}
	
	//return Color;
}







template<const long PIXELMASK, const long long SETPIXELMASK, const long ABE, const long ABRCODE>
inline void GPU::Draw_Rectangle_t ()
{
	// before sse2, was sending 1 pixels at a time
	static const int c_iVectorSize = 1;
	static const int c_iVectorSize8 = 8;

	s32 x0, y0, x1, y1;
	u64 pixel, pixel_temp;
	
	s32 StartX, EndX, StartY, EndY;
	//u32 PixelsPerLine;
	
	union
	{
	u16 *ptr;
	u16 *ptr16;
	u32 *ptr32;
	u64 *ptr64;
	};
	
	u64 DestPixel;
	
	s32 Line;
	s64 x_across;
	u32 bgr;
	
#ifdef _ENABLE_SSE2_RECTANGLE_MONO
	static const u32 c_iBitsPerPixel = 5;
	static const u32 c_iShiftHalf_Mask = ~( ( 1 << ( c_iBitsPerPixel - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 2 ) - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 3 ) - 1 ) ) );
	static const u32 c_iShiftQuarter_Mask = ~( ( 3 << ( c_iBitsPerPixel - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 2 ) - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 3 ) - 2 ) ) );
	static const u32 c_iClampMask = ( ( 1 << ( c_iBitsPerPixel ) ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) + ( 1 << ( c_iBitsPerPixel * 3 ) ) );
	static const u32 c_iLoBitMask = ( ( 1 ) + ( 1 << c_iBitsPerPixel ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) );
	static const u32 c_iPixelMask = 0x7fff;
	static const __m128i c_vLoBitMask = _mm_set1_epi16 ( c_iLoBitMask );
	static const __m128i c_vPixelMask = _mm_set1_epi16 ( c_iPixelMask );
	static const __m128i c_vClampMask = _mm_set1_epi16 ( c_iClampMask );
	static const __m128i c_vShiftHalf_Mask = _mm_set1_epi16 ( c_iShiftHalf_Mask );
	static const __m128i c_vShiftQuarter_Mask = _mm_set1_epi16 ( c_iShiftQuarter_Mask );
	__m128i vDestPixel, vPixelMask, vSetPixelMask, vStoreMask;
	__m128i vStartX, vEndX, vx_across, vVectorSize;
	__m128i vpixel, vpixel_temp;
	
	static const __m128i vSeq = _mm_set_epi16 ( 7, 6, 5, 4, 3, 2, 1, 0 );
	vVectorSize = _mm_set1_epi16 ( c_iVectorSize8 );
	
	vpixel = _mm_set1_epi16 ( bgr );
	//PixelMask = _mm_setzero_si128 ();
	//SetPixelMask = _mm_setzero_si128 ();
	if ( PIXELMASK ) { vPixelMask = _mm_set1_epi16 ( 0x8080 ); }
	if ( SETPIXELMASK ) { vSetPixelMask = _mm_set1_epi16 ( 0x8000 ); }
#endif

	

	// initialize number of pixels drawn
	NumberOfPixelsDrawn = 0;
	
	// check for some important conditions
	if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
		return;
	}
	
	
	if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
		return;
	}
	
	// get top left corner of sprite and bottom right corner of sprite
	x0 = x;
	y0 = y;
	x1 = x + w - 1;
	y1 = y + h - 1;
	
	// get color(s)
	bgr = gbgr [ 0 ];
	
	// ?? convert to 16-bit ?? (or should leave 24-bit?)
	bgr = ( ( bgr & ( 0xf8 << 0 ) ) >> 3 ) | ( ( bgr & ( 0xf8 << 8 ) ) >> 6 ) | ( ( bgr & ( 0xf8 << 16 ) ) >> 9 );
	
	pixel = bgr;
	
#ifdef ENABLE_TEMPLATE_MULTIPIXEL
	if ( !PIXELMASK )
	{
		pixel |= ( pixel << 16 ) | ( pixel << 32 ) | ( pixel << 48 );
	}
#endif
	
	//////////////////////////////////////////
	// get coordinates on screen
	x0 = DrawArea_OffsetX + x0;
	y0 = DrawArea_OffsetY + y0;
	x1 = DrawArea_OffsetX + x1;
	y1 = DrawArea_OffsetY + y1;
	
	// check if sprite is within draw area
	if ( x1 < ((s32)DrawArea_TopLeftX) || x0 > ((s32)DrawArea_BottomRightX) || y1 < ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	
	
	
	StartX = x0;
	EndX = x1;
	StartY = y0;
	EndY = y1;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		StartY = DrawArea_TopLeftY;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY;
	}
	
	if ( StartX < ((s32)DrawArea_TopLeftX) )
	{
		StartX = DrawArea_TopLeftX;
	}
	
	if ( EndX > ((s32)DrawArea_BottomRightX) )
	{
		EndX = DrawArea_BottomRightX;
	}

#ifdef _ENABLE_SSE2_RECTANGLE_MONO
	vStartX = _mm_add_epi16 ( vSeq, _mm_set1_epi16 ( StartX ) );
	vEndX = _mm_set1_epi16 ( EndX + 1 );
#endif

	NumberOfPixelsDrawn = ( EndX - StartX + 1 ) * ( EndY - StartY + 1 );
	
	if ( ABE )
	{
#ifdef _ENABLE_SSE2_RECTANGLE_MONO
		vpixel_temp = vpixel;
#else
		pixel_temp = pixel;
#endif
	}
	
	if ( SETPIXELMASK && !ABE )
	{
#ifdef _ENABLE_SSE2_RECTANGLE_MONO
		vpixel = _mm_or_si128 ( vpixel, vSetPixelMask );
#else
		// check if we should set mask bit when drawing
		pixel |= SETPIXELMASK;
#endif
	}
	
	
	for ( Line = StartY; Line <= EndY; Line++ )
	{
		ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
		
#ifdef _ENABLE_SSE2_RECTANGLE_MONO
	vx_across = vStartX;
#endif

		// draw horizontal line
#ifdef _ENABLE_SSE2_RECTANGLE_MONO
		for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize8 )
		{
			if ( ABE || PIXELMASK )
			{
				vDestPixel = _mm_loadu_si128 ((__m128i const*) ptr);
			}
			
			if ( ABE )
			{
				vpixel = vSemiTransparency16_t<ABRCODE>( vDestPixel, vpixel_temp, c_vLoBitMask, c_vPixelMask, c_vClampMask, c_vShiftHalf_Mask, c_vShiftQuarter_Mask );
				
				if ( SETPIXELMASK )
				{
					vpixel = _mm_or_si128 ( vpixel, vSetPixelMask );
				}
			}
			
			if ( PIXELMASK )
			{
				vStoreMask = _mm_andnot_si128 ( _mm_and_si128 ( _mm_srai_epi16 ( vDestPixel, 15 ), vPixelMask ), _mm_cmplt_epi16 ( vx_across, vEndX ) );
			}
			else
			{
				vStoreMask = _mm_cmplt_epi16 ( vx_across, vEndX );
			}
			
			_mm_maskmoveu_si128 ( vpixel, vStoreMask, (char*) ptr );
			vx_across = _mm_add_epi16 ( vx_across, vVectorSize );
			
			ptr += c_iVectorSize8;
		}
#else
#ifdef ENABLE_TEMPLATE_MULTIPIXEL
		if ( /* !ABE && */ !PIXELMASK )
		{
			x_across = StartX;
			if ( ( x_across & 1 ) && ( x_across <= EndX ) )
			{
				if ( ABE )
				{
					DestPixel = *ptr16;
					pixel = SemiTransparency16_t<ABRCODE> ( DestPixel, pixel_temp );
					if ( SETPIXELMASK ) pixel |= SETPIXELMASK;
				}
				*ptr16++ = pixel;
				x_across++;
			}
			if ( ( x_across & 3 ) && ( x_across <= ( EndX - 1 ) ) )
			{
				if ( ABE )
				{
					DestPixel = *ptr32;
					pixel = SemiTransparency16_64t<ABRCODE> ( DestPixel, pixel_temp );
					if ( SETPIXELMASK ) pixel |= SETPIXELMASK;
				}
				*ptr32++ = pixel;
				x_across += 2;
			}
			while ( x_across <= ( EndX - 3 ) )
			{
				if ( ABE )
				{
					DestPixel = *ptr64;
					pixel = SemiTransparency16_64t<ABRCODE> ( DestPixel, pixel_temp );
					if ( SETPIXELMASK ) pixel |= SETPIXELMASK;
				}
				*ptr64++ = pixel;
				x_across += 4;
			}
			if ( x_across <= ( EndX - 1 ) )
			{
				if ( ABE )
				{
					DestPixel = *ptr32;
					pixel = SemiTransparency16_64t<ABRCODE> ( DestPixel, pixel_temp );
					if ( SETPIXELMASK ) pixel |= SETPIXELMASK;
				}
				*ptr32++ = pixel;
				x_across += 2;
			}
			if ( x_across <= EndX )
			{
				if ( ABE )
				{
					DestPixel = *ptr16;
					pixel = SemiTransparency16_t<ABRCODE> ( DestPixel, pixel_temp );
					if ( SETPIXELMASK ) pixel |= SETPIXELMASK;
				}
				*ptr16++ = pixel;
				x_across++;
			}
		}
		else
#endif
		{
		for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
		{
			if ( ABE || PIXELMASK )
			{
				// read pixel from frame buffer if we need to check mask bit
				DestPixel = *ptr;
			}
			
			// semi-transparency
			if ( ABE )
			{
				pixel = SemiTransparency16_t<ABRCODE> ( DestPixel, pixel_temp );
				
				if ( SETPIXELMASK )
				{
					// check if we should set mask bit when drawing
					pixel |= SETPIXELMASK;
				}
			}
			

			// draw pixel if we can draw to mask pixels or mask bit not set
			if ( PIXELMASK )
			{
				if ( ! ( DestPixel & PIXELMASK ) ) *ptr = pixel;
			}
			else
			{
				*ptr = pixel;
			}
			
			// update pointer for pixel out
			ptr += c_iVectorSize;
		}
		} // end if ( !ABE && !PIXELMASK )
#endif
	}
	
	// set the amount of time drawing used up
	BusyCycles = NumberOfPixelsDrawn * 1;
	
}




template<const long PIXELMASK, const long ABE, const long ABRCODE>
inline void GPU::SelectRectangle3_t ()
{
	if ( GPU_CTRL_Read.MD )
	{
		Draw_Rectangle_t <PIXELMASK,0x8000800080008000LL,ABE,ABRCODE> ();
	}
	else
	{
		Draw_Rectangle_t <PIXELMASK,0,ABE,ABRCODE> ();
	}
}


template<const long ABE, const long ABRCODE>
inline void GPU::SelectRectangle2_t ()
{
	if ( GPU_CTRL_Read.ME )
	{
		SelectRectangle3_t <0x8000,ABE,ABRCODE> ();
	}
	else
	{
		SelectRectangle3_t <0,ABE,ABRCODE> ();
	}
}


template<const long ABE>
void GPU::SelectRectangle_t ()
{
	switch ( GPU_CTRL_Read.ABR )
	{
		case 0:
			SelectRectangle2_t <ABE,0> ();
			break;
			
		case 1:
			SelectRectangle2_t <ABE,1> ();
			break;
			
		case 2:
			SelectRectangle2_t <ABE,2> ();
			break;
			
		case 3:
			SelectRectangle2_t <ABE,3> ();
			break;
	}
}


template<const long ABE>
void GPU::Draw_Rectangle_60_t ()
{
	SelectRectangle_t <ABE> ();
}

template<const long ABE>
void GPU::Draw_Rectangle8x8_70_t ()
{
	w = 8; h = 8;
	//Draw_Rectangle_60 ();
	SelectRectangle_t <ABE> ();
}


template<const long ABE>
void GPU::Draw_Rectangle16x16_78_t ()
{
	w = 16; h = 16;
	//Draw_Rectangle_60 ();
	SelectRectangle_t <ABE> ();
}





//template<const long PIXELMASK, const long SETPIXELMASK, const long ABE, const long ABRCODE, const long TGE, const long TP>
//void GPU::PlotPixel_Texture ( 




template<const long PIXELMASK, const long SETPIXELMASK, const long ABE, const long ABRCODE, const long TGE, const long TP>
inline void GPU::DrawSprite_t ()
{
	// before sse2, was sending 1 pixels at a time
	static const int c_iVectorSize = 1;

	// notes: looks like sprite size is same as specified by w/h

	//u32 Pixel,
	
	u32 pixel, pixel_temp;
	
	u32 TexelIndex;
	
	u32 color_add;
	
	u16 *ptr_texture, *ptr_clut;
	u32 clut_xoffset, clut_yoffset;
	
	u16 *ptr;
	s32 StartX, EndX, StartY, EndY;
	
	//u32 tge;
	
	u32 DestPixel;
	u32 TexCoordX, TexCoordY;
	
	// new local variables
	s32 x0, x1, y0, y1;
	s32 u0, v0;
	u32 bgr, bgr_temp;
	s64 iU, iV;
	s64 x_across;
	s32 Line;
	
#ifdef _ENABLE_SSE2_SPRITE
	static const u32 c_iBitsPerPixel = 5;
	static const u32 c_iShiftHalf_Mask = ~( ( 1 << ( c_iBitsPerPixel - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 2 ) - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 3 ) - 1 ) ) );
	static const u32 c_iShiftQuarter_Mask = ~( ( 3 << ( c_iBitsPerPixel - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 2 ) - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 3 ) - 2 ) ) );
	static const u32 c_iClampMask = ( ( 1 << ( c_iBitsPerPixel ) ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) + ( 1 << ( c_iBitsPerPixel * 3 ) ) );
	static const u32 c_iLoBitMask = ( ( 1 ) + ( 1 << c_iBitsPerPixel ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) );
	static const u32 c_iPixelMask = 0x7fff;
	static const __m128i c_vLoBitMask = _mm_set1_epi16 ( c_iLoBitMask );
	static const __m128i c_vPixelMask = _mm_set1_epi16 ( c_iPixelMask );
	static const __m128i c_vClampMask = _mm_set1_epi16 ( c_iClampMask );
	static const __m128i c_vShiftHalf_Mask = _mm_set1_epi16 ( c_iShiftHalf_Mask );
	static const __m128i c_vShiftQuarter_Mask = _mm_set1_epi16 ( c_iShiftQuarter_Mask );
	__m128i vDestPixel, vPixelMask, vSetPixelMask, vStoreMask;
	__m128i vStartX, vEndX, vx_across, vVectorSize;
	__m128i vpixel, vpixel_temp;
	
	static const __m128i vSeq = _mm_set_epi16 ( 7, 6, 5, 4, 3, 2, 1, 0 );
	vVectorSize = _mm_set1_epi16 ( c_iVectorSize8 );
	
	vpixel = _mm_set1_epi16 ( bgr );
	//PixelMask = _mm_setzero_si128 ();
	//SetPixelMask = _mm_setzero_si128 ();
	if ( PIXELMASK ) { vPixelMask = _mm_set1_epi16 ( 0x8080 ); }
	if ( SETPIXELMASK ) { vSetPixelMask = _mm_set1_epi16 ( 0x8000 ); }
	
	// variables for textures //
	__m128i vTWYTWH, vTWXTWW, vNot_TWH, vNot_TWW;
	__m128i viU, viV, 
#endif

	u32 TWYTWH, TWXTWW, Not_TWH, Not_TWW;
	
	TWYTWH = ( ( TWY & TWH ) << 3 );
	TWXTWW = ( ( TWX & TWW ) << 3 );
	
	Not_TWH = ~( TWH << 3 );
	Not_TWW = ~( TWW << 3 );

	
	//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
	
	/////////////////////////////////////////////////////////
	// Get offset into texture page
	//TextureOffset = ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 );
	
	//u32 PixelsPerLine;

	//tge = command_tge;
	//if ( ( bgr & 0x00ffffff ) == 0x00808080 ) tge = 1;
	
	//////////////////////////////////////////////////////
	// Get offset into color lookup table
	//u32 ClutOffset = ( clut_x << 4 ) + ( clut_y << 10 );
	
	clut_xoffset = clut_x << 4;
	ptr_clut = & ( VRAM [ clut_y << 10 ] );
	ptr_texture = & ( VRAM [ ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) ] );
	
	/*
	if ( tpage_tp == 0 )
	{
		And2 = 0xf;
		
		Shift1 = 2; Shift2 = 2;
		And1 = 3; And2 = 0xf;
	}
	else if ( tpage_tp == 1 )
	{
		And2 = 0xff;
		
		Shift1 = 1; Shift2 = 3;
		And1 = 1; And2 = 0xff;
	}
	*/
	

	// initialize number of pixels drawn
	NumberOfPixelsDrawn = 0;
	
	// check for some important conditions
	if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
		return;
	}
	
	if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
		return;
	}


	// get the color
	bgr = gbgr [ 0 ];
	
	color_add = bgr;

	
	// get top left corner of sprite and bottom right corner of sprite
	x0 = x;
	y0 = y;
	x1 = x + w - 1;
	y1 = y + h - 1;

	// get texture coords
	u0 = u;
	v0 = v;
	
	//////////////////////////////////////////
	// get coordinates on screen
	x0 = DrawArea_OffsetX + x0;
	y0 = DrawArea_OffsetY + y0;
	x1 = DrawArea_OffsetX + x1;
	y1 = DrawArea_OffsetY + y1;
	
	// check if sprite is within draw area
	if ( x1 < ((s32)DrawArea_TopLeftX) || x0 > ((s32)DrawArea_BottomRightX) || y1 < ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	

	StartX = x0;
	EndX = x1;
	StartY = y0;
	EndY = y1;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		v0 += ( DrawArea_TopLeftY - StartY );
		StartY = DrawArea_TopLeftY;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY;
	}
	
	if ( StartX < ((s32)DrawArea_TopLeftX) )
	{
		u0 += ( DrawArea_TopLeftX - StartX );
		StartX = DrawArea_TopLeftX;
	}
	
	if ( EndX > ((s32)DrawArea_BottomRightX) )
	{
		EndX = DrawArea_BottomRightX;
	}

	
	iV = v0;
	
	NumberOfPixelsDrawn = ( EndX - StartX + 1 ) * ( EndY - StartY + 1 );
		


//#define DEBUG_DRAWSPRITE
#ifdef DEBUG_DRAWSPRITE
	debug << "\r\nTWX=" << TWX << " TWY=" << TWY << " TWW=" << TWW << " TWH=" << TWH << " TextureWindow_X=" << TextureWindow_X << " TextureWindow_Y=" << TextureWindow_Y << " TextureWindow_Width=" << TextureWindow_Width << " TextureWindow_Height=" << TextureWindow_Height;
#endif

	for ( Line = StartY; Line <= EndY; Line++ )
	{
			// need to start texture coord from left again
			iU = u0;
			//TexCoordY = (u8) ( ( iV & ~( TWH << 3 ) ) | ( ( TWY & TWH ) << 3 ) );

			TexCoordY = (u8) ( ( iV & Not_TWH ) | ( TWYTWH ) );
			TexCoordY <<= 10;

			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			

			// draw horizontal line
			//for ( x_across = StartX; x_across <= EndX; x_across++ )
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
					TexCoordX = (u8) ( ( iU & Not_TWW ) | ( TWXTWW ) );
					
					// vars needed: TexCoordX, TexCoordY, clut_xoffset, color_add, ptr_texture, ptr_clut
					switch ( TP )
					{
						case 0:
							//And2 = 0xf;
							//Shift1 = 2; Shift2 = 2;
							//And1 = 3; And2 = 0xf;
							//bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + TexCoordY ];
							//bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2 ) ) & FrameBuffer_XMask ];
							pixel = ptr_texture [ ( TexCoordX >> 2 ) + TexCoordY ];
							pixel = ptr_clut [ ( clut_xoffset + ( ( pixel >> ( ( TexCoordX & 3 ) << 2 ) ) & 0xf ) ) & FrameBuffer_XMask ];
							break;
							
						case 1:
							//And2 = 0xff;
							//Shift1 = 1; Shift2 = 3;
							//And1 = 1; And2 = 0xff;
							pixel = ptr_texture [ ( TexCoordX >> 1 ) + TexCoordY ];
							pixel = ptr_clut [ ( clut_xoffset + ( ( pixel >> ( ( TexCoordX & 1 ) << 3 ) ) & 0xff ) ) & FrameBuffer_XMask ];
							break;
							
						case 2:
							pixel = ptr_texture [ ( TexCoordX ) + TexCoordY ];
							break;
					}
					
					
					if ( pixel )
					{
						if ( ABE || PIXELMASK )
						{
							// read pixel from frame buffer if we need to check mask bit
							DestPixel = *ptr;
						}

						if ( ABE || !TGE )
						{
							pixel_temp = pixel & 0x8000;
						}
				
						//if ( !tge )
						if ( !TGE )
						{
							// brightness calculation
							pixel = ColorMultiply1624 ( pixel, color_add );
						}
						
						// semi-transparency
						//if ( command_abe && ( bgr & 0x8000 ) )
						if ( ABE )
						{
							if ( pixel_temp )
							{
								pixel = SemiTransparency16_t <ABRCODE> ( DestPixel, pixel );
							}
						}
						
						if ( ABE || !TGE )
						{
							// check if we should set mask bit when drawing
							//bgr_temp |= SetPixelMask | ( bgr & 0x8000 );
							pixel |= pixel_temp;
						}
						
						if ( SETPIXELMASK )
						{
							// set pixel mask if specified
							pixel |= SETPIXELMASK;
						}

						// draw pixel if we can draw to mask pixels or mask bit not set
						if ( PIXELMASK )
						{
							if ( ! ( DestPixel & PIXELMASK ) ) *ptr = pixel;
						}
						else
						{
							*ptr = pixel;
						}
					}
					
				/////////////////////////////////////////////////////////
				// interpolate texture coords across
				//iU++;	//+= dU_across;
				iU += c_iVectorSize;
				
				// update pointer for pixel out
				//ptr++;
				ptr += c_iVectorSize;
					
			}
		
		/////////////////////////////////////////////////////////
		// interpolate texture coords down
		iV++;	//+= dV_left;
	}
}


template<const long PIXELMASK, const long SETPIXELMASK, const long ABE, const long ABRCODE, const long TGE, const long TP>
inline void GPU::SelectSprite5_t ()
{
	if ( gbgr [ 0 ] == 0x808080 )
	{
		DrawSprite_t <PIXELMASK, SETPIXELMASK, ABE, ABRCODE, 1, TP> ();
	}
	else
	{
		DrawSprite_t <PIXELMASK, SETPIXELMASK, ABE, ABRCODE, TGE, TP> ();
	}
}



template<const long PIXELMASK, const long ABE, const long ABRCODE, const long TGE, const long TP>
inline void GPU::SelectSprite4_t ()
{
	if ( GPU_CTRL_Read.MD )
	{
		SelectSprite5_t <PIXELMASK, 0x8000, ABE, ABRCODE, TGE, TP> ();
	}
	else
	{
		SelectSprite5_t <PIXELMASK, 0, ABE, ABRCODE, TGE, TP> ();
	}
}


template<const long ABE, const long ABRCODE, const long TGE, const long TP>
inline void GPU::SelectSprite3_t ()
{
	if ( GPU_CTRL_Read.ME )
	{
		SelectSprite4_t<0x8000, ABE, ABRCODE, TGE, TP> ();
	}
	else
	{
		SelectSprite4_t<0, ABE, ABRCODE, TGE, TP> ();
	}
}


template<const long ABE, const long ABRCODE, const long TGE>
inline void GPU::SelectSprite2_t ()
{
	switch ( GPU_CTRL_Read.TP )
	{
		case 0:
			SelectSprite3_t<ABE, ABRCODE, TGE, 0> ();
			break;
			
		case 1:
			SelectSprite3_t<ABE, ABRCODE, TGE, 1> ();
			break;
			
		case 2:
			SelectSprite3_t<ABE, ABRCODE, TGE, 2> ();
			break;
	}
}




template<const long PIXELMASK, const long long SETPIXELMASK, const long ABE, const long ABRCODE>
void GPU::DrawTriangle_Mono_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{	
	// before sse2, was sending 1 pixels at a time
	static const int c_iVectorSize = 1;
	static const int c_iVectorSize8 = 8;

	//s32 x0, y0, x1, y1;
	u64 pixel, pixel_temp;
	
	union
	{
	u16 *ptr;
	u16 *ptr16;
	u32 *ptr32;
	u64 *ptr64;
	};
	
	s64 Temp;
	s64 LeftMostX, RightMostX;
	
	s32 StartX, EndX, StartY, EndY;
	//s64 Error_Left;
	
	u64 DestPixel;

	s64 r10, r20, r21;

	// new local variables
	s32 x0, x1, x2, y0, y1, y2;
	s64 dx_left, dx_right;
	s64 x_left, x_right, x_across;
	u32 bgr, bgr_temp;
	s32 Line;
	s64 t0, t1, denominator;
	
#ifdef _ENABLE_SSE2_TRIANGLE_MONO
	static const u32 c_iBitsPerPixel = 5;
	static const u32 c_iShiftHalf_Mask = ~( ( 1 << ( c_iBitsPerPixel - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 2 ) - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 3 ) - 1 ) ) );
	static const u32 c_iShiftQuarter_Mask = ~( ( 3 << ( c_iBitsPerPixel - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 2 ) - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 3 ) - 2 ) ) );
	static const u32 c_iClampMask = ( ( 1 << ( c_iBitsPerPixel ) ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) + ( 1 << ( c_iBitsPerPixel * 3 ) ) );
	static const u32 c_iLoBitMask = ( ( 1 ) + ( 1 << c_iBitsPerPixel ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) );
	static const u32 c_iPixelMask = 0x7fff;
	static const __m128i c_vLoBitMask = _mm_set1_epi16 ( c_iLoBitMask );
	static const __m128i c_vPixelMask = _mm_set1_epi16 ( c_iPixelMask );
	static const __m128i c_vClampMask = _mm_set1_epi16 ( c_iClampMask );
	static const __m128i c_vShiftHalf_Mask = _mm_set1_epi16 ( c_iShiftHalf_Mask );
	static const __m128i c_vShiftQuarter_Mask = _mm_set1_epi16 ( c_iShiftQuarter_Mask );
	__m128i vDestPixel, vPixelMask, vSetPixelMask, vStoreMask;
	__m128i /*vbgr, vbgr_temp,*/ vStartX, vEndX, vx_across, vSeq, vVectorSize;
	__m128i vpixel, vpixel_temp;
	
	vSeq = _mm_set_epi16 ( 7, 6, 5, 4, 3, 2, 1, 0 );
	vVectorSize = _mm_set1_epi16 ( c_iVectorSize8 );
	
	vpixel = _mm_set1_epi16 ( bgr );
	//PixelMask = _mm_setzero_si128 ();
	//SetPixelMask = _mm_setzero_si128 ();
	if ( PIXELMASK ) { vPixelMask = _mm_set1_epi16 ( 0x8080 ); }
	if ( SETPIXELMASK ) { vSetPixelMask = _mm_set1_epi16 ( 0x8000 ); }
#endif


	
	// initialize number of pixels drawn
	NumberOfPixelsDrawn = 0;
	
	// check for some important conditions
	if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
		return;
	}
	
	if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
		return;
	}
	

	
	// get color(s)
	bgr = gbgr [ 0 ];
	
	// ?? convert to 16-bit ?? (or should leave 24-bit?)
	bgr = ( ( bgr & ( 0xf8 << 0 ) ) >> 3 ) | ( ( bgr & ( 0xf8 << 8 ) ) >> 6 ) | ( ( bgr & ( 0xf8 << 16 ) ) >> 9 );
	
	// get y-values
	//y0 = Buffer [ Coord0 ].y;
	//y1 = Buffer [ Coord1 ].y;
	//y2 = Buffer [ Coord2 ].y;
	
	///////////////////////////////////
	// put top coordinates in x0,y0
	//if ( y1 < y0 )
	if ( gy [ Coord1 ] < gy [ Coord0 ] )
	{
		//Swap ( y0, y1 );
		Swap ( Coord0, Coord1 );
	}
	
	//if ( y2 < y0 )
	if ( gy [ Coord2 ] < gy [ Coord0 ] )
	{
		//Swap ( y0, y2 );
		Swap ( Coord0, Coord2 );
	}
	
	///////////////////////////////////////
	// put middle coordinates in x1,y1
	//if ( y2 < y1 )
	if ( gy [ Coord2 ] < gy [ Coord1 ] )
	{
		//Swap ( y1, y2 );
		Swap ( Coord1, Coord2 );
	}
	
	// get x-values
	x0 = gx [ Coord0 ];
	x1 = gx [ Coord1 ];
	x2 = gx [ Coord2 ];
	
	// get y-values
	y0 = gy [ Coord0 ];
	y1 = gy [ Coord1 ];
	y2 = gy [ Coord2 ];
	
	
	//////////////////////////////////////////
	// get coordinates on screen
	x0 = DrawArea_OffsetX + x0;
	y0 = DrawArea_OffsetY + y0;
	x1 = DrawArea_OffsetX + x1;
	y1 = DrawArea_OffsetY + y1;
	x2 = DrawArea_OffsetX + x2;
	y2 = DrawArea_OffsetY + y2;
	
	
	// get the pixel
	pixel = bgr;
	
#ifdef ENABLE_TEMPLATE_MULTIPIXEL
	if ( !PIXELMASK )
	{
		pixel |= ( pixel << 16 ) | ( pixel << 32 ) | ( pixel << 48 );
	}
#endif
	
	
	
	// get the left/right most x
	LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
	LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
	RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
	RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

	// check if sprite is within draw area
	if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	// skip drawing if distance between vertices is greater than max allowed by GPU
	if ( ( _Abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( _Abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
	{
		// skip drawing polygon
		return;
	}
	
	
	
	/////////////////////////////////////////////////
	// draw top part of triangle
	
	// denominator is negative when x1 is on the left, positive when x1 is on the right
	t0 = y1 - y2;
	t1 = y0 - y2;
	denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );

	
	// get reciprocals
	if ( y1 - y0 ) r10 = ( 1LL << 48 ) / ((s64)( y1 - y0 ));
	if ( y2 - y0 ) r20 = ( 1LL << 48 ) / ((s64)( y2 - y0 ));
	if ( y2 - y1 ) r21 = ( 1LL << 48 ) / ((s64)( y2 - y1 ));

	
	///////////////////////////////////////////
	// start at y0
	//Line = y0;
	
	
	//if ( denominator < 0 )
	//{
		// x1 is on the left and x0 is on the right //
		
		////////////////////////////////////
		// get slopes
		
		if ( y1 - y0 )
		{
			/////////////////////////////////////////////
			// init x on the left and right
			x_left = ( ((s64)x0) << 16 );
			x_right = x_left;
			
			if ( denominator < 0 )
			{
				//dx_left = (((s64)( x1 - x0 )) << 16 ) / ((s64)( y1 - y0 ));
				//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
				dx_left = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
				dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			}
			else
			{
				dx_right = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
				dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			}
		}
		else
		{
			if ( denominator < 0 )
			{
				// change x_left and x_right where y1 is on left
				x_left = ( ((s64)x1) << 16 );
				x_right = ( ((s64)x0) << 16 );
			
				//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
				//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
				dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
				dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			}
			else
			{
				x_right = ( ((s64)x1) << 16 );
				x_left = ( ((s64)x0) << 16 );
			
				dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
				dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			}
		}
	//}

	/////////////////////////////////////////////////
	// swap left and right if they are reversed
	//if ( ( ( x_right + dx_right ) < ( x_left + dx_left ) ) || ( x_right < x_left ) )
	/*
	if ( denominator > 0 )
	{
		// x1,y1 is on the right //
		
		Swap ( x_left, x_right );
		Swap ( dx_left, dx_right );
	}
	*/
	
	
	////////////////
	// *** TODO *** at this point area of full triangle can be calculated and the rest of the drawing can be put on another thread *** //
	
	
	
	StartY = y0;
	EndY = y1;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		
		if ( EndY < ((s32)DrawArea_TopLeftY) )
		{
			Temp = EndY - StartY;
			StartY = EndY;
		}
		else
		{
			Temp = DrawArea_TopLeftY - StartY;
			StartY = DrawArea_TopLeftY;
		}
		
		x_left += dx_left * Temp;
		x_right += dx_right * Temp;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY + 1;
	}

	
	if ( ABE )
	{
#ifdef _ENABLE_SSE2_TRIANGLE_MONO
		vpixel_temp = vpixel;
#else
		pixel_temp = pixel;
#endif
	}
	
	if ( SETPIXELMASK && !ABE )
	{
#ifdef _ENABLE_SSE2_TRIANGLE_MONO
		vpixel = _mm_or_si128 ( vpixel, vSetPixelMask );
#else
		// check if we should set mask bit when drawing
		pixel |= SETPIXELMASK;
#endif
	}
	
	
	if ( EndY > StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y1
	for ( Line = StartY; Line < EndY; Line++ )
	{
		//StartX = _Round( x_left ) >> 32;
		//EndX = _Round( x_right ) >> 32;
		
		// left point is included if points are equal
		StartX = ( (s64) ( x_left + 0xffffLL ) ) >> 16;
		EndX = ( x_right - 1 ) >> 16;
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			// get the difference between x_left and StartX
			//Temp = ( StartX << 16 ) - ( x_left >> 16 );
		
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				//Temp = DrawArea_TopLeftX - StartX;
				StartX = DrawArea_TopLeftX;
			}
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			NumberOfPixelsDrawn += EndX - StartX + 1;
			
#ifdef _ENABLE_SSE2_TRIANGLE_MONO_NONTEMPLATE
	vx_across = _mm_add_epi16 ( vSeq, _mm_set1_epi16 ( StartX ) );
	vEndX = _mm_set1_epi16 ( EndX + 1 );
#endif

			// draw horizontal line
			// x_left and x_right need to be rounded off
			
#ifdef _ENABLE_SSE2_TRIANGLE_MONO
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize8 )
			{
				if ( ABE || PIXELMASK )
				{
					vDestPixel = _mm_loadu_si128 ((__m128i const*) ptr);
				}
				
				if ( ABE )
				{
					//vbgr_temp = vbgr;
					//if ( command_abe ) vbgr_temp = vSemiTransparency16( DestPixel, vbgr_temp, GPU_CTRL_Read.ABR );
					vpixel = vSemiTransparency16_t<ABRCODE>( vDestPixel, vpixel_temp, c_vLoBitMask, c_vPixelMask, c_vClampMask, c_vShiftHalf_Mask, c_vShiftQuarter_Mask );
					
					if ( SETPIXELMASK )
					{
						vpixel = _mm_or_si128 ( vpixel, vSetPixelMask );
					}
				}
				
				if ( PIXELMASK )
				{
					vStoreMask = _mm_andnot_si128 ( _mm_and_si128 ( _mm_srai_epi16 ( vDestPixel, 15 ), vPixelMask ), _mm_cmplt_epi16 ( vx_across, vEndX ) );
				}
				else
				{
					vStoreMask = _mm_cmplt_epi16 ( vx_across, vEndX );
				}
				
				_mm_maskmoveu_si128 ( vpixel, vStoreMask, (char*) ptr );
				vx_across = _mm_add_epi16 ( vx_across, vVectorSize );
				
				ptr += c_iVectorSize8;
			}
#else
#ifdef ENABLE_TEMPLATE_MULTIPIXEL
		if ( /* !ABE && */ !PIXELMASK )
		{
			x_across = StartX;
			if ( ( x_across & 1 ) && ( x_across <= EndX ) )
			{
				if ( ABE )
				{
					DestPixel = *ptr16;
					pixel = SemiTransparency16_t<ABRCODE> ( DestPixel, pixel_temp );
					if ( SETPIXELMASK ) pixel |= SETPIXELMASK;
				}
				*ptr16++ = pixel;
				x_across++;
			}
			if ( ( x_across & 3 ) && ( x_across <= ( EndX - 1 ) ) )
			{
				if ( ABE )
				{
					DestPixel = *ptr32;
					pixel = SemiTransparency16_64t<ABRCODE> ( DestPixel, pixel_temp );
					if ( SETPIXELMASK ) pixel |= SETPIXELMASK;
				}
				*ptr32++ = pixel;
				x_across += 2;
			}
			while ( x_across <= ( EndX - 3 ) )
			{
				if ( ABE )
				{
					DestPixel = *ptr64;
					pixel = SemiTransparency16_64t<ABRCODE> ( DestPixel, pixel_temp );
					if ( SETPIXELMASK ) pixel |= SETPIXELMASK;
				}
				*ptr64++ = pixel;
				x_across += 4;
			}
			if ( x_across <= ( EndX - 1 ) )
			{
				if ( ABE )
				{
					DestPixel = *ptr32;
					pixel = SemiTransparency16_64t<ABRCODE> ( DestPixel, pixel_temp );
					if ( SETPIXELMASK ) pixel |= SETPIXELMASK;
				}
				*ptr32++ = pixel;
				x_across += 2;
			}
			if ( x_across <= EndX )
			{
				if ( ABE )
				{
					DestPixel = *ptr16;
					pixel = SemiTransparency16_t<ABRCODE> ( DestPixel, pixel_temp );
					if ( SETPIXELMASK ) pixel |= SETPIXELMASK;
				}
				*ptr16++ = pixel;
				x_across++;
			}
		}
		else
#endif
		{
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
				if ( ABE || PIXELMASK )
				{
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
				}
				
				// semi-transparency
				if ( ABE )
				{
					pixel = SemiTransparency16_t<ABRCODE> ( DestPixel, pixel_temp );
					
					if ( SETPIXELMASK )
					{
						// check if we should set mask bit when drawing
						pixel |= SETPIXELMASK;
					}
				}
				

				// draw pixel if we can draw to mask pixels or mask bit not set
				if ( PIXELMASK )
				{
					if ( ! ( DestPixel & PIXELMASK ) ) *ptr = pixel;
				}
				else
				{
					*ptr = pixel;
				}
				
				//ptr++;
				ptr += c_iVectorSize;
			}
		
		} // end if ( !ABE && !PIXELMASK )
#endif
			
		}
		
		//////////////////////////////////
		// draw next line
		//Line++;
		
		/////////////////////////////////////
		// update x on left and right
		x_left += dx_left;
		x_right += dx_right;
	}
	
	}

	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	/////////////////////////////////////////////
	// init x on the left and right
	
	//////////////////////////////////////////////////////
	// check if y1 is on the left or on the right
	//if ( Y1_OnLeft )
	if ( denominator < 0 )
	{
		// y1 is on the left //
		
		x_left = ( ((s64)x1) << 16 );
		
		//if ( y2 - y1 )
		//{
			//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
		//}
	}
	else
	{
		// y1 is on the right //
		
		x_right = ( ((s64)x1) << 16 );
		
		//if ( y2 - y1 )
		//{
			//dx_right = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
		//}
	}

	
	// the line starts at y1 from here
	//Line = y1;

	StartY = y1;
	EndY = y2;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		
		if ( EndY < ((s32)DrawArea_TopLeftY) )
		{
			Temp = EndY - StartY;
			StartY = EndY;
		}
		else
		{
			Temp = DrawArea_TopLeftY - StartY;
			StartY = DrawArea_TopLeftY;
		}
		
		x_left += dx_left * Temp;
		x_right += dx_right * Temp;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY + 1;
	}


	if ( EndY > StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y2
	//while ( Line < y2 )
	for ( Line = StartY; Line < EndY; Line++ )
	{
		//cy = Line;

		//StartX = _Round( x_left ) >> 32;
		//EndX = _Round( x_right ) >> 32;
		
		// left point is included if points are equal
		StartX = ( x_left + 0xffffLL ) >> 16;
		EndX = ( x_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				//Temp = DrawArea_TopLeftX - StartX;
				StartX = DrawArea_TopLeftX;
			}
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			NumberOfPixelsDrawn += EndX - StartX + 1;

			// draw horizontal line
			// x_left and x_right need to be rounded off
#ifdef _ENABLE_SSE2_TRIANGLE_MONO
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize8 )
			{
				if ( ABE || PIXELMASK )
				{
					vDestPixel = _mm_loadu_si128 ((__m128i const*) ptr);
				}
				
				if ( ABE )
				{
					//vbgr_temp = vbgr;
					//if ( command_abe ) vbgr_temp = vSemiTransparency16( DestPixel, vbgr_temp, GPU_CTRL_Read.ABR );
					vpixel = vSemiTransparency16_t<ABRCODE>( vDestPixel, vpixel_temp, c_vLoBitMask, c_vPixelMask, c_vClampMask, c_vShiftHalf_Mask, c_vShiftQuarter_Mask );
					
					if ( SETPIXELMASK )
					{
						vpixel = _mm_or_si128 ( vpixel, vSetPixelMask );
					}
				}
				
				if ( PIXELMASK )
				{
					vStoreMask = _mm_andnot_si128 ( _mm_and_si128 ( _mm_srai_epi16 ( vDestPixel, 15 ), vPixelMask ), _mm_cmplt_epi16 ( vx_across, vEndX ) );
				}
				else
				{
					vStoreMask = _mm_cmplt_epi16 ( vx_across, vEndX );
				}
				
				_mm_maskmoveu_si128 ( vpixel, vStoreMask, (char*) ptr );
				vx_across = _mm_add_epi16 ( vx_across, vVectorSize );
				
				ptr += c_iVectorSize8;
			}
#else
#ifdef ENABLE_TEMPLATE_MULTIPIXEL
		if ( /* !ABE && */ !PIXELMASK )
		{
			x_across = StartX;
			if ( ( x_across & 1 ) && ( x_across <= EndX ) )
			{
				if ( ABE )
				{
					DestPixel = *ptr16;
					pixel = SemiTransparency16_t<ABRCODE> ( DestPixel, pixel_temp );
					if ( SETPIXELMASK ) pixel |= SETPIXELMASK;
				}
				*ptr16++ = pixel;
				x_across++;
			}
			if ( ( x_across & 3 ) && ( x_across <= ( EndX - 1 ) ) )
			{
				if ( ABE )
				{
					DestPixel = *ptr32;
					pixel = SemiTransparency16_64t<ABRCODE> ( DestPixel, pixel_temp );
					if ( SETPIXELMASK ) pixel |= SETPIXELMASK;
				}
				*ptr32++ = pixel;
				x_across += 2;
			}
			while ( x_across <= ( EndX - 3 ) )
			{
				if ( ABE )
				{
					DestPixel = *ptr64;
					pixel = SemiTransparency16_64t<ABRCODE> ( DestPixel, pixel_temp );
					if ( SETPIXELMASK ) pixel |= SETPIXELMASK;
				}
				*ptr64++ = pixel;
				x_across += 4;
			}
			if ( x_across <= ( EndX - 1 ) )
			{
				if ( ABE )
				{
					DestPixel = *ptr32;
					pixel = SemiTransparency16_64t<ABRCODE> ( DestPixel, pixel_temp );
					if ( SETPIXELMASK ) pixel |= SETPIXELMASK;
				}
				*ptr32++ = pixel;
				x_across += 2;
			}
			if ( x_across <= EndX )
			{
				if ( ABE )
				{
					DestPixel = *ptr16;
					pixel = SemiTransparency16_t<ABRCODE> ( DestPixel, pixel_temp );
					if ( SETPIXELMASK ) pixel |= SETPIXELMASK;
				}
				*ptr16++ = pixel;
				x_across++;
			}
		}
		else
#endif
		{
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
				if ( ABE || PIXELMASK )
				{
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
				}
				
				// semi-transparency
				if ( ABE )
				{
					pixel = SemiTransparency16_t<ABRCODE> ( DestPixel, pixel_temp );
					
					if ( SETPIXELMASK )
					{
						// check if we should set mask bit when drawing
						pixel |= SETPIXELMASK;
					}
				}
				

				// draw pixel if we can draw to mask pixels or mask bit not set
				if ( PIXELMASK )
				{
					if ( ! ( DestPixel & PIXELMASK ) ) *ptr = pixel;
				}
				else
				{
					*ptr = pixel;
				}
				
				//ptr++;
				ptr += c_iVectorSize;
			}
			
		} // end if ( !ABE && !PIXELMASK )
#endif
		}
		
		/////////////////////////////////////
		// update x on left and right
		x_left += dx_left;
		x_right += dx_right;
	}
	
	}

}



template<const long PIXELMASK, const long ABE, const long ABRCODE>
inline void GPU::SelectTriangle_Mono3_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	if ( GPU_CTRL_Read.MD )
	{
		DrawTriangle_Mono_t <PIXELMASK,0x8000800080008000LL,ABE,ABRCODE> ( Coord0, Coord1, Coord2 );
	}
	else
	{
		DrawTriangle_Mono_t <PIXELMASK,0,ABE,ABRCODE> ( Coord0, Coord1, Coord2 );
	}
}


template<const long ABE, const long ABRCODE>
inline void GPU::SelectTriangle_Mono2_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	if ( GPU_CTRL_Read.ME )
	{
		SelectTriangle_Mono3_t <0x8000,ABE,ABRCODE> ( Coord0, Coord1, Coord2 );
	}
	else
	{
		SelectTriangle_Mono3_t <0,ABE,ABRCODE> ( Coord0, Coord1, Coord2 );
	}
}


template<const long ABE>
void GPU::SelectTriangle_Mono_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	switch ( GPU_CTRL_Read.ABR )
	{
		case 0:
			SelectTriangle_Mono2_t <ABE,0> ( Coord0, Coord1, Coord2 );
			break;
			
		case 1:
			SelectTriangle_Mono2_t <ABE,1> ( Coord0, Coord1, Coord2 );
			break;
			
		case 2:
			SelectTriangle_Mono2_t <ABE,2> ( Coord0, Coord1, Coord2 );
			break;
			
		case 3:
			SelectTriangle_Mono2_t <ABE,3> ( Coord0, Coord1, Coord2 );
			break;
	}
}


template<const long ABE>
void GPU::Draw_MonoTriangle_20_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	//DrawTriangle_Mono ();
	SelectTriangle_Mono_t <ABE> ( Coord0, Coord1, Coord2 );
	
	// *** TODO *** calculate cycles to draw here
	// check for alpha blending - add in an extra cycle for this for now
	if ( ABE )
	{
		BusyCycles += NumberOfPixelsDrawn * dAlphaBlending_CyclesPerPixel;
	}
	
	// add in cycles to draw mono-triangle
	BusyCycles += NumberOfPixelsDrawn * dMonoTriangle_20_CyclesPerPixel;
	
#ifdef ENABLE_DRAW_OVERHEAD
	BusyCycles += DrawOverhead_Cycles;
#endif
}

template<const long ABE>
void GPU::Draw_MonoRectangle_28_t ()
{
	//x_save [ 0 ] = x0; x_save [ 1 ] = x1; x_save [ 2 ] = x2; x_save [ 3 ] = x3;
	//y_save [ 0 ] = y0; y_save [ 1 ] = y1; y_save [ 2 ] = y2; y_save [ 3 ] = y3;
	//bgr_save [ 0 ] = bgr;
	
	//Draw_MonoTriangle_20 ();
	Draw_MonoTriangle_20_t <ABE> ( 0, 1, 2 );
	
	//x0 = x_save [ 1 ];
	//y0 = y_save [ 1 ];
	//x1 = x_save [ 2 ];
	//y1 = y_save [ 2 ];
	//x2 = x_save [ 3 ];
	//y2 = y_save [ 3 ];
	
	//bgr = bgr_save [ 0 ];
	
	//Draw_MonoTriangle_20 ();
	Draw_MonoTriangle_20_t <ABE> ( 1, 2, 3 );
}




template<const long PIXELMASK, const long SETPIXELMASK, const long ABE, const long ABRCODE, const long TGE, const long TP>
void GPU::DrawTriangle_Texture_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	//debug << "DrawTriangle_Texture_t->";
	
	// before sse2, was sending 1 pixels at a time
	static const int c_iVectorSize = 1;

	//s32 x0, y0, x1, y1;
	u32 pixel, pixel_temp;
	//s64 iU, iV;
	//s64 x_left, x_right, dx_left, dx_right, U_left, U_right, dU_left, dU_right, V_left, V_right, dV_left, dV_right;
	
	u32 clut_xoffset;

	//u32 Pixel, TexelIndex;
	//u32 Y1_OnLeft;
	
	u32 color_add;

	s64 r10, r20, r21;
	
	u16 *ptr_texture, *ptr_clut;
	u16 *ptr;
	
	s32 StartX, EndX, StartY, EndY;
	s64 Temp, TMin, TMax;
	s64 LeftMostX, RightMostX;

	u32 DestPixel;


	// new local variables
	s32 x0, x1, x2, y0, y1, y2;
	s64 dx_left, dx_right;
	s64 x_left, x_right, x_across;
	u32 bgr, bgr_temp;
	s32 Line;
	s64 t0, t1, denominator;

	// new local variables for texture mapping
	s64 dU_left, dV_left, dU_across, dV_across, U_left, V_left, iU, iV;
	s32 u0, v0, u1, v1, u2, v2;
	
	//u32 PixelMask = 0, SetPixelMask = 0;
	//u32 Shift1 = 0, Shift2 = 0, And1 = 0, And2 = 0;
	

	s64 Error_Left;
	
	//s64 TexOffset_X, TexOffset_Y;
	u32 TexCoordX, TexCoordY;
	
	u32 TWYTWH, TWXTWW, Not_TWH, Not_TWW;
	
	TWYTWH = ( ( TWY & TWH ) << 3 );
	TWXTWW = ( ( TWX & TWW ) << 3 );
	
	Not_TWH = ~( TWH << 3 );
	Not_TWW = ~( TWW << 3 );
	
	//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
	
	// initialize number of pixels drawn
	// initialize number of pixels drawn
	NumberOfPixelsDrawn = 0;
	
	clut_xoffset = clut_x << 4;
	ptr_clut = & ( VRAM [ clut_y << 10 ] );
	ptr_texture = & ( VRAM [ ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) ] );
	
	// check for some important conditions
	if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
		return;
	}
	
	if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
		return;
	}


	// get color(s)
	bgr = gbgr [ 0 ];
	
	// ?? convert to 16-bit ?? (or should leave 24-bit?)
	//bgr = ( ((u32)Buffer [ 0 ].Red) >> 3 ) | ( ( ((u32)Buffer [ 0 ].Green) >> 3 ) << 5 ) | ( ( ((u32)Buffer [ 0 ].Blue) >> 3 ) << 10 );
	
	// get y-values
	//y0 = Buffer [ Coord0 ].y;
	//y1 = Buffer [ Coord1 ].y;
	//y2 = Buffer [ Coord2 ].y;
	
	///////////////////////////////////
	// put top coordinates in x0,y0
	//if ( y1 < y0 )
	if ( gy [ Coord1 ] < gy [ Coord0 ] )
	{
		//Swap ( y0, y1 );
		Swap ( Coord0, Coord1 );
	}
	
	//if ( y2 < y0 )
	if ( gy [ Coord2 ] < gy [ Coord0 ] )
	{
		//Swap ( y0, y2 );
		Swap ( Coord0, Coord2 );
	}
	
	///////////////////////////////////////
	// put middle coordinates in x1,y1
	//if ( y2 < y1 )
	if ( gy [ Coord2 ] < gy [ Coord1 ] )
	{
		//Swap ( y1, y2 );
		Swap ( Coord1, Coord2 );
	}
	
	// get x-values
	x0 = gx [ Coord0 ];
	x1 = gx [ Coord1 ];
	x2 = gx [ Coord2 ];

	// get y-values
	y0 = gy [ Coord0 ];
	y1 = gy [ Coord1 ];
	y2 = gy [ Coord2 ];
	
	// get texture coords
	u0 = gu [ Coord0 ];
	u1 = gu [ Coord1 ];
	u2 = gu [ Coord2 ];
	v0 = gv [ Coord0 ];
	v1 = gv [ Coord1 ];
	v2 = gv [ Coord2 ];
	
	
	//////////////////////////////////////////
	// get coordinates on screen
	x0 = DrawArea_OffsetX + x0;
	y0 = DrawArea_OffsetY + y0;
	x1 = DrawArea_OffsetX + x1;
	y1 = DrawArea_OffsetY + y1;
	x2 = DrawArea_OffsetX + x2;
	y2 = DrawArea_OffsetY + y2;
	
	
	////////////////////////////////
	// y1 starts on the left
	//Y1_OnLeft = 1;
	
	color_add = bgr;

	//////////////////////////////////////////////////////
	// Get offset into color lookup table
	//u32 ClutOffset = ( clut_x << 4 ) + ( clut_y << 10 );
	
	/////////////////////////////////////////////////////////
	// Get offset into texture page
	//u32 TextureOffset = ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 );
	
	/*
	if ( tpage_tp == 0 )
	{
		And2 = 0xf;
		
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
		Shift1 = _mm_set_epi32 ( 0, 0, 0, 2 );
		Shift2 = _mm_set_epi32 ( 0, 0, 0, 2 );
		And1 = _mm_set1_epi16 ( 3 );
#else
		Shift1 = 2; Shift2 = 2;
		And1 = 3; And2 = 0xf;
#endif
	}
	else if ( tpage_tp == 1 )
	{
		And2 = 0xff;
		
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
		Shift1 = _mm_set_epi32 ( 0, 0, 0, 1 );
		Shift2 = _mm_set_epi32 ( 0, 0, 0, 3 );
		And1 = _mm_set1_epi16 ( 1 );
#else
		Shift1 = 1; Shift2 = 3;
		And1 = 1; And2 = 0xff;
#endif
	}
	*/
	
	
	// get the left/right most x
	LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
	LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
	RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
	RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

	// check if sprite is within draw area
	if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	// skip drawing if distance between vertices is greater than max allowed by GPU
	if ( ( _Abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( _Abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
	{
		// skip drawing polygon
		return;
	}
	

	// calculate across
	t0 = y1 - y2;
	t1 = y0 - y2;
	denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );
	if ( denominator )
	{
		denominator = ( 1LL << 48 ) / denominator;
		
		//dU_across = ( ( (s64) ( ( ( u0 - u2 ) * t0 ) - ( ( u1 - u2 ) * t1 ) ) ) << 24 ) / denominator;
		//dV_across = ( ( (s64) ( ( ( v0 - v2 ) * t0 ) - ( ( v1 - v2 ) * t1 ) ) ) << 24 ) / denominator;
		dU_across = ( ( (s64) ( ( ( u0 - u2 ) * t0 ) - ( ( u1 - u2 ) * t1 ) ) ) * denominator ) >> 24;
		dV_across = ( ( (s64) ( ( ( v0 - v2 ) * t0 ) - ( ( v1 - v2 ) * t1 ) ) ) * denominator ) >> 24;
		
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
		vdU_across = _mm_set1_epi32 ( dU_across * 8 );
		vdV_across = _mm_set1_epi32 ( dV_across * 8 );
#endif
	}

	
	// get reciprocals
	if ( y1 - y0 ) r10 = ( 1LL << 48 ) / ((s64)( y1 - y0 ));
	if ( y2 - y0 ) r20 = ( 1LL << 48 ) / ((s64)( y2 - y0 ));
	if ( y2 - y1 ) r21 = ( 1LL << 48 ) / ((s64)( y2 - y1 ));
	
	/////////////////////////////////////////////////
	// draw top part of triangle
	

	///////////////////////////////////////////
	// start at y0
	//Line = y0;
	
	if ( y1 - y0 )
	{
		x_left = ( ((s64)x0) << 16 );
		x_right = x_left;
		
		U_left = ( ((s64)u0) << 24 );
		V_left = ( ((s64)v0) << 24 );
		//U_right = U_left;
		//V_right = V_left;
		
		if ( denominator < 0 )
		{
			//dx_left = (((s64)( x1 - x0 )) << 16 ) / ((s64)( y1 - y0 ));
			//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			dx_left = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
			dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			//dU_left = (((s64)( u1 - u0 )) << 24 ) / ((s64)( y1 - y0 ));
			//dV_left = (((s64)( v1 - v0 )) << 24 ) / ((s64)( y1 - y0 ));
			dU_left = ( ((s64)( u1 - u0 )) * r10 ) >> 24;
			dV_left = ( ((s64)( v1 - v0 )) * r10 ) >> 24;
		}
		else
		{
			dx_right = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
			dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			//dU_right = (((s64)( u2 - u0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dV_right = (((s64)( v2 - v0 )) << 24 ) / ((s64)( y2 - y0 ));
			dU_left = ( ((s64)( u2 - u0 )) * r20 ) >> 24;
			dV_left = ( ((s64)( v2 - v0 )) * r20 ) >> 24;
		}
	}
	else
	{
		if ( denominator < 0 )
		{
			// change x_left and x_right where y1 is on left
			x_left = ( ((s64)x1) << 16 );
			x_right = ( ((s64)x0) << 16 );
			
			//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			// change U_left and V_right where y1 is on left
			U_left = ( ((s64)u1) << 24 );
			V_left = ( ((s64)v1) << 24 );
		
			//dU_left = (((s64)( u2 - u1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dV_left = (((s64)( v2 - v1 )) << 24 ) / ((s64)( y2 - y1 ));
			dU_left = ( ((s64)( u2 - u1 )) * r21 ) >> 24;
			dV_left = ( ((s64)( v2 - v1 )) * r21 ) >> 24;
		}
		else
		{
			x_right = ( ((s64)x1) << 16 );
			x_left = ( ((s64)x0) << 16 );
			
			dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			U_left = ( ((s64)u0) << 24 );
			V_left = ( ((s64)v0) << 24 );
		
			//dU_right = (((s64)( u2 - u0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dV_right = (((s64)( v2 - v0 )) << 24 ) / ((s64)( y2 - y0 ));
			dU_left = ( ((s64)( u2 - u0 )) * r20 ) >> 24;
			dV_left = ( ((s64)( v2 - v0 )) * r20 ) >> 24;
		}
	}

	

	/////////////////////////////////////////////////
	// swap left and right if they are reversed
	//if ( ( ( x_right + dx_right ) < ( x_left + dx_left ) ) || ( x_right < x_left ) )
	/*
	if ( denominator > 0 )
	{
		// x1,y1 is on the right //
		
		Y1_OnLeft = 0;
		
		Swap ( x_left, x_right );
		Swap ( dx_left, dx_right );

		Swap ( dU_left, dU_right );
		Swap ( dV_left, dV_right );

		Swap ( U_left, U_right );
		Swap ( V_left, V_right );
	}
	*/
	
	////////////////
	// *** TODO *** at this point area of full triangle can be calculated and the rest of the drawing can be put on another thread *** //
	

	// u,v values are not specified with a fractional part, so there must be an initial fractional part
	U_left |= ( 1 << 23 );
	V_left |= ( 1 << 23 );
	
	
	StartY = y0;
	EndY = y1;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		
		if ( EndY < ((s32)DrawArea_TopLeftY) )
		{
			Temp = EndY - StartY;
			StartY = EndY;
		}
		else
		{
			Temp = DrawArea_TopLeftY - StartY;
			StartY = DrawArea_TopLeftY;
		}
		
		x_left += dx_left * Temp;
		x_right += dx_right * Temp;
		
		U_left += dU_left * Temp;
		V_left += dV_left * Temp;
		
		//U_right += dU_right * Temp;
		//V_right += dV_right * Temp;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY + 1;
	}

#ifdef _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
	vSeq32_u1 = _custom_mul_32 ( _mm_set1_epi32 ( dU_across ), vSeq32_1 );
	vSeq32_u2 = _custom_mul_32 ( _mm_set1_epi32 ( dU_across ), vSeq32_2 );
	vSeq32_v1 = _custom_mul_32 ( _mm_set1_epi32 ( dV_across ), vSeq32_1 );
	vSeq32_v2 = _custom_mul_32 ( _mm_set1_epi32 ( dV_across ), vSeq32_2 );
#endif

	if ( EndY > StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y1
	//while ( Line < y1 )
	for ( Line = StartY; Line < EndY; Line++ )
	{
		//StartX = _Round( x_left ) >> 32;
		//EndX = _Round( x_right ) >> 32;
		
		// left point is included if points are equal
		StartX = ( x_left + 0xffffLL ) >> 16;
		EndX = ( x_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			iU = U_left;
			iV = V_left;
			
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - x_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iU += dU_across * Temp;
				//iV += dV_across * Temp;
			}
			
			iU += ( dU_across >> 12 ) * ( Temp >> 4 );
			iV += ( dV_across >> 12 ) * ( Temp >> 4 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			NumberOfPixelsDrawn += EndX - StartX + 1;

			// draw horizontal line
			// x_left and x_right need to be rounded off
			for ( x_across = StartX; x_across <= EndX; x_across++ )
			{
					//TexCoordY = (u8) ( ( ( _Round24 ( iV ) >> 24 ) & Not_TWH ) | TWYTWH );
					//TexCoordX = (u8) ( ( ( _Round24 ( iU ) >> 24 ) & Not_TWW ) | TWXTWW );
					TexCoordY = (u8) ( ( ( iV >> 24 ) & Not_TWH ) | TWYTWH );
					TexCoordX = (u8) ( ( ( iU >> 24 ) & Not_TWW ) | TWXTWW );
					
					// *** testing ***
					//debug << dec << "\r\nTexCoordX=" << TexCoordX << " TexCoordY=" << TexCoordY;
					
					switch ( TP )
					{
						case 0:
							pixel = ptr_texture [ ( TexCoordX >> 2 ) + ( TexCoordY << 10 ) ];
							pixel = ptr_clut [ ( clut_xoffset + ( ( pixel >> ( ( TexCoordX & 3 ) << 2 ) ) & 0xf ) ) & FrameBuffer_XMask ];
							break;
							
						case 1:
							pixel = ptr_texture [ ( TexCoordX >> 1 ) + ( TexCoordY << 10 ) ];
							pixel = ptr_clut [ ( clut_xoffset + ( ( pixel >> ( ( TexCoordX & 1 ) << 3 ) ) & 0xff ) ) & FrameBuffer_XMask ];
							break;
							
						case 2:
							pixel = ptr_texture [ ( TexCoordX ) + ( TexCoordY << 10 ) ];
							break;
					}
					
					
					if ( pixel )
					{
						if ( ABE || PIXELMASK )
						{
							// read pixel from frame buffer if we need to check mask bit
							DestPixel = *ptr;
						}

						if ( ABE || !TGE )
						{
							pixel_temp = pixel & 0x8000;
						}
				
						//if ( !tge )
						if ( !TGE )
						{
							// brightness calculation
							pixel = ColorMultiply1624 ( pixel, color_add );
						}
						
						// semi-transparency
						//if ( command_abe && ( bgr & 0x8000 ) )
						if ( ABE )
						{
							if ( pixel_temp )
							{
								pixel = SemiTransparency16_t <ABRCODE> ( DestPixel, pixel );
							}
						}
						
						if ( ABE || !TGE )
						{
							// check if we should set mask bit when drawing
							//bgr_temp |= SetPixelMask | ( bgr & 0x8000 );
							pixel |= pixel_temp;
						}
						
						if ( SETPIXELMASK )
						{
							// set pixel mask if specified
							pixel |= SETPIXELMASK;
						}

						// draw pixel if we can draw to mask pixels or mask bit not set
						if ( PIXELMASK )
						{
							if ( ! ( DestPixel & PIXELMASK ) ) *ptr = pixel;
						}
						else
						{
							*ptr = pixel;
						}
					}
					
					/////////////////////////////////////////////////////
					// update number of cycles used to draw polygon
					//NumberOfPixelsDrawn++;
				//}
				
				/////////////////////////////////////////////////////////
				// interpolate texture coords
				iU += dU_across;
				iV += dV_across;
				
				//ptr++;
				ptr += c_iVectorSize;
			}
			
		}
		
		/////////////////////////////////////
		// update x on left and right
		x_left += dx_left;
		x_right += dx_right;
		
		U_left += dU_left;
		V_left += dV_left;
		//U_right += dU_right;
		//V_right += dV_right;
	}
	
	}

	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	/////////////////////////////////////////////
	// init x on the left and right
	
	//////////////////////////////////////////////////////
	// check if y1 is on the left or on the right
	//if ( Y1_OnLeft )
	if ( denominator < 0 )
	{
		x_left = ( ((s64)x1) << 16 );

		U_left = ( ((s64)u1) << 24 );
		V_left = ( ((s64)v1) << 24 );

		// u,v values are not specified with a fractional part, so there must be an initial fractional part
		U_left |= ( 1 << 23 );
		V_left |= ( 1 << 23 );
		
		//if ( y2 - y1 )
		//{
			//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			
			//dU_left = (((s64)( u2 - u1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dV_left = (((s64)( v2 - v1 )) << 24 ) / ((s64)( y2 - y1 ));
			dU_left = ( ((s64)( u2 - u1 )) * r21 ) >> 24;
			dV_left = ( ((s64)( v2 - v1 )) * r21 ) >> 24;
		//}
	}
	else
	{
		x_right = ( ((s64)x1) << 16 );

		//U_right = ( ((s64)u1) << 24 );
		//V_right = ( ((s64)v1) << 24 );

		//if ( y2 - y1 )
		//{
			//dx_right = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			
			//dU_right = (((s64)( u2 - u1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dV_right = (((s64)( v2 - v1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dU_right = ( ((s64)( u2 - u1 )) * r21 ) >> 24;
			//dV_right = ( ((s64)( v2 - v1 )) * r21 ) >> 24;
		//}
	}
	
	// the line starts at y1 from here
	//Line = y1;

	StartY = y1;
	EndY = y2;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		
		if ( EndY < ((s32)DrawArea_TopLeftY) )
		{
			Temp = EndY - StartY;
			StartY = EndY;
		}
		else
		{
			Temp = DrawArea_TopLeftY - StartY;
			StartY = DrawArea_TopLeftY;
		}
		
		x_left += dx_left * Temp;
		x_right += dx_right * Temp;
		
		U_left += dU_left * Temp;
		V_left += dV_left * Temp;
		
		//U_right += dU_right * Temp;
		//V_right += dV_right * Temp;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY + 1;
	}


	if ( EndY > StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y2
	//while ( Line < y2 )
	for ( Line = StartY; Line < EndY; Line++ )
	{
		//StartX = _Round( x_left ) >> 32;
		//EndX = _Round( x_right ) >> 32;
		
		// left point is included if points are equal
		StartX = ( (s64) ( x_left + 0xffffLL ) ) >> 16;
		EndX = ( x_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			iU = U_left;
			iV = V_left;
			
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - x_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iU += dU_across * Temp;
				//iV += dV_across * Temp;
			}
			
			iU += ( dU_across >> 12 ) * ( Temp >> 4 );
			iV += ( dV_across >> 12 ) * ( Temp >> 4 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			NumberOfPixelsDrawn += EndX - StartX + 1;
			
			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across < EndX; x_across++ )
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
					//TexCoordY = (u8) ( ( ( _Round24 ( iV ) >> 24 ) & Not_TWH ) | TWYTWH );
					//TexCoordX = (u8) ( ( ( _Round24 ( iU ) >> 24 ) & Not_TWW ) | TWXTWW );
					TexCoordY = (u8) ( ( ( iV >> 24 ) & Not_TWH ) | TWYTWH );
					TexCoordX = (u8) ( ( ( iU >> 24 ) & Not_TWW ) | TWXTWW );
					
					// *** testing ***
					//debug << dec << "\r\nTexCoordX=" << TexCoordX << " TexCoordY=" << TexCoordY;
					
					switch ( TP )
					{
						case 0:
							pixel = ptr_texture [ ( TexCoordX >> 2 ) + ( TexCoordY << 10 ) ];
							pixel = ptr_clut [ ( clut_xoffset + ( ( pixel >> ( ( TexCoordX & 3 ) << 2 ) ) & 0xf ) ) & FrameBuffer_XMask ];
							break;
							
						case 1:
							pixel = ptr_texture [ ( TexCoordX >> 1 ) + ( TexCoordY << 10 ) ];
							pixel = ptr_clut [ ( clut_xoffset + ( ( pixel >> ( ( TexCoordX & 1 ) << 3 ) ) & 0xff ) ) & FrameBuffer_XMask ];
							break;
							
						case 2:
							pixel = ptr_texture [ ( TexCoordX ) + ( TexCoordY << 10 ) ];
							break;
					}
					
					
					if ( pixel )
					{
						if ( ABE || PIXELMASK )
						{
							// read pixel from frame buffer if we need to check mask bit
							DestPixel = *ptr;
						}

						if ( ABE || !TGE )
						{
							pixel_temp = pixel & 0x8000;
						}
				
						//if ( !tge )
						if ( !TGE )
						{
							// brightness calculation
							pixel = ColorMultiply1624 ( pixel, color_add );
						}
						
						// semi-transparency
						//if ( command_abe && ( bgr & 0x8000 ) )
						if ( ABE )
						{
							if ( pixel_temp )
							{
								pixel = SemiTransparency16_t <ABRCODE> ( DestPixel, pixel );
							}
						}
						
						if ( ABE || !TGE )
						{
							// check if we should set mask bit when drawing
							//bgr_temp |= SetPixelMask | ( bgr & 0x8000 );
							pixel |= pixel_temp;
						}
						
						if ( SETPIXELMASK )
						{
							// set pixel mask if specified
							pixel |= SETPIXELMASK;
						}

						// draw pixel if we can draw to mask pixels or mask bit not set
						if ( PIXELMASK )
						{
							if ( ! ( DestPixel & PIXELMASK ) ) *ptr = pixel;
						}
						else
						{
							*ptr = pixel;
						}
					}
					
				/////////////////////////////////////////////////////////
				// interpolate texture coords
				iU += dU_across;
				iV += dV_across;
				
				//ptr++;
				ptr += c_iVectorSize;
			}
			
		}
		
		/////////////////////////////////////
		// update x on left and right
		x_left += dx_left;
		x_right += dx_right;
		
		U_left += dU_left;
		V_left += dV_left;
		//U_right += dU_right;
		//V_right += dV_right;
	}
	
	}

}



template<const long PIXELMASK, const long SETPIXELMASK, const long ABE, const long ABRCODE, const long TGE, const long TP>
inline void GPU::SelectTriangle_Texture5_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	//debug << "SelectTriangle_Texture5_t->";
	
	//if ( ( bgr & 0xffffff ) == 0x808080 )
	if ( ( Buffer [ 0 ].Value & 0x00ffffff ) == 0x00808080 )
	{
		DrawTriangle_Texture_t <PIXELMASK, SETPIXELMASK, ABE, ABRCODE, 1, TP> ( Coord0, Coord1, Coord2 );
	}
	else
	{
		DrawTriangle_Texture_t <PIXELMASK, SETPIXELMASK, ABE, ABRCODE, TGE, TP> ( Coord0, Coord1, Coord2 );
	}
}



template<const long PIXELMASK, const long ABE, const long ABRCODE, const long TGE, const long TP>
inline void GPU::SelectTriangle_Texture4_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	//debug << "SelectTriangle_Texture4_t->";
	
	if ( GPU_CTRL_Read.MD )
	{
		SelectTriangle_Texture5_t <PIXELMASK, 0x8000, ABE, ABRCODE, TGE, TP> ( Coord0, Coord1, Coord2 );
	}
	else
	{
		SelectTriangle_Texture5_t <PIXELMASK, 0, ABE, ABRCODE, TGE, TP> ( Coord0, Coord1, Coord2 );
	}
}


template<const long ABE, const long ABRCODE, const long TGE, const long TP>
inline void GPU::SelectTriangle_Texture3_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	//debug << "SelectTriangle_Texture3_t->";
	
	if ( GPU_CTRL_Read.ME )
	{
		SelectTriangle_Texture4_t<0x8000, ABE, ABRCODE, TGE, TP> ( Coord0, Coord1, Coord2 );
	}
	else
	{
		SelectTriangle_Texture4_t<0, ABE, ABRCODE, TGE, TP> ( Coord0, Coord1, Coord2 );
	}
}


template<const long ABE, const long ABRCODE, const long TGE>
inline void GPU::SelectTriangle_Texture2_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	//debug << "SelectTriangle_Texture2_t->";
	
	switch ( tpage_tp )
	{
		case 0:
			SelectTriangle_Texture3_t<ABE, ABRCODE, TGE, 0> ( Coord0, Coord1, Coord2 );
			break;
			
		case 1:
			SelectTriangle_Texture3_t<ABE, ABRCODE, TGE, 1> ( Coord0, Coord1, Coord2 );
			break;
			
		case 2:
			SelectTriangle_Texture3_t<ABE, ABRCODE, TGE, 2> ( Coord0, Coord1, Coord2 );
			break;
	}
}


template<const long ABE, const long TGE>
void GPU::SelectTriangle_Texture_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	//debug << "SelectTriangle_Texture_t->";
	
	switch ( tpage_abr )
	{
		case 0:
			SelectTriangle_Texture2_t<ABE, 0, TGE> ( Coord0, Coord1, Coord2 );
			break;
			
		case 1:
			SelectTriangle_Texture2_t<ABE, 1, TGE> ( Coord0, Coord1, Coord2 );
			break;
			
		case 2:
			SelectTriangle_Texture2_t<ABE, 2, TGE> ( Coord0, Coord1, Coord2 );
			break;
			
		case 3:
			SelectTriangle_Texture2_t<ABE, 3, TGE> ( Coord0, Coord1, Coord2 );
			break;
	}
}



template<const long ABE, const long TGE>
void GPU::Draw_TextureTriangle_24_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	//debug << "Draw_TextureTriangle_24_t->";
	//u32 tge;
	//tge = command_tge;
	
	//if ( ( bgr & 0x00ffffff ) == 0x00808080 )
	//{
	//	command_tge = 1;
	//}
	
	//DrawTriangle_Texture ();
	SelectTriangle_Texture_t <ABE,TGE> ( Coord0, Coord1, Coord2 );
	
	// restore tge
	//command_tge = tge;
	
	// check for alpha blending - add in an extra cycle for this for now
	if ( ABE )
	{
		BusyCycles += NumberOfPixelsDrawn * dAlphaBlending_CyclesPerPixel;
	}
	
	// check for brightness calculation
	if ( !TGE )
	{
		BusyCycles += NumberOfPixelsDrawn * dBrightnessCalculation_CyclesPerPixel;
	}
	
	switch ( tpage_tp )
	{
		case 0:		// 4-bit clut
			BusyCycles += NumberOfPixelsDrawn * dTextureTriangle4_24_CyclesPerPixel;	//dTextureTriangle4_CyclesPerPixel;
			break;
			
		case 1:		// 8-bit clut
			BusyCycles += NumberOfPixelsDrawn * dTextureTriangle8_24_CyclesPerPixel;	//dTextureTriangle8_CyclesPerPixel;
			break;
			
		case 2:		// 15-bit color
			BusyCycles += NumberOfPixelsDrawn * dTextureTriangle16_24_CyclesPerPixel;	//dTextureTriangle16_CyclesPerPixel;
			break;
	}
	
#ifdef ENABLE_DRAW_OVERHEAD
	BusyCycles += DrawOverhead_Cycles;
#endif
}

template<const long ABE, const long TGE>
void GPU::Draw_TextureRectangle_2c_t ()
{
	//debug << "\r\nDraw_TextureRectangle_2c_t->";

	//x_save [ 0 ] = x0; x_save [ 1 ] = x1; x_save [ 2 ] = x2; x_save [ 3 ] = x3;
	//y_save [ 0 ] = y0; y_save [ 1 ] = y1; y_save [ 2 ] = y2; y_save [ 3 ] = y3;
	//bgr_save [ 0 ] = bgr; bgr_save [ 1 ] = bgr0; bgr_save [ 2 ] = bgr1; bgr_save [ 3 ] = bgr2; bgr_save [ 4 ] = bgr3;
	//u_save [ 0 ] = u0; u_save [ 1 ] = u1; u_save [ 2 ] = u2; u_save [ 3 ] = u3;
	//v_save [ 0 ] = v0; v_save [ 1 ] = v1; v_save [ 2 ] = v2; v_save [ 3 ] = v3;
	
	//Draw_TextureTriangle_24 ();
	Draw_TextureTriangle_24_t <ABE,TGE> ( 0, 1, 2 );

	//x0 = x_save [ 1 ];
	//y0 = y_save [ 1 ];
	//x1 = x_save [ 2 ];
	//y1 = y_save [ 2 ];
	//x2 = x_save [ 3 ];
	//y2 = y_save [ 3 ];


	//bgr = bgr_save [ 0 ];
	
	//u0 = u_save [ 1 ];
	//v0 = v_save [ 1 ];
	//u1 = u_save [ 2 ];
	//v1 = v_save [ 2 ];
	//u2 = u_save [ 3 ];
	//v2 = v_save [ 3 ];
	
	//Draw_TextureTriangle_24 ();
	Draw_TextureTriangle_24_t <ABE,TGE> ( 1, 2, 3 );
}




template<const long PIXELMASK, const long SETPIXELMASK, const long ABE, const long ABRCODE, const long DTD>
void GPU::DrawTriangle_Gradient_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	// before sse2, was sending 1 pixels at a time
	static const int c_iVectorSize = 1;
	static const int c_iVectorSize4 = 4;
	static const int c_iVectorSize8 = 8;

	u32 pixel;
	
	//u32 Pixel, TexelIndex,
	
	//u32 Y1_OnLeft;
	
	//u32 color_add;

	s64 r10, r20, r21;
	
	u16 *ptr;
	
	s64 Temp;
	s64 LeftMostX, RightMostX;
	
	s32 StartX, EndX, StartY, EndY;

	s64 Error_Left;
	
	s64* DitherArray;
	s64* DitherLine;
	s64 DitherValue;
	
	
	s64 Red, Green, Blue;
	
	u32 DestPixel;

	// new local variables
	s32 x0, x1, x2, y0, y1, y2;
	s64 dx_left, dx_right;
	s64 x_left, x_right, x_across;
	u32 bgr, bgr_temp;
	s32 Line;
	s64 t0, t1, denominator;

	// more local variables for gradient triangle
	s64 dR_left, dG_left, dB_left, dR_across, dG_across, dB_across, iR, iG, iB, R_left, G_left, B_left;
	s32 r0, r1, r2, g0, g1, g2, b0, b1, b2;


	
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT
	static const u32 c_iBitsPerPixel = 5;
	static const u32 c_iShiftHalf_Mask = ~( ( 1 << ( c_iBitsPerPixel - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 2 ) - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 3 ) - 1 ) ) );
	static const u32 c_iShiftQuarter_Mask = ~( ( 3 << ( c_iBitsPerPixel - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 2 ) - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 3 ) - 2 ) ) );
	static const u32 c_iClampMask = ( ( 1 << ( c_iBitsPerPixel ) ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) + ( 1 << ( c_iBitsPerPixel * 3 ) ) );
	static const u32 c_iLoBitMask = ( ( 1 ) + ( 1 << c_iBitsPerPixel ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) );
	static const u32 c_iPixelMask = 0x7fff;
	static const __m128i c_vLoBitMask = _mm_set1_epi16 ( c_iLoBitMask );
	static const __m128i c_vPixelMask = _mm_set1_epi16 ( c_iPixelMask );
	static const __m128i c_vClampMask = _mm_set1_epi16 ( c_iClampMask );
	static const __m128i c_vShiftHalf_Mask = _mm_set1_epi16 ( c_iShiftHalf_Mask );
	static const __m128i c_vShiftQuarter_Mask = _mm_set1_epi16 ( c_iShiftQuarter_Mask );
	__m128i vDestPixel, vPixelMask, vSetPixelMask, vStoreMask;
	__m128i vStartX, vEndX, vx_across, vVectorSize;
	__m128i vpixel, vpixel_temp;
	
	static const __m128i vSeq = _mm_set_epi16 ( 7, 6, 5, 4, 3, 2, 1, 0 );
	vVectorSize = _mm_set1_epi16 ( c_iVectorSize8 );
	
	//vpixel = _mm_set1_epi16 ( bgr );
	//PixelMask = _mm_setzero_si128 ();
	//SetPixelMask = _mm_setzero_si128 ();
	if ( PIXELMASK ) { vPixelMask = _mm_set1_epi16 ( 0x8080 ); }
	if ( SETPIXELMASK ) { vSetPixelMask = _mm_set1_epi16 ( 0x8000 ); }
	
	static const __m128i vSeq32_1 = _mm_set_epi32 ( 3, 2, 1, 0 );
	static const __m128i vSeq32_2 = _mm_set_epi32 ( 7, 6, 5, 4 );
	
	// variables needed for gradient //
	
	s16 *vDitherArray_add, *vDitherArray_sub, *vDitherLine_add, *vDitherLine_sub;
	__m128i vR_Start, vG_Start, vB_Start, viR, viG, viB, vDitherValue_add, vDitherValue_sub, vdR_across4, vdG_across4, vdB_across4, vRed, vGreen, vBlue;
#endif
	
	// initialize number of pixels drawn
	NumberOfPixelsDrawn = 0;
	
	// check for some important conditions
	if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
		return;
	}
	
	if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
		return;
	}
	
	///////////////////////////////////////////////////
	// Initialize dithering
	
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
	vDitherArray_add = (s16*) c_iDitherZero;
	vDitherArray_sub = (s16*) c_iDitherZero;
#else
	DitherArray = c_iDitherZero;
#endif
	
	///////////////////////////////////////////////////
	// Initialize dithering
	
	//DitherArray = (s32*) c_iDitherZero;
	
	if ( DTD )
	{
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
		vDitherArray_add = c_viDitherValues16_add;
		vDitherArray_sub = c_viDitherValues16_sub;
#else
		//DitherArray = c_iDitherValues;
		DitherArray = c_iDitherValues24;
#endif
	}



	////////////////////////////////
	// y1 starts on the left
	//Y1_OnLeft = 1;

	// get color(s)
	//bgr = Buffer [ 0 ].Value & 0xffffff;
	
	// ?? convert to 16-bit ?? (or should leave 24-bit?)
	//bgr = ( ((u32)Buffer [ 0 ].Red) >> 3 ) | ( ( ((u32)Buffer [ 0 ].Green) >> 3 ) << 5 ) | ( ( ((u32)Buffer [ 0 ].Blue) >> 3 ) << 10 )
	
	// get y-values
	//y0 = Buffer [ Coord0 ].y;
	//y1 = Buffer [ Coord1 ].y;
	//y2 = Buffer [ Coord2 ].y;
	
	///////////////////////////////////
	// put top coordinates in x0,y0
	//if ( y1 < y0 )
	if ( gy [ Coord1 ] < gy [ Coord0 ] )
	{
		//Swap ( y0, y1 );
		Swap ( Coord0, Coord1 );
	}
	
	//if ( y2 < y0 )
	if ( gy [ Coord2 ] < gy [ Coord0 ] )
	{
		//Swap ( y0, y2 );
		Swap ( Coord0, Coord2 );
	}
	
	///////////////////////////////////////
	// put middle coordinates in x1,y1
	//if ( y2 < y1 )
	if ( gy [ Coord2 ] < gy [ Coord1 ] )
	{
		//Swap ( y1, y2 );
		Swap ( Coord1, Coord2 );
	}
	
	// get x-values
	x0 = gx [ Coord0 ];
	x1 = gx [ Coord1 ];
	x2 = gx [ Coord2 ];
	
	// get y-values
	y0 = gy [ Coord0 ];
	y1 = gy [ Coord1 ];
	y2 = gy [ Coord2 ];

	// get rgb-values
	r0 = gr [ Coord0 ];
	r1 = gr [ Coord1 ];
	r2 = gr [ Coord2 ];
	g0 = gg [ Coord0 ];
	g1 = gg [ Coord1 ];
	g2 = gg [ Coord2 ];
	b0 = gb [ Coord0 ];
	b1 = gb [ Coord1 ];
	b2 = gb [ Coord2 ];


	
	//////////////////////////////////////////
	// get coordinates on screen
	x0 = DrawArea_OffsetX + x0;
	y0 = DrawArea_OffsetY + y0;
	x1 = DrawArea_OffsetX + x1;
	y1 = DrawArea_OffsetY + y1;
	x2 = DrawArea_OffsetX + x2;
	y2 = DrawArea_OffsetY + y2;
	
	
	
	
	
	// get the left/right most x
	LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
	LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
	RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
	RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

	// check if sprite is within draw area
	if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	// skip drawing if distance between vertices is greater than max allowed by GPU
	if ( ( _Abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( _Abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
	{
		// skip drawing polygon
		return;
	}
	
	
	////////////////////////////////////////////////
	// get length of longest scanline
	
	// calculate across
	// denominator is negative when x1 is on the left, positive when x1 is on the right
	t0 = y1 - y2;
	t1 = y0 - y2;
	denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );
	if ( denominator )
	{
		//dR_across = ( ( (s64) ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) ) << 24 ) / denominator;
		//dG_across = ( ( (s64) ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) ) << 24 ) / denominator;
		//dB_across = ( ( (s64) ( ( ( b0 - b2 ) * t0 ) - ( ( b1 - b2 ) * t1 ) ) ) << 24 ) / denominator;
		
		denominator = ( 1ll << 48 ) / denominator;
		dR_across = ( ( (s64) ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) ) * denominator ) >> 24;
		dG_across = ( ( (s64) ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) ) * denominator ) >> 24;
		dB_across = ( ( (s64) ( ( ( b0 - b2 ) * t0 ) - ( ( b1 - b2 ) * t1 ) ) ) * denominator ) >> 24;
		
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
		vdR_across = _mm_set1_epi32 ( dR_across * 8 );
		vdG_across = _mm_set1_epi32 ( dG_across * 8 );
		vdB_across = _mm_set1_epi32 ( dB_across * 8 );
#endif
	}
	
	//debug << dec << "\r\nx0=" << x0 << " y0=" << y0 << " x1=" << x1 << " y1=" << y1 << " x2=" << x2 << " y2=" << y2;
	//debug << "\r\nfixed: denominator=" << denominator;
	
	/////////////////////////////////////////////////
	// draw top part of triangle
	

	///////////////////////////////////////////
	// start at y0
	//Line = y0;
	
	
	
	// get reciprocals
	if ( y1 - y0 ) r10 = ( 1LL << 48 ) / ((s64)( y1 - y0 ));
	if ( y2 - y0 ) r20 = ( 1LL << 48 ) / ((s64)( y2 - y0 ));
	if ( y2 - y1 ) r21 = ( 1LL << 48 ) / ((s64)( y2 - y1 ));

	////////////////////////////////////
	// get slopes
	
	if ( y1 - y0 )
	{
		x_left = ( ((s64)x0) << 16 );
		x_right = x_left;
		
		//R_left = ( ((s64)r0) << 32 );
		//G_left = ( ((s64)g0) << 32 );
		//B_left = ( ((s64)b0) << 32 );
		R_left = ( ((s64)r0) << 24 );
		G_left = ( ((s64)g0) << 24 );
		B_left = ( ((s64)b0) << 24 );
		//R_right = R_left;
		//G_right = G_left;
		//B_right = B_left;
		
		if ( denominator < 0 )
		{
			//dx_left = (((s64)( x1 - x0 )) << 16 ) / ((s64)( y1 - y0 ));
			//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			dx_left = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
			dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			//dR_left = (((s64)( r1 - r0 )) << 24 ) / ((s64)( y1 - y0 ));
			//dG_left = (((s64)( g1 - g0 )) << 24 ) / ((s64)( y1 - y0 ));
			//dB_left = (((s64)( b1 - b0 )) << 24 ) / ((s64)( y1 - y0 ));
			dR_left = ( ((s64)( r1 - r0 )) * r10 ) >> 24;
			dG_left = ( ((s64)( g1 - g0 )) * r10 ) >> 24;
			dB_left = ( ((s64)( b1 - b0 )) * r10 ) >> 24;
		}
		else
		{
			dx_right = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
			dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			//dR_right = (((s64)( r2 - r0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dG_right = (((s64)( g2 - g0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dB_right = (((s64)( b2 - b0 )) << 24 ) / ((s64)( y2 - y0 ));
			dR_left = ( ((s64)( r2 - r0 )) * r20 ) >> 24;
			dG_left = ( ((s64)( g2 - g0 )) * r20 ) >> 24;
			dB_left = ( ((s64)( b2 - b0 )) * r20 ) >> 24;
		}
	}
	else
	{
		if ( denominator < 0 )
		{
			// change x_left and x_right where y1 is on left
			x_left = ( ((s64)x1) << 16 );
			x_right = ( ((s64)x0) << 16 );
			
			//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			R_left = ( ((s64)r1) << 24 );
			G_left = ( ((s64)g1) << 24 );
			B_left = ( ((s64)b1) << 24 );

			//dR_left = (((s64)( r2 - r1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dG_left = (((s64)( g2 - g1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dB_left = (((s64)( b2 - b1 )) << 24 ) / ((s64)( y2 - y1 ));
			dR_left = ( ((s64)( r2 - r1 )) * r21 ) >> 24;
			dG_left = ( ((s64)( g2 - g1 )) * r21 ) >> 24;
			dB_left = ( ((s64)( b2 - b1 )) * r21 ) >> 24;
		}
		else
		{
			x_right = ( ((s64)x1) << 16 );
			x_left = ( ((s64)x0) << 16 );
			
			dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			R_left = ( ((s64)r0) << 24 );
			G_left = ( ((s64)g0) << 24 );
			B_left = ( ((s64)b0) << 24 );
			
			//dR_right = (((s64)( r2 - r0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dG_right = (((s64)( g2 - g0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dB_right = (((s64)( b2 - b0 )) << 24 ) / ((s64)( y2 - y0 ));
			dR_left = ( ((s64)( r2 - r0 )) * r20 ) >> 24;
			dG_left = ( ((s64)( g2 - g0 )) * r20 ) >> 24;
			dB_left = ( ((s64)( b2 - b0 )) * r20 ) >> 24;
		}
	}

	/////////////////////////////////////////////////
	// swap left and right if they are reversed
	//if ( ( ( x_right + dx_right ) < ( x_left + dx_left ) ) || ( x_right < x_left ) )
	/*
	if ( denominator > 0 )
	{
		// x1,y1 is on the right //
		
		//Y1_OnLeft = 0;
		
		Swap ( x_left, x_right );
		Swap ( dx_left, dx_right );

		Swap ( dR_left, dR_right );
		Swap ( dG_left, dG_right );
		Swap ( dB_left, dB_right );

		Swap ( R_left, R_right );
		Swap ( G_left, G_right );
		Swap ( B_left, B_right );
	}
	*/
	
	
	// r,g,b values are not specified with a fractional part, so there must be an initial fractional part
	R_left |= ( 1 << 23 );
	G_left |= ( 1 << 23 );
	B_left |= ( 1 << 23 );
	
	
	StartY = y0;
	EndY = y1;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		
		if ( EndY < ((s32)DrawArea_TopLeftY) )
		{
			Temp = EndY - StartY;
			StartY = EndY;
		}
		else
		{
			Temp = DrawArea_TopLeftY - StartY;
			StartY = DrawArea_TopLeftY;
		}
		
		x_left += dx_left * Temp;
		x_right += dx_right * Temp;
		
		R_left += dR_left * Temp;
		G_left += dG_left * Temp;
		B_left += dB_left * Temp;
		
		//R_right += dR_right * Temp;
		//G_right += dG_right * Temp;
		//B_right += dB_right * Temp;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY + 1;
	}

#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
	// setup the values to add going across
	vSeq32_dr1 = _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq32_1 );
	vSeq32_dr2 = _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq32_2 );
	vSeq32_dg1 = _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq32_1 );
	vSeq32_dg2 = _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq32_2 );
	vSeq32_db1 = _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq32_1 );
	vSeq32_db2 = _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq32_2 );
#endif

	if ( EndY > StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y1
	for ( Line = StartY; Line < EndY; Line++ )
	{
		//StartX = _Round( x_left ) >> 32;
		//EndX = _Round( x_right ) >> 32;
		
		// left point is included if points are equal
		StartX = ( x_left + 0xffffLL ) >> 16;
		EndX = ( x_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			
			iR = R_left;
			iG = G_left;
			iB = B_left;
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - x_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iR += dR_across * Temp;
				//iG += dG_across * Temp;
				//iB += dB_across * Temp;
			}
			
			iR += ( dR_across >> 12 ) * ( Temp >> 4 );
			iG += ( dG_across >> 12 ) * ( Temp >> 4 );
			iB += ( dB_across >> 12 ) * ( Temp >> 4 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			NumberOfPixelsDrawn += EndX - StartX + 1;

			if ( DTD )
			{
				DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			}
			
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
	vx_across = _mm_add_epi16 ( vSeq, _mm_set1_epi16 ( StartX ) );
	vEndX = _mm_set1_epi16 ( EndX + 1 );
	
	//viR1 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq32_1 ) );
	//viR2 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq32_2 ) );
	//viG1 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq32_1 ) );
	//viG2 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq32_2 ) );
	//viB1 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq32_1 ) );
	//viB2 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq32_2 ) );
	
	viR1 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), vSeq32_dr1 );
	viR2 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), vSeq32_dr2 );
	viG1 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), vSeq32_dg1 );
	viG2 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), vSeq32_dg2 );
	viB1 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), vSeq32_db1 );
	viB2 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), vSeq32_db2 );
	
	vDitherValue_add = _mm_loadu_si128 ((__m128i const*) ( &(vDitherArray_add [ ( StartX & 0x3 ) + ( ( Line & 0x3 ) << 4 ) ]) ));
	vDitherValue_sub = _mm_loadu_si128 ((__m128i const*) ( &(vDitherArray_sub [ ( StartX & 0x3 ) + ( ( Line & 0x3 ) << 4 ) ]) ));
#endif

			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across < EndX; x_across++ )
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize8 )
			{
				if ( ABE || PIXELMASK )
				{
					vDestPixel = _mm_loadu_si128 ((__m128i const*) ptr);
				}
				
				if ( DTD )
				{
					vRed = _mm_srai_epi32 ( viR, 16 );
					viR = _mm_add_epi32 ( viR, vdR_across4 );
					vRed = _mm_packs_epi32 ( vRed, _mm_srai_epi32 ( viR, 16 ) );
					vRed = _mm_adds_epu16 ( vRed, vDitherValue_add );
					vRed = _mm_subs_epu16 ( vRed, vDitherValue_sub );
					
					vGreen = _mm_srai_epi32 ( viG, 16 );
					viG = _mm_add_epi32 ( viG, vdG_across4 );
					vGreen = _mm_packs_epi32 ( vGreen, _mm_srai_epi32 ( viG, 16 ) );
					vGreen = _mm_adds_epu16 ( vGreen, vDitherValue_add );
					vGreen = _mm_subs_epu16 ( vGreen, vDitherValue_sub );
					
					vBlue = _mm_srai_epi32 ( viB, 16 );
					viB = _mm_add_epi32 ( viB, vdB_across4 );
					vBlue = _mm_packs_epi32 ( vBlue, _mm_srai_epi32 ( viB, 16 ) );
					vBlue = _mm_adds_epu16 ( vBlue, vDitherValue_add );
					vBlue = _mm_subs_epu16 ( vBlue, vDitherValue_sub );
					
					vRed = _mm_srli_epi16 ( vRed, 11 );
					vGreen = _mm_srli_epi16 ( vGreen, 11 );
					vBlue = _mm_srli_epi16 ( vBlue, 11 );
					
				}
				else
				{
					vRed = _mm_srli_epi32 ( viR, 27 );
					viR = _mm_add_epi32 ( viR, vdR_across4 );
					vRed = _mm_packs_epi32 ( vRed, _mm_srli_epi32 ( viR, 27 ) );
					
					vGreen = _mm_srli_epi32 ( viG, 27 );
					viG = _mm_add_epi32 ( viG, vdG_across4 );
					vGreen = _mm_packs_epi32 ( vGreen, _mm_srli_epi32 ( viG, 27 ) );
					
					vBlue = _mm_srli_epi32 ( viB, 27 );
					viB = _mm_add_epi32 ( viB, vdB_across4 );
					vBlue = _mm_packs_epi32 ( vBlue, _mm_srli_epi32 ( viB, 27 ) );
				}
					
				//vRed = _mm_slli_epi16 ( vRed, 0 );
				vGreen = _mm_slli_epi16 ( vGreen, 5 );
				vBlue = _mm_slli_epi16 ( vBlue, 10 );
				
				vpixel = _mm_or_si128 ( _mm_or_si128 ( vRed, vGreen ), vBlue );
				
				if ( ABE )
				{
					vpixel = vSemiTransparency16_t<ABRCODE>( vDestPixel, vpixel, c_vLoBitMask, c_vPixelMask, c_vClampMask, c_vShiftHalf_Mask, c_vShiftQuarter_Mask );
				}
				
				if ( SETPIXELMASK )
				{
					vpixel = _mm_or_si128 ( vpixel, vSetPixelMask );
				}
				
				if ( PIXELMASK )
				{
					vStoreMask = _mm_andnot_si128 ( _mm_and_si128 ( _mm_srai_epi16 ( vDestPixel, 15 ), vPixelMask ), _mm_cmplt_epi16 ( vx_across, vEndX ) );
				}
				else
				{
					vStoreMask = _mm_cmplt_epi16 ( vx_across, vEndX );
				}
				
				_mm_maskmoveu_si128 ( vpixel, vStoreMask, (char*) ptr );
				
				vx_across = _mm_add_epi16 ( vx_across, vVectorSize );
				
				viR = _mm_add_epi32 ( viR, vdR_across4 );
				//viR2 = _mm_add_epi32 ( viR2, vdR_across );
				viG = _mm_add_epi32 ( viG, vdG_across4 );
				//viG2 = _mm_add_epi32 ( viG2, vdG_across );
				viB = _mm_add_epi32 ( viB, vdB_across4 );
				//viB2 = _mm_add_epi32 ( viB2, vdB_across );
				
				ptr += c_iVectorSize8;
			}
#else
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
				if ( ABE || PIXELMASK )
				{
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
				}
					
				if ( DTD )
				{
					DitherValue = DitherLine [ x_across & 0x3 ];
					
					// perform dither
					Red = iR + DitherValue;
					Green = iG + DitherValue;
					Blue = iB + DitherValue;
					
					// perform shift
					Red >>= 27;
					Green >>= 27;
					Blue >>= 27;
					
					// if dithering, perform signed clamp to 5 bits
					Red = AddSignedClamp<s64,5> ( Red );
					Green = AddSignedClamp<s64,5> ( Green );
					Blue = AddSignedClamp<s64,5> ( Blue );
				}
				else
				{
					// perform shift
					Red = ( iR >> 27 );
					Green = ( iG >> 27 );
					Blue = ( iB >> 27 );
					
					// *** NOTE *** when dithering is applied this was actually clamped in "AddSignedClamp" function, so need to do same here!
					// *** NOTE *** no need to clamp here currently since Red, Green, Blue are s64 and not s32, but on PS2 need to do this here...
					// clamp
					//Red &= 0x1f;
					//Green &= 0x1f;
					//Blue &= 0x1f;
				}
					
				// combine
				pixel = ( Blue << 10 ) | ( Green << 5 ) | Red;
					
					
				// *** testing ***
				//debug << "\r\nDestPixel=" << hex << DestPixel;
				
					// *** testing ***
					//debug << " (before)bgr_temp=" << hex << bgr_temp;
					
				// semi-transparency
				if ( ABE )
				{
					pixel = SemiTransparency16_t<ABRCODE> ( DestPixel, pixel );
				}
					
				if ( SETPIXELMASK )
				{
					// check if we should set mask bit when drawing
					pixel |= SETPIXELMASK;
				}

					// *** testing ***
					//debug << " (before)bgr_temp=" << hex << bgr_temp;
					//debug << " SetPixelMask=" << SetPixelMask << " PixelMask=" << PixelMask << " DestPixel=" << DestPixel;
					
				if ( PIXELMASK )
				{
					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PIXELMASK ) ) *ptr = pixel;
				}
				else
				{
					*ptr = pixel;
				}
						
				iR += dR_across;
				iG += dG_across;
				iB += dB_across;
				
				//ptr++;
				ptr += c_iVectorSize;
			}
#endif
		}
		
		
		/////////////////////////////////////
		// update x on left and right
		x_left += dx_left;
		x_right += dx_right;
		
		R_left += dR_left;
		G_left += dG_left;
		B_left += dB_left;
		//R_right += dR_right;
		//G_right += dG_right;
		//B_right += dB_right;
	}

	}
	
	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	/////////////////////////////////////////////
	// init x on the left and right
	
	//////////////////////////////////////////////////////
	// check if y1 is on the left or on the right
	if ( denominator < 0 )
	{
		x_left = ( ((s64)x1) << 16 );

		R_left = ( ((s64)r1) << 24 );
		G_left = ( ((s64)g1) << 24 );
		B_left = ( ((s64)b1) << 24 );
		
		// r,g,b values are not specified with a fractional part, so there must be an initial fractional part
		R_left |= ( 1 << 23 );
		G_left |= ( 1 << 23 );
		B_left |= ( 1 << 23 );
		
		//if ( y2 - y1 )
		//{
			//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			
			//dR_left = (((s64)( r2 - r1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dG_left = (((s64)( g2 - g1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dB_left = (((s64)( b2 - b1 )) << 24 ) / ((s64)( y2 - y1 ));
			dR_left = ( ((s64)( r2 - r1 )) * r21 ) >> 24;
			dG_left = ( ((s64)( g2 - g1 )) * r21 ) >> 24;
			dB_left = ( ((s64)( b2 - b1 )) * r21 ) >> 24;
		//}
	}
	else
	{
		x_right = ( ((s64)x1) << 16 );

		//R_right = ( ((s64)r1) << 24 );
		//G_right = ( ((s64)g1) << 24 );
		//B_right = ( ((s64)b1) << 24 );
		
		//if ( y2 - y1 )
		//{
			//dx_right = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			
			//dR_right = ( ((s64)( r2 - r1 )) * r21 ) >> 24;
			//dG_right = ( ((s64)( g2 - g1 )) * r21 ) >> 24;
			//dB_right = ( ((s64)( b2 - b1 )) * r21 ) >> 24;
		//}
	}
	
	// *** testing ***
	//debug << "\r\nfixed: dR_across=" << dR_across << " dG_across=" << dG_across << " dB_across=" << dB_across;
	
	// the line starts at y1 from here
	//Line = y1;

	StartY = y1;
	EndY = y2;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		
		if ( EndY < ((s32)DrawArea_TopLeftY) )
		{
			Temp = EndY - StartY;
			StartY = EndY;
		}
		else
		{
			Temp = DrawArea_TopLeftY - StartY;
			StartY = DrawArea_TopLeftY;
		}
		
		x_left += dx_left * Temp;
		x_right += dx_right * Temp;
		
		R_left += dR_left * Temp;
		G_left += dG_left * Temp;
		B_left += dB_left * Temp;
		
		//R_right += dR_right * Temp;
		//G_right += dG_right * Temp;
		//B_right += dB_right * Temp;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY + 1;
	}


	if ( EndY > StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y2
	for ( Line = StartY; Line < EndY; Line++ )
	{
		//StartX = _Round( x_left ) >> 32;
		//EndX = _Round( x_right ) >> 32;
		
		// left point is included if points are equal
		StartX = ( x_left + 0xffffLL ) >> 16;
		EndX = ( x_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			iR = R_left;
			iG = G_left;
			iB = B_left;
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - x_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iR += dR_across * Temp;
				//iG += dG_across * Temp;
				//iB += dB_across * Temp;
			}
			
			iR += ( dR_across >> 12 ) * ( Temp >> 4 );
			iG += ( dG_across >> 12 ) * ( Temp >> 4 );
			iB += ( dB_across >> 12 ) * ( Temp >> 4 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			NumberOfPixelsDrawn += EndX - StartX + 1;

			if ( DTD )
			{
				DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			}
			
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
	vx_across = _mm_add_epi16 ( vSeq, _mm_set1_epi16 ( StartX ) );
	vEndX = _mm_set1_epi16 ( EndX + 1 );
	
	//viR1 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq32_1 ) );
	//viR2 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq32_2 ) );
	//viG1 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq32_1 ) );
	//viG2 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq32_2 ) );
	//viB1 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq32_1 ) );
	//viB2 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq32_2 ) );
	
	viR1 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), vSeq32_dr1 );
	viR2 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), vSeq32_dr2 );
	viG1 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), vSeq32_dg1 );
	viG2 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), vSeq32_dg2 );
	viB1 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), vSeq32_db1 );
	viB2 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), vSeq32_db2 );
	
	vDitherValue_add = _mm_loadu_si128 ((__m128i const*) ( &(vDitherArray_add [ ( StartX & 0x3 ) + ( ( Line & 0x3 ) << 4 ) ]) ));
	vDitherValue_sub = _mm_loadu_si128 ((__m128i const*) ( &(vDitherArray_sub [ ( StartX & 0x3 ) + ( ( Line & 0x3 ) << 4 ) ]) ));
#endif

			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across < EndX; x_across++ )
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize8 )
			{
				if ( ABE || PIXELMASK )
				{
					vDestPixel = _mm_loadu_si128 ((__m128i const*) ptr);
				}
				
				if ( DTD )
				{
					vRed = _mm_srai_epi32 ( viR, 16 );
					viR = _mm_add_epi32 ( viR, vdR_across4 );
					vRed = _mm_packs_epi32 ( vRed, _mm_srai_epi32 ( viR, 16 ) );
					vRed = _mm_adds_epu16 ( vRed, vDitherValue_add );
					vRed = _mm_subs_epu16 ( vRed, vDitherValue_sub );
					
					vGreen = _mm_srai_epi32 ( viG, 16 );
					viG = _mm_add_epi32 ( viG, vdG_across4 );
					vGreen = _mm_packs_epi32 ( vGreen, _mm_srai_epi32 ( viG, 16 ) );
					vGreen = _mm_adds_epu16 ( vGreen, vDitherValue_add );
					vGreen = _mm_subs_epu16 ( vGreen, vDitherValue_sub );
					
					vBlue = _mm_srai_epi32 ( viB, 16 );
					viB = _mm_add_epi32 ( viB, vdB_across4 );
					vBlue = _mm_packs_epi32 ( vBlue, _mm_srai_epi32 ( viB, 16 ) );
					vBlue = _mm_adds_epu16 ( vBlue, vDitherValue_add );
					vBlue = _mm_subs_epu16 ( vBlue, vDitherValue_sub );
					
					vRed = _mm_srli_epi16 ( vRed, 11 );
					vGreen = _mm_srli_epi16 ( vGreen, 11 );
					vBlue = _mm_srli_epi16 ( vBlue, 11 );
					
				}
				else
				{
					vRed = _mm_srli_epi32 ( viR, 27 );
					viR = _mm_add_epi32 ( viR, vdR_across4 );
					vRed = _mm_packs_epi32 ( vRed, _mm_srli_epi32 ( viR, 27 ) );
					
					vGreen = _mm_srli_epi32 ( viG, 27 );
					viG = _mm_add_epi32 ( viG, vdG_across4 );
					vGreen = _mm_packs_epi32 ( vGreen, _mm_srli_epi32 ( viG, 27 ) );
					
					vBlue = _mm_srli_epi32 ( viB, 27 );
					viB = _mm_add_epi32 ( viB, vdB_across4 );
					vBlue = _mm_packs_epi32 ( vBlue, _mm_srli_epi32 ( viB, 27 ) );
				}
					
				//vRed = _mm_slli_epi16 ( vRed, 0 );
				vGreen = _mm_slli_epi16 ( vGreen, 5 );
				vBlue = _mm_slli_epi16 ( vBlue, 10 );
				
				vpixel = _mm_or_si128 ( _mm_or_si128 ( vRed, vGreen ), vBlue );
				
				if ( ABE )
				{
					vpixel = vSemiTransparency16_t<ABRCODE>( vDestPixel, vpixel, c_vLoBitMask, c_vPixelMask, c_vClampMask, c_vShiftHalf_Mask, c_vShiftQuarter_Mask );
				}
				
				if ( SETPIXELMASK )
				{
					vpixel = _mm_or_si128 ( vpixel, vSetPixelMask );
				}
				
				if ( PIXELMASK )
				{
					vStoreMask = _mm_andnot_si128 ( _mm_and_si128 ( _mm_srai_epi16 ( vDestPixel, 15 ), vPixelMask ), _mm_cmplt_epi16 ( vx_across, vEndX ) );
				}
				else
				{
					vStoreMask = _mm_cmplt_epi16 ( vx_across, vEndX );
				}
				
				_mm_maskmoveu_si128 ( vpixel, vStoreMask, (char*) ptr );
				
				vx_across = _mm_add_epi16 ( vx_across, vVectorSize );
				
				viR = _mm_add_epi32 ( viR, vdR_across4 );
				//viR2 = _mm_add_epi32 ( viR2, vdR_across );
				viG = _mm_add_epi32 ( viG, vdG_across4 );
				//viG2 = _mm_add_epi32 ( viG2, vdG_across );
				viB = _mm_add_epi32 ( viB, vdB_across4 );
				//viB2 = _mm_add_epi32 ( viB2, vdB_across );
				
				ptr += c_iVectorSize8;
			}
#else
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
				if ( ABE || PIXELMASK )
				{
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
				}
					
				if ( DTD )
				{
					DitherValue = DitherLine [ x_across & 0x3 ];
					
					// perform dither
					Red = iR + DitherValue;
					Green = iG + DitherValue;
					Blue = iB + DitherValue;
					
					// perform shift
					Red >>= 27;
					Green >>= 27;
					Blue >>= 27;
					
					// if dithering, perform signed clamp to 5 bits
					Red = AddSignedClamp<s64,5> ( Red );
					Green = AddSignedClamp<s64,5> ( Green );
					Blue = AddSignedClamp<s64,5> ( Blue );
				}
				else
				{
					// perform shift
					Red = ( iR >> 27 );
					Green = ( iG >> 27 );
					Blue = ( iB >> 27 );
					
					// *** NOTE *** when dithering is applied this was actually clamped in "AddSignedClamp" function, so need to do same here!
					// *** NOTE *** no need to clamp here currently since Red, Green, Blue are s64 and not s32, but on PS2 need to do this here...
					// clamp
					//Red &= 0x1f;
					//Green &= 0x1f;
					//Blue &= 0x1f;
				}
				
				// combine
				pixel = ( Blue << 10 ) | ( Green << 5 ) | Red;
					
					
				// *** testing ***
				//debug << "\r\nDestPixel=" << hex << DestPixel;
				
					// *** testing ***
					//debug << " (before)bgr_temp=" << hex << bgr_temp;
					
				// semi-transparency
				if ( ABE )
				{
					pixel = SemiTransparency16_t<ABRCODE> ( DestPixel, pixel );
				}
					
				if ( SETPIXELMASK )
				{
					// check if we should set mask bit when drawing
					pixel |= SETPIXELMASK;
				}

					// *** testing ***
					//debug << " (before)bgr_temp=" << hex << bgr_temp;
					//debug << " SetPixelMask=" << SetPixelMask << " PixelMask=" << PixelMask << " DestPixel=" << DestPixel;
					
				if ( PIXELMASK )
				{
					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PIXELMASK ) ) *ptr = pixel;
				}
				else
				{
					*ptr = pixel;
				}

					
				iR += dR_across;
				iG += dG_across;
				iB += dB_across;
				
				//ptr++;
				ptr += c_iVectorSize;
			}
#endif
		}
		
		/////////////////////////////////////
		// update x on left and right
		x_left += dx_left;
		x_right += dx_right;
		
		R_left += dR_left;
		G_left += dG_left;
		B_left += dB_left;
		//R_right += dR_right;
		//G_right += dG_right;
		//B_right += dB_right;
	}
	
	}
		
}



template<const long PIXELMASK, const long SETPIXELMASK, const long ABE, const long ABRCODE>
inline void GPU::SelectTriangle_Gradient4_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	if ( GPU_CTRL_Read.DTD )
	{
		DrawTriangle_Gradient_t <PIXELMASK,SETPIXELMASK,ABE,ABRCODE,1> ( Coord0, Coord1, Coord2 );
	}
	else
	{
		DrawTriangle_Gradient_t <PIXELMASK,SETPIXELMASK,ABE,ABRCODE,0> ( Coord0, Coord1, Coord2 );
	}
}



template<const long PIXELMASK, const long ABE, const long ABRCODE>
inline void GPU::SelectTriangle_Gradient3_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	if ( GPU_CTRL_Read.MD )
	{
		SelectTriangle_Gradient4_t <PIXELMASK,0x8000,ABE,ABRCODE> ( Coord0, Coord1, Coord2 );
	}
	else
	{
		SelectTriangle_Gradient4_t <PIXELMASK,0,ABE,ABRCODE> ( Coord0, Coord1, Coord2 );
	}
}


template<const long ABE, const long ABRCODE>
inline void GPU::SelectTriangle_Gradient2_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	if ( GPU_CTRL_Read.ME )
	{
		SelectTriangle_Gradient3_t <0x8000,ABE,ABRCODE> ( Coord0, Coord1, Coord2 );
	}
	else
	{
		SelectTriangle_Gradient3_t <0,ABE,ABRCODE> ( Coord0, Coord1, Coord2 );
	}
}


template<const long ABE>
void GPU::SelectTriangle_Gradient_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	switch ( GPU_CTRL_Read.ABR )
	{
		case 0:
			SelectTriangle_Gradient2_t <ABE,0> ( Coord0, Coord1, Coord2 );
			break;
			
		case 1:
			SelectTriangle_Gradient2_t <ABE,1> ( Coord0, Coord1, Coord2 );
			break;
			
		case 2:
			SelectTriangle_Gradient2_t <ABE,2> ( Coord0, Coord1, Coord2 );
			break;
			
		case 3:
			SelectTriangle_Gradient2_t <ABE,3> ( Coord0, Coord1, Coord2 );
			break;
	}
}


template<const long ABE>
void GPU::Draw_GradientTriangle_30_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	//if ( ( bgr0 & 0x00ffffff ) == ( bgr1 & 0x00ffffff ) && ( bgr0 & 0x00ffffff ) == ( bgr2 & 0x00ffffff ) )
	if ( gbgr [ Coord0 ] == gbgr [ Coord1 ] && gbgr [ Coord0 ] == gbgr [ Coord2 ] )
	{
		//GetBGR ( Buffer [ 0 ] );
		gbgr [ 0 ] = gbgr [ Coord0 ];
		SelectTriangle_Mono_t <ABE> ( Coord0, Coord1, Coord2 );
	}
	else
	{
		SelectTriangle_Gradient_t <ABE> ( Coord0, Coord1, Coord2 );
	}
	
	//SelectTriangle_Gradient_t <ABE> ();
	
	// *** TODO *** calculate cycles to draw here
	// check for alpha blending - add in an extra cycle for this for now
	if ( ABE )
	{
		BusyCycles += NumberOfPixelsDrawn * dAlphaBlending_CyclesPerPixel;
	}
	
	// add in cycles to draw mono-triangle
	BusyCycles += NumberOfPixelsDrawn * dGradientTriangle_30_CyclesPerPixel;
	
#ifdef ENABLE_DRAW_OVERHEAD
	BusyCycles += DrawOverhead_Cycles;
#endif
}


template<const long ABE>
void GPU::Draw_GradientRectangle_38_t ()
{
	//x_save [ 0 ] = x0; x_save [ 1 ] = x1; x_save [ 2 ] = x2; x_save [ 3 ] = x3;
	//y_save [ 0 ] = y0; y_save [ 1 ] = y1; y_save [ 2 ] = y2; y_save [ 3 ] = y3;
	//bgr_save [ 0 ] = bgr0; bgr_save [ 1 ] = bgr1; bgr_save [ 2 ] = bgr2; bgr_save [ 3 ] = bgr3;
	
	Draw_GradientTriangle_30_t <ABE> ( 0, 1, 2 );
	
	//x0 = x_save [ 1 ];
	//y0 = y_save [ 1 ];
	//x1 = x_save [ 2 ];
	//y1 = y_save [ 2 ];
	//x2 = x_save [ 3 ];
	//y2 = y_save [ 3 ];
	
	//bgr0 = bgr_save [ 1 ];
	//bgr1 = bgr_save [ 2 ];
	//bgr2 = bgr_save [ 3 ];
	
	Draw_GradientTriangle_30_t <ABE> ( 1, 2, 3 );

}




template<const long PIXELMASK, const long SETPIXELMASK, const long ABE, const long ABRCODE, const long TGE, const long TP,const long DTD>
void GPU::DrawTriangle_TextureGradient_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	// before sse2, was sending 1 pixels at a time
	static const int c_iVectorSize = 1;

	u32 clut_xoffset;
	
	u32 pixel, pixel_temp;

	//u32 Pixel, TexelIndex;
	
	u32 color_add;

	s64 r10, r20, r21;
	
	u16 *ptr_texture, *ptr_clut;
	u16 *ptr;
	
	s64 Temp, TMin, TMax;
	s64 LeftMostX, RightMostX;
	
	s32 StartX, EndX, StartY, EndY;
	
	s32* DitherArray;
	s32* DitherLine;
	s32 DitherValue;
	

	u32 DestPixel;
	u32 TexCoordX, TexCoordY;
	
	//u32 PixelMask = 0, SetPixelMask = 0;
	//u32 Shift1 = 0, Shift2 = 0, And1 = 0, And2 = 0;

	//s64 Error_Left;

	// new local variables
	s32 x0, x1, x2, y0, y1, y2;
	s64 dx_left, dx_right;
	s64 x_left, x_right, x_across;
	u32 bgr, bgr_temp;
	s32 Line;
	s64 t0, t1, denominator;

	// more local variables for gradient triangle
	s64 dR_left, dG_left, dB_left, dR_across, dG_across, dB_across, iR, iG, iB, R_left, G_left, B_left;
	s32 r0, r1, r2, g0, g1, g2, b0, b1, b2;
	
	// new local variables for texture mapping
	s64 dU_left, dV_left, dU_across, dV_across, U_left, V_left, iU, iV;
	s32 u0, v0, u1, v1, u2, v2;

	
	
	s16 Red, Green, Blue;
	
	u32 TWYTWH, TWXTWW, Not_TWH, Not_TWW;
	
	TWYTWH = ( ( TWY & TWH ) << 3 );
	TWXTWW = ( ( TWX & TWW ) << 3 );
	
	Not_TWH = ~( TWH << 3 );
	Not_TWW = ~( TWW << 3 );
	
	//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;

	///////////////////////////////////////////////////
	// Initialize dithering
	
	//DitherArray = (s32*) c_iDitherZero;
	
	if ( DTD )
	{
		//DitherArray = c_iDitherValues;
		DitherArray = c_iDitherValues4;
	}
	
	
	// initialize number of pixels drawn
	NumberOfPixelsDrawn = 0;
	
	clut_xoffset = clut_x << 4;
	ptr_clut = & ( VRAM [ clut_y << 10 ] );
	ptr_texture = & ( VRAM [ ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) ] );
	
	// check for some important conditions
	if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
		return;
	}
	
	if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
		return;
	}
	
	///////////////////////////////////////////////////
	// Initialize dithering
	
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
	vDitherArray_add = (s16*) c_iDitherZero;
	vDitherArray_sub = (s16*) c_iDitherZero;
#else
	DitherArray = (s32*) c_iDitherZero;
#endif
	
	if ( GPU_CTRL_Read.DTD )
	{
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
		vDitherArray_add = c_viDitherValues16_add;
		vDitherArray_sub = c_viDitherValues16_sub;
#else
		//DitherArray = c_iDitherValues;
		DitherArray = c_iDitherValues4;
#endif
	}



	////////////////////////////////
	// y1 starts on the left
	//Y1_OnLeft = 1;

	// get y-values
	//y0 = Buffer [ Coord0 ].y;
	//y1 = Buffer [ Coord1 ].y;
	//y2 = Buffer [ Coord2 ].y;
	
	///////////////////////////////////
	// put top coordinates in x0,y0
	//if ( y1 < y0 )
	if ( gy [ Coord1 ] < gy [ Coord0 ] )
	{
		//Swap ( y0, y1 );
		Swap ( Coord0, Coord1 );
	}
	
	//if ( y2 < y0 )
	if ( gy [ Coord2 ] < gy [ Coord0 ] )
	{
		//Swap ( y0, y2 );
		Swap ( Coord0, Coord2 );
	}
	
	///////////////////////////////////////
	// put middle coordinates in x1,y1
	//if ( y2 < y1 )
	if ( gy [ Coord2 ] < gy [ Coord1 ] )
	{
		//Swap ( y1, y2 );
		Swap ( Coord1, Coord2 );
	}
	
	// get x-values
	x0 = gx [ Coord0 ];
	x1 = gx [ Coord1 ];
	x2 = gx [ Coord2 ];

	// get y-values
	y0 = gy [ Coord0 ];
	y1 = gy [ Coord1 ];
	y2 = gy [ Coord2 ];
	
	// get rgb-values
	r0 = gr [ Coord0 ];
	r1 = gr [ Coord1 ];
	r2 = gr [ Coord2 ];
	g0 = gg [ Coord0 ];
	g1 = gg [ Coord1 ];
	g2 = gg [ Coord2 ];
	b0 = gb [ Coord0 ];
	b1 = gb [ Coord1 ];
	b2 = gb [ Coord2 ];
	
	// get texture coords
	u0 = gu [ Coord0 ];
	u1 = gu [ Coord1 ];
	u2 = gu [ Coord2 ];
	v0 = gv [ Coord0 ];
	v1 = gv [ Coord1 ];
	v2 = gv [ Coord2 ];


	//////////////////////////////////////////
	// get coordinates on screen
	x0 = DrawArea_OffsetX + x0;
	y0 = DrawArea_OffsetY + y0;
	x1 = DrawArea_OffsetX + x1;
	y1 = DrawArea_OffsetY + y1;
	x2 = DrawArea_OffsetX + x2;
	y2 = DrawArea_OffsetY + y2;
	
	
	
	//////////////////////////////////////////////////////
	// Get offset into color lookup table
	//u32 ClutOffset = ( clut_x << 4 ) + ( clut_y << 10 );
	
	/////////////////////////////////////////////////////////
	// Get offset into texture page
	//u32 TextureOffset = ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 );
	
	
	//u32 NibblesPerPixel;
	
	//if ( tpage_tp == 0 ) NibblesPerPixel = 1; else if ( tpage_tp == 1 ) NibblesPerPixel = 2; else NibblesPerPixel = 4;

	/*
	if ( tpage_tp == 0 )
	{
		And2 = 0xf;
		
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
		Shift1 = _mm_set_epi32 ( 0, 0, 0, 2 );
		Shift2 = _mm_set_epi32 ( 0, 0, 0, 2 );
		And1 = _mm_set1_epi16 ( 3 );
#else
		Shift1 = 2; Shift2 = 2;
		And1 = 3; And2 = 0xf;
#endif
	}
	else if ( tpage_tp == 1 )
	{
		And2 = 0xff;
		
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
		Shift1 = _mm_set_epi32 ( 0, 0, 0, 1 );
		Shift2 = _mm_set_epi32 ( 0, 0, 0, 3 );
		And1 = _mm_set1_epi16 ( 1 );
#else
		Shift1 = 1; Shift2 = 3;
		And1 = 1; And2 = 0xff;
#endif
	}
	*/
	
	
	// get the left/right most x
	LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
	LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
	RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
	RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

	// check if sprite is within draw area
	if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	// skip drawing if distance between vertices is greater than max allowed by GPU
	if ( ( _Abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( _Abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
	{
		// skip drawing polygon
		return;
	}
	
	
	// calculate across
	t0 = y1 - y2;
	t1 = y0 - y2;
	denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );
	if ( denominator )
	{
		//dR_across = ( ( (s64) ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) ) << 24 ) / denominator;
		//dG_across = ( ( (s64) ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) ) << 24 ) / denominator;
		//dB_across = ( ( (s64) ( ( ( b0 - b2 ) * t0 ) - ( ( b1 - b2 ) * t1 ) ) ) << 24 ) / denominator;
		//dU_across = ( ( (s64) ( ( ( u0 - u2 ) * t0 ) - ( ( u1 - u2 ) * t1 ) ) ) << 24 ) / denominator;
		//dV_across = ( ( (s64) ( ( ( v0 - v2 ) * t0 ) - ( ( v1 - v2 ) * t1 ) ) ) << 24 ) / denominator;
		
		denominator = ( 1LL << 48 ) / denominator;
		dR_across = ( ( (s64) ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) ) * denominator ) >> 24;
		dG_across = ( ( (s64) ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) ) * denominator ) >> 24;
		dB_across = ( ( (s64) ( ( ( b0 - b2 ) * t0 ) - ( ( b1 - b2 ) * t1 ) ) ) * denominator ) >> 24;
		dU_across = ( ( (s64) ( ( ( u0 - u2 ) * t0 ) - ( ( u1 - u2 ) * t1 ) ) ) * denominator ) >> 24;
		dV_across = ( ( (s64) ( ( ( v0 - v2 ) * t0 ) - ( ( v1 - v2 ) * t1 ) ) ) * denominator ) >> 24;
		
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
		vdU_across = _mm_set1_epi32 ( dU_across * 8 );
		vdV_across = _mm_set1_epi32 ( dV_across * 8 );
		vdR_across = _mm_set1_epi32 ( dR_across * 8 );
		vdG_across = _mm_set1_epi32 ( dG_across * 8 );
		vdB_across = _mm_set1_epi32 ( dB_across * 8 );
#endif
	}

	
	// get reciprocals
	if ( y1 - y0 ) r10 = ( 1LL << 48 ) / ((s64)( y1 - y0 ));
	if ( y2 - y0 ) r20 = ( 1LL << 48 ) / ((s64)( y2 - y0 ));
	if ( y2 - y1 ) r21 = ( 1LL << 48 ) / ((s64)( y2 - y1 ));
	
	
	/////////////////////////////////////////////////
	// draw top part of triangle
	

	///////////////////////////////////////////
	// start at y0
	//Line = y0;
	
	if ( y1 - y0 )
	{
		x_left = ( ((s64)x0) << 16 );
		x_right = x_left;
		
		U_left = ( ((s64)u0) << 24 );
		V_left = ( ((s64)v0) << 24 );
		//U_right = U_left;
		//V_right = V_left;
			
		R_left = ( ((s64)r0) << 24 );
		G_left = ( ((s64)g0) << 24 );
		B_left = ( ((s64)b0) << 24 );
		//R_right = R_left;
		//G_right = G_left;
		//B_right = B_left;
		
		if ( denominator < 0 )
		{
			//dx_left = (((s64)( x1 - x0 )) << 16 ) / ((s64)( y1 - y0 ));
			//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			dx_left = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
			dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			//dU_left = (((s64)( u1 - u0 )) << 24 ) / ((s64)( y1 - y0 ));
			//dV_left = (((s64)( v1 - v0 )) << 24 ) / ((s64)( y1 - y0 ));
			dU_left = ( ((s64)( u1 - u0 )) * r10 ) >> 24;
			dV_left = ( ((s64)( v1 - v0 )) * r10 ) >> 24;

			//dR_left = (((s64)( r1 - r0 )) << 24 ) / ((s64)( y1 - y0 ));
			//dG_left = (((s64)( g1 - g0 )) << 24 ) / ((s64)( y1 - y0 ));
			//dB_left = (((s64)( b1 - b0 )) << 24 ) / ((s64)( y1 - y0 ));
			dR_left = ( ((s64)( r1 - r0 )) * r10 ) >> 24;
			dG_left = ( ((s64)( g1 - g0 )) * r10 ) >> 24;
			dB_left = ( ((s64)( b1 - b0 )) * r10 ) >> 24;
		}
		else
		{
			//dx_left = (((s64)( x1 - x0 )) << 16 ) / ((s64)( y1 - y0 ));
			//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			dx_right = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
			dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			//dU_right = (((s64)( u2 - u0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dV_right = (((s64)( v2 - v0 )) << 24 ) / ((s64)( y2 - y0 ));
			dU_left = ( ((s64)( u2 - u0 )) * r20 ) >> 24;
			dV_left = ( ((s64)( v2 - v0 )) * r20 ) >> 24;
			
			//dR_right = (((s64)( r2 - r0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dG_right = (((s64)( g2 - g0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dB_right = (((s64)( b2 - b0 )) << 24 ) / ((s64)( y2 - y0 ));
			dR_left = ( ((s64)( r2 - r0 )) * r20 ) >> 24;
			dG_left = ( ((s64)( g2 - g0 )) * r20 ) >> 24;
			dB_left = ( ((s64)( b2 - b0 )) * r20 ) >> 24;
		}
	}
	else
	{
		
		if ( denominator < 0 )
		{
			// change x_left and x_right where y1 is on left
			x_left = ( ((s64)x1) << 16 );
			x_right = ( ((s64)x0) << 16 );
			
			//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			// change U_left and V_right where y1 is on left
			U_left = ( ((s64)u1) << 24 );
			V_left = ( ((s64)v1) << 24 );
			
			R_left = ( ((s64)r1) << 24 );
			G_left = ( ((s64)g1) << 24 );
			B_left = ( ((s64)b1) << 24 );
		
			//dU_left = (((s64)( u2 - u1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dV_left = (((s64)( v2 - v1 )) << 24 ) / ((s64)( y2 - y1 ));
			dU_left = ( ((s64)( u2 - u1 )) * r21 ) >> 24;
			dV_left = ( ((s64)( v2 - v1 )) * r21 ) >> 24;
			
			//dR_left = (((s64)( r2 - r1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dG_left = (((s64)( g2 - g1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dB_left = (((s64)( b2 - b1 )) << 24 ) / ((s64)( y2 - y1 ));
			dR_left = ( ((s64)( r2 - r1 )) * r21 ) >> 24;
			dG_left = ( ((s64)( g2 - g1 )) * r21 ) >> 24;
			dB_left = ( ((s64)( b2 - b1 )) * r21 ) >> 24;
		}
		else
		{
			// change x_left and x_right where y1 is on left
			x_right = ( ((s64)x1) << 16 );
			x_left = ( ((s64)x0) << 16 );
			
			//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			U_left = ( ((s64)u0) << 24 );
			V_left = ( ((s64)v0) << 24 );
			
			R_left = ( ((s64)r0) << 24 );
			G_left = ( ((s64)g0) << 24 );
			B_left = ( ((s64)b0) << 24 );
			
			//dU_right = (((s64)( u2 - u0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dV_right = (((s64)( v2 - v0 )) << 24 ) / ((s64)( y2 - y0 ));
			dU_left = ( ((s64)( u2 - u0 )) * r20 ) >> 24;
			dV_left = ( ((s64)( v2 - v0 )) * r20 ) >> 24;
			
			//dR_right = (((s64)( r2 - r0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dG_right = (((s64)( g2 - g0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dB_right = (((s64)( b2 - b0 )) << 24 ) / ((s64)( y2 - y0 ));
			dR_left = ( ((s64)( r2 - r0 )) * r20 ) >> 24;
			dG_left = ( ((s64)( g2 - g0 )) * r20 ) >> 24;
			dB_left = ( ((s64)( b2 - b0 )) * r20 ) >> 24;
		}
	}

	
	
	/////////////////////////////////////////////////
	// swap left and right if they are reversed
	//if ( ( ( x_right + dx_right ) < ( x_left + dx_left ) ) || ( x_right < x_left ) )
	/*
	if ( denominator > 0 )
	{
		// x1,y1 is on the right //
		
		//Y1_OnLeft = 0;
		
		Swap ( x_left, x_right );
		Swap ( dx_left, dx_right );

		Swap ( dU_left, dU_right );
		Swap ( dV_left, dV_right );

		Swap ( U_left, U_right );
		Swap ( V_left, V_right );
		
		Swap ( dR_left, dR_right );
		Swap ( dG_left, dG_right );
		Swap ( dB_left, dB_right );

		Swap ( R_left, R_right );
		Swap ( G_left, G_right );
		Swap ( B_left, B_right );
	}
	*/


	// r,g,b,u,v values are not specified with a fractional part, so there must be an initial fractional part
	R_left |= ( 1 << 23 );
	G_left |= ( 1 << 23 );
	B_left |= ( 1 << 23 );
	U_left |= ( 1 << 23 );
	V_left |= ( 1 << 23 );
	
	
	StartY = y0;
	EndY = y1;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		
		if ( EndY < ((s32)DrawArea_TopLeftY) )
		{
			Temp = EndY - StartY;
			StartY = EndY;
		}
		else
		{
			Temp = DrawArea_TopLeftY - StartY;
			StartY = DrawArea_TopLeftY;
		}
		
		x_left += dx_left * Temp;
		x_right += dx_right * Temp;
		
		U_left += dU_left * Temp;
		V_left += dV_left * Temp;
		R_left += dR_left * Temp;
		G_left += dG_left * Temp;
		B_left += dB_left * Temp;
		
		//U_right += dU_right * Temp;
		//V_right += dV_right * Temp;
		//R_right += dR_right * Temp;
		//G_right += dG_right * Temp;
		//B_right += dB_right * Temp;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY + 1;
	}

#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
	vSeq32_u1 = _custom_mul_32 ( _mm_set1_epi32 ( dU_across ), vSeq32_1 );
	vSeq32_u2 = _custom_mul_32 ( _mm_set1_epi32 ( dU_across ), vSeq32_2 );
	vSeq32_v1 = _custom_mul_32 ( _mm_set1_epi32 ( dV_across ), vSeq32_1 );
	vSeq32_v2 = _custom_mul_32 ( _mm_set1_epi32 ( dV_across ), vSeq32_2 );
	vSeq32_r1 = _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq32_1 );
	vSeq32_r2 = _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq32_2 );
	vSeq32_g1 = _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq32_1 );
	vSeq32_g2 = _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq32_2 );
	vSeq32_b1 = _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq32_1 );
	vSeq32_b2 = _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq32_2 );
#endif
	
	if ( EndY > StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y1
	for ( Line = StartY; Line < EndY; Line++ )
	{
		//StartX = _Round( x_left ) >> 32;
		//EndX = _Round( x_right ) >> 32;
		
		// left point is included if points are equal
		StartX = ( x_left + 0xffffLL ) >> 16;
		EndX = ( x_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			iU = U_left;
			iV = V_left;
			
			iR = R_left;
			iG = G_left;
			iB = B_left;

			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - x_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iU += dU_across * Temp;
				//iV += dV_across * Temp;
				
				//iR += dR_across * Temp;
				//iG += dG_across * Temp;
				//iB += dB_across * Temp;
			}

			iU += ( dU_across >> 12 ) * ( Temp >> 4 );
			iV += ( dV_across >> 12 ) * ( Temp >> 4 );
			
			iR += ( dR_across >> 12 ) * ( Temp >> 4 );
			iG += ( dG_across >> 12 ) * ( Temp >> 4 );
			iB += ( dB_across >> 12 ) * ( Temp >> 4 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );

			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			NumberOfPixelsDrawn += EndX - StartX + 1;

			if ( DTD )
			{
				DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			}

			// draw horizontal line
			// x_left and x_right need to be rounded off
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
					if ( DTD )
					{
						DitherValue = DitherLine [ x_across & 0x3 ];
					}

					//TexCoordY = (u8) ( ( ( _Round24 ( iV ) >> 24 ) & Not_TWH ) | TWYTWH );
					//TexCoordX = (u8) ( ( ( _Round24 ( iU ) >> 24 ) & Not_TWW ) | TWXTWW );
					TexCoordY = (u8) ( ( ( iV >> 24 ) & Not_TWH ) | TWYTWH );
					TexCoordX = (u8) ( ( ( iU >> 24 ) & Not_TWW ) | TWXTWW );
					
					// *** testing ***
					//debug << dec << "\r\nTexCoordX=" << TexCoordX << " TexCoordY=" << TexCoordY;
					
					switch ( TP )
					{
						case 0:
							pixel = ptr_texture [ ( TexCoordX >> 2 ) + ( TexCoordY << 10 ) ];
							pixel = ptr_clut [ ( clut_xoffset + ( ( pixel >> ( ( TexCoordX & 3 ) << 2 ) ) & 0xf ) ) & FrameBuffer_XMask ];
							break;
							
						case 1:
							pixel = ptr_texture [ ( TexCoordX >> 1 ) + ( TexCoordY << 10 ) ];
							pixel = ptr_clut [ ( clut_xoffset + ( ( pixel >> ( ( TexCoordX & 1 ) << 3 ) ) & 0xff ) ) & FrameBuffer_XMask ];
							break;
							
						case 2:
							pixel = ptr_texture [ ( TexCoordX ) + ( TexCoordY << 10 ) ];
							break;
					}
					
					
					if ( pixel )
					{
						if ( ABE || PIXELMASK )
						{
							// read pixel from frame buffer if we need to check mask bit
							DestPixel = *ptr;
						}

						if ( ABE || !TGE )
						{
							pixel_temp = pixel & 0x8000;
						}
				
						//if ( !tge )
						if ( !TGE )
						{
							// brightness calculation
							// shade pixel
							Red = ( ( (s16) ( iR >> 24 ) & 0xff ) * ( (s16) ( pixel & 0x1f ) ) );
							Green = ( ( (s16) ( iG >> 24 ) & 0xff ) * ( (s16) ( ( pixel >> 5 ) & 0x1f ) ) );
							Blue = ( ( (s16) ( iB >> 24 ) & 0xff ) * ( (s16) ( ( pixel >> 10 ) & 0x1f ) ) );
						
							if ( DTD )
							{
								// apply dithering if it is enabled
								// dithering must be applied after the color multiply
								Red = Red + DitherValue;
								Green = Green + DitherValue;
								Blue = Blue + DitherValue;
							}
							
							// clamp
							//Red = Clamp5 ( Red >> 7 );
							//Green = Clamp5 ( Green >> 7 );
							//Blue = Clamp5 ( Blue >> 7 );
							Red = SignedClamp<s16,5> ( Red >> 7 );
							Green = SignedClamp<s16,5> ( Green >> 7 );
							Blue = SignedClamp<s16,5> ( Blue >> 7 );
							
							// combine
							pixel = ( Blue << 10 ) | ( Green << 5 ) | ( Red );
						}
						
						// semi-transparency
						//if ( command_abe && ( bgr & 0x8000 ) )
						if ( ABE )
						{
							if ( pixel_temp )
							{
								pixel = SemiTransparency16_t <ABRCODE> ( DestPixel, pixel );
							}
						}
						
						if ( ABE || !TGE )
						{
							// check if we should set mask bit when drawing
							//bgr_temp |= SetPixelMask | ( bgr & 0x8000 );
							pixel |= pixel_temp;
						}
						
						if ( SETPIXELMASK )
						{
							// set pixel mask if specified
							pixel |= SETPIXELMASK;
						}

						// draw pixel if we can draw to mask pixels or mask bit not set
						if ( PIXELMASK )
						{
							if ( ! ( DestPixel & PIXELMASK ) ) *ptr = pixel;
						}
						else
						{
							*ptr = pixel;
						}
					}
				
				/////////////////////////////////////////////////////////
				// interpolate texture coords
				iU += dU_across;
				iV += dV_across;
				
				iR += dR_across;
				iG += dG_across;
				iB += dB_across;
				
				ptr += c_iVectorSize;
			}
			
		}
		
		
		/////////////////////////////////////
		// update x on left and right
		x_left += dx_left;
		x_right += dx_right;
		
		U_left += dU_left;
		V_left += dV_left;
		//U_right += dU_right;
		//V_right += dV_right;
		
		R_left += dR_left;
		G_left += dG_left;
		B_left += dB_left;
		//R_right += dR_right;
		//G_right += dG_right;
		//B_right += dB_right;
	}
	
	}

	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	/////////////////////////////////////////////
	// init x on the left and right
	
	
	
	
	//////////////////////////////////////////////////////
	// check if y1 is on the left or on the right
	if ( denominator < 0 )
	{
		x_left = ( ((s64)x1) << 16 );

		U_left = ( ((s64)u1) << 24 );
		V_left = ( ((s64)v1) << 24 );

		R_left = ( ((s64)r1) << 24 );
		G_left = ( ((s64)g1) << 24 );
		B_left = ( ((s64)b1) << 24 );

		// r,g,b,u,v values are not specified with a fractional part, so there must be an initial fractional part
		R_left |= ( 1 << 23 );
		G_left |= ( 1 << 23 );
		B_left |= ( 1 << 23 );
		U_left |= ( 1 << 23 );
		V_left |= ( 1 << 23 );
		
		//if ( y2 - y1 )
		//{
			//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			
			//dU_left = (((s64)( u2 - u1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dV_left = (((s64)( v2 - v1 )) << 24 ) / ((s64)( y2 - y1 ));
			dU_left = ( ((s64)( u2 - u1 )) * r21 ) >> 24;
			dV_left = ( ((s64)( v2 - v1 )) * r21 ) >> 24;
			
			//dR_left = (((s64)( r2 - r1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dG_left = (((s64)( g2 - g1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dB_left = (((s64)( b2 - b1 )) << 24 ) / ((s64)( y2 - y1 ));
			dR_left = ( ((s64)( r2 - r1 )) * r21 ) >> 24;
			dG_left = ( ((s64)( g2 - g1 )) * r21 ) >> 24;
			dB_left = ( ((s64)( b2 - b1 )) * r21 ) >> 24;
		//}
	}
	else
	{
		x_right = ( ((s64)x1) << 16 );

		//U_right = ( ((s64)u1) << 24 );
		//V_right = ( ((s64)v1) << 24 );

		//R_right = ( ((s64)r1) << 24 );
		//G_right = ( ((s64)g1) << 24 );
		//B_right = ( ((s64)b1) << 24 );
		
		//if ( y2 - y1 )
		//{
			//dx_right = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			
			//dU_right = ( ((s64)( u2 - u1 )) * r21 ) >> 24;
			//dV_right = ( ((s64)( v2 - v1 )) * r21 ) >> 24;
			
			//dR_right = ( ((s64)( r2 - r1 )) * r21 ) >> 24;
			//dG_right = ( ((s64)( g2 - g1 )) * r21 ) >> 24;
			//dB_right = ( ((s64)( b2 - b1 )) * r21 ) >> 24;
		//}
	}
	
	// the line starts at y1 from here
	//Line = y1;

	StartY = y1;
	EndY = y2;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		
		if ( EndY < ((s32)DrawArea_TopLeftY) )
		{
			Temp = EndY - StartY;
			StartY = EndY;
		}
		else
		{
			Temp = DrawArea_TopLeftY - StartY;
			StartY = DrawArea_TopLeftY;
		}
		
		x_left += dx_left * Temp;
		x_right += dx_right * Temp;
		
		U_left += dU_left * Temp;
		V_left += dV_left * Temp;
		R_left += dR_left * Temp;
		G_left += dG_left * Temp;
		B_left += dB_left * Temp;
		
		//U_right += dU_right * Temp;
		//V_right += dV_right * Temp;
		//R_right += dR_right * Temp;
		//G_right += dG_right * Temp;
		//B_right += dB_right * Temp;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY + 1;
	}


	if ( EndY > StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y2
	for ( Line = StartY; Line < EndY; Line++ )
	{
		//StartX = _Round( x_left ) >> 32;
		//EndX = _Round( x_right ) >> 32;
		
		// left point is included if points are equal
		StartX = ( x_left + 0xffffLL ) >> 16;
		EndX = ( x_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			iU = U_left;
			iV = V_left;
			

			iR = R_left;
			iG = G_left;
			iB = B_left;

			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - x_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iU += dU_across * Temp;
				//iV += dV_across * Temp;
				
				//iR += dR_across * Temp;
				//iG += dG_across * Temp;
				//iB += dB_across * Temp;
			}
			
			iU += ( dU_across >> 12 ) * ( Temp >> 4 );
			iV += ( dV_across >> 12 ) * ( Temp >> 4 );
			
			iR += ( dR_across >> 12 ) * ( Temp >> 4 );
			iG += ( dG_across >> 12 ) * ( Temp >> 4 );
			iB += ( dB_across >> 12 ) * ( Temp >> 4 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			NumberOfPixelsDrawn += EndX - StartX + 1;
			
			if ( DTD )
			{
				DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			}
			
			// draw horizontal line
			// x_left and x_right need to be rounded off
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
					if ( DTD )
					{
						DitherValue = DitherLine [ x_across & 0x3 ];
					}

					//TexCoordY = (u8) ( ( ( _Round24 ( iV ) >> 24 ) & Not_TWH ) | TWYTWH );
					//TexCoordX = (u8) ( ( ( _Round24 ( iU ) >> 24 ) & Not_TWW ) | TWXTWW );
					TexCoordY = (u8) ( ( ( iV >> 24 ) & Not_TWH ) | TWYTWH );
					TexCoordX = (u8) ( ( ( iU >> 24 ) & Not_TWW ) | TWXTWW );
					
					// *** testing ***
					//debug << dec << "\r\nTexCoordX=" << TexCoordX << " TexCoordY=" << TexCoordY;
					
					switch ( TP )
					{
						case 0:
							pixel = ptr_texture [ ( TexCoordX >> 2 ) + ( TexCoordY << 10 ) ];
							pixel = ptr_clut [ ( clut_xoffset + ( ( pixel >> ( ( TexCoordX & 3 ) << 2 ) ) & 0xf ) ) & FrameBuffer_XMask ];
							break;
							
						case 1:
							pixel = ptr_texture [ ( TexCoordX >> 1 ) + ( TexCoordY << 10 ) ];
							pixel = ptr_clut [ ( clut_xoffset + ( ( pixel >> ( ( TexCoordX & 1 ) << 3 ) ) & 0xff ) ) & FrameBuffer_XMask ];
							break;
							
						case 2:
							pixel = ptr_texture [ ( TexCoordX ) + ( TexCoordY << 10 ) ];
							break;
					}
					
					
					if ( pixel )
					{
						if ( ABE || PIXELMASK )
						{
							// read pixel from frame buffer if we need to check mask bit
							DestPixel = *ptr;
						}

						if ( ABE || !TGE )
						{
							pixel_temp = pixel & 0x8000;
						}
				
						//if ( !tge )
						if ( !TGE )
						{
							// brightness calculation
							// shade pixel
							Red = ( ( (s16) ( iR >> 24 ) & 0xff ) * ( (s16) ( pixel & 0x1f ) ) );
							Green = ( ( (s16) ( iG >> 24 ) & 0xff ) * ( (s16) ( ( pixel >> 5 ) & 0x1f ) ) );
							Blue = ( ( (s16) ( iB >> 24 ) & 0xff ) * ( (s16) ( ( pixel >> 10 ) & 0x1f ) ) );
						
							if ( DTD )
							{
								// apply dithering if it is enabled
								// dithering must be applied after the color multiply
								Red = Red + DitherValue;
								Green = Green + DitherValue;
								Blue = Blue + DitherValue;
							}
							
							// clamp
							//Red = Clamp5 ( Red >> 7 );
							//Green = Clamp5 ( Green >> 7 );
							//Blue = Clamp5 ( Blue >> 7 );
							Red = SignedClamp<s16,5> ( Red >> 7 );
							Green = SignedClamp<s16,5> ( Green >> 7 );
							Blue = SignedClamp<s16,5> ( Blue >> 7 );
							
							// combine
							pixel = ( Blue << 10 ) | ( Green << 5 ) | ( Red );
						}
						
						// semi-transparency
						//if ( command_abe && ( bgr & 0x8000 ) )
						if ( ABE )
						{
							if ( pixel_temp )
							{
								pixel = SemiTransparency16_t <ABRCODE> ( DestPixel, pixel );
							}
						}
						
						if ( ABE || !TGE )
						{
							// check if we should set mask bit when drawing
							//bgr_temp |= SetPixelMask | ( bgr & 0x8000 );
							pixel |= pixel_temp;
						}
						
						if ( SETPIXELMASK )
						{
							// set pixel mask if specified
							pixel |= SETPIXELMASK;
						}

						// draw pixel if we can draw to mask pixels or mask bit not set
						if ( PIXELMASK )
						{
							if ( ! ( DestPixel & PIXELMASK ) ) *ptr = pixel;
						}
						else
						{
							*ptr = pixel;
						}
					}
				
				/////////////////////////////////////////////////////////
				// interpolate texture coords
				iU += dU_across;
				iV += dV_across;
				
				iR += dR_across;
				iG += dG_across;
				iB += dB_across;
				
				ptr += c_iVectorSize;
			}
			
		}
		
		
		/////////////////////////////////////
		// update x on left and right
		x_left += dx_left;
		x_right += dx_right;
		
		U_left += dU_left;
		V_left += dV_left;
		//U_right += dU_right;
		//V_right += dV_right;
		
		R_left += dR_left;
		G_left += dG_left;
		B_left += dB_left;
		//R_right += dR_right;
		//G_right += dG_right;
		//B_right += dB_right;
	}
	
	}

}




template<const long PIXELMASK, const long SETPIXELMASK, const long ABE, const long ABRCODE, const long TGE, const long TP>
inline void GPU::SelectTriangle_TextureGradient5_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	//debug << "SelectTriangle_Texture5_t->";
	
	if ( GPU_CTRL_Read.DTD )
	{
		DrawTriangle_TextureGradient_t <PIXELMASK, SETPIXELMASK, ABE, ABRCODE, TGE, TP,1> ( Coord0, Coord1, Coord2 );
	}
	else
	{
		DrawTriangle_TextureGradient_t <PIXELMASK, SETPIXELMASK, ABE, ABRCODE, TGE, TP,0> ( Coord0, Coord1, Coord2 );
	}
}



template<const long PIXELMASK, const long ABE, const long ABRCODE, const long TGE, const long TP>
inline void GPU::SelectTriangle_TextureGradient4_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	//debug << "SelectTriangle_Texture4_t->";
	
	if ( GPU_CTRL_Read.MD )
	{
		SelectTriangle_TextureGradient5_t <PIXELMASK, 0x8000, ABE, ABRCODE, TGE, TP> ( Coord0, Coord1, Coord2 );
	}
	else
	{
		SelectTriangle_TextureGradient5_t <PIXELMASK, 0, ABE, ABRCODE, TGE, TP> ( Coord0, Coord1, Coord2 );
	}
}


template<const long ABE, const long ABRCODE, const long TGE, const long TP>
inline void GPU::SelectTriangle_TextureGradient3_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	//debug << "SelectTriangle_Texture3_t->";
	
	if ( GPU_CTRL_Read.ME )
	{
		SelectTriangle_TextureGradient4_t<0x8000, ABE, ABRCODE, TGE, TP> ( Coord0, Coord1, Coord2 );
	}
	else
	{
		SelectTriangle_TextureGradient4_t<0, ABE, ABRCODE, TGE, TP> ( Coord0, Coord1, Coord2 );
	}
}


template<const long ABE, const long ABRCODE, const long TGE>
inline void GPU::SelectTriangle_TextureGradient2_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	//debug << "SelectTriangle_Texture2_t->";
	
	switch ( tpage_tp )
	{
		case 0:
			SelectTriangle_TextureGradient3_t<ABE, ABRCODE, TGE, 0> ( Coord0, Coord1, Coord2 );
			break;
			
		case 1:
			SelectTriangle_TextureGradient3_t<ABE, ABRCODE, TGE, 1> ( Coord0, Coord1, Coord2 );
			break;
			
		case 2:
			SelectTriangle_TextureGradient3_t<ABE, ABRCODE, TGE, 2> ( Coord0, Coord1, Coord2 );
			break;
	}
}


template<const long ABE, const long TGE>
void GPU::SelectTriangle_TextureGradient_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	//debug << "SelectTriangle_Texture_t->";
	
	switch ( tpage_abr )
	{
		case 0:
			SelectTriangle_TextureGradient2_t<ABE, 0, TGE> ( Coord0, Coord1, Coord2 );
			break;
			
		case 1:
			SelectTriangle_TextureGradient2_t<ABE, 1, TGE> ( Coord0, Coord1, Coord2 );
			break;
			
		case 2:
			SelectTriangle_TextureGradient2_t<ABE, 2, TGE> ( Coord0, Coord1, Coord2 );
			break;
			
		case 3:
			SelectTriangle_TextureGradient2_t<ABE, 3, TGE> ( Coord0, Coord1, Coord2 );
			break;
	}
}



template<const long ABE, const long TGE>
void GPU::Draw_TextureGradientTriangle_34_t ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
	//u32 tge;
	//tge = command_tge;
	
	//if ( ( bgr0 & 0x00ffffff ) == ( bgr1 & 0x00ffffff ) && ( bgr0 & 0x00ffffff ) == ( bgr2 & 0x00ffffff ) )
	if ( gbgr [ Coord0 ] == gbgr [ Coord1 ] && gbgr [ Coord0 ] == gbgr [ Coord2 ] )
	{
		//if ( ( bgr0 & 0x00ffffff ) == 0x00808080 )
		if ( gbgr [ Coord0 ] == 0x00808080 )
		{
			//command_tge = 1;
			SelectTriangle_Texture_t <ABE,1> ( Coord0, Coord1, Coord2 );
		}
		else
		{
			//bgr = bgr0;
			gbgr [ 0 ] = gbgr [ Coord0 ];
			SelectTriangle_Texture_t <ABE,TGE> ( Coord0, Coord1, Coord2 );
		}
		
		//DrawTriangle_Texture ();
	}
	else
	{
		if ( TGE )
		{
			SelectTriangle_Texture_t <ABE,TGE> ( Coord0, Coord1, Coord2 );
		}
		else
		{
			SelectTriangle_TextureGradient_t <ABE,TGE> ( Coord0, Coord1, Coord2 );
		}
	}
	
	// restore tge
	//command_tge = tge;

	
	// check for alpha blending - add in an extra cycle for this for now
	if ( ABE )
	{
		BusyCycles += NumberOfPixelsDrawn * dAlphaBlending_CyclesPerPixel;
	}
	
	// check for brightness calculation
	if ( !TGE )
	{
		BusyCycles += NumberOfPixelsDrawn * dBrightnessCalculation_CyclesPerPixel;
	}
	
	switch ( tpage_tp )
	{
		case 0:		// 4-bit clut
			BusyCycles += NumberOfPixelsDrawn * dTextureTriangle4_34_Gradient_CyclesPerPixel;	//dTextureTriangle4_CyclesPerPixel;
			break;
			
		case 1:		// 8-bit clut
			BusyCycles += NumberOfPixelsDrawn * dTextureTriangle8_34_Gradient_CyclesPerPixel;	//dTextureTriangle8_CyclesPerPixel;
			break;
			
		case 2:		// 15-bit color
			BusyCycles += NumberOfPixelsDrawn * dTextureTriangle16_34_Gradient_CyclesPerPixel;	//dTextureTriangle16_CyclesPerPixel;
			break;
	}
	
#ifdef ENABLE_DRAW_OVERHEAD
	BusyCycles += DrawOverhead_Cycles;
#endif
}


template<const long ABE, const long TGE>
void GPU::Draw_TextureGradientRectangle_3c_t ()
{
	//x_save [ 0 ] = x0; x_save [ 1 ] = x1; x_save [ 2 ] = x2; x_save [ 3 ] = x3;
	//y_save [ 0 ] = y0; y_save [ 1 ] = y1; y_save [ 2 ] = y2; y_save [ 3 ] = y3;
	//bgr_save [ 0 ] = bgr; bgr_save [ 1 ] = bgr0; bgr_save [ 2 ] = bgr1; bgr_save [ 3 ] = bgr2; bgr_save [ 4 ] = bgr3;
	//u_save [ 0 ] = u0; u_save [ 1 ] = u1; u_save [ 2 ] = u2; u_save [ 3 ] = u3;
	//v_save [ 0 ] = v0; v_save [ 1 ] = v1; v_save [ 2 ] = v2; v_save [ 3 ] = v3;
	
	//bgr = 0x808080;
	
	//Draw_TextureGradientTriangle_34 ();
	Draw_TextureGradientTriangle_34_t <ABE,TGE> ( 0, 1, 2 );
	
	//x0 = x_save [ 1 ];
	//y0 = y_save [ 1 ];
	//x1 = x_save [ 2 ];
	//y1 = y_save [ 2 ];
	//x2 = x_save [ 3 ];
	//y2 = y_save [ 3 ];


	//bgr = 0x808080;
	//bgr0 = bgr_save [ 2 ];
	//bgr1 = bgr_save [ 3 ];
	//bgr2 = bgr_save [ 4 ];
	
	//u0 = u_save [ 1 ];
	//v0 = v_save [ 1 ];
	//u1 = u_save [ 2 ];
	//v1 = v_save [ 2 ];
	//u2 = u_save [ 3 ];
	//v2 = v_save [ 3 ];
	
	//Draw_TextureGradientTriangle_34 ();
	Draw_TextureGradientTriangle_34_t <ABE,TGE> ( 1, 2, 3 );
}






// *** Template Functions *** //

//using namespace Playstation1;



template<const long ABE, const long TGE>
void GPU::SelectSprite_t ()
{
	switch ( GPU_CTRL_Read.ABR )
	{
		case 0:
			SelectSprite2_t<ABE, 0, TGE> ();
			break;
			
		case 1:
			SelectSprite2_t<ABE, 1, TGE> ();
			break;
			
		case 2:
			SelectSprite2_t<ABE, 2, TGE> ();
			break;
			
		case 3:
			SelectSprite2_t<ABE, 3, TGE> ();
			break;
	}
}


template<const long ABE, const long TGE>
void GPU::Draw_Sprite_64_t ()
{
	tpage_tx = GPU_CTRL_Read.TX;
	tpage_ty = GPU_CTRL_Read.TY;
	tpage_tp = GPU_CTRL_Read.TP;
	
	SelectSprite_t <ABE,TGE> ();

	// set number of cycles it takes to draw sprite
	switch ( tpage_tp )
	{
		case 0:		// 4-bit clut
			BusyCycles = NumberOfPixelsDrawn * dSprite4_64_Cycles;
			break;
			
		case 1:		// 8-bit clut
			BusyCycles = NumberOfPixelsDrawn * dSprite8_64_Cycles;
			break;
			
		case 2:		// 15-bit color
			BusyCycles = NumberOfPixelsDrawn * dSprite16_64_Cycles;
			break;
	}

}



template<const long ABE, const long TGE>
void GPU::Draw_Sprite8x8_74_t ()
{
	static const u32 SpriteSize = 8;
	
	tpage_tx = GPU_CTRL_Read.TX;
	tpage_ty = GPU_CTRL_Read.TY;
	tpage_tp = GPU_CTRL_Read.TP;
	//tpage_abr = GPU_CTRL_Read.ABR;
	
	w = 8; h = 8;
	SelectSprite_t <ABE,TGE> ();

	// set number of cycles it takes to draw 8x8 sprite
	switch ( tpage_tp )
	{
		case 0:		// 4-bit clut
			BusyCycles = dCyclesPerSprite8x8_74_4bit;	//121;
			break;
			
		case 1:		// 8-bit clut
			BusyCycles = dCyclesPerSprite8x8_74_8bit;	//121;
			break;
			
		case 2:		// 15-bit color
			BusyCycles = dCyclesPerSprite8x8_74_16bit;	//212;
			break;
	}


}



template<const long ABE, const long TGE>
void GPU::Draw_Sprite16x16_7c_t ()
{
	static const u32 SpriteSize = 16;
	
	tpage_tx = GPU_CTRL_Read.TX;
	tpage_ty = GPU_CTRL_Read.TY;
	tpage_tp = GPU_CTRL_Read.TP;
	//tpage_abr = GPU_CTRL_Read.ABR;
	
	w = 16; h = 16;
	SelectSprite_t <ABE,TGE> ();

	// set number of cycles it takes to draw 16x16 sprite
	switch ( tpage_tp )
	{
		case 0:		// 4-bit clut
			BusyCycles = dCyclesPerSprite16x16_7c_4bit;	//308;
			break;
			
		case 1:		// 8-bit clut
			BusyCycles = dCyclesPerSprite16x16_7c_8bit;	//484;
			break;
			
		case 2:		// 15-bit color
			BusyCycles = dCyclesPerSprite16x16_7c_16bit;	//847;
			break;
	}


}



template<const long PIXELMASK, const long SETPIXELMASK, const long ABE, const long ABRCODE>
void GPU::Draw_MonoLine_t ()
{
	static const double dMonoLine_40_CyclesPerPixel = 1.0L;

	s32 distance, x_distance, y_distance;
	
	//s64 rc;
	
	u16 *ptr16;
	
	s64 dxdc;
	s64 dydc;
	s64 line_x0, line_y0;
	s64 line_x1, line_y1;
	
	s64 line_x, line_y;
	
	s64 Temp, Temp0, Temp1;
	s64 dx, dy;
	
	s64 TestTop, TestBottom, TestLeft, TestRight;
	
	s64 dxdy, dydx;
	
	//s64 Start, End;

	// new local variables
	s64 x, y, x0, x1, y0, y1;
	u32 bgr, bgr2;
	s64 iX, iY;
	
	u32 DestPixel;
	//u32 PixelMask = 0, SetPixelMask = 0;
	
	NumberOfPixelsDrawn = 0;

	/*
	if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
	*/
	
	// get color(s)
	bgr = gbgr [ 0 ];
	
	// ?? convert to 16-bit ?? (or should leave 24-bit?)
	bgr = ( ( bgr & ( 0xf8 << 0 ) ) >> 3 ) | ( ( bgr & ( 0xf8 << 8 ) ) >> 6 ) | ( ( bgr & ( 0xf8 << 16 ) ) >> 9 );
	
	// get x-values
	x0 = gx [ 0 ];
	x1 = gx [ 1 ];
	
	// get y-values
	y0 = gy [ 0 ];
	y1 = gy [ 1 ];
	
	//////////////////////////////////////////
	// get coordinates on screen
	x0 = DrawArea_OffsetX + x0;
	y0 = DrawArea_OffsetY + y0;
	x1 = DrawArea_OffsetX + x1;
	y1 = DrawArea_OffsetY + y1;

	// make sure line might be on screen
	if ( ( ( x1 <= ((s32)DrawArea_TopLeftX) ) && ( x0 < ((s32)DrawArea_TopLeftX) ) ) ||
		( ( x1 >= ((s32)DrawArea_BottomRightX) ) && ( x0 > ((s32)DrawArea_BottomRightX) ) ) ||
		( ( y1 <= ((s32)DrawArea_TopLeftY) ) && ( y0 < ((s32)DrawArea_TopLeftY) ) ) ||
		( ( y1 >= ((s32)DrawArea_BottomRightY) ) && ( y0 > ((s32)DrawArea_BottomRightY) ) ) ) return;
		
	// get size of line
	x_distance = _Abs( x1 - x0 );
	y_distance = _Abs( y1 - y0 );
	//if ( x_distance > y_distance ) distance = x_distance; else distance = y_distance;

	// if we don't draw the final point, then a distance of zero does not get drawn
	// if both x and y distance are zero, then do not draw
	//if ( !distance ) return;
	if ( ! ( x_distance | y_distance ) ) return;
	
	// x.48 fixed point
	if ( x_distance ) dx = ( 1LL << 32 ) / x_distance; else dx = 0;
	if ( y_distance ) dy = ( 1LL << 32 ) / y_distance; else dy = 0;
	
	// x.48 fixed point
	dxdy = ( ( (s64) ( x1 - x0 ) ) * dy );
	dydx = ( ( (s64) ( y1 - y0 ) ) * dx );
	
	
	
	// close in from the starting point towards ending point
	
	// close in from the ending point towards starting point
	

	
	
	line_x0 = ( x0 << 16 );
	line_y0 = ( y0 << 16 );
	
	line_x1 = ( x1 << 16 );
	line_y1 = ( y1 << 16 );

	// initial fractional part
	line_x0 |= 0x8000;
	line_y0 |= 0x8000;
	line_x1 |= 0x8000;
	line_y1 |= 0x8000;


	
	// get window for the end point
	TestLeft = ( DrawArea_TopLeftX << 16 );
	TestRight = ( DrawArea_BottomRightX << 16 ) | 0xffff;
	//TestRight = ( ( DrawArea_BottomRightX + 1 ) << 16 );
	TestTop = ( DrawArea_TopLeftY << 16 );
	TestBottom = ( DrawArea_BottomRightY << 16 ) | 0xffff;
	//TestBottom = ( ( DrawArea_BottomRightY + 1 ) << 16 );
	
	
	// if start coord is off to the left or right, then fix
	if ( line_x0 < TestLeft && x1 > x0 )
	{
		// line must be headed to the right (dx positive)
		// but would have to be headed to the right or would not have reached here
		//if ( x1 <= x0 ) return;
		
		Temp = TestLeft - line_x0;
		line_y0 += ( ( Temp * dydx ) >> 32 );
		line_x0 = TestLeft;
	}
	else if ( line_x0 > TestRight && x1 < x0 )
	{
		// line must be headed to the left (dx negative)
		// but would have to be headed to the left or would not have reached here
		//if ( x1 >= x0 ) return;
		
		//Temp = TestRight - line_x0;
		Temp = line_x0 - TestRight;
		line_y0 += ( ( Temp * dydx ) >> 32 );
		line_x0 = TestRight;
	}
	

	// if start coord is off to the top or bottom, then fix
	if ( line_y0 < TestTop && y1 > y0 )
	{
		// line must be headed down (dy positive)
		// but would have to be headed down or would not have reached here
		//if ( y1 <= y0 ) return;
		
		Temp = TestTop - line_y0;
		line_x0 += ( ( Temp * dxdy ) >> 32 );
		line_y0 = TestTop;
	}
	else if ( line_y0 > TestBottom && y1 < y0 )
	{
		// line must be headed up (dy negative)
		// but would have to be headed up or would not have reached here
		//if ( y1 >= y0 ) return;
		
		//Temp = TestBottom - line_y0;
		Temp = line_y0 - TestBottom;
		line_x0 += ( ( Temp * dxdy ) >> 32 );
		line_y0 = TestBottom;
	}
	
	// round to nearest
	//line_x += 0x8000;
	//line_y += 0x8000;
	
	// make sure starting point is within draw window
	//x = line_x >> 16;
	//y = line_y >> 16;
	//if ( ( x < ((s32)DrawArea_TopLeftX) ) || ( x > ((s32)DrawArea_BottomRightX) ) || ( y < ((s32)DrawArea_TopLeftY) ) || ( y > ((s32)DrawArea_BottomRightY) ) ) return;
	// but also need to check again later, because could still be offscreen
	if ( ( line_x0 < TestLeft ) || ( line_x0 > TestRight ) || ( line_y0 < TestTop ) || ( line_y0 > TestBottom ) ) return;

	
	// if line endpoint is off, then fix
	// just need to trace back to exact point exact line is out of window
	if ( line_x1 < TestLeft )
	{
		//Temp = TestLeft - line_x1;
		Temp = line_x1 - TestLeft;
		line_y1 += ( ( Temp * dydx ) >> 32 );
		line_x1 = TestLeft;
	}
	else if ( line_x1 > TestRight )
	{
		Temp = TestRight - line_x1;
		line_y1 += ( ( Temp * dydx ) >> 32 );
		line_x1 = TestRight;
	}
	

	if ( line_y1 < TestTop )
	{
		//Temp = TestTop - line_y1;
		Temp = line_y1 - TestTop;
		line_x1 += ( ( Temp * dxdy ) >> 32 );
		line_y1 = TestTop;
	}
	else if ( line_y1 > TestBottom )
	{
		Temp = TestBottom - line_y1;
		line_x1 += ( ( Temp * dxdy ) >> 32 );
		line_y1 = TestBottom;
	}
	
	

	// get final distance to draw
	if ( x_distance > y_distance )
	{
		// round to nearest
		//line_x2 += 0x8000;
		
		// set dxdc,dydc
		if ( x1 > x0 ) dxdc = ( 1 << 16 ); else dxdc = ( -1 << 16 );
		
		// x.16 fixed point
		dydc = ( dydx >> 16 );
		
		// check if pixel is off 
		
		if ( x1 > x0 )
		{
			// line is going to right //
			
			// make sure start x is at .5 interval
			line_x = ( ( line_x0 + 0x7fff ) | 0x8000 ) & ~0x7fff;
			
			// make sure end x is at .5 interval
			line_y = ( ( line_x1 - 0x8000 ) | 0x8000 ) & ~0x7fff;
			
			// get difference
			Temp0 = line_x - line_x0;
			
			// get difference
			//Temp = line_x1 - line_y;
			Temp1 = line_y - line_x1;
		}
		else
		{
			// line is going to left //
			
			// make sure end x is at .5 interval
			line_x = ( ( line_x0 - 0x8000 ) | 0x8000 ) & ~0x7fff;
			
			// make sure start x is at .5 interval
			line_y = ( ( line_x1 + 0x7fff ) | 0x8000 ) & ~0x7fff;
			
			// get difference
			Temp0 = line_x0 - line_x;
			//Temp0 = line_x - line_x0;
			
			// get difference
			Temp1 = line_x1 - line_y;
			//Temp1 = line_y - line_x1;
		}

		
		// update y
		line_y0 += ( Temp0 * dydx ) >> 32;
		line_x0 = line_x;
		
		// update y
		line_y1 += ( Temp1 * dydx ) >> 32;
		line_x1 = line_y;
		
		// distance is in x direction
		distance = _Abs ( ( line_x0 >> 16 ) - ( line_x1 >> 16 ) );
		
		// can calculate rgb start here using drdc,dgdc,dbdc
	}
	else
	{
		// round to nearest
		//line_y2 += 0x8000;
		
		// set dxdc,dydc
		if ( y1 > y0 ) dydc = ( 1 << 16 ); else dydc = ( -1 << 16 );
		
		// x.16 fixed point
		dxdc = ( dxdy >> 16 );
		
		if ( y1 > y0 )
		{
			// line is going to bottom //
			
			// make sure start y is at .5 interval
			line_x = ( ( line_y0 + 0x7fff ) | 0x8000 ) & ~0x7fff;
			
			// make sure end y is at .5 interval
			line_y = ( ( line_y1 - 0x8000 ) | 0x8000 ) & ~0x7fff;
			
			// get difference
			Temp0 = line_x - line_y0;
			
			// get difference
			//Temp = line_x1 - line_y;
			Temp1 = line_y - line_y1;
		}
		else
		{
			// line is going to top //
			
			// make sure start y is at .5 interval
			line_x = ( ( line_y0 - 0x8000 ) | 0x8000 ) & ~0x7fff;
			
			// make sure end y is at .5 interval
			line_y = ( ( line_y1 + 0x7fff ) | 0x8000 ) & ~0x7fff;
			
			// get difference
			Temp0 = line_y0 - line_x;
			//Temp0 = line_x - line_x0;
			
			// get difference
			Temp1 = line_y1 - line_y;
			//Temp1 = line_y - line_x1;
		}

		// update x
		line_x0 += ( Temp0 * dxdy ) >> 32;
		line_y0 = line_x;
		
		// update x
		line_x1 += ( Temp1 * dxdy ) >> 32;
		line_y1 = line_y;
		
		// distance is in y direction
		distance = _Abs ( ( line_y0 >> 16 ) - ( line_y1 >> 16 ) );
		
		// can calculate rgb start here using drdc,dgdc,dbdc
	}

	
	
	// if the last point on screen is not the endpoint for entire line, then include it
	if ( x1 != ( line_x1 >> 16 ) || y1 != ( line_y1 >> 16 ) ) distance++;


	// if there is nothing to draw, then done
	if ( !distance ) return;
	
	// save the color
	bgr2 = bgr;
	
	// set pixel mask bit if needed
	if ( SETPIXELMASK )
	{
		// set pixel mask if specified
		bgr |= SETPIXELMASK;
	}
	
	// draw the line
	// note: the final point should probably not be drawn
	//for ( u32 i = 0; i <= distance; i++ )
	for ( u32 i = 0; i < distance; i++ )
	{
		// get x coord
		//iX = ( _Round( line_x ) >> 32 );
		iX = ( line_x0 >> 16 );
		
		// get y coord
		//iY = ( _Round( line_y ) >> 32 );
		iY = ( line_y0 >> 16 );
		
			
			ptr16 = & ( VRAM [ iX + ( iY << 10 ) ] );
			
			if ( ABE || PIXELMASK )
			{
				// read pixel from frame buffer if we need to check mask bit
				DestPixel = *ptr16;
			}
		
			// semi-transparency
			//if ( command_abe )
			if ( ABE )
			{
				//bgr2 = SemiTransparency16 ( DestPixel, bgr2, GPU_CTRL_Read.ABR );
				bgr = SemiTransparency16_t<ABRCODE> ( DestPixel, bgr2 );
				
				if ( SETPIXELMASK )
				{
					// set pixel mask if specified
					bgr |= SETPIXELMASK;
				}
			}
				
			// draw point
			
			// check if we should set mask bit when drawing
			//if ( GPU_CTRL_Read.MD ) bgr2 |= 0x8000;

			// draw pixel if we can draw to mask pixels or mask bit not set
			//if ( ! ( DestPixel & PIXELMASK ) ) *ptr16 = ( bgr2 | SETPIXELMASK );

			// draw pixel if we can draw to mask pixels or mask bit not set
			if ( PIXELMASK )
			{
				if ( ! ( DestPixel & PIXELMASK ) ) *ptr16 = bgr;
			}
			else
			{
				*ptr16 = bgr;
			}
			
			
		
		line_x0 += dxdc;
		line_y0 += dydc;
	}
	
	NumberOfPixelsDrawn = distance;
	BusyCycles += NumberOfPixelsDrawn * dMonoLine_40_CyclesPerPixel;
	
#ifdef ENABLE_DRAW_OVERHEAD
	BusyCycles += DrawOverhead_Cycles;
#endif
}


template<const long PIXELMASK, const long ABE, const long ABRCODE>
inline void GPU::Select_MonoLine3_t ()
{
	if ( GPU_CTRL_Read.MD )
	{
		Draw_MonoLine_t <PIXELMASK, 0x8000, ABE, ABRCODE> ();
	}
	else
	{
		Draw_MonoLine_t <PIXELMASK, 0, ABE, ABRCODE> ();
	}
}


template<const long ABE, const long ABRCODE>
inline void GPU::Select_MonoLine2_t ()
{
	if ( GPU_CTRL_Read.ME )
	{
		Select_MonoLine3_t<0x8000, ABE, ABRCODE> ();
	}
	else
	{
		Select_MonoLine3_t<0, ABE, ABRCODE> ();
	}
}

template<const long ABE>
void GPU::Select_MonoLine_t ()
{
	switch ( GPU_CTRL_Read.ABR )
	{
		case 0:
			Select_MonoLine2_t<ABE, 0> ();
			break;
			
		case 1:
			Select_MonoLine2_t<ABE, 1> ();
			break;
			
		case 2:
			Select_MonoLine2_t<ABE, 2> ();
			break;
			
		case 3:
			Select_MonoLine2_t<ABE, 3> ();
			break;
	}
}



template<const long PIXELMASK, const long SETPIXELMASK, const long ABE, const long ABRCODE,const long DTD>
void GPU::Draw_ShadedLine_t ()
{
	static const double dShadedLine_50_CyclesPerPixel = 2.0L;

	s32 distance, x_distance, y_distance;
	
	s64 dx;
	s64 dy;
	
	s64 dxdy, dydx;
	
	s64 dxdc;
	s64 dydc;
	s64 drdc, dgdc, dbdc;
	s64 line_x, line_y;
	s64 line_r, line_g, line_b;
	
	s64 line_x0, line_y0;
	s64 line_x1, line_y1;
	
	s64 TestTop, TestBottom, TestLeft, TestRight;
	
	s64 Temp, Temp0, Temp1, TempX;
	
	//s64 rc;
	u16* ptr16;
	
	// new local variables
	s32 x0, x1, y0, y1;
	u32 bgr;
	s64 iX, iY;
	s32 r0, r1, g0, g1, b0, b1;
	
	u32 DestPixel;
	//u32 PixelMask = 0, SetPixelMask = 0;
	
	s64* DitherArray;
	s64 DitherValue;
	
	
	s64 Red, Green, Blue;
	s64 iR, iG, iB;

	
	if ( DTD )
	{
		DitherArray = c_iDitherValues24;
	}
	
	
	NumberOfPixelsDrawn = 0;

	/*
	if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
	*/
	
	// get the x-values
	x0 = gx [ 0 ];
	x1 = gx [ 1 ];
	
	// get the y-values
	y0 = gy [ 0 ];
	y1 = gy [ 1 ];
	
	// get color components
	//r0 = bgr0 & 0xff;
	//r1 = bgr1 & 0xff;
	//g0 = ( bgr0 >> 8 ) & 0xff;
	//g1 = ( bgr1 >> 8 ) & 0xff;
	//b0 = ( bgr0 >> 16 ) & 0xff;
	//b1 = ( bgr1 >> 16 ) & 0xff;
	r0 = gr [ 0 ];
	r1 = gr [ 1 ];
	g0 = gg [ 0 ];
	g1 = gg [ 1 ];
	b0 = gb [ 0 ];
	b1 = gb [ 1 ];
	
	line_r = r0 << 16;
	line_g = g0 << 16;
	line_b = b0 << 16;
	
	// go ahead and add the intial fractional part
	line_r |= 0x8000;
	line_g |= 0x8000;
	line_b |= 0x8000;
	
	//////////////////////////////////////////
	// get coordinates on screen
	x0 = DrawArea_OffsetX + x0;
	y0 = DrawArea_OffsetY + y0;
	x1 = DrawArea_OffsetX + x1;
	y1 = DrawArea_OffsetY + y1;
	
	
	// make sure line might be on screen
	if ( ( ( x1 <= ((s32)DrawArea_TopLeftX) ) && ( x0 < ((s32)DrawArea_TopLeftX) ) ) ||
		( ( x1 >= ((s32)DrawArea_BottomRightX) ) && ( x0 > ((s32)DrawArea_BottomRightX) ) ) ||
		( ( y1 <= ((s32)DrawArea_TopLeftY) ) && ( y0 < ((s32)DrawArea_TopLeftY) ) ) ||
		( ( y1 >= ((s32)DrawArea_BottomRightY) ) && ( y0 > ((s32)DrawArea_BottomRightY) ) ) ) return;

	// get size of line
	x_distance = _Abs( x1 - x0 );
	y_distance = _Abs( y1 - y0 );
	//if ( x_distance > y_distance ) distance = x_distance; else distance = y_distance;

	// if we don't draw the final point, then a distance of zero does not get drawn
	// if both x and y distance are zero, then do not draw
	//if ( !distance ) return;
	if ( ! ( x_distance | y_distance ) ) return;
	
	// x.48 fixed point
	if ( x_distance ) dx = ( 1LL << 32 ) / x_distance; else dx = 0;
	if ( y_distance ) dy = ( 1LL << 32 ) / y_distance; else dy = 0;
	
	// x.32 fixed point
	dxdy = ( ( (s64) ( x1 - x0 ) ) * dy );
	dydx = ( ( (s64) ( y1 - y0 ) ) * dx );

	//drdx = ( ( (s64) ( r1 - r0 ) ) * dx ) >> 16;
	//dgdx = ( ( (s64) ( g1 - g0 ) ) * dx ) >> 16;
	//dbdx = ( ( (s64) ( b1 - b0 ) ) * dx ) >> 16;

	//drdy = ( ( (s64) ( r1 - r0 ) ) * dy ) >> 16;
	//dgdy = ( ( (s64) ( g1 - g0 ) ) * dy ) >> 16;
	//dbdy = ( ( (s64) ( b1 - b0 ) ) * dy ) >> 16;
	
	
	line_x0 = ( x0 << 16 );
	line_y0 = ( y0 << 16 );
	
	line_x1 = ( x1 << 16 );
	line_y1 = ( y1 << 16 );

	// initial fractional part
	line_x0 |= 0x8000;
	line_y0 |= 0x8000;
	line_x1 |= 0x8000;
	line_y1 |= 0x8000;
	
	
	// get window for the end point
	TestLeft = ( DrawArea_TopLeftX << 16 );
	TestRight = ( DrawArea_BottomRightX << 16 ) | 0xffff;
	//TestRight = ( ( DrawArea_BottomRightX + 1 ) << 16 );
	TestTop = ( DrawArea_TopLeftY << 16 );
	TestBottom = ( DrawArea_BottomRightY << 16 ) | 0xffff;
	//TestBottom = ( ( DrawArea_BottomRightY + 1 ) << 16 );
	
	
	// if start coord is off to the left or right, then fix
	if ( line_x0 < TestLeft && x1 > x0 )
	{
		// line must be headed to the right (dx positive)
		// but would have to be headed to the right or would not have reached here
		//if ( x1 <= x0 ) return;
		
		Temp = TestLeft - line_x0;
		line_y0 += ( ( Temp * dydx ) >> 32 );
		line_x0 = TestLeft;
	}
	else if ( line_x0 > TestRight && x1 < x0 )
	{
		// line must be headed to the left (dx negative)
		// but would have to be headed to the left or would not have reached here
		//if ( x1 >= x0 ) return;
		
		//Temp = TestRight - line_x0;
		Temp = line_x0 - TestRight;
		line_y0 += ( ( Temp * dydx ) >> 32 );
		line_x0 = TestRight;
	}
	

	// if start coord is off to the top or bottom, then fix
	if ( line_y0 < TestTop && y1 > y0 )
	{
		// line must be headed down (dy positive)
		// but would have to be headed down or would not have reached here
		//if ( y1 <= y0 ) return;
		
		Temp = TestTop - line_y0;
		line_x0 += ( ( Temp * dxdy ) >> 32 );
		line_y0 = TestTop;
	}
	else if ( line_y0 > TestBottom && y1 < y0 )
	{
		// line must be headed up (dy negative)
		// but would have to be headed up or would not have reached here
		//if ( y1 >= y0 ) return;
		
		//Temp = TestBottom - line_y0;
		Temp = line_y0 - TestBottom;
		line_x0 += ( ( Temp * dxdy ) >> 32 );
		line_y0 = TestBottom;
	}
	
	// round to nearest
	//line_x += 0x8000;
	//line_y += 0x8000;
	
	// make sure starting point is within draw window
	//x = line_x >> 16;
	//y = line_y >> 16;
	//if ( ( x < ((s32)DrawArea_TopLeftX) ) || ( x > ((s32)DrawArea_BottomRightX) ) || ( y < ((s32)DrawArea_TopLeftY) ) || ( y > ((s32)DrawArea_BottomRightY) ) ) return;
	// but also need to check again later, because could still be offscreen
	if ( ( line_x0 < TestLeft ) || ( line_x0 > TestRight ) || ( line_y0 < TestTop ) || ( line_y0 > TestBottom ) ) return;

	
	// if line endpoint is off, then fix
	// just need to trace back to exact point exact line is out of window
	if ( line_x1 < TestLeft )
	{
		//Temp = TestLeft - line_x1;
		Temp = line_x1 - TestLeft;
		line_y1 += ( ( Temp * dydx ) >> 32 );
		line_x1 = TestLeft;
	}
	else if ( line_x1 > TestRight )
	{
		Temp = TestRight - line_x1;
		line_y1 += ( ( Temp * dydx ) >> 32 );
		line_x1 = TestRight;
	}
	

	if ( line_y1 < TestTop )
	{
		//Temp = TestTop - line_y1;
		Temp = line_y1 - TestTop;
		line_x1 += ( ( Temp * dxdy ) >> 32 );
		line_y1 = TestTop;
	}
	else if ( line_y1 > TestBottom )
	{
		Temp = TestBottom - line_y1;
		line_x1 += ( ( Temp * dxdy ) >> 32 );
		line_y1 = TestBottom;
	}
	
	// set end point to draw to
	//line_x1 = line_x;
	//line_y1 = line_y;
	

	// get final distance to draw
	if ( x_distance > y_distance )
	{
		// round to nearest
		//line_x2 += 0x8000;
		
		// set dxdc,dydc
		if ( x1 > x0 ) dxdc = ( 1 << 16 ); else dxdc = ( -1 << 16 );
		
		// x.16 fixed point
		dydc = ( dydx >> 16 );
		
		// color values
		drdc = ( ( r1 - r0 ) * dx ) >> 16;
		dgdc = ( ( g1 - g0 ) * dx ) >> 16;
		dbdc = ( ( b1 - b0 ) * dx ) >> 16;
		
		// check if pixel is off 
		
		if ( x1 > x0 )
		{
			// line is going to right //
			
			// make sure start x is at .5 interval
			line_x = ( ( line_x0 + 0x7fff ) | 0x8000 ) & ~0x7fff;
			
			// make sure end x is at .5 interval
			line_y = ( ( line_x1 - 0x8000 ) | 0x8000 ) & ~0x7fff;
			
			// get difference
			Temp0 = line_x - line_x0;
			
			// get difference
			//Temp = line_x1 - line_y;
			Temp1 = line_y - line_x1;
			
			// get pixels since start of line
			TempX = ( line_x >> 16 ) - x0;
		}
		else
		{
			// line is going to left //
			
			// make sure end x is at .5 interval
			line_x = ( ( line_x0 - 0x8000 ) | 0x8000 ) & ~0x7fff;
			
			// make sure start x is at .5 interval
			line_y = ( ( line_x1 + 0x7fff ) | 0x8000 ) & ~0x7fff;
			
			// get difference
			Temp0 = line_x0 - line_x;
			//Temp0 = line_x - line_x0;
			
			// get difference
			Temp1 = line_x1 - line_y;
			//Temp1 = line_y - line_x1;
			
			// get pixels since start of line
			TempX = x0 - ( line_x >> 16 );
		}

		
		// update y
		line_y0 += ( Temp0 * dydx ) >> 32;
		line_x0 = line_x;
		
		// update y
		line_y1 += ( Temp1 * dydx ) >> 32;
		line_x1 = line_y;
		
		// distance is in x direction
		distance = _Abs ( ( line_x0 >> 16 ) - ( line_x1 >> 16 ) );
		
		line_r += TempX * drdc;
		line_g += TempX * dgdc;
		line_b += TempX * dbdc;
	}
	else
	{
		// round to nearest
		//line_y2 += 0x8000;
		
		// set dxdc,dydc
		if ( y1 > y0 ) dydc = ( 1 << 16 ); else dydc = ( -1 << 16 );
		
		// x.16 fixed point
		dxdc = ( dxdy >> 16 );
		
		// color values
		drdc = ( ( r1 - r0 ) * dy ) >> 16;
		dgdc = ( ( g1 - g0 ) * dy ) >> 16;
		dbdc = ( ( b1 - b0 ) * dy ) >> 16;
		
		if ( y1 > y0 )
		{
			// line is going to bottom //
			
			// make sure start y is at .5 interval
			line_x = ( ( line_y0 + 0x7fff ) | 0x8000 ) & ~0x7fff;
			
			// make sure end y is at .5 interval
			line_y = ( ( line_y1 - 0x8000 ) | 0x8000 ) & ~0x7fff;
			
			// get difference
			Temp0 = line_x - line_y0;
			
			// get difference
			//Temp = line_x1 - line_y;
			Temp1 = line_y - line_y1;
			
			// get pixels since start of line
			TempX = ( line_x >> 16 ) - y0;
		}
		else
		{
			// line is going to top //
			
			// make sure start y is at .5 interval
			line_x = ( ( line_y0 - 0x8000 ) | 0x8000 ) & ~0x7fff;
			
			// make sure end y is at .5 interval
			line_y = ( ( line_y1 + 0x7fff ) | 0x8000 ) & ~0x7fff;
			
			// get difference
			Temp0 = line_y0 - line_x;
			//Temp0 = line_x - line_x0;
			
			// get difference
			Temp1 = line_y1 - line_y;
			//Temp1 = line_y - line_x1;
			
			// get pixels since start of line
			TempX = y0 - ( line_x >> 16 );
		}

		// update x
		line_x0 += ( Temp0 * dxdy ) >> 32;
		line_y0 = line_x;
		
		// update x
		line_x1 += ( Temp1 * dxdy ) >> 32;
		line_y1 = line_y;
		
		// distance is in y direction
		distance = _Abs ( ( line_y0 >> 16 ) - ( line_y1 >> 16 ) );
		
		// can calculate rgb start here using drdc,dgdc,dbdc
		//Temp = ( ( line_y0 >> 16 ) - y0 );
		line_r += TempX * drdc;
		line_g += TempX * dgdc;
		line_b += TempX * dbdc;
	}

	// if starting point is outside of window, then update
	//if ( line_x0 >= TestRight || line_y0 >= TestBottom )
	//{
	//	line_x0 += dxdc;
	//	line_y0 += dydc;
	//}
	
	// if ending point is outside of window, then update
	//if ( line_x1 >= TestRight || line_y1 >= TestBottom )
	//{
	//	line_x1 -= dxdc;
	//	line_y1 -= dydc;
	//}
	
	
	// if the last point on screen is not the endpoint for entire line, then include it
	if ( x1 != ( line_x1 >> 16 ) || y1 != ( line_y1 >> 16 ) ) distance++;


	// draw the line
	// note: the final point should probably not be drawn
	//for ( u32 i = 0; i <= distance; i++ )
	for ( u32 i = 0; i < distance; i++ )
	{
		// get coords
		//iX = ( _Round( line_x ) >> 32 );
		//iY = ( _Round( line_y ) >> 32 );
		iX = ( line_x0 >> 16 );
		iY = ( line_y0 >> 16 );
		
		//if ( iX >= DrawArea_TopLeftX && iY >= DrawArea_TopLeftY
		//&& iX <= DrawArea_BottomRightX && iY <= DrawArea_BottomRightY )
		//{
			//bgr = ( _Round( line_r ) >> 35 ) | ( ( _Round( line_g ) >> 35 ) << 5 ) | ( ( _Round( line_b ) >> 35 ) << 10 );
			//bgr = ( line_r >> 27 ) | ( ( line_g >> 27 ) << 5 ) | ( ( line_b >> 27 ) << 10 );
			// TODO: dithering ??
			if ( DTD )
			{
				DitherValue = DitherArray [ ( iX & 0x3 ) + ( ( iY & 0x3 ) << 2 ) ];
				
				iR = line_r << 8;
				iG = line_g << 8;
				iB = line_b << 8;
				
				// perform dither
				Red = iR + DitherValue;
				Green = iG + DitherValue;
				Blue = iB + DitherValue;
				
				// perform shift
				Red >>= 27;
				Green >>= 27;
				Blue >>= 27;
				
				// if dithering, perform signed clamp to 5 bits
				Red = AddSignedClamp<s64,5> ( Red );
				Green = AddSignedClamp<s64,5> ( Green );
				Blue = AddSignedClamp<s64,5> ( Blue );
				
				bgr = Red | ( Green << 5 ) | ( Blue << 10 );
			}
			else
			{
				// perform shift
				//Red = ( iR >> 27 );
				//Green = ( iG >> 27 );
				//Blue = ( iB >> 27 );
				
				bgr = ( line_r >> 19 ) | ( ( line_g >> 19 ) << 5 ) | ( ( line_b >> 19 ) << 10 );
			}
				
			//bgr = ( line_r >> 19 ) | ( ( line_g >> 19 ) << 5 ) | ( ( line_b >> 19 ) << 10 );
			
			ptr16 = & ( VRAM [ iX + ( iY << 10 ) ] );
			
			// read pixel from frame buffer if we need to check mask bit
			//DestPixel = *ptr16;
			if ( ABE || PIXELMASK )
			{
				// read pixel from frame buffer if we need to check mask bit
				DestPixel = *ptr16;
			}
			
			// semi-transparency
			//if ( command_abe )
			if ( ABE )
			{
				//bgr = SemiTransparency16 ( DestPixel, bgr, GPU_CTRL_Read.ABR );
				bgr = SemiTransparency16_t<ABRCODE> ( DestPixel, bgr );
			}
				
			// draw point
			
			// check if we should set mask bit when drawing
			//if ( GPU_CTRL_Read.MD ) bgr |= 0x8000;
			if ( SETPIXELMASK )
			{
				// set pixel mask if specified
				bgr |= SETPIXELMASK;
			}

			// draw pixel if we can draw to mask pixels or mask bit not set
			//if ( ! ( DestPixel & PIXELMASK ) ) *ptr16 = ( bgr | SETPIXELMASK );
			if ( PIXELMASK )
			{
				if ( ! ( DestPixel & PIXELMASK ) ) *ptr16 = bgr;
			}
			else
			{
				*ptr16 = bgr;
			}
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			//NumberOfPixelsDrawn++;
		//}
		
		line_x0 += dxdc;
		line_y0 += dydc;
		line_r += drdc;
		line_g += dgdc;
		line_b += dbdc;
	}
	
	NumberOfPixelsDrawn = distance;
	BusyCycles += NumberOfPixelsDrawn * dShadedLine_50_CyclesPerPixel;
	
#ifdef ENABLE_DRAW_OVERHEAD
	BusyCycles += DrawOverhead_Cycles;
#endif
}


template<const long PIXELMASK, const long SETPIXELMASK, const long ABE, const long ABRCODE>
inline void GPU::Select_ShadedLine4_t ()
{
	if ( GPU_CTRL_Read.DTD )
	{
		Draw_ShadedLine_t <PIXELMASK, SETPIXELMASK, ABE, ABRCODE,1> ();
	}
	else
	{
		Draw_ShadedLine_t <PIXELMASK, SETPIXELMASK, ABE, ABRCODE,0> ();
	}
}


template<const long PIXELMASK, const long ABE, const long ABRCODE>
inline void GPU::Select_ShadedLine3_t ()
{
	if ( GPU_CTRL_Read.MD )
	{
		Select_ShadedLine4_t <PIXELMASK, 0x8000, ABE, ABRCODE> ();
	}
	else
	{
		Select_ShadedLine4_t <PIXELMASK, 0, ABE, ABRCODE> ();
	}
}


template<const long ABE, const long ABRCODE>
inline void GPU::Select_ShadedLine2_t ()
{
	if ( GPU_CTRL_Read.ME )
	{
		Select_ShadedLine3_t<0x8000, ABE, ABRCODE> ();
	}
	else
	{
		Select_ShadedLine3_t<0, ABE, ABRCODE> ();
	}
}

template<const long ABE>
void GPU::Select_ShadedLine_t ()
{
	switch ( GPU_CTRL_Read.ABR )
	{
		case 0:
			Select_ShadedLine2_t<ABE, 0> ();
			break;
			
		case 1:
			Select_ShadedLine2_t<ABE, 1> ();
			break;
			
		case 2:
			Select_ShadedLine2_t<ABE, 2> ();
			break;
			
		case 3:
			Select_ShadedLine2_t<ABE, 3> ();
			break;
	}
}



	};


};



#endif

