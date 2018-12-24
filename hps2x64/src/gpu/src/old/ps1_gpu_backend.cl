
// 64-bit double enable
//#pragma OPENCL EXTENSION cl_khr_fp64 : enable
//#undef USE_OVERFLOW_BUILTINS

#define SINGLE_SCANLINE_MODE


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long s64;


int divide_s32 ( int op1, int op2 );
u16 ColorMultiply1624 ( u64 Color16, u64 Color24 );
u16 SemiTransparency16 ( u16 B, u16 F, u32 abrCode );


constant static const int c_lFrameBuffer_Width = 1024;
constant static const int c_lFrameBuffer_Height = 512;

constant static const int c_lFrameBuffer_Width_Mask = c_lFrameBuffer_Width - 1;
constant static const int c_lFrameBuffer_Height_Mask = c_lFrameBuffer_Height - 1;



// maximum width/height of a polygon allowed
constant static const s32 c_MaxPolygonWidth = 1023;
constant static const s32 c_MaxPolygonHeight = 511;


constant const s32 c_iDitherValues24 [] = { -4 << 16, 0 << 16, -3 << 16, 1 << 16,
									2 << 16, -2 << 16, 3 << 16, -1 << 16,
									-3 << 16, 1 << 16, -4 << 16, 0 << 16,
									3 << 16, -1 << 16, 2 << 16, -2 << 16 };

									
	constant const int c_iVisibleArea_StartX_Cycle = 584;
	constant const int c_iVisibleArea_EndX_Cycle = 3192;
	constant const int c_iVisibleArea_StartY_Pixel_NTSC = 15;
	constant const int c_iVisibleArea_EndY_Pixel_NTSC = 257;
	constant const int c_iVisibleArea_StartY_Pixel_PAL = 34;
	constant const int c_iVisibleArea_EndY_Pixel_PAL = 292;

	constant const int c_iVisibleArea_StartY [] = { c_iVisibleArea_StartY_Pixel_NTSC, c_iVisibleArea_StartY_Pixel_PAL };
	constant const int c_iVisibleArea_EndY [] = { c_iVisibleArea_EndY_Pixel_NTSC, c_iVisibleArea_EndY_Pixel_PAL };

	constant const u32 c_iGPUCyclesPerPixel [] = { 10, 7, 8, 0, 5, 0, 4, 0 };


union DATA_Write_Format
{
	// Command | BGR
	struct
	{
		// Color / Shading info
		
		// bits 0-7
		u8 Red;
		
		// bits 8-15
		u8 Green;
		
		// bits 16-23
		u8 Blue;
		
		// the command for the packet
		// bits 24-31
		u8 Command;
	};
	
	// y | x
	struct
	{
		// 16-bit values of y and x in the frame buffer
		// these look like they are signed
		
		// bits 0-10 - x-coordinate
		s16 x;
		//s32 x : 11;
		
		// bits 11-15 - not used
		//s32 NotUsed0 : 5;
		
		// bits 16-26 - y-coordinate
		s16 y;
		//s32 y : 11;
		
		// bits 27-31 - not used
		//s32 NotUsed1 : 5;
	};
	
	struct
	{
		u8 u;
		u8 v;
		u16 filler11;
	};
	
	// clut | v | u
	/*
	struct
	{
		u16 filler13;
		
		struct
		{
			// x-coordinate x/16
			// bits 0-5
			u16 x : 6;
			
			// y-coordinate 0-511
			// bits 6-14
			u16 y : 9;
			
			// bit 15 - Unknown/Unused (should be 0)
			u16 unknown0 : 1;
		};
	} clut;
	*/
	
	// h | w
	struct
	{
		u16 w;
		u16 h;
	};
	
	// tpage | v | u
	/*
	struct
	{
		// filler for u and v
		u32 filler9 : 16;
		
		// texture page x-coordinate
		// X*64
		// bits 0-3
		u32 tx : 4;

		// texture page y-coordinate
		// 0 - 0; 1 - 256
		// bit 4
		u32 ty : 1;
		
		// Semi-transparency mode
		// 0: 0.5xB+0.5 xF; 1: 1.0xB+1.0 xF; 2: 1.0xB-1.0 xF; 3: 1.0xB+0.25xF
		// bits 5-6
		u32 abr : 2;
		
		// Color mode
		// 0 - 4bit CLUT; 1 - 8bit CLUT; 2 - 15bit direct color; 3 - 15bit direct color
		// bits 7-8
		u32 tp : 2;
		
		// bits 9-10 - Unused
		u32 zero0 : 2;
		
		// bit 11 - same as GP0(E1).bit11 - Texture Disable
		// 0: Normal; 1: Disable if GP1(09h).Bit0=1
		u32 TextureDisable : 1;

		// bits 12-15 - Unused (should be 0)
		u32 Zero0 : 4;
	} tpage;
	*/
	
	u32 Value;
		
};
typedef union DATA_Write_Format DATA_Write_Format;


union Pixel_24bit_Format
{
	// 2 pixels encoded using 3 frame buffer pixels
	// G0 | R0 | R1 | B0 | B1 | G1
	struct
	{
		// this format is stored in frame buffer as 2 pixels in a 48 bit space (3 "pixels" of space)
		u8 Green1;
		u8 Blue1;
		u8 Blue0;
		u8 Red1;
		u8 Red0;
		u8 Green0;
	};
	
	struct
	{
		u16 Pixel2;
		u16 Pixel1;
		u16 Pixel0;
	};
};
typedef union Pixel_24bit_Format Pixel_24bit_Format;



/*
union GPU_CTRL_Read_Format
{
	struct
	{
		// bits 0-3
		// Texture page X = tx*64: 0-0;1-64;2-128;3-196;4-...
		u32 TX : 4;

		// bit 4
		// Texture page Y: 0-0;1-256
		u32 TY : 1;

		// bits 5-6
		// Semi-transparent state: 00-0.5xB+0.5xF;01-1.0xB+1.0xF;10-1.0xB-1.0xF;11-1.0xB+0.25xF
		u32 ABR : 2;

		// bits 7-8
		// Texture page color mode: 00-4bit CLUT;01-8bit CLUT; 10-15bit
		u32 TP : 2;

		// bit 9
		// 0-dither off; 1-dither on
		u32 DTD : 1;

		// bit 10
		// 0-Draw to display area prohibited;1-Draw to display area allowed
		// 1 - draw all fields; 0 - only allow draw to display for even fields
		u32 DFE : 1;

		// bit 11
		// 0-off;1-on, apply mask bit to drawn pixels
		u32 MD : 1;

		// bit 12
		// 0-off;1-on, no drawing pixels with mask bit set
		u32 ME : 1;

		// bits 13-15
		//u32 Unknown1 : 3;
		
		// bit 13 - reserved (seems to be always set?)
		u32 Reserved : 1;
		
		// bit 14 - Reverse Flag (0: Normal; 1: Distorted)
		u32 ReverseFlag : 1;
		
		// bit 15 - Texture disable (0: Normal; 1: Disable textures)
		u32 TextureDisable : 1;

		// bits 16-18
		// screen width is: 000-256 pixels;010-320 pixels;100-512 pixels;110-640 pixels;001-368 pixels
		u32 WIDTH : 3;

		// bit 19
		// 0-screen height is 240 pixels; 1-screen height is 480 pixels
		u32 HEIGHT : 1;

		// bit 20
		// 0-NTSC;1-PAL
		u32 VIDEO : 1;

		// bit 21
		// 0- 15 bit direct mode;1- 24-bit direct mode
		u32 ISRGB24 : 1;

		// bit 22
		// 0-Interlace off; 1-Interlace on
		u32 ISINTER : 1;

		// bit 23
		// 0-Display enabled;1-Display disabled
		u32 DEN : 1;

		// bits 24-25
		//u32 Unknown0 : 2;
		
		// bit 24 - Interrupt Request (0: off; 1: on) [GP0(0x1f)/GP1(0x02)]
		u32 IRQ : 1;
		
		// bit 25 - DMA / Data Request
		// When GP1(0x04)=0 -> always zero
		// when GP1(0x04)=1 -> FIFO State (0: full; 1: NOT full)
		// when GP1(0x04)=2 -> Same as bit 28
		// when GP1(0x04)=3 -> same as bit 27
		u32 DataRequest : 1;

		// bit 26
		// Ready to receive CMD word
		// 0: NOT Ready; 1: Ready
		// 0-GPU is busy;1-GPU is idle
		u32 BUSY : 1;

		// bit 27
		// 0-Not ready to send image (packet 0xc0);1-Ready to send image
		u32 IMG : 1;

		// bit 28
		// Ready to receive DMA block
		// 0: NOT Ready; 1: Ready
		// 0-Not ready to receive commands;1-Ready to receive commands
		u32 COM : 1;

		// bit 29-30
		// 00-DMA off, communication through GP0;01-?;10-DMA CPU->GPU;11-DMA GPU->CPU
		u32 DMA : 2;

		// bit 31
		// 0-Drawing even lines in interlace mode;1-Drawing uneven lines in interlace mode
		u32 LCF : 1;
	};
	
	u32 Value;
};
typedef union GPU_CTRL_Read_Format GPU_CTRL_Read_Format;
*/












kernel void Draw_FrameBufferRectangle_02 ( __global u16* VRAM, __global DATA_Write_Format* inputdata )
{
	// goes at the top for opencl function

	const int local_id = get_local_id( 0 );
	const int group_id = get_group_id( 0 );
	const int num_local_threads = get_local_size ( 0 );
	const int num_global_groups = get_num_groups( 0 );
	
#ifdef SINGLE_SCANLINE_MODE
	const int xid = local_id;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	
	// the space between consecutive yid's
	const int yinc = num_global_groups;
#endif

//inputdata format:
//0: -------
//1: -------
//2: -------
//3: -------
//4: -------
//5: -------
//6: -------
//7: GetBGR ( Buffer [ 0 ] );
//8: GetXY ( Buffer [ 1 ] );
//9: GetHW ( Buffer [ 2 ] );


	// ptr to vram
	__global u16 *ptr;
	//u16 *ptr16;
	__local u16 bgr16;
	__local u32 bgr32;
	
	__local u32 w, h, xmax, ymax, ymax2;
	__local s32 x, y;
	
	__local s32 group_yoffset;
	
	__private u32 xoff, yoff;
	
	
	// set __local variables
	if ( !local_id )
	{
		// set bgr64
		//bgr64 = gbgr [ 0 ];
		//bgr64 |= ( bgr64 << 16 );
		//bgr64 |= ( bgr64 << 32 );
		bgr32 = inputdata [ 7 ].Value;
		bgr16 = ( ( bgr32 >> 9 ) & 0x7c00 ) | ( ( bgr32 >> 6 ) & 0x3e0 ) | ( ( bgr32 >> 3 ) & 0x1f );
		
		x = inputdata [ 8 ].x;
		y = inputdata [ 8 ].y;
		
		// x and y are actually 11 bits
		// doesn't matter for frame buffer
		//x = ( x << 5 ) >> 5;
		//y = ( y << 5 ) >> 5;
		
		w = inputdata [ 9 ].w;
		h = inputdata [ 9 ].h;
		
		// Xpos=(Xpos AND 3F0h)
		x &= 0x3f0;
		
		// ypos & 0x1ff
		y &= 0x1ff;
		
		// Xsiz=((Xsiz AND 3FFh)+0Fh) AND (NOT 0Fh)
		w = ( ( w & 0x3ff ) + 0xf ) & ~0xf;
		
		// Ysiz=((Ysiz AND 1FFh))
		h &= 0x1ff;
	
		// adding xmax, ymax
		xmax = x + w;
		ymax = y + h;
		
		//printf( "\ninputdata= %x %x %x %x", inputdata [ 0 ].Value, inputdata [ 1 ].Value, inputdata [ 2 ].Value, inputdata [ 3 ].Value );
		//printf( "\nvram= %x %x %x %x", VRAM [ 0 ], VRAM [ 1 ], VRAM [ 2 ], VRAM [ 3 ] );
		
		ymax2 = 0;
		if ( ymax > c_lFrameBuffer_Height )
		{
			ymax2 = ymax - c_lFrameBuffer_Height;
			ymax = c_lFrameBuffer_Height;
		}
		
		// offset to get to this compute unit's scanline
		group_yoffset = group_id - ( y % num_global_groups );
		if ( group_yoffset < 0 )
		{
			group_yoffset += num_global_groups;
		}
		
		//printf ( "local_id= %i num_global_groups= %i group_id= %i group_yoffset= %i yoff= %i", local_id, num_global_groups, group_id, group_yoffset, y + group_yoffset + ( yid * yinc ) );
	}

	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );

	
	// *** NOTE: coordinates wrap *** //
	
	//printf( "\nlocal_id#%i group_id= %i __local_size= %i group_max_size= %i x= %i y= %i w= %i h= %i", local_id, yinit, xinc, yinc, x, y, w, h );
	//printf( "\n__local_size= %i", xinc );
	//printf( "\ngroup_max_size= %i", yinc );
	
	
	// need to first make sure there is something to draw
	if ( h > 0 && w > 0 )
	{
		for ( yoff = y + group_yoffset + yid; yoff < ymax; yoff += yinc )
		{
			for ( xoff = x + xid; xoff < xmax; xoff += xinc )
			{
				VRAM [ ( xoff & c_lFrameBuffer_Width_Mask ) + ( yoff << 10 ) ] = bgr16;
			}
		}
		
		for ( yoff = group_id + yid; yoff < ymax2; yoff += yinc )
		{
			for ( xoff = x + xid; xoff < xmax; xoff += xinc )
			{
				VRAM [ ( xoff & c_lFrameBuffer_Width_Mask ) + ( yoff << 10 ) ] = bgr16;
			}
		}
	}
	
	///////////////////////////////////////////////
	// set amount of time GPU will be busy for
	//BusyCycles += (u32) ( ( (u64) h * (u64) w * dFrameBufferRectangle_02_CyclesPerPixel ) );
}







// *** TODO ***
kernel void Draw_ShadedLine_50 ( __global u16* VRAM, __global DATA_Write_Format* inputdata )
{
	const int local_id = get_local_id( 0 );
	const int group_id = get_group_id( 0 );
	const int num_local_threads = get_local_size ( 0 );
	const int num_global_groups = get_num_groups( 0 );
	
#ifdef SINGLE_SCANLINE_MODE
	const int xid = local_id;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
#endif


//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7:GetBGR0_8 ( Buffer [ 0 ] );
//8:GetXY0 ( Buffer [ 1 ] );
//9:GetBGR1_8 ( Buffer [ 2 ] );
//10:GetXY1 ( Buffer [ 3 ] );

	
	__local u32 GPU_CTRL_Read, GPU_CTRL_Read_ABR;
	__local s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	__local s32 DrawArea_OffsetX, DrawArea_OffsetY;
	__local u32 Command_ABE;
	__local u32 GPU_CTRL_Read_DTD;
	
	__global u16 *ptr;
	
	__private s32 Temp;
	__local s32 LeftMostX, RightMostX;
	
	__private s32 StartX, EndX;
	__local s32 StartY, EndY;

	//s64 r10, r20, r21;
	
	//s32* DitherArray;
	//s32* DitherLine;
	__private s32 DitherValue;

	// new __local variables
	__local s32 x0, x1, x2, y0, y1, y2;
	__local s32 dx_left, dx_right;
	__local s32 x_left, x_right;
	__private s32 x_across;
	__private u32 bgr, bgr_temp;
	__private s32 Line;
	__local s32 t0, t1, denominator;

	// more __local variables for gradient triangle
	__local s32 dR_left, dG_left, dB_left;
	__local s32 dR_across, dG_across, dB_across;
	__private s32 iR, iG, iB;
	__local s32 R_left, G_left, B_left;
	__private s32 Roff_left, Goff_left, Boff_left;
	__local s32 r0, r1, r2, g0, g1, g2, b0, b1, b2;
	//__local s32 gr [ 3 ], gg [ 3 ], gb [ 3 ];

	__local s32 gx [ 3 ], gy [ 3 ], gbgr [ 3 ];
	
	__private s32 xoff_left, xoff_right;
	
	__private s32 Red, Green, Blue;
	__private u32 DestPixel;
	__local u32 PixelMask, SetPixelMask;

	__local u32 Coord0, Coord1, Coord2;
	__local s32 group_yoffset;

	// setup __local vars
	if ( !local_id )
	{
		// no bitmaps in opencl ??
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;
		
		gbgr [ 0 ] = inputdata [ 7 ].Value & 0x00ffffff;
		gbgr [ 1 ] = inputdata [ 9 ].Value & 0x00ffffff;
		//gbgr [ 2 ] = inputdata [ 11 ].Value & 0x00ffffff;
		gx [ 0 ] = (s32) ( ( inputdata [ 8 ].x << 5 ) >> 5 );
		gy [ 0 ] = (s32) ( ( inputdata [ 8 ].y << 5 ) >> 5 );
		gx [ 1 ] = (s32) ( ( inputdata [ 10 ].x << 5 ) >> 5 );
		gy [ 1 ] = (s32) ( ( inputdata [ 10 ].y << 5 ) >> 5 );
		//gx [ 2 ] = (s32) ( ( inputdata [ 12 ].x << 5 ) >> 5 );
		//gy [ 2 ] = (s32) ( ( inputdata [ 12 ].y << 5 ) >> 5 );
		
		GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		Command_ABE = inputdata [ 7 ].Command & 2;
		
		// DTD is bit 9 in GPU_CTRL_Read
		GPU_CTRL_Read_DTD = ( GPU_CTRL_Read >> 9 ) & 1;
		//GPU_CTRL_Read_DTD = 0;
		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		Coord0 = 0;
		Coord1 = 1;
		//Coord2 = 2;
		
		// initialize number of pixels drawn
		//NumberOfPixelsDrawn = 0;
		
		
		
		
		///////////////////////////////////
		// put top coordinates in x0,y0
		//if ( y1 < y0 )
		if ( gy [ Coord1 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y1 );
			//Swap ( Coord0, Coord1 );
			Temp = Coord0;
			Coord0 = Coord1;
			Coord1 = Temp;
		}
		
		
		// get x-values
		x0 = gx [ Coord0 ];
		x1 = gx [ Coord1 ];
		
		// get y-values
		y0 = gy [ Coord0 ];
		y1 = gy [ Coord1 ];

		// get rgb-values
		r0 = gbgr [ Coord0 ] & 0xff;
		r1 = gbgr [ Coord1 ] & 0xff;
		g0 = ( gbgr [ Coord0 ] >> 8 ) & 0xff;
		g1 = ( gbgr [ Coord1 ] >> 8 ) & 0xff;
		b0 = ( gbgr [ Coord0 ] >> 16 ) & 0xff;
		b1 = ( gbgr [ Coord1 ] >> 16 ) & 0xff;
		
		//////////////////////////////////////////
		// get coordinates on screen
		x0 += DrawArea_OffsetX;
		y0 += DrawArea_OffsetY;
		x1 += DrawArea_OffsetX;
		y1 += DrawArea_OffsetY;
		
		
		
		// get the left/right most x
		LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
		RightMostX = ( ( x0 > x1 ) ? x0 : x1 );

		
		
		
		/////////////////////////////////////////////////
		// draw top part of triangle
		
		
		
		
		
		//if ( denominator < 0 )
		//{
			// x1 is on the left and x0 is on the right //
			
			////////////////////////////////////
			// get slopes
			
			if ( y1 - y0 )
			{
				/////////////////////////////////////////////
				// init x on the left and right
				x_left = ( x0 << 16 );
				//x_right = x_left;
				
				R_left = ( r0 << 16 );
				G_left = ( g0 << 16 );
				B_left = ( b0 << 16 );
				
				if ( denominator < 0 )
				{
					dx_left = divide_s32( ( ( x1 - x0 ) << 16 ), ( y1 - y0 ) );
					//dx_right = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					dR_left = divide_s32( (( r1 - r0 ) << 16 ), ( y1 - y0 ) );
					dG_left = divide_s32( (( g1 - g0 ) << 16 ), ( y1 - y0 ) );
					dB_left = divide_s32( (( b1 - b0 ) << 16 ), ( y1 - y0 ) );
				}
				else
				{
					//dx_right = divide_s32( ( ( x1 - x0 ) << 16 ), ( y1 - y0 ) );
					dx_left = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					dR_left = divide_s32( (( r2 - r0 ) << 16 ), ( y2 - y0 ) );
					dG_left = divide_s32( (( g2 - g0 ) << 16 ), ( y2 - y0 ) );
					dB_left = divide_s32( (( b2 - b0 ) << 16 ), ( y2 - y0 ) );
				}
			}
			else
			{
				if ( denominator < 0 )
				{
					// change x_left and x_right where y1 is on left
					x_left = ( x1 << 16 );
					x_right = ( x0 << 16 );
					
					//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
					//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					//dx_left = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
					//dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					dx_left = divide_s32( ( ( x2 - x1 ) << 16 ), ( y2 - y1 ) );
					dx_right = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					R_left = ( r1 << 16 );
					G_left = ( g1 << 16 );
					B_left = ( b1 << 16 );

					//dR_left = (( r2 - r1 ) << 16 ) / ( y2 - y1 );
					//dG_left = (( g2 - g1 ) << 16 ) / ( y2 - y1 );
					//dB_left = (( b2 - b1 ) << 16 ) / ( y2 - y1 );
					dR_left = divide_s32( (( r2 - r1 ) << 16 ), ( y2 - y1 ) );
					dG_left = divide_s32( (( g2 - g1 ) << 16 ), ( y2 - y1 ) );
					dB_left = divide_s32( (( b2 - b1 ) << 16 ), ( y2 - y1 ) );
				}
				else
				{
					x_right = ( x1 << 16 );
					x_left = ( x0 << 16 );
				
					//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
					//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					//dx_right = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
					//dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					dx_right = divide_s32( ( ( x2 - x1 ) << 16 ), ( y2 - y1 ) );
					dx_left = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					R_left = ( r0 << 16 );
					G_left = ( g0 << 16 );
					B_left = ( b0 << 16 );
					
					//dR_left = (( r2 - r0 ) << 16 ) / ( y2 - y0 );
					//dG_left = (( g2 - g0 ) << 16 ) / ( y2 - y0 );
					//dB_left = (( b2 - b0 ) << 16 ) / ( y2 - y0 );
					dR_left = divide_s32( (( r2 - r0 ) << 16 ), ( y2 - y0 ) );
					dG_left = divide_s32( (( g2 - g0 ) << 16 ), ( y2 - y0 ) );
					dB_left = divide_s32( (( b2 - b0 ) << 16 ), ( y2 - y0 ) );
				}
			}
		//}
		


	
		////////////////
		// *** TODO *** at this point area of full triangle can be calculated and the rest of the drawing can be put on another thread *** //
		
		
		
		// r,g,b values are not specified with a fractional part, so there must be an initial fractional part
		R_left |= ( 1 << 15 );
		G_left |= ( 1 << 15 );
		B_left |= ( 1 << 15 );
		
		
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
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY + 1;
		}

		
		// offset to get to this compute unit's scanline
		group_yoffset = group_id - ( StartY % num_global_groups );
		if ( group_yoffset < 0 )
		{
			group_yoffset += num_global_groups;
		}
		
		//printf( "x_left=%x x_right=%x dx_left=%i dx_right=%i R_left=%x G_left=%x B_left=%x OffsetX=%i OffsetY=%i",x_left,x_right,dx_left,dx_right,R_left,G_left,B_left, DrawArea_OffsetX, DrawArea_OffsetY );
		//printf( "x0=%i y0=%i x1=%i y1=%i x2=%i y2=%i r0=%i r1=%i r2=%i g0=%i g1=%i g2=%i b0=%i b1=%i b2=%i", x0, y0, x1, y1, x2, y2, r0, r1, r2, g0, g1, g2, b0, b1, b2 );
		//printf( "dR_across=%x dG_across=%x dB_across=%x", dR_across, dG_across, dB_across );

	}	// end if ( !local_id )
	
	

	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );

	
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

	// check if sprite is within draw area
	if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	// skip drawing if distance between vertices is greater than max allowed by GPU
	if ( ( abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
	{
		// skip drawing polygon
		return;
	}


	
	



	
	/////////////////////////////////////////////
	// init x on the left and right
	
	


	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	Roff_left = R_left + ( dR_left * (group_yoffset + yid) );
	Goff_left = G_left + ( dG_left * (group_yoffset + yid) );
	Boff_left = B_left + ( dB_left * (group_yoffset + yid) );
	
	//////////////////////////////////////////////
	// draw down to y1
	//for ( Line = StartY; Line < EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		
		// left point is included if points are equal
		StartX = ( xoff_left + 0xffffLL ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			
			iR = Roff_left;
			iG = Goff_left;
			iB = Boff_left;
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - xoff_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iR += dR_across * Temp;
				//iG += dG_across * Temp;
				//iB += dB_across * Temp;
			}
			
			iR += ( dR_across >> 8 ) * ( Temp >> 8 );
			iG += ( dG_across >> 8 ) * ( Temp >> 8 );
			iB += ( dB_across >> 8 ) * ( Temp >> 8 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + xid + ( Line << 10 ) ] );
			//DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			
			
			iR += ( dR_across * xid );
			iG += ( dG_across * xid );
			iB += ( dB_across * xid );

			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				if ( GPU_CTRL_Read_DTD )
				{
					//bgr = ( _Round( iR ) >> 32 ) | ( ( _Round( iG ) >> 32 ) << 8 ) | ( ( _Round( iB ) >> 32 ) << 16 );
					//bgr = ( _Round( iR ) >> 35 ) | ( ( _Round( iG ) >> 35 ) << 5 ) | ( ( _Round( iB ) >> 35 ) << 10 );
					//DitherValue = DitherLine [ x_across & 0x3 ];
					DitherValue = c_iDitherValues24 [ ( x_across & 3 ) + ( ( Line & 3 ) << 2 ) ];
					
					// perform dither
					//Red = iR + DitherValue;
					//Green = iG + DitherValue;
					//Blue = iB + DitherValue;
					Red = iR + DitherValue;
					Green = iG + DitherValue;
					Blue = iB + DitherValue;
					
					//Red = Clamp5 ( ( iR + DitherValue ) >> 27 );
					//Green = Clamp5 ( ( iG + DitherValue ) >> 27 );
					//Blue = Clamp5 ( ( iB + DitherValue ) >> 27 );
					
					// perform shift
					Red >>= ( 16 + 3 );
					Green >>= ( 16 + 3 );
					Blue >>= ( 16 + 3 );
					
					Red = clamp ( Red, 0, 0x1f );
					Green = clamp ( Green, 0, 0x1f );
					Blue = clamp ( Blue, 0, 0x1f );
				}
				else
				{
					Red = iR >> ( 16 + 3 );
					Green = iG >> ( 16 + 3 );
					Blue = iB >> ( 16 + 3 );
				}
					
				
					
					// if dithering, perform signed clamp to 5 bits
					//Red = AddSignedClamp<s64,5> ( Red );
					//Green = AddSignedClamp<s64,5> ( Green );
					//Blue = AddSignedClamp<s64,5> ( Blue );
					
					bgr_temp = ( Blue << 10 ) | ( Green << 5 ) | Red;
					
					// shade pixel color
				
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
					
					
					//bgr_temp = bgr;
		
					
					// semi-transparency
					if ( Command_ABE )
					{
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
					}
					
					// check if we should set mask bit when drawing
					bgr_temp |= SetPixelMask;

					
					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
						
					iR += ( dR_across * xinc );
					iG += ( dG_across * xinc );
					iB += ( dB_across * xinc );
				
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		
		/////////////////////////////////////
		// update x on left and right
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
		
		Roff_left += ( dR_left * yinc );
		Goff_left += ( dG_left * yinc );
		Boff_left += ( dB_left * yinc );
	}

	} // end if ( EndY > StartY )
}



