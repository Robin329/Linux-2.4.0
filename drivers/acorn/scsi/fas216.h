/*
 *  linux/drivers/acorn/scsi/fas216.h
 *
 *  Copyright (C) 1997-2000 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  FAS216 generic driver
 */
#ifndef FAS216_H
#define FAS216_H

#ifndef NO_IRQ
#define NO_IRQ 255
#endif

#include "queue.h"
#include "msgqueue.h"

/* FAS register definitions */

/* transfer count low */
#define REG_CTCL(x)		((x)->scsi.io_port)
#define REG_STCL(x)		((x)->scsi.io_port)

/* transfer count medium */
#define REG_CTCM(x)		((x)->scsi.io_port + (1 << (x)->scsi.io_shift))
#define REG_STCM(x)		((x)->scsi.io_port + (1 << (x)->scsi.io_shift))

/* fifo data */
#define REG_FF(x)		((x)->scsi.io_port + (2 << (x)->scsi.io_shift))

/* command */
#define REG_CMD(x)		((x)->scsi.io_port + (3 << (x)->scsi.io_shift))
#define CMD_NOP			0x00
#define CMD_FLUSHFIFO		0x01
#define CMD_RESETCHIP		0x02
#define CMD_RESETSCSI		0x03

#define CMD_TRANSFERINFO	0x10
#define CMD_INITCMDCOMPLETE	0x11
#define CMD_MSGACCEPTED		0x12
#define CMD_PADBYTES		0x18
#define CMD_SETATN		0x1a
#define CMD_RSETATN		0x1b

#define CMD_SELECTWOATN		0x41
#define CMD_SELECTATN		0x42
#define CMD_SELECTATNSTOP	0x43
#define CMD_ENABLESEL		0x44
#define CMD_DISABLESEL		0x45
#define CMD_SELECTATN3		0x46
#define CMD_RESEL3		0x47

#define CMD_WITHDMA		0x80

/* status register (read) */
#define REG_STAT(x)		((x)->scsi.io_port + (4 << (x)->scsi.io_shift))
#define STAT_IO			(1 << 0)			/* IO phase		*/
#define STAT_CD			(1 << 1)			/* CD phase		*/
#define STAT_MSG		(1 << 2)			/* MSG phase		*/
#define STAT_TRANSFERDONE	(1 << 3)			/* Transfer completed	*/
#define STAT_TRANSFERCNTZ	(1 << 4)			/* Transfer counter is zero */
#define STAT_PARITYERROR	(1 << 5)			/* Parity error		*/
#define STAT_REALBAD		(1 << 6)			/* Something bad	*/
#define STAT_INT		(1 << 7)			/* Interrupt		*/

#define STAT_BUSMASK		(STAT_MSG|STAT_CD|STAT_IO)
#define STAT_DATAOUT		(0)				/* Data out		*/
#define STAT_DATAIN		(STAT_IO)			/* Data in		*/
#define STAT_COMMAND		(STAT_CD)			/* Command out		*/
#define STAT_STATUS		(STAT_CD|STAT_IO)		/* Status In		*/
#define STAT_MESGOUT		(STAT_MSG|STAT_CD)		/* Message out		*/
#define STAT_MESGIN		(STAT_MSG|STAT_CD|STAT_IO)	/* Message In		*/

/* bus ID for select / reselect */
#define REG_SDID(x)		((x)->scsi.io_port + (4 << (x)->scsi.io_shift))
#define BUSID(target)		((target) & 7)

/* Interrupt status register (read) */
#define REG_INST(x)		((x)->scsi.io_port + (5 << (x)->scsi.io_shift))
#define INST_SELWOATN		(1 << 0)			/* Select w/o ATN	*/
#define INST_SELATN		(1 << 1)			/* Select w/ATN		*/
#define INST_RESELECTED		(1 << 2)			/* Reselected		*/
#define INST_FUNCDONE		(1 << 3)			/* Function done	*/
#define INST_BUSSERVICE		(1 << 4)			/* Bus service		*/
#define INST_DISCONNECT		(1 << 5)			/* Disconnect		*/
#define INST_ILLEGALCMD		(1 << 6)			/* Illegal command	*/
#define INST_BUSRESET		(1 << 7)			/* SCSI Bus reset	*/

/* Timeout register (write) */
#define REG_STIM(x)		((x)->scsi.io_port + (5 << (x)->scsi.io_shift))

/* Sequence step register (read) */
#define REG_IS(x)		((x)->scsi.io_port + (6 << (x)->scsi.io_shift))
#define IS_BITS			0x07
#define IS_SELARB		0x00				/* Select & Arb ok	*/
#define IS_MSGBYTESENT		0x01				/* One byte message sent*/
#define IS_NOTCOMMAND		0x02				/* Not in command state	*/
#define IS_EARLYPHASE		0x03				/* Early phase change	*/
#define IS_COMPLETE		0x04				/* Command ok		*/
#define IS_SOF			0x08				/* Sync off flag	*/

