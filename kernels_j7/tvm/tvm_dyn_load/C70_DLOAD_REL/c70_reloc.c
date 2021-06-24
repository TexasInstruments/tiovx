/*
* c70_reloc.c
*
* Process C6x-specific dynamic relocations for core dynamic loader.
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

#include <limits.h>
#include <stdlib.h>
#include "relocate.h"
#include "symtab.h"
#include "c7x_elf32.h"
#include "dload_api.h"
#include "util.h"
#include "dload_endian.h"
#include "c70_reloc.h"

#if LOADER_DEBUG || LOADER_PROFILE
extern int DLREL_relocations;
extern time_t DLREL_total_reloc_time;
#endif

/*****************************************************************************/
/* Keys for specifying overflow checks                                       */
/*****************************************************************************/
typedef enum 
{
  OV_SIGNED,    // value must be representable as n-bit signed value
  OV_UNSIGNED,  // value must be representable as n-bit unsigned value
  OV_XSIGNED    // value must fit in n bits; signedness unspecified
} OVCHECK_KIND; 

static void write_field(uint8_t*, int, int, uint64_t, int, int);
static void check_overflow(C7X_RELOC_TYPE, Elf_Addr, 
                           uint64_t, int, OVCHECK_KIND kind); 
static void swap_data(uint8_t*, int); 

