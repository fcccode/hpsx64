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



#include "R3000ADebugPrint.h"

using namespace std;
using namespace R3000A::Instruction;


const char Print::XyzwLUT [ 4 ] = { 'x', 'y', 'z', 'w' };
const char* Print::BCType [ 4 ] = { "F", "T", "FL", "TL" };


string Print::PrintInstruction ( long instruction )
{
	stringstream ss;
	ss.str("");
	
	//LookupTable [ ( ( instruction >> 16 ) | ( instruction << 16 ) ) & 0x3fffff ] ( ss, instruction );
	FunctionList [ Lookup::FindByInstruction ( instruction ) ] ( ss, instruction );
	
	return ss.str().c_str ();
}

/*
void Print::Special ( stringstream &strInstString, long instruction )
{
	AllPS2SpecialInsts [ GET_SPECIAL( instruction ) ] ( strInstString, instruction );
}

void Print::Regimm ( stringstream &strInstString, long instruction )
{
	AllRegimmB2Codes [ GET_B2 ( instruction ) ] ( strInstString, instruction );
}

void Print::COP0 ( stringstream &strInstString, long instruction )
{
	AllCop0FmtCodes [ GET_FMT( instruction ) ] ( strInstString, instruction );
}


void Print::COP2 ( stringstream &strInstString, long instruction )
{
	AllCop2FmtCodes [ GET_FMT( instruction ) ] ( strInstString, instruction );
}


void Print::CO ( stringstream &strInstString, long instruction )
{
	AllC0Insts [ GET_SPECIAL( instruction ) ] ( strInstString, instruction );
}
*/


//static void Print::BC0 ( stringstream &strInstString, long instruction )
//{
//	strInstString << "BC0" << BCType [ GET_B2( instruction ) & 3 ];
//	AddInstArgs ( strInstString, instruction, FORMAT23 );
//	strInstString << "    " << GET_B2( instruction );
//}

//static void Print::BC2 ( stringstream &strInstString, long instruction )
//{
//	strInstString << "BC2" << BCType [ GET_B2( instruction ) & 3 ];
//	AddInstArgs ( strInstString, instruction, FORMAT23 );
//}


static void Print::MFC0 ( stringstream &strInstString, long instruction )
{
	strInstString << "MFC0";	// mind as well make it MFC since debug instructions won't be encountered
	AddInstArgs ( strInstString, instruction, FORMAT19 );
}

static void Print::MTC0 ( stringstream &strInstString, long instruction )
{
	strInstString << "MTC0";	// mind as well make it MTC since debug instructions won't be encountered
	AddInstArgs ( strInstString, instruction, FORMAT19 );
}

//static void Print::iEI ( stringstream &strInstString, long instruction )
//{
//	strInstString << "EI";
//}

//static void Print::iDI ( stringstream &strInstString, long instruction )
//{
//	strInstString << "DI";
//}