kernel void TransferPixelPacketIn ( __global u16* VRAM, __global DATA_Write_Format* inputdata )
{
	const int local_id = get_local_id( 0 );
	const int group_id = get_group_id( 0 );
	const int num_local_threads = get_local_size ( 0 );
	const int num_global_groups = get_num_groups( 0 );
	
#ifdef SINGLE_SCANLINE_MODE
	const int xid = local_id;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
#endif

//inputdata format:
//0: GPU_CTRL_Read
//1: (DrawArea_TopLeft)
//2: (DrawArea_BottomRight)
//3: (DrawArea_Offset)
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7 Buffer [ 0 ]
//1:sX = Buffer [ 1 ].x;
//1:sY = Buffer [ 1 ].y;
//2:dX = Buffer [ 2 ].x;
//2:dY = Buffer [ 2 ].y;
//3:h = Buffer [ 3 ].h;
//3:w = Buffer [ 3 ].w;
//4: OffsetX
//5: OffsetY


	__local u32 GPU_CTRL_Read;
	__private u32 SrcPixel, DstPixel;
	__local u32 PixelMask, SetPixelMask;
	
	__local u32 SrcStartX, SrcStartY, DstStartX, DstStartY, Height, Width;
	__private u32 CurX, CurY;
	__global u16 *SrcPtr, *DstPtr;

	__private u32 sX, sY, dX, dY;
	__local s32 w, h;
	
	__local s32 OffsetX, OffsetY;

	if ( !local_id )
	{
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		
		sX = inputdata [ 1 ].x;
		sY = inputdata [ 1 ].y;
		dX = inputdata [ 2 ].x;
		dY = inputdata [ 2 ].y;
		h = inputdata [ 3 ].h;
		w = inputdata [ 3 ].w;
		
		// nocash psx specifications: transfer/move vram-to-vram use masking
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		// xpos & 0x3ff
		//sX &= 0x3ff;
		SrcStartX = sX & 0x3ff;
		//dX &= 0x3ff;
		DstStartX = dX & 0x3ff;
		
		// ypos & 0x1ff
		//sY &= 0x1ff;
		SrcStartY = sY & 0x1ff;
		//dY &= 0x1ff;
		DstStartY = dY & 0x1ff;
		
		// Xsiz=((Xsiz-1) AND 3FFh)+1
		Width = ( ( w - 1 ) & 0x3ff ) + 1;
		
		// Ysiz=((Ysiz-1) AND 1FFh)+1
		Height = ( ( h - 1 ) & 0x1ff ) + 1;
		
		OffsetX = inputdata [ 4 ].Value;
		OffsetY = inputdata [ 5 ].Value;
	}
	
	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );

	//u32 bgr2;
	//u32 pix0, pix1;
	//u32 DestPixel, PixelMask = 0, SetPixelMask = 0;
	
#ifdef INLINE_DEBUG_PIX_WRITE
	debug << "; TRANSFER PIX IN; h = " << dec << h << "; w = " << w << "; iX = " << iX << "; iY = " << iY;
#endif

	//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;

	if ( local_id < 16 )
	{
		SrcPtr = (__global u16*) & ( inputdata [ 8 ].Value );
		
		CurY = OffsetY;
		if ( CurY >= Height )
		{
			return;
		}
		
		for ( CurX = OffsetX + xid; CurX >= Width; CurX -= Width )
		{
			CurY += yinc;
		}
		
		DstPtr = & ( VRAM [ ( ( CurX + SrcStartX ) & c_lFrameBuffer_Width_Mask ) + ( ( ( CurY + SrcStartY ) & c_lFrameBuffer_Height_Mask ) << 10 ) ] );
		
		DstPixel = *DstPtr;
		
		//bgr2 |= SetPixelMask;

		// draw pixel if we can draw to mask pixels or mask bit not set
		if ( ! ( DstPixel & PixelMask ) )
		{
			*DstPtr = SrcPtr [ local_id ] | SetPixelMask;
		}
	}
	
}



kernel void Transfer_MoveImage_80 ( __global u16* VRAM, __global DATA_Write_Format* inputdata )
{
	const int local_id = get_local_id( 0 );
	const int group_id = get_group_id( 0 );
	const int num_local_threads = get_local_size ( 0 );
	const int num_global_groups = get_num_groups( 0 );
	
#ifdef SINGLE_SCANLINE_MODE
	const int xid = local_id;
	const int yid = 0;
	
	//const int xinc = num_local_threads;
	//const int yinc = num_global_groups;
	const int xinc = 1;
	const int yinc = 1;
#endif

//inputdata format:
//0: GPU_CTRL_Read
//1: (DrawArea_TopLeft)
//2: (DrawArea_BottomRight)
//3: (DrawArea_Offset)
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7 Buffer [ 0 ]
//1:sX = Buffer [ 1 ].x;
//1:sY = Buffer [ 1 ].y;
//2:dX = Buffer [ 2 ].x;
//2:dY = Buffer [ 2 ].y;
//3:h = Buffer [ 3 ].h;
//3:w = Buffer [ 3 ].w;


	__local u32 GPU_CTRL_Read;
	__private u32 SrcPixel, DstPixel;
	__local u32 PixelMask, SetPixelMask;
	
	__local u32 SrcStartX, SrcStartY, DstStartX, DstStartY, Height, Width;
	__private u32 CurX, CurY;
	__global u16 *SrcPtr, *DstPtr;

	__private u32 sX, sY, dX, dY;
	__local s32 w, h;

	if ( !local_id )
	{
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		
		sX = inputdata [ 1 ].x;
		sY = inputdata [ 1 ].y;
		dX = inputdata [ 2 ].x;
		dY = inputdata [ 2 ].y;
		h = inputdata [ 3 ].h;
		w = inputdata [ 3 ].w;
		
		// nocash psx specifications: transfer/move vram-to-vram use masking
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		// xpos & 0x3ff
		//sX &= 0x3ff;
		SrcStartX = sX & 0x3ff;
		//dX &= 0x3ff;


		DstStartX = dX & 0x3ff;
		
		// ypos & 0x1ff
		//sY &= 0x1ff;
		SrcStartY = sY & 0x1ff;
		//dY &= 0x1ff;
		DstStartY = dY & 0x1ff;
		
		// Xsiz=((Xsiz-1) AND 3FFh)+1
		Width = ( ( w - 1 ) & 0x3ff ) + 1;
		
		// Ysiz=((Ysiz-1) AND 1FFh)+1
		Height = ( ( h - 1 ) & 0x1ff ) + 1;
	//}
	
	// synchronize __local variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

		
		// *** NOTE: coordinates wrap *** //
		
		// for opencl, Image moves can only be done with one worker !!! //
	
		//for ( CurY = 0; CurY < Height; CurY++ )
		for ( CurY = yid; CurY < Height; CurY += yinc )
		{
			// start Src/Dst pointers for line
			//SrcLinePtr = & ( VRAM [ ( ( SrcStartY + CurY ) & FrameBuffer_YMask ) << 10 ] );
			//DstLinePtr = & ( VRAM [ ( ( DstStartY + CurY ) & FrameBuffer_YMask ) << 10 ] );
			
			//for ( ; CurX < Width; CurX++ )
			for ( CurX = xid; CurX < Width; CurX += xinc )
			{
				SrcPtr = & ( VRAM [ ( ( SrcStartX + CurX ) & c_lFrameBuffer_Width_Mask ) + ( ( ( SrcStartY + CurY ) & c_lFrameBuffer_Height_Mask ) << 10 ) ] );
				DstPtr = & ( VRAM [ ( ( DstStartX + CurX ) & c_lFrameBuffer_Width_Mask ) + ( ( ( DstStartY + CurY ) & c_lFrameBuffer_Height_Mask ) << 10 ) ] );
				SrcPixel = *SrcPtr;
				DstPixel = *DstPtr;
				
				//SrcPixel |= SetPixelMask;
				
				if ( ! ( DstPixel & PixelMask ) ) *DstPtr = ( SrcPixel | SetPixelMask );
			}
			
		}
	}
	
}




kernel void Draw_Pixel_68 ( __global u16* VRAM, __global DATA_Write_Format* inputdata )
{
	const int local_id = get_local_id( 0 );
	

//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7:GetBGR24 ( Buffer [ 0 ] );
//8:GetXY ( Buffer [ 1 ] );


	__local u32 GPU_CTRL_Read, GPU_CTRL_Read_ABR;
	__local s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	__local s32 DrawArea_OffsetX, DrawArea_OffsetY;
	__local u32 Command_ABE;
	
	__local u32 bgr;
	//s32 Absolute_DrawX, Absolute_DrawY;
	__local s32 x, y;
	
	__global u16* ptr16;
	
	__local u32 DestPixel, PixelMask;
	__local u32 SetPixelMask;
	
	
	if ( !local_id )
	{
		// no bitmaps in opencl ??
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;

		GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		Command_ABE = inputdata [ 7 ].Command & 2;
		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		bgr = inputdata [ 7 ].Value & 0x00ffffff;
		x = (s32) ( ( inputdata [ 8 ].x << 5 ) >> 5 );
		y = (s32) ( ( inputdata [ 8 ].y << 5 ) >> 5 );
		
		// ?? convert to 16-bit ?? (or should leave 24-bit?)
		bgr = ( ( bgr & ( 0xf8 << 0 ) ) >> 3 ) | ( ( bgr & ( 0xf8 << 8 ) ) >> 6 ) | ( ( bgr & ( 0xf8 << 16 ) ) >> 9 );
		
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
		
		
		///////////////////////////////////////////////
		// set amount of time GPU will be busy for
		//BusyCycles += 1;
		
		/////////////////////////////////////////
		// Draw the pixel
		x += DrawArea_OffsetX;
		y += DrawArea_OffsetY;

		// make sure we are putting pixel within draw area
		if ( x >= DrawArea_TopLeftX && y >= DrawArea_TopLeftY && x <= DrawArea_BottomRightX && y <= DrawArea_BottomRightY )
		{
			ptr16 = & ( VRAM [ x + ( y << 10 ) ] );
			
			
			// read pixel from frame buffer if we need to check mask bit
			//DestPixel = VRAM [ Absolute_DrawX + ( Absolute_DrawY << 10 ) ];
			DestPixel = *ptr16;
			
			// semi-transparency
			if ( Command_ABE )
			{
				bgr = SemiTransparency16 ( DestPixel, bgr, GPU_CTRL_Read_ABR );
			}
			
			// check if we should set mask bit when drawing
			//if ( GPU_CTRL_Read.MD ) bgr |= 0x8000;

			// draw pixel if we can draw to mask pixels or mask bit not set
			if ( ! ( DestPixel & PixelMask ) ) *ptr16 = bgr | SetPixelMask;
		}
	}
}



kernel void DrawTriangle_Mono ( __global u16* VRAM, __global DATA_Write_Format* inputdata )
{
	const int local_id = get_local_id( 0 );
	const int group_id = get_group_id( 0 );
	const int num_local_threads = get_local_size ( 0 );
	const int num_global_groups = get_num_groups( 0 );
	
#ifdef SINGLE_SCANLINE_MODE
	const int xid = local_id;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
#endif

//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7: GetBGR24 ( Buffer [ 0 ] );
//8: GetXY0 ( Buffer [ 1 ] );
//9: GetXY1 ( Buffer [ 2 ] );
//10: GetXY2 ( Buffer [ 3 ] );



	__local u32 GPU_CTRL_Read, GPU_CTRL_Read_ABR;
	__local s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	__local s32 DrawArea_OffsetX, DrawArea_OffsetY;
	
	__local s32 Temp;
	__local s32 LeftMostX, RightMostX;
	
	
	// the y starts and ends at the same place, but the x is different for each line
	__local s32 StartY, EndY;
	
	
	//__local s64 r10, r20, r21;
	
	// new __local variables
	__local s32 x0, x1, x2, y0, y1, y2;
	__local s32 dx_left, dx_right;
	__local u32 bgr;
	__local s32 t0, t1, denominator;
	
	__local u32 Coord0, Coord1, Coord2;
	
	__local u32 PixelMask, SetPixelMask;
	
	__local s32 gx [ 3 ], gy [ 3 ], gbgr [ 3 ];
	__local u32 Command_ABE;
	
	__local s32 x_left, x_right;
	
	__local s32 group_yoffset;
	
	
	__private s32 StartX, EndX;
	__private s32 x_across;
	__private u32 xoff, yoff;
	__private s32 xoff_left, xoff_right;
	__private u32 DestPixel;
	__private u32 bgr_temp;
	__private s32 Line;
	__global u16 *ptr;
	


	// setup __local vars
	if ( !local_id )
	{
		// no bitmaps in opencl ??
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;
		
		gbgr [ 0 ] = inputdata [ 7 ].Value & 0x00ffffff;
		gx [ 0 ] = (s32) ( ( inputdata [ 8 ].x << 5 ) >> 5 );
		gy [ 0 ] = (s32) ( ( inputdata [ 8 ].y << 5 ) >> 5 );
		gx [ 1 ] = (s32) ( ( inputdata [ 9 ].x << 5 ) >> 5 );
		gy [ 1 ] = (s32) ( ( inputdata [ 9 ].y << 5 ) >> 5 );
		gx [ 2 ] = (s32) ( ( inputdata [ 10 ].x << 5 ) >> 5 );
		gy [ 2 ] = (s32) ( ( inputdata [ 10 ].y << 5 ) >> 5 );
		
		GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		Command_ABE = inputdata [ 7 ].Command & 2;
		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		Coord0 = 0;
		Coord1 = 1;
		Coord2 = 2;
		
		// initialize number of pixels drawn
		//NumberOfPixelsDrawn = 0;
		
		
		// get color(s)
		bgr = gbgr [ 0 ];
		
		// ?? convert to 16-bit ?? (or should leave 24-bit?)
		//bgr = ( ( bgr & ( 0xf8 << 0 ) ) >> 3 ) | ( ( bgr & ( 0xf8 << 8 ) ) >> 6 ) | ( ( bgr & ( 0xf8 << 16 ) ) >> 9 );
		bgr = ( ( bgr >> 9 ) & 0x7c00 ) | ( ( bgr >> 6 ) & 0x3e0 ) | ( ( bgr >> 3 ) & 0x1f );
		
		
		///////////////////////////////////
		// put top coordinates in x0,y0
		//if ( y1 < y0 )
		if ( gy [ Coord1 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y1 );
			//Swap ( Coord0, Coord1 );
			Temp = Coord0;
			Coord0 = Coord1;
			Coord1 = Temp;
		}
		
		//if ( y2 < y0 )
		if ( gy [ Coord2 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y2 );
			//Swap ( Coord0, Coord2 );
			Temp = Coord0;
			Coord0 = Coord2;
			Coord2 = Temp;
		}
		
		///////////////////////////////////////
		// put middle coordinates in x1,y1
		//if ( y2 < y1 )
		if ( gy [ Coord2 ] < gy [ Coord1 ] )
		{
			//Swap ( y1, y2 );
			//Swap ( Coord1, Coord2 );
			Temp = Coord1;
			Coord1 = Coord2;
			Coord2 = Temp;
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
		
		
		
		// get the left/right most x
		LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
		LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
		RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
		RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

		
		
		
		/////////////////////////////////////////////////
		// draw top part of triangle
		
		// denominator is negative when x1 is on the left, positive when x1 is on the right
		t0 = y1 - y2;
		t1 = y0 - y2;
		denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );

		// get reciprocals
		// *** todo ***
		//if ( y1 - y0 ) r10 = ( 1LL << 48 ) / ((s64)( y1 - y0 ));
		//if ( y2 - y0 ) r20 = ( 1LL << 48 ) / ((s64)( y2 - y0 ));
		//if ( y2 - y1 ) r21 = ( 1LL << 48 ) / ((s64)( y2 - y1 ));
		
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
				x_left = ( x0 << 16 );
				x_right = x_left;
				
				if ( denominator < 0 )
				{
					dx_left = ( ( x1 - x0 ) << 16 ) / ( y1 - y0 );
					dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					//dx_left = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
					//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
				}
				else
				{
					dx_right = ( ( x1 - x0 ) << 16 ) / ( y1 - y0 );
					dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					//dx_right = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
					//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
				}
			}
			else
			{
				if ( denominator < 0 )
				{
					// change x_left and x_right where y1 is on left
					x_left = ( x1 << 16 );
					x_right = ( x0 << 16 );
				
					dx_left = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
					dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
					//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
				}
				else
				{
					x_right = ( x1 << 16 );
					x_left = ( x0 << 16 );
				
					dx_right = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
					dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
					//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
				}
			}
		//}
		


	
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

		
		// offset to get to this compute unit's scanline
		group_yoffset = group_id - ( StartY % num_global_groups );
		if ( group_yoffset < 0 )
		{
			group_yoffset += num_global_groups;
		}

	}	// end if ( !local_id )
	
	

	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );

	
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

	// check if sprite is within draw area
	if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	// skip drawing if distance between vertices is greater than max allowed by GPU
	if ( ( abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
	{
		// skip drawing polygon
		return;
	}
	
	

	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	//////////////////////////////////////////////
	// draw down to y1
	//for ( Line = StartY; Line < EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		
		// left point is included if points are equal
		//StartX = ( (s64) ( x_left + 0xffffLL ) ) >> 16;
		//EndX = ( x_right - 1 ) >> 16;
		StartX = ( xoff_left + 0xffffLL ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
		
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				StartX = DrawArea_TopLeftX;
			}
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + xid + ( Line << 10 ) ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			//NumberOfPixelsDrawn += EndX - StartX + 1;
			

			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				// read pixel from frame buffer if we need to check mask bit
				DestPixel = *ptr;
				
				bgr_temp = bgr;
	
				// semi-transparency
				if ( Command_ABE )
				{
					bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
				}
				
				// check if we should set mask bit when drawing
				bgr_temp |= SetPixelMask;

				// draw pixel if we can draw to mask pixels or mask bit not set
				//if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
				DestPixel = ( ! ( DestPixel & PixelMask ) ) ? bgr_temp : DestPixel;
				*ptr = DestPixel;
				
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		
		/////////////////////////////////////
		// update x on left and right
		//x_left += dx_left;
		//x_right += dx_right;
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
	}

	} // end if ( EndY > StartY )

	
	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );

	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	/////////////////////////////////////////////
	// init x on the left and right
	
	if ( !local_id )
	{
	
		//////////////////////////////////////////////////////
		// check if y1 is on the left or on the right
		if ( denominator < 0 )
		{
			// y1 is on the left //
			
			x_left = ( x1 << 16 );
			
			// need to recalculate the other side when doing this in parallel with this algorithm
			x_right = ( x0 << 16 ) + ( ( y1 - y0 ) * dx_right );
			
			//if ( y2 - y1 )
			//{
				dx_left = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
				//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			//}
		}
		else
		{
			// y1 is on the right //
			
			x_right = ( x1 << 16 );
			
			// need to recalculate the other side when doing this in parallel with this algorithm
			x_left = ( x0 << 16 ) + ( ( y1 - y0 ) * dx_left );
			
			//if ( y2 - y1 )
			//{
				dx_right = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
				//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
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
		
		
		// offset to get to this compute unit's scanline
		group_yoffset = group_id - ( StartY % num_global_groups );
		if ( group_yoffset < 0 )
		{
			group_yoffset += num_global_groups;
		}
	}

	
	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );

	
	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	//////////////////////////////////////////////
	// draw down to y2
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		
		// left point is included if points are equal
		//StartX = ( x_left + 0xffffLL ) >> 16;
		//EndX = ( x_right - 1 ) >> 16;
		StartX = ( xoff_left + 0xffffLL ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				StartX = DrawArea_TopLeftX;
			}
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + xid + ( Line << 10 ) ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			//NumberOfPixelsDrawn += EndX - StartX + 1;
			

			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				// read pixel from frame buffer if we need to check mask bit
				DestPixel = *ptr;
				
				bgr_temp = bgr;
	
				// semi-transparency
				if ( Command_ABE )
				{
					bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
				}
				
				// check if we should set mask bit when drawing
				bgr_temp |= SetPixelMask;

				// draw pixel if we can draw to mask pixels or mask bit not set
				//if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
				DestPixel = ( ! ( DestPixel & PixelMask ) ) ? bgr_temp : DestPixel;
				*ptr = DestPixel;
				
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		/////////////////////////////////////
		// update x on left and right
		//x_left += dx_left;
		//x_right += dx_right;
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
	}
	
	} // end if ( EndY > StartY )

}




