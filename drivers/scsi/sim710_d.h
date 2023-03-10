/* DO NOT EDIT - Generated automatically by script_asm.pl */
static u32 SCRIPT[] = {
/*






ABSOLUTE dsa_select  = 0
ABSOLUTE dsa_msgout  = 8
ABSOLUTE dsa_cmnd    = 16
ABSOLUTE dsa_status  = 24
ABSOLUTE dsa_msgin   = 32
ABSOLUTE dsa_datain  = 40	
ABSOLUTE dsa_dataout = 1064	
ABSOLUTE dsa_size     = 2088

ABSOLUTE reselected_identify = 0
ABSOLUTE msgin_buf = 0



ABSOLUTE int_bad_extmsg1a	= 0xab930000
ABSOLUTE int_bad_extmsg1b	= 0xab930001
ABSOLUTE int_bad_extmsg2a	= 0xab930002
ABSOLUTE int_bad_extmsg2b	= 0xab930003
ABSOLUTE int_bad_extmsg3a	= 0xab930004
ABSOLUTE int_bad_extmsg3b	= 0xab930005
ABSOLUTE int_bad_msg1		= 0xab930006
ABSOLUTE int_bad_msg2		= 0xab930007
ABSOLUTE int_bad_msg3		= 0xab930008
ABSOLUTE int_cmd_bad_phase	= 0xab930009
ABSOLUTE int_cmd_complete	= 0xab93000a
ABSOLUTE int_data_bad_phase	= 0xab93000b
ABSOLUTE int_msg_sdtr1		= 0xab93000c
ABSOLUTE int_msg_sdtr2		= 0xab93000d
ABSOLUTE int_msg_sdtr3		= 0xab93000e
ABSOLUTE int_no_msgout1		= 0xab93000f
ABSOLUTE int_no_msgout2		= 0xab930010
ABSOLUTE int_no_msgout3		= 0xab930011
ABSOLUTE int_not_cmd_complete	= 0xab930012
ABSOLUTE int_sel_no_ident	= 0xab930013
ABSOLUTE int_sel_not_cmd	= 0xab930014
ABSOLUTE int_status_not_msgin	= 0xab930015
ABSOLUTE int_resel_not_msgin	= 0xab930016
ABSOLUTE int_reselected		= 0xab930017
ABSOLUTE int_selected		= 0xab930018
ABSOLUTE int_disc1		= 0xab930019
ABSOLUTE int_disc2		= 0xab93001a
ABSOLUTE int_disc3		= 0xab93001b
ABSOLUTE int_not_rej		= 0xab93001c




ABSOLUTE had_select	= 0x01
ABSOLUTE had_msgout	= 0x02
ABSOLUTE had_cmdout	= 0x04
ABSOLUTE had_datain	= 0x08
ABSOLUTE had_dataout	= 0x10
ABSOLUTE had_status	= 0x20
ABSOLUTE had_msgin	= 0x40
ABSOLUTE had_extmsg	= 0x80






ENTRY do_select
do_select:
	CLEAR TARGET

at 0x00000000 : */	0x60000200,0x00000000,
/*
	MOVE SCRATCH0 & 0 TO SCRATCH0

at 0x00000002 : */	0x7c340000,0x00000000,
/*
	; Enable selection timer
	MOVE CTEST7 & 0xef TO CTEST7

at 0x00000004 : */	0x7c1bef00,0x00000000,
/*
	SELECT ATN FROM dsa_select, reselect

at 0x00000006 : */	0x43000000,0x00000c48,
/*
	JUMP get_status, WHEN STATUS

at 0x00000008 : */	0x830b0000,0x000000a0,
/*
	; Disable selection timer
	MOVE CTEST7 | 0x10 TO CTEST7

at 0x0000000a : */	0x7a1b1000,0x00000000,
/*
	MOVE SCRATCH0 | had_select TO SCRATCH0

at 0x0000000c : */	0x7a340100,0x00000000,
/*
	INT int_sel_no_ident, IF NOT MSG_OUT

at 0x0000000e : */	0x9e020000,0xab930013,
/*
	MOVE SCRATCH0 | had_msgout TO SCRATCH0

at 0x00000010 : */	0x7a340200,0x00000000,
/*
	MOVE FROM dsa_msgout, when MSG_OUT

at 0x00000012 : */	0x1e000000,0x00000008,
/*
ENTRY done_ident
done_ident:
	JUMP get_status, IF STATUS

at 0x00000014 : */	0x830a0000,0x000000a0,
/*
redo_msgin1:
	JUMP get_msgin1, WHEN MSG_IN

at 0x00000016 : */	0x870b0000,0x00000920,
/*
	INT int_sel_not_cmd, IF NOT CMD

at 0x00000018 : */	0x9a020000,0xab930014,
/*
ENTRY resume_cmd
resume_cmd:
	MOVE SCRATCH0 | had_cmdout TO SCRATCH0

at 0x0000001a : */	0x7a340400,0x00000000,
/*
	MOVE FROM dsa_cmnd, WHEN CMD

at 0x0000001c : */	0x1a000000,0x00000010,
/*
ENTRY resume_pmm
resume_pmm:
redo_msgin2:
	JUMP get_msgin2, WHEN MSG_IN

at 0x0000001e : */	0x870b0000,0x00000a20,
/*
	JUMP get_status, IF STATUS

at 0x00000020 : */	0x830a0000,0x000000a0,
/*
	JUMP input_data, IF DATA_IN

at 0x00000022 : */	0x810a0000,0x000000e0,
/*
	JUMP output_data, IF DATA_OUT

at 0x00000024 : */	0x800a0000,0x000004f8,
/*
	INT int_cmd_bad_phase

at 0x00000026 : */	0x98080000,0xab930009,
/*

get_status:
	; Disable selection timer
	MOVE CTEST7 | 0x10 TO CTEST7

at 0x00000028 : */	0x7a1b1000,0x00000000,
/*
	MOVE FROM dsa_status, WHEN STATUS

at 0x0000002a : */	0x1b000000,0x00000018,
/*
	INT int_status_not_msgin, WHEN NOT MSG_IN

at 0x0000002c : */	0x9f030000,0xab930015,
/*
	MOVE FROM dsa_msgin, WHEN MSG_IN

at 0x0000002e : */	0x1f000000,0x00000020,
/*
	INT int_not_cmd_complete, IF NOT 0x00

at 0x00000030 : */	0x98040000,0xab930012,
/*
	CLEAR ACK

at 0x00000032 : */	0x60000040,0x00000000,
/*
ENTRY wait_disc_complete
wait_disc_complete:
	WAIT DISCONNECT

at 0x00000034 : */	0x48000000,0x00000000,
/*
	INT int_cmd_complete

at 0x00000036 : */	0x98080000,0xab93000a,
/*

input_data:
	MOVE SCRATCH0 | had_datain TO SCRATCH0

at 0x00000038 : */	0x7a340800,0x00000000,
/*
ENTRY patch_input_data
patch_input_data:
	JUMP 0

at 0x0000003a : */	0x80080000,0x00000000,
/*
	MOVE FROM dsa_datain+0x0000, WHEN DATA_IN

at 0x0000003c : */	0x19000000,0x00000028,
/*
	MOVE FROM dsa_datain+0x0008, WHEN DATA_IN

at 0x0000003e : */	0x19000000,0x00000030,
/*
	MOVE FROM dsa_datain+0x0010, WHEN DATA_IN

at 0x00000040 : */	0x19000000,0x00000038,
/*
	MOVE FROM dsa_datain+0x0018, WHEN DATA_IN

at 0x00000042 : */	0x19000000,0x00000040,
/*
	MOVE FROM dsa_datain+0x0020, WHEN DATA_IN

at 0x00000044 : */	0x19000000,0x00000048,
/*
	MOVE FROM dsa_datain+0x0028, WHEN DATA_IN

at 0x00000046 : */	0x19000000,0x00000050,
/*
	MOVE FROM dsa_datain+0x0030, WHEN DATA_IN

at 0x00000048 : */	0x19000000,0x00000058,
/*
	MOVE FROM dsa_datain+0x0038, WHEN DATA_IN

at 0x0000004a : */	0x19000000,0x00000060,
/*
	MOVE FROM dsa_datain+0x0040, WHEN DATA_IN

at 0x0000004c : */	0x19000000,0x00000068,
/*
	MOVE FROM dsa_datain+0x0048, WHEN DATA_IN

at 0x0000004e : */	0x19000000,0x00000070,
/*
	MOVE FROM dsa_datain+0x0050, WHEN DATA_IN

at 0x00000050 : */	0x19000000,0x00000078,
/*
	MOVE FROM dsa_datain+0x0058, WHEN DATA_IN

at 0x00000052 : */	0x19000000,0x00000080,
/*
	MOVE FROM dsa_datain+0x0060, WHEN DATA_IN

at 0x00000054 : */	0x19000000,0x00000088,
/*
	MOVE FROM dsa_datain+0x0068, WHEN DATA_IN

at 0x00000056 : */	0x19000000,0x00000090,
/*
	MOVE FROM dsa_datain+0x0070, WHEN DATA_IN

at 0x00000058 : */	0x19000000,0x00000098,
/*
	MOVE FROM dsa_datain+0x0078, WHEN DATA_IN

at 0x0000005a : */	0x19000000,0x000000a0,
/*
	MOVE FROM dsa_datain+0x0080, WHEN DATA_IN

at 0x0000005c : */	0x19000000,0x000000a8,
/*
	MOVE FROM dsa_datain+0x0088, WHEN DATA_IN

at 0x0000005e : */	0x19000000,0x000000b0,
/*
	MOVE FROM dsa_datain+0x0090, WHEN DATA_IN

at 0x00000060 : */	0x19000000,0x000000b8,
/*
	MOVE FROM dsa_datain+0x0098, WHEN DATA_IN

at 0x00000062 : */	0x19000000,0x000000c0,
/*
	MOVE FROM dsa_datain+0x00a0, WHEN DATA_IN

at 0x00000064 : */	0x19000000,0x000000c8,
/*
	MOVE FROM dsa_datain+0x00a8, WHEN DATA_IN

at 0x00000066 : */	0x19000000,0x000000d0,
/*
	MOVE FROM dsa_datain+0x00b0, WHEN DATA_IN

at 0x00000068 : */	0x19000000,0x000000d8,
/*
	MOVE FROM dsa_datain+0x00b8, WHEN DATA_IN

at 0x0000006a : */	0x19000000,0x000000e0,
/*
	MOVE FROM dsa_datain+0x00c0, WHEN DATA_IN

at 0x0000006c : */	0x19000000,0x000000e8,
/*
	MOVE FROM dsa_datain+0x00c8, WHEN DATA_IN

at 0x0000006e : */	0x19000000,0x000000f0,
/*
	MOVE FROM dsa_datain+0x00d0, WHEN DATA_IN

at 0x00000070 : */	0x19000000,0x000000f8,
/*
	MOVE FROM dsa_datain+0x00d8, WHEN DATA_IN

at 0x00000072 : */	0x19000000,0x00000100,
/*
	MOVE FROM dsa_datain+0x00e0, WHEN DATA_IN

at 0x00000074 : */	0x19000000,0x00000108,
/*
	MOVE FROM dsa_datain+0x00e8, WHEN DATA_IN

at 0x00000076 : */	0x19000000,0x00000110,
/*
	MOVE FROM dsa_datain+0x00f0, WHEN DATA_IN

at 0x00000078 : */	0x19000000,0x00000118,
/*
	MOVE FROM dsa_datain+0x00f8, WHEN DATA_IN

at 0x0000007a : */	0x19000000,0x00000120,
/*
	MOVE FROM dsa_datain+0x0100, WHEN DATA_IN

at 0x0000007c : */	0x19000000,0x00000128,
/*
	MOVE FROM dsa_datain+0x0108, WHEN DATA_IN

at 0x0000007e : */	0x19000000,0x00000130,
/*
	MOVE FROM dsa_datain+0x0110, WHEN DATA_IN

at 0x00000080 : */	0x19000000,0x00000138,
/*
	MOVE FROM dsa_datain+0x0118, WHEN DATA_IN

at 0x00000082 : */	0x19000000,0x00000140,
/*
	MOVE FROM dsa_datain+0x0120, WHEN DATA_IN

at 0x00000084 : */	0x19000000,0x00000148,
/*
	MOVE FROM dsa_datain+0x0128, WHEN DATA_IN

at 0x00000086 : */	0x19000000,0x00000150,
/*
	MOVE FROM dsa_datain+0x0130, WHEN DATA_IN

at 0x00000088 : */	0x19000000,0x00000158,
/*
	MOVE FROM dsa_datain+0x0138, WHEN DATA_IN

at 0x0000008a : */	0x19000000,0x00000160,
/*
	MOVE FROM dsa_datain+0x0140, WHEN DATA_IN

at 0x0000008c : */	0x19000000,0x00000168,
/*
	MOVE FROM dsa_datain+0x0148, WHEN DATA_IN

at 0x0000008e : */	0x19000000,0x00000170,
/*
	MOVE FROM dsa_datain+0x0150, WHEN DATA_IN

at 0x00000090 : */	0x19000000,0x00000178,
/*
	MOVE FROM dsa_datain+0x0158, WHEN DATA_IN

at 0x00000092 : */	0x19000000,0x00000180,
/*
	MOVE FROM dsa_datain+0x0160, WHEN DATA_IN

at 0x00000094 : */	0x19000000,0x00000188,
/*
	MOVE FROM dsa_datain+0x0168, WHEN DATA_IN

at 0x00000096 : */	0x19000000,0x00000190,
/*
	MOVE FROM dsa_datain+0x0170, WHEN DATA_IN

at 0x00000098 : */	0x19000000,0x00000198,
/*
	MOVE FROM dsa_datain+0x0178, WHEN DATA_IN

at 0x0000009a : */	0x19000000,0x000001a0,
/*
	MOVE FROM dsa_datain+0x0180, WHEN DATA_IN

at 0x0000009c : */	0x19000000,0x000001a8,
/*
	MOVE FROM dsa_datain+0x0188, WHEN DATA_IN

at 0x0000009e : */	0x19000000,0x000001b0,
/*
	MOVE FROM dsa_datain+0x0190, WHEN DATA_IN

at 0x000000a0 : */	0x19000000,0x000001b8,
/*
	MOVE FROM dsa_datain+0x0198, WHEN DATA_IN

at 0x000000a2 : */	0x19000000,0x000001c0,
/*
	MOVE FROM dsa_datain+0x01a0, WHEN DATA_IN

at 0x000000a4 : */	0x19000000,0x000001c8,
/*
	MOVE FROM dsa_datain+0x01a8, WHEN DATA_IN

at 0x000000a6 : */	0x19000000,0x000001d0,
/*
	MOVE FROM dsa_datain+0x01b0, WHEN DATA_IN

at 0x000000a8 : */	0x19000000,0x000001d8,
/*
	MOVE FROM dsa_datain+0x01b8, WHEN DATA_IN

at 0x000000aa : */	0x19000000,0x000001e0,
/*
	MOVE FROM dsa_datain+0x01c0, WHEN DATA_IN

at 0x000000ac : */	0x19000000,0x000001e8,
/*
	MOVE FROM dsa_datain+0x01c8, WHEN DATA_IN

at 0x000000ae : */	0x19000000,0x000001f0,
/*
	MOVE FROM dsa_datain+0x01d0, WHEN DATA_IN

at 0x000000b0 : */	0x19000000,0x000001f8,
/*
	MOVE FROM dsa_datain+0x01d8, WHEN DATA_IN

at 0x000000b2 : */	0x19000000,0x00000200,
/*
	MOVE FROM dsa_datain+0x01e0, WHEN DATA_IN

at 0x000000b4 : */	0x19000000,0x00000208,
/*
	MOVE FROM dsa_datain+0x01e8, WHEN DATA_IN

at 0x000000b6 : */	0x19000000,0x00000210,
/*
	MOVE FROM dsa_datain+0x01f0, WHEN DATA_IN

at 0x000000b8 : */	0x19000000,0x00000218,
/*
	MOVE FROM dsa_datain+0x01f8, WHEN DATA_IN

at 0x000000ba : */	0x19000000,0x00000220,
/*
	MOVE FROM dsa_datain+0x0200, WHEN DATA_IN

at 0x000000bc : */	0x19000000,0x00000228,
/*
	MOVE FROM dsa_datain+0x0208, WHEN DATA_IN

at 0x000000be : */	0x19000000,0x00000230,
/*
	MOVE FROM dsa_datain+0x0210, WHEN DATA_IN

at 0x000000c0 : */	0x19000000,0x00000238,
/*
	MOVE FROM dsa_datain+0x0218, WHEN DATA_IN

at 0x000000c2 : */	0x19000000,0x00000240,
/*
	MOVE FROM dsa_datain+0x0220, WHEN DATA_IN

at 0x000000c4 : */	0x19000000,0x00000248,
/*
	MOVE FROM dsa_datain+0x0228, WHEN DATA_IN

at 0x000000c6 : */	0x19000000,0x00000250,
/*
	MOVE FROM dsa_datain+0x0230, WHEN DATA_IN

at 0x000000c8 : */	0x19000000,0x00000258,
/*
	MOVE FROM dsa_datain+0x0238, WHEN DATA_IN

at 0x000000ca : */	0x19000000,0x00000260,
/*
	MOVE FROM dsa_datain+0x0240, WHEN DATA_IN

at 0x000000cc : */	0x19000000,0x00000268,
/*
	MOVE FROM dsa_datain+0x0248, WHEN DATA_IN

at 0x000000ce : */	0x19000000,0x00000270,
/*
	MOVE FROM dsa_datain+0x0250, WHEN DATA_IN

at 0x000000d0 : */	0x19000000,0x00000278,
/*
	MOVE FROM dsa_datain+0x0258, WHEN DATA_IN

at 0x000000d2 : */	0x19000000,0x00000280,
/*
	MOVE FROM dsa_datain+0x0260, WHEN DATA_IN

at 0x000000d4 : */	0x19000000,0x00000288,
/*
	MOVE FROM dsa_datain+0x0268, WHEN DATA_IN

at 0x000000d6 : */	0x19000000,0x00000290,
/*
	MOVE FROM dsa_datain+0x0270, WHEN DATA_IN

at 0x000000d8 : */	0x19000000,0x00000298,
/*
	MOVE FROM dsa_datain+0x0278, WHEN DATA_IN

at 0x000000da : */	0x19000000,0x000002a0,
/*
	MOVE FROM dsa_datain+0x0280, WHEN DATA_IN

at 0x000000dc : */	0x19000000,0x000002a8,
/*
	MOVE FROM dsa_datain+0x0288, WHEN DATA_IN

at 0x000000de : */	0x19000000,0x000002b0,
/*
	MOVE FROM dsa_datain+0x0290, WHEN DATA_IN

at 0x000000e0 : */	0x19000000,0x000002b8,
/*
	MOVE FROM dsa_datain+0x0298, WHEN DATA_IN

at 0x000000e2 : */	0x19000000,0x000002c0,
/*
	MOVE FROM dsa_datain+0x02a0, WHEN DATA_IN

at 0x000000e4 : */	0x19000000,0x000002c8,
/*
	MOVE FROM dsa_datain+0x02a8, WHEN DATA_IN

at 0x000000e6 : */	0x19000000,0x000002d0,
/*
	MOVE FROM dsa_datain+0x02b0, WHEN DATA_IN

at 0x000000e8 : */	0x19000000,0x000002d8,
/*
	MOVE FROM dsa_datain+0x02b8, WHEN DATA_IN

at 0x000000ea : */	0x19000000,0x000002e0,
/*
	MOVE FROM dsa_datain+0x02c0, WHEN DATA_IN

at 0x000000ec : */	0x19000000,0x000002e8,
/*
	MOVE FROM dsa_datain+0x02c8, WHEN DATA_IN

at 0x000000ee : */	0x19000000,0x000002f0,
/*
	MOVE FROM dsa_datain+0x02d0, WHEN DATA_IN

at 0x000000f0 : */	0x19000000,0x000002f8,
/*
	MOVE FROM dsa_datain+0x02d8, WHEN DATA_IN

at 0x000000f2 : */	0x19000000,0x00000300,
/*
	MOVE FROM dsa_datain+0x02e0, WHEN DATA_IN

at 0x000000f4 : */	0x19000000,0x00000308,
/*
	MOVE FROM dsa_datain+0x02e8, WHEN DATA_IN

at 0x000000f6 : */	0x19000000,0x00000310,
/*
	MOVE FROM dsa_datain+0x02f0, WHEN DATA_IN

at 0x000000f8 : */	0x19000000,0x00000318,
/*
	MOVE FROM dsa_datain+0x02f8, WHEN DATA_IN

at 0x000000fa : */	0x19000000,0x00000320,
/*
	MOVE FROM dsa_datain+0x0300, WHEN DATA_IN

at 0x000000fc : */	0x19000000,0x00000328,
/*
	MOVE FROM dsa_datain+0x0308, WHEN DATA_IN

at 0x000000fe : */	0x19000000,0x00000330,
/*
	MOVE FROM dsa_datain+0x0310, WHEN DATA_IN

at 0x00000100 : */	0x19000000,0x00000338,
/*
	MOVE FROM dsa_datain+0x0318, WHEN DATA_IN

at 0x00000102 : */	0x19000000,0x00000340,
/*
	MOVE FROM dsa_datain+0x0320, WHEN DATA_IN

at 0x00000104 : */	0x19000000,0x00000348,
/*
	MOVE FROM dsa_datain+0x0328, WHEN DATA_IN

at 0x00000106 : */	0x19000000,0x00000350,
/*
	MOVE FROM dsa_datain+0x0330, WHEN DATA_IN

at 0x00000108 : */	0x19000000,0x00000358,
/*
	MOVE FROM dsa_datain+0x0338, WHEN DATA_IN

at 0x0000010a : */	0x19000000,0x00000360,
/*
	MOVE FROM dsa_datain+0x0340, WHEN DATA_IN

at 0x0000010c : */	0x19000000,0x00000368,
/*
	MOVE FROM dsa_datain+0x0348, WHEN DATA_IN

at 0x0000010e : */	0x19000000,0x00000370,
/*
	MOVE FROM dsa_datain+0x0350, WHEN DATA_IN

at 0x00000110 : */	0x19000000,0x00000378,
/*
	MOVE FROM dsa_datain+0x0358, WHEN DATA_IN

at 0x00000112 : */	0x19000000,0x00000380,
/*
	MOVE FROM dsa_datain+0x0360, WHEN DATA_IN

at 0x00000114 : */	0x19000000,0x00000388,
/*
	MOVE FROM dsa_datain+0x0368, WHEN DATA_IN

at 0x00000116 : */	0x19000000,0x00000390,
/*
	MOVE FROM dsa_datain+0x0370, WHEN DATA_IN

at 0x00000118 : */	0x19000000,0x00000398,
/*
	MOVE FROM dsa_datain+0x0378, WHEN DATA_IN

at 0x0000011a : */	0x19000000,0x000003a0,
/*
	MOVE FROM dsa_datain+0x0380, WHEN DATA_IN

at 0x0000011c : */	0x19000000,0x000003a8,
/*
	MOVE FROM dsa_datain+0x0388, WHEN DATA_IN

at 0x0000011e : */	0x19000000,0x000003b0,
/*
	MOVE FROM dsa_datain+0x0390, WHEN DATA_IN

at 0x00000120 : */	0x19000000,0x000003b8,
/*
	MOVE FROM dsa_datain+0x0398, WHEN DATA_IN

at 0x00000122 : */	0x19000000,0x000003c0,
/*
	MOVE FROM dsa_datain+0x03a0, WHEN DATA_IN

at 0x00000124 : */	0x19000000,0x000003c8,
/*
	MOVE FROM dsa_datain+0x03a8, WHEN DATA_IN

at 0x00000126 : */	0x19000000,0x000003d0,
/*
	MOVE FROM dsa_datain+0x03b0, WHEN DATA_IN

at 0x00000128 : */	0x19000000,0x000003d8,
/*
	MOVE FROM dsa_datain+0x03b8, WHEN DATA_IN

at 0x0000012a : */	0x19000000,0x000003e0,
/*
	MOVE FROM dsa_datain+0x03c0, WHEN DATA_IN

at 0x0000012c : */	0x19000000,0x000003e8,
/*
	MOVE FROM dsa_datain+0x03c8, WHEN DATA_IN

at 0x0000012e : */	0x19000000,0x000003f0,
/*
	MOVE FROM dsa_datain+0x03d0, WHEN DATA_IN

at 0x00000130 : */	0x19000000,0x000003f8,
/*
	MOVE FROM dsa_datain+0x03d8, WHEN DATA_IN

at 0x00000132 : */	0x19000000,0x00000400,
/*
	MOVE FROM dsa_datain+0x03e0, WHEN DATA_IN

at 0x00000134 : */	0x19000000,0x00000408,
/*
	MOVE FROM dsa_datain+0x03e8, WHEN DATA_IN

at 0x00000136 : */	0x19000000,0x00000410,
/*
	MOVE FROM dsa_datain+0x03f0, WHEN DATA_IN

at 0x00000138 : */	0x19000000,0x00000418,
/*
	MOVE FROM dsa_datain+0x03f8, WHEN DATA_IN

at 0x0000013a : */	0x19000000,0x00000420,
/*
	JUMP end_data_trans

at 0x0000013c : */	0x80080000,0x00000908,
/*

output_data:
	MOVE SCRATCH0 | had_dataout TO SCRATCH0

at 0x0000013e : */	0x7a341000,0x00000000,
/*
ENTRY patch_output_data
patch_output_data:
	JUMP 0

at 0x00000140 : */	0x80080000,0x00000000,
/*
	MOVE FROM dsa_dataout+0x0000, WHEN DATA_OUT

at 0x00000142 : */	0x18000000,0x00000428,
/*
	MOVE FROM dsa_dataout+0x0008, WHEN DATA_OUT

at 0x00000144 : */	0x18000000,0x00000430,
/*
	MOVE FROM dsa_dataout+0x0010, WHEN DATA_OUT

at 0x00000146 : */	0x18000000,0x00000438,
/*
	MOVE FROM dsa_dataout+0x0018, WHEN DATA_OUT

at 0x00000148 : */	0x18000000,0x00000440,
/*
	MOVE FROM dsa_dataout+0x0020, WHEN DATA_OUT

at 0x0000014a : */	0x18000000,0x00000448,
/*
	MOVE FROM dsa_dataout+0x0028, WHEN DATA_OUT

at 0x0000014c : */	0x18000000,0x00000450,
/*
	MOVE FROM dsa_dataout+0x0030, WHEN DATA_OUT

at 0x0000014e : */	0x18000000,0x00000458,
/*
	MOVE FROM dsa_dataout+0x0038, WHEN DATA_OUT

at 0x00000150 : */	0x18000000,0x00000460,
/*
	MOVE FROM dsa_dataout+0x0040, WHEN DATA_OUT

at 0x00000152 : */	0x18000000,0x00000468,
/*
	MOVE FROM dsa_dataout+0x0048, WHEN DATA_OUT

at 0x00000154 : */	0x18000000,0x00000470,
/*
	MOVE FROM dsa_dataout+0x0050, WHEN DATA_OUT

at 0x00000156 : */	0x18000000,0x00000478,
/*
	MOVE FROM dsa_dataout+0x0058, WHEN DATA_OUT

at 0x00000158 : */	0x18000000,0x00000480,
/*
	MOVE FROM dsa_dataout+0x0060, WHEN DATA_OUT

at 0x0000015a : */	0x18000000,0x00000488,
/*
	MOVE FROM dsa_dataout+0x0068, WHEN DATA_OUT

at 0x0000015c : */	0x18000000,0x00000490,
/*
	MOVE FROM dsa_dataout+0x0070, WHEN DATA_OUT

at 0x0000015e : */	0x18000000,0x00000498,
/*
	MOVE FROM dsa_dataout+0x0078, WHEN DATA_OUT

at 0x00000160 : */	0x18000000,0x000004a0,
/*
	MOVE FROM dsa_dataout+0x0080, WHEN DATA_OUT

at 0x00000162 : */	0x18000000,0x000004a8,
/*
	MOVE FROM dsa_dataout+0x0088, WHEN DATA_OUT

at 0x00000164 : */	0x18000000,0x000004b0,
/*
	MOVE FROM dsa_dataout+0x0090, WHEN DATA_OUT

at 0x00000166 : */	0x18000000,0x000004b8,
/*
	MOVE FROM dsa_dataout+0x0098, WHEN DATA_OUT

at 0x00000168 : */	0x18000000,0x000004c0,
/*
	MOVE FROM dsa_dataout+0x00a0, WHEN DATA_OUT

at 0x0000016a : */	0x18000000,0x000004c8,
/*
	MOVE FROM dsa_dataout+0x00a8, WHEN DATA_OUT

at 0x0000016c : */	0x18000000,0x000004d0,
/*
	MOVE FROM dsa_dataout+0x00b0, WHEN DATA_OUT

at 0x0000016e : */	0x18000000,0x000004d8,
/*
	MOVE FROM dsa_dataout+0x00b8, WHEN DATA_OUT

at 0x00000170 : */	0x18000000,0x000004e0,
/*
	MOVE FROM dsa_dataout+0x00c0, WHEN DATA_OUT

at 0x00000172 : */	0x18000000,0x000004e8,
/*
	MOVE FROM dsa_dataout+0x00c8, WHEN DATA_OUT

at 0x00000174 : */	0x18000000,0x000004f0,
/*
	MOVE FROM dsa_dataout+0x00d0, WHEN DATA_OUT

at 0x00000176 : */	0x18000000,0x000004f8,
/*
	MOVE FROM dsa_dataout+0x00d8, WHEN DATA_OUT

at 0x00000178 : */	0x18000000,0x00000500,
/*
	MOVE FROM dsa_dataout+0x00e0, WHEN DATA_OUT

at 0x0000017a : */	0x18000000,0x00000508,
/*
	MOVE FROM dsa_dataout+0x00e8, WHEN DATA_OUT

at 0x0000017c : */	0x18000000,0x00000510,
/*
	MOVE FROM dsa_dataout+0x00f0, WHEN DATA_OUT

at 0x0000017e : */	0x18000000,0x00000518,
/*
	MOVE FROM dsa_dataout+0x00f8, WHEN DATA_OUT

at 0x00000180 : */	0x18000000,0x00000520,
/*
	MOVE FROM dsa_dataout+0x0100, WHEN DATA_OUT

at 0x00000182 : */	0x18000000,0x00000528,
/*
	MOVE FROM dsa_dataout+0x0108, WHEN DATA_OUT

at 0x00000184 : */	0x18000000,0x00000530,
/*
	MOVE FROM dsa_dataout+0x0110, WHEN DATA_OUT

at 0x00000186 : */	0x18000000,0x00000538,
/*
	MOVE FROM dsa_dataout+0x0118, WHEN DATA_OUT

at 0x00000188 : */	0x18000000,0x00000540,
/*
	MOVE FROM dsa_dataout+0x0120, WHEN DATA_OUT

at 0x0000018a : */	0x18000000,0x00000548,
/*
	MOVE FROM dsa_dataout+0x0128, WHEN DATA_OUT

at 0x0000018c : */	0x18000000,0x00000550,
/*
	MOVE FROM dsa_dataout+0x0130, WHEN DATA_OUT

at 0x0000018e : */	0x18000000,0x00000558,
/*
	MOVE FROM dsa_dataout+0x0138, WHEN DATA_OUT

at 0x00000190 : */	0x18000000,0x00000560,
/*
	MOVE FROM dsa_dataout+0x0140, WHEN DATA_OUT

at 0x00000192 : */	0x18000000,0x00000568,
/*
	MOVE FROM dsa_dataout+0x0148, WHEN DATA_OUT

at 0x00000194 : */	0x18000000,0x00000570,
/*
	MOVE FROM dsa_dataout+0x0150, WHEN DATA_OUT

at 0x00000196 : */	0x18000000,0x00000578,
/*
	MOVE FROM dsa_dataout+0x0158, WHEN DATA_OUT

at 0x00000198 : */	0x18000000,0x00000580,
/*
	MOVE FROM dsa_dataout+0x0160, WHEN DATA_OUT

at 0x0000019a : */	0x18000000,0x00000588,
/*
	MOVE FROM dsa_dataout+0x0168, WHEN DATA_OUT

at 0x0000019c : */	0x18000000,0x00000590,
/*
	MOVE FROM dsa_dataout+0x0170, WHEN DATA_OUT

at 0x0000019e : */	0x18000000,0x00000598,
/*
	MOVE FROM dsa_dataout+0x0178, WHEN DATA_OUT

at 0x000001a0 : */	0x18000000,0x000005a0,
/*
	MOVE FROM dsa_dataout+0x0180, WHEN DATA_OUT

at 0x000001a2 : */	0x18000000,0x000005a8,
/*
	MOVE FROM dsa_dataout+0x0188, WHEN DATA_OUT

at 0x000001a4 : */	0x18000000,0x000005b0,
/*
	MOVE FROM dsa_dataout+0x0190, WHEN DATA_OUT

at 0x000001a6 : */	0x18000000,0x000005b8,
/*
	MOVE FROM dsa_dataout+0x0198, WHEN DATA_OUT

at 0x000001a8 : */	0x18000000,0x000005c0,
/*
	MOVE FROM dsa_dataout+0x01a0, WHEN DATA_OUT

at 0x000001aa : */	0x18000000,0x000005c8,
/*
	MOVE FROM dsa_dataout+0x01a8, WHEN DATA_OUT

at 0x000001ac : */	0x18000000,0x000005d0,
/*
	MOVE FROM dsa_dataout+0x01b0, WHEN DATA_OUT

at 0x000001ae : */	0x18000000,0x000005d8,
/*
	MOVE FROM dsa_dataout+0x01b8, WHEN DATA_OUT

at 0x000001b0 : */	0x18000000,0x000005e0,
/*
	MOVE FROM dsa_dataout+0x01c0, WHEN DATA_OUT

at 0x000001b2 : */	0x18000000,0x000005e8,
/*
	MOVE FROM dsa_dataout+0x01c8, WHEN DATA_OUT

at 0x000001b4 : */	0x18000000,0x000005f0,
/*
	MOVE FROM dsa_dataout+0x01d0, WHEN DATA_OUT

at 0x000001b6 : */	0x18000000,0x000005f8,
/*
	MOVE FROM dsa_dataout+0x01d8, WHEN DATA_OUT

at 0x000001b8 : */	0x18000000,0x00000600,
/*
	MOVE FROM dsa_dataout+0x01e0, WHEN DATA_OUT

at 0x000001ba : */	0x18000000,0x00000608,
/*
	MOVE FROM dsa_dataout+0x01e8, WHEN DATA_OUT

at 0x000001bc : */	0x18000000,0x00000610,
/*
	MOVE FROM dsa_dataout+0x01f0, WHEN DATA_OUT

at 0x000001be : */	0x18000000,0x00000618,
/*
	MOVE FROM dsa_dataout+0x01f8, WHEN DATA_OUT

at 0x000001c0 : */	0x18000000,0x00000620,
/*
	MOVE FROM dsa_dataout+0x0200, WHEN DATA_OUT

at 0x000001c2 : */	0x18000000,0x00000628,
/*
	MOVE FROM dsa_dataout+0x0208, WHEN DATA_OUT

at 0x000001c4 : */	0x18000000,0x00000630,
/*
	MOVE FROM dsa_dataout+0x0210, WHEN DATA_OUT

at 0x000001c6 : */	0x18000000,0x00000638,
/*
	MOVE FROM dsa_dataout+0x0218, WHEN DATA_OUT

at 0x000001c8 : */	0x18000000,0x00000640,
/*
	MOVE FROM dsa_dataout+0x0220, WHEN DATA_OUT

at 0x000001ca : */	0x18000000,0x00000648,
/*
	MOVE FROM dsa_dataout+0x0228, WHEN DATA_OUT

at 0x000001cc : */	0x18000000,0x00000650,
/*
	MOVE FROM dsa_dataout+0x0230, WHEN DATA_OUT

at 0x000001ce : */	0x18000000,0x00000658,
/*
	MOVE FROM dsa_dataout+0x0238, WHEN DATA_OUT

at 0x000001d0 : */	0x18000000,0x00000660,
/*
	MOVE FROM dsa_dataout+0x0240, WHEN DATA_OUT

at 0x000001d2 : */	0x18000000,0x00000668,
/*
	MOVE FROM dsa_dataout+0x0248, WHEN DATA_OUT

at 0x000001d4 : */	0x18000000,0x00000670,
/*
	MOVE FROM dsa_dataout+0x0250, WHEN DATA_OUT

at 0x000001d6 : */	0x18000000,0x00000678,
/*
	MOVE FROM dsa_dataout+0x0258, WHEN DATA_OUT

at 0x000001d8 : */	0x18000000,0x00000680,
/*
	MOVE FROM dsa_dataout+0x0260, WHEN DATA_OUT

at 0x000001da : */	0x18000000,0x00000688,
/*
	MOVE FROM dsa_dataout+0x0268, WHEN DATA_OUT

at 0x000001dc : */	0x18000000,0x00000690,
/*
	MOVE FROM dsa_dataout+0x0270, WHEN DATA_OUT

at 0x000001de : */	0x18000000,0x00000698,
/*
	MOVE FROM dsa_dataout+0x0278, WHEN DATA_OUT

at 0x000001e0 : */	0x18000000,0x000006a0,
/*
	MOVE FROM dsa_dataout+0x0280, WHEN DATA_OUT

at 0x000001e2 : */	0x18000000,0x000006a8,
/*
	MOVE FROM dsa_dataout+0x0288, WHEN DATA_OUT

at 0x000001e4 : */	0x18000000,0x000006b0,
/*
	MOVE FROM dsa_dataout+0x0290, WHEN DATA_OUT

at 0x000001e6 : */	0x18000000,0x000006b8,
/*
	MOVE FROM dsa_dataout+0x0298, WHEN DATA_OUT

at 0x000001e8 : */	0x18000000,0x000006c0,
/*
	MOVE FROM dsa_dataout+0x02a0, WHEN DATA_OUT

at 0x000001ea : */	0x18000000,0x000006c8,
/*
	MOVE FROM dsa_dataout+0x02a8, WHEN DATA_OUT

at 0x000001ec : */	0x18000000,0x000006d0,
/*
	MOVE FROM dsa_dataout+0x02b0, WHEN DATA_OUT

at 0x000001ee : */	0x18000000,0x000006d8,
/*
	MOVE FROM dsa_dataout+0x02b8, WHEN DATA_OUT

at 0x000001f0 : */	0x18000000,0x000006e0,
/*
	MOVE FROM dsa_dataout+0x02c0, WHEN DATA_OUT

at 0x000001f2 : */	0x18000000,0x000006e8,
/*
	MOVE FROM dsa_dataout+0x02c8, WHEN DATA_OUT

at 0x000001f4 : */	0x18000000,0x000006f0,
/*
	MOVE FROM dsa_dataout+0x02d0, WHEN DATA_OUT

at 0x000001f6 : */	0x18000000,0x000006f8,
/*
	MOVE FROM dsa_dataout+0x02d8, WHEN DATA_OUT

at 0x000001f8 : */	0x18000000,0x00000700,
/*
	MOVE FROM dsa_dataout+0x02e0, WHEN DATA_OUT

at 0x000001fa : */	0x18000000,0x00000708,
/*
	MOVE FROM dsa_dataout+0x02e8, WHEN DATA_OUT

at 0x000001fc : */	0x18000000,0x00000710,
/*
	MOVE FROM dsa_dataout+0x02f0, WHEN DATA_OUT

at 0x000001fe : */	0x18000000,0x00000718,
/*
	MOVE FROM dsa_dataout+0x02f8, WHEN DATA_OUT

at 0x00000200 : */	0x18000000,0x00000720,
/*
	MOVE FROM dsa_dataout+0x0300, WHEN DATA_OUT

at 0x00000202 : */	0x18000000,0x00000728,
/*
	MOVE FROM dsa_dataout+0x0308, WHEN DATA_OUT

at 0x00000204 : */	0x18000000,0x00000730,
/*
	MOVE FROM dsa_dataout+0x0310, WHEN DATA_OUT

at 0x00000206 : */	0x18000000,0x00000738,
/*
	MOVE FROM dsa_dataout+0x0318, WHEN DATA_OUT

at 0x00000208 : */	0x18000000,0x00000740,
/*
	MOVE FROM dsa_dataout+0x0320, WHEN DATA_OUT

at 0x0000020a : */	0x18000000,0x00000748,
/*
	MOVE FROM dsa_dataout+0x0328, WHEN DATA_OUT

at 0x0000020c : */	0x18000000,0x00000750,
/*
	MOVE FROM dsa_dataout+0x0330, WHEN DATA_OUT

at 0x0000020e : */	0x18000000,0x00000758,
/*
	MOVE FROM dsa_dataout+0x0338, WHEN DATA_OUT

at 0x00000210 : */	0x18000000,0x00000760,
/*
	MOVE FROM dsa_dataout+0x0340, WHEN DATA_OUT

at 0x00000212 : */	0x18000000,0x00000768,
/*
	MOVE FROM dsa_dataout+0x0348, WHEN DATA_OUT

at 0x00000214 : */	0x18000000,0x00000770,
/*
	MOVE FROM dsa_dataout+0x0350, WHEN DATA_OUT

at 0x00000216 : */	0x18000000,0x00000778,
/*
	MOVE FROM dsa_dataout+0x0358, WHEN DATA_OUT

at 0x00000218 : */	0x18000000,0x00000780,
/*
	MOVE FROM dsa_dataout+0x0360, WHEN DATA_OUT

at 0x0000021a : */	0x18000000,0x00000788,
/*
	MOVE FROM dsa_dataout+0x0368, WHEN DATA_OUT

at 0x0000021c : */	0x18000000,0x00000790,
/*
	MOVE FROM dsa_dataout+0x0370, WHEN DATA_OUT

at 0x0000021e : */	0x18000000,0x00000798,
/*
	MOVE FROM dsa_dataout+0x0378, WHEN DATA_OUT

at 0x00000220 : */	0x18000000,0x000007a0,
/*
	MOVE FROM dsa_dataout+0x0380, WHEN DATA_OUT

at 0x00000222 : */	0x18000000,0x000007a8,
/*
	MOVE FROM dsa_dataout+0x0388, WHEN DATA_OUT

at 0x00000224 : */	0x18000000,0x000007b0,
/*
	MOVE FROM dsa_dataout+0x0390, WHEN DATA_OUT

at 0x00000226 : */	0x18000000,0x000007b8,
/*
	MOVE FROM dsa_dataout+0x0398, WHEN DATA_OUT

at 0x00000228 : */	0x18000000,0x000007c0,
/*
	MOVE FROM dsa_dataout+0x03a0, WHEN DATA_OUT

at 0x0000022a : */	0x18000000,0x000007c8,
/*
	MOVE FROM dsa_dataout+0x03a8, WHEN DATA_OUT

at 0x0000022c : */	0x18000000,0x000007d0,
/*
	MOVE FROM dsa_dataout+0x03b0, WHEN DATA_OUT

at 0x0000022e : */	0x18000000,0x000007d8,
/*
	MOVE FROM dsa_dataout+0x03b8, WHEN DATA_OUT

at 0x00000230 : */	0x18000000,0x000007e0,
/*
	MOVE FROM dsa_dataout+0x03c0, WHEN DATA_OUT

at 0x00000232 : */	0x18000000,0x000007e8,
/*
	MOVE FROM dsa_dataout+0x03c8, WHEN DATA_OUT

at 0x00000234 : */	0x18000000,0x000007f0,
/*
	MOVE FROM dsa_dataout+0x03d0, WHEN DATA_OUT

at 0x00000236 : */	0x18000000,0x000007f8,
/*
	MOVE FROM dsa_dataout+0x03d8, WHEN DATA_OUT

at 0x00000238 : */	0x18000000,0x00000800,
/*
	MOVE FROM dsa_dataout+0x03e0, WHEN DATA_OUT

at 0x0000023a : */	0x18000000,0x00000808,
/*
	MOVE FROM dsa_dataout+0x03e8, WHEN DATA_OUT

at 0x0000023c : */	0x18000000,0x00000810,
/*
	MOVE FROM dsa_dataout+0x03f0, WHEN DATA_OUT

at 0x0000023e : */	0x18000000,0x00000818,
/*
	MOVE FROM dsa_dataout+0x03f8, WHEN DATA_OUT

at 0x00000240 : */	0x18000000,0x00000820,
/*
ENTRY end_data_trans
end_data_trans:
redo_msgin3:
	JUMP get_status, WHEN STATUS

at 0x00000242 : */	0x830b0000,0x000000a0,
/*
	JUMP get_msgin3, WHEN MSG_IN

at 0x00000244 : */	0x870b0000,0x00000b20,
/*
	INT int_data_bad_phase

at 0x00000246 : */	0x98080000,0xab93000b,
/*

get_msgin1:
	MOVE SCRATCH0 | had_msgin TO SCRATCH0

at 0x00000248 : */	0x7a344000,0x00000000,
/*
	MOVE 1, msgin_buf, WHEN MSG_IN

at 0x0000024a : */	0x0f000001,0x00000000,
/*
	JUMP ext_msg1, IF 0x01		; Extended Message

at 0x0000024c : */	0x800c0001,0x00000968,
/*
	JUMP ignore_msg1, IF 0x02	; Save Data Pointers

at 0x0000024e : */	0x800c0002,0x00000958,
/*
	JUMP ignore_msg1, IF 0x03	; Save Restore Pointers

at 0x00000250 : */	0x800c0003,0x00000958,
/*
	JUMP disc1, IF 0x04		; Disconnect

at 0x00000252 : */	0x800c0004,0x000009c8,
/*
	INT int_bad_msg1

at 0x00000254 : */	0x98080000,0xab930006,
/*
ignore_msg1:
	CLEAR ACK

at 0x00000256 : */	0x60000040,0x00000000,
/*
	JUMP redo_msgin1

at 0x00000258 : */	0x80080000,0x00000058,
/*
ext_msg1:
	MOVE SCRATCH0 | had_extmsg TO SCRATCH0

at 0x0000025a : */	0x7a348000,0x00000000,
/*
	CLEAR ACK

at 0x0000025c : */	0x60000040,0x00000000,
/*
	MOVE 1, msgin_buf + 1, WHEN MSG_IN

at 0x0000025e : */	0x0f000001,0x00000001,
/*
	JUMP ext_msg1a, IF 0x03

at 0x00000260 : */	0x800c0003,0x00000990,
/*
	INT int_bad_extmsg1a

at 0x00000262 : */	0x98080000,0xab930000,
/*
ext_msg1a:
	CLEAR ACK

at 0x00000264 : */	0x60000040,0x00000000,
/*
	MOVE 1, msgin_buf + 2, WHEN MSG_IN

at 0x00000266 : */	0x0f000001,0x00000002,
/*
	JUMP ext_msg1b, IF 0x01		; Must be SDTR

at 0x00000268 : */	0x800c0001,0x000009b0,
/*
	INT int_bad_extmsg1b

at 0x0000026a : */	0x98080000,0xab930001,
/*
ext_msg1b:
	CLEAR ACK

at 0x0000026c : */	0x60000040,0x00000000,
/*
	MOVE 2, msgin_buf + 3, WHEN MSG_IN

at 0x0000026e : */	0x0f000002,0x00000003,
/*
	INT int_msg_sdtr1

at 0x00000270 : */	0x98080000,0xab93000c,
/*
disc1:
	CLEAR ACK

at 0x00000272 : */	0x60000040,0x00000000,
/*
ENTRY wait_disc1
wait_disc1:
	WAIT DISCONNECT

at 0x00000274 : */	0x48000000,0x00000000,
/*
	INT int_disc1

at 0x00000276 : */	0x98080000,0xab930019,
/*
ENTRY resume_msgin1a
resume_msgin1a:
	CLEAR ACK

at 0x00000278 : */	0x60000040,0x00000000,
/*
	JUMP redo_msgin1

at 0x0000027a : */	0x80080000,0x00000058,
/*
ENTRY resume_msgin1b
resume_msgin1b:
	SET ATN

at 0x0000027c : */	0x58000008,0x00000000,
/*
	CLEAR ACK

at 0x0000027e : */	0x60000040,0x00000000,
/*
	INT int_no_msgout1, WHEN NOT MSG_OUT

at 0x00000280 : */	0x9e030000,0xab93000f,
/*
	MOVE SCRATCH0 | had_msgout TO SCRATCH0

at 0x00000282 : */	0x7a340200,0x00000000,
/*
	MOVE FROM dsa_msgout, when MSG_OUT

at 0x00000284 : */	0x1e000000,0x00000008,
/*
	JUMP redo_msgin1

at 0x00000286 : */	0x80080000,0x00000058,
/*

get_msgin2:
	MOVE SCRATCH0 | had_msgin TO SCRATCH0

at 0x00000288 : */	0x7a344000,0x00000000,
/*
	MOVE 1, msgin_buf, WHEN MSG_IN

at 0x0000028a : */	0x0f000001,0x00000000,
/*
	JUMP ext_msg2, IF 0x01		; Extended Message

at 0x0000028c : */	0x800c0001,0x00000a68,
/*
	JUMP ignore_msg2, IF 0x02	; Save Data Pointers

at 0x0000028e : */	0x800c0002,0x00000a58,
/*
	JUMP ignore_msg2, IF 0x03	; Save Restore Pointers

at 0x00000290 : */	0x800c0003,0x00000a58,
/*
	JUMP disc2, IF 0x04		; Disconnect

at 0x00000292 : */	0x800c0004,0x00000ac8,
/*
	INT int_bad_msg2

at 0x00000294 : */	0x98080000,0xab930007,
/*
ignore_msg2:
	CLEAR ACK

at 0x00000296 : */	0x60000040,0x00000000,
/*
	JUMP redo_msgin2

at 0x00000298 : */	0x80080000,0x00000078,
/*
ext_msg2:
	MOVE SCRATCH0 | had_extmsg TO SCRATCH0

at 0x0000029a : */	0x7a348000,0x00000000,
/*
	CLEAR ACK

at 0x0000029c : */	0x60000040,0x00000000,
/*
	MOVE 1, msgin_buf + 1, WHEN MSG_IN

at 0x0000029e : */	0x0f000001,0x00000001,
/*
	JUMP ext_msg2a, IF 0x03

at 0x000002a0 : */	0x800c0003,0x00000a90,
/*
	INT int_bad_extmsg2a

at 0x000002a2 : */	0x98080000,0xab930002,
/*
ext_msg2a:
	CLEAR ACK

at 0x000002a4 : */	0x60000040,0x00000000,
/*
	MOVE 1, msgin_buf + 2, WHEN MSG_IN

at 0x000002a6 : */	0x0f000001,0x00000002,
/*
	JUMP ext_msg2b, IF 0x01		; Must be SDTR

at 0x000002a8 : */	0x800c0001,0x00000ab0,
/*
	INT int_bad_extmsg2b

at 0x000002aa : */	0x98080000,0xab930003,
/*
ext_msg2b:
	CLEAR ACK

at 0x000002ac : */	0x60000040,0x00000000,
/*
	MOVE 2, msgin_buf + 3, WHEN MSG_IN

at 0x000002ae : */	0x0f000002,0x00000003,
/*
	INT int_msg_sdtr2

at 0x000002b0 : */	0x98080000,0xab93000d,
/*
disc2:
	CLEAR ACK

at 0x000002b2 : */	0x60000040,0x00000000,
/*
ENTRY wait_disc2
wait_disc2:
	WAIT DISCONNECT

at 0x000002b4 : */	0x48000000,0x00000000,
/*
	INT int_disc2

at 0x000002b6 : */	0x98080000,0xab93001a,
/*
ENTRY resume_msgin2a
resume_msgin2a:
	CLEAR ACK

at 0x000002b8 : */	0x60000040,0x00000000,
/*
	JUMP redo_msgin2

at 0x000002ba : */	0x80080000,0x00000078,
/*
ENTRY resume_msgin2b
resume_msgin2b:
	SET ATN

at 0x000002bc : */	0x58000008,0x00000000,
/*
	CLEAR ACK

at 0x000002be : */	0x60000040,0x00000000,
/*
	INT int_no_msgout2, WHEN NOT MSG_OUT

at 0x000002c0 : */	0x9e030000,0xab930010,
/*
	MOVE SCRATCH0 | had_msgout TO SCRATCH0

at 0x000002c2 : */	0x7a340200,0x00000000,
/*
	MOVE FROM dsa_msgout, when MSG_OUT

at 0x000002c4 : */	0x1e000000,0x00000008,
/*
	JUMP redo_msgin2

at 0x000002c6 : */	0x80080000,0x00000078,
/*

get_msgin3:
	MOVE SCRATCH0 | had_msgin TO SCRATCH0

at 0x000002c8 : */	0x7a344000,0x00000000,
/*
	MOVE 1, msgin_buf, WHEN MSG_IN

at 0x000002ca : */	0x0f000001,0x00000000,
/*
	JUMP ext_msg3, IF 0x01		; Extended Message

at 0x000002cc : */	0x800c0001,0x00000b68,
/*
	JUMP ignore_msg3, IF 0x02	; Save Data Pointers

at 0x000002ce : */	0x800c0002,0x00000b58,
/*
	JUMP ignore_msg3, IF 0x03	; Save Restore Pointers

at 0x000002d0 : */	0x800c0003,0x00000b58,
/*
	JUMP disc3, IF 0x04		; Disconnect

at 0x000002d2 : */	0x800c0004,0x00000bc8,
/*
	INT int_bad_msg3

at 0x000002d4 : */	0x98080000,0xab930008,
/*
ignore_msg3:
	CLEAR ACK

at 0x000002d6 : */	0x60000040,0x00000000,
/*
	JUMP redo_msgin3

at 0x000002d8 : */	0x80080000,0x00000908,
/*
ext_msg3:
	MOVE SCRATCH0 | had_extmsg TO SCRATCH0

at 0x000002da : */	0x7a348000,0x00000000,
/*
	CLEAR ACK

at 0x000002dc : */	0x60000040,0x00000000,
/*
	MOVE 1, msgin_buf + 1, WHEN MSG_IN

at 0x000002de : */	0x0f000001,0x00000001,
/*
	JUMP ext_msg3a, IF 0x03

at 0x000002e0 : */	0x800c0003,0x00000b90,
/*
	INT int_bad_extmsg3a

at 0x000002e2 : */	0x98080000,0xab930004,
/*
ext_msg3a:
	CLEAR ACK

at 0x000002e4 : */	0x60000040,0x00000000,
/*
	MOVE 1, msgin_buf + 2, WHEN MSG_IN

at 0x000002e6 : */	0x0f000001,0x00000002,
/*
	JUMP ext_msg3b, IF 0x01		; Must be SDTR

at 0x000002e8 : */	0x800c0001,0x00000bb0,
/*
	INT int_bad_extmsg3b

at 0x000002ea : */	0x98080000,0xab930005,
/*
ext_msg3b:
	CLEAR ACK

at 0x000002ec : */	0x60000040,0x00000000,
/*
	MOVE 2, msgin_buf + 3, WHEN MSG_IN

at 0x000002ee : */	0x0f000002,0x00000003,
/*
	INT int_msg_sdtr3

at 0x000002f0 : */	0x98080000,0xab93000e,
/*
disc3:
	CLEAR ACK

at 0x000002f2 : */	0x60000040,0x00000000,
/*
ENTRY wait_disc3
wait_disc3:
	WAIT DISCONNECT

at 0x000002f4 : */	0x48000000,0x00000000,
/*
	INT int_disc3

at 0x000002f6 : */	0x98080000,0xab93001b,
/*
ENTRY resume_msgin3a
resume_msgin3a:
	CLEAR ACK

at 0x000002f8 : */	0x60000040,0x00000000,
/*
	JUMP redo_msgin3

at 0x000002fa : */	0x80080000,0x00000908,
/*
ENTRY resume_msgin3b
resume_msgin3b:
	SET ATN

at 0x000002fc : */	0x58000008,0x00000000,
/*
	CLEAR ACK

at 0x000002fe : */	0x60000040,0x00000000,
/*
	INT int_no_msgout3, WHEN NOT MSG_OUT

at 0x00000300 : */	0x9e030000,0xab930011,
/*
	MOVE SCRATCH0 | had_msgout TO SCRATCH0

at 0x00000302 : */	0x7a340200,0x00000000,
/*
	MOVE FROM dsa_msgout, when MSG_OUT

at 0x00000304 : */	0x1e000000,0x00000008,
/*
	JUMP redo_msgin3

at 0x00000306 : */	0x80080000,0x00000908,
/*

ENTRY resume_rej_ident
resume_rej_ident:
	CLEAR ATN

at 0x00000308 : */	0x60000008,0x00000000,
/*
	MOVE 1, msgin_buf, WHEN MSG_IN

at 0x0000030a : */	0x0f000001,0x00000000,
/*
	INT int_not_rej, IF NOT 0x07		; Reject

at 0x0000030c : */	0x98040007,0xab93001c,
/*
	CLEAR ACK

at 0x0000030e : */	0x60000040,0x00000000,
/*
	JUMP done_ident

at 0x00000310 : */	0x80080000,0x00000050,
/*

ENTRY reselect
reselect:
	; Disable selection timer
	MOVE CTEST7 | 0x10 TO CTEST7

at 0x00000312 : */	0x7a1b1000,0x00000000,
/*
	WAIT RESELECT resel_err

at 0x00000314 : */	0x50000000,0x00000c70,
/*
	INT int_resel_not_msgin, WHEN NOT MSG_IN

at 0x00000316 : */	0x9f030000,0xab930016,
/*
	MOVE 1, reselected_identify, WHEN MSG_IN

at 0x00000318 : */	0x0f000001,0x00000000,
/*
	INT int_reselected

at 0x0000031a : */	0x98080000,0xab930017,
/*
resel_err:
	MOVE CTEST2 & 0x40 TO SFBR

at 0x0000031c : */	0x74164000,0x00000000,
/*
	JUMP selected, IF 0x00

at 0x0000031e : */	0x800c0000,0x00000cb0,
/*
	MOVE SFBR & 0 TO SFBR

at 0x00000320 : */	0x7c080000,0x00000000,
/*
ENTRY patch_new_dsa
patch_new_dsa:
	MOVE SFBR | 0x11 TO DSA0

at 0x00000322 : */	0x6a101100,0x00000000,
/*
	MOVE SFBR | 0x22 TO DSA1

at 0x00000324 : */	0x6a112200,0x00000000,
/*
	MOVE SFBR | 0x33 TO DSA2

at 0x00000326 : */	0x6a123300,0x00000000,
/*
	MOVE SFBR | 0x44 TO DSA3

at 0x00000328 : */	0x6a134400,0x00000000,
/*
	JUMP do_select

at 0x0000032a : */	0x80080000,0x00000000,
/*

selected:
	INT int_selected

at 0x0000032c : */	0x98080000,0xab930018,
};