/*****************************************************************************/
/* RELOC_DO() - Process a single relocation entry.                           */
/*****************************************************************************/
static void reloc_do(C7X_RELOC_TYPE r_type,
                     Elf_Addr segment_vaddr, 
                     uint32_t spc, 
                     uint8_t *segment_buffer, 
                     int endianness, 
                     Elf_Addr symval,
                     Elf_Addend addend)
{
   uint64_t reloc_value = 0;
   Elf_Addr reloc_vaddr = segment_vaddr + spc;
   uint8_t *data = segment_buffer + spc;
   int64_t opnd_p = reloc_vaddr & ~(uint64_t)3;

#if LOADER_DEBUG || LOADER_PROFILE
   /*------------------------------------------------------------------------*/
   /* In debug mode, keep a count of the number of relocations processed.    */
   /* In profile mode, start the clock on a given relocation.                */
   /*------------------------------------------------------------------------*/
   int start_time = 0;
   if (debugging_on || profiling_on)
   {
      DLREL_relocations++;
      if (profiling_on) start_time = clock();
   }
#endif

   /*------------------------------------------------------------------------*/
   /* Calculate the relocation value.                                        */
   /* C7000 Notes:                                                           */
   /*  1. C70 encodes code as BE-8; that is, always LSB-first regardless     */
   /*     of target endianness. For relocations that apply to instructions,  */
   /*     the endianness is always passed as "LSB" to reflect this.          */
   /*  2. Many wider fields are relocated with multiple relocations. We only */
   /*     apply the overflow check to the most-significant field.            */
   /*  3. Overflow checks are performed before scaling; therefore the field  */
   /*     size used for the overflow check is the size of the "virtual"      */
   /*     field prior to scaling or splitting.                               */
   /*  4. The OVCHECK_KIND indicates whether the field should be considered  */
   /*     signed, unsigned, or don't-care for the purpose of overflow        */
   /*     checking.                                                          */
   /* See the C7000 ELF ABI Specification for more details.                  */
   /*------------------------------------------------------------------------*/
   switch(r_type)
   {
      // Placeholder reloc: no operation
      case R_C7X_NONE:
         break;

      // Absolute (symbol-relative) relocations applied to data (e.g. .word)
      case R_C7X_ABS16:
         reloc_value = symval + addend;
	 check_overflow(r_type, reloc_vaddr, reloc_value, 16, OV_XSIGNED);
	 write_field(data, 16, endianness, reloc_value, 16, 0);
	 break;

      case R_C7X_ABS32:
         reloc_value = symval + addend;
	 check_overflow(r_type, reloc_vaddr, reloc_value, 32, OV_XSIGNED);
	 write_field(data, 32, endianness, reloc_value, 32, 0);
	 break;

      case R_C7X_ABS64:
         reloc_value = symval + addend;
	 check_overflow(r_type, reloc_vaddr, reloc_value, 64, OV_XSIGNED);
	 write_field(data, 64, endianness, reloc_value, 64, 0);
	 break;

      // PC-relative relocation applied to data (used in EH tables)
      case R_C7X_PREL30:
         reloc_value = symval + addend - opnd_p;
	 check_overflow(r_type, reloc_vaddr, reloc_value, 32, OV_SIGNED);
	 write_field(data, 32, endianness, (reloc_value>>2), 30, 0);
	 break;

      // 32-bit symbol-relative, applied to code (MVK), split into 2 fields
      case R_C7X_MVK32_LO5:
         reloc_value = symval + addend;
	 write_field(data, 32, ELFDATA2LSB, reloc_value, 5, 14); 
	 break;
      case R_C7X_MVK32_HI27:
	 reloc_value = symval + addend;
	 check_overflow(r_type, reloc_vaddr, reloc_value, 32, OV_XSIGNED);
	 write_field(data, 32, ELFDATA2LSB, (reloc_value>>5), 27, 5);
	 break;

      // 49-bit or 64-bit symbol-relative, applied to code (MVK49/MVK64),
      // split into 3 fields. 
      case R_C7X_MVK_LO10:
         reloc_value = symval + addend;
	 write_field(data, 32, ELFDATA2LSB, reloc_value, 10, 14);
	 break;
      case R_C7X_MVK64_MID27:
         reloc_value = symval + addend;
	 write_field(data, 32, ELFDATA2LSB, (reloc_value>>10), 27, 5);
	 break;
      case R_C7X_MVK49_HI12:
         reloc_value = symval + addend;
	 check_overflow(r_type, reloc_vaddr, reloc_value, 49, OV_XSIGNED);
	 write_field(data, 32, ELFDATA2LSB, (reloc_value>>37), 27, 5);
	 break;
      case R_C7X_MVK64_HI27:
         reloc_value = symval + addend;
	 check_overflow(r_type, reloc_vaddr, reloc_value, 64, OV_XSIGNED);
	 write_field(data, 32, ELFDATA2LSB, (reloc_value>>37), 27, 5);
	 break;

      // 32-bit PC-relative, applied to code (ADDKPC etc), split into 2 fields
      case R_C7X_PCR_OFFSET_LO5:
      case R_C7X_PCR_OFFSET_ADDKPC_LO5:
         reloc_value = symval + addend - opnd_p;
	 write_field(data, 32, ELFDATA2LSB, reloc_value, 5, 20);
	 break;
      case R_C7X_PCR_OFFSET_HI27:
      case R_C7X_PCR_OFFSET_ADDKPC_HI27:
         reloc_value = symval + addend - opnd_p;
	 check_overflow(r_type, reloc_vaddr, reloc_value, 32, OV_SIGNED);
	 write_field(data, 32, ELFDATA2LSB, (reloc_value>>5), 27, 5);
	 break;

      // 21-bit "short" PC-relative branch
      case R_C7X_PCR_BRANCH_LO19:
         reloc_value = symval + addend - opnd_p;
	 check_overflow(r_type, reloc_vaddr, reloc_value, 21, OV_SIGNED);
	 write_field(data, 32, ELFDATA2LSB, (reloc_value>>2), 19, 8);
         break;

      // 26-bit "medium" PC-relative branch/call
      case R_C7X_PCR_BRANCH_LO24:
         reloc_value = symval + addend - opnd_p;
	 check_overflow(r_type, reloc_vaddr, reloc_value, 26, OV_SIGNED);
	 write_field(data, 32, ELFDATA2LSB, (reloc_value>>2), 24, 8);
         break;

      // 48-bit "long" PC-relative branch/call, split into 2 fields
      case R_C7X_PCR_EBRANCH_LO19:
         reloc_value = symval + addend - opnd_p;
	 write_field(data, 32, ELFDATA2LSB, (reloc_value>>2), 19, 8);
         break;
      case R_C7X_PCR_EBRANCH_HI27:
         reloc_value = symval + addend - opnd_p;
	 check_overflow(r_type, reloc_vaddr, reloc_value, 48, OV_SIGNED);
	 write_field(data, 32, ELFDATA2LSB, ((reloc_value>>2)>>19), 27, 5);
         break;

      // This does not appear to be used
      case R_C7X_PCR16:
      default: 
         DLIF_error(DLET_RELOC, "invalid relocation type: %d\n", r_type);
         break;
   }

#if LOADER_DEBUG || LOADER_PROFILE
   /*------------------------------------------------------------------------*/
   /* In profile mode, add elapsed time for this relocation to total time    */
   /* spent doing relocations.                                               */
   /*------------------------------------------------------------------------*/
   if (profiling_on)
      DLREL_total_reloc_time += (clock() - start_time);
   if (debugging_on)
   {
      DLIF_trace("reloc_do: r_type=%d addr=%x sym=%x addend=%x result=%x\n", 
                 r_type, 
		 (int32_t)reloc_vaddr, (int32_t)symval, (int32_t)addend,
                 (int32_t)reloc_value); 
   } 
#endif
}

