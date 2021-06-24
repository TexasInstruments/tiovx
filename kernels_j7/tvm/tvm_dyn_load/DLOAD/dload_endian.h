/*
* dload_endian.h
*
* Specification of functions used to assist loader with endian-ness issues.
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

#ifndef DLOAD_ENDIAN_H
#define DLOAD_ENDIAN_H

#include "elf32.h"

/*---------------------------------------------------------------------------*/
/* Prototypes for ELF file object reader endianness swap routines.           */
/*---------------------------------------------------------------------------*/

int     DLIMP_get_endian(void);
void    DLIMP_change_endian64(int64_t* to_change);
void    DLIMP_change_endian32(int32_t* to_change);
void    DLIMP_change_endian16(int16_t* to_change);
void    DLIMP_change_ehdr_endian(Elf_Ehdr* to_change);
void    DLIMP_change_phdr_endian(Elf_Phdr* to_change);
void    DLIMP_change_dynent_endian(Elf_Dyn* to_change);
void    DLIMP_change_sym_endian(Elf_Sym* to_change);
void    DLIMP_change_rela_endian(Elf_Rela* to_change);
void    DLIMP_change_rel_endian(Elf_Rel* to_change);

#endif