#define A_dsa_cmnd	0x00000010
static u32 A_dsa_cmnd_used[] __attribute((unused)) = {
	0x0000001d,
};

#define A_dsa_datain	0x00000028
static u32 A_dsa_datain_used[] __attribute((unused)) = {
	0x0000003d,
	0x0000003f,
	0x00000041,
	0x00000043,
	0x00000045,
	0x00000047,
	0x00000049,
	0x0000004b,
	0x0000004d,
	0x0000004f,
	0x00000051,
	0x00000053,
	0x00000055,
	0x00000057,
	0x00000059,
	0x0000005b,
	0x0000005d,
	0x0000005f,
	0x00000061,
	0x00000063,
	0x00000065,
	0x00000067,
	0x00000069,
	0x0000006b,
	0x0000006d,
	0x0000006f,
	0x00000071,
	0x00000073,
	0x00000075,
	0x00000077,
	0x00000079,
	0x0000007b,
	0x0000007d,
	0x0000007f,
	0x00000081,
	0x00000083,
	0x00000085,
	0x00000087,
	0x00000089,
	0x0000008b,
	0x0000008d,
	0x0000008f,
	0x00000091,
	0x00000093,
	0x00000095,
	0x00000097,
	0x00000099,
	0x0000009b,
	0x0000009d,
	0x0000009f,
	0x000000a1,
	0x000000a3,
	0x000000a5,
	0x000000a7,
	0x000000a9,
	0x000000ab,
	0x000000ad,
	0x000000af,
	0x000000b1,
	0x000000b3,
	0x000000b5,
	0x000000b7,
	0x000000b9,
	0x000000bb,
	0x000000bd,
	0x000000bf,
	0x000000c1,
	0x000000c3,
	0x000000c5,
	0x000000c7,
	0x000000c9,
	0x000000cb,
	0x000000cd,
	0x000000cf,
	0x000000d1,
	0x000000d3,
	0x000000d5,
	0x000000d7,
	0x000000d9,
	0x000000db,
	0x000000dd,
	0x000000df,
	0x000000e1,
	0x000000e3,
	0x000000e5,
	0x000000e7,
	0x000000e9,
	0x000000eb,
	0x000000ed,
	0x000000ef,
	0x000000f1,
	0x000000f3,
	0x000000f5,
	0x000000f7,
	0x000000f9,
	0x000000fb,
	0x000000fd,
	0x000000ff,
	0x00000101,
	0x00000103,
	0x00000105,
	0x00000107,
	0x00000109,
	0x0000010b,
	0x0000010d,
	0x0000010f,
	0x00000111,
	0x00000113,
	0x00000115,
	0x00000117,
	0x00000119,
	0x0000011b,
	0x0000011d,
	0x0000011f,
	0x00000121,
	0x00000123,
	0x00000125,
	0x00000127,
	0x00000129,
	0x0000012b,
	0x0000012d,
	0x0000012f,
	0x00000131,
	0x00000133,
	0x00000135,
	0x00000137,
	0x00000139,
	0x0000013b,
};