/*****************************************************************************/
/* CHECK_OVERFLOW()                                                          */
/*                                                                           */
/*    Check relocation value against the range associated with a given       */
/*    relocation type field size and signedness.                             */
/*                                                                           */
/*    The relocation value passed in is prior to any scaling done for        */
/*    encoding. The field size reflects that. For example if there is        */
/*    a value that can range from 0-31 (5 bits) which is scaled by 2 and     */
/*    encoded in a 3 bit field, the field size here would be 5.              */
/*                                                                           */
/*****************************************************************************/
static void check_overflow(
   C7X_RELOC_TYPE r_type,    // only used to report error  
   Elf_Addr reloc_vaddr,     // ditto
   uint64_t reloc_value,     // result value, before scaling
   int field_size,           // size of "virtual" encoding field
   OVCHECK_KIND kind)        // signed/uns/either
{
   int ok = TRUE; 
   int64_t sbits;
   uint64_t zbits; 

   // We need bits above the field to check overflow
   if (field_size == 64) return;

   sbits = ((int64_t)reloc_value >> (field_size-1)); 
   zbits = ((uint64_t)reloc_value >> field_size); 

   if (kind == OV_SIGNED)
      ok = (sbits == 0) || (sbits == -1);
   else if (kind == OV_UNSIGNED)
      ok = (zbits == 0);
   else if (kind == OV_XSIGNED)
      ok = (zbits == 0) || (sbits == -1); 

   if (!ok)
      DLIF_error(DLET_RELOC, "relocation overflow at vaddr %x, "
                             "rtype=%d reloc_value=%llx\n",
	         (uint32_t)reloc_vaddr, r_type, (uint64_t)reloc_value);
}

/*****************************************************************************/
/* WRITE_FIELD() -                                                           */
/*    Given a relocated value and a location, patch the value back into the  */
/*    object code.                                                           */
/*****************************************************************************/
static void write_field(
   uint8_t* object_data,     // pointer to container in target shadow memory
   int container_size,       // size in bits of container
   int endianness,           // target's data format
   uint64_t reloc_value,     // new value of field
   int field_size,           // size of field in bits
   int field_offset)         // offset of field from lsb of container
{
   uint8_t *p;
   uint64_t container_val;
   uint64_t mask;
   int i;

   // The approach we use here is to read and write the field as little-endian,
   // independent of the host endianness.  Put the object data into 
   // little-endian format if it's not already.
   if (endianness == ELFDATA2MSB)
      swap_data(object_data, container_size); 

   // Read current object data from container, as little-endian
   p = object_data;
   container_val = 0;
   for (i = 0; i < container_size; i += 8)
      container_val = ((uint64_t)*p++ << i) | container_val;

   // Patch relocation value into the field within the container
   mask = ((((uint64_t)1 << field_size) - 1) << field_offset); 
   container_val &= (container_val & ~mask);
   container_val |= ((reloc_value << field_offset) & mask);

   // Write patched container value back to object data, as little-endian
   p = object_data;
   for (i = 0; i < container_size; i += 8)
      *p++ = (uint8_t)(container_val >> i); 

   // Restore the object data to its original format
   if (endianness == ELFDATA2MSB)
      swap_data(object_data, container_size); 
}


/******************************************************************************/
/* SWAP_DATA()                                                                */
/*   Byte-swap a specified "container" of target memory, so that relocation   */
/*   can be applied in an endian-agnostic way.                                */
/******************************************************************************/
static void swap_data(uint8_t* object_data, int container_size)
{
   switch(container_size)
   {
      case 8 : break;
      case 16: DLIMP_change_endian16((int16_t*)object_data);  break;
      case 32: DLIMP_change_endian32((int32_t*)object_data);  break;
      case 64: DLIMP_change_endian64((int64_t*)object_data);  break;
      default:
	DLIF_error(DLET_RELOC, 
		   "unsupported container size: %d\n", container_size); 
   }
}