kernel void Draw_Rectangle_60 ( __global u16* VRAM, __global DATA_Write_Format* inputdata )
{
	const int local_id = get_local_id( 0 );
	const int group_id = get_group_id( 0 );
	const int num_local_threads = get_local_size ( 0 );
	const int num_global_groups = get_num_groups( 0 );
	
#ifdef SINGLE_SCANLINE_MODE
	const int xid = local_id;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
#endif

//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_BottomRightX
//2: DrawArea_TopLeftX
//3: DrawArea_BottomRightY
//4: DrawArea_TopLeftY
//5: DrawArea_OffsetX
//6: DrawArea_OffsetY
//----------------
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7: GetBGR24 ( Buffer [ 0 ] );
//8: GetXY ( Buffer [ 1 ] );
//9: GetHW ( Buffer [ 2 ] );

	//u32 Pixel;
	
	__local u32 GPU_CTRL_Read, GPU_CTRL_Read_ABR;
	__local s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	__local s32 DrawArea_OffsetX, DrawArea_OffsetY;
	__local u32 Command_ABE;
	
	__local s32 StartX, EndX, StartY, EndY;
	//u32 PixelsPerLine;
	
	// new __local variables
	__local s32 x, y;
	__local u32 w, h;
	__local s32 x0, x1, y0, y1;
	__local u32 bgr;
	
	__local u32 PixelMask, SetPixelMask;
	
	__local s32 group_yoffset;
	
	__global u16 *ptr;
	
	__private u32 DestPixel;
	__private u32 bgr_temp;
	__private s32 x_across;
	__private s32 Line;
	
	//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;


	// set __local variables
	if ( !local_id )
	{
		// set bgr64
		//bgr64 = gbgr [ 0 ];
		//bgr64 |= ( bgr64 << 16 );
		//bgr64 |= ( bgr64 << 32 );
		bgr = inputdata [ 7 ].Value;
		bgr = ( ( bgr >> 9 ) & 0x7c00 ) | ( ( bgr >> 6 ) & 0x3e0 ) | ( ( bgr >> 3 ) & 0x1f );
		
		x = inputdata [ 8 ].x;
		y = inputdata [ 8 ].y;
		
		// x and y are actually 11 bits
		x = ( x << ( 5 + 16 ) ) >> ( 5 + 16 );
		y = ( y << ( 5 + 16 ) ) >> ( 5 + 16 );
		
		w = inputdata [ 9 ].w;
		h = inputdata [ 9 ].h;
	
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		//DrawArea_BottomRightX = inputdata [ 1 ].Value;
		//DrawArea_TopLeftX = inputdata [ 2 ].Value;
		//DrawArea_BottomRightY = inputdata [ 3 ].Value;
		//DrawArea_TopLeftY = inputdata [ 4 ].Value;
		//DrawArea_OffsetX = inputdata [ 5 ].Value;
		//DrawArea_OffsetY = inputdata [ 6 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;
		
		/*
		gbgr [ 0 ] = inputdata [ 7 ].Value & 0x00ffffff;
		gx [ 0 ] = (s32) ( ( inputdata [ 8 ].x << 4 ) >> 4 );
		gy [ 0 ] = (s32) ( ( inputdata [ 8 ].y << 4 ) >> 4 );
		gx [ 1 ] = (s32) ( ( inputdata [ 9 ].x << 4 ) >> 4 );
		gy [ 1 ] = (s32) ( ( inputdata [ 9 ].y << 4 ) >> 4 );
		gx [ 2 ] = (s32) ( ( inputdata [ 10 ].x << 4 ) >> 4 );
		gy [ 2 ] = (s32) ( ( inputdata [ 10 ].y << 4 ) >> 4 );
		*/
		// get color(s)
		//bgr = gbgr [ 0 ];
		
		// ?? convert to 16-bit ?? (or should leave 24-bit?)
		//bgr = ( ( bgr & ( 0xf8 << 0 ) ) >> 3 ) | ( ( bgr & ( 0xf8 << 8 ) ) >> 6 ) | ( ( bgr & ( 0xf8 << 16 ) ) >> 9 );
		
		
		// get top left corner of sprite and bottom right corner of sprite
		x0 = x + DrawArea_OffsetX;
		y0 = y + DrawArea_OffsetY;
		x1 = x0 + w - 1;
		y1 = y0 + h - 1;
		
		//////////////////////////////////////////
		// get coordinates on screen
		//x0 = DrawArea_OffsetX + x0;
		//y0 = DrawArea_OffsetY + y0;
		//x1 = DrawArea_OffsetX + x1;
		//y1 = DrawArea_OffsetY + y1;
		
		
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
		
		GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		Command_ABE = inputdata [ 7 ].Command & 2;
		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		// initialize number of pixels drawn
		//NumberOfPixelsDrawn = 0;
		
		
		// get color(s)
		//bgr = gbgr [ 0 ];
		
		// ?? convert to 16-bit ?? (or should leave 24-bit?)
		//bgr = ( ( bgr & ( 0xf8 << 0 ) ) >> 3 ) | ( ( bgr & ( 0xf8 << 8 ) ) >> 6 ) | ( ( bgr & ( 0xf8 << 16 ) ) >> 9 );

		// offset to get to this compute unit's scanline
		group_yoffset = group_id - ( StartY % num_global_groups );
		if ( group_yoffset < 0 )
		{
			group_yoffset += num_global_groups;
		}
		
		
		//printf ( "x0=%i y0=%i x1=%i y1=%i StartX=%i StartY=%i EndX=%i EndY=%i", x0, y0, x1, y1, StartX, StartY, EndX, EndY );
	}

	
	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );
	
	
	// initialize number of pixels drawn
	//NumberOfPixelsDrawn = 0;
	
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

	// check if sprite is within draw area
	if ( x1 < ((s32)DrawArea_TopLeftX) || x0 > ((s32)DrawArea_BottomRightX) || y1 < ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	

	
	//NumberOfPixelsDrawn = ( EndX - StartX + 1 ) * ( EndY - StartY + 1 );
	
	
	//for ( Line = StartY; Line <= EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line <= EndY; Line += yinc )
	{
		ptr = & ( VRAM [ StartX + xid + ( Line << 10 ) ] );
		

		// draw horizontal line
		//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
		for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
		{
			// read pixel from frame buffer if we need to check mask bit
			DestPixel = *ptr;
			
			bgr_temp = bgr;

			// semi-transparency
			if ( Command_ABE )
			{
				bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
			}
			
			// check if we should set mask bit when drawing
			bgr_temp |= SetPixelMask;

			// draw pixel if we can draw to mask pixels or mask bit not set
			//if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
			DestPixel = ( ! ( DestPixel & PixelMask ) ) ? bgr_temp : DestPixel;
			*ptr = DestPixel;
			
			// update pointer for pixel out
			//ptr += c_iVectorSize;
			ptr += xinc;
		}
	}
	
	// set the amount of time drawing used up
	//BusyCycles = NumberOfPixelsDrawn * 1;
}


kernel void DrawTriangle_Gradient ( __global u16* VRAM, __global DATA_Write_Format* inputdata )
{
	const int local_id = get_local_id( 0 );
	const int group_id = get_group_id( 0 );
	const int num_local_threads = get_local_size ( 0 );
	const int num_global_groups = get_num_groups( 0 );
	
#ifdef SINGLE_SCANLINE_MODE
	const int xid = local_id;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
#endif


//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7: GetBGR0_8 ( Buffer [ 0 ] );
//8: GetXY0 ( Buffer [ 1 ] );
//9: GetBGR1_8 ( Buffer [ 2 ] );
//10: GetXY1 ( Buffer [ 3 ] );
//11: GetBGR2_8 ( Buffer [ 4 ] );
//12: GetXY2 ( Buffer [ 5 ] );

	
	__local u32 GPU_CTRL_Read, GPU_CTRL_Read_ABR;
	__local s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	__local s32 DrawArea_OffsetX, DrawArea_OffsetY;
	__local u32 Command_ABE;
	__local u32 GPU_CTRL_Read_DTD;
	
	__global u16 *ptr;
	
	__private s32 Temp;
	__local s32 LeftMostX, RightMostX;
	
	__private s32 StartX, EndX;
	__local s32 StartY, EndY;

	//s64 r10, r20, r21;
	
	//s32* DitherArray;
	//s32* DitherLine;
	__private s32 DitherValue;

	// new __local variables
	__local s32 x0, x1, x2, y0, y1, y2;
	__local s32 dx_left, dx_right;
	__local s32 x_left, x_right;
	__private s32 x_across;
	__private u32 bgr, bgr_temp;
	__private s32 Line;
	__local s32 t0, t1, denominator;

	// more __local variables for gradient triangle
	__local s32 dR_left, dG_left, dB_left;
	__local s32 dR_across, dG_across, dB_across;
	__private s32 iR, iG, iB;
	__local s32 R_left, G_left, B_left;
	__private s32 Roff_left, Goff_left, Boff_left;
	__local s32 r0, r1, r2, g0, g1, g2, b0, b1, b2;
	//__local s32 gr [ 3 ], gg [ 3 ], gb [ 3 ];

	__local s32 gx [ 3 ], gy [ 3 ], gbgr [ 3 ];
	
	__private s32 xoff_left, xoff_right;
	
	__private s32 Red, Green, Blue;
	__private u32 DestPixel;
	__local u32 PixelMask, SetPixelMask;

	__local u32 Coord0, Coord1, Coord2;
	__local s32 group_yoffset;

	// setup __local vars
	if ( !local_id )
	{
		// no bitmaps in opencl ??
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;
		
		gbgr [ 0 ] = inputdata [ 7 ].Value & 0x00ffffff;
		gbgr [ 1 ] = inputdata [ 9 ].Value & 0x00ffffff;
		gbgr [ 2 ] = inputdata [ 11 ].Value & 0x00ffffff;
		gx [ 0 ] = (s32) ( ( inputdata [ 8 ].x << 5 ) >> 5 );
		gy [ 0 ] = (s32) ( ( inputdata [ 8 ].y << 5 ) >> 5 );
		gx [ 1 ] = (s32) ( ( inputdata [ 10 ].x << 5 ) >> 5 );
		gy [ 1 ] = (s32) ( ( inputdata [ 10 ].y << 5 ) >> 5 );
		gx [ 2 ] = (s32) ( ( inputdata [ 12 ].x << 5 ) >> 5 );
		gy [ 2 ] = (s32) ( ( inputdata [ 12 ].y << 5 ) >> 5 );
		
		GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		Command_ABE = inputdata [ 7 ].Command & 2;
		
		// DTD is bit 9 in GPU_CTRL_Read
		GPU_CTRL_Read_DTD = ( GPU_CTRL_Read >> 9 ) & 1;
		//GPU_CTRL_Read_DTD = 0;
		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		Coord0 = 0;
		Coord1 = 1;
		Coord2 = 2;
		
		// initialize number of pixels drawn
		//NumberOfPixelsDrawn = 0;
		
		
		
		
		///////////////////////////////////
		// put top coordinates in x0,y0
		//if ( y1 < y0 )
		if ( gy [ Coord1 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y1 );
			//Swap ( Coord0, Coord1 );
			Temp = Coord0;
			Coord0 = Coord1;
			Coord1 = Temp;
		}
		
		//if ( y2 < y0 )
		if ( gy [ Coord2 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y2 );
			//Swap ( Coord0, Coord2 );
			Temp = Coord0;
			Coord0 = Coord2;
			Coord2 = Temp;
		}
		
		///////////////////////////////////////
		// put middle coordinates in x1,y1
		//if ( y2 < y1 )
		if ( gy [ Coord2 ] < gy [ Coord1 ] )
		{
			//Swap ( y1, y2 );
			//Swap ( Coord1, Coord2 );
			Temp = Coord1;
			Coord1 = Coord2;
			Coord2 = Temp;
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
		//r0 = gr [ Coord0 ];
		//r1 = gr [ Coord1 ];
		//r2 = gr [ Coord2 ];
		//g0 = gg [ Coord0 ];
		//g1 = gg [ Coord1 ];
		///g2 = gg [ Coord2 ];
		//b0 = gb [ Coord0 ];
		//b1 = gb [ Coord1 ];
		//b2 = gb [ Coord2 ];
		r0 = gbgr [ Coord0 ] & 0xff;
		r1 = gbgr [ Coord1 ] & 0xff;
		r2 = gbgr [ Coord2 ] & 0xff;
		g0 = ( gbgr [ Coord0 ] >> 8 ) & 0xff;
		g1 = ( gbgr [ Coord1 ] >> 8 ) & 0xff;
		g2 = ( gbgr [ Coord2 ] >> 8 ) & 0xff;
		b0 = ( gbgr [ Coord0 ] >> 16 ) & 0xff;
		b1 = ( gbgr [ Coord1 ] >> 16 ) & 0xff;
		b2 = ( gbgr [ Coord2 ] >> 16 ) & 0xff;
		
		//////////////////////////////////////////
		// get coordinates on screen
		x0 += DrawArea_OffsetX;
		y0 += DrawArea_OffsetY;
		x1 += DrawArea_OffsetX;
		y1 += DrawArea_OffsetY;
		x2 += DrawArea_OffsetX;
		y2 += DrawArea_OffsetY;
		
		
		
		// get the left/right most x
		LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
		LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
		RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
		RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

		
		
		
		/////////////////////////////////////////////////
		// draw top part of triangle
		
		// denominator is negative when x1 is on the left, positive when x1 is on the right
		t0 = y1 - y2;
		t1 = y0 - y2;
		denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );
		if ( denominator )
		{
			//dR_across = ( ( (s32) ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) ) << 6 ) / denominator;
			//dG_across = ( ( (s32) ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) ) << 6 ) / denominator;
			//dB_across = ( ( (s32) ( ( ( b0 - b2 ) * t0 ) - ( ( b1 - b2 ) * t1 ) ) ) << 6 ) / denominator;
			//dR_across <<= 10;
			//dG_across <<= 10;
			//dB_across <<= 10;
			
			
			//dR_across = divide_s32 ( ( (s32) ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) ) << 8, denominator );
			//dG_across = divide_s32 ( ( (s32) ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) ) << 8, denominator );
			//dB_across = divide_s32 ( ( (s32) ( ( ( b0 - b2 ) * t0 ) - ( ( b1 - b2 ) * t1 ) ) ) << 8, denominator );
			dR_across <<= 8;
			dG_across <<= 8;
			dB_across <<= 8;
			
			//printf ( "dR_across=%x top=%i bottom=%i divide=%x", dR_across, ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ), denominator, ( ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) << 16 )/denominator );
			//printf ( "dG_across=%x top=%i bottom=%i divide=%x", dG_across, ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ), denominator, ( ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) << 16 )/denominator );
		}
		
		
		
		
		//if ( denominator < 0 )
		//{
			// x1 is on the left and x0 is on the right //
			
			////////////////////////////////////
			// get slopes
			
			if ( y1 - y0 )
			{
				/////////////////////////////////////////////
				// init x on the left and right
				x_left = ( x0 << 16 );
				x_right = x_left;
				
				R_left = ( r0 << 16 );
				G_left = ( g0 << 16 );
				B_left = ( b0 << 16 );
				
				if ( denominator < 0 )
				{
					//dx_left = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
					//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					//dx_left = ( ( x1 - x0 ) << 16 ) / ( y1 - y0 );
					//dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					
					
					//dx_left = divide_s32( ( ( x1 - x0 ) << 16 ), ( y1 - y0 ) );
					//dx_right = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					//dR_left = (( r1 - r0 ) << 16 ) / ( y1 - y0 );
					//dG_left = (( g1 - g0 ) << 16 ) / ( y1 - y0 );
					//dB_left = (( b1 - b0 ) << 16 ) / ( y1 - y0 );
					
					
					//dR_left = divide_s32( (( r1 - r0 ) << 16 ), ( y1 - y0 ) );
					//dG_left = divide_s32( (( g1 - g0 ) << 16 ), ( y1 - y0 ) );
					//dB_left = divide_s32( (( b1 - b0 ) << 16 ), ( y1 - y0 ) );
				}
				else
				{
					//dx_right = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
					//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					//dx_right = ( ( x1 - x0 ) << 16 ) / ( y1 - y0 );
					//dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					
					
					//dx_right = divide_s32( ( ( x1 - x0 ) << 16 ), ( y1 - y0 ) );
					//dx_left = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					//dR_left = (( r2 - r0 ) << 16 ) / ( y2 - y0 );
					//dG_left = (( g2 - g0 ) << 16 ) / ( y2 - y0 );
					//dB_left = (( b2 - b0 ) << 16 ) / ( y2 - y0 );
					
					
					//dR_left = divide_s32( (( r2 - r0 ) << 16 ), ( y2 - y0 ) );
					//dG_left = divide_s32( (( g2 - g0 ) << 16 ), ( y2 - y0 ) );
					//dB_left = divide_s32( (( b2 - b0 ) << 16 ), ( y2 - y0 ) );
				}
			}
			else
			{
				if ( denominator < 0 )
				{
					// change x_left and x_right where y1 is on left
					x_left = ( x1 << 16 );
					x_right = ( x0 << 16 );
					
					//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
					//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					//dx_left = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
					//dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					
					
					//dx_left = divide_s32( ( ( x2 - x1 ) << 16 ), ( y2 - y1 ) );
					//dx_right = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					R_left = ( r1 << 16 );
					G_left = ( g1 << 16 );
					B_left = ( b1 << 16 );

					//dR_left = (( r2 - r1 ) << 16 ) / ( y2 - y1 );
					//dG_left = (( g2 - g1 ) << 16 ) / ( y2 - y1 );
					//dB_left = (( b2 - b1 ) << 16 ) / ( y2 - y1 );
					
					
					//dR_left = divide_s32( (( r2 - r1 ) << 16 ), ( y2 - y1 ) );
					//dG_left = divide_s32( (( g2 - g1 ) << 16 ), ( y2 - y1 ) );
					//dB_left = divide_s32( (( b2 - b1 ) << 16 ), ( y2 - y1 ) );
				}
				else
				{
					x_right = ( x1 << 16 );
					x_left = ( x0 << 16 );
				
					//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
					//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					//dx_right = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
					//dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					
					
					//dx_right = divide_s32( ( ( x2 - x1 ) << 16 ), ( y2 - y1 ) );
					//dx_left = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					R_left = ( r0 << 16 );
					G_left = ( g0 << 16 );
					B_left = ( b0 << 16 );
					
					//dR_left = (( r2 - r0 ) << 16 ) / ( y2 - y0 );
					//dG_left = (( g2 - g0 ) << 16 ) / ( y2 - y0 );
					//dB_left = (( b2 - b0 ) << 16 ) / ( y2 - y0 );
					
					
					//dR_left = divide_s32( (( r2 - r0 ) << 16 ), ( y2 - y0 ) );
					//dG_left = divide_s32( (( g2 - g0 ) << 16 ), ( y2 - y0 ) );
					//dB_left = divide_s32( (( b2 - b0 ) << 16 ), ( y2 - y0 ) );
				}
			}
		//}
		


	
		////////////////
		// *** TODO *** at this point area of full triangle can be calculated and the rest of the drawing can be put on another thread *** //
		
		
		
		// r,g,b values are not specified with a fractional part, so there must be an initial fractional part
		R_left |= ( 1 << 15 );
		G_left |= ( 1 << 15 );
		B_left |= ( 1 << 15 );
		
		
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
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY + 1;
		}

		
		// offset to get to this compute unit's scanline
		//group_yoffset = group_id - ( StartY % num_global_groups );
		//if ( group_yoffset < 0 )
		//{
		//	group_yoffset += num_global_groups;
		//}
		
		//printf( "x_left=%x x_right=%x dx_left=%i dx_right=%i R_left=%x G_left=%x B_left=%x OffsetX=%i OffsetY=%i",x_left,x_right,dx_left,dx_right,R_left,G_left,B_left, DrawArea_OffsetX, DrawArea_OffsetY );
		//printf( "x0=%i y0=%i x1=%i y1=%i x2=%i y2=%i r0=%i r1=%i r2=%i g0=%i g1=%i g2=%i b0=%i b1=%i b2=%i", x0, y0, x1, y1, x2, y2, r0, r1, r2, g0, g1, g2, b0, b1, b2 );
		//printf( "dR_across=%x dG_across=%x dB_across=%x", dR_across, dG_across, dB_across );

	}	// end if ( !local_id )
	
	

	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );

	
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

	// check if sprite is within draw area
	if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	// skip drawing if distance between vertices is greater than max allowed by GPU
	if ( ( abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
	{
		// skip drawing polygon
		return;
	}


	
	



	
	/////////////////////////////////////////////
	// init x on the left and right
	
	


	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	Roff_left = R_left + ( dR_left * (group_yoffset + yid) );
	Goff_left = G_left + ( dG_left * (group_yoffset + yid) );
	Boff_left = B_left + ( dB_left * (group_yoffset + yid) );
	
	//////////////////////////////////////////////
	// draw down to y1
	//for ( Line = StartY; Line < EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		
		// left point is included if points are equal
		StartX = ( xoff_left + 0xffffLL ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			
			iR = Roff_left;
			iG = Goff_left;
			iB = Boff_left;
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - xoff_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iR += dR_across * Temp;
				//iG += dG_across * Temp;
				//iB += dB_across * Temp;
			}
			
			iR += ( dR_across >> 8 ) * ( Temp >> 8 );
			iG += ( dG_across >> 8 ) * ( Temp >> 8 );
			iB += ( dB_across >> 8 ) * ( Temp >> 8 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + xid + ( Line << 10 ) ] );
			//DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			
			
			iR += ( dR_across * xid );
			iG += ( dG_across * xid );
			iB += ( dB_across * xid );

			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				if ( GPU_CTRL_Read_DTD )
				{
					//bgr = ( _Round( iR ) >> 32 ) | ( ( _Round( iG ) >> 32 ) << 8 ) | ( ( _Round( iB ) >> 32 ) << 16 );
					//bgr = ( _Round( iR ) >> 35 ) | ( ( _Round( iG ) >> 35 ) << 5 ) | ( ( _Round( iB ) >> 35 ) << 10 );
					//DitherValue = DitherLine [ x_across & 0x3 ];
					DitherValue = c_iDitherValues24 [ ( x_across & 3 ) + ( ( Line & 3 ) << 2 ) ];
					
					// perform dither
					//Red = iR + DitherValue;
					//Green = iG + DitherValue;
					//Blue = iB + DitherValue;
					Red = iR + DitherValue;
					Green = iG + DitherValue;
					Blue = iB + DitherValue;
					
					//Red = Clamp5 ( ( iR + DitherValue ) >> 27 );
					//Green = Clamp5 ( ( iG + DitherValue ) >> 27 );
					//Blue = Clamp5 ( ( iB + DitherValue ) >> 27 );
					
					// perform shift
					Red >>= ( 16 + 3 );
					Green >>= ( 16 + 3 );
					Blue >>= ( 16 + 3 );
					
					Red = clamp ( Red, 0, 0x1f );
					Green = clamp ( Green, 0, 0x1f );
					Blue = clamp ( Blue, 0, 0x1f );
				}
				else
				{
					Red = iR >> ( 16 + 3 );
					Green = iG >> ( 16 + 3 );
					Blue = iB >> ( 16 + 3 );
				}
					
				
					
					// if dithering, perform signed clamp to 5 bits
					//Red = AddSignedClamp<s64,5> ( Red );
					//Green = AddSignedClamp<s64,5> ( Green );
					//Blue = AddSignedClamp<s64,5> ( Blue );
					
					bgr_temp = ( Blue << 10 ) | ( Green << 5 ) | Red;
					
					// shade pixel color
				
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
					
					
					//bgr_temp = bgr;
		
					
					// semi-transparency
					if ( Command_ABE )
					{
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
					}
					
					// check if we should set mask bit when drawing
					bgr_temp |= SetPixelMask;

					
					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
						
					iR += ( dR_across * xinc );
					iG += ( dG_across * xinc );
					iB += ( dB_across * xinc );
				
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		
		/////////////////////////////////////
		// update x on left and right
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
		
		Roff_left += ( dR_left * yinc );
		Goff_left += ( dG_left * yinc );
		Boff_left += ( dB_left * yinc );
	}

	} // end if ( EndY > StartY )

	
	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );

	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	/////////////////////////////////////////////
	// init x on the left and right
	
	if ( !local_id )
	{
		//////////////////////////////////////////////////////
		// check if y1 is on the left or on the right
		if ( denominator < 0 )
		{
			x_left = ( x1 << 16 );

			x_right = ( x0 << 16 ) + ( dx_right * ( y1 - y0 ) );
			
			R_left = ( r1 << 16 );
			G_left = ( g1 << 16 );
			B_left = ( b1 << 16 );
			
			
			//if ( y2 - y1 )
			//{
				//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
				//dx_left = (( x2 - x1 ) << 16 ) / ( y2 - y1 );
			
			
				//dx_left = divide_s32( (( x2 - x1 ) << 16 ), ( y2 - y1 ) );
				
				//dR_left = ( ((s64)( r2 - r1 )) * r21 ) >> 24;
				//dG_left = ( ((s64)( g2 - g1 )) * r21 ) >> 24;
				//dB_left = ( ((s64)( b2 - b1 )) * r21 ) >> 24;
				//dR_left = (( r2 - r1 ) << 16 ) / ( y2 - y1 );
				//dG_left = (( g2 - g1 ) << 16 ) / ( y2 - y1 );
				//dB_left = (( b2 - b1 ) << 16 ) / ( y2 - y1 );
				
				
				//dR_left = divide_s32( (( r2 - r1 ) << 16 ), ( y2 - y1 ) );
				//dG_left = divide_s32( (( g2 - g1 ) << 16 ), ( y2 - y1 ) );
				//dB_left = divide_s32( (( b2 - b1 ) << 16 ), ( y2 - y1 ) );
			//}
		}
		else
		{
			x_right = ( x1 << 16 );

			x_left = ( x0 << 16 ) + ( dx_left * ( y1 - y0 ) );
			
			R_left = ( r0 << 16 ) + ( dR_left * ( y1 - y0 ) );
			G_left = ( g0 << 16 ) + ( dG_left * ( y1 - y0 ) );
			B_left = ( b0 << 16 ) + ( dB_left * ( y1 - y0 ) );
			
			//if ( y2 - y1 )
			//{
				//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
				//dx_right = (( x2 - x1 ) << 16 ) / ( y2 - y1 );
			
			
				//dx_right = divide_s32( (( x2 - x1 ) << 16 ), ( y2 - y1 ) );
				
			//}
		}


		R_left += ( 1 << 15 );
		G_left += ( 1 << 15 );
		B_left += ( 1 << 15 );

		

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
			
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY + 1;
		}
		
		// offset to get to this compute unit's scanline
		group_yoffset = group_id - ( StartY % num_global_groups );
		if ( group_yoffset < 0 )
		{
			group_yoffset += num_global_groups;
		}
	}

	
	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );

	
	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	Roff_left = R_left + ( dR_left * (group_yoffset + yid) );
	Goff_left = G_left + ( dG_left * (group_yoffset + yid) );
	Boff_left = B_left + ( dB_left * (group_yoffset + yid) );
	
	//////////////////////////////////////////////
	// draw down to y2
	//for ( Line = StartY; Line < EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		
		// left point is included if points are equal
		StartX = ( xoff_left + 0xffffLL ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			iR = Roff_left;
			iG = Goff_left;
			iB = Boff_left;
			
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - xoff_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iR += dR_across * Temp;
				//iG += dG_across * Temp;
				//iB += dB_across * Temp;
			}
			
			iR += ( dR_across >> 8 ) * ( Temp >> 8 );
			iG += ( dG_across >> 8 ) * ( Temp >> 8 );
			iB += ( dB_across >> 8 ) * ( Temp >> 8 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + xid + ( Line << 10 ) ] );
			//DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			//NumberOfPixelsDrawn += EndX - StartX + 1;
			
			iR += ( dR_across * xid );
			iG += ( dG_across * xid );
			iB += ( dB_across * xid );

			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				if ( GPU_CTRL_Read_DTD )
				{
					//bgr = ( _Round( iR ) >> 32 ) | ( ( _Round( iG ) >> 32 ) << 8 ) | ( ( _Round( iB ) >> 32 ) << 16 );
					//bgr = ( _Round( iR ) >> 35 ) | ( ( _Round( iG ) >> 35 ) << 5 ) | ( ( _Round( iB ) >> 35 ) << 10 );
					//DitherValue = DitherLine [ x_across & 0x3 ];
					DitherValue = c_iDitherValues24 [ ( x_across & 3 ) + ( ( Line & 3 ) << 2 ) ];
					
					// perform dither
					//Red = iR + DitherValue;
					//Green = iG + DitherValue;
					//Blue = iB + DitherValue;
					Red = iR + DitherValue;
					Green = iG + DitherValue;
					Blue = iB + DitherValue;
					
					//Red = Clamp5 ( ( iR + DitherValue ) >> 27 );
					//Green = Clamp5 ( ( iG + DitherValue ) >> 27 );
					//Blue = Clamp5 ( ( iB + DitherValue ) >> 27 );
					
					// perform shift
					Red >>= ( 16 + 3 );
					Green >>= ( 16 + 3 );
					Blue >>= ( 16 + 3 );
					
					Red = clamp ( Red, 0, 0x1f );
					Green = clamp ( Green, 0, 0x1f );
					Blue = clamp ( Blue, 0, 0x1f );
				}
				else
				{
					Red = iR >> ( 16 + 3 );
					Green = iG >> ( 16 + 3 );
					Blue = iB >> ( 16 + 3 );
				}
					
					bgr = ( Blue << 10 ) | ( Green << 5 ) | Red;
					
					// shade pixel color
				
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
					
					bgr_temp = bgr;
		
					// semi-transparency
					if ( Command_ABE )
					{
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
					}
					
					// check if we should set mask bit when drawing
					bgr_temp |= SetPixelMask;

					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;

					
				iR += ( dR_across * xinc );
				iG += ( dG_across * xinc );
				iB += ( dB_across * xinc );
				
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		/////////////////////////////////////
		// update x on left and right
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
		
		Roff_left += ( dR_left * yinc );
		Goff_left += ( dG_left * yinc );
		Boff_left += ( dB_left * yinc );
	}
	
	} // end if ( EndY > StartY )
		
}