/* Transfer period step (write) */
#define REG_STP(x)		((x)->scsi.io_port + (6 << (x)->scsi.io_shift))

/* Synchronous Offset (write) */
#define REG_SOF(x)		((x)->scsi.io_port + (7 << (x)->scsi.io_shift))

/* Fifo state register (read) */
#define REG_CFIS(x)		((x)->scsi.io_port + (7 << (x)->scsi.io_shift))
#define CFIS_CF			0x1f				/* Num bytes in FIFO	*/
#define CFIS_IS			0xe0				/* Step			*/

/* config register 1 */
#define REG_CNTL1(x)		((x)->scsi.io_port + (8 << (x)->scsi.io_shift))
#define CNTL1_CID		(7 << 0)			/* Chip ID			*/
#define CNTL1_STE		(1 << 3)			/* Self test enable		*/
#define CNTL1_PERE		(1 << 4)			/* Parity enable reporting en.	*/
#define CNTL1_PTE		(1 << 5)			/* Parity test enable		*/
#define CNTL1_DISR		(1 << 6)			/* Disable Irq on SCSI reset	*/
#define CNTL1_ETM		(1 << 7)			/* Extended Timing Mode		*/

/* Clock conversion factor (read) */
#define REG_CLKF(x)		((x)->scsi.io_port + (9 << (x)->scsi.io_shift))
#define CLKF_F37MHZ		0x00				/* 35.01 - 40 MHz		*/
#define CLKF_F10MHZ		0x02				/* 10 MHz			*/
#define CLKF_F12MHZ		0x03				/* 10.01 - 15 MHz		*/
#define CLKF_F17MHZ		0x04				/* 15.01 - 20 MHz		*/
#define CLKF_F22MHZ		0x05				/* 20.01 - 25 MHz		*/
#define CLKF_F27MHZ		0x06				/* 25.01 - 30 MHz		*/
#define CLKF_F32MHZ		0x07				/* 30.01 - 35 MHz		*/

/* Chip test register (write) */
#define REG0_FTM(x)		((x)->scsi.io_port + (10 << (x)->scsi.io_shift))
#define TEST_FTM		0x01				/* Force target mode		*/
#define TEST_FIM		0x02				/* Force initiator mode		*/
#define TEST_FHI		0x04				/* Force high impedance mode	*/

/* Configuration register 2 (read/write) */
#define REG_CNTL2(x)		((x)->scsi.io_port + (11 << (x)->scsi.io_shift))
#define CNTL2_PGDP		(1 << 0)			/* Pass Th/Generate Data Parity	*/
#define CNTL2_PGRP		(1 << 1)			/* Pass Th/Generate Reg Parity	*/
#define CNTL2_ACDPE		(1 << 2)			/* Abort on Cmd/Data Parity Err	*/
#define CNTL2_S2FE		(1 << 3)			/* SCSI2 Features Enable	*/
#define CNTL2_TSDR		(1 << 4)			/* Tristate DREQ		*/
#define CNTL2_SBO		(1 << 5)			/* Select Byte Order		*/
#define CNTL2_ENF		(1 << 6)			/* Enable features		*/
#define CNTL2_DAE		(1 << 7)			/* Data Alignment Enable	*/

/* Configuration register 3 (read/write) */
#define REG_CNTL3(x)		((x)->scsi.io_port + (12 << (x)->scsi.io_shift))
#define CNTL3_BS8		(1 << 0)			/* Burst size 8			*/
#define CNTL3_MDM		(1 << 1)			/* Modify DMA mode		*/
#define CNTL3_LBTM		(1 << 2)			/* Last Byte Transfer mode	*/
#define CNTL3_FASTCLK		(1 << 3)			/* Fast SCSI clocking		*/
#define CNTL3_FASTSCSI		(1 << 4)			/* Fast SCSI			*/
#define CNTL3_G2CB		(1 << 5)			/* Group2 SCSI support		*/
#define CNTL3_QTAG		(1 << 6)			/* Enable 3 byte msgs		*/
#define CNTL3_ADIDCHK		(1 << 7)			/* Additional ID check		*/

/* High transfer count (read/write) */
#define REG_CTCH(x)		((x)->scsi.io_port + (14 << (x)->scsi.io_shift))
#define REG_STCH(x)		((x)->scsi.io_port + (14 << (x)->scsi.io_shift))

/* ID reigster (read only) */
#define REG1_ID(x)		((x)->scsi.io_port + (14 << (x)->scsi.io_shift))

/* Data alignment */
#define REG0_DAL(x)		((x)->scsi.io_port + (15 << (x)->scsi.io_shift))