#define A_dsa_dataout	0x00000428
static u32 A_dsa_dataout_used[] __attribute((unused)) = {
	0x00000143,
	0x00000145,
	0x00000147,
	0x00000149,
	0x0000014b,
	0x0000014d,
	0x0000014f,
	0x00000151,
	0x00000153,
	0x00000155,
	0x00000157,
	0x00000159,
	0x0000015b,
	0x0000015d,
	0x0000015f,
	0x00000161,
	0x00000163,
	0x00000165,
	0x00000167,
	0x00000169,
	0x0000016b,
	0x0000016d,
	0x0000016f,
	0x00000171,
	0x00000173,
	0x00000175,
	0x00000177,
	0x00000179,
	0x0000017b,
	0x0000017d,
	0x0000017f,
	0x00000181,
	0x00000183,
	0x00000185,
	0x00000187,
	0x00000189,
	0x0000018b,
	0x0000018d,
	0x0000018f,
	0x00000191,
	0x00000193,
	0x00000195,
	0x00000197,
	0x00000199,
	0x0000019b,
	0x0000019d,
	0x0000019f,
	0x000001a1,
	0x000001a3,
	0x000001a5,
	0x000001a7,
	0x000001a9,
	0x000001ab,
	0x000001ad,
	0x000001af,
	0x000001b1,
	0x000001b3,
	0x000001b5,
	0x000001b7,
	0x000001b9,
	0x000001bb,
	0x000001bd,
	0x000001bf,
	0x000001c1,
	0x000001c3,
	0x000001c5,
	0x000001c7,
	0x000001c9,
	0x000001cb,
	0x000001cd,
	0x000001cf,
	0x000001d1,
	0x000001d3,
	0x000001d5,
	0x000001d7,
	0x000001d9,
	0x000001db,
	0x000001dd,
	0x000001df,
	0x000001e1,
	0x000001e3,
	0x000001e5,
	0x000001e7,
	0x000001e9,
	0x000001eb,
	0x000001ed,
	0x000001ef,
	0x000001f1,
	0x000001f3,
	0x000001f5,
	0x000001f7,
	0x000001f9,
	0x000001fb,
	0x000001fd,
	0x000001ff,
	0x00000201,
	0x00000203,
	0x00000205,
	0x00000207,
	0x00000209,
	0x0000020b,
	0x0000020d,
	0x0000020f,
	0x00000211,
	0x00000213,
	0x00000215,
	0x00000217,
	0x00000219,
	0x0000021b,
	0x0000021d,
	0x0000021f,
	0x00000221,
	0x00000223,
	0x00000225,
	0x00000227,
	0x00000229,
	0x0000022b,
	0x0000022d,
	0x0000022f,
	0x00000231,
	0x00000233,
	0x00000235,
	0x00000237,
	0x00000239,
	0x0000023b,
	0x0000023d,
	0x0000023f,
	0x00000241,
};