kernel void DrawTriangle_Texture ( __global u16* VRAM, __global DATA_Write_Format* inputdata )
{
	const int local_id = get_local_id( 0 );
	const int group_id = get_group_id( 0 );
	const int num_local_threads = get_local_size ( 0 );
	const int num_global_groups = get_num_groups( 0 );
	
#ifdef SINGLE_SCANLINE_MODE
	const int xid = local_id;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
#endif


//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7:GetBGR24 ( Buffer [ 0 ] );
//8:GetXY0 ( Buffer [ 1 ] );
//9:GetUV0 ( Buffer [ 2 ] );
//9:GetCLUT ( Buffer [ 2 ] );
//10:GetXY1 ( Buffer [ 3 ] );
//11:GetUV1 ( Buffer [ 4 ] );
//11:GetTPAGE ( Buffer [ 4 ] );
//12:GetXY2 ( Buffer [ 5 ] );
//13:GetUV2 ( Buffer [ 6 ] );

	
	__local u32 GPU_CTRL_Read, GPU_CTRL_Read_ABR;
	__local s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	__local s32 DrawArea_OffsetX, DrawArea_OffsetY;
	__local u32 Command_ABE;
	__local u32 Command_TGE;
	//__local u32 GPU_CTRL_Read_DTD;
	
	__global u16 *ptr;
	
	__private s32 Temp;
	__local s32 LeftMostX, RightMostX;
	
	__private s32 StartX, EndX;
	__local s32 StartY, EndY;

	//s64 r10, r20, r21;
	
	__private s32 DitherValue;

	// new __local variables
	__local s32 x0, x1, x2, y0, y1, y2;
	__local s32 dx_left, dx_right;
	__local s32 x_left, x_right;
	__private s32 x_across;
	__private u32 bgr, bgr_temp;
	__private s32 Line;
	__local s32 t0, t1, denominator;

	// more __local variables for gradient triangle
	//__local s32 dR_left, dG_left, dB_left;
	//__local s32 dR_across, dG_across, dB_across;
	//__private s32 iR, iG, iB;
	//__local s32 R_left, G_left, B_left;
	//__private s32 Roff_left, Goff_left, Boff_left;
	//__local s32 r0, r1, r2, g0, g1, g2, b0, b1, b2;
	
	// __local variables for texture triangle
	__local s32 dU_left, dV_left;
	__local s32 dU_across, dV_across;
	__private s32 iU, iV;
	__local s32 U_left, V_left;
	__private s32 Uoff_left, Voff_left;
	__local s32 u0, u1, u2, v0, v1, v2;
	//__local s32 gu [ 3 ], gv [ 3 ];
	

	__local s32 gx [ 3 ], gy [ 3 ], gbgr [ 3 ];
	
	__private s32 xoff_left, xoff_right;
	
	__private s32 Red, Green, Blue;
	__private u32 DestPixel;
	__local u32 PixelMask, SetPixelMask;

	__local u32 Coord0, Coord1, Coord2;
	__local s32 group_yoffset;


	__local u32 color_add;
	
	__global u16 *ptr_texture, *ptr_clut;
	__local u32 clut_xoffset, clut_yoffset;
	__local u32 clut_x, clut_y, tpage_tx, tpage_ty, tpage_abr, tpage_tp, command_tge, command_abe, command_abr;
	
	__private u32 TexCoordX, TexCoordY;
	__local u32 Shift1, Shift2, And1, And2;
	__local u32 TextureOffset;

	__local u32 TWYTWH, TWXTWW, Not_TWH, Not_TWW;
	__local u32 TWX, TWY, TWW, TWH;
	
	
	
	// setup __local vars
	if ( !local_id )
	{
		// no bitmaps in opencl ??
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;
		
		gbgr [ 0 ] = inputdata [ 7 ].Value & 0x00ffffff;
		//gbgr [ 1 ] = inputdata [ 9 ].Value & 0x00ffffff;
		//gbgr [ 2 ] = inputdata [ 11 ].Value & 0x00ffffff;
		gx [ 0 ] = (s32) ( ( inputdata [ 8 ].x << 5 ) >> 5 );
		gy [ 0 ] = (s32) ( ( inputdata [ 8 ].y << 5 ) >> 5 );
		gx [ 1 ] = (s32) ( ( inputdata [ 10 ].x << 5 ) >> 5 );
		gy [ 1 ] = (s32) ( ( inputdata [ 10 ].y << 5 ) >> 5 );
		gx [ 2 ] = (s32) ( ( inputdata [ 12 ].x << 5 ) >> 5 );
		gy [ 2 ] = (s32) ( ( inputdata [ 12 ].y << 5 ) >> 5 );
		
		GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		Command_ABE = inputdata [ 7 ].Command & 2;
		Command_TGE = inputdata [ 7 ].Command & 1;
		

		// bits 0-5 in upper halfword
		clut_x = ( inputdata [ 9 ].Value >> ( 16 + 0 ) ) & 0x3f;
		clut_y = ( inputdata [ 9 ].Value >> ( 16 + 6 ) ) & 0x1ff;

		TWY = ( inputdata [ 4 ].Value >> 15 ) & 0x1f;
		TWX = ( inputdata [ 4 ].Value >> 10 ) & 0x1f;
		TWH = ( inputdata [ 4 ].Value >> 5 ) & 0x1f;
		TWW = inputdata [ 4 ].Value & 0x1f;

		
		// DTD is bit 9 in GPU_CTRL_Read
		//GPU_CTRL_Read_DTD = ( GPU_CTRL_Read >> 9 ) & 1;
		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		Coord0 = 0;
		Coord1 = 1;
		Coord2 = 2;
		
		// initialize number of pixels drawn
		//NumberOfPixelsDrawn = 0;
		
		// bits 0-3
		tpage_tx = GPU_CTRL_Read & 0xf;
		
		// bit 4
		tpage_ty = ( GPU_CTRL_Read >> 4 ) & 1;
		
		// bits 5-6
		GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		
		// bits 7-8
		tpage_tp = ( GPU_CTRL_Read >> 7 ) & 3;
		
		Shift1 = 0;
		Shift2 = 0;
		And1 = 0;
		And2 = 0;


		TWYTWH = ( ( TWY & TWH ) << 3 );
		TWXTWW = ( ( TWX & TWW ) << 3 );
		
		Not_TWH = ~( TWH << 3 );
		Not_TWW = ~( TWW << 3 );

		
		
		/////////////////////////////////////////////////////////
		// Get offset into texture page
		TextureOffset = ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 );
		
		clut_xoffset = clut_x << 4;
		
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
		
		
		
		
		///////////////////////////////////
		// put top coordinates in x0,y0
		//if ( y1 < y0 )
		if ( gy [ Coord1 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y1 );
			//Swap ( Coord0, Coord1 );
			Temp = Coord0;
			Coord0 = Coord1;
			Coord1 = Temp;
		}
		
		//if ( y2 < y0 )
		if ( gy [ Coord2 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y2 );
			//Swap ( Coord0, Coord2 );
			Temp = Coord0;
			Coord0 = Coord2;
			Coord2 = Temp;
		}
		
		///////////////////////////////////////
		// put middle coordinates in x1,y1
		//if ( y2 < y1 )
		if ( gy [ Coord2 ] < gy [ Coord1 ] )
		{
			//Swap ( y1, y2 );
			//Swap ( Coord1, Coord2 );
			Temp = Coord1;
			Coord1 = Coord2;
			Coord2 = Temp;
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
		//r0 = gbgr [ Coord0 ] & 0xff;
		//r1 = gbgr [ Coord1 ] & 0xff;
		//r2 = gbgr [ Coord2 ] & 0xff;
		//g0 = ( gbgr [ Coord0 ] >> 8 ) & 0xff;
		//g1 = ( gbgr [ Coord1 ] >> 8 ) & 0xff;
		//g2 = ( gbgr [ Coord2 ] >> 8 ) & 0xff;
		//b0 = ( gbgr [ Coord0 ] >> 16 ) & 0xff;
		//b1 = ( gbgr [ Coord1 ] >> 16 ) & 0xff;
		//b2 = ( gbgr [ Coord2 ] >> 16 ) & 0xff;
		// ?? convert to 16-bit ?? (or should leave 24-bit?)
		bgr = gbgr [ 0 ];
		bgr = ( ( bgr >> 9 ) & 0x7c00 ) | ( ( bgr >> 6 ) & 0x3e0 ) | ( ( bgr >> 3 ) & 0x1f );
		
		if ( ( bgr & 0x00ffffff ) == 0x00808080 ) Command_TGE = 1;
		
		color_add = bgr;
		
		//////////////////////////////////////////
		// get coordinates on screen
		x0 += DrawArea_OffsetX;
		y0 += DrawArea_OffsetY;
		x1 += DrawArea_OffsetX;
		y1 += DrawArea_OffsetY;
		x2 += DrawArea_OffsetX;
		y2 += DrawArea_OffsetY;
		
		
		
		// get the left/right most x
		LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
		LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
		RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
		RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

		
		
		
		/////////////////////////////////////////////////
		// draw top part of triangle
		
		// denominator is negative when x1 is on the left, positive when x1 is on the right
		t0 = y1 - y2;
		t1 = y0 - y2;
		denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );
		if ( denominator )
		{
			//dR_across = divide_s32 ( ( (s32) ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) ) << 8, denominator );
			//dG_across = divide_s32 ( ( (s32) ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) ) << 8, denominator );
			//dB_across = divide_s32 ( ( (s32) ( ( ( b0 - b2 ) * t0 ) - ( ( b1 - b2 ) * t1 ) ) ) << 8, denominator );
			//dR_across <<= 8;
			//dG_across <<= 8;
			//dB_across <<= 8;
			dU_across = divide_s32 ( ( (s32) ( ( ( u0 - u2 ) * t0 ) - ( ( u1 - u2 ) * t1 ) ) ) << 8, denominator );
			dV_across = divide_s32 ( ( (s32) ( ( ( v0 - v2 ) * t0 ) - ( ( v1 - v2 ) * t1 ) ) ) << 8, denominator );
			dU_across <<= 8;
			dV_across <<= 8;
			
			//printf ( "dR_across=%x top=%i bottom=%i divide=%x", dR_across, ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ), denominator, ( ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) << 16 )/denominator );
			//printf ( "dG_across=%x top=%i bottom=%i divide=%x", dG_across, ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ), denominator, ( ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) << 16 )/denominator );
		}
		
		
		
		
		//if ( denominator < 0 )
		//{
			// x1 is on the left and x0 is on the right //
			
			////////////////////////////////////
			// get slopes
			
			if ( y1 - y0 )
			{
				/////////////////////////////////////////////
				// init x on the left and right
				x_left = ( x0 << 16 );
				x_right = x_left;
				
				//R_left = ( r0 << 16 );
				//G_left = ( g0 << 16 );
				//B_left = ( b0 << 16 );

				U_left = ( u0 << 16 );
				V_left = ( v0 << 16 );
				
				if ( denominator < 0 )
				{
					//dx_left = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
					//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					//dx_left = ( ( x1 - x0 ) << 16 ) / ( y1 - y0 );
					//dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					dx_left = divide_s32( ( ( x1 - x0 ) << 16 ), ( y1 - y0 ) );
					dx_right = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					//dR_left = (( r1 - r0 ) << 16 ) / ( y1 - y0 );
					//dG_left = (( g1 - g0 ) << 16 ) / ( y1 - y0 );
					//dB_left = (( b1 - b0 ) << 16 ) / ( y1 - y0 );
					//dR_left = divide_s32( (( r1 - r0 ) << 16 ), ( y1 - y0 ) );
					//dG_left = divide_s32( (( g1 - g0 ) << 16 ), ( y1 - y0 ) );
					//dB_left = divide_s32( (( b1 - b0 ) << 16 ), ( y1 - y0 ) );
					
					dU_left = divide_s32( (( u1 - u0 ) << 16 ), ( y1 - y0 ) );
					dV_left = divide_s32( (( v1 - v0 ) << 16 ), ( y1 - y0 ) );
				}
				else
				{
					//dx_right = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
					//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					//dx_right = ( ( x1 - x0 ) << 16 ) / ( y1 - y0 );
					//dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					dx_right = divide_s32( ( ( x1 - x0 ) << 16 ), ( y1 - y0 ) );
					dx_left = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					//dR_left = (( r2 - r0 ) << 16 ) / ( y2 - y0 );
					//dG_left = (( g2 - g0 ) << 16 ) / ( y2 - y0 );
					//dB_left = (( b2 - b0 ) << 16 ) / ( y2 - y0 );
					//dR_left = divide_s32( (( r2 - r0 ) << 16 ), ( y2 - y0 ) );
					//dG_left = divide_s32( (( g2 - g0 ) << 16 ), ( y2 - y0 ) );
					//dB_left = divide_s32( (( b2 - b0 ) << 16 ), ( y2 - y0 ) );
					
					dU_left = divide_s32( (( u2 - u0 ) << 16 ), ( y2 - y0 ) );
					dV_left = divide_s32( (( v2 - v0 ) << 16 ), ( y2 - y0 ) );
				}
			}
			else
			{
				if ( denominator < 0 )
				{
					// change x_left and x_right where y1 is on left
					x_left = ( x1 << 16 );
					x_right = ( x0 << 16 );
					
					//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
					//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					//dx_left = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
					//dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					dx_left = divide_s32( ( ( x2 - x1 ) << 16 ), ( y2 - y1 ) );
					dx_right = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					//R_left = ( r1 << 16 );
					//G_left = ( g1 << 16 );
					//B_left = ( b1 << 16 );

					U_left = ( u1 << 16 );
					V_left = ( v1 << 16 );
					
					//dR_left = (( r2 - r1 ) << 16 ) / ( y2 - y1 );
					//dG_left = (( g2 - g1 ) << 16 ) / ( y2 - y1 );
					//dB_left = (( b2 - b1 ) << 16 ) / ( y2 - y1 );
					//dR_left = divide_s32( (( r2 - r1 ) << 16 ), ( y2 - y1 ) );
					//dG_left = divide_s32( (( g2 - g1 ) << 16 ), ( y2 - y1 ) );
					//dB_left = divide_s32( (( b2 - b1 ) << 16 ), ( y2 - y1 ) );
					
					dU_left = divide_s32( (( u2 - u1 ) << 16 ), ( y2 - y1 ) );
					dV_left = divide_s32( (( v2 - v1 ) << 16 ), ( y2 - y1 ) );
				}
				else
				{
					x_right = ( x1 << 16 );
					x_left = ( x0 << 16 );
				
					//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
					//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					//dx_right = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
					//dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					dx_right = divide_s32( ( ( x2 - x1 ) << 16 ), ( y2 - y1 ) );
					dx_left = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					//R_left = ( r0 << 16 );
					//G_left = ( g0 << 16 );
					//B_left = ( b0 << 16 );
					
					U_left = ( u0 << 16 );
					V_left = ( v0 << 16 );
					
					//dR_left = (( r2 - r0 ) << 16 ) / ( y2 - y0 );
					//dG_left = (( g2 - g0 ) << 16 ) / ( y2 - y0 );
					//dB_left = (( b2 - b0 ) << 16 ) / ( y2 - y0 );
					//dR_left = divide_s32( (( r2 - r0 ) << 16 ), ( y2 - y0 ) );
					//dG_left = divide_s32( (( g2 - g0 ) << 16 ), ( y2 - y0 ) );
					//dB_left = divide_s32( (( b2 - b0 ) << 16 ), ( y2 - y0 ) );
					
					dU_left = divide_s32( (( u2 - u0 ) << 16 ), ( y2 - y0 ) );
					dV_left = divide_s32( (( v2 - v0 ) << 16 ), ( y2 - y0 ) );
				}
			}
		//}
		


	
		////////////////
		// *** TODO *** at this point area of full triangle can be calculated and the rest of the drawing can be put on another thread *** //
		
		
		
		// r,g,b values are not specified with a fractional part, so there must be an initial fractional part
		//R_left |= ( 1 << 15 );
		//G_left |= ( 1 << 15 );
		//B_left |= ( 1 << 15 );

		U_left |= ( 1 << 15 );
		V_left |= ( 1 << 15 );
		
		
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
			
			//R_left += dR_left * Temp;
			//G_left += dG_left * Temp;
			//B_left += dB_left * Temp;
			
			U_left += dU_left * Temp;
			V_left += dV_left * Temp;
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY + 1;
		}

		
		// offset to get to this compute unit's scanline
		group_yoffset = group_id - ( StartY % num_global_groups );
		if ( group_yoffset < 0 )
		{
			group_yoffset += num_global_groups;
		}
		
		//printf( "x_left=%x x_right=%x dx_left=%i dx_right=%i R_left=%x G_left=%x B_left=%x OffsetX=%i OffsetY=%i",x_left,x_right,dx_left,dx_right,R_left,G_left,B_left, DrawArea_OffsetX, DrawArea_OffsetY );
		//printf( "x0=%i y0=%i x1=%i y1=%i x2=%i y2=%i r0=%i r1=%i r2=%i g0=%i g1=%i g2=%i b0=%i b1=%i b2=%i", x0, y0, x1, y1, x2, y2, r0, r1, r2, g0, g1, g2, b0, b1, b2 );
		//printf( "dR_across=%x dG_across=%x dB_across=%x", dR_across, dG_across, dB_across );

	}	// end if ( !local_id )
	
	

	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );

	
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

	// check if sprite is within draw area
	if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	// skip drawing if distance between vertices is greater than max allowed by GPU
	if ( ( abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
	{
		// skip drawing polygon
		return;
	}


	
	
	ptr_clut = & ( VRAM [ clut_y << 10 ] );
	ptr_texture = & ( VRAM [ ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) ] );

	



	
	/////////////////////////////////////////////
	// init x on the left and right
	
	


	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	//Roff_left = R_left + ( dR_left * (group_yoffset + yid) );
	//Goff_left = G_left + ( dG_left * (group_yoffset + yid) );
	//Boff_left = B_left + ( dB_left * (group_yoffset + yid) );
	
	Uoff_left = U_left + ( dU_left * (group_yoffset + yid) );
	Voff_left = V_left + ( dV_left * (group_yoffset + yid) );
	
	//////////////////////////////////////////////
	// draw down to y1
	//for ( Line = StartY; Line < EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		
		// left point is included if points are equal
		StartX = ( xoff_left + 0xffffLL ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			
			//iR = Roff_left;
			//iG = Goff_left;
			//iB = Boff_left;
			
			iU = Uoff_left;
			iV = Voff_left;
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - xoff_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
			}
			
			//iR += ( dR_across >> 8 ) * ( Temp >> 8 );
			//iG += ( dG_across >> 8 ) * ( Temp >> 8 );
			//iB += ( dB_across >> 8 ) * ( Temp >> 8 );
			
			iU += ( dU_across >> 8 ) * ( Temp >> 8 );
			iV += ( dV_across >> 8 ) * ( Temp >> 8 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + xid + ( Line << 10 ) ] );
			//DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			
			
			//iR += ( dR_across * xid );
			//iG += ( dG_across * xid );
			//iB += ( dB_across * xid );

			iU += ( dU_across * xid );
			iV += ( dV_across * xid );
			
			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				
				/*
				if ( GPU_CTRL_Read_DTD )
				{
					//bgr = ( _Round( iR ) >> 32 ) | ( ( _Round( iG ) >> 32 ) << 8 ) | ( ( _Round( iB ) >> 32 ) << 16 );
					//bgr = ( _Round( iR ) >> 35 ) | ( ( _Round( iG ) >> 35 ) << 5 ) | ( ( _Round( iB ) >> 35 ) << 10 );
					//DitherValue = DitherLine [ x_across & 0x3 ];
					DitherValue = c_iDitherValues24 [ ( x_across & 3 ) + ( ( Line & 3 ) << 2 ) ];
					
					// perform dither
					//Red = iR + DitherValue;
					//Green = iG + DitherValue;
					//Blue = iB + DitherValue;
					Red = iR + DitherValue;
					Green = iG + DitherValue;
					Blue = iB + DitherValue;
					
					//Red = Clamp5 ( ( iR + DitherValue ) >> 27 );
					//Green = Clamp5 ( ( iG + DitherValue ) >> 27 );
					//Blue = Clamp5 ( ( iB + DitherValue ) >> 27 );
					
					// perform shift
					Red >>= ( 16 + 3 );
					Green >>= ( 16 + 3 );
					Blue >>= ( 16 + 3 );
					
					Red = clamp ( Red, 0, 0x1f );
					Green = clamp ( Green, 0, 0x1f );
					Blue = clamp ( Blue, 0, 0x1f );
				}
				else
				{
					Red = iR >> ( 16 + 3 );
					Green = iG >> ( 16 + 3 );
					Blue = iB >> ( 16 + 3 );
				}
				
				color_add = ( Blue << 10 ) | ( Green << 5 ) | Red;
				*/

				TexCoordY = (u8) ( ( iV & Not_TWH ) | ( TWYTWH ) );
				TexCoordY <<= 10;

				//TexCoordX = (u8) ( ( iU & ~( TWW << 3 ) ) | ( ( TWX & TWW ) << 3 ) );
				TexCoordX = (u8) ( ( iU & Not_TWW ) | ( TWXTWW ) );
				
				//bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
				bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + TexCoordY ];
				
				if ( Shift1 )
				{
					//bgr = VRAM [ ( ( ( clut_x << 4 ) + TexelIndex ) & FrameBuffer_XMask ) + ( clut_y << 10 ) ];
					bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2 ) ) & c_lFrameBuffer_Width_Mask ];
				}

				
				if ( bgr )
				{
					
					// shade pixel color
					
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
					
					
					bgr_temp = bgr;
		
					if ( !Command_TGE )
					{
						// brightness calculation
						//bgr_temp = Color24To16 ( ColorMultiply24 ( Color16To24 ( bgr_temp ), color_add ) );
						bgr_temp = ColorMultiply1624 ( bgr_temp, color_add );
					}
					
					// semi-transparency
					if ( Command_ABE && ( bgr & 0x8000 ) )
					{
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
					}
					
					// check if we should set mask bit when drawing
					bgr_temp |= SetPixelMask | ( bgr & 0x8000 );

					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
					
				}
						
				//iR += ( dR_across * xinc );
				//iG += ( dG_across * xinc );
				//iB += ( dB_across * xinc );
			
				iU += ( dU_across * xinc );
				iV += ( dV_across * xinc );
					
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		
		/////////////////////////////////////
		// update x on left and right
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
		
		//Roff_left += ( dR_left * yinc );
		//Goff_left += ( dG_left * yinc );
		//Boff_left += ( dB_left * yinc );
		
		Uoff_left += ( dU_left * yinc );
		Voff_left += ( dV_left * yinc );
	}

	} // end if ( EndY > StartY )

	
	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );

	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	/////////////////////////////////////////////
	// init x on the left and right
	
	if ( !local_id )
	{
		//////////////////////////////////////////////////////
		// check if y1 is on the left or on the right
		if ( denominator < 0 )
		{
			x_left = ( x1 << 16 );

			x_right = ( x0 << 16 ) + ( dx_right * ( y1 - y0 ) );
			
			//R_left = ( r1 << 16 );
			//G_left = ( g1 << 16 );
			//B_left = ( b1 << 16 );
			
			U_left = ( u1 << 16 );
			V_left = ( v1 << 16 );
			
			//if ( y2 - y1 )
			//{
				//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
				//dx_left = (( x2 - x1 ) << 16 ) / ( y2 - y1 );
				dx_left = divide_s32( (( x2 - x1 ) << 16 ), ( y2 - y1 ) );
				
				//dR_left = (( r2 - r1 ) << 16 ) / ( y2 - y1 );
				//dG_left = (( g2 - g1 ) << 16 ) / ( y2 - y1 );
				//dB_left = (( b2 - b1 ) << 16 ) / ( y2 - y1 );
				//dR_left = divide_s32( (( r2 - r1 ) << 16 ), ( y2 - y1 ) );
				//dG_left = divide_s32( (( g2 - g1 ) << 16 ), ( y2 - y1 ) );
				//dB_left = divide_s32( (( b2 - b1 ) << 16 ), ( y2 - y1 ) );
				
				dU_left = divide_s32( (( u2 - u1 ) << 16 ), ( y2 - y1 ) );
				dV_left = divide_s32( (( v2 - v1 ) << 16 ), ( y2 - y1 ) );
			//}
		}
		else
		{
			x_right = ( x1 << 16 );

			x_left = ( x0 << 16 ) + ( dx_left * ( y1 - y0 ) );
			
			//R_left = ( r0 << 16 ) + ( dR_left * ( y1 - y0 ) );
			//G_left = ( g0 << 16 ) + ( dG_left * ( y1 - y0 ) );
			//B_left = ( b0 << 16 ) + ( dB_left * ( y1 - y0 ) );
			
			U_left = ( u0 << 16 ) + ( dU_left * ( y1 - y0 ) );
			V_left = ( v0 << 16 ) + ( dV_left * ( y1 - y0 ) );
			
			//if ( y2 - y1 )
			//{
				//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
				//dx_right = (( x2 - x1 ) << 16 ) / ( y2 - y1 );
				dx_right = divide_s32( (( x2 - x1 ) << 16 ), ( y2 - y1 ) );
				
			//}
		}


		//R_left += ( 1 << 15 );
		//G_left += ( 1 << 15 );
		//B_left += ( 1 << 15 );

		U_left += ( 1 << 15 );
		V_left += ( 1 << 15 );
		

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
			
			//R_left += dR_left * Temp;
			//G_left += dG_left * Temp;
			//B_left += dB_left * Temp;
			
			U_left += dU_left * Temp;
			V_left += dV_left * Temp;
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY + 1;
		}
		
		// offset to get to this compute unit's scanline
		group_yoffset = group_id - ( StartY % num_global_groups );
		if ( group_yoffset < 0 )
		{
			group_yoffset += num_global_groups;
		}
	}

	
	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );

	
	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	//Roff_left = R_left + ( dR_left * (group_yoffset + yid) );
	//Goff_left = G_left + ( dG_left * (group_yoffset + yid) );
	//Boff_left = B_left + ( dB_left * (group_yoffset + yid) );
	
	Uoff_left = U_left + ( dU_left * (group_yoffset + yid) );
	Voff_left = V_left + ( dV_left * (group_yoffset + yid) );
	
	//////////////////////////////////////////////
	// draw down to y2
	//for ( Line = StartY; Line < EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		
		// left point is included if points are equal
		StartX = ( xoff_left + 0xffffLL ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			//iR = Roff_left;
			//iG = Goff_left;
			//iB = Boff_left;
			
			iU = Uoff_left;
			iV = Voff_left;
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - xoff_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
			}
			
			//iR += ( dR_across >> 8 ) * ( Temp >> 8 );
			//iG += ( dG_across >> 8 ) * ( Temp >> 8 );
			//iB += ( dB_across >> 8 ) * ( Temp >> 8 );
			
			iU += ( dU_across >> 8 ) * ( Temp >> 8 );
			iV += ( dV_across >> 8 ) * ( Temp >> 8 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + xid + ( Line << 10 ) ] );
			//DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			
			
			
			//iR += ( dR_across * xid );
			//iG += ( dG_across * xid );
			//iB += ( dB_across * xid );

			iU += ( dU_across * xid );
			iV += ( dV_across * xid );
			
			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				
				/*
				if ( GPU_CTRL_Read_DTD )
				{
					//bgr = ( _Round( iR ) >> 32 ) | ( ( _Round( iG ) >> 32 ) << 8 ) | ( ( _Round( iB ) >> 32 ) << 16 );
					//bgr = ( _Round( iR ) >> 35 ) | ( ( _Round( iG ) >> 35 ) << 5 ) | ( ( _Round( iB ) >> 35 ) << 10 );
					//DitherValue = DitherLine [ x_across & 0x3 ];
					DitherValue = c_iDitherValues24 [ ( x_across & 3 ) + ( ( Line & 3 ) << 2 ) ];
					
					// perform dither
					//Red = iR + DitherValue;
					//Green = iG + DitherValue;
					//Blue = iB + DitherValue;
					Red = iR + DitherValue;
					Green = iG + DitherValue;
					Blue = iB + DitherValue;
					
					//Red = Clamp5 ( ( iR + DitherValue ) >> 27 );
					//Green = Clamp5 ( ( iG + DitherValue ) >> 27 );
					//Blue = Clamp5 ( ( iB + DitherValue ) >> 27 );
					
					// perform shift
					Red >>= ( 16 + 3 );
					Green >>= ( 16 + 3 );
					Blue >>= ( 16 + 3 );
					
					Red = clamp ( Red, 0, 0x1f );
					Green = clamp ( Green, 0, 0x1f );
					Blue = clamp ( Blue, 0, 0x1f );
				}
				else
				{
					Red = iR >> ( 16 + 3 );
					Green = iG >> ( 16 + 3 );
					Blue = iB >> ( 16 + 3 );
				}
				
				color_add = ( Blue << 10 ) | ( Green << 5 ) | Red;
				*/

				TexCoordY = (u8) ( ( iV & Not_TWH ) | ( TWYTWH ) );
				TexCoordY <<= 10;

				//TexCoordX = (u8) ( ( iU & ~( TWW << 3 ) ) | ( ( TWX & TWW ) << 3 ) );
				TexCoordX = (u8) ( ( iU & Not_TWW ) | ( TWXTWW ) );
				
				//bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
				bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + TexCoordY ];
				
				if ( Shift1 )
				{
					//bgr = VRAM [ ( ( ( clut_x << 4 ) + TexelIndex ) & FrameBuffer_XMask ) + ( clut_y << 10 ) ];
					bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2 ) ) & c_lFrameBuffer_Width_Mask ];
				}

				
				if ( bgr )
				{
					
					// shade pixel color
					
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
					
					
					bgr_temp = bgr;
		
					if ( !Command_TGE )
					{
						// brightness calculation
						//bgr_temp = Color24To16 ( ColorMultiply24 ( Color16To24 ( bgr_temp ), color_add ) );
						bgr_temp = ColorMultiply1624 ( bgr_temp, color_add );
					}
					
					// semi-transparency
					if ( Command_ABE && ( bgr & 0x8000 ) )
					{
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
					}
					
					// check if we should set mask bit when drawing
					bgr_temp |= SetPixelMask | ( bgr & 0x8000 );

					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
					
				}

					
				//iR += ( dR_across * xinc );
				//iG += ( dG_across * xinc );
				//iB += ( dB_across * xinc );
				
				iU += ( dU_across * xinc );
				iV += ( dV_across * xinc );
				
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		/////////////////////////////////////
		// update x on left and right
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
		
		//Roff_left += ( dR_left * yinc );
		//Goff_left += ( dG_left * yinc );
		//Boff_left += ( dB_left * yinc );
		
		Uoff_left += ( dU_left * yinc );
		Voff_left += ( dV_left * yinc );
	}
	
	} // end if ( EndY > StartY )
		
}







