/*
* symtab.c
*
* Symbol table creation, maintenance, and management.  This module also
* contains implementations of local and global symbol table lookup
* algorithms, as appropriate for the platform that we are running on
* (assumed to be DSP Bridge or Linux model, indicated by 
* direct_dependent_only flag in a given Module).
*
* Copyright (C) 2009-2014 Texas Instruments Incorporated - http://www.ti.com/
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
#include "ArrayList.h"

/*---------------------------------------------------------------------------*/
/* Set up a Queue of Int32 type data objects.                                */
/*---------------------------------------------------------------------------*/
#include "Queue.h"
TYPE_QUEUE_DEFINITION(int32_t, Int32)
TYPE_QUEUE_IMPLEMENTATION(int32_t, Int32)

#include "symtab.h"
#include "dload_api.h"
#include <string.h>

/*---------------------------------------------------------------------------*/
/* Holds the handle of the ET_EXEC-type mmodule loaded, if any.              */
/*---------------------------------------------------------------------------*/
int32_t DLIMP_application_handle = 0;

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
BOOL DLSYM_lookup_global_symtab(const char *sym_name, Elf_Sym *symtab,
                                Elf_Word symnum, Elf_Addr *sym_value,
                                const char *strtab);

/*****************************************************************************/
/* DLSYM_COPY_GLOBALS() - Copy global symbols from the dynamic module's      */
/*      symbol table to the loader's global symbol table.                    */
/*****************************************************************************/
void DLSYM_copy_globals(DLIMP_Dynamic_Module *dyn_module) 
{
   Elf_Word i, global_index, global_symnum;
   DLIMP_Loaded_Module *module = dyn_module->loaded_module;

#if LOADER_DEBUG
   if (debugging_on)
      DLIF_trace("DLSYM_copy_globals:\n");
#endif

   /*------------------------------------------------------------------------*/
   /* The dynamic symbol table is sorted so that the local symbols come      */
   /* before the global symbols. gsymtab_offset points to the address where  */
   /* the first global symbol starts. Only the global symbols need to be     */
   /* copied into the persistent info.                                       */
   /*------------------------------------------------------------------------*/
   global_index  = dyn_module->gsymtab_offset / sizeof(Elf_Sym);
   global_symnum = dyn_module->symnum - global_index;

   /*------------------------------------------------------------------------*/
   /* Create space for the new global symbol table.                          */
   /*------------------------------------------------------------------------*/
   if (module->gsymtab)
   {
       DLIF_free(module->gsymtab);
       module->gsymtab = NULL; 
   }

   if (global_symnum > 0)
   {
      module->gsymtab = DLIF_malloc(sizeof(Elf_Sym) * global_symnum);

      memcpy(module->gsymtab, 
             &dyn_module->symtab[global_index],
             sizeof(Elf_Sym) * global_symnum);
   }
   module->gsymnum = global_symnum;

   /*------------------------------------------------------------------------*/
   /* Copy the string table part that contains the global symbol names.      */
   /*------------------------------------------------------------------------*/
   if (module->gstrtab)
   {
       DLIF_free(module->gstrtab);
       module->gstrtab = NULL; 
   }

   module->gstrsz  = dyn_module->strsz - dyn_module->gstrtab_offset;
   if (module->gstrsz)
   {
      module->gstrtab = DLIF_malloc(module->gstrsz);

      memcpy(module->gstrtab, 
             dyn_module->strtab + dyn_module->gstrtab_offset, 
             module->gstrsz);
   }

   /*------------------------------------------------------------------------*/
   /* Update the symbol names of the global symbol entries to point to       */
   /* the symbol names in the string table.                                  */
   /*------------------------------------------------------------------------*/
   for (i = 0; i < global_symnum; i++)
   {

      Elf_Word old_offset = dyn_module->symtab[i + global_index].st_name;
      Elf_Word new_offset = old_offset - dyn_module->gstrtab_offset;
      module->gsymtab[i].st_name = new_offset;

#if LOADER_DEBUG
      if (debugging_on) 
      { 
	 char *symname = module->gstrtab + new_offset;
         DLIF_trace("Copying symbol: %s\n", symname);
      }
#endif
   }
}

