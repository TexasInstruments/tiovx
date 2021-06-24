/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
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
#include "dload.h"
#include "elf32.h"

#ifdef C60_TARGET
#include "c60_dynamic.h"
#include "c60_reloc.h"
#endif

#ifdef C70_TARGET
#include "c70_dynamic.h"
#include "c70_reloc.h"
#endif

#ifdef ARM_TARGET
#include "arm_dynamic.h"
#include "arm_reloc.h"
#endif

/*****************************************************************************/
/* Define a virtual target class to give access to target specific functions */
/*****************************************************************************/
typedef struct vtarget 
{
   int machine_id;
 
   BOOL (*relocate_dynamic_tag_info)(DLIMP_Dynamic_Module *dyn_module, int i);
   BOOL (*process_eiosabi)(DLIMP_Dynamic_Module* dyn_module);
   BOOL (*process_dynamic_tag)(DLIMP_Dynamic_Module *dyn_module, int i);
   void (*relocate)(DLOAD_HANDLE handle, LOADER_FILE_DESC *elf_file, 
                    DLIMP_Dynamic_Module *dyn_module);

} VIRTUAL_TARGET;



/*****************************************************************************/
/* Populate this for each target supported.                                  */
/*****************************************************************************/
VIRTUAL_TARGET vt_arr[] = {

#ifdef C60_TARGET
                 { 
                    EM_TI_C6000, 
                    DLDYN_c60_relocate_dynamic_tag_info, 
                    DLDYN_c60_process_eiosabi, 
                    DLDYN_c60_process_dynamic_tag, 
                    DLREL_c60_relocate
                 },
#endif
#ifdef C70_TARGET
                 {
                    EM_TI_C7X,
                    DLDYN_c70_relocate_dynamic_tag_info,
                    DLDYN_c70_process_eiosabi,
                    DLDYN_c70_process_dynamic_tag,
                    DLREL_c70_relocate
                 },
#endif
#ifdef ARM_TARGET
                 { 
                    EM_ARM, 
                    DLDYN_arm_relocate_dynamic_tag_info, 
                    DLDYN_arm_process_eiosabi, 
                    DLDYN_arm_process_dynamic_tag, 
                    DLREL_arm_relocate
                 },
#endif
                 { 
                    EM_NONE, 
                    0, 
                    0, 
                    0, 
                    0 
                 }
};