static void Print::LB ( stringstream &strInstString, long instruction )
{
	strInstString << "LB";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

static void Print::LBU ( stringstream &strInstString, long instruction )
{
	strInstString << "LBU";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}


static void Print::LH ( stringstream &strInstString, long instruction )
{
	strInstString << "LH";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

static void Print::LHU ( stringstream &strInstString, long instruction )
{
	strInstString << "LHU";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

static void Print::LWR ( stringstream &strInstString, long instruction )
{
	strInstString << "LWR";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

static void Print::LWL ( stringstream &strInstString, long instruction )
{
	strInstString << "LWL";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

//static void Print::LWU ( stringstream &strInstString, long instruction )
//{
//	strInstString << "LWU";
//	AddInstArgs ( strInstString, instruction, FORMAT11 );
//}

static void Print::SB ( stringstream &strInstString, long instruction )
{
	strInstString << "SB";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}


static void Print::SH ( stringstream &strInstString, long instruction )
{
	strInstString << "SH";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

static void Print::SWL ( stringstream &strInstString, long instruction )
{
	strInstString << "SWL";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

static void Print::SWR ( stringstream &strInstString, long instruction )
{
	strInstString << "SWR";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}


static void Print::SW ( stringstream &strInstString, long instruction )
{
	strInstString << "SW";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

static void Print::LW ( stringstream &strInstString, long instruction )
{
	strInstString << "LW";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}


static void Print::J ( stringstream &strInstString, long instruction )
{
	strInstString << "J";
	AddInstArgs ( strInstString, instruction, FTJ );
}

static void Print::JAL ( stringstream &strInstString, long instruction )
{
	strInstString << "JAL";
	AddInstArgs ( strInstString, instruction, FTJAL );
}

static void Print::BEQ ( stringstream &strInstString, long instruction )
{
	strInstString << "BEQ";
	AddInstArgs ( strInstString, instruction, FTBEQ );
}

static void Print::BNE ( stringstream &strInstString, long instruction )
{
	strInstString << "BNE";
	AddInstArgs ( strInstString, instruction, FTBNE );
}

static void Print::BLEZ ( stringstream &strInstString, long instruction )
{
	strInstString << "BLEZ";
	AddInstArgs ( strInstString, instruction, FTBLEZ );
}

static void Print::BGTZ ( stringstream &strInstString, long instruction )
{
	strInstString << "BGTZ";
	AddInstArgs ( strInstString, instruction, FTBGTZ );
}

static void Print::ADDI ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDI";
	AddInstArgs ( strInstString, instruction, FTADDI );
}

static void Print::ADDIU ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDIU";
	AddInstArgs ( strInstString, instruction, FTADDIU );
}

static void Print::SLTI ( stringstream &strInstString, long instruction )
{
	strInstString << "SLTI";
	AddInstArgs ( strInstString, instruction, FTSLTI );
}

static void Print::SLTIU ( stringstream &strInstString, long instruction )
{
	strInstString << "SLTIU";
	AddInstArgs ( strInstString, instruction, FTSLTIU );
}

static void Print::ANDI ( stringstream &strInstString, long instruction )
{
	strInstString << "ANDI";
	AddInstArgs ( strInstString, instruction, FTANDI );
}

static void Print::ORI ( stringstream &strInstString, long instruction )
{
	strInstString << "ORI";
	AddInstArgs ( strInstString, instruction, FTORI );
}

static void Print::XORI ( stringstream &strInstString, long instruction )
{
	strInstString << "XORI";
	AddInstArgs ( strInstString, instruction, FTXORI );
}

static void Print::LUI ( stringstream &strInstString, long instruction )
{
	strInstString << "LUI";
	AddInstArgs ( strInstString, instruction, FTLUI );
}


static void Print::Invalid ( stringstream &strInstString, long instruction )
{
	strInstString << "Invalid Instruction: Opcode: " << GET_OPCODE( instruction ) << ", Special: " << GET_SPECIAL( instruction ) << ", B1: " << GET_RS( instruction ) << ", B2: " << GET_RT( instruction );
}

//static void Print::BEQL ( stringstream &strInstString, long instruction )
//{
//	strInstString << "BEQL";
//	AddInstArgs ( strInstString, instruction, FTBEQL );
//}

//static void Print::BNEL ( stringstream &strInstString, long instruction )
//{
//	strInstString << "BNEL";
//	AddInstArgs ( strInstString, instruction, FTBNEL );
//}

//static void Print::BLEZL ( stringstream &strInstString, long instruction )
//{
//	strInstString << "BLEZL";
//	AddInstArgs ( strInstString, instruction, FTBLEZL );
//}

//static void Print::BGTZL ( stringstream &strInstString, long instruction )
//{
//	strInstString << "BGTZL";
//	AddInstArgs ( strInstString, instruction, FTBGTZL );
//}



//static void Print::CACHE ( stringstream &strInstString, long instruction )
//{
//	strInstString << "CACHE";
//	AddInstArgs ( strInstString, instruction, FTCACHE );
//}

//static void Print::PREF ( stringstream &strInstString, long instruction )
//{
//	strInstString << "PREF";
//	AddInstArgs ( strInstString, instruction, FTPREF );
//}

static void Print::LWC2 ( stringstream &strInstString, long instruction )
{
	strInstString << "LWC2";
	AddInstArgs ( strInstString, instruction, FORMAT37 /*FTLQC2*/ );
}

static void Print::SWC2 ( stringstream &strInstString, long instruction )
{
	strInstString << "SWC2";
	AddInstArgs ( strInstString, instruction, FORMAT37 /*FTSQC2*/ );
}

static void Print::BLTZ ( stringstream &strInstString, long instruction )
{
	strInstString << "BLTZ";
	AddInstArgs ( strInstString, instruction, FTBLTZ );
}

static void Print::BGEZ ( stringstream &strInstString, long instruction )
{
	strInstString << "BGEZ";
	AddInstArgs ( strInstString, instruction, FTBGEZ );
}

//static void Print::BLTZL ( stringstream &strInstString, long instruction )
//{
//	strInstString << "BLTZL";
//	AddInstArgs ( strInstString, instruction, FTBLTZL );
//}

//static void Print::TGEI ( stringstream &strInstString, long instruction )
//{
//	strInstString << "TGEI";
//	AddInstArgs ( strInstString, instruction, FTTGEI );
//}

//static void Print::TGEIU ( stringstream &strInstString, long instruction )
//{
//	strInstString << "TGEIU";
//	AddInstArgs ( strInstString, instruction, FTTGEIU );
//}

//static void Print::TLTI ( stringstream &strInstString, long instruction )
//{
//	strInstString << "TLTI";
//	AddInstArgs ( strInstString, instruction, FTTLTI );
//}

//static void Print::TLTIU ( stringstream &strInstString, long instruction )
//{
//	strInstString << "TLTIU";
//	AddInstArgs ( strInstString, instruction, FTTLTIU );
//}

//static void Print::TEQI ( stringstream &strInstString, long instruction )
//{
//	strInstString << "TEQI";
//	AddInstArgs ( strInstString, instruction, FTTEQI );
//}

//static void Print::TNEI ( stringstream &strInstString, long instruction )
//{
//	strInstString << "TNEI";
//	AddInstArgs ( strInstString, instruction, FTTNEI );
//}

static void Print::BLTZAL ( stringstream &strInstString, long instruction )
{
	strInstString << "BLTZAL";
	AddInstArgs ( strInstString, instruction, FTBLTZAL );
}

static void Print::BGEZAL ( stringstream &strInstString, long instruction )
{
	strInstString << "BGEZAL";
	AddInstArgs ( strInstString, instruction, FTBGEZAL );
}

//static void Print::BLTZALL ( stringstream &strInstString, long instruction )
//{
//	strInstString << "BLTZALL";
//	AddInstArgs ( strInstString, instruction, FTBLTZALL );
//}

//static void Print::BGEZALL ( stringstream &strInstString, long instruction )
//{
//	strInstString << "BGEZALL";
//	AddInstArgs ( strInstString, instruction, FTBGEZALL );
//}

static void Print::SLL ( stringstream &strInstString, long instruction )
{
	strInstString << "SLL";
	AddInstArgs ( strInstString, instruction, FTSLL );
}

/*
static void Print::MOVCI ( stringstream &strInstString, long instruction )
{
	strInstString << "MOVCI";
//	AddInstArgs ( strInstString, instruction, FTMOVCI );
}
*/

static void Print::SRA ( stringstream &strInstString, long instruction )
{
	strInstString << "SRA";
	AddInstArgs ( strInstString, instruction, FTSRA );
}

static void Print::SRL ( stringstream &strInstString, long instruction )
{
	strInstString << "SRL";
	AddInstArgs ( strInstString, instruction, FTSRL );
}

static void Print::SLLV ( stringstream &strInstString, long instruction )
{
	strInstString << "SLLV";
	AddInstArgs ( strInstString, instruction, FTSLLV );
}

static void Print::SRLV ( stringstream &strInstString, long instruction )
{
	strInstString << "SRLV";
	AddInstArgs ( strInstString, instruction, FTSRLV );
}

static void Print::SRAV ( stringstream &strInstString, long instruction )
{
	strInstString << "SRAV";
	AddInstArgs ( strInstString, instruction, FTSRAV );
}

static void Print::JR ( stringstream &strInstString, long instruction )
{
	strInstString << "JR";
	AddInstArgs ( strInstString, instruction, FTJR );
}

static void Print::JALR ( stringstream &strInstString, long instruction )
{
	strInstString << "JALR";
	AddInstArgs ( strInstString, instruction, FTJALR );
}

//static void Print::MOVZ ( stringstream &strInstString, long instruction )
//{
//	strInstString << "MOVZ";
//	AddInstArgs ( strInstString, instruction, FTMOVZ );
//}

//static void Print::MOVN ( stringstream &strInstString, long instruction )
//{
//	strInstString << "MOVN";
//	AddInstArgs ( strInstString, instruction, FTMOVN );
//}

static void Print::SYSCALL ( stringstream &strInstString, long instruction )
{
	strInstString << "SYSCALL";
	AddInstArgs ( strInstString, instruction, FTSYSCALL );
}

static void Print::BREAK ( stringstream &strInstString, long instruction )
{
	strInstString << "BREAK";
	AddInstArgs ( strInstString, instruction, FTBREAK );
}

/*
static void Print::SYNC ( stringstream &strInstString, long instruction )
{
	strInstString << "SYNC";
//	AddInstArgs ( strInstString, instruction, FTSYNC );
}
*/

static void Print::MFHI ( stringstream &strInstString, long instruction )
{
	strInstString << "MFHI";
	AddInstArgs ( strInstString, instruction, FTMFHI );
}

static void Print::MTHI ( stringstream &strInstString, long instruction )
{
	strInstString << "MTHI";
	AddInstArgs ( strInstString, instruction, FTMTHI );
}

static void Print::MFLO ( stringstream &strInstString, long instruction )
{
	strInstString << "MFLO";
	AddInstArgs ( strInstString, instruction, FTMFLO );
}

static void Print::MTLO ( stringstream &strInstString, long instruction )
{
	strInstString << "MTLO";
	AddInstArgs ( strInstString, instruction, FTMTLO );
}

static void Print::MULT ( stringstream &strInstString, long instruction )
{
	strInstString << "MULT";
	AddInstArgs ( strInstString, instruction, FTMULT );
}

static void Print::MULTU ( stringstream &strInstString, long instruction )
{
	strInstString << "MULTU";
	AddInstArgs ( strInstString, instruction, FTMULTU );
}

static void Print::DIV ( stringstream &strInstString, long instruction )
{
	strInstString << "DIV";
	AddInstArgs ( strInstString, instruction, FTDIV );
}

static void Print::DIVU ( stringstream &strInstString, long instruction )
{
	strInstString << "DIVU";
	AddInstArgs ( strInstString, instruction, FTDIVU );
}

static void Print::ADD ( stringstream &strInstString, long instruction )
{
	strInstString << "ADD";
	AddInstArgs ( strInstString, instruction, FTADD );
}

static void Print::ADDU ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDU";
	AddInstArgs ( strInstString, instruction, FTADDU );
}

static void Print::SUB ( stringstream &strInstString, long instruction )
{
	strInstString << "SUB";
	AddInstArgs ( strInstString, instruction, FTSUB );
}

static void Print::SUBU ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBU";
	AddInstArgs ( strInstString, instruction, FTSUBU );
}

static void Print::AND ( stringstream &strInstString, long instruction )
{
	strInstString << "AND";
	AddInstArgs ( strInstString, instruction, FTAND );
}

static void Print::OR ( stringstream &strInstString, long instruction )
{
	strInstString << "OR";
	AddInstArgs ( strInstString, instruction, FTOR );
}

static void Print::XOR ( stringstream &strInstString, long instruction )
{
	strInstString << "XOR";
	AddInstArgs ( strInstString, instruction, FTXOR );
}

static void Print::NOR ( stringstream &strInstString, long instruction )
{
	strInstString << "NOR";
	AddInstArgs ( strInstString, instruction, FTNOR );
}

//static void Print::MFSA ( stringstream &strInstString, long instruction )
//{
//	strInstString << "MFSA";
//	AddInstArgs ( strInstString, instruction, FTMFSA );
//}

//static void Print::MTSA ( stringstream &strInstString, long instruction )
//{
//	strInstString << "MTSA";
//	AddInstArgs ( strInstString, instruction, FTMTSA );
//}

static void Print::SLT ( stringstream &strInstString, long instruction )
{
	strInstString << "SLT";
	AddInstArgs ( strInstString, instruction, FTSLT );
}

static void Print::SLTU ( stringstream &strInstString, long instruction )
{
	strInstString << "SLTU";
	AddInstArgs ( strInstString, instruction, FTSLTU );
}

//static void Print::TGE ( stringstream &strInstString, long instruction )
//{
//	strInstString << "TGE";
//	AddInstArgs ( strInstString, instruction, FTTGE );
//}

//static void Print::TGEU ( stringstream &strInstString, long instruction )
//{
//	strInstString << "TGEU";
//	AddInstArgs ( strInstString, instruction, FTTGEU );
//}

//static void Print::TLT ( stringstream &strInstString, long instruction )
//{
//	strInstString << "TLT";
//	AddInstArgs ( strInstString, instruction, FTTLT );
//}

//static void Print::TLTU ( stringstream &strInstString, long instruction )
//{
//	strInstString << "TLTU";
//	AddInstArgs ( strInstString, instruction, FTTLTU );
//}

//static void Print::TEQ ( stringstream &strInstString, long instruction )
//{
//	strInstString << "TEQ";
//	AddInstArgs ( strInstString, instruction, FTTEQ );
//}

//static void Print::TNE ( stringstream &strInstString, long instruction )
//{
//	strInstString << "TNE";
//	AddInstArgs ( strInstString, instruction, FTTNE );
//}

//static void Print::TLBR ( stringstream &strInstString, long instruction )
//{
//	strInstString << "TLBR";
//	AddInstArgs ( strInstString, instruction, FTTLBR );
//}

//static void Print::TLBWI ( stringstream &strInstString, long instruction )
//{
//	strInstString << "TLBWI";
//	AddInstArgs ( strInstString, instruction, FTTLBWI );
//}

//static void Print::TLBWR ( stringstream &strInstString, long instruction )
//{
//	strInstString << "TLBWR";
//	AddInstArgs ( strInstString, instruction, FTTLBWR );
//}

//static void Print::TLBP ( stringstream &strInstString, long instruction )
//{
//	strInstString << "TLBP";
//	AddInstArgs ( strInstString, instruction, FTTLBP );
//}

//static void Print::ERET ( stringstream &strInstString, long instruction )
//{
//	strInstString << "ERET";
//	AddInstArgs ( strInstString, instruction, FTERET );
//}

static void Print::RFE ( stringstream &strInstString, long instruction )
{
	strInstString << "RFE";
	AddInstArgs ( strInstString, instruction, FTRFE );
}


//static void Print::DERET ( stringstream &strInstString, long instruction )
//{
//	strInstString << "DERET";
//	AddInstArgs ( strInstString, instruction, FTDERET );
//}

//static void Print::WAIT ( stringstream &strInstString, long instruction )
//{
//	strInstString << "WAIT";
//	AddInstArgs ( strInstString, instruction, FTWAIT );
//}

// PS1 may be able to use this, so I'll just comment it out for now
//static void Print::MFC2 ( stringstream &strInstString, long instruction )
//{
//	strInstString << "MFC2";
//	AddInstArgs ( strInstString, instruction, FTMFC2 );
//}

static void Print::CFC2 ( stringstream &strInstString, long instruction )
{
	strInstString << "CFC2";
	AddInstArgs ( strInstString, instruction, FTCFC2 );
}

static void Print::CTC2 ( stringstream &strInstString, long instruction )
{
	strInstString << "CTC2";
	AddInstArgs ( strInstString, instruction, FTCTC2 );
}


// PS1 may be able to use this, so I'll just comment it out for now
//static void Print::MTC2 ( stringstream &strInstString, long instruction )
//{
//	strInstString << "MTC2";
//	AddInstArgs ( strInstString, instruction, FTMTC2 );
//}



static void Print::MFC2 ( stringstream &strInstString, long instruction )
{
	strInstString << "MFC2";
	AddInstArgs ( strInstString, instruction, FTQMFC2 );
}

static void Print::MTC2 ( stringstream &strInstString, long instruction )
{
	strInstString << "MTC2";
	AddInstArgs ( strInstString, instruction, FTQMTC2 );
}




static void Print::RTPS ( stringstream &strInstString, long instruction )
{
	strInstString << "RTPS";
}

static void Print::NCLIP ( stringstream &strInstString, long instruction )
{
	strInstString << "NCLIP";
}

static void Print::OP ( stringstream &strInstString, long instruction )
{
	strInstString << "OP";
}

static void Print::DPCS ( stringstream &strInstString, long instruction )
{
	strInstString << "DPCS";
}

static void Print::INTPL ( stringstream &strInstString, long instruction )
{
	strInstString << "INTPL";
}

static void Print::MVMVA ( stringstream &strInstString, long instruction )
{
	strInstString << "MVMVA";
}

static void Print::NCDS ( stringstream &strInstString, long instruction )
{
	strInstString << "NCDS";
}

static void Print::CDP ( stringstream &strInstString, long instruction )
{
	strInstString << "CDP";
}

static void Print::NCDT ( stringstream &strInstString, long instruction )
{
	strInstString << "NCDT";
}

static void Print::NCCS ( stringstream &strInstString, long instruction )
{
	strInstString << "NCCS";
}

static void Print::CC ( stringstream &strInstString, long instruction )
{
	strInstString << "CC";
}

static void Print::NCS ( stringstream &strInstString, long instruction )
{
	strInstString << "NCS";
}

static void Print::NCT ( stringstream &strInstString, long instruction )
{
	strInstString << "NCT";
}

static void Print::SQR ( stringstream &strInstString, long instruction )
{
	strInstString << "SQR";
}

static void Print::DCPL ( stringstream &strInstString, long instruction )
{
	strInstString << "DCPL";
}

static void Print::DPCT ( stringstream &strInstString, long instruction )
{
	strInstString << "DPCT";
}

static void Print::AVSZ3 ( stringstream &strInstString, long instruction )
{
	strInstString << "AVSZ3";
}

static void Print::AVSZ4 ( stringstream &strInstString, long instruction )
{
	strInstString << "AVSZ4";
}

static void Print::RTPT ( stringstream &strInstString, long instruction )
{
	strInstString << "RTPT";
}

static void Print::GPF ( stringstream &strInstString, long instruction )
{
	strInstString << "GPF";
}

static void Print::GPL ( stringstream &strInstString, long instruction )
{
	strInstString << "GPL";
}

static void Print::NCCT ( stringstream &strInstString, long instruction )
{
	strInstString << "NCCT";
}




static void Print::AddVuDestArgs ( stringstream &strVuArgs, long Instruction )
{
	// add the dot
	strVuArgs << ".";
	
	if ( GET_DESTX( Instruction ) ) strVuArgs << "x";
	if ( GET_DESTY( Instruction ) ) strVuArgs << "y";
	if ( GET_DESTZ( Instruction ) ) strVuArgs << "z";
	if ( GET_DESTW( Instruction ) ) strVuArgs << "w";
}


static void Print::AddInstArgs ( stringstream &strMipsArgs, long Instruction, long InstFormat )
{

	// don't clear the string since we are adding onto it

	switch ( InstFormat )
	{
		case FORMAT1:
		
			//op rd, rs, rt
			strMipsArgs << dec << " r" << GET_RD( Instruction ) << ", r" << GET_RS( Instruction ) << ", r" << GET_RT( Instruction );
			break;
			
		case FORMAT2:
		
			//op rt, rs, immediate
			strMipsArgs << dec << " r" << GET_RT( Instruction ) << ", r" << GET_RS( Instruction ) << ", " << GET_IMMED( Instruction );
			break;
			
		case FORMAT3:
		
			//op rs, rt, offset
			strMipsArgs << dec << " r" << GET_RS( Instruction ) << ", r" << GET_RT( Instruction ) << ", " << GET_IMMED( Instruction );
			break;
			
		case FORMAT4:
		
			//op rs, offset
			strMipsArgs << dec << " r" << GET_RS( Instruction ) << ", " << GET_IMMED( Instruction );
			break;
			
		case FORMAT5:
		
			//op rs, rt
			strMipsArgs << dec << " r" << GET_RS( Instruction ) << ", r" << GET_RT( Instruction );
			break;
			
		case FORMAT6:
		
			//op rd, rt, sa
			strMipsArgs << dec << " r" << GET_RD( Instruction ) << ", r" << GET_RT( Instruction ) << ", " << GET_SHIFT( Instruction );
			break;
			
		case FORMAT7:
		
			//op rd, rt, rs
			strMipsArgs << dec << " r" << GET_RD( Instruction ) << ", r" << GET_RT( Instruction ) << ", r" << GET_RS( Instruction );
			break;

		case FORMAT8:
		
			//op target
			strMipsArgs << " " << hex << ( GET_ADDRESS( Instruction ) << 2 );
			break;
			
		case FORMAT9:
		
			//op rd, rs
			strMipsArgs << dec << " r" << GET_RD( Instruction ) << ", r" << GET_RS( Instruction );
			break;
			
		case FORMAT10:
		
			//op rs
			strMipsArgs << dec << " r" << GET_RS( Instruction );
			break;
			
		case FORMAT11:
		
			//op rt, offset (base)
			strMipsArgs << dec << " r" << GET_RT( Instruction ) << ", " << ((short)GET_IMMED( Instruction )) << "(r" << GET_BASE( Instruction ) << ")";
			break;
			
		case FORMAT12:
		
			//op rt, immediate
			strMipsArgs << dec << " r" << GET_RT( Instruction ) << ", " << GET_IMMED( Instruction );
			break;
			
		case FORMAT13:
		
			//op rd
			strMipsArgs << dec << " r" << GET_RD( Instruction );
			break;
			
		case FORMAT14:
		
			//op hint, offset (base)
			strMipsArgs << dec << " " << GET_HINT( Instruction ) << ", " << GET_IMMED( Instruction ) << "(r" << GET_BASE( Instruction ) << ")";
			break;
			
		case FORMAT15:
		
			//op rd, rt
			strMipsArgs << dec << " r" << GET_RD( Instruction ) << ", r" << GET_RT( Instruction );
			break;
			
		case FORMAT16:
		
			//op
			break;
			
		case FORMAT17:
		
			//op rt
			strMipsArgs << dec << " r" << GET_RT( Instruction );
			break;
			
		case FORMAT18:
		
			//op rt, reg
			strMipsArgs << dec << " r" << GET_RT( Instruction ) << ", " << GET_REG( Instruction );
			break;
			
		case FORMAT19:

			//op rt, rd
			strMipsArgs << dec << " r" << GET_RT ( Instruction ) << ", r" << GET_RD( Instruction );
			break;
			
		case FORMAT20:
		
			//op fd, fs
			strMipsArgs << " f" << GET_FD( Instruction ) << ", f" << GET_FS( Instruction );
			break;
			
		case FORMAT21:
		
			//op fd, fs, ft
			strMipsArgs << " f" << GET_FD( Instruction ) << ", f" << GET_FS( Instruction ) << ", f" << GET_FT( Instruction );
			break;
			
		case FORMAT22:
		
			//op fs, ft
			strMipsArgs << " f" << GET_FS( Instruction ) << ", f" << GET_FT( Instruction );
			break;
			
		case FORMAT23:
		
			//op offset
			strMipsArgs << " " << GET_IMMED( Instruction );
			break;
			
		case FORMAT24:
		
			//op ft, fs
			strMipsArgs << " f" << GET_FT( Instruction ) << ", f" << GET_FS( Instruction );
			break;
			
		case FORMAT25:
		
			//op fd, ft
			strMipsArgs << " f" << GET_FD( Instruction ) << ", f" << GET_FT( Instruction );
			break;
			
		case FORMAT26:

			//op.dest ft, fs
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vf" << GET_FT( Instruction ) << ", vf" << GET_FS( Instruction );
			break;
			
		case FORMAT27:
		
			//op.dest fd, fs, ft
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vf" << GET_FD( Instruction ) << ", vf" << GET_FS( Instruction ) << ", vf" << GET_FT( Instruction );
			break;
			
		case FORMAT28:
		
			//op.dest fd, fs
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vf" << GET_FD( Instruction ) << ", vf" << GET_FS( Instruction );
			break;
			
		case FORMAT29:

			//op.dest fs, ft
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vf" << GET_FS( Instruction ) << ", vf" << GET_FT( Instruction );
			break;
			
		case FORMAT30:
		
			//op.dest fs
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vf" << GET_FS( Instruction );
			break;
			
		case FORMAT31:

			//op Imm15
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " " << GET_IMM15( Instruction );
			break;
			
		case FORMAT32:
		
			//op fsfsf, ftftf
			strMipsArgs << " vf" << GET_FS( Instruction ) << "." << XyzwLUT [ GET_FSF( Instruction ) ] << ", vf" << GET_FT( Instruction ) << "." << XyzwLUT [ GET_FTF( Instruction ) ];
			break;
			
		case FORMAT33:
		
			//op id, is, it
			strMipsArgs << " vi" << GET_ID( Instruction ) << ", vi" << GET_IS( Instruction ) << ", vi" << GET_IT( Instruction );
			break;
			
		case FORMAT34:
		
			//op it, is, Imm5
			strMipsArgs << " vi" << GET_IT( Instruction ) << ", vi" << GET_IS( Instruction ) << ", " << GET_IMM5( Instruction );
			break;
			
		case FORMAT35:
		
			//op.dest it, (is)
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vi" << GET_IT( Instruction ) << ", (vi" << GET_IS( Instruction ) << ")";
			break;
			
		case FORMAT36:
		
			//op fsfsf
			strMipsArgs << " vf" << GET_FS( Instruction ) << "." << XyzwLUT [ GET_FSF( Instruction ) ];
			break;
			
		case FORMAT37:
		
			//op ft, offset (base)
			strMipsArgs << " f" << GET_FT( Instruction ) << ", " << GET_IMMED( Instruction ) << "(r" << GET_BASE( Instruction ) << ")";
			break;
			
		case FORMAT38:
		
			//op ftftf
			strMipsArgs << " vf" << GET_FS( Instruction ) << "." << XyzwLUT [ GET_FTF( Instruction ) ];
			break;
			
		case FORMAT39:
		
			//op.dest ft
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vf" << GET_FT( Instruction );
			break;
			
		case FORMAT40:
		
			//op.dest fs, (it)
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vf" << GET_FS( Instruction ) << ", (vi" << GET_IT( Instruction ) << ")";
			break;
			
		case FORMAT41:
		
			//op it, fsfsf
			strMipsArgs << " vi" << GET_IT( Instruction ) << ", vf" << GET_FS( Instruction ) << "." << XyzwLUT [ GET_FSF( Instruction ) ];
			break;

		case FORMAT42:

			//op.dest ft, is
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vf" << GET_FT( Instruction ) << ", vi" << GET_IS( Instruction );
			break;

	}

}




static const Print::Function Print::FunctionList []
{
	// instructions on both R3000A and R5900
	Print::Invalid,
	Print::J, Print::JAL, Print::JR, Print::JALR, Print::BEQ, Print::BNE, Print::BGTZ, Print::BGEZ,
	Print::BLTZ, Print::BLEZ, Print::BGEZAL, Print::BLTZAL, Print::ADD, Print::ADDI, Print::ADDU, Print::ADDIU,
	Print::SUB, Print::SUBU, Print::MULT, Print::MULTU, Print::DIV, Print::DIVU, Print::AND, Print::ANDI,
	Print::OR, Print::ORI, Print::XOR, Print::XORI, Print::NOR, Print::LUI, Print::SLL, Print::SRL,
	Print::SRA, Print::SLLV, Print::SRLV, Print::SRAV, Print::SLT, Print::SLTI, Print::SLTU, Print::SLTIU,
	Print::LB, Print::LBU, Print::LH, Print::LHU, Print::LW, Print::LWL, Print::LWR, Print::SB,
	Print::SH, Print::SW, Print::SWL, Print::SWR, Print::MFHI, Print::MTHI, Print::MFLO, Print::MTLO,
	Print::MFC0, Print::MTC0, Print::CFC2, Print::CTC2, Print::SYSCALL, Print::BREAK,
	
	// instructions on R3000A ONLY
	Print::MFC2, Print::MTC2, Print::LWC2, Print::SWC2, Print::RFE,
	Print::RTPS, Print::RTPT, Print::CC, Print::CDP, Print::DCPL, Print::DPCS, Print::DPCT, Print::NCS,
	Print::NCT, Print::NCDS, Print::NCDT, Print::NCCS, Print::NCCT, Print::GPF, Print::GPL, Print::AVSZ3,
	Print::AVSZ4, Print::SQR, Print::OP, Print::NCLIP, Print::INTPL, Print::MVMVA
};




/*
Print::Function Print::LookupTable [ 64 * 32 * 32 * 64 ];


// in format: instruction name, opcode, rs, funct, rt
R3000A::Instruction::Entry<R3000A::Instruction::Print::Function> Print::Entries [] = {
{ "BLTZ", 0x1, DOES_NOT_MATTER, DOES_NOT_MATTER, 0x0, Print::BLTZ },
{ "BGEZ", 0x1, DOES_NOT_MATTER, DOES_NOT_MATTER, 0x1, Print::BGEZ },
{ "BLTZAL", 0x1, DOES_NOT_MATTER, DOES_NOT_MATTER, 0x10, Print::BLTZAL },
{ "BGEZAL", 0x1, DOES_NOT_MATTER, DOES_NOT_MATTER, 0x11, Print::BGEZAL },
{ "BEQ", 0x4, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::BEQ },
{ "BNE", 0x5, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::BNE },
{ "BLEZ", 0x6, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::BLEZ },
{ "BGTZ", 0x7, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::BGTZ },
{ "J", 0x2, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::J },
{ "JAL", 0x3, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::JAL },
{ "JR", 0x0, DOES_NOT_MATTER, 0x8, DOES_NOT_MATTER, Print::JR },
{ "JALR", 0x0, DOES_NOT_MATTER, 0x9, DOES_NOT_MATTER, Print::JALR },
{ "LB", 0x20, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::LB },
{ "LH", 0x21, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::LH },
{ "LWL", 0x22, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::LWL },
{ "LW", 0x23, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::LW },
{ "LBU", 0x24, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::LBU },
{ "LHU", 0x25, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::LHU },
{ "LWR", 0x26, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::LWR },
{ "SB", 0x28, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::SB },
{ "SH", 0x29, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::SH },
{ "SWL", 0x2a, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::SWL },
{ "SW", 0x2b, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::SW },
{ "SWR", 0x2e, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::SWR },
{ "LWC2", 0x32, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::LWC2 },
{ "SWC2", 0x3a, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::SWC2 },
{ "ADDI", 0x8, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::ADDI },
{ "ADDIU", 0x9, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::ADDIU },
{ "SLTI", 0xa, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::SLTI },
{ "SLTIU", 0xb, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::SLTIU },
{ "ANDI", 0xc, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::ANDI },
{ "ORI", 0xd, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::ORI },
{ "XORI", 0xe, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::XORI },
{ "LUI", 0xf, DOES_NOT_MATTER, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::LUI },
{ "SLL", 0x0, DOES_NOT_MATTER, 0x0, DOES_NOT_MATTER, Print::SLL },
{ "SRL", 0x0, DOES_NOT_MATTER, 0x2, DOES_NOT_MATTER, Print::SRL },
{ "SRA", 0x0, DOES_NOT_MATTER, 0x3, DOES_NOT_MATTER, Print::SRA },
{ "SLLV", 0x0, DOES_NOT_MATTER, 0x4, DOES_NOT_MATTER, Print::SLLV },
{ "SRLV", 0x0, DOES_NOT_MATTER, 0x6, DOES_NOT_MATTER, Print::SRLV },
{ "SRAV", 0x0, DOES_NOT_MATTER, 0x7, DOES_NOT_MATTER, Print::SRAV },
{ "SYSCALL", 0x0, DOES_NOT_MATTER, 0xc, DOES_NOT_MATTER, Print::SYSCALL },
{ "BREAK", 0x0, DOES_NOT_MATTER, 0xd, DOES_NOT_MATTER, Print::BREAK },
{ "MFHI", 0x0, DOES_NOT_MATTER, 0x10, DOES_NOT_MATTER, Print::MFHI },
{ "MTHI", 0x0, DOES_NOT_MATTER, 0x11, DOES_NOT_MATTER, Print::MTHI },
{ "MFLO", 0x0, DOES_NOT_MATTER, 0x12, DOES_NOT_MATTER, Print::MFLO },
{ "MTLO", 0x0, DOES_NOT_MATTER, 0x13, DOES_NOT_MATTER, Print::MTLO },
{ "MULT", 0x0, DOES_NOT_MATTER, 0x18, DOES_NOT_MATTER, Print::MULT },
{ "MULTU", 0x0, DOES_NOT_MATTER, 0x19, DOES_NOT_MATTER, Print::MULTU },
{ "DIV", 0x0, DOES_NOT_MATTER, 0x1a, DOES_NOT_MATTER, Print::DIV },
{ "DIVU", 0x0, DOES_NOT_MATTER, 0x1b, DOES_NOT_MATTER, Print::DIVU },
{ "ADD", 0x0, DOES_NOT_MATTER, 0x20, DOES_NOT_MATTER, Print::ADD },
{ "ADDU", 0x0, DOES_NOT_MATTER, 0x21, DOES_NOT_MATTER, Print::ADDU },
{ "SUB", 0x0, DOES_NOT_MATTER, 0x22, DOES_NOT_MATTER, Print::SUB },
{ "SUBU", 0x0, DOES_NOT_MATTER, 0x23, DOES_NOT_MATTER, Print::SUBU },
{ "AND", 0x0, DOES_NOT_MATTER, 0x24, DOES_NOT_MATTER, Print::AND },
{ "OR", 0x0, DOES_NOT_MATTER, 0x25, DOES_NOT_MATTER, Print::OR },
{ "XOR", 0x0, DOES_NOT_MATTER, 0x26, DOES_NOT_MATTER, Print::XOR },
{ "NOR", 0x0, DOES_NOT_MATTER, 0x27, DOES_NOT_MATTER, Print::NOR },
{ "SLT", 0x0, DOES_NOT_MATTER, 0x2a, DOES_NOT_MATTER, Print::SLT },
{ "SLTU", 0x0, DOES_NOT_MATTER, 0x2b, DOES_NOT_MATTER, Print::SLTU },
{ "MFC0", 0x10, 0x0, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::MFC0 },
{ "MTC0", 0x10, 0x4, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::MTC0 },
{ "RFE", 0x10, 0x10, 0x10, DOES_NOT_MATTER, Print::RFE },
{ "RFE", 0x10, 0x11, 0x10, DOES_NOT_MATTER, Print::RFE },
{ "RFE", 0x10, 0x12, 0x10, DOES_NOT_MATTER, Print::RFE },
{ "RFE", 0x10, 0x13, 0x10, DOES_NOT_MATTER, Print::RFE },
{ "RFE", 0x10, 0x14, 0x10, DOES_NOT_MATTER, Print::RFE },
{ "RFE", 0x10, 0x15, 0x10, DOES_NOT_MATTER, Print::RFE },
{ "RFE", 0x10, 0x16, 0x10, DOES_NOT_MATTER, Print::RFE },
{ "RFE", 0x10, 0x17, 0x10, DOES_NOT_MATTER, Print::RFE },
{ "RFE", 0x10, 0x18, 0x10, DOES_NOT_MATTER, Print::RFE },
{ "RFE", 0x10, 0x19, 0x10, DOES_NOT_MATTER, Print::RFE },
{ "RFE", 0x10, 0x1a, 0x10, DOES_NOT_MATTER, Print::RFE },
{ "RFE", 0x10, 0x1b, 0x10, DOES_NOT_MATTER, Print::RFE },
{ "RFE", 0x10, 0x1c, 0x10, DOES_NOT_MATTER, Print::RFE },
{ "RFE", 0x10, 0x1d, 0x10, DOES_NOT_MATTER, Print::RFE },
{ "RFE", 0x10, 0x1e, 0x10, DOES_NOT_MATTER, Print::RFE },
{ "RFE", 0x10, 0x1f, 0x10, DOES_NOT_MATTER, Print::RFE },
{ "MFC2", 0x12, 0x0, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::MFC2 },
{ "CFC2", 0x12, 0x2, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::CFC2 },
{ "MTC2", 0x12, 0x4, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::MTC2 },
{ "CTC2", 0x12, 0x6, DOES_NOT_MATTER, DOES_NOT_MATTER, Print::CTC2 },

// *** COP 2 Instructions ***

{ "RTPS", 0x12, DOES_NOT_MATTER, 0x1, DOES_NOT_MATTER, Print::RTPS },
{ "NCLIP", 0x12, DOES_NOT_MATTER, 0x6, DOES_NOT_MATTER, Print::NCLIP },
{ "OP", 0x12, DOES_NOT_MATTER, 0xc, DOES_NOT_MATTER, Print::OP },
{ "DPCS", 0x12, DOES_NOT_MATTER, 0x10, DOES_NOT_MATTER, Print::DPCS },
{ "INTPL", 0x12, DOES_NOT_MATTER, 0x11, DOES_NOT_MATTER, Print::INTPL },
{ "MVMVA", 0x12, DOES_NOT_MATTER, 0x12, DOES_NOT_MATTER, Print::MVMVA },
{ "NCDS", 0x12, DOES_NOT_MATTER, 0x13, DOES_NOT_MATTER, Print::NCDS },
{ "CDP", 0x12, DOES_NOT_MATTER, 0x14, DOES_NOT_MATTER, Print::CDP },
{ "NCDT", 0x12, DOES_NOT_MATTER, 0x16, DOES_NOT_MATTER, Print::NCDT },
{ "NCCS", 0x12, DOES_NOT_MATTER, 0x1b, DOES_NOT_MATTER, Print::NCCS },
{ "CC", 0x12, DOES_NOT_MATTER, 0x1c, DOES_NOT_MATTER, Print::CC },
{ "NCS", 0x12, DOES_NOT_MATTER, 0x1e, DOES_NOT_MATTER, Print::NCS },
{ "NCT", 0x12, DOES_NOT_MATTER, 0x20, DOES_NOT_MATTER, Print::NCT },
{ "SQR", 0x12, DOES_NOT_MATTER, 0x28, DOES_NOT_MATTER, Print::SQR },
{ "DCPL", 0x12, DOES_NOT_MATTER, 0x29, DOES_NOT_MATTER, Print::DCPL },
{ "DPCT", 0x12, DOES_NOT_MATTER, 0x2a, DOES_NOT_MATTER, Print::DPCT },
{ "AVSZ3", 0x12, DOES_NOT_MATTER, 0x2d, DOES_NOT_MATTER, Print::AVSZ3 },
{ "AVSZ4", 0x12, DOES_NOT_MATTER, 0x2e, DOES_NOT_MATTER, Print::AVSZ4 },
{ "RTPT", 0x12, DOES_NOT_MATTER, 0x30, DOES_NOT_MATTER, Print::RTPT },
{ "GPF", 0x12, DOES_NOT_MATTER, 0x3d, DOES_NOT_MATTER, Print::GPF },
{ "GPL", 0x12, DOES_NOT_MATTER, 0x3e, DOES_NOT_MATTER, Print::GPL },
{ "NCCT", 0x12, DOES_NOT_MATTER, 0x3f, DOES_NOT_MATTER, Print::NCCT }
};
*/





//Debug::Log Execute::debug;


static void Print::Start ()
{
	Lookup::Start ();
}

/*
	u32 Opcode, Rs, Funct, Rt, Index, ElementsInExecute, ElementsInBranchLoad1;
	Instruction::Format i;

	cout << "Running Print::Start\n";

#ifdef INLINE_DEBUG_ENABLE	
	debug.Create ( "R3000A_Print_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "Running Print::Start\r\n";
#endif

	
	ElementsInExecute = (sizeof(Entries) / sizeof(Entries[0]));

	for ( Opcode = 0; Opcode < 64; Opcode++ )
	{
	
		for ( Rs = 0; Rs < 32; Rs++ )
		{
		
			for ( Funct = 0; Funct < 64; Funct++ )
			{
			
				for ( Rt = 0; Rt < 32; Rt++ )
				{
					i.Opcode = Opcode;
					i.Rs = Rs;
					i.Funct = Funct;
					i.Rt = Rt;
				
					// initialize entry in LUT to Invalid instruction
					LookupTable [ ( ( i.Value >> 16 ) | ( i.Value << 16 ) ) & 0x3fffff ] = &Print::Invalid;
					
					// lookup entry in list of instructions
					for ( Index = 0; Index < ElementsInExecute; Index++ )
					{
						// check if we have found the instruction to insert into current position of LUT
						if ( ( Entries [ Index ].Opcode == Opcode || Entries [ Index ].Opcode == DOES_NOT_MATTER )
						&& ( Entries [ Index ].Rs == Rs || Entries [ Index ].Rs == DOES_NOT_MATTER )
						&& ( Entries [ Index ].Funct == Funct || Entries [ Index ].Funct == DOES_NOT_MATTER )
						&& ( Entries [ Index ].Rt == Rt || Entries [ Index ].Rt == DOES_NOT_MATTER ) )
						{
							// enter function for entry into LUT
							LookupTable [ ( ( i.Value >> 16 ) | ( i.Value << 16 ) ) & 0x3fffff ] = Entries [ Index ].FunctionToCall;

							
							break;
						}

					}
				
				}
				
			}
			
		}
		
	}
}
*/


