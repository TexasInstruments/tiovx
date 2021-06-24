/*
* relocate.h
*
* Declare the dynamic loader's relocation API. 
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

#ifndef RELOCATE_H
#define RELOCATE_H

#include <inttypes.h>
#include "elf32.h"
#include "dload.h"
#include "dload_api.h"

/*---------------------------------------------------------------------------*/
/* Declare some globals that are used for internal debugging and profiling.  */
/*---------------------------------------------------------------------------*/
#if LOADER_DEBUG || LOADER_PROFILE
#include <time.h>
extern int DLREL_relocations;
extern time_t DLREL_total_reloc_time;
#endif


/*---------------------------------------------------------------------------*/
/* Landing point for core loader's relocation processor.                     */
/*---------------------------------------------------------------------------*/
void DLREL_relocate(DLOAD_HANDLE handle, LOADER_FILE_DESC *fd, 
                                            DLIMP_Dynamic_Module *dyn_module);

#endif