kernel void DrawTriangle_TextureGradient ( __global u16* VRAM, __global DATA_Write_Format* inputdata )
{
	const int local_id = get_local_id( 0 );
	const int group_id = get_group_id( 0 );
	const int num_local_threads = get_local_size ( 0 );
	const int num_global_groups = get_num_groups( 0 );
	
#ifdef SINGLE_SCANLINE_MODE
	const int xid = local_id;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
#endif


//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7:GetBGR0_8 ( Buffer [ 0 ] );
//8:GetXY0 ( Buffer [ 1 ] );
//9:GetCLUT ( Buffer [ 2 ] );
//9:GetUV0 ( Buffer [ 2 ] );
//10:GetBGR1_8 ( Buffer [ 3 ] );
//11:GetXY1 ( Buffer [ 4 ] );
//12:GetTPAGE ( Buffer [ 5 ] );
//12:GetUV1 ( Buffer [ 5 ] );
//13:GetBGR2_8 ( Buffer [ 6 ] );
//14:GetXY2 ( Buffer [ 7 ] );
//15:GetUV2 ( Buffer [ 8 ] );

	
	__local u32 GPU_CTRL_Read;
	//__local u32 GPU_CTRL_Read_ABR;
	__local s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	__local s32 DrawArea_OffsetX, DrawArea_OffsetY;
	__local u32 Command_ABE;
	__local u32 Command_TGE;
	__local u32 GPU_CTRL_Read_DTD;
	
	__global u16 *ptr;
	
	__private s32 Temp;
	__local s32 LeftMostX, RightMostX;
	
	__private s32 StartX, EndX;
	__local s32 StartY, EndY;

	__private s32 DitherValue;

	// new __local variables
	__local s32 x0, x1, x2, y0, y1, y2;
	__local s32 dx_left, dx_right;
	__local s32 x_left, x_right;
	__private s32 x_across;
	__private u32 bgr, bgr_temp;
	__private s32 Line;
	__local s32 t0, t1, denominator;

	// more __local variables for gradient triangle
	__local s32 dR_left, dG_left, dB_left;
	__local s32 dR_across, dG_across, dB_across;
	__private s32 iR, iG, iB;
	__local s32 R_left, G_left, B_left;
	__private s32 Roff_left, Goff_left, Boff_left;
	__local s32 r0, r1, r2, g0, g1, g2, b0, b1, b2;
	//__local s32 gr [ 3 ], gg [ 3 ], gb [ 3 ];
	
	// __local variables for texture triangle
	__local s32 dU_left, dV_left;
	__local s32 dU_across, dV_across;
	__private s32 iU, iV;
	__local s32 U_left, V_left;
	__private s32 Uoff_left, Voff_left;
	__local s32 u0, u1, u2, v0, v1, v2;
	//__local s32 gu [ 3 ], gv [ 3 ];
	

	__local s32 gx [ 3 ], gy [ 3 ], gbgr [ 3 ];
	
	__private s32 xoff_left, xoff_right;
	
	__private s32 Red, Green, Blue;
	__private u32 DestPixel;
	__local u32 PixelMask, SetPixelMask;

	__local u32 Coord0, Coord1, Coord2;
	__local s32 group_yoffset;


	__local u32 color_add;
	
	__global u16 *ptr_texture, *ptr_clut;
	__local u32 clut_xoffset, clut_yoffset;
	__local u32 clut_x, clut_y, tpage_tx, tpage_ty, tpage_abr, tpage_tp;
	
	__private u32 TexCoordX, TexCoordY;
	__local u32 Shift1, Shift2, And1, And2;
	__local u32 TextureOffset;

	__local u32 TWYTWH, TWXTWW, Not_TWH, Not_TWW;
	__local u32 TWX, TWY, TWW, TWH;
	
	
	
	// setup __local vars
	if ( !local_id )
	{
		// no bitmaps in opencl ??
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;
		
		gbgr [ 0 ] = inputdata [ 7 ].Value & 0x00ffffff;
		gbgr [ 1 ] = inputdata [ 10 ].Value & 0x00ffffff;
		gbgr [ 2 ] = inputdata [ 13 ].Value & 0x00ffffff;
		gx [ 0 ] = (s32) ( ( inputdata [ 8 ].x << 5 ) >> 5 );
		gy [ 0 ] = (s32) ( ( inputdata [ 8 ].y << 5 ) >> 5 );
		gx [ 1 ] = (s32) ( ( inputdata [ 11 ].x << 5 ) >> 5 );
		gy [ 1 ] = (s32) ( ( inputdata [ 11 ].y << 5 ) >> 5 );
		gx [ 2 ] = (s32) ( ( inputdata [ 14 ].x << 5 ) >> 5 );
		gy [ 2 ] = (s32) ( ( inputdata [ 14 ].y << 5 ) >> 5 );
		
		u0 = inputdata [ 9 ].u;
		v0 = inputdata [ 9 ].v;
		u1 = inputdata [ 12 ].u;
		v1 = inputdata [ 12 ].v;
		u2 = inputdata [ 15 ].u;
		v2 = inputdata [ 15 ].v;
		
		//GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		Command_ABE = inputdata [ 7 ].Command & 2;
		Command_TGE = inputdata [ 7 ].Command & 1;
		
		if ( ( bgr & 0x00ffffff ) == 0x00808080 ) Command_TGE = 1;

		// bits 0-5 in upper halfword
		clut_x = ( inputdata [ 9 ].Value >> ( 16 + 0 ) ) & 0x3f;
		clut_y = ( inputdata [ 9 ].Value >> ( 16 + 6 ) ) & 0x1ff;

		TWY = ( inputdata [ 4 ].Value >> 15 ) & 0x1f;
		TWX = ( inputdata [ 4 ].Value >> 10 ) & 0x1f;
		TWH = ( inputdata [ 4 ].Value >> 5 ) & 0x1f;
		TWW = inputdata [ 4 ].Value & 0x1f;

		
		// DTD is bit 9 in GPU_CTRL_Read
		GPU_CTRL_Read_DTD = ( GPU_CTRL_Read >> 9 ) & 1;
		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		Coord0 = 0;
		Coord1 = 1;
		Coord2 = 2;
		
		// initialize number of pixels drawn
		//NumberOfPixelsDrawn = 0;
		
		// bits 0-3
		//tpage_tx = GPU_CTRL_Read & 0xf;
		tpage_tx = ( inputdata [ 12 ].Value >> ( 16 + 0 ) ) & 0xf;
		
		// bit 4
		//tpage_ty = ( GPU_CTRL_Read >> 4 ) & 1
		tpage_ty = ( inputdata [ 12 ].Value >> ( 16 + 4 ) ) & 1;
		
		// bits 5-6
		//GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		tpage_abr = ( inputdata [ 12 ].Value >> ( 16 + 5 ) ) & 3;
		
		// bits 7-8
		//tpage_tp = ( GPU_CTRL_Read >> 7 ) & 3;
		tpage_tp = ( inputdata [ 12 ].Value >> ( 16 + 7 ) ) & 3;
		
		Shift1 = 0;
		Shift2 = 0;
		And1 = 0;
		And2 = 0;


		TWYTWH = ( ( TWY & TWH ) << 3 );
		TWXTWW = ( ( TWX & TWW ) << 3 );
		
		Not_TWH = ~( TWH << 3 );
		Not_TWW = ~( TWW << 3 );

		
		
		/////////////////////////////////////////////////////////
		// Get offset into texture page
		TextureOffset = ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 );
		
		clut_xoffset = clut_x << 4;
		
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
		
		
		
		
		///////////////////////////////////
		// put top coordinates in x0,y0
		//if ( y1 < y0 )
		if ( gy [ Coord1 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y1 );
			//Swap ( Coord0, Coord1 );
			Temp = Coord0;
			Coord0 = Coord1;
			Coord1 = Temp;
		}
		
		//if ( y2 < y0 )
		if ( gy [ Coord2 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y2 );
			//Swap ( Coord0, Coord2 );
			Temp = Coord0;
			Coord0 = Coord2;
			Coord2 = Temp;
		}
		
		///////////////////////////////////////
		// put middle coordinates in x1,y1
		//if ( y2 < y1 )
		if ( gy [ Coord2 ] < gy [ Coord1 ] )
		{
			//Swap ( y1, y2 );
			//Swap ( Coord1, Coord2 );
			Temp = Coord1;
			Coord1 = Coord2;
			Coord2 = Temp;
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
		r0 = gbgr [ Coord0 ] & 0xff;
		r1 = gbgr [ Coord1 ] & 0xff;
		r2 = gbgr [ Coord2 ] & 0xff;
		g0 = ( gbgr [ Coord0 ] >> 8 ) & 0xff;
		g1 = ( gbgr [ Coord1 ] >> 8 ) & 0xff;
		g2 = ( gbgr [ Coord2 ] >> 8 ) & 0xff;
		b0 = ( gbgr [ Coord0 ] >> 16 ) & 0xff;
		b1 = ( gbgr [ Coord1 ] >> 16 ) & 0xff;
		b2 = ( gbgr [ Coord2 ] >> 16 ) & 0xff;
		// ?? convert to 16-bit ?? (or should leave 24-bit?)
		//bgr = gbgr [ 0 ];
		//bgr = ( ( bgr >> 9 ) & 0x7c00 ) | ( ( bgr >> 6 ) & 0x3e0 ) | ( ( bgr >> 3 ) & 0x1f );
		
		//color_add = bgr;
		
		//////////////////////////////////////////
		// get coordinates on screen
		x0 += DrawArea_OffsetX;
		y0 += DrawArea_OffsetY;
		x1 += DrawArea_OffsetX;
		y1 += DrawArea_OffsetY;
		x2 += DrawArea_OffsetX;
		y2 += DrawArea_OffsetY;
		
		
		
		// get the left/right most x
		LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
		LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
		RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
		RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

		
		
		
		/////////////////////////////////////////////////
		// draw top part of triangle
		
		// denominator is negative when x1 is on the left, positive when x1 is on the right
		t0 = y1 - y2;
		t1 = y0 - y2;
		denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );
		if ( denominator )
		{
			dR_across = divide_s32 ( ( (s32) ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) ) << 8, denominator );
			dG_across = divide_s32 ( ( (s32) ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) ) << 8, denominator );
			dB_across = divide_s32 ( ( (s32) ( ( ( b0 - b2 ) * t0 ) - ( ( b1 - b2 ) * t1 ) ) ) << 8, denominator );
			
			dU_across = divide_s32 ( ( (s32) ( ( ( u0 - u2 ) * t0 ) - ( ( u1 - u2 ) * t1 ) ) ) << 8, denominator );
			dV_across = divide_s32 ( ( (s32) ( ( ( v0 - v2 ) * t0 ) - ( ( v1 - v2 ) * t1 ) ) ) << 8, denominator );
			
			dR_across <<= 8;
			dG_across <<= 8;
			dB_across <<= 8;
			
			dU_across <<= 8;
			dV_across <<= 8;
			
			//printf ( "dR_across=%x top=%i bottom=%i divide=%x", dR_across, ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ), denominator, ( ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) << 16 )/denominator );
			//printf ( "dG_across=%x top=%i bottom=%i divide=%x", dG_across, ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ), denominator, ( ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) << 16 )/denominator );
		}
		
		
		
		
		//if ( denominator < 0 )
		//{
			// x1 is on the left and x0 is on the right //
			
			////////////////////////////////////
			// get slopes
			
			if ( y1 - y0 )
			{
				/////////////////////////////////////////////
				// init x on the left and right
				x_left = ( x0 << 16 );
				x_right = x_left;
				
				R_left = ( r0 << 16 );
				G_left = ( g0 << 16 );
				B_left = ( b0 << 16 );

				U_left = ( u0 << 16 );
				V_left = ( v0 << 16 );
				
				if ( denominator < 0 )
				{
					//dx_left = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
					//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					//dx_left = ( ( x1 - x0 ) << 16 ) / ( y1 - y0 );
					//dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					dx_left = divide_s32( ( ( x1 - x0 ) << 16 ), ( y1 - y0 ) );
					dx_right = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					//dR_left = (( r1 - r0 ) << 16 ) / ( y1 - y0 );
					//dG_left = (( g1 - g0 ) << 16 ) / ( y1 - y0 );
					//dB_left = (( b1 - b0 ) << 16 ) / ( y1 - y0 );
					dR_left = divide_s32( (( r1 - r0 ) << 16 ), ( y1 - y0 ) );
					dG_left = divide_s32( (( g1 - g0 ) << 16 ), ( y1 - y0 ) );
					dB_left = divide_s32( (( b1 - b0 ) << 16 ), ( y1 - y0 ) );
					
					dU_left = divide_s32( (( u1 - u0 ) << 16 ), ( y1 - y0 ) );
					dV_left = divide_s32( (( v1 - v0 ) << 16 ), ( y1 - y0 ) );
				}
				else
				{
					//dx_right = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
					//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					//dx_right = ( ( x1 - x0 ) << 16 ) / ( y1 - y0 );
					//dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					dx_right = divide_s32( ( ( x1 - x0 ) << 16 ), ( y1 - y0 ) );
					dx_left = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					//dR_left = (( r2 - r0 ) << 16 ) / ( y2 - y0 );
					//dG_left = (( g2 - g0 ) << 16 ) / ( y2 - y0 );
					//dB_left = (( b2 - b0 ) << 16 ) / ( y2 - y0 );
					dR_left = divide_s32( (( r2 - r0 ) << 16 ), ( y2 - y0 ) );
					dG_left = divide_s32( (( g2 - g0 ) << 16 ), ( y2 - y0 ) );
					dB_left = divide_s32( (( b2 - b0 ) << 16 ), ( y2 - y0 ) );
					
					dU_left = divide_s32( (( u2 - u0 ) << 16 ), ( y2 - y0 ) );
					dV_left = divide_s32( (( v2 - v0 ) << 16 ), ( y2 - y0 ) );
				}
			}
			else
			{
				if ( denominator < 0 )
				{
					// change x_left and x_right where y1 is on left
					x_left = ( x1 << 16 );
					x_right = ( x0 << 16 );
					
					//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
					//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					//dx_left = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
					//dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					dx_left = divide_s32( ( ( x2 - x1 ) << 16 ), ( y2 - y1 ) );
					dx_right = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					R_left = ( r1 << 16 );
					G_left = ( g1 << 16 );
					B_left = ( b1 << 16 );

					U_left = ( u1 << 16 );
					V_left = ( v1 << 16 );
					
					//dR_left = (( r2 - r1 ) << 16 ) / ( y2 - y1 );
					//dG_left = (( g2 - g1 ) << 16 ) / ( y2 - y1 );
					//dB_left = (( b2 - b1 ) << 16 ) / ( y2 - y1 );
					dR_left = divide_s32( (( r2 - r1 ) << 16 ), ( y2 - y1 ) );
					dG_left = divide_s32( (( g2 - g1 ) << 16 ), ( y2 - y1 ) );
					dB_left = divide_s32( (( b2 - b1 ) << 16 ), ( y2 - y1 ) );
					
					dU_left = divide_s32( (( u2 - u1 ) << 16 ), ( y2 - y1 ) );
					dV_left = divide_s32( (( v2 - v1 ) << 16 ), ( y2 - y1 ) );
				}
				else
				{
					x_right = ( x1 << 16 );
					x_left = ( x0 << 16 );
				
					//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
					//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					//dx_right = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
					//dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					dx_right = divide_s32( ( ( x2 - x1 ) << 16 ), ( y2 - y1 ) );
					dx_left = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					R_left = ( r0 << 16 );
					G_left = ( g0 << 16 );
					B_left = ( b0 << 16 );
					
					U_left = ( u0 << 16 );
					V_left = ( v0 << 16 );
					
					//dR_left = (( r2 - r0 ) << 16 ) / ( y2 - y0 );
					//dG_left = (( g2 - g0 ) << 16 ) / ( y2 - y0 );
					//dB_left = (( b2 - b0 ) << 16 ) / ( y2 - y0 );
					dR_left = divide_s32( (( r2 - r0 ) << 16 ), ( y2 - y0 ) );
					dG_left = divide_s32( (( g2 - g0 ) << 16 ), ( y2 - y0 ) );
					dB_left = divide_s32( (( b2 - b0 ) << 16 ), ( y2 - y0 ) );
					
					dU_left = divide_s32( (( u2 - u0 ) << 16 ), ( y2 - y0 ) );
					dV_left = divide_s32( (( v2 - v0 ) << 16 ), ( y2 - y0 ) );
				}
			}
		//}
		


	
		////////////////
		// *** TODO *** at this point area of full triangle can be calculated and the rest of the drawing can be put on another thread *** //
		
		
		
		// r,g,b values are not specified with a fractional part, so there must be an initial fractional part
		R_left |= ( 1 << 15 );
		G_left |= ( 1 << 15 );
		B_left |= ( 1 << 15 );

		U_left |= ( 1 << 15 );
		V_left |= ( 1 << 15 );
		
		
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
			
			U_left += dU_left * Temp;
			V_left += dV_left * Temp;
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY + 1;
		}

		
		// offset to get to this compute unit's scanline
		group_yoffset = group_id - ( StartY % num_global_groups );
		if ( group_yoffset < 0 )
		{
			group_yoffset += num_global_groups;
		}
		
		//printf( "x_left=%x x_right=%x dx_left=%i dx_right=%i R_left=%x G_left=%x B_left=%x OffsetX=%i OffsetY=%i",x_left,x_right,dx_left,dx_right,R_left,G_left,B_left, DrawArea_OffsetX, DrawArea_OffsetY );
		//printf( "x0=%i y0=%i x1=%i y1=%i x2=%i y2=%i r0=%i r1=%i r2=%i g0=%i g1=%i g2=%i b0=%i b1=%i b2=%i", x0, y0, x1, y1, x2, y2, r0, r1, r2, g0, g1, g2, b0, b1, b2 );
		//printf( "dR_across=%x dG_across=%x dB_across=%x", dR_across, dG_across, dB_across );

	}	// end if ( !local_id )
	
	

	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );

	
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

	// check if sprite is within draw area
	if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	// skip drawing if distance between vertices is greater than max allowed by GPU
	if ( ( abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
	{
		// skip drawing polygon
		return;
	}


	
	
	ptr_clut = & ( VRAM [ clut_y << 10 ] );
	ptr_texture = & ( VRAM [ ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) ] );

	



	
	/////////////////////////////////////////////
	// init x on the left and right
	
	


	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	Roff_left = R_left + ( dR_left * (group_yoffset + yid) );
	Goff_left = G_left + ( dG_left * (group_yoffset + yid) );
	Boff_left = B_left + ( dB_left * (group_yoffset + yid) );
	
	Uoff_left = U_left + ( dU_left * (group_yoffset + yid) );
	Voff_left = V_left + ( dV_left * (group_yoffset + yid) );
	
	//////////////////////////////////////////////
	// draw down to y1
	//for ( Line = StartY; Line < EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		// left point is included if points are equal
		StartX = ( xoff_left + 0xffffLL ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			iR = Roff_left;
			iG = Goff_left;
			iB = Boff_left;
			
			iU = Uoff_left;
			iV = Voff_left;
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - xoff_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
			}
			
			iR += ( dR_across >> 8 ) * ( Temp >> 8 );
			iG += ( dG_across >> 8 ) * ( Temp >> 8 );
			iB += ( dB_across >> 8 ) * ( Temp >> 8 );
			
			iU += ( dU_across >> 8 ) * ( Temp >> 8 );
			iV += ( dV_across >> 8 ) * ( Temp >> 8 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + xid + ( Line << 10 ) ] );
			//DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			
			
			iR += ( dR_across * xid );
			iG += ( dG_across * xid );
			iB += ( dB_across * xid );

			iU += ( dU_across * xid );
			iV += ( dV_across * xid );
			
			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				TexCoordY = (u8) ( ( iV & Not_TWH ) | ( TWYTWH ) );
				TexCoordY <<= 10;

				//TexCoordX = (u8) ( ( iU & ~( TWW << 3 ) ) | ( ( TWX & TWW ) << 3 ) );
				TexCoordX = (u8) ( ( iU & Not_TWW ) | ( TWXTWW ) );
				
				//bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
				bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + TexCoordY ];
				
				if ( Shift1 )
				{
					//bgr = VRAM [ ( ( ( clut_x << 4 ) + TexelIndex ) & FrameBuffer_XMask ) + ( clut_y << 10 ) ];
					bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2 ) ) & c_lFrameBuffer_Width_Mask ];
				}

				
				if ( bgr )
				{
					
					// shade pixel color
					
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
					
					
					bgr_temp = bgr;
		
					if ( !Command_TGE )
					{
						if ( GPU_CTRL_Read_DTD )
						{
							DitherValue = c_iDitherValues24 [ ( x_across & 3 ) + ( ( Line & 3 ) << 2 ) ];
							
							// perform dither
							Red = iR + DitherValue;
							Green = iG + DitherValue;
							Blue = iB + DitherValue;
							
							// perform shift
							Red >>= ( 16 + 3 );
							Green >>= ( 16 + 3 );
							Blue >>= ( 16 + 3 );
							
							Red = clamp ( Red, 0, 0x1f );
							Green = clamp ( Green, 0, 0x1f );
							Blue = clamp ( Blue, 0, 0x1f );
						}
						else
						{
							Red = iR >> ( 16 + 3 );
							Green = iG >> ( 16 + 3 );
							Blue = iB >> ( 16 + 3 );
						}
						
						color_add = ( Blue << 10 ) | ( Green << 5 ) | Red;
						
						// brightness calculation
						//bgr_temp = Color24To16 ( ColorMultiply24 ( Color16To24 ( bgr_temp ), color_add ) );
						bgr_temp = ColorMultiply1624 ( bgr_temp, color_add );
					}
					
					// semi-transparency
					if ( Command_ABE && ( bgr & 0x8000 ) )
					{
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, tpage_abr );
					}
					
					// check if we should set mask bit when drawing
					bgr_temp |= SetPixelMask | ( bgr & 0x8000 );

					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
					
				}
						
				iR += ( dR_across * xinc );
				iG += ( dG_across * xinc );
				iB += ( dB_across * xinc );
			
				iU += ( dU_across * xinc );
				iV += ( dV_across * xinc );
					
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		
		/////////////////////////////////////
		// update x on left and right
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
		
		Roff_left += ( dR_left * yinc );
		Goff_left += ( dG_left * yinc );
		Boff_left += ( dB_left * yinc );
		
		Uoff_left += ( dU_left * yinc );
		Voff_left += ( dV_left * yinc );
	}

	} // end if ( EndY > StartY )

	
	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );

	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	/////////////////////////////////////////////
	// init x on the left and right
	
	if ( !local_id )
	{
		//////////////////////////////////////////////////////
		// check if y1 is on the left or on the right
		if ( denominator < 0 )
		{
			x_left = ( x1 << 16 );

			x_right = ( x0 << 16 ) + ( dx_right * ( y1 - y0 ) );
			
			R_left = ( r1 << 16 );
			G_left = ( g1 << 16 );
			B_left = ( b1 << 16 );
			
			U_left = ( u1 << 16 );
			V_left = ( v1 << 16 );
			
			//if ( y2 - y1 )
			//{
				//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
				//dx_left = (( x2 - x1 ) << 16 ) / ( y2 - y1 );
				dx_left = divide_s32( (( x2 - x1 ) << 16 ), ( y2 - y1 ) );
				
				//dR_left = ( ((s64)( r2 - r1 )) * r21 ) >> 24;
				//dG_left = ( ((s64)( g2 - g1 )) * r21 ) >> 24;
				//dB_left = ( ((s64)( b2 - b1 )) * r21 ) >> 24;
				//dR_left = (( r2 - r1 ) << 16 ) / ( y2 - y1 );
				//dG_left = (( g2 - g1 ) << 16 ) / ( y2 - y1 );
				//dB_left = (( b2 - b1 ) << 16 ) / ( y2 - y1 );
				dR_left = divide_s32( (( r2 - r1 ) << 16 ), ( y2 - y1 ) );
				dG_left = divide_s32( (( g2 - g1 ) << 16 ), ( y2 - y1 ) );
				dB_left = divide_s32( (( b2 - b1 ) << 16 ), ( y2 - y1 ) );
				
				dU_left = divide_s32( (( u2 - u1 ) << 16 ), ( y2 - y1 ) );
				dV_left = divide_s32( (( v2 - v1 ) << 16 ), ( y2 - y1 ) );
			//}
		}
		else
		{
			x_right = ( x1 << 16 );

			x_left = ( x0 << 16 ) + ( dx_left * ( y1 - y0 ) );
			
			R_left = ( r0 << 16 ) + ( dR_left * ( y1 - y0 ) );
			G_left = ( g0 << 16 ) + ( dG_left * ( y1 - y0 ) );
			B_left = ( b0 << 16 ) + ( dB_left * ( y1 - y0 ) );
			
			U_left = ( u0 << 16 ) + ( dU_left * ( y1 - y0 ) );
			V_left = ( v0 << 16 ) + ( dV_left * ( y1 - y0 ) );
			
			//if ( y2 - y1 )
			//{
				//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
				//dx_right = (( x2 - x1 ) << 16 ) / ( y2 - y1 );
				dx_right = divide_s32( (( x2 - x1 ) << 16 ), ( y2 - y1 ) );
				
			//}
		}


		R_left += ( 1 << 15 );
		G_left += ( 1 << 15 );
		B_left += ( 1 << 15 );

		U_left += ( 1 << 15 );
		V_left += ( 1 << 15 );
		

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
			
			U_left += dU_left * Temp;
			V_left += dV_left * Temp;
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY + 1;
		}
		
		// offset to get to this compute unit's scanline
		group_yoffset = group_id - ( StartY % num_global_groups );
		if ( group_yoffset < 0 )
		{
			group_yoffset += num_global_groups;
		}
	}

	
	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );

	
	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	Roff_left = R_left + ( dR_left * (group_yoffset + yid) );
	Goff_left = G_left + ( dG_left * (group_yoffset + yid) );
	Boff_left = B_left + ( dB_left * (group_yoffset + yid) );
	
	Uoff_left = U_left + ( dU_left * (group_yoffset + yid) );
	Voff_left = V_left + ( dV_left * (group_yoffset + yid) );
	
	//////////////////////////////////////////////
	// draw down to y2
	//for ( Line = StartY; Line < EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		
		// left point is included if points are equal
		StartX = ( xoff_left + 0xffffLL ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			iR = Roff_left;
			iG = Goff_left;
			iB = Boff_left;
			
			iU = Uoff_left;
			iV = Voff_left;
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - xoff_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
			}
			
			iR += ( dR_across >> 8 ) * ( Temp >> 8 );
			iG += ( dG_across >> 8 ) * ( Temp >> 8 );
			iB += ( dB_across >> 8 ) * ( Temp >> 8 );
			
			iU += ( dU_across >> 8 ) * ( Temp >> 8 );
			iV += ( dV_across >> 8 ) * ( Temp >> 8 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + xid + ( Line << 10 ) ] );
			//DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			//NumberOfPixelsDrawn += EndX - StartX + 1;
			
			iR += ( dR_across * xid );
			iG += ( dG_across * xid );
			iB += ( dB_across * xid );

			iU += ( dU_across * xid );
			iV += ( dV_across * xid );
			
			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				TexCoordY = (u8) ( ( iV & Not_TWH ) | ( TWYTWH ) );
				TexCoordY <<= 10;

				//TexCoordX = (u8) ( ( iU & ~( TWW << 3 ) ) | ( ( TWX & TWW ) << 3 ) );
				TexCoordX = (u8) ( ( iU & Not_TWW ) | ( TWXTWW ) );
				
				//bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
				bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + TexCoordY ];
				
				if ( Shift1 )
				{
					//bgr = VRAM [ ( ( ( clut_x << 4 ) + TexelIndex ) & FrameBuffer_XMask ) + ( clut_y << 10 ) ];
					bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2 ) ) & c_lFrameBuffer_Width_Mask ];
				}

				
				if ( bgr )
				{
					
					// shade pixel color
					
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
					
					
					bgr_temp = bgr;
		
					if ( !Command_TGE )
					{
						if ( GPU_CTRL_Read_DTD )
						{
							DitherValue = c_iDitherValues24 [ ( x_across & 3 ) + ( ( Line & 3 ) << 2 ) ];
							
							// perform dither
							Red = iR + DitherValue;
							Green = iG + DitherValue;
							Blue = iB + DitherValue;
							
							// perform shift
							Red >>= ( 16 + 3 );
							Green >>= ( 16 + 3 );
							Blue >>= ( 16 + 3 );
							
							Red = clamp ( Red, 0, 0x1f );
							Green = clamp ( Green, 0, 0x1f );
							Blue = clamp ( Blue, 0, 0x1f );
						}
						else
						{
							Red = iR >> ( 16 + 3 );
							Green = iG >> ( 16 + 3 );
							Blue = iB >> ( 16 + 3 );
						}
						
						color_add = ( Blue << 10 ) | ( Green << 5 ) | Red;
					
						// brightness calculation
						//bgr_temp = Color24To16 ( ColorMultiply24 ( Color16To24 ( bgr_temp ), color_add ) );
						bgr_temp = ColorMultiply1624 ( bgr_temp, color_add );
					}
					
					// semi-transparency
					if ( Command_ABE && ( bgr & 0x8000 ) )
					{
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, tpage_abr );
					}
					
					// check if we should set mask bit when drawing
					bgr_temp |= SetPixelMask | ( bgr & 0x8000 );

					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
					
				}

					
				iR += ( dR_across * xinc );
				iG += ( dG_across * xinc );
				iB += ( dB_across * xinc );
				
				iU += ( dU_across * xinc );
				iV += ( dV_across * xinc );
				
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		/////////////////////////////////////
		// update x on left and right
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
		
		Roff_left += ( dR_left * yinc );
		Goff_left += ( dG_left * yinc );
		Boff_left += ( dB_left * yinc );
		
		Uoff_left += ( dU_left * yinc );
		Voff_left += ( dV_left * yinc );
	}
	
	} // end if ( EndY > StartY )
		
}