typedef enum {
	PHASE_IDLE,					/* we're not planning on doing anything	*/
	PHASE_SELECTION,				/* selecting a device			*/
	PHASE_SELSTEPS,					/* selection with command steps		*/
	PHASE_COMMAND,					/* command sent				*/
	PHASE_MESSAGESENT,				/* selected, and we're sending cmd	*/
	PHASE_RECONNECTED,				/* reconnected				*/
	PHASE_DATAOUT,					/* data out to device			*/
	PHASE_DATAIN,					/* data in from device			*/
	PHASE_MSGIN,					/* message in from device		*/
	PHASE_MSGIN_DISCONNECT,				/* disconnecting from bus		*/
	PHASE_MSGOUT,					/* after message out phase		*/
	PHASE_MSGOUT_EXPECT,				/* expecting message out		*/
	PHASE_STATUS,					/* status from device			*/
	PHASE_DONE					/* Command complete			*/
} phase_t;

typedef enum {
	DMA_OUT,					/* DMA from memory to chip		*/
	DMA_IN						/* DMA from chip to memory		*/
} fasdmadir_t;

typedef enum {
	fasdma_none,					/* No dma				*/
	fasdma_pio,					/* PIO mode				*/
	fasdma_pseudo,					/* Pseudo DMA				*/
	fasdma_real_block,				/* Real DMA, on block by block basis	*/
	fasdma_real_all					/* Real DMA, on request by request	*/
} fasdmatype_t;

typedef enum {
	neg_wait,					/* Negociate with device		*/
	neg_inprogress,					/* Negociation sent			*/
	neg_complete,					/* Negociation complete			*/
	neg_targcomplete,				/* Target completed negociation		*/
	neg_invalid					/* Negociation not supported		*/
} neg_t;

#define MAGIC	0x441296bdUL
#define NR_MSGS	8

typedef struct {
	unsigned long		magic_start;
	struct Scsi_Host	*host;			/* host					*/
	Scsi_Cmnd		*SCpnt;			/* currently processing command		*/
	Scsi_Cmnd		*origSCpnt;		/* original connecting command		*/
	Scsi_Cmnd		*reqSCpnt;		/* request sense command		*/

	/* driver information */
	struct {
		unsigned int	io_port;		/* base address of FAS216		*/
		unsigned int	io_shift;		/* shift to adjust reg offsets by	*/
		unsigned int	irq;			/* interrupt				*/
		unsigned char	cfg[4];			/* configuration registers		*/
		const char	*type;			/* chip type				*/
		phase_t		phase;			/* current phase			*/

		struct {
			unsigned char	target;		/* reconnected target			*/
			unsigned char	lun;		/* reconnected lun			*/
			unsigned char	tag;		/* reconnected tag			*/
		} reconnected;

		Scsi_Pointer	SCp;			/* current commands data pointer	*/

		MsgQueue_t	msgs;			/* message queue for connected device	*/

		unsigned int	async_stp;		/* Async transfer STP value		*/
		unsigned char	msgin_fifo;		/* bytes in fifo at time of message in	*/
		unsigned char	message[256];		/* last message received from device	*/
		unsigned int	msglen;			/* length of last message received	*/

		unsigned char	disconnectable:1;	/* this command can be disconnected	*/
		unsigned char	aborting:1;		/* aborting command			*/
	} scsi;

	/* statistics information */
	struct {
		unsigned int	queues;
		unsigned int	removes;
		unsigned int	fins;
		unsigned int	reads;
		unsigned int	writes;
		unsigned int	miscs;
		unsigned int	disconnects;
		unsigned int	aborts;
		unsigned int	bus_resets;
		unsigned int	host_resets;
	} stats;

	/* configuration information */
	struct {
		unsigned char	clockrate;		/* clock rate of FAS device (MHz)	*/
		unsigned char	select_timeout;		/* timeout (R5)				*/
		unsigned char	sync_max_depth;		/* Synchronous xfer max fifo depth	*/
		unsigned char	wide_max_size;		/* Maximum wide transfer size		*/
		unsigned char	cntl3;			/* Control Reg 3			*/
		unsigned int	asyncperiod;		/* Async transfer period (ns)		*/
		unsigned int	disconnect_ok:1;	/* Disconnects allowed?			*/
	} ifcfg;

	/* queue handling */
	struct {
	    	Queue_t		issue;			/* issue queue				*/
    		Queue_t		disconnected;		/* disconnected command queue		*/
	} queues;

	/* per-device info */
	struct fas216_device {
		unsigned char	disconnect_ok:1;	/* device can disconnect		*/
		unsigned char	period;			/* sync xfer period in (*4ns)		*/
		unsigned char	stp;			/* synchronous transfer period		*/
		unsigned char	sof;			/* synchronous offset register		*/
		unsigned char	wide_xfer;		/* currently negociated wide transfer	*/
		neg_t		sync_state;		/* synchronous transfer mode		*/
		neg_t		wide_state;		/* wide transfer mode			*/
	} device[8];
	unsigned char	busyluns[8];			/* array of bits indicating LUNs busy	*/

	/* dma */
	struct {
		fasdmatype_t	transfer_type;		/* current type of DMA transfer		*/
		fasdmatype_t	(*setup) (struct Scsi_Host *host, Scsi_Pointer *SCp, fasdmadir_t direction, fasdmatype_t min_dma);
		void		(*pseudo)(struct Scsi_Host *host, Scsi_Pointer *SCp, fasdmadir_t direction, int transfer);
		void		(*stop)  (struct Scsi_Host *host, Scsi_Pointer *SCp);
	} dma;

	/* miscellaneous */
	int			internal_done;		/* flag to indicate request done */

	unsigned long		magic_end;
} FAS216_Info;

