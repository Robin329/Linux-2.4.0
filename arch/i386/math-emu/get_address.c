/*---------------------------------------------------------------------------+
 |  get_address.c                                                            |
 |                                                                           |
 | Get the effective address from an FPU instruction.                        |
 |                                                                           |
 | Copyright (C) 1992,1993,1994,1997                                         |
 |                       W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail   billm@suburbia.net             |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------+
 | Note:                                                                     |
 |    The file contains code which accesses user memory.                     |
 |    Emulator static data may change when user memory is accessed, due to   |
 |    other processes using the emulator while swapping is in progress.      |
 +---------------------------------------------------------------------------*/


#include <linux/stddef.h>

#include <asm/uaccess.h>
#include <asm/desc.h>

#include "fpu_system.h"
#include "exception.h"
#include "fpu_emu.h"


#define FPU_WRITE_BIT 0x10

static int reg_offset[] = {
	offsetof(struct info,___eax),
	offsetof(struct info,___ecx),
	offsetof(struct info,___edx),
	offsetof(struct info,___ebx),
	offsetof(struct info,___esp),
	offsetof(struct info,___ebp),
	offsetof(struct info,___esi),
	offsetof(struct info,___edi)
};

#define REG_(x) (*(long *)(reg_offset[(x)]+(u_char *) FPU_info))

static int reg_offset_vm86[] = {
	offsetof(struct info,___cs),
	offsetof(struct info,___vm86_ds),
	offsetof(struct info,___vm86_es),
	offsetof(struct info,___vm86_fs),
	offsetof(struct info,___vm86_gs),
	offsetof(struct info,___ss),
	offsetof(struct info,___vm86_ds)
      };

#define VM86_REG_(x) (*(unsigned short *) \
		      (reg_offset_vm86[((unsigned)x)]+(u_char *) FPU_info))

/* These are dummy, fs and gs are not saved on the stack. */
#define ___FS ___ds
#define ___GS ___ds

static int reg_offset_pm[] = {
	offsetof(struct info,___cs),
	offsetof(struct info,___ds),
	offsetof(struct info,___es),
	offsetof(struct info,___FS),
	offsetof(struct info,___GS),
	offsetof(struct info,___ss),
	offsetof(struct info,___ds)
      };

#define PM_REG_(x) (*(unsigned short *) \
		      (reg_offset_pm[((unsigned)x)]+(u_char *) FPU_info))


/* Decode the SIB byte. This function assumes mod != 0 */
static int sib(int mod, unsigned long *fpu_eip)
{
  u_char ss,index,base;
  long offset;

  RE_ENTRANT_CHECK_OFF;
  FPU_code_verify_area(1);
  FPU_get_user(base, (u_char *) (*fpu_eip));   /* The SIB byte */
  RE_ENTRANT_CHECK_ON;
  (*fpu_eip)++;
  ss = base >> 6;
  index = (base >> 3) & 7;
  base &= 7;

  if ((mod == 0) && (base == 5))
    offset = 0;              /* No base register */
  else
    offset = REG_(base);

  if (index == 4)
    {
      /* No index register */
      /* A non-zero ss is illegal */
      if ( ss )
	EXCEPTION(EX_Invalid);
    }
  else
    {
      offset += (REG_(index)) << ss;
    }

  if (mod == 1)
    {
      /* 8 bit signed displacement */
      long displacement;
      RE_ENTRANT_CHECK_OFF;
      FPU_code_verify_area(1);
      FPU_get_user(displacement, (signed char *) (*fpu_eip));
      offset += displacement;
      RE_ENTRANT_CHECK_ON;
      (*fpu_eip)++;
    }
  else if (mod == 2 || base == 5) /* The second condition also has mod==0 */
    {
      /* 32 bit displacement */
      long displacement;
      RE_ENTRANT_CHECK_OFF;
      FPU_code_verify_area(4);
      FPU_get_user(displacement, (long *) (*fpu_eip));
      offset += displacement;
      RE_ENTRANT_CHECK_ON;
      (*fpu_eip) += 4;
    }

  return offset;
}


static unsigned long vm86_segment(u_char segment,
				  struct address *addr)
{
  segment--;
#ifdef PARANOID
  if ( segment > PREFIX_SS_ )
    {
      EXCEPTION(EX_INTERNAL|0x130);
      math_abort(FPU_info,SIGSEGV);
    }
#endif PARANOID
  addr->selector = VM86_REG_(segment);
  return (unsigned long)VM86_REG_(segment) << 4;
}