#define A_dsa_msgin	0x00000020
static u32 A_dsa_msgin_used[] __attribute((unused)) = {
	0x0000002f,
};

#define A_dsa_msgout	0x00000008
static u32 A_dsa_msgout_used[] __attribute((unused)) = {
	0x00000013,
	0x00000285,
	0x000002c5,
	0x00000305,
};

#define A_dsa_select	0x00000000
static u32 A_dsa_select_used[] __attribute((unused)) = {
	0x00000006,
};

#define A_dsa_size	0x00000828
static u32 A_dsa_size_used[] __attribute((unused)) = {
};

#define A_dsa_status	0x00000018
static u32 A_dsa_status_used[] __attribute((unused)) = {
	0x0000002b,
};

#define A_had_cmdout	0x00000004
static u32 A_had_cmdout_used[] __attribute((unused)) = {
	0x0000001a,
};

#define A_had_datain	0x00000008
static u32 A_had_datain_used[] __attribute((unused)) = {
	0x00000038,
};

#define A_had_dataout	0x00000010
static u32 A_had_dataout_used[] __attribute((unused)) = {
	0x0000013e,
};

#define A_had_extmsg	0x00000080
static u32 A_had_extmsg_used[] __attribute((unused)) = {
	0x0000025a,
	0x0000029a,
	0x000002da,
};

