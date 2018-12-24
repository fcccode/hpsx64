
#define SINGLE_SCANLINE_MODE


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;

typedef char s8;
typedef short s16;
typedef long s32;
typedef long long s64;


constant const int c_lFrameBuffer_Width = 1024;
constant const int c_lFrameBuffer_Height = 512;

constant const int c_lFrameBuffer_Width_Mask = c_lFrameBuffer_Width - 1;
constant const int c_lFrameBuffer_Height_Mask = c_lFrameBuffer_Height - 1;


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



kernel void ps1_gfx_test( global u16* VRAM, global u32* inputdata, global u32* ps1gfxsettings )
{
	Draw_FrameBufferRectangle_02( VRAM, (DATA_Write_Format*) inputdata );
}


void Draw_FrameBufferRectangle_02 ( u16* VRAM, DATA_Write_Format* inputdata )
{
	// *** todo *** fix wrapping and sizing of frame buffer fill //
	
	const int lidx = get_local_id( 0 );
	const int lsize = get_local_size ( 0 );

	// ptr to vram
	//u16 *ptr;
	u16 *ptr16;
	local u16 bgr16;
	
	local u32 width1, width2, height1, height2, xmax, ymax;
	
	local u32 x, y, w, h;
	private u32 xoff, yoff, xinit, yinit;
	
	
	// set local variables
	if ( !lidx )
	{
		// set bgr64
		bgr64 = gbgr [ 0 ];
		bgr64 |= ( bgr64 << 16 );
		bgr64 |= ( bgr64 << 32 );
		
		
		// Xpos=(Xpos AND 3F0h)
		x &= 0x3f0;
		
		// ypos & 0x1ff
		y &= 0x1ff;
		
		// Xsiz=((Xsiz AND 3FFh)+0Fh) AND (NOT 0Fh)
		w = ( ( w & 0x3ff ) + 0xf ) & ~0xf;
		
		// Ysiz=((Ysiz AND 1FFh))
		h &= 0x1ff;
	
	
#ifdef SINGLE_SCANLINE_MODE
		// set the local increment for this core
		xinc = lsize;
		yinc = 1;
#endif
	}

	// synchronize local variables across workers
	barrier ( CLK_LOCAL_MEM_FENCE );

	
#ifdef SINGLE_SCANLINE_MODE
	// set the offset for this worker
	xinit = lidx;
	yinit = 0;
#endif

	
	// *** NOTE: coordinates wrap *** //
	
	
	
	// need to first make sure there is something to draw
	if ( h > 0 && w > 0 )
	{
		for ( yoff = yinit; yoff < h; yoff += yinc )
		{
			for ( xoff = xinit; xoff < w; xoff += xinc )
			{
				if ( ( xoff < w ) && ( yoff < h ) )
				{
					VRAM [ ( ( x + xoff ) & c_lFrameBuffer_Width_Mask ) + ( ( ( y + yoff ) & c_lFrameBuffer_Height_Mask ) << 10 ) ] = bgr16;
				}
			}
		}
	}
	
	///////////////////////////////////////////////
	// set amount of time GPU will be busy for
	//BusyCycles += (u32) ( ( (u64) h * (u64) w * dFrameBufferRectangle_02_CyclesPerPixel ) );
}
