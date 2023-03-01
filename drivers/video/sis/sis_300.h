#include <linux/config.h>
#include "initdef.h"

USHORT DRAMType[17][5]={{0x0C,0x0A,0x02,0x40,0x39},{0x0D,0x0A,0x01,0x40,0x48},
                     {0x0C,0x09,0x02,0x20,0x35},{0x0D,0x09,0x01,0x20,0x44},
                     {0x0C,0x08,0x02,0x10,0x31},{0x0D,0x08,0x01,0x10,0x40},
                     {0x0C,0x0A,0x01,0x20,0x34},{0x0C,0x09,0x01,0x08,0x32},
                     {0x0B,0x08,0x02,0x08,0x21},{0x0C,0x08,0x01,0x08,0x30},
                     {0x0A,0x08,0x02,0x04,0x11},{0x0B,0x0A,0x01,0x10,0x28},
                     {0x09,0x08,0x02,0x02,0x01},{0x0B,0x09,0x01,0x08,0x24},
                     {0x0B,0x08,0x01,0x04,0x20},{0x0A,0x08,0x01,0x02,0x10},
                     {0x09,0x08,0x01,0x01,0x00}};

USHORT MDA_DAC[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
               0x15,0x15,0x15,0x15,0x15,0x15,0x15,0x15,
               0x15,0x15,0x15,0x15,0x15,0x15,0x15,0x15,
               0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
               0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
               0x15,0x15,0x15,0x15,0x15,0x15,0x15,0x15,
               0x15,0x15,0x15,0x15,0x15,0x15,0x15,0x15,
               0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F};

USHORT CGA_DAC[]={0x00,0x10,0x04,0x14,0x01,0x11,0x09,0x15,
               0x00,0x10,0x04,0x14,0x01,0x11,0x09,0x15,
               0x2A,0x3A,0x2E,0x3E,0x2B,0x3B,0x2F,0x3F,
               0x2A,0x3A,0x2E,0x3E,0x2B,0x3B,0x2F,0x3F,
               0x00,0x10,0x04,0x14,0x01,0x11,0x09,0x15,
               0x00,0x10,0x04,0x14,0x01,0x11,0x09,0x15,
               0x2A,0x3A,0x2E,0x3E,0x2B,0x3B,0x2F,0x3F,
               0x2A,0x3A,0x2E,0x3E,0x2B,0x3B,0x2F,0x3F};

USHORT EGA_DAC[]={0x00,0x10,0x04,0x14,0x01,0x11,0x05,0x15,
               0x20,0x30,0x24,0x34,0x21,0x31,0x25,0x35,
               0x08,0x18,0x0C,0x1C,0x09,0x19,0x0D,0x1D,
               0x28,0x38,0x2C,0x3C,0x29,0x39,0x2D,0x3D,
               0x02,0x12,0x06,0x16,0x03,0x13,0x07,0x17,
               0x22,0x32,0x26,0x36,0x23,0x33,0x27,0x37,
               0x0A,0x1A,0x0E,0x1E,0x0B,0x1B,0x0F,0x1F,
               0x2A,0x3A,0x2E,0x3E,0x2B,0x3B,0x2F,0x3F};

USHORT VGA_DAC[]={0x00,0x10,0x04,0x14,0x01,0x11,0x09,0x15,
               0x2A,0x3A,0x2E,0x3E,0x2B,0x3B,0x2F,0x3F,
               0x00,0x05,0x08,0x0B,0x0E,0x11,0x14,0x18,
               0x1C,0x20,0x24,0x28,0x2D,0x32,0x38,0x3F,

               0x00,0x10,0x1F,0x2F,0x3F,0x1F,0x27,0x2F,
               0x37,0x3F,0x2D,0x31,0x36,0x3A,0x3F,0x00,
               0x07,0x0E,0x15,0x1C,0x0E,0x11,0x15,0x18,
               0x1C,0x14,0x16,0x18,0x1A,0x1C,0x00,0x04,
               0x08,0x0C,0x10,0x08,0x0A,0x0C,0x0E,0x10,
               0x0B,0x0C,0x0D,0x0F,0x10};

#ifdef CONFIG_FB_SIS_LINUXBIOS
unsigned char SRegsInit[] = { 
 	0x03, 0x00, 0x03, 0x00, 0x02, 0xa1, 0x00, 0x13,
	0x2f, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
 	0x00, 0x0f, 0x00, 0x00, 0x4f, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 
 	0xa1, 0x76, 0xb2, 0xf6, 0x0d, 0x00, 0x00, 0x00,
	0x37, 0x61, 0x80, 0x1b, 0xe1, 0x01, 0x55, 0x43, 
 	0x80, 0x00, 0x01, 0xff, 0x00, 0x00, 0x00, 0xff,
	0x8e, 0x40, 0x00, 0x00, 0x08, 0x00, 0xff, 0xff
};