#define A_had_msgin	0x00000040
static u32 A_had_msgin_used[] __attribute((unused)) = {
	0x00000248,
	0x00000288,
	0x000002c8,
};

#define A_had_msgout	0x00000002
static u32 A_had_msgout_used[] __attribute((unused)) = {
	0x00000010,
	0x00000282,
	0x000002c2,
	0x00000302,
};

#define A_had_select	0x00000001
static u32 A_had_select_used[] __attribute((unused)) = {
	0x0000000c,
};

#define A_had_status	0x00000020
static u32 A_had_status_used[] __attribute((unused)) = {
};

#define A_int_bad_extmsg1a	0xab930000
static u32 A_int_bad_extmsg1a_used[] __attribute((unused)) = {
	0x00000263,
};

#define A_int_bad_extmsg1b	0xab930001
static u32 A_int_bad_extmsg1b_used[] __attribute((unused)) = {
	0x0000026b,
};

#define A_int_bad_extmsg2a	0xab930002
static u32 A_int_bad_extmsg2a_used[] __attribute((unused)) = {
	0x000002a3,
};

#define A_int_bad_extmsg2b	0xab930003
static u32 A_int_bad_extmsg2b_used[] __attribute((unused)) = {
	0x000002ab,
};

#define A_int_bad_extmsg3a	0xab930004
static u32 A_int_bad_extmsg3a_used[] __attribute((unused)) = {
	0x000002e3,
};