/*****************************************************************************/
/* The rest of this file deals with reading the relocation tables from the   */
/* object file and decoding them. Very little of it is C7x-specific.         */
/*****************************************************************************/

/*****************************************************************************/
/* READ_REL_TABLE()                                                          */
/*                                                                           */
/*    Read in an Elf_Rel type relocation table.  This function allocates     */
/*    host memory for the table.                                             */
/*                                                                           */
/*****************************************************************************/
static void read_rel_table(Elf_Rel **rel_table,
                           int32_t table_offset,
			   uint32_t relnum, uint32_t relent,
			   LOADER_FILE_DESC *fd, BOOL wrong_endian)
{
   if (relnum == 0) { *rel_table = NULL; return; }

   *rel_table = (Elf_Rel *)DLIF_malloc(relnum * relent);
   DLIF_fseek(fd, table_offset, LOADER_SEEK_SET);
   DLIF_fread(*rel_table, relnum, relent, fd);

   if (wrong_endian)
   {
      int i;
      for (i = 0; i < relnum; i++)
         DLIMP_change_rel_endian(*rel_table + i);
   }
}

/*****************************************************************************/
/* PROCESS_REL_TABLE()                                                       */
/*                                                                           */
/*    Process table of Elf_Rel type relocations.                           */
/*                                                                           */
/*****************************************************************************/
static void process_rel_table(DLOAD_HANDLE handle,
                              DLIMP_Loaded_Segment* seg,
                              Elf_Rel *rel_table,
			      uint32_t relnum, 
			      int32_t *start_relidx,
			      DLIMP_Dynamic_Module* dyn_module)
{
   DLIF_error(DLET_RELOC,
	      "C7000 does not support REL-type dynamic relocations\n");
   DLIF_exit(EXIT_FAILURE);
}

/*****************************************************************************/
/* READ_RELA_TABLE()                                                         */
/*                                                                           */
/*    Read in an Elf_Rela type relocation table.  This function allocates  */
/*    host memory for the table.                                             */
/*                                                                           */
/*****************************************************************************/
static void read_rela_table(Elf_Rela **rela_table,
                            int32_t table_offset,
			    uint32_t relanum, uint32_t relaent,
			    LOADER_FILE_DESC *fd, BOOL wrong_endian)
{
    if (relanum == 0) { *rela_table = NULL; return; }
   *rela_table = (Elf_Rela *)DLIF_malloc(relanum * relaent);
   DLIF_fseek(fd, table_offset, LOADER_SEEK_SET);
   DLIF_fread(*rela_table, relanum, relaent, fd);

   if (wrong_endian)
   {
      int i;
      for (i = 0; i < relanum; i++)
         DLIMP_change_rela_endian(*rela_table + i);
   }
}

/*****************************************************************************/
/* PROCESS_RELA_TABLE()                                                      */
/*                                                                           */
/*    Process a table of Elf_Rela type relocations.                        */
/*                                                                           */
/*****************************************************************************/
static void process_rela_table(DLOAD_HANDLE handle,
                               DLIMP_Loaded_Segment *seg,
                               Elf_Rela *rela_table,
			       uint32_t relanum, 
			       int32_t *start_relidx,
			       DLIMP_Dynamic_Module *dyn_module)
{
    Elf_Addr seg_start_addr = seg->input_vaddr;
    Elf_Addr seg_end_addr   = seg_start_addr + seg->phdr.p_memsz;
    BOOL       found        = FALSE;
    int32_t    relidx       = *start_relidx;
    int        endianness   = dyn_module->fhdr.e_ident[EI_DATA];

    /*-----------------------------------------------------------------------*/
    /* If the given start reloc index is out of range, then start from       */
    /* the beginning of the given table.                                     */
    /*-----------------------------------------------------------------------*/
    if (relidx > relanum) relidx = 0;

    /*-----------------------------------------------------------------------*/
    /* Spin through RELA relocation table.                                   */
    /*-----------------------------------------------------------------------*/
    for ( ; relidx < relanum; relidx++)
    {
        /*-------------------------------------------------------------------*/
        /* If the relocation offset falls within the segment, process it.    */
        /*-------------------------------------------------------------------*/
        if (rela_table[relidx].r_offset >= seg_start_addr &&
            rela_table[relidx].r_offset < seg_end_addr)
        {
	    Elf_Addr r_symval;
	    C7X_RELOC_TYPE r_type  = 
	              (C7X_RELOC_TYPE)ELF64_R_TYPE(rela_table[relidx].r_info);
	    int32_t r_symid = ELF64_R_SYM(rela_table[relidx].r_info);

	    found = TRUE;

            /*---------------------------------------------------------------*/
	    /* If symbol definition is not found, don't do the relocation.   */
	    /* An error is generated by the lookup function.                 */
            /*---------------------------------------------------------------*/
            if (!DLSYM_canonical_lookup(handle, r_symid, dyn_module, &r_symval))
                continue;

            /*---------------------------------------------------------------*/
            /* Perform actual relocation.                                    */
            /*---------------------------------------------------------------*/
            reloc_do(r_type,
                     seg->phdr.p_vaddr, 
                     rela_table[relidx].r_offset - seg->input_vaddr,
                     seg->host_address, 
		     endianness,
                     r_symval,
                     rela_table[relidx].r_addend);
        }
	
        else if (found)
            break;
    }
}