kernel void DrawSprite ( __global u16* VRAM, __global DATA_Write_Format* inputdata )
{
	const int local_id = get_local_id( 0 );
	const int group_id = get_group_id( 0 );
	const int num_local_threads = get_local_size ( 0 );
	const int num_global_groups = get_num_groups( 0 );
	
#ifdef SINGLE_SCANLINE_MODE
	const int xid = local_id;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
#endif


//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_BottomRightX
//2: DrawArea_TopLeftX
//3: DrawArea_BottomRightY
//4: DrawArea_TopLeftY
//5: DrawArea_OffsetX
//6: DrawArea_OffsetY
//7: TextureWindow
//-------------------------
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: TextureWindow
//5: ------------
//6: ------------
//7: GetBGR24 ( Buffer [ 0 ] );
//8: GetXY ( Buffer [ 1 ] );
//9: GetCLUT ( Buffer [ 2 ] );
//9: GetUV ( Buffer [ 2 ] );
//10: GetHW ( Buffer [ 3 ] );


	// notes: looks like sprite size is same as specified by w/h

	//u32 Pixel,

	__local u32 GPU_CTRL_Read, GPU_CTRL_Read_ABR;
	__local s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	__local s32 DrawArea_OffsetX, DrawArea_OffsetY;
	__local u32 Command_ABE;
	__local u32 Command_TGE;

	
	__private u32 TexelIndex;
	
	
	__global u16 *ptr;
	__local s32 StartX, EndX, StartY, EndY;
	__local s32 x, y, w, h;
	
	__local s32 group_yoffset;
	
	__local u32 Temp;
	
	// new __local variables
	__local s32 x0, x1, y0, y1;
	__local s32 u0, v0;
	__local s32 u, v;
	__private u32 bgr, bgr_temp;
	__private s32 iU, iV;
	__private s32 x_across;
	__private s32 Line;
	
	__private u32 DestPixel;
	__local u32 PixelMask, SetPixelMask;

	
	__local u32 color_add;
	
	__global u16 *ptr_texture, *ptr_clut;
	__local u32 clut_xoffset, clut_yoffset;
	__local u32 clut_x, clut_y, tpage_tx, tpage_ty, tpage_abr, tpage_tp, command_tge, command_abe, command_abr;
	
	__private u32 TexCoordX, TexCoordY;
	__local u32 Shift1, Shift2, And1, And2;
	__local u32 TextureOffset;

	__local u32 TWYTWH, TWXTWW, Not_TWH, Not_TWW;
	__local u32 TWX, TWY, TWW, TWH;
	

	// set __local variables
	if ( !local_id )
	{
		// set bgr64
		//bgr64 = gbgr [ 0 ];
		//bgr64 |= ( bgr64 << 16 );
		//bgr64 |= ( bgr64 << 32 );
		bgr = inputdata [ 7 ].Value & 0x00ffffff;
		//bgr = ( ( bgr >> 9 ) & 0x7c00 ) | ( ( bgr >> 6 ) & 0x3e0 ) | ( ( bgr >> 3 ) & 0x1f );


		
		Command_ABE = inputdata [ 7 ].Command & 2;
		Command_TGE = inputdata [ 7 ].Command & 1;
		
		if ( ( bgr & 0x00ffffff ) == 0x00808080 ) Command_TGE = 1;

		
		x = inputdata [ 8 ].x;
		y = inputdata [ 8 ].y;
		
		// x and y are actually 11 bits
		x = ( x << ( 5 + 16 ) ) >> ( 5 + 16 );
		y = ( y << ( 5 + 16 ) ) >> ( 5 + 16 );
		
		w = inputdata [ 10 ].w;
		h = inputdata [ 10 ].h;
		
		u = inputdata [ 9 ].u;
		v = inputdata [ 9 ].v;
		
		// bits 0-5 in upper halfword
		clut_x = ( inputdata [ 9 ].Value >> ( 16 + 0 ) ) & 0x3f;
		clut_y = ( inputdata [ 9 ].Value >> ( 16 + 6 ) ) & 0x1ff;
	
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;
		
		
		TWY = ( inputdata [ 4 ].Value >> 15 ) & 0x1f;
		TWX = ( inputdata [ 4 ].Value >> 10 ) & 0x1f;
		TWH = ( inputdata [ 4 ].Value >> 5 ) & 0x1f;
		TWW = inputdata [ 4 ].Value & 0x1f;

		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		// bits 0-3
		tpage_tx = GPU_CTRL_Read & 0xf;
		
		// bit 4
		tpage_ty = ( GPU_CTRL_Read >> 4 ) & 1;
		
		// bits 5-6
		GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		
		// bits 7-8
		tpage_tp = ( GPU_CTRL_Read >> 7 ) & 3;
		
		Shift1 = 0;
		Shift2 = 0;
		And1 = 0;
		And2 = 0;


		TWYTWH = ( ( TWY & TWH ) << 3 );
		TWXTWW = ( ( TWX & TWW ) << 3 );
		
		Not_TWH = ~( TWH << 3 );
		Not_TWW = ~( TWW << 3 );

		
		
		/////////////////////////////////////////////////////////
		// Get offset into texture page
		TextureOffset = ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 );
		

		
		//////////////////////////////////////////////////////
		// Get offset into color lookup table
		//u32 ClutOffset = ( clut_x << 4 ) + ( clut_y << 10 );
		
		clut_xoffset = clut_x << 4;
		
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
		
		
		color_add = bgr;
		
		
		
		// get top left corner of sprite and bottom right corner of sprite
		x0 = x + DrawArea_OffsetX;
		y0 = y + DrawArea_OffsetY;
		x1 = x0 + w - 1;
		y1 = y0 + h - 1;

		
		// get texture coords
		u0 = u;
		v0 = v;
		
		
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

		
		
		
		// initialize number of pixels drawn
		//NumberOfPixelsDrawn = 0;
		
		

		// offset to get to this compute unit's scanline
		group_yoffset = group_id - ( StartY % num_global_groups );
		if ( group_yoffset < 0 )
		{
			group_yoffset += num_global_groups;
		}
		
//printf( "StartX= %i EndX= %i StartY= %i EndY= %i x= %i y= %i w= %i h=%i", StartX, EndX, StartY, EndY, x, y, w, h );
	}

	
	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );
	

	// initialize number of pixels drawn
	//NumberOfPixelsDrawn = 0;
	
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
	
	
	// check if sprite is within draw area
	if ( x1 < ((s32)DrawArea_TopLeftX) || x0 > ((s32)DrawArea_BottomRightX) || y1 < ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	
	//NumberOfPixelsDrawn = ( EndX - StartX + 1 ) * ( EndY - StartY + 1 );
		

	ptr_clut = & ( VRAM [ clut_y << 10 ] );
	ptr_texture = & ( VRAM [ ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) ] );

	
	//iV = v0;
	iV = v0 + group_yoffset + yid;

	//for ( Line = StartY; Line <= EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line <= EndY; Line += yinc )
	{
			// need to start texture coord from left again
			//iU = u0;
			iU = u0 + xid;

			TexCoordY = (u8) ( ( iV & Not_TWH ) | ( TWYTWH ) );
			TexCoordY <<= 10;

			//ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			ptr = & ( VRAM [ StartX + xid + ( Line << 10 ) ] );
			

			// draw horizontal line
			//for ( x_across = StartX; x_across <= EndX; x_across += xinc )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				//TexCoordX = (u8) ( ( iU & ~( TWW << 3 ) ) | ( ( TWX & TWW ) << 3 ) );
				TexCoordX = (u8) ( ( iU & Not_TWW ) | ( TWXTWW ) );
				
				//bgr = VRAM [ TextureOffset + ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
				//bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
				bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + TexCoordY ];
				
				if ( Shift1 )
				{
					//TexelIndex = ( ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2 );
					//bgr = VRAM [ ( ( ( clut_x << 4 ) + TexelIndex ) & FrameBuffer_XMask ) + ( clut_y << 10 ) ];
					bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2 ) ) & c_lFrameBuffer_Width_Mask ];
				}

				
				if ( bgr )
				{
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;	//VRAM [ x_across + ( Line << 10 ) ];
					
					bgr_temp = bgr;
		
					if ( !Command_TGE )
					{
						// brightness calculation
						//bgr_temp = Color24To16 ( ColorMultiply24 ( Color16To24 ( bgr_temp ), color_add ) );
						bgr_temp = ColorMultiply1624 ( bgr_temp, color_add );
					}
					
					// semi-transparency
					if ( Command_ABE && ( bgr & 0x8000 ) )
					{
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
					}
					
					// check if we should set mask bit when drawing
					bgr_temp |= SetPixelMask | ( bgr & 0x8000 );

					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
				}
					
				/////////////////////////////////////////////////////////
				// interpolate texture coords across
				//iU += c_iVectorSize;
				iU += xinc;
				
				// update pointer for pixel out
				//ptr += c_iVectorSize;
				ptr += xinc;
					
			}
		
		/////////////////////////////////////////////////////////
		// interpolate texture coords down
		//iV++;	//+= dV_left;
		iV += yinc;
	}
}