#define A_int_bad_extmsg3b	0xab930005
static u32 A_int_bad_extmsg3b_used[] __attribute((unused)) = {
	0x000002eb,
};

#define A_int_bad_msg1	0xab930006
static u32 A_int_bad_msg1_used[] __attribute((unused)) = {
	0x00000255,
};

#define A_int_bad_msg2	0xab930007
static u32 A_int_bad_msg2_used[] __attribute((unused)) = {
	0x00000295,
};

#define A_int_bad_msg3	0xab930008
static u32 A_int_bad_msg3_used[] __attribute((unused)) = {
	0x000002d5,
};

#define A_int_cmd_bad_phase	0xab930009
static u32 A_int_cmd_bad_phase_used[] __attribute((unused)) = {
	0x00000027,
};

#define A_int_cmd_complete	0xab93000a
static u32 A_int_cmd_complete_used[] __attribute((unused)) = {
	0x00000037,
};

#define A_int_data_bad_phase	0xab93000b
static u32 A_int_data_bad_phase_used[] __attribute((unused)) = {
	0x00000247,
};

#define A_int_disc1	0xab930019
static u32 A_int_disc1_used[] __attribute((unused)) = {
	0x00000277,
};

#define A_int_disc2	0xab93001a
static u32 A_int_disc2_used[] __attribute((unused)) = {
	0x000002b7,
};