/*****************************************************************************/
/* PROCESS_GOT_RELOCS()                                                      */
/*                                                                           */
/*    Process all GOT relocations.  It is possible to have both Elf_Rel      */
/*    and Elf_Rela type relocations in the same file, so we handle tham      */
/*    both.                                                                  */
/*                                                                           */
/*    Note: "GOT relocation table" is a bit of a misnomer; these are any     */ 
/*    and all dynamic relocations that are not specifically PLTGOT.          */
/*                                                                           */
/*****************************************************************************/
static void process_got_relocs(DLOAD_HANDLE handle,
                               Elf_Rel* rel_table, uint32_t relnum,
                               Elf_Rela* rela_table, uint32_t relanum,
			       DLIMP_Dynamic_Module* dyn_module)
{
   DLIMP_Loaded_Segment *seg = 
       (DLIMP_Loaded_Segment*)(dyn_module->loaded_module->loaded_segments.buf);
   uint32_t num_segs = dyn_module->loaded_module->loaded_segments.size;
   int32_t  rel_relidx = 0;
   int32_t  rela_relidx = 0;
   uint32_t seg_idx = 0;

   /*------------------------------------------------------------------------*/
   /* Process relocations segment by segment.                                */
   /*------------------------------------------------------------------------*/
   for (seg_idx = 0; seg_idx < num_segs; seg_idx++)
   {
      /*---------------------------------------------------------------------*/
      /* Relocations should not occur in uninitialized segments.             */
      /*---------------------------------------------------------------------*/
      if (!seg[seg_idx].phdr.p_filesz) continue;

      if (rela_table)
         process_rela_table(handle, (seg + seg_idx), 
	                    rela_table, relanum, &rela_relidx, dyn_module);

      if (rel_table)
         process_rel_table(handle, (seg + seg_idx), 
	                    rel_table, relnum, &rel_relidx, dyn_module);
   }
}

