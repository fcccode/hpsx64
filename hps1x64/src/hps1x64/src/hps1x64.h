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




#include "ps1_system.h"

namespace Playstation1
{


	class hps1x64
	{
	public:
	
		static const unsigned long long CyclesToRunContinuous = 500000;

		static const int ProgramWindow_X = 10;
		static const int ProgramWindow_Y = 10;
		//static const int ProgramWindow_Width = GPU::c_iScreenOutput_MaxWidth;	//640;
		//static const int ProgramWindow_Height = GPU::c_iScreenOutput_MaxHeight;	//480;
		static const int ProgramWindow_Width = 640;
		static const int ProgramWindow_Height = 480;
		
		
		static WindowClass::Window *ProgramWindow;

		System _SYSTEM;
		
		union MenuClicked
		{
			struct
			{
				u64 File_Load_State : 1;
				u64 File_Load_BIOS : 1;
				u64 File_Load_GameDisk : 1;
				u64 File_Load_AudioDisk : 1;
				
				u64 File_Save_State : 1;
				
				u64 File_Reset : 1;
				u64 File_Run : 1;
				u64 File_Exit : 1;
				
				u64 Debug_Break : 1;
				u64 Debug_StepInto : 1;

				u64 Debug_ShowWindow_All : 1;
				u64 Debug_ShowWindow_FrameBuffer : 1;
				u64 Debug_ShowWindow_R3000A : 1;
				u64 Debug_ShowWindow_Memory : 1;
				u64 Debug_ShowWindow_DMA : 1;
				u64 Debug_ShowWindow_TIMER : 1;
				u64 Debug_ShowWindow_SPU : 1;
				u64 Debug_ShowWindow_INTC : 1;
				u64 Debug_ShowWindow_GPU : 1;
				u64 Debug_ShowWindow_MDEC : 1;
				u64 Debug_ShowWindow_SIO : 1;
				u64 Debug_ShowWindow_PIO : 1;
				u64 Debug_ShowWindow_CD : 1;
				u64 Debug_ShowWindow_BUS : 1;
				u64 Debug_ShowWindow_ICACHE : 1;
				
				u64 Controllers_Configure : 1;
				u64 Pad1Type_Digital : 1;
				u64 Pad1Type_Analog : 1;
				u64 Pad2Type_Digital : 1;
				u64 Pad2Type_Analog : 1;
				
				u64 MemoryCard1_Connected : 1;
				u64 MemoryCard1_Disconnected : 1;
				u64 MemoryCard2_Connected : 1;
				u64 MemoryCard2_Disconnected : 1;
				
				u64 Region_Europe : 1;
				u64 Region_Japan : 1;
				u64 Region_NorthAmerica : 1;
				
				u64 Audio_Enable : 1;
				u64 Audio_Volume_100 : 1;
				u64 Audio_Volume_75 : 1;
				u64 Audio_Volume_50 : 1;
				u64 Audio_Volume_25 : 1;
				u64 Audio_Buffer_8k : 1;
				u64 Audio_Buffer_16k : 1;
				u64 Audio_Buffer_32k : 1;
				u64 Audio_Buffer_64k : 1;
				u64 Audio_Buffer_1m : 1;
				u64 Audio_Filter : 1;
				
				u64 Video_FullScreen : 1;
			};
			
			u64 Value;
		};
		
		static volatile u32 _MenuClick;
		
		union RunMode
		{
			struct
			{
				u64 RunNormal : 1;
				u64 RunDebug : 1;
				u64 Exit : 1;
			};
			
			u64 Value;
		};
		
		static volatile RunMode _RunMode;
		
	public:
	
		// constructor
		hps1x64 ();
		
		// destructor
		~hps1x64 ();
		
		// reset application
		void Reset ();
	
		void Update_CheckMarksOnMenu ();
		
		int InitializeProgram ();

		int RunProgram ();
		
		int HandleMenuClick ();
		
		void DrawFrameToProgramWindow ();

		void SaveState ( string FilePath = "" );
		void LoadState ( string FilePath = "" );
		void LoadBIOS ( string FilePath = "" );
		string LoadDisk ( string FilePath = "" );
		
		void LoadConfig ( string ConfigFileName );
		void SaveConfig ( string ConfigFileName );
		
		static const int c_iExeMaxPathLength = 2048;
		static string ExecutablePath;
		static string BiosPath;
		
		// events for menu items //
		