unsigned char SRegs[] = { 
 	0x03, 0x01, 0x0F, 0x00, 0x0E, 0xA1, 0x02, 0x13,
	0x3F, 0x86, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
 	0x0B, 0x0F, 0x00, 0x00, 0x4F, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x40, 0x00,
 	0xA1, 0xB6, 0xB2, 0xF6, 0x0D, 0x00, 0xF8, 0xF0,
	0x37, 0x61, 0x80, 0x1B, 0xE1, 0x80, 0x55, 0x43,
 	0x80, 0x00, 0x11, 0xFF, 0x00, 0x00, 0x00, 0xFF,
	0x8E, 0x40, 0x00, 0x00, 0x08, 0x00, 0xFF, 0xFF
};

unsigned char CRegs[] = { 
	0x5f, 0x4f, 0x50, 0x82, 0x55, 0x81, 0x0b, 0x3e,
	0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
	0xe9, 0x0b, 0xdf, 0x50, 0x40, 0xe7, 0x04, 0xa3,
	0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff
};	// clear CR11[7]

unsigned char GRegs[] = { 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0f, 0xff, 0x00
};

unsigned char ARegs[] = { 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

unsigned char MReg = 0x6f;

#endif

USHORT      P3c4,P3d4,P3c0,P3ce,P3c2,P3ca,P3c6,P3c7,P3c8,P3c9,P3da;
USHORT	 CRT1VCLKLen; //VCLKData table length of bytes of each entry
USHORT   flag_clearbuffer; //0: no clear frame buffer 1:clear frame buffer
int      RAMType;
int      ModeIDOffset,StandTable,CRT1Table,ScreenOffset,VCLKData,MCLKData, ECLKData;
int      REFIndex,ModeType;
USHORT	 IF_DEF_LVDS,IF_DEF_TRUMPION;
USHORT   VBInfo,LCDResInfo,LCDTypeInfo,LCDInfo;

//int    init300(int,int,int);
VOID   SetMemoryClock(ULONG);
VOID   SetDRAMSize(PHW_DEVICE_EXTENSION);
//extern      "C"    int     ChkBUSWidth(int);

//int    setmode(int,int,int,int);
BOOLEAN  SearchModeID(ULONG, USHORT);
BOOLEAN  CheckMemorySize(ULONG);
VOID     GetModePtr(ULONG, USHORT);
BOOLEAN  GetRatePtr(ULONG, USHORT);
VOID     SetSeqRegs(ULONG);
VOID     SetMiscRegs(ULONG);
VOID     SetCRTCRegs(ULONG);
VOID     SetATTRegs(ULONG);
VOID     SetGRCRegs(ULONG);
VOID     ClearExt1Regs(VOID);
VOID     SetSync(ULONG);
VOID     SetCRT1CRTC(ULONG);
VOID     SetCRT1Offset(ULONG);
VOID     SetCRT1FIFO(ULONG);
VOID     SetCRT1FIFO2(ULONG);
VOID     SetCRT1VCLK(PHW_DEVICE_EXTENSION, ULONG);
VOID     LoadDAC(ULONG);
VOID     DisplayOn(VOID);
VOID     SetCRT1ModeRegs(ULONG, USHORT);
VOID     SetVCLKState(PHW_DEVICE_EXTENSION, ULONG, USHORT);
VOID     WriteDAC(USHORT, USHORT, USHORT, USHORT);
VOID     ClearBuffer(PHW_DEVICE_EXTENSION);
USHORT   ChkBUSWidth(ULONG);
USHORT   GetModeIDLength(ULONG, USHORT);
USHORT   GetRefindexLength(ULONG, USHORT);
VOID     SetInterlace(ULONG, USHORT);
USHORT   CalcDelay2(ULONG ,USHORT);
void 	 Set_LVDS_TRUMPION(VOID);
BOOLEAN SiSSetMode(PHW_DEVICE_EXTENSION HwDeviceExtension,
                   USHORT ModeNo);
#ifndef CONFIG_FB_SIS_LINUXBIOS
static USHORT   CalcDelay(ULONG ,USHORT);
#endif

extern BOOLEAN SetCRT2Group(USHORT BaseAddr,ULONG ROMAddr,USHORT ModeNo,
	PHW_DEVICE_EXTENSION HwDeviceExtension);
extern VOID GetVBInfo(USHORT BaseAddr,ULONG ROMAddr);
extern VOID PresetScratchregister(USHORT P3d4,PHW_DEVICE_EXTENSION HwDeviceExtension);
extern BOOLEAN GetLCDResInfo(ULONG ROMAddr,USHORT P3d4);
extern VOID SetTVSystem(PHW_DEVICE_EXTENSION HwDeviceExtension,ULONG ROMAddr);
extern BOOLEAN GetLCDDDCInfo(PHW_DEVICE_EXTENSION HwDeviceExtension);
extern BOOLEAN GetSenseStatus(PHW_DEVICE_EXTENSION HwDeviceExtension,USHORT BaseAddr,ULONG ROMAddr);
extern BOOLEAN DetectMonitor(PHW_DEVICE_EXTENSION HwDeviceExtension);
extern USHORT GetVCLKLen(ULONG ROMAddr);
extern void SetReg1(u16 port, u16 index, u16 data);
extern void SetReg3(u16 port, u16 data);
extern void SetReg4(u16 port, unsigned long data);
extern u8 GetReg1(u16 port, u16 index);
extern u8 GetReg2(u16 port);
extern u32 GetReg3(u16 port);
extern void ClearDAC(u16 port);
