/******************************************************************************
 * Copyright (c) 2021, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

/*****************************************************************************/
/* C7X_ELF32.H - C7X PROCESSOR EXTENSIONS FOR 32-BIT ELF OBJECT FILES.       */
/*                                                                           */
/* The extensions in this file come from these specifications:               */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*    C7X ELF                                                                */
/*---------------------------------------------------------------------------*/
/*    ELF for the C7X Architecture                                           */
/*    Codegen Tools, SDO                                                     */
/*                                                                           */
/*****************************************************************************/
#ifndef C7X_ELF32_H
#define C7X_ELF32_H

#include "elf32.h"

/*****************************************************************************/
/* ELF Header                                                                */
/*****************************************************************************/

/* EI_OSABI */
enum
{
   ELFOSABI_C7X_ELFABI = 64,  /* C7X Baremetal OSABI                */
   ELFOSABI_C7X_LINUX  = 65   /* C7X Linux OSABI                    */
};

/*---------------------------------------------------------------------------*/
/* File Header Flags (value of "e_flags")                                    */
/*---------------------------------------------------------------------------*/
enum
{
   EF_C7X_REL = 0x01        /* Contains static relocations. A ET_EXEC or  */
                              /* ET_DYN file w/ this flag set can be        */
                              /* treated as ET_REL during static linking.   */
};

/*****************************************************************************/
/* Program Header                                                            */
/* PP. 2-2                                                                   */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/* Segment Types (value of "p_type")                                         */
/*---------------------------------------------------------------------------*/
enum
{
   PT_C7X_PHATTR    = 0x70000000      /* Extended Program Header Attributes*/
};

/*****************************************************************************/
/* Sections                                                                  */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/* Section Types (value of "sh_type")                                        */
/*---------------------------------------------------------------------------*/
enum
{
   /*------------------------------------------------------------------------*/
   /* Section types defined by the C7X ELFABI.                               */
   /* Note: ABI defined section type should be named SHT_C7X_xxx             */
   /*------------------------------------------------------------------------*/
   SHT_C7X_UNWIND     = 0x70000001,   /* Exception Index Table             */
   SHT_C7X_PREEMPTMAP = 0x70000002,   /* Pre-emption Map                   */
   SHT_C7X_ATTRIBUTES = 0x70000003,   /* Obj File Compatability Attributes */

   /*------------------------------------------------------------------------*/
   /* The following section types are not part of C7x ABI. As per the ABI,   */
   /* the processor specific values not defined in the ABI are reserved for  */
   /* future use. Here we reserve the range 0x7F000000 through 0x7FFFFFFFF   */
   /* for the TI specific processor section types.                           */
   /* Note: TI specific section type should be named SHT_TI_xxx              */
   /*------------------------------------------------------------------------*/
#if 0
   /*------------------------------------------------------------------------*/
   /* These types were first defined in c6000_elf32.h and cannot be          */
   /* redefined. Thus they remain here as comments to mark that C7X uses     */
   /* them.                                                                  */
   /*------------------------------------------------------------------------*/
   SHT_TI_ICODE         = 0x7F000000,   /* ICODE representation              */
   SHT_TI_HANDLER       = 0x7F000002,   /* Handler function table            */
   SHT_TI_INITINFO      = 0x7F000003,   /* Info for C auto-init of variables */
   SHT_TI_PHATTRS       = 0x7F000004    /* Extended Program Header Attributes*/
#endif
};

/*---------------------------------------------------------------------------*/
/* Section Indexes (value of "st_shndx" in symbol entry)                     */
/* Currently empty for C7X.                                                  */
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
/* C7X-Specific Dynamic Array Tags (C7X ELF ABI Section ??? - AEGUPD)        */
/* NOTE:                                                                     */
/*     As per GABI a tag whose value is even number indicates a dynamic tag  */
/*     that uses d_ptr. Odd number indicates the use of d_val or neither     */
/*     d_val nor d_ptr.                                                      */
/*****************************************************************************/
/* Name                   Value           d_un        Executable  Shared Obj.*/
/* ----                   -----           ----        ----------  -----------*/
enum
{
   /*------------------------------------------------------------------------*/
   /* OSABI specific tags:                                                   */
   /*          From 0x6000000D thru 0x6FFFF000                               */
   /*------------------------------------------------------------------------*/
   DT_C7X_GSYM_OFFSET   = 0x6000000D,  /* d_val    -- OSABI Specific    -- */
   DT_C7X_GSTR_OFFSET   = 0x6000000F,  /* d_val    -- OSABI Specific    -- */
   DT_C7X_PRELINKED     = 0x60000011,  /* d_val    -- OSABI Specific    -- */

   /*------------------------------------------------------------------------*/
   /* Processor specific tags:                                               */
   /*          From 0x70000000 thru 0x7FFFFFFF                               */
   /*------------------------------------------------------------------------*/
   DT_C7X_PREEMPTMAP    = 0x70000002,  /* d_ptr    -- Platform Specific -- */

};

/*---------------------------------------------------------------------------*/
/* C7x Dynamic Relocation Types                                              */
/*---------------------------------------------------------------------------*/
typedef enum 
{ 
   R_C7X_NONE = 0,
   R_C7X_PCR16 = 4,
   R_C7X_ABS16 = 16,
   R_C7X_ABS32 = 17,
   R_C7X_ABS64 = 18,
   R_C7X_MVK32_LO5 = 19,
   R_C7X_MVK32_HI27 = 20,
   R_C7X_MVK_LO10 = 21,
   R_C7X_MVK64_MID27 = 22,
   R_C7X_MVK49_HI12 = 23,
   R_C7X_MVK64_HI27 = 24,
   R_C7X_PCR_OFFSET_LO5 = 25,
   R_C7X_PCR_OFFSET_HI27 = 26,
   R_C7X_PCR_BRANCH_LO19 = 27,
   R_C7X_PCR_BRANCH_LO24 = 28,
   R_C7X_PCR_EBRANCH_LO19 = 29,
   R_C7X_PCR_EBRANCH_HI27 = 30,
   R_C7X_PREL30 = 31,
   R_C7X_PCR_OFFSET_ADDKPC_LO5 = 32,
   R_C7X_PCR_OFFSET_ADDKPC_HI27 = 33,
} C7X_RELOC_TYPE;

#endif /* C7X_ELF32_H */