/* This should work for 16 and 32 bit protected mode. */
static long pm_address(u_char FPU_modrm, u_char segment,
		       struct address *addr, long offset)
{ 
  struct desc_struct descriptor;
  unsigned long base_address, limit, address, seg_top;
  unsigned short selector;

  segment--;

#ifdef PARANOID
  /* segment is unsigned, so this also detects if segment was 0: */
  if ( segment > PREFIX_SS_ )
    {
      EXCEPTION(EX_INTERNAL|0x132);
      math_abort(FPU_info,SIGSEGV);
    }
#endif PARANOID

  switch ( segment )
    {
      /* fs and gs aren't used by the kernel, so they still have their
	 user-space values. */
    case PREFIX_FS_-1:
      /* The cast is needed here to get gcc 2.8.0 to use a 16 bit register
	 in the assembler statement. */

      __asm__("mov %%fs,%0":"=r" (selector));
      addr->selector = selector;
      break;
    case PREFIX_GS_-1:
      /* The cast is needed here to get gcc 2.8.0 to use a 16 bit register
	 in the assembler statement. */
      __asm__("mov %%gs,%0":"=r" (selector));
      addr->selector = selector;
      break;
    default:
      addr->selector = PM_REG_(segment);
    }

  descriptor = LDT_DESCRIPTOR(PM_REG_(segment));
  base_address = SEG_BASE_ADDR(descriptor);
  address = base_address + offset;
  limit = base_address
	+ (SEG_LIMIT(descriptor)+1) * SEG_GRANULARITY(descriptor) - 1;
  if ( limit < base_address ) limit = 0xffffffff;

  if ( SEG_EXPAND_DOWN(descriptor) )
    {
      if ( SEG_G_BIT(descriptor) )
	seg_top = 0xffffffff;
      else
	{
	  seg_top = base_address + (1 << 20);
	  if ( seg_top < base_address ) seg_top = 0xffffffff;
	}
      access_limit =
	(address <= limit) || (address >= seg_top) ? 0 :
	  ((seg_top-address) >= 255 ? 255 : seg_top-address);
    }
  else
    {
      access_limit =
	(address > limit) || (address < base_address) ? 0 :
	  ((limit-address) >= 254 ? 255 : limit-address+1);
    }
  if ( SEG_EXECUTE_ONLY(descriptor) ||
      (!SEG_WRITE_PERM(descriptor) && (FPU_modrm & FPU_WRITE_BIT)) )
    {
      access_limit = 0;
    }
  return address;
}


/*
       MOD R/M byte:  MOD == 3 has a special use for the FPU
                      SIB byte used iff R/M = 100b

       7   6   5   4   3   2   1   0
       .....   .........   .........
        MOD    OPCODE(2)     R/M


       SIB byte

       7   6   5   4   3   2   1   0
       .....   .........   .........
        SS      INDEX        BASE

*/

void *FPU_get_address(u_char FPU_modrm, unsigned long *fpu_eip,
		  struct address *addr,
		  fpu_addr_modes addr_modes)
{
  u_char mod;
  unsigned rm = FPU_modrm & 7;
  long *cpu_reg_ptr;
  int address = 0;     /* Initialized just to stop compiler warnings. */

  /* Memory accessed via the cs selector is write protected
     in `non-segmented' 32 bit protected mode. */
  if ( !addr_modes.default_mode && (FPU_modrm & FPU_WRITE_BIT)
      && (addr_modes.override.segment == PREFIX_CS_) )
    {
      math_abort(FPU_info,SIGSEGV);
    }

  addr->selector = FPU_DS;   /* Default, for 32 bit non-segmented mode. */

  mod = (FPU_modrm >> 6) & 3;

  if (rm == 4 && mod != 3)
    {
      address = sib(mod, fpu_eip);
    }
  else
    {
      cpu_reg_ptr = & REG_(rm);
      switch (mod)
	{
	case 0:
	  if (rm == 5)
	    {
	      /* Special case: disp32 */
	      RE_ENTRANT_CHECK_OFF;
	      FPU_code_verify_area(4);
	      FPU_get_user(address, (unsigned long *) (*fpu_eip));
	      (*fpu_eip) += 4;
	      RE_ENTRANT_CHECK_ON;
	      addr->offset = address;
	      return (void *) address;
	    }
	  else
	    {
	      address = *cpu_reg_ptr;  /* Just return the contents
					  of the cpu register */
	      addr->offset = address;
	      return (void *) address;
	    }
	case 1:
	  /* 8 bit signed displacement */
	  RE_ENTRANT_CHECK_OFF;
	  FPU_code_verify_area(1);
	  FPU_get_user(address, (signed char *) (*fpu_eip));
	  RE_ENTRANT_CHECK_ON;
	  (*fpu_eip)++;
	  break;
	case 2:
	  /* 32 bit displacement */
	  RE_ENTRANT_CHECK_OFF;
	  FPU_code_verify_area(4);
	  FPU_get_user(address, (long *) (*fpu_eip));
	  (*fpu_eip) += 4;
	  RE_ENTRANT_CHECK_ON;
	  break;
	case 3:
	  /* Not legal for the FPU */
	  EXCEPTION(EX_Invalid);
	}
      address += *cpu_reg_ptr;
    }

  addr->offset = address;

  switch ( addr_modes.default_mode )
    {
    case 0:
      break;
    case VM86:
      address += vm86_segment(addr_modes.override.segment, addr);
      break;
    case PM16:
    case SEG32:
      address = pm_address(FPU_modrm, addr_modes.override.segment,
			   addr, address);
      break;
    default:
      EXCEPTION(EX_INTERNAL|0x133);
    }

  return (void *)address;
}