/*****************************************************************************/
/* BREADTH_FIRST_LOOKUP() - Perform a breadth-first search of the Module     */
/*     dependency graph to find specified symbol name (sym_name).            */
/*****************************************************************************/
static BOOL breadth_first_lookup(DLOAD_HANDLE phandle, 
                                 const char* sym_name, 
                                 int handle, 
                                 Elf_Addr *sym_value)
{
   /*------------------------------------------------------------------------*/
   /* We start this function by putting the specified file handle on the     */
   /* file_handle_queue.                                                     */
   /*------------------------------------------------------------------------*/
   LOADER_OBJECT *dHandle = (LOADER_OBJECT *)phandle;
   Int32_Queue file_handle_queue = TYPE_QUEUE_INITIALIZER;
   Int32_enqueue(&file_handle_queue, handle);

   /*------------------------------------------------------------------------*/
   /* While the queue is not empty, keep looking for the symbol.             */
   /*------------------------------------------------------------------------*/
   while(file_handle_queue.size)
   {
      int i;

      /*---------------------------------------------------------------------*/
      /* Set up a pointer to front of the list of loaded files so that we    */
      /* can be sure that dependent files will be searched in load order.    */
      /*---------------------------------------------------------------------*/
      loaded_module_ptr_Queue_Node* mod_node = 
                                    dHandle->DLIMP_loaded_objects.front_ptr;
      int* dependencies = (int*)(mod_node->value->dependencies.buf); 
      
      /*---------------------------------------------------------------------*/
      /* Pluck off the file handle at the front of the file_handle_queue.    */
      /* We will search this file next.                                      */
      /*---------------------------------------------------------------------*/
      handle = Int32_dequeue(&file_handle_queue); 
      
      /*---------------------------------------------------------------------*/
      /* Locate the Module associated with the current file handle.          */
      /*---------------------------------------------------------------------*/
      while (mod_node->value->file_handle != handle) mod_node++;
      
      /*---------------------------------------------------------------------*/
      /* Search the symbol table of the current file handle's Module.        */
      /* If the symbol was found, then we're finished.                       */
      /*---------------------------------------------------------------------*/
      if (DLSYM_lookup_global_symtab(sym_name, 
                                     mod_node->value->gsymtab, 
                                     mod_node->value->gsymnum, 
                                     sym_value,
                                     mod_node->value->gstrtab))
         return TRUE;

      /*---------------------------------------------------------------------*/
      /* If our symbol was not in the current Module, then add this Module's */
      /* dependents to the end of the file_handle_queue.                     */
      /*---------------------------------------------------------------------*/
      for (i = 0; i < mod_node->value->dependencies.size; i++)
         Int32_enqueue(&file_handle_queue, dependencies[i]);
   }

   /*------------------------------------------------------------------------*/
   /* We didn't find our symbol; return FALSE.                               */
   /*------------------------------------------------------------------------*/
   return FALSE;
}