kernel void draw_screen( __global u16* VRAM, __global DATA_Write_Format* inputdata, __global u32* PixelBuffer )
{
	const int local_id = get_local_id( 0 );
	const int group_id = get_group_id( 0 );
	const int num_local_threads = get_local_size ( 0 );
	const int num_global_groups = get_num_groups( 0 );
	
#ifdef SINGLE_SCANLINE_MODE
	const int xid = local_id;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
	const int group_yoffset = group_id;
#endif


//inputdata format:
//0: GPU_CTRL_Read
//1: DisplayRange_Horizontal
//2: DisplayRange_Vertical
//3: ScreenArea_TopLeft
//4: bEnableScanline
//5: Y_Pixel
//6: -----------
//7: Command



	

	__local u32 GPU_CTRL_Read;
	__local u32 DisplayRange_X1;
	__local u32 DisplayRange_X2;
	__local u32 DisplayRange_Y1;
	__local u32 DisplayRange_Y2;
	__local u32 ScreenArea_TopLeftX;
	__local u32 ScreenArea_TopLeftY;
	__local u32 bEnableScanline;
	__local u32 Y_Pixel;

	
	// so the max viewable width for PAL is 3232/4-544/4 = 808-136 = 672
	// so the max viewable height for PAL is 292-34 = 258
	
	// actually, will initially start with a 1 pixel border based on screen width/height and then will shift if something is off screen

	// need to know visible range of screen for NTSC and for PAL (each should be different)
	// NTSC visible y range is usually from 16-256 (0x10-0x100) (height=240)
	// PAL visible y range is usually from 35-291 (0x23-0x123) (height=256)
	// NTSC visible x range is.. I don't know. start with from about gpu cycle#544 to about gpu cycle#3232 (must use gpu cycles since res changes)
	__local s32 VisibleArea_StartX, VisibleArea_EndX, VisibleArea_StartY, VisibleArea_EndY, VisibleArea_Width, VisibleArea_Height;
	
	// there the frame buffer pixel, and then there's the screen buffer pixel
	__private u32 Pixel16, Pixel32_0, Pixel32_1;
	//__private u64 Pixel64;
	
	
	// this allows you to calculate horizontal pixels
	__local u32 GPU_CyclesPerPixel;
	
	
	__private Pixel_24bit_Format Pixel24;
	
	
	__private s32 FramePixel_X, FramePixel_Y;
	
	// need to know where to draw the actual image at
	__local s32 Draw_StartX, Draw_StartY, Draw_EndX, Draw_EndY, Draw_Width, Draw_Height;
	
	__local s32 Source_Height;
	
	
	__global u16* ptr_vram16;
	__global u32* ptr_pixelbuffer32;
	
	__local s32 TopBorder_Height, BottomBorder_Height, LeftBorder_Width, RightBorder_Width;
	__private s32 current_x, current_y;
	__local s32 current_xmax, current_ymax;

	
	__local u32 GPU_CTRL_Read_ISINTER;
	__local u32 GPU_CTRL_Read_HEIGHT;
	__local u32 GPU_CTRL_Read_WIDTH;
	__local u32 GPU_CTRL_Read_DEN;
	__local u32 GPU_CTRL_Read_ISRGB24;
	__local u32 GPU_CTRL_Read_VIDEO;

		
	
	
	
	if ( !local_id )
	{
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DisplayRange_X1 = inputdata [ 1 ].Value & 0xfff;
		DisplayRange_X2 = ( inputdata [ 1 ].Value >> 12 ) & 0xfff;
		DisplayRange_Y1 = inputdata [ 2 ].Value & 0x3ff;
		DisplayRange_Y2 = ( inputdata [ 2 ].Value >> 10 ) & 0x7ff;
		ScreenArea_TopLeftX = inputdata [ 3 ].Value & 0x3ff;
		ScreenArea_TopLeftY = ( inputdata [ 3 ].Value >> 10 ) & 0x1ff;
		bEnableScanline = inputdata [ 4 ].Value;
		Y_Pixel = inputdata [ 5 ].Value;

		
		// bits 16-18
		GPU_CTRL_Read_WIDTH = ( GPU_CTRL_Read >> 16 ) & 7;
		
		// bit 19
		GPU_CTRL_Read_HEIGHT = ( GPU_CTRL_Read >> 19 ) & 1;
		
		// bit 20
		GPU_CTRL_Read_VIDEO = ( GPU_CTRL_Read >> 20 ) & 1;
		
		// bit 21
		GPU_CTRL_Read_ISRGB24 = ( GPU_CTRL_Read >> 21 ) & 1;
		
		// bit 22
		GPU_CTRL_Read_ISINTER = ( GPU_CTRL_Read >> 22 ) & 1;
		
		// bit 23
		GPU_CTRL_Read_DEN = ( GPU_CTRL_Read >> 23 ) & 1;
		
		
		// GPU cycles per pixel depends on width
		GPU_CyclesPerPixel = c_iGPUCyclesPerPixel [ GPU_CTRL_Read_WIDTH ];

		// get the pixel to start and stop drawing at
		Draw_StartX = DisplayRange_X1 / GPU_CyclesPerPixel;
		Draw_EndX = DisplayRange_X2 / GPU_CyclesPerPixel;
		Draw_StartY = DisplayRange_Y1;
		Draw_EndY = DisplayRange_Y2;

		Draw_Width = Draw_EndX - Draw_StartX;
		Draw_Height = Draw_EndY - Draw_StartY;
		// get the pixel to start and stop at for visible area
		VisibleArea_StartX = c_iVisibleArea_StartX_Cycle / GPU_CyclesPerPixel;
		VisibleArea_EndX = c_iVisibleArea_EndX_Cycle / GPU_CyclesPerPixel;

		// visible area start and end y depends on pal/ntsc
		VisibleArea_StartY = c_iVisibleArea_StartY [ GPU_CTRL_Read_VIDEO ];
		VisibleArea_EndY = c_iVisibleArea_EndY [ GPU_CTRL_Read_VIDEO ];

		VisibleArea_Width = VisibleArea_EndX - VisibleArea_StartX;
		VisibleArea_Height = VisibleArea_EndY - VisibleArea_StartY;


		Source_Height = Draw_Height;

		if ( GPU_CTRL_Read_ISINTER && GPU_CTRL_Read_HEIGHT )
		{
			// 480i mode //
			
			// if not simulating scanlines, then the draw height should double too
			if ( !bEnableScanline )
			{
				VisibleArea_EndY += Draw_Height;
				VisibleArea_Height += Draw_Height;
				
				Draw_EndY += Draw_Height;
				
				Draw_Height <<= 1;
			}
			
			Source_Height <<= 1;
		}
	
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; GPU_CyclesPerPixel=" << dec << GPU_CyclesPerPixel << " Draw_StartX=" << Draw_StartX << " Draw_EndX=" << Draw_EndX;
	debug << "\r\nDraw_StartY=" << Draw_StartY << " Draw_EndY=" << Draw_EndY << " VisibleArea_StartX=" << VisibleArea_StartX;
	debug << "\r\nVisibleArea_EndX=" << VisibleArea_EndX << " VisibleArea_StartY=" << VisibleArea_StartY << " VisibleArea_EndY=" << VisibleArea_EndY;
#endif

		
		
		if ( !GPU_CTRL_Read_DEN )
		{
			BottomBorder_Height = VisibleArea_EndY - Draw_EndY;
			LeftBorder_Width = Draw_StartX - VisibleArea_StartX;
			TopBorder_Height = Draw_StartY - VisibleArea_StartY;
			RightBorder_Width = VisibleArea_EndX - Draw_EndX;
			
			if ( BottomBorder_Height < 0 ) BottomBorder_Height = 0;
			if ( LeftBorder_Width < 0 ) LeftBorder_Width = 0;
			
			//cout << "\n(before)Left=" << dec << LeftBorder_Width << " Bottom=" << BottomBorder_Height << " Draw_Width=" << Draw_Width << " VisibleArea_Width=" << VisibleArea_Width;
			
			
			current_ymax = Draw_Height + BottomBorder_Height;
			current_xmax = Draw_Width + LeftBorder_Width;
			
			// make suree that ymax and xmax are not greater than the size of visible area
			if ( current_xmax > VisibleArea_Width )
			{
				// entire image is not on the screen, so take from left border and recalc xmax //

				LeftBorder_Width -= ( current_xmax - VisibleArea_Width );
				if ( LeftBorder_Width < 0 ) LeftBorder_Width = 0;
				current_xmax = Draw_Width + LeftBorder_Width;
				
				// make sure again we do not draw past the edge of screen
				if ( current_xmax > VisibleArea_Width ) current_xmax = VisibleArea_Width;
			}
			
			if ( current_ymax > VisibleArea_Height )
			{
				BottomBorder_Height -= ( current_ymax - VisibleArea_Height );
				if ( BottomBorder_Height < 0 ) BottomBorder_Height = 0;
				current_ymax = Draw_Height + BottomBorder_Height;
				
				// make sure again we do not draw past the edge of screen
				if ( current_ymax > VisibleArea_Height ) current_ymax = VisibleArea_Height;
			}
			
		}	// end if ( !GPU_CTRL_Read_DEN )
		
	}	// end if ( !local_id )

	
	// synchronize __local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );
	

	// *** new stuff *** //

	//FramePixel = 0;
	ptr_pixelbuffer32 = PixelBuffer;
	//ptr_pixelbuffer64 = (u64*) PixelBuffer;

	
		//cout << "\n(after)Left=" << dec << LeftBorder_Width << " Bottom=" << BottomBorder_Height << " Draw_Width=" << Draw_Width << " VisibleArea_Width=" << VisibleArea_Width;
		//cout << "\n(after2)current_xmax=" << current_xmax << " current_ymax=" << current_ymax;
		
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; Drawing bottom border";
#endif

	if ( !GPU_CTRL_Read_DEN )
	{
		// current_y should start at zero for even field and one for odd
		//current_y = 0;
		current_y = group_yoffset + yid;
		
		// added for opencl, need to start in pixel buffer on the right line
		ptr_pixelbuffer32 += ( VisibleArea_Width * ( group_yoffset + yid ) );
		
		// put in bottom border //
		
		
		// check if scanlines simulation is enabled
		if ( bEnableScanline )
		{
			// spread out workers on every other line
			ptr_pixelbuffer32 += ( VisibleArea_Width * ( group_yoffset + yid ) );
			
			// if this is an odd field, then start writing on the next line
			if ( Y_Pixel & 1 )
			{
				// odd field //
				
				ptr_pixelbuffer32 += VisibleArea_Width;
			}
		}
		

		while ( current_y < BottomBorder_Height )
		{
			//current_x = 0;
			current_x = xid;
			
			while ( current_x < VisibleArea_Width )
			{
				// *ptr_pixelbuffer32++ = 0;
				ptr_pixelbuffer32 [ current_x ] = 0;
				
				//current_x++;
				current_x += xinc;
			}
			
			//current_y++;
			current_y += yinc;
			
			// added for opencl, update pixel buffer multiple lines
			ptr_pixelbuffer32 += ( VisibleArea_Width * yinc );
			
			// check if scanline simulation is enabled
			if ( bEnableScanline )
			{
				// update again since doing every other line
				//ptr_pixelbuffer32 += VisibleArea_Width;
				ptr_pixelbuffer32 += ( VisibleArea_Width * yinc );
			}
		}

#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; Putting in screen";
	debug << " current_ymax=" << dec << current_ymax;
	debug << " current_xmax=" << current_xmax;
	debug << " VisibleArea_Width=" << VisibleArea_Width;
	debug << " VisibleArea_Height=" << VisibleArea_Height;
	debug << " LeftBorder_Width=" << LeftBorder_Width;
#endif
		
		// put in screen
		
		
		FramePixel_Y = ScreenArea_TopLeftY + Source_Height - 1;
		FramePixel_X = ScreenArea_TopLeftX;


//if ( !__global_id )
//{
//	printf( "FramePixel_X= %i FramePixel_Y= %i", FramePixel_X, FramePixel_Y );
//}

		
		// for opencl, spread the workers across the lines
		//FramePixel_Y -= group_yoffset + yid;
		FramePixel_Y -= ( current_y - BottomBorder_Height );
		
		// check if simulating scanlines
		if ( bEnableScanline )
		{
			// check if 480i
			if ( GPU_CTRL_Read_ISINTER && GPU_CTRL_Read_HEIGHT )
			{
				// 480i //
				
				// for opencl, spread interlace mode to every other line
				//FramePixel_Y -= group_yoffset + yid;
				FramePixel_Y -= ( current_y - BottomBorder_Height );
				
				// check if in odd field or even field
				if ( Y_Pixel & 1 )
				{
					// odd field //
					
					// if the height is even, then it is ok
					// if the height is odd, need to compensate
					if ( ! ( Source_Height & 1 ) )
					{
						FramePixel_Y--;
					}
				}
				else
				{
					// even field //
					
					// if the height is odd, then it is ok
					// if the height is even, need to compensate
					if ( Source_Height & 1 )
					{
						FramePixel_Y--;
					}
				}
				
			} // end if ( GPU_CTRL_Read.ISINTER && GPU_CTRL_Read.HEIGHT )
		}
		
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; drawing screen pixels";
	debug << " current_x=" << dec << current_x;
	debug << " FramePixel_X=" << FramePixel_X;
	debug << " FramePixel_Y=" << FramePixel_Y;
#endif
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\ncheck: current_x=" << current_x;
	debug << " current_xmax=" << current_xmax;
	debug << " ptr_vram32=" << ( (u64) ptr_vram32 );
	debug << " VRAM=" << ( (u64) VRAM );
	debug << " ptr_pixelbuffer64=" << ( (u64) ptr_pixelbuffer64 );
	debug << " PixelBuffer=" << ( (u64) PixelBuffer );
#endif




	
		while ( current_y < current_ymax )
		{
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; drawing left border";
	debug << " current_y=" << dec << current_y;
#endif
			// put in the left border
			//current_x = 0;
			current_x = xid;

			while ( current_x < LeftBorder_Width )
			{
				// *ptr_pixelbuffer32++ = 0;
				ptr_pixelbuffer32 [ current_x ] = 0;
				
				//current_x++;
				current_x += xinc;
			}
			
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; drawing screen pixels";
	debug << " current_x=" << dec << current_x;
	debug << " FramePixel_X=" << FramePixel_X;
	debug << " FramePixel_Y=" << FramePixel_Y;
#endif

			// *** important note *** this wraps around the VRAM
			ptr_vram16 = & (VRAM [ FramePixel_X + ( ( FramePixel_Y & c_lFrameBuffer_Height_Mask ) << 10 ) ]);
			
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; got vram ptr";
#endif

			// put in screeen pixels
			if ( !GPU_CTRL_Read_ISRGB24 )
			{
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; !ISRGB24";
#endif

				while ( current_x < current_xmax )
				{
//#ifdef INLINE_DEBUG_DRAW_SCREEN
//	debug << "\r\ndrawx1; current_x=" << current_x;
//#endif

					//Pixel16 = *ptr_vram16++;
					Pixel16 = ptr_vram16 [ current_x - LeftBorder_Width ];
					
					// the previous pixel conversion is wrong
					Pixel32_0 = ( ( Pixel16 & 0x1f ) << 3 ) | ( ( Pixel16 & 0x3e0 ) << 6 ) | ( ( Pixel16 & 0x7c00 ) << 9 );
					
					// *ptr_pixelbuffer32++ = Pixel32_0;
					ptr_pixelbuffer32 [ current_x ] = Pixel32_0;
					
					
					//current_x++;
					current_x += xinc;
				}
			}
			else
			{
				while ( current_x < current_xmax )
				{
					//Pixel24.Pixel0 = *ptr_vram16++;
					//Pixel24.Pixel1 = *ptr_vram16++;
					//Pixel24.Pixel2 = *ptr_vram16++;
					Pixel24.Pixel0 = ptr_vram16 [ ( ( ( current_x - LeftBorder_Width ) >> 1 ) * 3 ) + 0 ];
					Pixel24.Pixel1 = ptr_vram16 [ ( ( ( current_x - LeftBorder_Width ) >> 1 ) * 3 ) + 1 ];
					Pixel24.Pixel2 = ptr_vram16 [ ( ( ( current_x - LeftBorder_Width ) >> 1 ) * 3 ) + 2 ];
					
					// draw first pixel
					Pixel32_0 = ( ((u32)Pixel24.Red0) ) | ( ((u32)Pixel24.Green0) << 8 ) | ( ((u32)Pixel24.Blue0) << 16 );
					
					// draw second pixel
					Pixel32_1 = ( ((u32)Pixel24.Red1) ) | ( ((u32)Pixel24.Green1) << 8 ) | ( ((u32)Pixel24.Blue1) << 16 );
					
					// *ptr_pixelbuffer32++ = Pixel32_0;
					// *ptr_pixelbuffer32++ = Pixel32_1;
					ptr_pixelbuffer32 [ current_x ] = Pixel32_0;
					ptr_pixelbuffer32 [ current_x + 1 ] = Pixel32_1;
					
					//current_x += 2;
					current_x += ( xinc << 1 );
				}
			}
			
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; drawing right border";
	debug << " current_x=" << dec << current_x;
#endif

			// put in right border
			while ( current_x < VisibleArea_Width )
			{
				// *ptr_pixelbuffer32++ = 0;
				ptr_pixelbuffer32 [ current_x ] = 0;
				
				//current_x++;
				current_x += xinc;
			}
			
			
			//current_y++;
			current_y += yinc;
			
			// for opencl, update pixel buffer to next line
			ptr_pixelbuffer32 += ( VisibleArea_Width * yinc );
			
			if ( bEnableScanline )
			{
				// check if this is 480i
				if ( GPU_CTRL_Read_ISINTER && GPU_CTRL_Read_HEIGHT )
				{
					// 480i mode //
					
					// jump two lines in source image
					//FramePixel_Y -= 2;
					FramePixel_Y -= ( yinc << 1 );
				}
				else
				{
					// go to next line in frame buffer
					//FramePixel_Y--;
					FramePixel_Y -= yinc;
				}
				
				// also go to next line in destination buffer
				//ptr_pixelbuffer32 += VisibleArea_Width;
				ptr_pixelbuffer32 += ( VisibleArea_Width * yinc );
			}
			else
			{
				// go to next line in frame buffer
				//FramePixel_Y--;
				FramePixel_Y -= yinc;
			}
			
			
		} // end while ( current_y < current_ymax )
		
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; Drawing top border";
#endif

		// put in top border //
		

		while ( current_y < VisibleArea_Height )
		{
			//current_x = 0;
			current_x = xid;
			
			while ( current_x < VisibleArea_Width )
			{
				// *ptr_pixelbuffer32++ = 0;
				ptr_pixelbuffer32 [ current_x ] = 0;
				
				//current_x++;
				current_x += xinc;
			} // end while ( current_x < VisibleArea_Width )
				
			//current_y++;
			current_y += yinc;
				
			// for opencl, update pixel buffer to next line
			ptr_pixelbuffer32 += ( VisibleArea_Width * yinc );
			
			// check if scanline simulation is enabled
			if ( bEnableScanline )
			{
				// also go to next line in destination buffer
				//ptr_pixelbuffer32 += VisibleArea_Width;
				ptr_pixelbuffer32 += ( VisibleArea_Width * yinc );
			}
			
		} // end while ( current_y < current_ymax )
	}
	else
	{
		// display disabled //
		

		//current_y = 0;
		current_y = group_yoffset + yid;
		
		// set initial row for pixel buffer pointer
		ptr_pixelbuffer32 += ( VisibleArea_Width * current_y );
		
		if ( bEnableScanline )
		{
			// space out to every other line
			ptr_pixelbuffer32 += ( VisibleArea_Width * current_y );
			
			if ( Y_Pixel & 1 )
			{
				// odd field //
				
				ptr_pixelbuffer32 += VisibleArea_Width;
			}
		}
		
		while ( current_y < VisibleArea_Height )
		{
			//current_x = 0;
			current_x = xid;
			
			while ( current_x < VisibleArea_Width )
			{
				// *ptr_pixelbuffer32++ = 0;
				ptr_pixelbuffer32 [ current_x ] = 0;
				
				//current_x++;
				current_x += xinc;
			}
			
			//current_y++;
			current_y += yinc;
			
			// for opencl, update pixel buffer to next line
			ptr_pixelbuffer32 += ( VisibleArea_Width * yinc );
			
			if ( bEnableScanline )
			{
				//ptr_pixelbuffer32 += VisibleArea_Width;
				ptr_pixelbuffer32 += ( VisibleArea_Width * yinc );
			}
		}

	}
}