/* Function: int fas216_init (struct Scsi_Host *instance)
 * Purpose : initialise FAS/NCR/AMD SCSI ic.
 * Params  : instance - a driver-specific filled-out structure
 * Returns : 0 on success
 */
extern int fas216_init (struct Scsi_Host *instance);

/* Function: int fas216_abort (Scsi_Cmnd *SCpnt)
 * Purpose : abort a command if something horrible happens.
 * Params  : SCpnt - Command that is believed to be causing a problem.
 * Returns : one of SCSI_ABORT_ macros.
 */
extern int fas216_abort (Scsi_Cmnd *);

/* Function: int fas216_reset (Scsi_Cmnd *SCpnt, unsigned int reset_flags)
 * Purpose : resets the adapter if something horrible happens.
 * Params  : SCpnt - Command that is believed to be causing a problem.
 *	     reset_flags - flags indicating reset type that is believed to be required.
 * Returns : one of SCSI_RESET_ macros, or'd with the SCSI_RESET_*_RESET macros.
 */
extern int fas216_reset (Scsi_Cmnd *, unsigned int);

/* Function: int fas216_queue_command (Scsi_Cmnd *SCpnt, void (*done)(Scsi_Cmnd *))
 * Purpose : queue a command for adapter to process.
 * Params  : SCpnt - Command to queue
 *	     done  - done function to call once command is complete
 * Returns : 0 - success, else error
 */
extern int fas216_queue_command (Scsi_Cmnd *, void (*done)(Scsi_Cmnd *));

/* Function: int fas216_command (Scsi_Cmnd *SCpnt)
 * Purpose : queue a command for adapter to process.
 * Params  : SCpnt - Command to queue
 * Returns : scsi result code
 */
extern int fas216_command (Scsi_Cmnd *);

/* Function: void fas216_intr (struct Scsi_Host *instance)
 * Purpose : handle interrupts from the interface to progress a command
 * Params  : instance - interface to service
 */
extern void fas216_intr (struct Scsi_Host *instance);

/* Function: int fas216_release (struct Scsi_Host *instance)
 * Purpose : release all resources and put everything to bed for FAS/NCR/AMD SCSI ic.
 * Params  : instance - a driver-specific filled-out structure
 * Returns : 0 on success
 */
extern int fas216_release (struct Scsi_Host *instance);

extern int fas216_info(FAS216_Info *info, char *buffer);
extern int fas216_print_host(FAS216_Info *info, char *buffer);
extern int fas216_print_stats(FAS216_Info *info, char *buffer);
extern int fas216_print_device(FAS216_Info *info, Scsi_Device *scd, char *buffer);

/* Function: int fas216_eh_abort(Scsi_Cmnd *SCpnt)
 * Purpose : abort this command
 * Params  : SCpnt - command to abort
 * Returns : FAILED if unable to abort
 */
extern int fas216_eh_abort(Scsi_Cmnd *SCpnt);

/* Function: int fas216_eh_device_reset(Scsi_Cmnd *SCpnt)
 * Purpose : Reset the device associated with this command
 * Params  : SCpnt - command specifing device to reset
 * Returns : FAILED if unable to reset
 */
extern int fas216_eh_device_reset(Scsi_Cmnd *SCpnt);

/* Function: int fas216_eh_bus_reset(Scsi_Cmnd *SCpnt)
 * Purpose : Reset the complete bus associated with this command
 * Params  : SCpnt - command specifing bus to reset
 * Returns : FAILED if unable to reset
 */
extern int fas216_eh_bus_reset(Scsi_Cmnd *SCpnt);

/* Function: int fas216_eh_host_reset(Scsi_Cmnd *SCpnt)
 * Purpose : Reset the host associated with this command
 * Params  : SCpnt - command specifing host to reset
 * Returns : FAILED if unable to reset
 */
extern int fas216_eh_host_reset(Scsi_Cmnd *SCpnt);

#endif /* FAS216_H */