/*****************************************************************************/
/* DLSYM_global_lookup() - Search the global symbol table to find the        */
/*                         definition of the given symbol name.              */
/*****************************************************************************/
BOOL DLSYM_global_lookup(DLOAD_HANDLE handle, 
                         const char    *sym_name,
                         DLIMP_Loaded_Module *loaded_module, 
                         Elf_Addr    *sym_value)
{
   int i = 0;
   loaded_module_ptr_Queue_Node* node;
   LOADER_OBJECT *dHandle = (LOADER_OBJECT *)handle;

#if LOADER_DEBUG
   if (debugging_on)
      DLIF_trace("DLSYM_global_lookup: %s\n", sym_name);
#endif

   /*------------------------------------------------------------------------*/
   /* We will choose a different lookup algorithm based on what kind of      */
   /* platform we are supporting.  In the Braveheart case, the global symbol */
   /* lookup algorithm searches the base image first, followed by the        */
   /* explicit children of the specified Module.                             */
   /*------------------------------------------------------------------------*/
   if (loaded_module->direct_dependent_only)
   {
      int* child_handle = (int*)(loaded_module->dependencies.buf); 

      /*---------------------------------------------------------------------*/
      /* Spin through list of this Module's dependencies (anything on its    */
      /* DT_NEEDED list), searching through each dependent's symbol table    */
      /* to find the symbol we are after.                                    */
      /*---------------------------------------------------------------------*/
      for (i = 0; i < loaded_module->dependencies.size; i++)
      {
         for (node = dHandle->DLIMP_loaded_objects.front_ptr;
           node->value->file_handle != child_handle[i];
           node=node->next_ptr);

         /*------------------------------------------------------------------*/
         /* Return true if we find the symbol.                               */
         /*------------------------------------------------------------------*/
         if (DLSYM_lookup_global_symtab(sym_name, 
                                        node->value->gsymtab, 
                                        node->value->gsymnum,
                                        sym_value,
                                        node->value->gstrtab))
            return TRUE;
      }
      /*---------------------------------------------------------------------*/
      /* Lastly, search the current module. This catches pre-emptable        */
      /* definitions in the current file that were not defined elsewhere.    */
      /*---------------------------------------------------------------------*/
      if (DLSYM_lookup_global_symtab(sym_name, 
                                     loaded_module->gsymtab, 
                                     loaded_module->gsymnum,
                                     sym_value,
                                     loaded_module->gstrtab))
         return TRUE;
   }

   /*------------------------------------------------------------------------*/
   /* In the LINUX model, we will use a breadth-first global symbol lookup   */
   /* algorithm.  First, the application's global symbol table is searched,  */
   /* followed by its children, followed by their children, and so on.       */
   /* It is up to the client of this module to set the application handle.   */
   /*------------------------------------------------------------------------*/
   else
   {
      if (breadth_first_lookup(handle, sym_name, DLIMP_application_handle, 
                               sym_value))
         return TRUE;
   }

   /*------------------------------------------------------------------------*/
   /* If we got this far, then symbol was not found.                         */
   /*------------------------------------------------------------------------*/
   DLIF_error(DLET_SYMBOL, "Could not resolve symbol %s!\n", sym_name);

   return FALSE;
}

/*****************************************************************************/
/* DLSYM_lookup_symtab() - Lookup the symbol name in the given symbol table. */
/*                         Symbol must have specified binding. Return the    */
/*                         value in sym_value and return TRUE if the lookup  */
/*                         succeeds.                                         */
/*****************************************************************************/
static BOOL DLSYM_lookup_symtab(const char *sym_name, Elf_Sym *symtab,
                                Elf_Word symnum, Elf_Addr *sym_value,
                                BOOL require_local_binding,
                                const char *strtab)
{
   Elf_Addr sym_idx;

#if LOADER_DEBUG
      if (debugging_on)
         DLIF_trace("DLSYM_lookup_symtab, sym to find : %s\n", sym_name);
#endif

   for (sym_idx = 0; sym_idx < symnum; sym_idx++)
   {
      const char *candidate = strtab + symtab[sym_idx].st_name;
#if LOADER_DEBUG
      if (debugging_on)
         DLIF_trace("\tPotential symbol match : %s\n", candidate);
#endif

      if ((symtab[sym_idx].st_shndx != SHN_UNDEF) && ((require_local_binding && 
          (ELF32_ST_BIND(symtab[sym_idx].st_info) == STB_LOCAL)) ||
  	  (!require_local_binding && 
	  (ELF32_ST_BIND(symtab[sym_idx].st_info) != STB_LOCAL))) &&
          !strcmp(sym_name, candidate))
      {
         if (sym_value) *sym_value = symtab[sym_idx].st_value;
         return TRUE;
      }
   }
   if (sym_value) *sym_value = 0;
   return FALSE;
}

