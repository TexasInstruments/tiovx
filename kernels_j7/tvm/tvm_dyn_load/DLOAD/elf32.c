/*
* elf32.c
*
* ELF info "database" structures. 
*
* The data structures in this file come primarily from this specification:
*
*    Tool Interface Standard (TIS)
*    Executable and Linking Format (ELF) Specification
*    Version 1.2
*
*    TIS Committee
*    May 1995
*
* Additions and enhancements from this specification are also included:
*
*    System V Application Binary Interface
*    DRAFT 17
*    December 2003
*
*    http://sco.com/developers/gabi/2003-12-17/contents.html
*
* This is a C implementation of the data base objects that are commonly
* used in the source for TI development tools that support ELF.
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

#include "elf32.h"

/*---------------------------------------------------------------------------*/
/* Dynamic Tag Database                                                      */
/*---------------------------------------------------------------------------*/

const struct EDYN_TAG EDYN_TAG_DB[] =
{
   /* EDYN_TAG_NULL */
   {
      /* d_tag_name   */ "DT_NULL",
      /* d_tag_value  */ DT_NULL,
      /* d_untype     */ EDYN_UNTYPE_IGNORED,
      /* d_exec_req   */ EDYN_TAGREQ_MANDATORY,
      /* d_shared_req */ EDYN_TAGREQ_MANDATORY
   },
      
   /* EDYN_TAG_NEEDED */
   {
      /* d_tag_name   */ "DT_NEEDED",
      /* d_tag_value  */ DT_NEEDED,
      /* d_untype     */ EDYN_UNTYPE_VAL,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },
      
   /* EDYN_TAG_PLTRELSZ */
   {
      /* d_tag_name   */ "DT_PLTRELSZ",
      /* d_tag_value  */ DT_PLTRELSZ,
      /* d_untype     */ EDYN_UNTYPE_VAL,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },
      
   /* EDYN_TAG_PLTGOT */
   {
      /* d_tag_name   */ "DT_PLTGOT",
      /* d_tag_value  */ DT_PLTGOT,
      /* d_untype     */ EDYN_UNTYPE_PTR,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },
      
   /* EDYN_TAG_HASH */
   {
      /* d_tag_name   */ "DT_HASH",
      /* d_tag_value  */ DT_HASH,
      /* d_untype     */ EDYN_UNTYPE_PTR,
      /* d_exec_req   */ EDYN_TAGREQ_MANDATORY,
      /* d_shared_req */ EDYN_TAGREQ_MANDATORY
   },
      
   /* EDYN_TAG_STRTAB */
   {
      /* d_tag_name   */ "DT_STRTAB",
      /* d_tag_value  */ DT_STRTAB,
      /* d_untype     */ EDYN_UNTYPE_PTR,
      /* d_exec_req   */ EDYN_TAGREQ_MANDATORY,
      /* d_shared_req */ EDYN_TAGREQ_MANDATORY
   },
      
   /* EDYN_TAG_SYMTAB */
   {
      /* d_tag_name   */ "DT_SYMTAB",
      /* d_tag_value  */ DT_SYMTAB,
      /* d_untype     */ EDYN_UNTYPE_PTR,
      /* d_exec_req   */ EDYN_TAGREQ_MANDATORY,
      /* d_shared_req */ EDYN_TAGREQ_MANDATORY
   },
      
   /* EDYN_TAG_RELA */
   {
      /* d_tag_name   */ "DT_RELA",
      /* d_tag_value  */ DT_RELA,
      /* d_untype     */ EDYN_UNTYPE_PTR,
      /* d_exec_req   */ EDYN_TAGREQ_MANDATORY,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },
      
   /* EDYN_TAG_RELASZ */
   {
      /* d_tag_name   */ "DT_RELASZ",
      /* d_tag_value  */ DT_RELASZ,
      /* d_untype     */ EDYN_UNTYPE_VAL,
      /* d_exec_req   */ EDYN_TAGREQ_MANDATORY,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },
      
   /* EDYN_TAG_RELAENT */
   {
      /* d_tag_name   */ "DT_RELAENT",
      /* d_tag_value  */ DT_RELAENT,
      /* d_untype     */ EDYN_UNTYPE_VAL,
      /* d_exec_req   */ EDYN_TAGREQ_MANDATORY,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },
      
   /* EDYN_TAG_STRSZ */
   {
      /* d_tag_name   */ "DT_STRSZ",
      /* d_tag_value  */ DT_STRSZ,
      /* d_untype     */ EDYN_UNTYPE_VAL,
      /* d_exec_req   */ EDYN_TAGREQ_MANDATORY,
      /* d_shared_req */ EDYN_TAGREQ_MANDATORY
   },
      
   /* EDYN_TAG_SYMENT */
   {
      /* d_tag_name   */ "DT_SYMENT",
      /* d_tag_value  */ DT_SYMENT,
      /* d_untype     */ EDYN_UNTYPE_VAL,
      /* d_exec_req   */ EDYN_TAGREQ_MANDATORY,
      /* d_shared_req */ EDYN_TAGREQ_MANDATORY
   },
      
   /* EDYN_TAG_INIT */
   {
      /* d_tag_name   */ "DT_INIT",
      /* d_tag_value  */ DT_INIT,
      /* d_untype     */ EDYN_UNTYPE_PTR,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },
      
   /* EDYN_TAG_FINI */
   {
      /* d_tag_name   */ "DT_FINI",
      /* d_tag_value  */ DT_FINI,
      /* d_untype     */ EDYN_UNTYPE_PTR,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },

   /* EDYN_TAG_SONAME */
   {
      /* d_tag_name   */ "DT_SONAME",
      /* d_tag_value  */ DT_SONAME,
      /* d_untype     */ EDYN_UNTYPE_VAL,
      /* d_exec_req   */ EDYN_TAGREQ_IGNORED,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },
      
   /* EDYN_TAG_RPATH */
   {
      /* d_tag_name   */ "DT_RPATH",
      /* d_tag_value  */ DT_RPATH,
      /* d_untype     */ EDYN_UNTYPE_VAL,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_IGNORED
   },
      
   /* EDYN_TAG_SYMBOLIC */
   {
      /* d_tag_name   */ "DT_SYMBOLIC",
      /* d_tag_value  */ DT_SYMBOLIC,
      /* d_untype     */ EDYN_UNTYPE_IGNORED,
      /* d_exec_req   */ EDYN_TAGREQ_IGNORED,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },
      
   /* EDYN_TAG_REL */
   {
      /* d_tag_name   */ "DT_REL",
      /* d_tag_value  */ DT_REL,
      /* d_untype     */ EDYN_UNTYPE_PTR,
      /* d_exec_req   */ EDYN_TAGREQ_MANDATORY,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },
      
   /* EDYN_TAG_RELSZ */
   {
      /* d_tag_name   */ "DT_RELSZ",
      /* d_tag_value  */ DT_RELSZ,
      /* d_untype     */ EDYN_UNTYPE_VAL,
      /* d_exec_req   */ EDYN_TAGREQ_MANDATORY,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },
      
   /* EDYN_TAG_RELENT */
   {
      /* d_tag_name   */ "DT_RELENT",
      /* d_tag_value  */ DT_RELENT,
      /* d_untype     */ EDYN_UNTYPE_VAL,
      /* d_exec_req   */ EDYN_TAGREQ_MANDATORY,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },

   /* EDYN_TAG_PLTREL */
   {
      /* d_tag_name   */ "DT_PLTREL",
      /* d_tag_value  */ DT_PLTREL,
      /* d_untype     */ EDYN_UNTYPE_VAL,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },

   /* EDYN_TAG_DEBUG */
   {
      /* d_tag_name   */ "DT_DEBUG",
      /* d_tag_value  */ DT_DEBUG,
      /* d_untype     */ EDYN_UNTYPE_PTR,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_IGNORED
   },

   /* EDYN_TAG_TEXTREL */
   {
      /* d_tag_name   */ "DT_TEXTREL",
      /* d_tag_value  */ DT_TEXTREL,
      /* d_untype     */ EDYN_UNTYPE_IGNORED,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },

   /* EDYN_TAG_JMPREL */
   {
      /* d_tag_name   */ "DT_JMPREL",
      /* d_tag_value  */ DT_JMPREL,
      /* d_untype     */ EDYN_UNTYPE_PTR,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },

   /* EDYN_TAG_BIND_NOW */
   {
      /* d_tag_name   */ "DT_BIND_NOW",
      /* d_tag_value  */ DT_BIND_NOW,
      /* d_untype     */ EDYN_UNTYPE_IGNORED,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },

   /* EDYN_TAG_INIT_ARRAY */
   {
      /* d_tag_name   */ "DT_INIT_ARRAY",
      /* d_tag_value  */ DT_INIT_ARRAY,
      /* d_untype     */ EDYN_UNTYPE_PTR,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },

   /* EDYN_TAG_FINI_ARRAY */
   {
      /* d_tag_name   */ "DT_FINI_ARRAY",
      /* d_tag_value  */ DT_FINI_ARRAY,
      /* d_untype     */ EDYN_UNTYPE_PTR,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },

   /* EDYN_TAG_INIT_ARRAYSZ */
   {
      /* d_tag_name   */ "DT_INIT_ARRAYSZ",
      /* d_tag_value  */ DT_INIT_ARRAYSZ,
      /* d_untype     */ EDYN_UNTYPE_VAL,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },

   /* EDYN_TAG_FINI_ARRAYSZ */
   {
      /* d_tag_name   */ "DT_FINI_ARRAYSZ",
      /* d_tag_value  */ DT_FINI_ARRAYSZ,
      /* d_untype     */ EDYN_UNTYPE_VAL,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },

   /* EDYN_TAG_RUNPATH */
   {
      /* d_tag_name   */ "DT_RUNPATH",
      /* d_tag_value  */ DT_RUNPATH,
      /* d_untype     */ EDYN_UNTYPE_VAL,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },

   /* EDYN_TAG_FLAGS */
   {
      /* d_tag_name   */ "DT_FLAGS",
      /* d_tag_value  */ DT_FLAGS,
      /* d_untype     */ EDYN_UNTYPE_VAL,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_OPTIONAL
   },

   /* EDYN_TAG_ENCODING */
   {
      /* d_tag_name   */ "DT_ENCODING",
      /* d_tag_value  */ DT_ENCODING,
      /* d_untype     */ EDYN_UNTYPE_UNSPECIFIED,
      /* d_exec_req   */ EDYN_TAGREQ_UNSPECIFIED,
      /* d_shared_req */ EDYN_TAGREQ_UNSPECIFIED
   },

   /* EDYN_TAG_PREINIT_ARRAY */
   {
      /* d_tag_name   */ "DT_PREINIT_ARRAY",
      /* d_tag_value  */ DT_PREINIT_ARRAY,
      /* d_untype     */ EDYN_UNTYPE_PTR,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_IGNORED
   },

   /* EDYN_TAG_PREINIT_ARRAYSZ */
   {
      /* d_tag_name   */ "DT_PREINIT_ARRAYSZ",
      /* d_tag_value  */ DT_PREINIT_ARRAYSZ,
      /* d_untype     */ EDYN_UNTYPE_VAL,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_IGNORED
   },

   /* Terminate array with an id of -1 */
   {
      /* d_tag_name   */ "",
      /* d_tag_value  */ -1,
      /* d_untype     */ EDYN_UNTYPE_UNSPECIFIED,
      /* d_exec_req   */ EDYN_TAGREQ_OPTIONAL,
      /* d_shared_req */ EDYN_TAGREQ_IGNORED
   }
};