#define A_int_disc3	0xab93001b
static u32 A_int_disc3_used[] __attribute((unused)) = {
	0x000002f7,
};

#define A_int_msg_sdtr1	0xab93000c
static u32 A_int_msg_sdtr1_used[] __attribute((unused)) = {
	0x00000271,
};

#define A_int_msg_sdtr2	0xab93000d
static u32 A_int_msg_sdtr2_used[] __attribute((unused)) = {
	0x000002b1,
};

#define A_int_msg_sdtr3	0xab93000e
static u32 A_int_msg_sdtr3_used[] __attribute((unused)) = {
	0x000002f1,
};

#define A_int_no_msgout1	0xab93000f
static u32 A_int_no_msgout1_used[] __attribute((unused)) = {
	0x00000281,
};

#define A_int_no_msgout2	0xab930010
static u32 A_int_no_msgout2_used[] __attribute((unused)) = {
	0x000002c1,
};

#define A_int_no_msgout3	0xab930011
static u32 A_int_no_msgout3_used[] __attribute((unused)) = {
	0x00000301,
};

#define A_int_not_cmd_complete	0xab930012
static u32 A_int_not_cmd_complete_used[] __attribute((unused)) = {
	0x00000031,
};

#define A_int_not_rej	0xab93001c
static u32 A_int_not_rej_used[] __attribute((unused)) = {
	0x0000030d,
};

