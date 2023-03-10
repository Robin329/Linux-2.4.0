/*
    NetWinder Floating Point Emulator
    (c) Rebel.com, 1998-1999

    Direct questions, comments to Scott Bambrough <scottb@netwinder.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <asm/system.h>

#include "fpa11.h"
#include "fpopcode.h"

#include "fpmodule.h"
#include "fpmodule.inl"

/* forward declarations */
unsigned int EmulateCPDO(const unsigned int);
unsigned int EmulateCPDT(const unsigned int);
unsigned int EmulateCPRT(const unsigned int);

/* Emulator registers */
FPA11 *fpa11;

/* Reset the FPA11 chip.  Called to initialize and reset the emulator. */
void resetFPA11(void)
{
  int i;

  /* initialize the register type array */
  for (i=0;i<=7;i++)
  {
    fpa11->fType[i] = typeNone;
  }
  
  /* FPSR: set system id to FP_EMULATOR, clear all other bits */
  fpa11->fpsr = FP_EMULATOR;
  
  /* FPCR: set SB, AB and DA bits, clear all others */
#if MAINTAIN_FPCR
  fpa11->fpcr = MASK_RESET;
#endif
}

void SetRoundingMode(const unsigned int opcode)
{
#if MAINTAIN_FPCR
   fpa11->fpcr &= ~MASK_ROUNDING_MODE;
#endif   
   switch (opcode & MASK_ROUNDING_MODE)
   {
      default:
      case ROUND_TO_NEAREST:
         float_rounding_mode = float_round_nearest_even;
#if MAINTAIN_FPCR         
         fpa11->fpcr |= ROUND_TO_NEAREST;
#endif         
      break;
      
      case ROUND_TO_PLUS_INFINITY:
         float_rounding_mode = float_round_up;
#if MAINTAIN_FPCR         
         fpa11->fpcr |= ROUND_TO_PLUS_INFINITY;
#endif         
      break;
      
      case ROUND_TO_MINUS_INFINITY:
         float_rounding_mode = float_round_down;
#if MAINTAIN_FPCR         
         fpa11->fpcr |= ROUND_TO_MINUS_INFINITY;
#endif         
      break;
      
      case ROUND_TO_ZERO:
         float_rounding_mode = float_round_to_zero;
#if MAINTAIN_FPCR         
         fpa11->fpcr |= ROUND_TO_ZERO;
#endif         
      break;
  }
}

void SetRoundingPrecision(const unsigned int opcode)
{
#if MAINTAIN_FPCR
   fpa11->fpcr &= ~MASK_ROUNDING_PRECISION;
#endif   
   switch (opcode & MASK_ROUNDING_PRECISION)
   {
      case ROUND_SINGLE:
         floatx80_rounding_precision = 32;
#if MAINTAIN_FPCR         
         fpa11->fpcr |= ROUND_SINGLE;
#endif         
      break;
      
      case ROUND_DOUBLE:
         floatx80_rounding_precision = 64;
#if MAINTAIN_FPCR         
         fpa11->fpcr |= ROUND_DOUBLE;
#endif         
      break;
      
      case ROUND_EXTENDED:
         floatx80_rounding_precision = 80;
#if MAINTAIN_FPCR         
         fpa11->fpcr |= ROUND_EXTENDED;
#endif         
      break;
      
      default: floatx80_rounding_precision = 80;
  }
}

/* Emulate the instruction in the opcode. */
unsigned int EmulateAll(unsigned int opcode)
{
  unsigned int nRc = 0;
  unsigned long flags;

  save_flags(flags); sti();

  if (fpa11->initflag == 0)		/* good place for __builtin_expect */
  {
    resetFPA11();
    SetRoundingMode(ROUND_TO_NEAREST);
    SetRoundingPrecision(ROUND_EXTENDED);
    fpa11->initflag = 1;
  }

  if (TEST_OPCODE(opcode,MASK_CPRT))
  {
    /* Emulate conversion opcodes. */
    /* Emulate register transfer opcodes. */
    /* Emulate comparison opcodes. */
    nRc = EmulateCPRT(opcode);
  }
  else if (TEST_OPCODE(opcode,MASK_CPDO))
  {
    /* Emulate monadic arithmetic opcodes. */
    /* Emulate dyadic arithmetic opcodes. */
    nRc = EmulateCPDO(opcode);
  }
  else if (TEST_OPCODE(opcode,MASK_CPDT))
  {
    /* Emulate load/store opcodes. */
    /* Emulate load/store multiple opcodes. */
    nRc = EmulateCPDT(opcode);
  }
  else
  {
    /* Invalid instruction detected.  Return FALSE. */
    nRc = 0;
  }

  restore_flags(flags);

  return(nRc);
}

#if 0
unsigned int EmulateAll1(unsigned int opcode)
{
  switch ((opcode >> 24) & 0xf)
  {
     case 0xc:
     case 0xd:
       if ((opcode >> 20) & 0x1)
       {
          switch ((opcode >> 8) & 0xf)
          {
             case 0x1: return PerformLDF(opcode); break;
             case 0x2: return PerformLFM(opcode); break;
             default: return 0;
          }
       }
       else
       {
          switch ((opcode >> 8) & 0xf)
          {
             case 0x1: return PerformSTF(opcode); break;
             case 0x2: return PerformSFM(opcode); break;
             default: return 0;
          }
      }
     break;
     
     case 0xe: 
       if (opcode & 0x10)
         return EmulateCPDO(opcode);
       else
         return EmulateCPRT(opcode);
     break;
  
     default: return 0;
  }
}
#endif