/*---------------------------------------------------------------------------*/
/* Special Section Database                                                  */
/*---------------------------------------------------------------------------*/
const struct ESCN ESCN_DB[] =
{
   /* .bss */
   {
      /* name       */ ESCN_BSS_name,
      /* sh_type    */ SHT_NOBITS,
      /* sh_entsize */ 0,
      /* sh_flags   */ SHF_ALLOC + SHF_WRITE
   },

   /* .comment */
   {
      /* name       */ ESCN_COMMENT_name,
      /* sh_type    */ SHT_PROGBITS,
      /* sh_entsize */ 0,
      /* sh_flags   */ 0,
      /* unit_size_m*/ 8
   },

   /* .data */
   {
      /* name       */ ESCN_DATA_name,
      /* sh_type    */ SHT_PROGBITS,
      /* sh_entsize */ 0,
      /* sh_flags   */ SHF_ALLOC + SHF_WRITE
   },

   /* .data1 */
   {
      /* name       */ ESCN_DATA1_name,
      /* sh_type    */ SHT_PROGBITS,
      /* sh_entsize */ 0,
      /* sh_flags   */ SHF_ALLOC + SHF_WRITE
   },

   /* .debug */
   {
      /* name       */ ESCN_DEBUG_name,
      /* sh_type    */ SHT_PROGBITS,
      /* sh_entsize */ 0,
      /* sh_flags   */ 0,
      /* unit_size_m*/ 8
   },

   /* .dynamic */
   {
      /* name       */ ESCN_DYNAMIC_name,
      /* sh_type    */ SHT_DYNAMIC,
      /* sh_entsize */ sizeof(Elf_Dyn),
      /* sh_flags   */ SHF_ALLOC,
      /* unit_size_m*/ 8
   },

   /* .dynstr */
   {
      /* name       */ ESCN_DYNSTR_name,
      /* sh_type    */ SHT_STRTAB,
      /* sh_entsize */ sizeof(char),
      /* sh_flags   */ SHF_ALLOC + SHF_STRINGS,
      /* unit_size_m*/ 8
   },

   /* .dynsym */
   {
      /* name       */ ESCN_DYNSYM_name,
      /* sh_type    */ SHT_DYNSYM,
      /* sh_entsize */ sizeof(Elf_Sym),
      /* sh_flags   */ SHF_ALLOC,
      /* unit_size_m*/ 8
   },

   /* .fini */
   {
      /* name       */ ESCN_FINI_name,
      /* sh_type    */ SHT_PROGBITS,
      /* sh_entsize */ 0,
      /* sh_flags   */ SHF_ALLOC + SHF_EXECINSTR
   },

   /* .fini_array */
   {
      /* name       */ ESCN_FINI_ARRAY_name,
      /* sh_type    */ SHT_FINI_ARRAY,
      /* sh_entsize */ 0,
      /* sh_flags   */ SHF_ALLOC + SHF_WRITE
   },

   /* .got */
   {
      /* name       */ ESCN_GOT_name,
      /* sh_type    */ SHT_PROGBITS,
      /* sh_entsize */ 4,
      /* sh_flags   */ SHF_ALLOC + SHF_WRITE
   },

   /* .hash */
   {
      /* name       */ ESCN_HASH_name,
      /* sh_type    */ SHT_HASH,
      /* sh_entsize */ 0,
      /* sh_flags   */ SHF_ALLOC,
      /* unit_size_m*/ 8
   },

   /* .init */
   {
      /* name       */ ESCN_INIT_name,
      /* sh_type    */ SHT_PROGBITS,
      /* sh_entsize */ 0,
      /* sh_flags   */ SHF_ALLOC + SHF_EXECINSTR
   },

   /* .init_array */
   {
      /* name       */ ESCN_INIT_ARRAY_name,
      /* sh_type    */ SHT_INIT_ARRAY,
      /* sh_entsize */ 0,
      /* sh_flags   */ SHF_ALLOC + SHF_WRITE
   },

   /* .interp */
   {
      /* name       */ ESCN_INTERP_name,
      /* sh_type    */ SHT_PROGBITS,
      /* sh_entsize */ 0,
      /* sh_flags   */ 0,
      /* unit_size_m*/ 8
   },

   /* .line */
   {
      /* name       */ ESCN_LINE_name,
      /* sh_type    */ SHT_PROGBITS,
      /* sh_entsize */ 0,
      /* sh_flags   */ 0,
      /* unit_size_m*/ 8
   },

   /* .note */
   {
      /* name       */ ESCN_NOTE_name,
      /* sh_type    */ SHT_NOTE,
      /* sh_entsize */ 0,
      /* sh_flags   */ 0,
      /* unit_size_m*/ 8
   },

   /* .plt */
   {
      /* name       */ ESCN_PLT_name,
      /* sh_type    */ SHT_PROGBITS,
      /* sh_entsize */ 0,
      /* sh_flags   */ 0,
      /* unit_size_m*/ 8
   },

   /* .preinit_array */
   {
      /* name       */ ESCN_PREINIT_ARRAY_name,
      /* sh_type    */ SHT_PREINIT_ARRAY,
      /* sh_entsize */ 0,
      /* sh_flags   */ SHF_ALLOC + SHF_WRITE
   },

   /* .rel */
   {
      /* name       */ ESCN_REL_name,
      /* sh_type    */ SHT_REL,
      /* sh_entsize */ sizeof(Elf_Rel),
      /* sh_flags   */ 0,
      /* unit_size_m*/ 8
   },

   /* .rela */
   {
      /* name       */ ESCN_RELA_name,
      /* sh_type    */ SHT_RELA,
      /* sh_entsize */ sizeof(Elf_Rela),
      /* sh_flags   */ 0,
      /* unit_size_m*/ 8
   },

   /* .rodata */
   {
      /* name       */ ESCN_RODATA_name,
      /* sh_type    */ SHT_PROGBITS,
      /* sh_entsize */ 0,
      /* sh_flags   */ SHF_ALLOC
   },

   /* .rodata1 */
   {
      /* name       */ ESCN_RODATA1_name,
      /* sh_type    */ SHT_PROGBITS,
      /* sh_entsize */ 0,
      /* sh_flags   */ SHF_ALLOC
   },

   /* .shstrtab */
   {
      /* name       */ ESCN_SHSTRTAB_name,
      /* sh_type    */ SHT_STRTAB,
      /* sh_entsize */ sizeof(char),
      /* sh_flags   */ SHF_STRINGS,
      /* unit_size_m*/ 8
   },

   /* .strtab */
   {
      /* name       */ ESCN_STRTAB_name,
      /* sh_type    */ SHT_STRTAB,
      /* sh_entsize */ sizeof(char),
      /* sh_flags   */ SHF_STRINGS,
      /* unit_size_m*/ 8
   },

   /* .symtab */
   {
      /* name       */ ESCN_SYMTAB_name,
      /* sh_type    */ SHT_SYMTAB,
      /* sh_entsize */ sizeof(Elf_Sym),
      /* sh_flags   */ 0,
      /* unit_size_m*/ 8
   },

   /* .symtab_shndx */
   {
      /* name       */ ESCN_SYMTAB_SHNDX_name,
      /* sh_type    */ SHT_SYMTAB_SHNDX,
      /* sh_entsize */ sizeof(Elf_Word),
      /* sh_flags   */ 0,
      /* unit_size_m*/ 8
   },

   /* .tbss */
   {
      /* name       */ ESCN_TBSS_name,
      /* sh_type    */ SHT_NOBITS,
      /* sh_entsize */ 0,
      /* sh_flags   */ SHF_ALLOC + SHF_WRITE + SHF_TLS
   },

   /* .tdata */
   {
      /* name       */ ESCN_TDATA_name,
      /* sh_type    */ SHT_PROGBITS,
      /* sh_entsize */ 0,
      /* sh_flags   */ SHF_ALLOC + SHF_WRITE + SHF_TLS
   },

   /* .tdata1 */
   {
      /* name       */ ESCN_TDATA1_name,
      /* sh_type    */ SHT_PROGBITS,
      /* sh_entsize */ 0,
      /* sh_flags   */ SHF_ALLOC + SHF_WRITE + SHF_TLS
   },

   /* .text */
   {
      /* name       */ ESCN_TEXT_name,
      /* sh_type    */ SHT_PROGBITS,
      /* sh_entsize */ 0,
      /* sh_flags   */ SHF_ALLOC + SHF_EXECINSTR
   },

   /* .build.attributes */
   {
      /* name       */ ESCN_ATTRIBUTES_name,
      /* sh_type    */ SHT_ATTRIBUTES,
      /* sh_entsize */ 0,
      /* sh_flags   */ 0,
      /* unit_size_m*/ 8
   },

   /* .TI.section.flags */
   {
      /* name       */ ESCN_TI_SH_FLAGS_name,
      /* sh_type    */ SHT_TI_SH_FLAGS,
      /* sh_entsize */ 0,
      /* sh_flags   */ 0,
      /* unit_size_m*/ 8
   },

   /* .TI.symalias*/
   {
      /* name       */ ESCN_TI_SYMALIAS_name,
      /* sh_type    */ SHT_TI_SYMALIAS,
      /* sh_entsize */ 0,
      /* sh_flags   */ 0,
      /* unit_size_m*/ 8
   },

   /* .TI.section.page */
   {
      /* name       */ ESCN_TI_SH_PAGE_name,
      /* sh_type    */ SHT_TI_SH_PAGE,
      /* sh_entsize */ 0,
      /* sh_flags   */ 0,
      /* unit_size_m*/ 8
   },

   /* Terminate array with a NULL name field */
   {
      /* name       */ (const char*)0,
      /* sh_type    */ 0,
      /* sh_entsize */ 0,
      /* sh_flags   */ 0,
      /* unit_size_m*/ 0
   }
};