/*
kernel void ps1_gfx_test( __global u16* VRAM, __global DATA_Write_Format* inputdata, __global u32* PixelBuffer, __global u32* Dummy )
{
	const int local_id = get_local_id( 0 );
	
	
	//const int __global_id = get___global_id( 0 );
	//
	//if ( !__global_id )
	//{
	//	printf( "\nkernel inputdata= %x %x %x %x", inputdata [ 0 ].Value, inputdata [ 1 ].Value, inputdata [ 2 ].Value, inputdata [ 3 ].Value );
	//	printf( "\nkernel vram= %x %x %x %x", VRAM [ 0 ], VRAM [ 1 ], VRAM [ 2 ], VRAM [ 3 ] );
	//}
	
	
	// get the command
	switch ( inputdata [ 7 ].Command )
	{
		case 0x01:
			draw_screen( VRAM, inputdata, PixelBuffer );
			break;
			
		case 0x02:
			Draw_FrameBufferRectangle_02( VRAM, inputdata );
			break;
			
		// monochrome triangle
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
			DrawTriangle_Mono( VRAM, inputdata );
			break;
			
		// textured triangle
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
			DrawTriangle_Texture ( VRAM, inputdata );
			break;
		
		
		//7:GetBGR24 ( Buffer [ 0 ] );
		//8:GetXY0 ( Buffer [ 1 ] );
		//9:GetXY1 ( Buffer [ 2 ] );
		//10:GetXY2 ( Buffer [ 3 ] );
		//11:GetXY3 ( Buffer [ 4 ] );
		// monochrome rectangle
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		
			DrawTriangle_Mono( VRAM, inputdata );
			
			if ( !local_id )
			{
				inputdata [ 8 ].Value = inputdata [ 11 ].Value;
			}
			
			DrawTriangle_Mono( VRAM, inputdata );
			break;
			
			
		//7:GetBGR24 ( Buffer [ 0 ] );
		//8:GetXY0 ( Buffer [ 1 ] );
		//9:GetCLUT ( Buffer [ 2 ] );
		//9:GetUV0 ( Buffer [ 2 ] );
		//10:GetXY1 ( Buffer [ 3 ] );
		//11:GetTPAGE ( Buffer [ 4 ] );
		//11:GetUV1 ( Buffer [ 4 ] );
		//12:GetXY2 ( Buffer [ 5 ] );
		//13:GetUV2 ( Buffer [ 6 ] );
		//14:GetXY3 ( Buffer [ 7 ] );
		//15:GetUV3 ( Buffer [ 8 ] );
		// textured rectangle
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:
			DrawTriangle_Texture ( VRAM, inputdata );
			
			if ( !local_id )
			{
				inputdata [ 8 ].Value = inputdata [ 14 ].Value;
				inputdata [ 9 ].Value = ( inputdata [ 9 ].Value & ~0xffff ) | ( inputdata [ 15 ].Value & 0xffff );
			}
			
			DrawTriangle_Texture ( VRAM, inputdata );
			break;
			
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
			DrawTriangle_Gradient ( VRAM, inputdata );
			break;
			
		// texture gradient triangle
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
			DrawTriangle_TextureGradient ( VRAM, inputdata );
			break;
			
		//7:GetBGR0_8 ( Buffer [ 0 ] );
		//8:GetXY0 ( Buffer [ 1 ] );
		//9:GetBGR1_8 ( Buffer [ 2 ] );
		//10:GetXY1 ( Buffer [ 3 ] );
		//11:GetBGR2_8 ( Buffer [ 4 ] );
		//12:GetXY2 ( Buffer [ 5 ] );
		//13:GetBGR3_8 ( Buffer [ 6 ] );
		//14:GetXY3 ( Buffer [ 7 ] );
		// gradient rectangle
		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
		
			DrawTriangle_Gradient( VRAM, inputdata );
			
			if ( !local_id )
			{
				inputdata [ 7 ].Value = ( inputdata [ 7 ].Value & ~0xffffff ) | ( inputdata [ 13 ].Value & 0xffffff );
				inputdata [ 8 ].Value = inputdata [ 14 ].Value;
			}
			
			DrawTriangle_Gradient( VRAM, inputdata );
			break;


		//7:GetBGR0_8 ( Buffer [ 0 ] );
		//8:GetXY0 ( Buffer [ 1 ] );
		//9:GetCLUT ( Buffer [ 2 ] );
		//9:GetUV0 ( Buffer [ 2 ] );
		//10:GetBGR1_8 ( Buffer [ 3 ] );
		//11:GetXY1 ( Buffer [ 4 ] );
		//12:GetTPAGE ( Buffer [ 5 ] );
		//12:GetUV1 ( Buffer [ 5 ] );
		//13:GetBGR2_8 ( Buffer [ 6 ] );
		//14:GetXY2 ( Buffer [ 7 ] );
		//15:GetUV2 ( Buffer [ 8 ] );
		//GetBGR3_8 ( Buffer [ 9 ] );
		//GetXY3 ( Buffer [ 10 ] );
		//GetUV3 ( Buffer [ 11 ] );
		// texture gradient rectangle
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
			DrawTriangle_TextureGradient ( VRAM, inputdata );
			break;
			
		// monochrome line
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
			break;
			
			
		// monochrome polyline
		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
		case 0x4c:
		case 0x4d:
		case 0x4e:
		case 0x4f:
			break;
		
		
		// gradient line
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
			break;

			
		// gradient polyline
		case 0x58:
		case 0x59:
		case 0x5c:
		case 0x5d:
		case 0x5a:
		case 0x5b:
		case 0x5e:
		case 0x5f:
			break;

			
		//GetBGR24 ( Buffer [ 0 ] );
		//GetXY ( Buffer [ 1 ] );
		//GetHW ( Buffer [ 2 ] );
		// x by y rectangle
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
			Draw_Rectangle_60( VRAM, inputdata );
			break;
			
			
		//GetBGR24 ( Buffer [ 0 ] );
		//GetXY ( Buffer [ 1 ] );
		//GetCLUT ( Buffer [ 2 ] );
		//GetUV ( Buffer [ 2 ] );
		//GetHW ( Buffer [ 3 ] );
		// x by y sprite
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
			DrawSprite( VRAM, inputdata );
			break;

			
		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
			Draw_Pixel_68( VRAM, inputdata );
			break;


		//GetBGR24 ( Buffer [ 0 ] );
		//GetXY ( Buffer [ 1 ] );
		// 8x8 rectangle
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
			inputdata [ 2 ].w = 8;
			inputdata [ 2 ].h = 8;
			Draw_Rectangle_60( VRAM, inputdata );
			break;
			
			
		//GetBGR24 ( Buffer [ 0 ] );
		//GetXY ( Buffer [ 1 ] );
		//GetCLUT ( Buffer [ 2 ] );
		//GetUV ( Buffer [ 2 ] );
		// 8x8 sprite
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
			inputdata [ 3 ].w = 8;
			inputdata [ 3 ].h = 8;
			DrawSprite( VRAM, inputdata );
			break;
			


		// 16x16 rectangle
		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
			inputdata [ 2 ].w = 16;
			inputdata [ 2 ].h = 16;
			Draw_Rectangle_60( VRAM, inputdata );
			break;

				
		// 16x16 sprite
		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f:
			inputdata [ 3 ].w = 16;
			inputdata [ 3 ].h = 16;
			DrawSprite( VRAM, inputdata );
			break;
		
		
		default:
			break;
	}
}
*/


int divide_s32 ( int op1, int op2 )
{
	int i;
	int savea;
	int a;
	int q;
	
	// for the signed version only
	int sign = op1 ^ op2;
	op1 = ( ( op1 < 0 ) ? -op1 : op1 );
	op2 = ( ( op2 < 0 ) ? -op2 : op2 );
	
	a = 0;
	q = op1;
	
	for ( i = 0; i < 32; i++ )
	{
		
		// shift left aq
		a <<= 1;
		a |= (((unsigned int) q) >> 31 );
		
		q <<= 1;
		
		// save a
		savea = a;
		
		// a = a - m
		a = a - op2;
		
		if ( a < 0 )
		{
			a = savea;
		}
		else
		{
			q |= 1;
		}
		
	}
	
	// for the signed version only
	q = ( ( sign >> 31 ) ^ q ) + ( ( sign >> 31 ) & 1 );
	
	return q;
}


int mod_s32 ( int op1, int op2 )
{
	int i;
	int savea;
	int a = 0;
	int q = op1;
	
	// for the signed version only
	op1 = abs ( op1 );
	op2 = abs ( op2 );
	
	for ( i = 0; i < 32; i++ )
	{
		
		// shift left aq
		a <<= 1;
		a |= (((unsigned int) q) >> 31 );
		
		q <<= 1;
		
		// save a
		savea = a;
		
		// a = a - m
		a = a - op2;
		
		if ( a < 0 )
		{
			a = savea;
		}
		else
		{
			q |= 1;
		}
		
	}
	
	return a;
}



u16 ColorMultiply1624 ( u32 Color16, u32 Color24 )
{
	const u32 c_iBitsPerPixel16 = 5;
	const u32 c_iRedShift16 = c_iBitsPerPixel16 * 2;
	const u32 c_iRedMask16 = ( 0x1f << c_iRedShift16 );
	const u32 c_iGreenShift16 = c_iBitsPerPixel16 * 1;
	const u32 c_iGreenMask16 = ( 0x1f << c_iGreenShift16 );
	const u32 c_iBlueShift16 = 0;
	const u32 c_iBlueMask16 = ( 0x1f << c_iBlueShift16 );

	const u32 c_iBitsPerPixel24 = 8;
	const u32 c_iRedShift24 = c_iBitsPerPixel24 * 2;
	const u32 c_iRedMask24 = ( 0xff << ( c_iBitsPerPixel24 * 2 ) );
	const u32 c_iGreenShift24 = c_iBitsPerPixel24 * 1;
	const u32 c_iGreenMask24 = ( 0xff << ( c_iBitsPerPixel24 * 1 ) );
	const u32 c_iBlueShift24 = 0;
	const u32 c_iBlueMask24 = ( 0xff << ( c_iBitsPerPixel24 * 0 ) );
	
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






u16 SemiTransparency16 ( u16 B, u16 F, u32 abrCode )
{
	const u32 ShiftSame = 0;
	const u32 ShiftHalf = 1;
	const u32 ShiftQuarter = 2;
	
	const u32 c_iBitsPerPixel = 5;
	//static const u32 c_iShiftHalf_Mask = ~( ( 1 << 4 ) + ( 1 << 9 ) );
	const u32 c_iShiftHalf_Mask = ~( ( 1 << ( c_iBitsPerPixel - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 2 ) - 1 ) ) + ( 1 << ( ( c_iBitsPerPixel * 3 ) - 1 ) ) );
	//static const u32 c_iShiftQuarter_Mask = ~( ( 3 << 3 ) + ( 3 << 8 ) );
	const u32 c_iShiftQuarter_Mask = ~( ( 3 << ( c_iBitsPerPixel - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 2 ) - 2 ) ) + ( 3 << ( ( c_iBitsPerPixel * 3 ) - 2 ) ) );
	//static const u32 c_iClamp_Mask = ( ( 1 << 5 ) + ( 1 << 10 ) + ( 1 << 15 ) );
	const u32 c_iClampMask = ( ( 1 << ( c_iBitsPerPixel ) ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) + ( 1 << ( c_iBitsPerPixel * 3 ) ) );
	const u32 c_iLoBitMask = ( ( 1 ) + ( 1 << c_iBitsPerPixel ) + ( 1 << ( c_iBitsPerPixel * 2 ) ) );
	const u32 c_iPixelMask = 0x7fff;
	
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


