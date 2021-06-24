/*
* symtab.h
*
* Specification of functions used by the core loader to create, maintain,
* and destroy internal symbol tables.
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

#ifndef SYMTAB_H
#define SYMTAB_H

#include "ArrayList.h"
#include "dload.h"

/*****************************************************************************/
/* This is the top-level application file handle.  It should only be needed  */
/* under the Linux and DSBT models.                                          */ 
/*****************************************************************************/
extern int32_t DLIMP_application_handle;

/*---------------------------------------------------------------------------*/
/* Core Loader Symbol Table Management Functions                             */
/*---------------------------------------------------------------------------*/
BOOL DLSYM_canonical_lookup(DLOAD_HANDLE handle, 
                            int32_t sym_index,
                            DLIMP_Dynamic_Module *dyn_module, 
                            Elf_Addr *sym_value);

BOOL DLSYM_global_lookup(DLOAD_HANDLE handle, 
                         const char *sym_name, 
                         DLIMP_Loaded_Module *pentry, 
                         Elf_Addr *sym_value);

BOOL DLSYM_lookup_local_symtab(const char       *sym_name,
                               Elf_Sym          *symtab,
                               Elf_Word          symnum,
                               Elf_Addr         *sym_value,
                               const char       *strtab);

void DLSYM_copy_globals(DLIMP_Dynamic_Module *dyn_module);

#endif
