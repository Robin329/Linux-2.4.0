/*
** -----------------------------------------------------------------------------
**
**  Perle Specialix driver for Linux
**  Ported from existing RIO Driver for SCO sources.
 *
 *  (C) 1990 - 2000 Specialix International Ltd., Byfleet, Surrey, UK.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**	Module		: func.h
**	SID		: 1.3
**	Last Modified	: 11/6/98 11:34:10
**	Retrieved	: 11/6/98 11:34:21
**
**  ident @(#)func.h	1.3
**
** -----------------------------------------------------------------------------
*/

#ifndef __func_h_def
#define __func_h_def

#ifdef SCCS_LABELS
#ifndef lint
static char *_func_h_sccs_ = "@(#)func.h	1.3";
#endif
#endif

/* rioboot.c */
int RIOBootCodeRTA(struct rio_info *, struct DownLoad *);
int RIOBootCodeHOST(struct rio_info *, register struct DownLoad *);
int RIOBootCodeUNKNOWN(struct rio_info *, struct DownLoad *);
void msec_timeout(struct Host *);
int RIOBootRup(struct rio_info *, uint, struct Host *, struct PKT *);
int RIOBootComplete(struct rio_info *, struct Host *, uint, struct PktCmd *);
int RIOBootOk(struct rio_info *,struct Host *, ulong);
int RIORtaBound(struct rio_info *, uint); 
void FillSlot(int, int, uint, struct Host *);

/* riocmd.c */
int RIOFoadRta(struct Host *, struct Map *);
int RIOZombieRta(struct Host *, struct Map *);
int RIOCommandRta(struct rio_info *, uint, int (* func)( struct Host *, 
								struct Map *));
int RIOIdentifyRta(struct rio_info *, caddr_t); 
int RIOKillNeighbour(struct rio_info *, caddr_t);
int RIOSuspendBootRta(struct Host *, int, int);
int RIOFoadWakeup(struct rio_info *);
int RIOCommandRup(struct rio_info *, uint, struct Host *, struct PKT *);
struct CmdBlk * RIOGetCmdBlk(void);
void RIOFreeCmdBlk(struct CmdBlk *);
int RIOQueueCmdBlk(struct Host *, uint, struct CmdBlk *);
void RIOPollHostCommands(struct rio_info *, struct Host *);
int RIOStrlen(register char *);
int RIOStrCmp(register char *, register char *);
int RIOStrnCmp(register char *, register char *, int);
void  RIOStrNCpy(char *, char *, int);
int RIOWFlushMark(int, struct CmdBlk *);
int RIORFlushEnable(int, struct CmdBlk *);
int RIOUnUse(int, struct CmdBlk *);
void ShowPacket(uint, struct PKT *);

/* rioctrl.c */
int copyin(int, caddr_t, int);
int copyout(caddr_t, int, int);
int riocontrol(struct rio_info *, dev_t,int,caddr_t,int); 
int RIOPreemptiveCmd(struct rio_info *,struct Port *,uchar);

/* rioinit.c */
void rioinit(struct rio_info *, struct RioHostInfo *);
void RIOInitHosts(struct rio_info *, struct RioHostInfo *);
void RIOISAinit(struct rio_info *,int);
int RIODoAT(struct rio_info *, int, int);
caddr_t RIOCheckForATCard(int);
int RIOAssignAT(struct rio_info *, int, caddr_t, int);
int RIOBoardTest(paddr_t, caddr_t, uchar, int);
int RIOScrub(int, BYTE *, int);
void RIOAllocateInterrupts(struct rio_info *);
void RIOStopInterrupts(struct rio_info *, int, int);
void RIOAllocDataStructs(struct rio_info *);
void RIOSetupDataStructs(struct rio_info *);
int RIODefaultName(struct rio_info *, struct Host *, uint);
int RIOReport(struct rio_info *);
struct rioVersion * RIOVersid(void);
int RIOMapin(paddr_t, int, caddr_t *);
void RIOMapout(paddr_t, long, caddr_t);
void RIOHostReset(uint, volatile struct DpRam *, uint);

/* riointr.c */
void riopoll(struct rio_info *);
void riointr(struct rio_info *);
void RIOTxEnable(char *);
void RIOServiceHost(struct rio_info *, struct Host *, int);
void RIOReceive(struct rio_info *, struct Port *);
int riotproc(struct rio_info *, register struct ttystatics *, int, int);

/* rioparam.c */
int RIOParam(struct Port *, int, int, int);
int RIODelay(struct Port *PortP, int);
int RIODelay_ni(struct Port *PortP, int);
void ms_timeout(struct Port *);
int can_add_transmit(struct PKT **, struct Port *);
void add_transmit(struct Port *);
void put_free_end(struct Host *, struct PKT *);
int can_remove_receive(struct PKT **, struct Port *);
void remove_receive(struct Port *);

/* rioroute.c */
int RIORouteRup(struct rio_info *, uint, struct Host *, struct PKT *);
void RIOFixPhbs(struct rio_info *, struct Host *, uint); 
int RIOCheckIsolated(struct rio_info *, struct Host *, uint);
int RIOIsolate(struct rio_info *, struct Host *, uint);
int RIOCheck(struct Host *, uint);
uint GetUnitType(uint);
int RIOSetChange(struct rio_info *);
void RIOConCon(struct rio_info *, struct Host *, uint, uint, uint, uint, int);
int RIOFindFreeID(struct rio_info *, struct Host *, uint *, uint *);
int RIOFreeDisconnected(struct rio_info *, struct Host *, int );
int RIORemoveFromSavedTable(struct rio_info *, struct Map *);


/* riotty.c */

int riotopen(struct tty_struct * tty, struct file * filp);
int riotclose(void  *ptr);
int RIOCookMode(struct ttystatics *);
int riotioctl(struct rio_info *, dev_t, register int, register caddr_t); 
void ttyseth(struct Port *, struct ttystatics *, struct old_sgttyb *sg);

/* riotable.c */
int RIONewTable(struct rio_info *);
int RIOApel(struct rio_info *);
int RIODeleteRta(struct rio_info *, struct Map *);
int RIOAssignRta(struct rio_info *, struct Map *);
int RIOReMapPorts(struct rio_info *, struct Host *, struct Map *);
int RIOChangeName(struct rio_info *, struct Map*);

#if 0
/* riodrvr.c */
struct rio_info * rio_install(struct RioHostInfo *);
int rio_uninstall(register struct rio_info *);
int rio_open(struct rio_info *, int, struct file *);
int rio_close(struct rio_info *, struct file *);
int rio_read(struct rio_info *, struct file *, char *, int);
int rio_write(struct rio_info *, struct file *	f, char *, int);
int rio_ioctl(struct rio_info *, struct file *, int, char *);
int rio_select(struct rio_info *, struct file *	f, int, struct sel *);
int	rio_intr(char *);
int rio_isr_thread(char  *);
struct rio_info * rio_info_store( int cmd, struct rio_info * p);
#endif

extern int    rio_pcicopy(char *src, char *dst, int n);
extern int rio_minor (kdev_t device);
extern int rio_ismodem (kdev_t device);
extern void rio_udelay (int usecs);

extern void rio_start_card_running (struct Host * HostP);

#endif	/* __func_h_def */