		static void OnClick_Debug_Break ( u32 i );
		static void OnClick_Debug_StepInto ( u32 i );
		static void OnClick_Debug_OutputCurrentSector ( u32 i );
		static void OnClick_Debug_Show_All ( u32 i );
		static void OnClick_Debug_Show_FrameBuffer ( u32 i );
		static void OnClick_Debug_Show_R3000A ( u32 i );
		static void OnClick_Debug_Show_Memory ( u32 i );
		static void OnClick_Debug_Show_DMA ( u32 i );
		static void OnClick_Debug_Show_TIMER ( u32 i );
		static void OnClick_Debug_Show_SPU ( u32 i );
		static void OnClick_Debug_Show_CD ( u32 i );
		static void OnClick_Debug_Show_INTC ( u32 i );

		static void OnClick_File_Load_State ( u32 i );
		static void OnClick_File_Load_BIOS ( u32 i );
		static void OnClick_File_Load_GameDisk ( u32 i );
		static void OnClick_File_Load_AudioDisk ( u32 i );
		static void OnClick_File_Save_State ( u32 i );
		static void OnClick_File_Reset ( u32 i );
		static void OnClick_File_Run ( u32 i );
		static void OnClick_File_Exit ( u32 i );
		
		static void OnClick_Controllers_Configure ( u32 i );
		static void OnClick_Pad1Type_Digital ( u32 i );
		static void OnClick_Pad1Type_Analog ( u32 i );
		static void OnClick_Pad2Type_Digital ( u32 i );
		static void OnClick_Pad2Type_Analog ( u32 i );
		
		static void OnClick_Card1_Connect ( u32 i );
		static void OnClick_Card1_Disconnect ( u32 i );
		static void OnClick_Card2_Connect ( u32 i );
		static void OnClick_Card2_Disconnect ( u32 i );
		
		static void OnClick_Region_Europe ( u32 i );
		static void OnClick_Region_Japan ( u32 i );
		static void OnClick_Region_NorthAmerica ( u32 i );
		
		static void OnClick_Audio_Enable ( u32 i );
		static void OnClick_Audio_Volume_100 ( u32 i );
		static void OnClick_Audio_Volume_75 ( u32 i );
		static void OnClick_Audio_Volume_50 ( u32 i );
		static void OnClick_Audio_Volume_25 ( u32 i );
		static void OnClick_Audio_Buffer_8k ( u32 i );
		static void OnClick_Audio_Buffer_16k ( u32 i );
		static void OnClick_Audio_Buffer_32k ( u32 i );
		static void OnClick_Audio_Buffer_64k ( u32 i );
		static void OnClick_Audio_Buffer_1m ( u32 i );
		static void OnClick_Audio_Filter ( u32 i );
		
		static void OnClick_Video_ScanlinesEnable ( u32 i );
		static void OnClick_Video_ScanlinesDisable ( u32 i );
		static void OnClick_Video_FullScreen ( u32 i );
		
		static void OnClick_R3000ACPU_Interpreter ( u32 i );
		static void OnClick_R3000ACPU_Recompiler ( u32 i );
		
		static void LoadClick ( u32 i );
		static void SaveStateClick ( u32 i );
		static void LoadStateClick ( u32 i );
		static void LoadBiosClick ( u32 i );
		static void StartClick ( u32 i );
		static void StopClick ( u32 i );
		static void StepInstructionClick ( u32 i );
		//static void ExitClick ( u32 i );
		static void SaveBIOSClick ( u32 i );
		static void SaveRAMClick ( u32 i );
		static void SetBreakPointClick ( u32 i );
		static void SetCycleBreakPointClick ( u32 i );
		static void SetAddressBreakPointClick ( u32 i );
		static void SetValueClick ( u32 i );
		static void SetMemoryClick ( u32 i );

		
		void DebugWindow_Update ();
	};

	
	class Dialog_KeyConfigure
	{
	public:
		static const int c_iDialog_NumberOfButtons = 16;
		
		static WindowClass::Window *wDialog;
		
		static WindowClass::Button *CmdButtonOk, *CmdButtonCancel;
		static WindowClass::Button* KeyButtons [ c_iDialog_NumberOfButtons ];
		
		static WindowClass::Static *InfoLabel;
		static WindowClass::Static* KeyLabels [ c_iDialog_NumberOfButtons ];
		
		static u32 isDialogShowing;
		static volatile s32 ButtonClick;
		
		static u32 KeyConfigure [ c_iDialog_NumberOfButtons ];
		
		static inline u32 _Abs ( s32 Value )
		{
			return ( ( Value >> 31 ) ^ Value ) - ( Value >> 31 );
		}
		
		static int population_count64(unsigned long long w);
		static int bit_scan_lsb ( unsigned long v );
		
		// returns true if they want to keep settings, false when the dialog was cancelled
		static bool Show_ConfigureKeysDialog ();
		
		static void ConfigureDialog_AnyClick ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam );
		
		static void Refresh ();
	};
}