/*****************************************************************************/
/* PROCESS_PLTGOT_RELOCS()                                                   */
/*                                                                           */
/*    Process all PLTGOT relocation entries.  The PLTGOT relocation table    */
/*    can be either Elf_Rel or Elf_Rela type.  All PLTGOT relocations    */
/*    ar guaranteed to belong to the same segment.                           */
/*                                                                           */
/*****************************************************************************/
static void process_pltgot_relocs(DLOAD_HANDLE handle,
                                  void* plt_reloc_table, 
                                  int reltype,
                                  uint32_t pltnum,
				  DLIMP_Dynamic_Module* dyn_module)
{
   Elf_Addr r_offset = (reltype == DT_REL) ?
                             ((Elf_Rel *)plt_reloc_table)->r_offset :
			     ((Elf_Rela *)plt_reloc_table)->r_offset;

   DLIMP_Loaded_Segment* seg =
      (DLIMP_Loaded_Segment*)(dyn_module->loaded_module->loaded_segments.buf);

   uint32_t num_segs = dyn_module->loaded_module->loaded_segments.size;
   int32_t  plt_relidx = 0;
   uint32_t seg_idx = 0;

   /*------------------------------------------------------------------------*/
   /* For each segment s, check if the relocation falls within s. If so,     */
   /* then all other relocations are guaranteed to fall with s. Process      */
   /* all relocations and then return.                                       */
   /*------------------------------------------------------------------------*/
   for (seg_idx = 0; seg_idx < num_segs; seg_idx++)
   {
      Elf_Addr seg_start_addr = seg[seg_idx].input_vaddr;
      Elf_Addr seg_end_addr   = seg_start_addr + seg[seg_idx].phdr.p_memsz; 
      
      /*---------------------------------------------------------------------*/
      /* Relocations should not occur in uninitialized segments.             */
      /*---------------------------------------------------------------------*/
      if(!seg[seg_idx].phdr.p_filesz) continue; 
      
      if (r_offset >= seg_start_addr &&
          r_offset < seg_end_addr)
      {
         if (reltype == DT_REL)
	    process_rel_table(handle, (seg + seg_idx),
	                      (Elf_Rel *)plt_reloc_table, 
			      pltnum, &plt_relidx, 
			      dyn_module);
	 else
	    process_rela_table(handle, (seg + seg_idx),
	                       (Elf_Rela *)plt_reloc_table,
			       pltnum, &plt_relidx, 
			       dyn_module); 

         break;
      }
   }
}

