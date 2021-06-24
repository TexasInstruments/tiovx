/*
* c70_dynamic.c
*
* C7x-specific dynamic loader functionality
*
* Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
*
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the
* distribution.
*
* Neither the name of Texas Instruments Incorporated nor the names of
* its contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#ifdef C70_TARGET
#include "c7x_elf32.h"
#include <inttypes.h>
#include "dload.h"

/*****************************************************************************/
/* c70_process_dynamic_tag()                                                 */
/*                                                                           */
/*   Process C7x specific dynamic tags.                                      */
/*****************************************************************************/
BOOL DLDYN_c70_process_dynamic_tag(DLIMP_Dynamic_Module* dyn_module, int i)
{
   switch (dyn_module->dyntab[i].d_tag)
   {
      /*------------------------------------------------------------------*/
      /* DT_C7X_GSYM_OFFSET:                                              */
      /*  Dynamic symbol table is partitioned into local and global       */
      /*  symbols. This tag has the offset into the dynamic symbol table  */
      /*  where the global symbol table starts.                           */
      /*------------------------------------------------------------------*/
      case DT_C7X_GSYM_OFFSET:
         dyn_module->gsymtab_offset = dyn_module->dyntab[i].d_un.d_val;
#if LOADER_DEBUG
         if (debugging_on)
            DLIF_trace("Found global symbol table: %d\n", 
                       dyn_module->gsymtab_offset);
#endif
         return TRUE;

      /*------------------------------------------------------------------*/
      /* DT_C7X_GSTR_OFFSET:                                              */
      /*   Contains the offset into the dynamic string table where the    */
      /*   global symbol names start.                                     */
      /*------------------------------------------------------------------*/
      case DT_C7X_GSTR_OFFSET:
         dyn_module->gstrtab_offset = dyn_module->dyntab[i].d_un.d_val;
#if LOADER_DEBUG
         if (debugging_on)
            DLIF_trace("Found global string table: %d\n", 
                       dyn_module->gstrtab_offset);
#endif
         return TRUE;
   }

   return FALSE;
}

/*****************************************************************************/
/* DLDYN_C70_relocate_dynamic_tag_info()                                     */
/*                                                                           */
/*    Update any target specific dynamic tag values that are associated with */
/*    a section address. Return TRUE if the tag value is successfully        */
/*    updated or if the tag is not associated with a section address, and    */
/*    FALSE if we can't find the sectoin associated with the tag or if the   */
/*    tag type is not recognized.                                            */
/*                                                                           */
/*****************************************************************************/
BOOL DLDYN_c70_relocate_dynamic_tag_info(DLIMP_Dynamic_Module *dyn_module, 
                                         int32_t i)
{
   switch (dyn_module->dyntab[i].d_tag)
   {
      /*---------------------------------------------------------------------*/
      /* These tags do not point to sections.                                */
      /*---------------------------------------------------------------------*/
      case DT_C7X_GSYM_OFFSET:
      case DT_C7X_GSTR_OFFSET:
      case DT_C7X_PRELINKED:
      case DT_C7X_PREEMPTMAP:
         return TRUE;
   }

   DLIF_error(DLET_MISC, "Invalid dynamic tag encountered, %d\n", 
                         (int)dyn_module->dyntab[i].d_tag);
   return FALSE;
}

/*****************************************************************************/
/* c70_process_eiosabi()                                                     */
/*                                                                           */
/*   Process the EI_OSABI value. Verify that the OSABI is supported and set  */
/*   any variables which depend on the OSABI.                                */
/*****************************************************************************/
BOOL DLDYN_c70_process_eiosabi(DLIMP_Dynamic_Module* dyn_module)
{
    uint8_t osabi = dyn_module->fhdr.e_ident[EI_OSABI];

    if (dyn_module->relocatable)
    {
        /*-------------------------------------------------------------------*/
        /* ELFOSABI_c7000_ELFABI - C7x Baremetal ABI                         */
        /*-------------------------------------------------------------------*/
        if (osabi == ELFOSABI_C7X_ELFABI)
            return TRUE;
    }
    else
    {
        /*-------------------------------------------------------------------*/
        /* Static executables should have an OSABI of NONE.                  */
        /*-------------------------------------------------------------------*/
        if (osabi == ELFOSABI_NONE)
            return TRUE;
    }

    return FALSE;
}

#endif