/*****************************************************************************/
/* DLSYM_lookup_global_symtab() - Lookup the symbol name in the given symbol */
/*                               table. Symbol must have global binding.     */
/*                               Return the value in sym_value and return    */
/*                               TRUE if the lookup succeeds.                */
/*****************************************************************************/
BOOL DLSYM_lookup_global_symtab(const char *sym_name, Elf_Sym *symtab,
                                Elf_Word symnum, Elf_Addr *sym_value,
                                const char *strtab)
{
   return DLSYM_lookup_symtab(sym_name, symtab, symnum, sym_value, 
                              FALSE, strtab);
}

/*****************************************************************************/
/* DLSYM_lookup_local_symtab() - Lookup the symbol name in the given symbol  */
/*                               table. Symbol must have local binding.      */
/*                               Return the value in sym_value and return    */
/*                               TRUE if the lookup succeeds.                */
/*****************************************************************************/
BOOL DLSYM_lookup_local_symtab(const char *sym_name, Elf_Sym *symtab,
                               Elf_Word symnum, Elf_Addr *sym_value, 
			       const char *strtab)
{
   return DLSYM_lookup_symtab(sym_name, symtab, symnum, sym_value, 
                              TRUE, strtab);
}

/*****************************************************************************/
/* CANONICAL_SYMBOL_LOOKUP() - Find the symbol definition. Look up the local */
/*                             symbol table to find the symbol. If it is a   */
/*                             definition and cannot be pre-empted, return   */
/*                             it. Otherwise, do a look up in the global     */
/*                             symbol table that contains the symbol tables  */
/*                             from all the necessary modules.               */
/*****************************************************************************/
BOOL DLSYM_canonical_lookup(DLOAD_HANDLE handle, int32_t sym_index, 
                            DLIMP_Dynamic_Module *dyn_module, 
                            Elf_Addr *sym_value)
{
   /*------------------------------------------------------------------------*/
   /* Lookup the symbol table to get the symbol characteristics.             */
   /*------------------------------------------------------------------------*/
   Elf_Sym          *sym     = &dyn_module->symtab[sym_index];
   int32_t           st_bind = ELF32_ST_BIND(sym->st_info);
   int32_t           st_vis  = ELF32_ST_VISIBILITY(sym->st_other);
   BOOL              is_def  = (sym->st_shndx != SHN_UNDEF && 
                               (sym->st_shndx < SHN_LORESERVE || 
                                sym->st_shndx == SHN_ABS || 
                                sym->st_shndx == SHN_COMMON || 
                                sym->st_shndx == SHN_XINDEX));
   const char *sym_name = (const char *)dyn_module->strtab + sym->st_name;

#if LOADER_DEBUG
   if (debugging_on)
      DLIF_trace("DLSYM_canonical_lookup: %d, %s\n", sym_index, sym_name);
#endif

   /*------------------------------------------------------------------------*/
   /* Local symbols and symbol definitions that cannot be pre-empted         */
   /* are resolved by the definition in the same module.                     */
   /*------------------------------------------------------------------------*/
   if (st_bind == STB_LOCAL || st_vis != STV_DEFAULT)
   {
      /*---------------------------------------------------------------------*/
      /* If it is a local symbol or non-local that cannot be preempted,      */
      /* the definition should be found in the same module. If we don't      */
      /* find the definition it is an error.                                 */
      /*---------------------------------------------------------------------*/
      if (!is_def)
      {
         DLIF_error(DLET_SYMBOL, 
                    "Local/non-imported symbol %s definition is not found "
                    "in module %s!\n", sym_name, dyn_module->name);
         return FALSE;
      }
      else
      {
         if (sym_value) *sym_value = sym->st_value;
         return TRUE; 
      }
   }
   /*------------------------------------------------------------------------*/
   /* Else we have either pre-emptable definition or undef symbol. Try a     */
   /* global look up.                                                        */
   /*------------------------------------------------------------------------*/
   return DLSYM_global_lookup(handle, sym_name, dyn_module->loaded_module, 
                              sym_value); 
}