/*****************************************************************************/
/* RELOCATE() - Perform RELA and REL type relocations for given ELF object   */
/*      file that we are in the process of loading and relocating.           */
/*                                                                           */
/*  C70-specific notes                                                       */
/*    1. This is largely based on the C60 implementation.                    */
/*    2. Currently all dynamic relocs are RELA. We leave the REL code        */
/*       here as a placeholder.                                              */
/*    3. Currently there are no PLTGOT (PLT-specific PIC) relocations.       */
/*    4. Addresses can be 64 bits, but we assume sizes and file offsets      */
/*       fit in 32 bits even though their types in the object file are 64.   */
/*****************************************************************************/
void DLREL_c70_relocate(DLOAD_HANDLE handle,
                        LOADER_FILE_DESC *fd, DLIMP_Dynamic_Module *dyn_module)
{
   Elf_Dyn  *dyn_nugget     = dyn_module->dyntab;
   Elf_Rela *rela_table     = NULL;
   Elf_Rel  *rel_table      = NULL;
   Elf_Rela *rela_plt_table = NULL;
   Elf_Rel  *rel_plt_table  = NULL;

   /*------------------------------------------------------------------------*/
   /* Read the size of the relocation table (DT_RELASZ) and the size per     */
   /* relocation (DT_RELAENT) from the dynamic segment.                      */
   /*------------------------------------------------------------------------*/
   uint32_t relasz  = DLIMP_get_first_dyntag(DT_RELASZ, dyn_nugget);
   uint32_t relaent = DLIMP_get_first_dyntag(DT_RELAENT, dyn_nugget);
   uint32_t relanum = 0;

   /*------------------------------------------------------------------------*/
   /* Read the size of the relocation table (DT_RELSZ) and the size per      */
   /* relocation (DT_RELENT) from the dynamic segment.                       */
   /*------------------------------------------------------------------------*/
   uint32_t relsz  = DLIMP_get_first_dyntag(DT_RELSZ, dyn_nugget);
   uint32_t relent = DLIMP_get_first_dyntag(DT_RELENT, dyn_nugget);
   uint32_t relnum = 0;

   /*------------------------------------------------------------------------*/
   /* Read the size of the relocation table (DT_PLTRELSZ) and the type of    */
   /* of the PLTGOT relocation table (DT_PLTREL): one of DT_REL or DT_RELA   */
   /*------------------------------------------------------------------------*/
   uint32_t pltrelsz  = DLIMP_get_first_dyntag(DT_PLTRELSZ, dyn_nugget);
   int      pltreltyp = DLIMP_get_first_dyntag(DT_PLTREL, dyn_nugget);
   uint32_t pltnum    = 0;

   /*------------------------------------------------------------------------*/
   /* Read the PLTGOT relocation table from the file                         */
   /* The PLTGOT table is a subsection at the end of either the DT_REL or    */
   /* DT_RELA table.  The size of the table it belongs to DT_REL(A)SZ        */
   /* includes the size of the PLTGOT table.  So it must be adjusted so that */
   /* the GOT relocation tables only contain actual GOT relocations.         */
   /*------------------------------------------------------------------------*/
   if (pltrelsz != INT_MAX && pltrelsz != 0)
   {
      if (pltreltyp == DT_REL)
      {
         pltnum = pltrelsz/relent;
	 relsz -= pltrelsz;
	 read_rel_table((&rel_plt_table),
	                DLIMP_get_first_dyntag(DT_JMPREL, dyn_nugget),
			pltnum, relent, fd, dyn_module->wrong_endian);
      } 
      
      else if (pltreltyp == DT_RELA) 
      {
         pltnum = pltrelsz/relaent;
	 relasz -= pltrelsz;
	 read_rela_table((&rela_plt_table),
	                 DLIMP_get_first_dyntag(DT_JMPREL, dyn_nugget),
			 pltnum, relaent, fd, dyn_module->wrong_endian);
      } 
      
      else 
      {
         DLIF_error(DLET_RELOC,
	            "DT_PLTREL is invalid: must be either %d or %d\n",
		    DT_REL, DT_RELA);
      }
   }

   /*------------------------------------------------------------------------*/
   /* Read the DT_RELA GOT relocation table from the file.                   */
   /* Note: "GOT relocation table" is a bit of a misnomer; these are any     */
   /* and all dynamic relocations that are not specifically PLTGOT.          */
   /*------------------------------------------------------------------------*/
   if (relasz != INT_MAX && relasz != 0)
   {
      relanum = relasz/relaent;
      read_rela_table(&rela_table, DLIMP_get_first_dyntag(DT_RELA, dyn_nugget),
                      relanum, relaent, fd, dyn_module->wrong_endian);
   }

   /*------------------------------------------------------------------------*/
   /* Read the DT_REL GOT relocation table from the file                     */
   /*------------------------------------------------------------------------*/ 
   if (relsz != INT_MAX && relsz != 0)
   {
      relnum = relsz/relent;
      read_rel_table(&rel_table, DLIMP_get_first_dyntag(DT_REL, dyn_nugget),
                     relnum, relent, fd, dyn_module->wrong_endian);
   }

   /*------------------------------------------------------------------------*/
   /* Process the PLTGOT relocations                                         */
   /*------------------------------------------------------------------------*/
   if (rela_plt_table) 
      process_pltgot_relocs(handle, rela_plt_table, pltreltyp, pltnum, 
                            dyn_module); 

   if (rel_plt_table) 
      process_pltgot_relocs(handle, rel_plt_table, pltreltyp, pltnum, 
                            dyn_module); 
   
   /*------------------------------------------------------------------------*/
   /* Process the GOT relocations                                            */
   /* Note: see "misnomer" above.                                            */
   /*------------------------------------------------------------------------*/
   if (rel_table || rela_table)
      process_got_relocs(handle, rel_table, relnum, rela_table, relanum, 
                         dyn_module); 
   
   /*-------------------------------------------------------------------------*/
   /* Free memory used for ELF relocation table copies.                       */
   /*-------------------------------------------------------------------------*/
   if (rela_table)     DLIF_free(rela_table);
   if (rel_table)      DLIF_free(rel_table);
   if (rela_plt_table) DLIF_free(rela_plt_table);
   if (rel_plt_table)  DLIF_free(rel_plt_table);
}


/*****************************************************************************/
/* UNIT TESTING INTERFACE                                                    */
/*****************************************************************************/
#ifdef UNIT_TEST
void unit_c70_reloc_do(C7X_RELOC_TYPE r_type,
                       uint8_t *address_space,
                       Elf_Addr addend, Elf_Addr symval, uint32_t pc,
                       int endianness)
{
    reloc_do(r_type, (Elf_Addr)address_space, pc, 
             address_space, endianness, symval, addend); 
}

#if 0 /* RELA TYPE RELOCATIONS HAVE ADDEND IN RELOCATION ENTRY */
void unit_c70_rel_unpack_addend(C60_RELOC_TYPE r_type,
                                uint8_t* address,
				uint32_t* addend)
{
    rel_unpack_addend(r_type, address, addend);
}
#endif

BOOL unit_c70_rel_overflow(C7X_RELOC_TYPE r_type, int32_t reloc_value)
{
   return rel_overflow(r_type, reloc_value);
}
#endif