void *FPU_get_address_16(u_char FPU_modrm, unsigned long *fpu_eip,
		     struct address *addr,
		     fpu_addr_modes addr_modes)
{
  u_char mod;
  unsigned rm = FPU_modrm & 7;
  int address = 0;     /* Default used for mod == 0 */

  /* Memory accessed via the cs selector is write protected
     in `non-segmented' 32 bit protected mode. */
  if ( !addr_modes.default_mode && (FPU_modrm & FPU_WRITE_BIT)
      && (addr_modes.override.segment == PREFIX_CS_) )
    {
      math_abort(FPU_info,SIGSEGV);
    }

  addr->selector = FPU_DS;   /* Default, for 32 bit non-segmented mode. */

  mod = (FPU_modrm >> 6) & 3;

  switch (mod)
    {
    case 0:
      if (rm == 6)
	{
	  /* Special case: disp16 */
	  RE_ENTRANT_CHECK_OFF;
	  FPU_code_verify_area(2);
	  FPU_get_user(address, (unsigned short *) (*fpu_eip));
	  (*fpu_eip) += 2;
	  RE_ENTRANT_CHECK_ON;
	  goto add_segment;
	}
      break;
    case 1:
      /* 8 bit signed displacement */
      RE_ENTRANT_CHECK_OFF;
      FPU_code_verify_area(1);
      FPU_get_user(address, (signed char *) (*fpu_eip));
      RE_ENTRANT_CHECK_ON;
      (*fpu_eip)++;
      break;
    case 2:
      /* 16 bit displacement */
      RE_ENTRANT_CHECK_OFF;
      FPU_code_verify_area(2);
      FPU_get_user(address, (unsigned short *) (*fpu_eip));
      (*fpu_eip) += 2;
      RE_ENTRANT_CHECK_ON;
      break;
    case 3:
      /* Not legal for the FPU */
      EXCEPTION(EX_Invalid);
      break;
    }
  switch ( rm )
    {
    case 0:
      address += FPU_info->___ebx + FPU_info->___esi;
      break;
    case 1:
      address += FPU_info->___ebx + FPU_info->___edi;
      break;
    case 2:
      address += FPU_info->___ebp + FPU_info->___esi;
      if ( addr_modes.override.segment == PREFIX_DEFAULT )
	addr_modes.override.segment = PREFIX_SS_;
      break;
    case 3:
      address += FPU_info->___ebp + FPU_info->___edi;
      if ( addr_modes.override.segment == PREFIX_DEFAULT )
	addr_modes.override.segment = PREFIX_SS_;
      break;
    case 4:
      address += FPU_info->___esi;
      break;
    case 5:
      address += FPU_info->___edi;
      break;
    case 6:
      address += FPU_info->___ebp;
      if ( addr_modes.override.segment == PREFIX_DEFAULT )
	addr_modes.override.segment = PREFIX_SS_;
      break;
    case 7:
      address += FPU_info->___ebx;
      break;
    }

 add_segment:
  address &= 0xffff;

  addr->offset = address;

  switch ( addr_modes.default_mode )
    {
    case 0:
      break;
    case VM86:
      address += vm86_segment(addr_modes.override.segment, addr);
      break;
    case PM16:
    case SEG32:
      address = pm_address(FPU_modrm, addr_modes.override.segment,
			   addr, address);
      break;
    default:
      EXCEPTION(EX_INTERNAL|0x131);
    }

  return (void *)address ;
}