#define A_int_resel_not_msgin	0xab930016
static u32 A_int_resel_not_msgin_used[] __attribute((unused)) = {
	0x00000317,
};

#define A_int_reselected	0xab930017
static u32 A_int_reselected_used[] __attribute((unused)) = {
	0x0000031b,
};

#define A_int_sel_no_ident	0xab930013
static u32 A_int_sel_no_ident_used[] __attribute((unused)) = {
	0x0000000f,
};

#define A_int_sel_not_cmd	0xab930014
static u32 A_int_sel_not_cmd_used[] __attribute((unused)) = {
	0x00000019,
};

#define A_int_selected	0xab930018
static u32 A_int_selected_used[] __attribute((unused)) = {
	0x0000032d,
};

#define A_int_status_not_msgin	0xab930015
static u32 A_int_status_not_msgin_used[] __attribute((unused)) = {
	0x0000002d,
};

#define A_msgin_buf	0x00000000
static u32 A_msgin_buf_used[] __attribute((unused)) = {
	0x0000024b,
	0x0000025f,
	0x00000267,
	0x0000026f,
	0x0000028b,
	0x0000029f,
	0x000002a7,
	0x000002af,
	0x000002cb,
	0x000002df,
	0x000002e7,
	0x000002ef,
	0x0000030b,
};

#define A_reselected_identify	0x00000000
static u32 A_reselected_identify_used[] __attribute((unused)) = {
	0x00000319,
};

#define Ent_do_select	0x00000000
#define Ent_done_ident	0x00000050
#define Ent_end_data_trans	0x00000908
#define Ent_patch_input_data	0x000000e8
#define Ent_patch_new_dsa	0x00000c88
#define Ent_patch_output_data	0x00000500
#define Ent_reselect	0x00000c48
#define Ent_resume_cmd	0x00000068
#define Ent_resume_msgin1a	0x000009e0
#define Ent_resume_msgin1b	0x000009f0
#define Ent_resume_msgin2a	0x00000ae0
#define Ent_resume_msgin2b	0x00000af0
#define Ent_resume_msgin3a	0x00000be0
#define Ent_resume_msgin3b	0x00000bf0
#define Ent_resume_pmm	0x00000078
#define Ent_resume_rej_ident	0x00000c20
#define Ent_wait_disc1	0x000009d0
#define Ent_wait_disc2	0x00000ad0
#define Ent_wait_disc3	0x00000bd0
#define Ent_wait_disc_complete	0x000000d0
static u32 LABELPATCHES[] __attribute((unused)) = {
	0x00000007,
	0x00000009,
	0x00000015,
	0x00000017,
	0x0000001f,
	0x00000021,
	0x00000023,
	0x00000025,
	0x0000013d,
	0x00000243,
	0x00000245,
	0x0000024d,
	0x0000024f,
	0x00000251,
	0x00000253,
	0x00000259,
	0x00000261,
	0x00000269,
	0x0000027b,
	0x00000287,
	0x0000028d,
	0x0000028f,
	0x00000291,
	0x00000293,
	0x00000299,
	0x000002a1,
	0x000002a9,
	0x000002bb,
	0x000002c7,
	0x000002cd,
	0x000002cf,
	0x000002d1,
	0x000002d3,
	0x000002d9,
	0x000002e1,
	0x000002e9,
	0x000002fb,
	0x00000307,
	0x00000311,
	0x00000315,
	0x0000031f,
	0x0000032b,
};

static struct {
	u32	offset;
	void		*address;
} EXTERNAL_PATCHES[] __attribute((unused)) = {
};

static u32 INSTRUCTIONS __attribute((unused))	= 407;
static u32 PATCHES __attribute((unused))	= 42;
static u32 EXTERNAL_PATCHES_LEN __attribute((unused))	= 0;
