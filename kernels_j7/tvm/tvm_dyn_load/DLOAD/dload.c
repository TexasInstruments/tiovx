/*
* dload.c
*
* Core Dynamic Loader Reference Implementation
*
* This implementation of the core dynamic loader is platform independent,
* but it is object file format dependent.  In particular, this
* implementation supports ELF object file format.
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

#include <limits.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "ArrayList.h"
#include "Queue.h"
#include "Stack.h"

#include "symtab.h"
#include "dload_endian.h"
#include "elf32.h"
#include "dload.h"
#include "relocate.h"
#include "dload_api.h"

#ifdef ARM_TARGET
#include "arm_dynamic.h"
#endif

#ifdef C60_TARGET
#include "c60_dynamic.h"
#endif

#ifdef C70_TARGET
#include "c70_dynamic.h"
#endif

#include "virtual_targets.h"

/*---------------------------------------------------------------------------*/
/* These globals are used only to test the reference client implementation.  */
/*---------------------------------------------------------------------------*/
int global_argc;
char **global_argv;

/*---------------------------------------------------------------------------*/
/* Contains filenames (type const char*) the system is in the process of     */
/* loading.  Used to detect cycles in incorrectly compiled ELF binaries.     */
/*---------------------------------------------------------------------------*/
Array_List DLIMP_module_dependency_list;

/*---------------------------------------------------------------------------*/
/* Contains objects (type DLIMP_Loaded_Module) that the system has loaded into     */
/* target memory.                                                            */
/*---------------------------------------------------------------------------*/
TYPE_QUEUE_IMPLEMENTATION(DLIMP_Loaded_Module*, loaded_module_ptr)
loaded_module_ptr_Queue DLIMP_loaded_objects = TYPE_QUEUE_INITIALIZER;

/*---------------------------------------------------------------------------*/
/* Dependency Graph Queue - FIFO queue of dynamic modules that are loaded    */
/* when client asks to load a dynamic executable or library. Note that       */
/* dependents that have already been loaded with another module will not     */
/* appear on this queue.                                                     */
/*---------------------------------------------------------------------------*/
TYPE_STACK_IMPLEMENTATION(DLIMP_Dynamic_Module*, dynamic_module_ptr)
dynamic_module_ptr_Stack DLIMP_dependency_stack = TYPE_STACK_INITIALIZER;

/*---------------------------------------------------------------------------*/
/* Current virtual target set after reading the file headers. This is used   */
/* to access target specific functions.                                      */ 
/*---------------------------------------------------------------------------*/
VIRTUAL_TARGET *cur_target = NULL;

/*---------------------------------------------------------------------------*/
/* Support for profiling performance of dynamic loader core.                 */
/*---------------------------------------------------------------------------*/
#if LOADER_DEBUG
static clock_t cycle0 = 0;
static clock_t cycle_end = 0;
#define profile_start_clock() (cycle0 = clock())
#define profile_stop_clock()  (cycle_end = clock())
#define profile_cycle_count() (cycle_end - cycle0)
#endif

/*---------------------------------------------------------------------------*/
/* The dynamic loader will now create a table TI_init_table to store         */
/* pre-init and init data. This is done because pre-init and                 */
/* init functions could reference as-yet unrelocated symbols from other      */
/* modules. As such it is safer to store relevant function addresses and     */
/* execute them only after all modules are relocated.                        */
/*---------------------------------------------------------------------------*/
TYPE_QUEUE_IMPLEMENTATION(IF_single_record*, IF_table)
IF_table_Queue TI_init_table = TYPE_QUEUE_INITIALIZER;

static VIRTUAL_TARGET *get_vt_obj(int given_id);
static void read_args_from_section(DLIMP_Loaded_Module* ep_module);
static BOOL seg_has_space_for_write(DLIMP_Loaded_Module* lmodule, int sz); 
static BOOL write_arguments_to_args_section(DLOAD_HANDLE handle, 
                                            int argc, char** argv, 
					    DLIMP_Loaded_Module *ep_module);

/*****************************************************************************/
/* DLOAD_create()                                                            */
/*                                                                           */
/*    Create an instance of the dynamic loader core.                         */
/*                                                                           */
/*    client_handle:  Private client token to be returned during select DLIF */
/*                   function calls.                                         */
/*                                                                           */
/*    returns: an opaque DLOAD core loader handle, identifying this instance.*/
/*                                                                           */
/*****************************************************************************/
DLOAD_HANDLE DLOAD_create(void *client_handle)
{
    LOADER_OBJECT *pLoaderObject = DLIF_malloc(sizeof(LOADER_OBJECT));
                                                                               
    /*-----------------------------------------------------------------------*/
    /* Fill out the Loader Object:                                           */
    /*-----------------------------------------------------------------------*/
    /* Set up initial objects_loading queue.                                 */
    /*-----------------------------------------------------------------------*/
    AL_initialize(&(pLoaderObject->DLIMP_module_dependency_list),
		  sizeof (const char*), 1);
									   
    /*-----------------------------------------------------------------------*/
    /* Initialize Loaded Module Ptr Queue                                    */
    /*-----------------------------------------------------------------------*/
    loaded_module_ptr_initialize_queue(&pLoaderObject->DLIMP_loaded_objects);
									   
    /*-----------------------------------------------------------------------*/
    /* Initialize Dynamic Module Ptr Stack                                   */
    /*-----------------------------------------------------------------------*/
    dynamic_module_ptr_initialize_stack(&pLoaderObject->DLIMP_dependency_stack);
									   
    pLoaderObject->file_handle = 1;
									   
    /*-----------------------------------------------------------------------*/
    /* Store client token, so it can be handed back during DLIF calls        */
    /*-----------------------------------------------------------------------*/
    pLoaderObject->client_handle = client_handle;
                                                                               
    return((DLOAD_HANDLE)pLoaderObject);
}
                                                                               
/*****************************************************************************/
/* DLOAD_destroy()                                                           */
/*                                                                           */
/*    Remove an instance of the dynamic loader core, and free all resources  */
/*    allocated during DLOAD_create().                                       */
/*                                                                           */
/*    client_handle:  Private client token to be returned during select DLIF */
/*                   function calls.                                         */
/*    Preconditions: 1) handle must be valid.                                */
/*                   2) Loader instance must be in "UNLOADED" state.         */
/*                                                                           */
/*****************************************************************************/
void  DLOAD_destroy(DLOAD_HANDLE handle)
{
    LOADER_OBJECT     * pLoaderObject;
                                                                               
    pLoaderObject = (LOADER_OBJECT *)handle;
    AL_destroy(&(pLoaderObject->DLIMP_module_dependency_list));
                                                                               
    /*--------------------------*/
    /* Free the instance object */
    /*--------------------------*/
    DLIF_free (pLoaderObject);
}

/*****************************************************************************/
/* DLIMP_get_first_dyntag()                                                  */
/*                                                                           */
/*    Return value for first tag entry in the given dynamic table whose      */
/*    tag type matches the given key.                                        */
/*                                                                           */
/*****************************************************************************/
uint32_t DLIMP_get_first_dyntag(int tag, Elf_Dyn* dyn_table)
{
   /*------------------------------------------------------------------------*/
   /* Spin through dynamic segment looking for a specific dynamic tag.       */
   /* Return the value associated with the tag, if the tag is found.         */
   /*------------------------------------------------------------------------*/
   Elf_Dyn *dtp = dyn_table;

   while (dtp->d_tag != DT_NULL)
   {
      if (dtp->d_tag == tag) return dtp->d_un.d_val;
      else dtp++;
   }

   /*------------------------------------------------------------------------*/
   /* Tag wasn't found, return a known bogus value for the tag.              */
   /*------------------------------------------------------------------------*/
   return INT_MAX;
}

/*****************************************************************************/
/* dload_and_allocate_dependencies()                                         */
/*                                                                           */
/*    If not already loaded, load each dependent file identified in the      */
/*    dynamic segment with a DT_NEEDED tag.  Dependent files are listed in   */
/*    order and should be loaded in the same order that they appear in the   */
/*    dynamic segment.                                                       */
/*                                                                           */
/*****************************************************************************/
static BOOL dload_and_allocate_dependencies( DLOAD_HANDLE handle,
                                             DLIMP_Dynamic_Module *dyn_module)
{
   /*------------------------------------------------------------------------*/
   /* Spin through each dynamic tag entry in the dynamic segment.            */
   /*------------------------------------------------------------------------*/
   Elf_Dyn* dyn_nugget = dyn_module->dyntab;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

#if LOADER_DEBUG
   if (debugging_on) 
      DLIF_trace("Starting dload_and_allocate_dependencies() for %s ...\n", 
                 dyn_module->name);
#endif

   while(dyn_nugget->d_tag != DT_NULL)
   {
      /*---------------------------------------------------------------------*/
      /* For each DT_NEEDED dynamic tag that we find in the dynamic segment, */
      /* load the dependent file identified by the so_name value attached    */
      /* to the DT_NEEDED dynamic tag.                                       */
      /*---------------------------------------------------------------------*/
      if (dyn_nugget->d_tag == DT_NEEDED)
      {
         loaded_module_ptr_Queue_Node* ptr;

#if LOADER_DEBUG
         if (debugging_on)
            DLIF_trace("Found DT_NEEDED: %s\n", 
                       dyn_module->strtab+dyn_nugget->d_un.d_val);
#endif

         /*------------------------------------------------------------------*/
         /* Find out if the file named by the DT_NEEDED tag has already      */
         /* been loaded.  If it has, then we only have to bump the use count */
         /* of the named dependent file.                                     */
         /*------------------------------------------------------------------*/
         for (ptr = pHandle->DLIMP_loaded_objects.front_ptr; ptr != NULL; 
              ptr = ptr->next_ptr)
         {


            if (!strcmp(ptr->value->name,
                        dyn_module->strtab + dyn_nugget->d_un.d_val))
            {
               ptr->value->use_count++;
               AL_append(&(dyn_module->loaded_module->dependencies),
                         &(ptr->value->file_handle));
               break;
            }
         }

         /*------------------------------------------------------------------*/
         /* If the named dependent file has not been loaded, then we ask the */
         /* client to invoke a load of the dependent file on our behalf.     */ 
         /*------------------------------------------------------------------*/
         if (ptr == NULL)
         {
            int32_t dependent_handle = DLIF_load_dependent(
                                             pHandle->client_handle,
                                             dyn_module->strtab + 
                                                       dyn_nugget->d_un.d_val);
            AL_append(&(dyn_module->loaded_module->dependencies), 
	                                                    &dependent_handle);
            if (dependent_handle == 0) return FALSE;
         }
      }

      dyn_nugget++;
   }

#if LOADER_DEBUG
   if (debugging_on) 
      DLIF_trace("Finished dload_and_allocate_dependencies() for %s\n", 
                 dyn_module->name);
#endif

   return TRUE;
}

/*****************************************************************************/
/* load_object()                                                             */
/*                                                                           */
/*    Finish the process of loading an object file.                          */
/*                                                                           */
/*****************************************************************************/
static int load_object(LOADER_FILE_DESC *fd, DLIMP_Dynamic_Module *dyn_module)
{
   /*------------------------------------------------------------------------*/
   /* With the dynamic loader already running on the target, we are able to  */
   /* relocate directly into target memory, so there is nothing more to be   */
   /* done (at least in the bare-metal dynamic linking ABI model).           */
   /*------------------------------------------------------------------------*/
   return 1;
}

/*****************************************************************************/
/* write_arguments_to_args_section()                                         */
/*                                                                           */
/*    Write passed-in argv and argc to image of the .args section in target  */
/*    memory, so they can be accessed by the target program.                 */
/*                                                                           */
/*    This implementation assumes that the host and target are the same;     */
/*    that is, we can write directly to target memory.                       */
/*                                                                           */
/*****************************************************************************/
static BOOL write_arguments_to_args_section(DLOAD_HANDLE handle, 
                                            int argc, char** argv, 
                                            DLIMP_Loaded_Module *ep_module)
{
   int mem_inc   = MEM_INC;
   int ptr_sz    = PTR_SZ;
   int p_size    = ptr_sz / mem_inc;
   int i_size    = T_INTSZ / mem_inc;
   int c_size    = T_CHARSZ /mem_inc;
   int argv_offset = 0;
   int str_offset  = 0;
   int size        = 0;
   int arg;
   char **targ_argv_pointers = NULL;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   TARGET_ADDRESS c_args_addr;
   uint8_t *c_args = NULL;

#if LOADER_DEBUG
   if (debugging_on)
      DLIF_trace("Write_arguments_to_args_section:\n");
#endif

   /*-----------------------------------------------------------------------*/
   /* IF NO ARGUMENTS, ABORT QUIETLY, WITH a SUCCESSFUL CODE.               */
   /*-----------------------------------------------------------------------*/
   if (argc == 0) return TRUE;

   /*-----------------------------------------------------------------------*/
   /* __c_args__ points to the beginning of the .args section, if there     */
   /* is one.  This is stored in the Loaded Module, and must have a         */
   /* legitimate address. If not, abort with Warning.                       */
   /*-----------------------------------------------------------------------*/
   c_args_addr = ep_module->c_args;
   if (c_args_addr == 0 || c_args_addr == (TARGET_ADDRESS)-1)
   { 
      DLIF_warning(DLWT_MISC, "__c_args__ does not have valid value.\n"); 
      return FALSE;
   }

   /*-----------------------------------------------------------------------*/
   /* WE OUGHT TO WORRY ABOUT ALIGNMENT: IF SECTION ISN'T PROPERLY ALIGNED, */
   /* ABORT THE PROCESSING OF ARGUMENTS WITH A NICE ERROR MESSAGE.          */
   /*-----------------------------------------------------------------------*/
   if (c_args_addr && (c_args_addr & (MAX(p_size, i_size) - 1)))
   { 
      DLIF_warning(DLWT_MISC, ".args section not properly aligned\n"); 
      return FALSE;
   }
   /*-----------------------------------------------------------------------*/
   /* Assumes target address points into host memory; that is, the target   */
   /* *is* the host.                                                        */
   /*-----------------------------------------------------------------------*/
   c_args = (uint8_t *)c_args_addr;

   /*-----------------------------------------------------------------------*/
   /* CALCULATE OFFSET IN TABLE WHERE ARGV AND THE STRINGS WILL BE STORED.  */
   /* NOTE THAT argv MAY NEED MORE ALIGNMENT THAN AN INTEGER, SO ITS OFFSET */
   /* IS REALLY THE MAXIMUM OF A POINTER SIZE AND INTEGER SIZE.  ALSO NOTE  */
   /* WE NEED TO ALLOCATE AN EXTRA POINTER FOR argv[argc].                  */
   /*-----------------------------------------------------------------------*/
   argv_offset = MAX(p_size, i_size);
   str_offset  = argv_offset + (argc * p_size) + p_size;

   /*-----------------------------------------------------------------------*/
   /* CALCULATE SPACE REQUIRED FOR WRITING OUT .args SECTION. CHECK IF THE  */
   /* SEGMENT HAS ENOUGH SPACE AVAILABLE. IF NOT, RETURN WITH ERROR CODE.   */ 
   /*-----------------------------------------------------------------------*/
   size = str_offset;

   for (arg = 0; arg < argc; arg++)
        size += (c_size * (strlen(argv[arg]) + 1));
   
   if (!seg_has_space_for_write(ep_module, size)) 
   {
      DLIF_warning(DLWT_MISC, 
                 "Segment has insufficient space for .args contents\n");
      return FALSE;
   }

   /*-----------------------------------------------------------------------*/
   /* OVERALL, WE NEED TO CREATE A TARGET IMAGE THAT CORRESPONDS TO:        */
   /*     int argc;                                                         */
   /*     char *argv[argc];                                                 */
   /*     <strings pointed to by argv>                                      */
   /* So say, for C6x, for "-v -d", we would need 22 bytes:                 */
   /*     4 bytes // argc                                                   */
   /*     4 bytes // argv[0] pointer value                                  */
   /*     4 bytes // argv[1] pointer value                                  */
   /*     4 bytes // argv[argc] end of pointer value array, normally 0      */
   /*     3 bytes // "-v"                                                   */
   /*     3 bytes // "-d"                                                   */
   /*-----------------------------------------------------------------------*/

   /*-----------------------------------------------------------------------*/
   /* FIRST WRITE OUT ARGC.                                                 */
   /*-----------------------------------------------------------------------*/
#if LOADER_DEBUG
   if (debugging_on)
      DLIF_trace ("-- Copy %d bytes from "ADDRFMT" to "ADDRFMT"\n",
                  i_size, &argc, c_args);
#endif

   DLIF_memcpy(pHandle->client_handle, c_args, &argc, i_size);

   /*-----------------------------------------------------------------------*/
   /* CREATE AN INTERNAL ARRAY OF ARGV POINTER VALUES, THEN WRITE THEM OUT  */
   /*-----------------------------------------------------------------------*/
   targ_argv_pointers = (char **)DLIF_malloc((argc + 1) * sizeof(char*));
   for (arg = 0; arg < argc ; arg++)
   {
       targ_argv_pointers[arg] = (char *)(c_args + str_offset);
       str_offset += (strlen(argv[arg]) + 1) * c_size;

#if LOADER_DEBUG
   if (debugging_on)
       DLIF_trace ("\t\ttarg_argv_pointers[%d] : "ADDRFMT"\n",
                   arg, targ_argv_pointers[arg]);
#endif
   }

   targ_argv_pointers[argc] = (char *)0;

   /*-----------------------------------------------------------------------*/
   /* WRITE OUT THIS INTERNAL ARRAY OF ARGV POINTER VALUES                  */
   /*-----------------------------------------------------------------------*/
   for (arg = 0; arg <= argc; arg++)
   {
#if LOADER_DEBUG
   if (debugging_on)
       DLIF_trace ("-- Copy %d bytes from "ADDRFMT" to "ADDRFMT"\n",
                   p_size, &targ_argv_pointers[arg], 
                   (c_args + argv_offset));
#endif
       DLIF_memcpy(pHandle->client_handle, 
                   (void *)(c_args + argv_offset), 
                   &targ_argv_pointers[arg], 
		   p_size);
       argv_offset += p_size;
   }

#if LOADER_DEBUG
if (debugging_on)
{
   DLIF_trace ("\t\targv being copied : "ADDRFMT"\n", argv);
   for (arg = 0; arg < argc; arg++)
   {
       DLIF_trace ("\t\t---\n\t\t&argv[%d] being copied : "ADDRFMT"\n", arg, 
                   &argv[arg]);
       DLIF_trace ("\t\targv[%d] being copied : "ADDRFMT"\n", arg, 
                   argv[arg]);
       DLIF_trace ("\t\targv[%d] being copied : %s\n", arg, (char *)argv[arg]);
   }
}
#endif

   /*-----------------------------------------------------------------------*/
   /* LASTLY WRITE OUT ALL THE STRINGS.                                     */
   /*-----------------------------------------------------------------------*/
   for (arg = 0; arg < argc; arg++)
   {
#if LOADER_DEBUG
   if (debugging_on)
      DLIF_trace ("-- Copy %d bytes from "ADDRFMT" to "ADDRFMT"\n",
                  (uint32_t)strlen(argv[arg]) + 1,
                  &argv[arg], 
                  targ_argv_pointers[arg]); 
#endif
      DLIF_memcpy(pHandle->client_handle, 
                  (void *)(targ_argv_pointers[arg]), 
                  argv[arg],  
                  strlen(argv[arg]) + 1); 
   }

  DLIF_free(targ_argv_pointers);

  return TRUE;
}


/*****************************************************************************/
/* initialize_loaded_module()                                                */
/*                                                                           */
/*    Initialize DLIMP_Loaded_Module internal data object associated with a  */
/*    dynamic module.  This function will also set up a queue of             */
/*    DLIMP_Loaded_Segment(s) associated with the loaded module.             */
/*    This function is called as we are getting ready to actually load the   */
/*    object file contents into target memory.  Each segment will get a      */
/*    target memory request that it can use to ask the client for target     */
/*    memory space.  This function will also assign a file handle to the     */
/*    loaded module.                                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* In applications that use the DSBT model, this function will also need to  */
/* negotiate the module's DSBT index with the client.                        */
/*                                                                           */
/*****************************************************************************/
static void initialize_loaded_module(DLOAD_HANDLE handle,
                                     DLIMP_Dynamic_Module *dyn_module)
{
   int i;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   /*------------------------------------------------------------------------*/
   /* Allocate a DLIMP_Loaded_Module data structure for the specified ELF    */
   /* file and assign a file handle for it (bumping the file handle counter  */
   /* as we go).                                                             */
   /*------------------------------------------------------------------------*/
   DLIMP_Loaded_Module *loaded_module =
          dyn_module->loaded_module = DLIF_malloc(sizeof(DLIMP_Loaded_Module));

#if LOADER_DEBUG || LOADER_PROFILE
   /*------------------------------------------------------------------------*/
   /* Start clock on initialization of loaded module object.                 */
   /*------------------------------------------------------------------------*/
   if (debugging_on || profiling_on)
   {
      DLIF_trace("Starting initialize_loaded_module() ...\n");
      if (profiling_on) profile_start_clock();
   }
#endif

   if (dyn_module->name) 
   {
      loaded_module->name = DLIF_malloc(strlen(dyn_module->name) + 1);
      strcpy(loaded_module->name, dyn_module->name);
   }
   else 
      loaded_module->name = "<unknown>"; 

   loaded_module->file_handle = pHandle->file_handle++;
   loaded_module->direct_dependent_only = dyn_module->direct_dependent_only;
   loaded_module->use_count = 1;

   /*------------------------------------------------------------------------*/
   /* In case we wrapped around the file handle, return error.               */
   /*------------------------------------------------------------------------*/
   if (pHandle->file_handle == 0)
      DLIF_error(DLET_MISC, "DLOAD File handle overflowed.\n");

   /*------------------------------------------------------------------------*/
   /* Initially the loaded module does not have access to its global         */
   /* symbols.  These need to be copied from the dynamic module (see call    */
   /* to DLSYM_copy_globals() below).                                        */
   /*                                                                        */
   /* THESE INITIALIZATIONS SHOULD BE MOVED TO AN INIT ROUTINE FOR THE       */
   /* LOADED MODULE                                                          */
   /*------------------------------------------------------------------------*/
   loaded_module->gsymtab = NULL;
   loaded_module->gstrtab = NULL;
   loaded_module->gsymnum = loaded_module->gstrsz = 0;

   /*------------------------------------------------------------------------*/
   /* Initialize the Array_List of dependencies.                             */
   /*------------------------------------------------------------------------*/
   AL_initialize(&(loaded_module->dependencies), sizeof(int), 1);

   if (dyn_module->symtab)
      DLSYM_copy_globals(dyn_module);
     
   /*------------------------------------------------------------------------*/
   /* Initialize the module loaded segments Array_List.                      */
   /*------------------------------------------------------------------------*/
   AL_initialize(&(loaded_module->loaded_segments),
                 sizeof(DLIMP_Loaded_Segment), dyn_module->phnum);

   /*------------------------------------------------------------------------*/
   /* Spin thru segment headers and process each load segment encountered.   */
   /*------------------------------------------------------------------------*/
   for (i = 0; i < dyn_module->phnum; i++)
      if (dyn_module->phdr[i].p_type == PT_LOAD)
      {
         /*------------------------------------------------------------------*/
         /* Note that this is parallel to and does not supplant the ELF      */
         /* phdr tables.                                                     */
         /*------------------------------------------------------------------*/
         DLIMP_Loaded_Segment seg;
         seg.obj_desc = DLIF_malloc(sizeof(struct DLOAD_MEMORY_SEGMENT));
         seg.phdr.p_vaddr = dyn_module->phdr[i].p_vaddr;
         seg.phdr.p_type = PT_NULL;
         seg.phdr.p_paddr = 0;
         seg.phdr.p_offset = dyn_module->phdr[i].p_offset;
         seg.obj_desc->target_page = 0; /*not used*/
         seg.obj_desc->target_address = 0; /*not loaded yet */
         seg.modified = 0;
         seg.phdr.p_filesz = seg.obj_desc->objsz_in_bytes 
	                   = dyn_module->phdr[i].p_filesz;
         seg.phdr.p_memsz = seg.obj_desc->memsz_in_bytes 
	                  = dyn_module->phdr[i].p_memsz;
         seg.phdr.p_align = dyn_module->phdr[i].p_align;
         seg.phdr.p_flags = dyn_module->phdr[i].p_flags;
         seg.host_address = NULL;
         seg.input_vaddr  = 0;
         AL_append(&(loaded_module->loaded_segments), &seg);
      }

   /*------------------------------------------------------------------------*/
   /* Initialize the DSO termination information for this module.            */
   /* It will be copied over from the enclosing dyn_module object when       */
   /* placement is completed and dyn_module's local copy of the dynamic      */
   /* table is updated.                                                      */
   /*------------------------------------------------------------------------*/
   loaded_module->fini_array   = (Elf_Addr) NULL;
   loaded_module->fini_arraysz = 0;
   loaded_module->fini         = (Elf_Addr) NULL;

#if LOADER_DEBUG || LOADER_PROFILE
   if (debugging_on || profiling_on)
   {
      DLIF_trace("Finished initialize_loaded_module()\n");
      if (profiling_on)
      {
         profile_stop_clock();
         DLIF_trace("Took %lu cycles.\n", 
	            (unsigned long)profile_cycle_count());
      }
   }
#endif

}

/*****************************************************************************/
/* load_static_segment()                                                     */
/*                                                                           */
/*    The core dynamic loader requires that a statically linked executable   */
/*    be placed in target memory at the location that was determined during  */
/*    the static link that created the executable.  Failure to get the       */
/*    required target memory where the static executable is to be loaded     */
/*    will cause the dynamic loader to emit an error and abort the load.     */
/*                                                                           */
/*****************************************************************************/
static BOOL load_static_segment(DLOAD_HANDLE handle, LOADER_FILE_DESC *fd, 
                                DLIMP_Dynamic_Module *dyn_module)
{
   int i;
   DLIMP_Loaded_Segment* seg = (DLIMP_Loaded_Segment*) 
                              (dyn_module->loaded_module->loaded_segments.buf);
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   /*------------------------------------------------------------------------*/
   /* For each segment in the loaded module, build up a target memory        */
   /* request for the segment, get rights to target memory where we want     */
   /* to load the segment from the client, then get the client to write the  */
   /* segment contents out to target memory to the appropriate address.      */
   /*------------------------------------------------------------------------*/
   for (i = 0; i < dyn_module->loaded_module->loaded_segments.size; i++)
   {
      struct DLOAD_MEMORY_REQUEST targ_req;
      seg[i].obj_desc->target_page = 0;
      targ_req.flags = 0;

      /*---------------------------------------------------------------------*/
      /* This is a static executable.  DLIF_allocate should give us the      */
      /* address we ask for or fail.                                         */
      /*---------------------------------------------------------------------*/
      if (seg[i].phdr.p_flags & PF_X) targ_req.flags |= DLOAD_SF_executable;
      if (seg[i].phdr.p_flags & PF_W) targ_req.flags |= DLOAD_SF_writable;


      targ_req.align = seg[i].phdr.p_align;
      seg[i].obj_desc->target_address = (TARGET_ADDRESS)seg[i].phdr.p_vaddr;
      targ_req.flags &= ~DLOAD_SF_relocatable;
      targ_req.fp = fd;
      targ_req.segment = seg[i].obj_desc;
      targ_req.offset = seg[i].phdr.p_offset;
      targ_req.flip_endian = dyn_module->wrong_endian;

      /*---------------------------------------------------------------------*/
      /* Ask the client side of the dynamic loader to allocate target memory */
      /* for this segment to be loaded into.                                 */
      /*---------------------------------------------------------------------*/
      if (!DLIF_allocate(pHandle->client_handle, &targ_req)) return FALSE;

      /*---------------------------------------------------------------------*/
      /* If there is any initialized data in the segment, we'll first write  */
      /* it into a host writable buffer (DLIF_copy()) and then flush it to   */
      /* target memory.                                                      */
      /*---------------------------------------------------------------------*/
      if (seg[i].phdr.p_filesz)
      {
         DLIF_copy(pHandle->client_handle, &targ_req);
         DLIF_write(pHandle->client_handle, &targ_req);
      }
   }

   return TRUE;
}

/*****************************************************************************/
/* relocate_target_dynamic_tag_info()                                        */
/*                                                                           */
/*    Update a target specific dynamic tag value that happens to be a        */
/*    virtual address of a section. Returns TRUE if the tag was updated or   */
/*    is not a virtual address and FALSE if it was not successfully updated  */
/*    or was not recognized.                                                 */
/*****************************************************************************/
static BOOL relocate_target_dynamic_tag_info(DLIMP_Dynamic_Module *dyn_module, 
                                             int i)
{
   return cur_target->relocate_dynamic_tag_info(dyn_module, i);
}

/*****************************************************************************/
/* DLIMP_update_dyntag_section_address()                                     */
/*                                                                           */
/*    Given the index of a dynamic tag which we happen to know points to a   */
/*    section address, find the program header table entry associated with   */
/*    the specified address and update the tag value with the real address   */
/*    of the section.                                                        */
/*                                                                           */
/*****************************************************************************/
BOOL DLIMP_update_dyntag_section_address(DLIMP_Dynamic_Module *dyn_module, 
                                         int32_t i)
{
   int j;
   DLIMP_Loaded_Segment *seg = (DLIMP_Loaded_Segment *)
                              (dyn_module->loaded_module->loaded_segments.buf);

   /*------------------------------------------------------------------------*/
   /* If dynamic tag does not access an existing section, then no update     */
   /* is required.                                                           */
   /*------------------------------------------------------------------------*/
   if (dyn_module->dyntab[i].d_un.d_ptr == (Elf_Addr)0)
      { return TRUE; }

   for (j = 0; j < dyn_module->loaded_module->loaded_segments.size; j++)
   {
      if ((dyn_module->dyntab[i].d_un.d_ptr >= seg[j].input_vaddr) &&
          (dyn_module->dyntab[i].d_un.d_ptr < 
	   (seg[j].input_vaddr + seg[j].phdr.p_memsz)))
      {
         dyn_module->dyntab[i].d_un.d_ptr += 
                                    (seg[j].phdr.p_vaddr - seg[j].input_vaddr);
         return TRUE;
      }
   }

   return FALSE;
}

/*****************************************************************************/
/* relocate_dynamic_tag_info()                                               */
/*                                                                           */
/*    Once segment allocation has been completed, we'll need to go through   */
/*    the dynamic table and update any tag values that happen to be virtual  */
/*    addresses of segments (DT_C6000_DSBT_BASE, for example).               */
/*                                                                           */
/*****************************************************************************/
static BOOL relocate_dynamic_tag_info(LOADER_FILE_DESC *fd,
                                      DLIMP_Dynamic_Module *dyn_module)
{
   /*------------------------------------------------------------------------*/
   /* Spin through dynamic table loking for tags that have a value which is  */
   /* the virtual address of a section. After the sections are allocated,    */
   /* we'll need to update these values with the new address of the section. */
   /*------------------------------------------------------------------------*/
   int i;
   for (i = 0; dyn_module->dyntab[i].d_tag != DT_NULL; i++)
   {
      switch (dyn_module->dyntab[i].d_tag)
      {
         /*------------------------------------------------------------------*/
	 /* Only tag values that are virtual addresses will be affected.     */
         /*------------------------------------------------------------------*/
         case DT_NEEDED: 
         case DT_PLTRELSZ:
         case DT_HASH:
         case DT_STRTAB:
         case DT_SYMTAB: 
         case DT_RELA:
         case DT_RELASZ:
         case DT_RELAENT:
         case DT_STRSZ:
         case DT_SYMENT:
         case DT_SONAME:
         case DT_RPATH:
         case DT_SYMBOLIC:
         case DT_REL:
         case DT_RELSZ:
         case DT_RELENT:
         case DT_PLTREL:
         case DT_DEBUG:
         case DT_TEXTREL:
         case DT_BIND_NOW:
         case DT_INIT_ARRAYSZ:
         case DT_RUNPATH:
         case DT_FLAGS:
         case DT_PREINIT_ARRAYSZ:
            continue;

         /*------------------------------------------------------------------*/
	 /* NOTE!!!                                                          */
         /* case DT_ENCODING:  -- tag type has same "id" as DT_PREINIT_ARRAY */
         /*------------------------------------------------------------------*/

         /*------------------------------------------------------------------*/
         /* This is a generic dynamic tag whose value is a virtual address   */
         /* of a section. It needs to be relocated to the section's actual   */
         /* address in target memory.                                        */
         /*------------------------------------------------------------------*/
         case DT_PREINIT_ARRAY:
         case DT_INIT:
         case DT_INIT_ARRAY:
	    if (!DLIMP_update_dyntag_section_address(dyn_module, i))
	       return FALSE;

            continue;

         /*------------------------------------------------------------------*/
	 /* Once we have resolved the actual address of termination function */
	 /* sections, we need to copy their addresses over to the loaded     */
	 /* module object (dyn_module will be deleted before we get to       */
	 /* unloading the module).                                           */
         /*------------------------------------------------------------------*/
         case DT_FINI_ARRAY:
         case DT_FINI:
	    if (!DLIMP_update_dyntag_section_address(dyn_module, i))
	       return FALSE;

	    if (dyn_module->dyntab[i].d_tag == DT_FINI)
	       dyn_module->loaded_module->fini = 
	                                      dyn_module->dyntab[i].d_un.d_ptr;
	    else
	       dyn_module->loaded_module->fini_array = 
	                                      dyn_module->dyntab[i].d_un.d_ptr;

            continue;

         case DT_FINI_ARRAYSZ:
	    dyn_module->loaded_module->fini_arraysz = 
	                                      dyn_module->dyntab[i].d_un.d_val;
	    continue;

         /*------------------------------------------------------------------*/
	 /* Is this a virtual address???                                     */
         /*------------------------------------------------------------------*/
         case DT_JMPREL: /* is this a virtual address??? */
            continue;

         /*------------------------------------------------------------------*/
         /* The remaining dynamic tag types should be target specific. If    */
         /* something generic slips through to here, then the handler for    */
         /* relocating target specific dynamic tags should fail.             */
         /*------------------------------------------------------------------*/
         default:
            if (!relocate_target_dynamic_tag_info(dyn_module, i))
               return FALSE;
      }
   }

   /*------------------------------------------------------------------------*/
   /* We've gotten through all of the dynamic table without incident.        */
   /* All dynamic tag values that were virtual section addresses should have */
   /* been updated with the final address of the section that they point to. */
   /*------------------------------------------------------------------------*/
   return TRUE;
}

/*****************************************************************************/
/* allocate_dynamic_segments_and relocate_symbols()                          */
/*                                                                           */
/*    Allocate target memory for each segment in this module, getting a      */
/*    host-accessible space to copy the content of each segment into.  Then  */
/*    update the symbol table and program header table to reflect the new    */
/*    target address for each segment.  Processing of the dynamic relocation */
/*    entries will wait until all dependent files have been loaded and       */
/*    allocated into target memory.                                          */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* The relocation entries in the ELF file do not handle the necessary        */
/* adjustments to the memory addresses in the program header or symbol       */
/* tables.  These must be done manually.                                     */
/*                                                                           */
/* This is harder for us than for most dynamic loaders, because we have to   */
/* work in environments without virtual memory and thus where the offsets    */
/* between segments in memory may be different than they were in the file.   */
/* So, even though a dynamic loader usually only has to adjust all the       */
/* segments by a single fixed offset, we need to offset the symbols and      */
/* program header addresses segment by segment.  This job is done by the     */
/* function below.                                                           */
/*                                                                           */
/*****************************************************************************/
static BOOL allocate_dynamic_segments_and_relocate_symbols
                                            (DLOAD_HANDLE handle,
                                             LOADER_FILE_DESC *fd,
                                             DLIMP_Dynamic_Module *dyn_module)
{
   int i,j;
   DLIMP_Loaded_Segment* seg = (DLIMP_Loaded_Segment*)
                             (dyn_module->loaded_module->loaded_segments.buf);
   Elf_Ehdr *fhdr = &(dyn_module->fhdr);
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

#if LOADER_DEBUG || LOADER_PROFILE
   if (debugging_on || profiling_on)
   {
      DLIF_trace("Dynamic executable found.\n"
                 "Starting allocate_dynamic_segments_and_relocate_symbols()"
		 "...\n");
      if (profiling_on) profile_start_clock();
   }
#endif

   /*------------------------------------------------------------------------*/
   /* Spin through the list of loaded segments from the current module.      */
   /*------------------------------------------------------------------------*/
   for (i = 0; i < dyn_module->loaded_module->loaded_segments.size; i++)
   {
      /*--------------------------------------------------------------------*/
      /* Allocate target memory for segment via client-provided target      */
      /* memory API.                                                        */
      /*--------------------------------------------------------------------*/
      Elf_Addr addr_offset;
      struct DLOAD_MEMORY_REQUEST targ_req;
      seg[i].obj_desc->target_page = 0;
      targ_req.flags = 0;
      if (seg[i].phdr.p_flags & PF_X) targ_req.flags |= DLOAD_SF_executable;
      if (seg[i].phdr.p_flags & PF_W) targ_req.flags |= DLOAD_SF_writable;

      targ_req.align = seg[i].phdr.p_align;
      seg[i].obj_desc->target_address = (TARGET_ADDRESS)seg[i].phdr.p_vaddr;
      targ_req.flags |= DLOAD_SF_relocatable;
      targ_req.fp = fd;
      targ_req.segment = seg[i].obj_desc;
      targ_req.offset = seg[i].phdr.p_offset;
      targ_req.flip_endian = dyn_module->wrong_endian;

      if (!DLIF_allocate(pHandle->client_handle, &targ_req))
      {
         DLIF_error(DLET_MEMORY, "DLIF allocation failure.\n");
         return FALSE;
      }

      /*--------------------------------------------------------------------*/
      /* Calculate the offset we need to adjust segment header and symbol   */
      /* table addresses.                                                   */
      /*--------------------------------------------------------------------*/
      addr_offset = seg[i].obj_desc->target_address - seg[i].phdr.p_vaddr;

#if LOADER_DEBUG
      if (debugging_on)
      {
         DLIF_trace("Segment %d (at "ADDRFMT", "ADDRFMT" bytes) "
	            "relocated to "ADDRFMT"\n",
		    i, 
                    seg[i].phdr.p_vaddr, 
                    seg[i].phdr.p_memsz, 
                    seg[i].obj_desc->target_address);
         DLIF_trace("Addr Offset is "ADDRFMT"\n", addr_offset);
      }
#endif

      /*--------------------------------------------------------------------*/
      /* Update program entry point if needed.  Need to replace to deal     */
      /* with full ELF initialization routine.                              */
      /*--------------------------------------------------------------------*/
      if (dyn_module->relocate_entry_point && 
          fhdr->e_entry >= seg[i].phdr.p_vaddr && 
          fhdr->e_entry < seg[i].phdr.p_vaddr + seg[i].phdr.p_memsz)
      {
         fhdr->e_entry += addr_offset;

         /*------------------------------------------------------------------*/
         /* Mark the entry point as being relocated so we will not do it     */
         /* again.                                                           */
         /*------------------------------------------------------------------*/
         dyn_module->relocate_entry_point = FALSE;
#if LOADER_DEBUG
         if (debugging_on)
         {
            DLIF_trace("Entry point relocated to "ADDRFMT"\n", fhdr->e_entry); 
         }
#endif
      }

      /*---------------------------------------------------------------------*/
      /* Fix program header entries in segment and Elf_Phdr structs.         */
      /*---------------------------------------------------------------------*/
      for (j = 0; j < fhdr->e_phnum; j++)
         if (dyn_module->phdr[j].p_vaddr == seg[i].phdr.p_vaddr)
         {
            dyn_module->phdr[j].p_vaddr += addr_offset;
            dyn_module->phdr[i].p_paddr += addr_offset;
            break;
         }

      seg[i].input_vaddr = seg[i].phdr.p_vaddr;
      seg[i].phdr.p_vaddr += addr_offset;

      /*---------------------------------------------------------------------*/
      /* Great, now the hard part: fix offsets in symbols.  It would be nice */
      /* if there were an easier way to deal with this.                      */
      /*---------------------------------------------------------------------*/
      {
         Elf_Sym *gsymtab = dyn_module->loaded_module->gsymtab;
         Elf_Addr segment_start = seg[i].phdr.p_vaddr;
         Elf_Addr segment_end   = seg[i].phdr.p_vaddr + seg[i].phdr.p_memsz;
         Elf_Word global_index  = dyn_module->symnum - 
                                    dyn_module->loaded_module->gsymnum;
      
         for (j = 0; j < dyn_module->symnum; j++)
         {
            /*---------------------------------------------------------------*/
            /* Get the relocated symbol value.                               */
            /*---------------------------------------------------------------*/
            Elf_Addr symval_adj = dyn_module->symtab[j].st_value + 
	                            addr_offset;

            /*---------------------------------------------------------------*/
            /* If the symbol is defined in this segment, update the symbol   */
            /* value and mark the symbol so that we don't relocate it again. */
            /*---------------------------------------------------------------*/
            if (symval_adj >= segment_start && symval_adj <  segment_end &&
                dyn_module->symtab[j].st_shndx != INT16_MAX)
            {
               dyn_module->symtab[j].st_value = symval_adj;

               /*------------------------------------------------------------*/
               /* The module symbol table only has the global symbols.       */
               /*------------------------------------------------------------*/
               if (j >= global_index)
                  gsymtab[j-global_index].st_value = symval_adj;

               /*------------------------------------------------------------*/
               /* Mark the symbol as relocated.                              */
               /*------------------------------------------------------------*/
               dyn_module->symtab[j].st_shndx = INT16_MAX;
            }
         }
      }
   }

   /*------------------------------------------------------------------------*/
   /* Update dynamic tag information. Some dynamic tags have values which    */
   /* are virtual addresses of sections. These values need to be updated     */
   /* once segment allocation is completed and the new segment addresses are */
   /* known.                                                                 */
   /*------------------------------------------------------------------------*/
   /* We should only traverse through the dynamic table once because we want */
   /* to avoid the possibility of updating the same tag multiple times (an   */
   /* error, if it happens).                                                 */
   /*------------------------------------------------------------------------*/
   if (!relocate_dynamic_tag_info(fd, dyn_module))
   {
      DLIF_error(DLET_MISC, "Failed dynamic table update.\n");
      return FALSE;
   }

#if LOADER_DEBUG || LOADER_PROFILE
   if (debugging_on || profiling_on)
   {
      DLIF_trace("Finished allocate_dynamic_segments_and_relocate_symbols()\n");
      if (profiling_on)
      {
         profile_stop_clock();
         DLIF_trace("Took %lu cycles.\n", 
	            (unsigned long) profile_cycle_count());
      }
   }
#endif

   return TRUE;
}

/*****************************************************************************/
/* delete_DLIMP_Loaded_Module()                                              */
/*                                                                           */
/*    Free host memory associated with a DLIMP_Loaded_Module data structure  */
/*    and all of the DLIMP_Loaded_Segment objects that are associated with   */
/*    it.                                                                    */
/*                                                                           */
/*****************************************************************************/
static void delete_DLIMP_Loaded_Module(DLOAD_HANDLE handle,
                                       DLIMP_Loaded_Module **pplm)
{
    DLIMP_Loaded_Module *loaded_module = *pplm;
    DLIMP_Loaded_Segment *segments = (DLIMP_Loaded_Segment*)
                                          (loaded_module->loaded_segments.buf);
    LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

    /*-----------------------------------------------------------------------*/
    /* Spin through the segments attached to this loaded module, freeing up  */
    /* any target memory that was allocated by the client for the segment.   */
    /* DLOAD_load_symbols() will not load segments, so check target_address. */
    /*-----------------------------------------------------------------------*/
    int i;
    for (i = 0; i < loaded_module->loaded_segments.size; i++)
    {
        if (segments[i].obj_desc->target_address != 0 &&
            !DLIF_release(pHandle->client_handle, segments[i].obj_desc))
            DLIF_error(DLET_MISC, "Failed call to DLIF_release!\n");;
        DLIF_free(segments[i].obj_desc);
    }

    /*----------------------------------------------------------------------*/
    /* Hacky way of indicating that the base image is no longer available.  */
    /* WHHHHAAAAAAATTT!?!?!?!?!?!                                           */
    /*----------------------------------------------------------------------*/
    if (loaded_module->file_handle == DLIMP_application_handle)
        DLIMP_application_handle = 0;

    /*-----------------------------------------------------------------------*/
    /* Free host heap memory that was allocated for the internal loaded      */
    /* module data structure members.                                        */
    /*-----------------------------------------------------------------------*/
    if (loaded_module->name)    DLIF_free(loaded_module->name);
    if (loaded_module->gsymtab) DLIF_free(loaded_module->gsymtab);
    loaded_module->gsymnum = 0;
    if (loaded_module->gstrtab) DLIF_free(loaded_module->gstrtab);
    loaded_module->gstrsz = 0;
    AL_destroy(&(loaded_module->loaded_segments));
    AL_destroy(&(loaded_module->dependencies));

    /*-----------------------------------------------------------------------*/
    /* Finally, free the host memory for the loaded module object, then NULL */
    /* the pointer that was passed in.                                       */
    /*-----------------------------------------------------------------------*/
    DLIF_free(loaded_module);
    *pplm = NULL;
}

/*****************************************************************************/
/* new_DLIMP_Dynamic_Module()                                                */
/*                                                                           */
/*   Allocate a dynamic module data structure from host memory and           */
/*   initialize its members to their default values.                         */
/*                                                                           */
/*****************************************************************************/
static DLIMP_Dynamic_Module *new_DLIMP_Dynamic_Module(LOADER_FILE_DESC *fd)
{
    /*-----------------------------------------------------------------------*/
    /* Allocate space for dynamic module data structure from host memory.    */
    /*-----------------------------------------------------------------------*/
    DLIMP_Dynamic_Module *dyn_module = 
             (DLIMP_Dynamic_Module *)DLIF_malloc(sizeof(DLIMP_Dynamic_Module));

    /*-----------------------------------------------------------------------*/
    /* Initialize data members of the new dynamic module data structure.     */
    /*-----------------------------------------------------------------------*/
    dyn_module->name = NULL;
    dyn_module->fd = fd;
    dyn_module->phdr = NULL;
    dyn_module->phnum = 0;
    dyn_module->strtab = NULL;
    dyn_module->strsz = 0;
    dyn_module->dyntab = NULL;
    dyn_module->symtab = NULL;
    dyn_module->symnum = 0;
    dyn_module->gsymtab_offset = 0;
    dyn_module->gstrtab_offset = 0;
    dyn_module->c_args = 0;
    dyn_module->static_base = 0;
    dyn_module->argc = 0;
    dyn_module->argv = NULL;
    dyn_module->loaded_module = NULL;
    dyn_module->wrong_endian = 0;
    dyn_module->direct_dependent_only = TRUE;
    dyn_module->relocatable = FALSE;
    dyn_module->relocate_entry_point = TRUE;
    dyn_module->dsbt_size = 0;
    dyn_module->dsbt_index = DSBT_INDEX_INVALID;
    dyn_module->dsbt_base_tagidx = -1;

    dyn_module->preinit_array_idx = -1;
    dyn_module->preinit_arraysz = 0;
    dyn_module->init_idx = -1;
    dyn_module->init_array_idx = -1;
    dyn_module->init_arraysz = 0;

    return dyn_module;
}

/*****************************************************************************/
/* detach_loaded_module()                                                    */
/*                                                                           */
/*    Detach loaded module data structure from given dynamic module.  When   */
/*    an object file has been successfully loaded, the loader core will      */
/*    detach the loaded module data structure from the dynamic module data   */
/*    structure because the loaded module must continue to persist until is  */
/*    is actually unloaded from target memory.  If there is a problem with   */
/*    the load, then the host memory associated with the loaded module will  */
/*    be released as part of the destruction of the dynamic module.          */
/*                                                                           */
/*****************************************************************************/
static 
DLIMP_Loaded_Module *detach_loaded_module(DLIMP_Dynamic_Module *dyn_module)
{
    if (dyn_module && dyn_module->loaded_module)
    {
        DLIMP_Loaded_Module *loaded_module = dyn_module->loaded_module;
        dyn_module->loaded_module = NULL;
        return loaded_module;
    }

    return NULL;
}
/*****************************************************************************/
/* delete_DLIMP_Dynamic_Module()                                             */
/*                                                                           */
/*    Remove local copies of the string table, symbol table, program header  */
/*    table, and dynamic table.                                              */
/*                                                                           */
/*****************************************************************************/
static void delete_DLIMP_Dynamic_Module(DLOAD_HANDLE handle,
                                        DLIMP_Dynamic_Module **ppdm)
{
   DLIMP_Dynamic_Module *dyn_module = NULL;
   
   if (!ppdm || (*ppdm == NULL))
   {
      DLIF_error(DLET_MISC, 
                 "Internal Error: invalid argument to dynamic module "
		 "destructor function; aborting loader\n");
      DLIF_exit(EXIT_FAILURE);
   }
   
   dyn_module = *ppdm;
   if (dyn_module->name)     DLIF_free(dyn_module->name);
   if (dyn_module->strtab)   DLIF_free(dyn_module->strtab);
   if (dyn_module->symtab)   DLIF_free(dyn_module->symtab);
   if (dyn_module->phdr)     DLIF_free(dyn_module->phdr);
   if (dyn_module->dyntab)   DLIF_free(dyn_module->dyntab);

   /*------------------------------------------------------------------------*/
   /* If we left the loaded module attached to the dynamic module, then      */
   /* something must have gone wrong with the load.  Remove the loaded       */
   /* module from the queue of loaded modules, if it is there.  Then free    */
   /* the host memory allocated to the loaded module and its segments.       */
   /*------------------------------------------------------------------------*/
   if (dyn_module->loaded_module != NULL)
      delete_DLIMP_Loaded_Module(handle, &(dyn_module->loaded_module));

   /*------------------------------------------------------------------------*/
   /* Finally, free the host memory for this dynamic module object and NULL  */
   /* the pointer to the object.                                             */
   /*------------------------------------------------------------------------*/
   DLIF_free(dyn_module);
   *ppdm = NULL;
}

/*****************************************************************************/
/* file_header_magic_number_is_valid()                                       */
/*                                                                           */
/*    Given an object file header, check the magic number to ensure that it  */
/*    is an object file format that we recognize.  This implementation of    */
/*    the dynamic loader core will handle ELF object file format.            */
/*                                                                           */
/*****************************************************************************/
static BOOL file_header_magic_number_is_valid(Elf_Ehdr* header)
{
   /*------------------------------------------------------------------------*/
   /* Check for correct ELF magic numbers in file header.                    */
   /*------------------------------------------------------------------------*/
   if (header->e_ident[EI_MAG0] != ELFMAG0 ||
       header->e_ident[EI_MAG1] != ELFMAG1 ||
       header->e_ident[EI_MAG2] != ELFMAG2 ||
       header->e_ident[EI_MAG3] != ELFMAG3)
   {
      DLIF_error(DLET_FILE, "Invalid ELF magic number.\n");
      return FALSE;
   }

   return TRUE;
}

/*****************************************************************************/
/* file_header_machine_is_valid()                                            */
/*                                                                           */
/*    Check if the machine specified in the file header is supported by the  */
/*    loader.  If the loader was compiled with support for all targets,      */
/*    the machine will be initially set to EM_NONE.  Once a module has been  */
/*    loaded, all remaining modules must have the same machine value.        */
/*****************************************************************************/
static int file_header_machine_is_valid(Elf_Half e_machine)
{
   /*------------------------------------------------------------------------*/
   /* Currently we support ARM, C6x, and C7x                                 */
   /*------------------------------------------------------------------------*/
   switch(e_machine)
   {
#ifdef ARM_TARGET
      case EM_ARM :      return TRUE;
#endif
#ifdef C60_TARGET
      case EM_TI_C6000 : return TRUE;
#endif
#ifdef C70_TARGET
      case EM_TI_C7X   : return TRUE;
#endif

      default :          return FALSE;
   }
}

/*****************************************************************************/
/* is_valid_elf_object_file()                                                */
/*                                                                           */
/*    Check file size against anticipated end location of string table,      */
/*    symbol table, program header tables, etc.  If we anything untoward,    */
/*    then we declare that the ELF file is corrupt and the load is aborted.  */
/*                                                                           */
/*****************************************************************************/
static BOOL is_valid_elf_object_file(LOADER_FILE_DESC *fd, 
                                     DLIMP_Dynamic_Module *dyn_module)
{
   uint32_t fsz;
   int i;

   /*------------------------------------------------------------------------*/
   /* Get file size.                                                         */
   /*------------------------------------------------------------------------*/
   DLIF_fseek(fd, 0, LOADER_SEEK_END);
   fsz = DLIF_ftell(fd);

   /*------------------------------------------------------------------------*/
   /* Check for invalid table sizes (string table, symbol table, and         */
   /* program header tables).                                                */
   /*------------------------------------------------------------------------*/
   if (!((dyn_module->strsz < fsz) &&
         (dyn_module->symnum < fsz) &&
         (dyn_module->phnum * sizeof(Elf_Phdr)) < fsz))
   {
      DLIF_error(DLET_FILE, "Invalid ELF table bounds.\n");
      return FALSE;
   }

   /*------------------------------------------------------------------------*/
   /* Check for null so_name string in file with dynamic information.        */
   /*------------------------------------------------------------------------*/
   if (dyn_module->dyntab && !strcmp(dyn_module->name, ""))
   {
      DLIF_error(DLET_MISC, "Dynamic file lacks SO_NAME identifier.\n");
      return FALSE;
   }

   /*------------------------------------------------------------------------*/
   /* Check for invalid program header information.                          */
   /*------------------------------------------------------------------------*/
   for (i = 0; i < dyn_module->phnum; i++)
   {
      Elf_Phdr* phdr = dyn_module->phdr + i;

      /*---------------------------------------------------------------------*/
      /* Sanity check for relative sizes of filesz and memsz.                */
      /*---------------------------------------------------------------------*/
      if (!(phdr->p_type != PT_LOAD || phdr->p_filesz <= phdr->p_memsz))
      {
         DLIF_error(DLET_MISC, 
                    "Invalid file or memory size for segment %d.\n", i);
         return FALSE;
      }

      /*---------------------------------------------------------------------*/
      /* Check that segment file offset doesn't go off the end of the file.  */
      /*---------------------------------------------------------------------*/
      if (!(phdr->p_offset + phdr->p_filesz < fsz))
      {
         DLIF_error(DLET_FILE,
                  "File location of segment %d is past the end of file.\n", i);
         return FALSE;
      }
   }

   /*------------------------------------------------------------------------*/
   /* Check that a ET_DYN-type file is relocatable.                          */
   /*------------------------------------------------------------------------*/
   if (dyn_module->fhdr.e_type == ET_DYN && !dyn_module->symtab) return FALSE;

   /*------------------------------------------------------------------------*/
   /* All checks passed.                                                     */
   /*------------------------------------------------------------------------*/
   return TRUE;
}

/*****************************************************************************/
/* process_eiosabi()                                                         */
/*                                                                           */
/*   Check the EI_OSABI field to validate it and set any parameters based on */
/*   it.                                                                     */
/*****************************************************************************/
static BOOL process_eiosabi(DLIMP_Dynamic_Module* dyn_module)
{
   return cur_target->process_eiosabi(dyn_module);
}

/*****************************************************************************/
/* dload_file_header()                                                       */
/*                                                                           */
/*    Read ELF file header.  Store critical information in the provided      */
/*    DLIMP_Dynamic_Module record.  Check file header for validity.          */
/*                                                                           */
/*****************************************************************************/
static BOOL dload_file_header(LOADER_FILE_DESC *fd, 
                              DLIMP_Dynamic_Module *dyn_module)
{
   /*------------------------------------------------------------------------*/
   /* Read ELF file header from given input file.                            */
   /*------------------------------------------------------------------------*/
   DLIF_fread(&(dyn_module->fhdr), sizeof(Elf_Ehdr), 1, fd);

   /*------------------------------------------------------------------------*/
   /* Determine target vs. host endian-ness.  Does header data need to be    */
   /* byte swapped?                                                          */
   /*------------------------------------------------------------------------*/
   dyn_module->wrong_endian = 
                     (dyn_module->fhdr.e_ident[EI_DATA] != DLIMP_get_endian());

   /*------------------------------------------------------------------------*/
   /* Swap file header structures, if needed.                                */
   /*------------------------------------------------------------------------*/
   if (dyn_module->wrong_endian)
      DLIMP_change_ehdr_endian(&(dyn_module->fhdr));

   /*------------------------------------------------------------------------*/
   /* Write out magic ELF information for debug purposes.                    */
   /*------------------------------------------------------------------------*/
#if LOADER_DEBUG
   if (debugging_on)
   {
       DLIF_trace("ELF: %c%c%c\n", dyn_module->fhdr.e_ident[1],
                               dyn_module->fhdr.e_ident[2],
                               dyn_module->fhdr.e_ident[3]);
       DLIF_trace("ELF file header entry point: "ADDRFMT"\n", 
                               dyn_module->fhdr.e_entry);
   }
#endif


   /*------------------------------------------------------------------------*/
   /* Verify magic numbers in ELF file header.                               */
   /*------------------------------------------------------------------------*/
   if (!file_header_magic_number_is_valid(&(dyn_module->fhdr)))
   {
      DLIF_error(DLET_FILE, "Invalid ELF file header magic number.\n");
      return FALSE;
   }

   if (!file_header_machine_is_valid(dyn_module->fhdr.e_machine))
   {
       DLIF_error(DLET_FILE, "Invalid ELF file target machine.\n");
       return FALSE;
   }

   /*------------------------------------------------------------------------*/
   /* Verify file is an executable or dynamic shared object or library.      */
   /*------------------------------------------------------------------------*/
   if ((dyn_module->fhdr.e_type != ET_EXEC) && 
       (dyn_module->fhdr.e_type != ET_DYN))
   {
      DLIF_error(DLET_FILE, "Invalid ELF file type.\n");
      return FALSE;
   }

#if LOADER_DEBUG || LOADER_PROFILE
   /*------------------------------------------------------------------------*/
   /* Stop profiling clock when file header information has finished         */
   /* loading.  Re-start clock on initialization of symbol table, and        */
   /* dynamic table pointers.                                                */
   /*------------------------------------------------------------------------*/
   if (debugging_on || profiling_on)
   {
      DLIF_trace("done.\n");
      if (profiling_on)
      {
         profile_stop_clock();
         DLIF_trace("Took %lu cycles.\n", 
	            (unsigned long)profile_cycle_count());
         profile_start_clock();
      }
   }
#endif

   return TRUE;
}

/*****************************************************************************/
/* dload_program_header_table()                                              */
/*                                                                           */
/*    Make a local copy of the ELF object file's program header table in the */
/*    dynamic module data structure.                                         */
/*                                                                           */
/*****************************************************************************/
static void dload_program_header_table(LOADER_FILE_DESC *fd,
                                       DLIMP_Dynamic_Module *dyn_module)
{
   /*------------------------------------------------------------------------*/
   /* Read the program header tables from the object file.                   */
   /*------------------------------------------------------------------------*/
   Elf_Ehdr *fhdr = &(dyn_module->fhdr);
   dyn_module->phdr = (Elf_Phdr*)
                              (DLIF_malloc(fhdr->e_phnum * fhdr->e_phentsize));
   DLIF_fseek(fd, fhdr->e_phoff, LOADER_SEEK_SET);
   DLIF_fread(dyn_module->phdr, fhdr->e_phentsize, fhdr->e_phnum,fd);
   dyn_module->phnum = fhdr->e_phnum;

   /*------------------------------------------------------------------------*/
   /* Byte swap the program header tables if the target endian-ness is not   */
   /* the same as the host endian-ness.                                      */
   /*------------------------------------------------------------------------*/
   if (dyn_module->wrong_endian)
   {
      int i;
      for (i = 0; i < dyn_module->phnum; i++)
         DLIMP_change_phdr_endian(dyn_module->phdr + i);
   }
}

/*****************************************************************************/
/* dload_headers()                                                           */
/*                                                                           */
/*    Read ELF object file header and program header table information into  */
/*    the given dynamic module data structure.  If the object file contains  */
/*    dynamic information, read in the dynamic tags, dynamic symbol table,   */
/*    and global string table.  Check to make sure that we are not already   */
/*    in the process of loading the module (circular dependencies), then     */
/*    perform some level of sanity checking on the content of the file to    */
/*    provide some assurance that the file is not corrupted.                 */
/*                                                                           */
/*****************************************************************************/
static BOOL dload_headers(LOADER_FILE_DESC *fd,
                          DLIMP_Dynamic_Module *dyn_module)
{
#if LOADER_DEBUG || LOADER_PROFILE
   /*------------------------------------------------------------------------*/
   /* More progress information.  Start timing if profiling is enabled.      */
   /*------------------------------------------------------------------------*/
   if (debugging_on || profiling_on)
   {
      DLIF_trace("\nReading file headers ...\n");
      if (profiling_on) profile_start_clock();
   }
#endif

   /*------------------------------------------------------------------------*/
   /* Read file header information and check vs. expected ELF object file    */
   /* header content.                                                        */
   /*------------------------------------------------------------------------*/
   if (!dload_file_header(fd, dyn_module))
      return FALSE;

   /*------------------------------------------------------------------------*/
   /* Read program header table information into the dynamic module object.  */
   /*------------------------------------------------------------------------*/
   dload_program_header_table(fd, dyn_module);

   /*------------------------------------------------------------------------*/
   /* Once headers have been read in, use e_machine to set virtual target.   */
   /* This can then be used to access target specific functions.             */
   /*------------------------------------------------------------------------*/
   cur_target = get_vt_obj(dyn_module->fhdr.e_machine);
   if (!cur_target) 
   {
      DLIF_error(DLET_FILE, "Attempt to load invalid ELF file, '%s'.\n",
                    dyn_module->name);
      return FALSE;
   } 

   return TRUE;
}

/*****************************************************************************/
/* find_dynamic_segment()                                                    */
/*                                                                           */
/*    Find the dynamic segment in the given ELF object file, if there is     */
/*    one.  If the segment is found, then the segment ID output parameter    */
/*    is set to the index of the dynamic segment in the program header       */
/*    table.  If the dynamic segment is not found, the dynamic module's      */
/*    relocatable flag is set to FALSE, and return FALSE.                    */
/*                                                                           */
/*****************************************************************************/
static BOOL find_dynamic_segment(DLIMP_Dynamic_Module *dyn_module,
                                 Elf_Word *dyn_seg_idx)
{
   int i;

   /*------------------------------------------------------------------------*/
   /* We should have a valid dynamic module pointer and somewhere to put the */
   /* dynamic segment id, if we find one.  If either of these are missing,   */
   /* we should get an internal error and abort the loader.                  */
   /*------------------------------------------------------------------------*/
   if ((dyn_module == NULL) || (dyn_seg_idx == NULL))
   {
      DLIF_error(DLET_MISC, "Internal error: find_dynamic_segment() needs "
                            "non-NULL arguments.\n");
      DLIF_exit(EXIT_FAILURE);
   }

   /*------------------------------------------------------------------------*/
   /* Spin through segment program headers to find the dynamic segment.      */
   /*------------------------------------------------------------------------*/
   dyn_module->relocatable = TRUE;
   for (i = 0; i < dyn_module->phnum; i++)
      if (dyn_module->phdr[i].p_type == PT_DYNAMIC)
         { *dyn_seg_idx = i; return TRUE; }

   /*------------------------------------------------------------------------*/
   /* No dynamic segment found, mark the object module as not relocatable    */
   /* and warn the user.                                                     */
   /*------------------------------------------------------------------------*/
   dyn_module->relocatable = FALSE;

   return FALSE;
}

/*****************************************************************************/
/* copy_dynamic_table()                                                      */
/*                                                                           */
/*    Make a local copy of the dynamic table read from the dynamic segment   */
/*    in the ELF object file.                                                */
/*                                                                           */
/*****************************************************************************/
static void copy_dynamic_table(LOADER_FILE_DESC *fd,
                               DLIMP_Dynamic_Module *dyn_module,
                               Elf_Word dyn_seg_idx)
{
   /*------------------------------------------------------------------------*/
   /* Allocate space for the dynamic table from host memory and read its     */
   /* content from the ELF object file.                                      */
   /*------------------------------------------------------------------------*/
   Elf_Word num_elem;
   dyn_module->dyntab = DLIF_malloc(dyn_module->phdr[dyn_seg_idx].p_filesz);
   num_elem = dyn_module->phdr[dyn_seg_idx].p_filesz / sizeof(Elf_Dyn);
   DLIF_fseek(fd, dyn_module->phdr[dyn_seg_idx].p_offset, LOADER_SEEK_SET);
   DLIF_fread(dyn_module->dyntab, sizeof(Elf_Dyn), num_elem, fd);

   /*------------------------------------------------------------------------*/
   /* If necessary, byte swap each entry in the dynamic table.               */
   /*------------------------------------------------------------------------*/
   if (dyn_module->wrong_endian)
   {
      int i;
      for (i = 0; i < num_elem; i++)
         DLIMP_change_dynent_endian(&dyn_module->dyntab[i]);
   }
}

/*****************************************************************************/
/* process_target_dynamic_tag()                                              */
/*                                                                           */
/* Process a target specific dynamic tag entry.  Returns TRUE if the tag     */
/* was handled and FALSE if it was not recognized.                           */
/*****************************************************************************/
static BOOL process_target_dynamic_tag(DLIMP_Dynamic_Module* dyn_module, int i)
{
   return cur_target->process_dynamic_tag(dyn_module, i);
}

/*****************************************************************************/
/* process_dynamic_table()                                                   */
/*                                                                           */
/*    Process dynamic tag entries from the dynamic table.  At the conclusion */
/*    of this function, we should have made a copy of the global symbols     */
/*    and the global symbol names.                                           */
/*                                                                           */
/*****************************************************************************/
static BOOL process_dynamic_table(LOADER_FILE_DESC *fd,
                                  DLIMP_Dynamic_Module *dyn_module)
{
   int      i;
   BOOL     soname_found  = FALSE;
   Elf_Addr soname_offset = 0;
   Elf_Addr strtab_offset = 0;
   Elf_Addr hash_offset   = 0;
   Elf_Addr symtab_offset = 0;

   /*------------------------------------------------------------------------*/
   /* Iterate over the dynamic table in order to process dynamic tags.       */
   /* See ELF TIS Specification for details on the meaning of each dynamic   */
   /* tag.  The C6000 ELF ABI Specification provides more details about the  */
   /* TI specific C6000 ELF ABI tags.                                        */
   /*------------------------------------------------------------------------*/
   for (i = 0; dyn_module->dyntab[i].d_tag != DT_NULL; i++)
   {
      switch(dyn_module->dyntab[i].d_tag)
      {
         /*------------------------------------------------------------------*/
         /* DT_SONAME: Contains name of dynamic object, used for dependency  */
         /*            comparisons.  Its value is an offset from the start   */
         /*            of the string table.  We need to copy the string at   */
         /*            this offset into dmodule->name.                       */
         /*------------------------------------------------------------------*/
         case DT_SONAME:
#if LOADER_DEBUG
            if (debugging_on) DLIF_trace("Found SO_NAME.\n");
#endif
            /*---------------------------------------------------------------*/
            /* We store the offset of the so_name in the dynamic string      */
            /* table so that it doesn't matter which dynamic tag we see      */
            /* first (DT_SONAME actually is generated before DT_STRTAB).     */
            /*---------------------------------------------------------------*/
            soname_found = TRUE;
            soname_offset = dyn_module->dyntab[i].d_un.d_ptr;
            break;

         /*------------------------------------------------------------------*/
         /* DT_STRSZ: Contains the size of the string table.                 */
         /*------------------------------------------------------------------*/
         case DT_STRSZ:
            dyn_module->strsz = dyn_module->dyntab[i].d_un.d_val;

#if LOADER_DEBUG
        if (debugging_on)
           DLIF_trace("Found string table Size: 0x%x\n", dyn_module->strsz);
#endif
            break;

         /*------------------------------------------------------------------*/
         /* DT_STRTAB: Contains the file offset of the string table.  The    */
         /*            tag directly after this is guaranteed to be DT_STRSZ, */
         /*            containing the string table size.  We need to         */
         /*            allocate memory for the string table and copy it from */
         /*            the file.                                             */
         /*------------------------------------------------------------------*/
         case DT_STRTAB:
            strtab_offset = dyn_module->dyntab[i].d_un.d_ptr;
#if LOADER_DEBUG
            if (debugging_on)
               DLIF_trace("Found string table: "ADDRFMT"\n", strtab_offset);
#endif
            break;

         /*------------------------------------------------------------------*/
         /* DT_HASH: Contains the file offset of the symbol hash table.      */
         /*------------------------------------------------------------------*/
         case DT_HASH:
            hash_offset = dyn_module->dyntab[i].d_un.d_ptr;
#if LOADER_DEBUG
            if (debugging_on)
               DLIF_trace("Found symbol hash table: "ADDRFMT"\n", hash_offset);
#endif
            break;

         /*------------------------------------------------------------------*/
         /* DT_SYMTAB: Contains the file offset of the symbol table.         */
         /*------------------------------------------------------------------*/
         case DT_SYMTAB:
            symtab_offset = dyn_module->dyntab[i].d_un.d_ptr;
#if LOADER_DEBUG
            if (debugging_on)
               DLIF_trace("Found symbol table: "ADDRFMT"\n", symtab_offset);
#endif
            break;

         /*------------------------------------------------------------------*/
	 /* DSO Initialization / Termination Model Dynamic Tags              */
         /*------------------------------------------------------------------*/
	 /* For initialization tags, we store indices and array sizes in     */
	 /* the dyn_module. Termination works a little different, the        */
	 /* indices into the local copy of the dynamic table are stored in   */
	 /* dyn_module, but the DT_FINI_ARRAYSZ value is recorded with the   */
	 /* loaded module.                                                   */
         /*------------------------------------------------------------------*/
	 /* After placement is done, the DT_FINI and DT_FINI_ARRAY values    */
	 /* need to be copied from the local dynamic table into the loaded   */
	 /* module object.                                                   */
         /*------------------------------------------------------------------*/
	 case DT_PREINIT_ARRAY:
	    dyn_module->preinit_array_idx = i;
	    break;

	 case DT_PREINIT_ARRAYSZ:
	    dyn_module->preinit_arraysz = dyn_module->dyntab[i].d_un.d_val;
	    break;

	 case DT_INIT:
            dyn_module->init_idx = i;
	    break;

	 case DT_INIT_ARRAY:
	    dyn_module->init_array_idx = i;
	    break;

	 case DT_INIT_ARRAYSZ:
	    dyn_module->init_arraysz = dyn_module->dyntab[i].d_un.d_val;
	    break;

         /*------------------------------------------------------------------*/
	 /* This information will be copied over to the loaded module        */
	 /* object after placement has been completed and the information    */
	 /* in the dynamic table has been relocated.                         */
         /*------------------------------------------------------------------*/
	 case DT_FINI_ARRAY:
	 case DT_FINI_ARRAYSZ:
	 case DT_FINI:
	    break;

         /*------------------------------------------------------------------*/
         /* Unrecognized tag, may not be illegal, but is not explicitly      */
         /* handled by this function.  Should it be?                         */
         /*------------------------------------------------------------------*/
         default:
         {
            if (!process_target_dynamic_tag(dyn_module, i))
            {
#if LOADER_DEBUG
               if (debugging_on)
                  DLIF_trace("Unrecognized dynamic tag: 0x%X\n",
                               dyn_module->dyntab[i].d_tag);
#endif
            }
            
            break;
         }

      }
   }

   /*------------------------------------------------------------------------*/
   /* If string table offset and size were found, read string table in from  */
   /* the ELF object file.                                                   */
   /*------------------------------------------------------------------------*/
   if (strtab_offset && dyn_module->strsz)
   {
      DLIF_fseek(fd, strtab_offset, LOADER_SEEK_SET);
      dyn_module->strtab = DLIF_malloc(dyn_module->strsz);
      DLIF_fread(dyn_module->strtab, sizeof(uint8_t), dyn_module->strsz, fd);
   }
   else
   {
      DLIF_warning(DLWT_MISC, 
                   "Mandatory dynamic tag DT_STRTAB/DT_STRSZ not found!\n");
      return FALSE;
   }


   /*------------------------------------------------------------------------*/
   /* If symbol hash table is found read-in the hash table.                  */
   /*------------------------------------------------------------------------*/
   if (hash_offset)
   {
      /*---------------------------------------------------------------------*/
      /* Hash table has the following format. nchain equals the number of    */
      /* entries in the symbol table (symnum)                                */
      /*                                                                     */
      /*             +----------------------------+                          */
      /*             |          nbucket           |                          */
      /*             +----------------------------+                          */
      /*             |          nchain            |                          */
      /*             +----------------------------+                          */
      /*             |         bucket[0]          |                          */
      /*             |            ...             |                          */
      /*             |     bucket[nbucket-1]      |                          */
      /*             +----------------------------+                          */
      /*             |          chain[0]          |                          */
      /*             |            ...             |                          */
      /*             |       chain[nchain-1]      |                          */
      /*             +----------------------------+                          */
      /*---------------------------------------------------------------------*/
      Elf_Word hash_nbucket;
      Elf_Word hash_nchain;

      /*---------------------------------------------------------------------*/
      /* Seek to the hash offset and read first two words into nbucket and   */
      /* symnum.                                                             */
      /*---------------------------------------------------------------------*/
      DLIF_fseek(fd, hash_offset, LOADER_SEEK_SET);
      DLIF_fread(&(hash_nbucket), sizeof(Elf_Word), 1, fd);
      DLIF_fread(&(hash_nchain), sizeof(Elf_Word), 1, fd);
      if (dyn_module->wrong_endian)
      {
         DLIMP_change_endian32((int32_t*)(&(hash_nbucket)));
         DLIMP_change_endian32((int32_t*)(&(hash_nchain)));
      }

      /*---------------------------------------------------------------------*/
      /* The number of entires in the dynamic symbol table is not encoded    */
      /* anywhere in the elf file. However, the nchain is guaranteed to be   */
      /* the same as the number of symbols. Use nchain to set the symnum.    */
      /*---------------------------------------------------------------------*/
      dyn_module->symnum = hash_nchain;
#if LOADER_DEBUG
      if (debugging_on) DLIF_trace("symnum=%d\n", hash_nchain);
#endif
   }
   else
   {
      DLIF_warning(DLWT_MISC, "Mandatory dynamic tag DT_HASH is not found!\n");
      return FALSE;
   }

   /*------------------------------------------------------------------------*/
   /* Read dynamic symbol table.                                             */
   /*------------------------------------------------------------------------*/
   if (symtab_offset)
   {
      int j = 0;
      DLIF_fseek(fd, symtab_offset, LOADER_SEEK_SET);
      dyn_module->symtab = 
                 DLIF_malloc(dyn_module->symnum * sizeof(Elf_Sym));
      DLIF_fread(dyn_module->symtab, sizeof(Elf_Sym),
                 dyn_module->symnum, fd);
      if (dyn_module->wrong_endian)
      {
         for (j = 0; j < dyn_module->symnum; j++)
            DLIMP_change_sym_endian(dyn_module->symtab + j);
      }
   }
   else
   {
      DLIF_warning(DLWT_MISC, 
                   "Mandatory dynamic tag DT_SYMTAB is not found!\n");
      return FALSE;
   }

   /*------------------------------------------------------------------------*/
   /* Read the SONAME.                                                       */
   /*------------------------------------------------------------------------*/
   if (!soname_found)
   {
      DLIF_warning(DLWT_MISC, "Dynamic tag DT_SONAME is not found!\n");
      dyn_module->name = DLIF_malloc(sizeof(char));
      *dyn_module->name = '\0';
   }
   else
   {
      dyn_module->name = 
                    DLIF_malloc(strlen(dyn_module->strtab + soname_offset) + 1);
      strcpy(dyn_module->name, dyn_module->strtab + soname_offset);

#if LOADER_DEBUG
      if (debugging_on)
         DLIF_trace("Name of dynamic object: %s\n", dyn_module->name);
#endif
   }

   return TRUE;
}


/*****************************************************************************/
/* dload_dynamic_information()                                               */
/*                                                                           */
/*    Given a dynamic module with a dynamic segment which is located via     */
/*    given dynamic segment index, make a local copy of the dynamic table    */
/*    in the dynamic module object, then process the dynamic tag entries in  */
/*    the table.                                                             */
/*                                                                           */
/*****************************************************************************/
static BOOL dload_dynamic_information(LOADER_FILE_DESC *fd,
                                      DLIMP_Dynamic_Module *dyn_module,
                                      Elf_Word dyn_seg_idx)
{
   /*------------------------------------------------------------------------*/
   /* Read a copy of the dynamic table into the dynamic module object.       */
   /*------------------------------------------------------------------------*/
   copy_dynamic_table(fd, dyn_module, dyn_seg_idx);

   /*------------------------------------------------------------------------*/
   /* Process dynamic entries in the dynamic table.  If any problems are     */
   /* encountered, the loader should emit an error or warning and return     */
   /* FALSE here.                                                            */
   /*------------------------------------------------------------------------*/
   return process_dynamic_table(fd, dyn_module);
}

/*****************************************************************************/
/* check_circular_dependency()                                               */
/*                                                                           */
/*    Determine whether a dynamic module is already in the process of being  */
/*    loaded before we try to start loading it again.  If it is already      */
/*    being loaded, then the dynamic loader has detected a circular          */
/*    dependency.  An error will be emitted and the load will be aborted.    */
/*                                                                           */
/*****************************************************************************/
static BOOL check_circular_dependency(DLOAD_HANDLE handle,
                                      const char *dyn_mod_name)
{
   /*------------------------------------------------------------------------*/
   /* Check the name of the given dependency module to be loaded against the */
   /* list of modules that are currently in the process of being loaded.     */
   /* Report an error if any circular dependencies are detected.             */
   /*------------------------------------------------------------------------*/
   int i;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   for (i = 0; i < pHandle->DLIMP_module_dependency_list.size; i++)
      if (!strcmp(dyn_mod_name, 
                  ((char**)(pHandle->DLIMP_module_dependency_list.buf))[i]))
      {
         DLIF_error(DLET_MISC, 
                    "Circular dependency detected, '%s' is already in the " 
                    "process of loading.\n", dyn_mod_name);
         return FALSE;
      }

   return TRUE;
}

/*****************************************************************************/
/* dload_dynamic_segment()                                                   */
/*                                                                           */
/*    Find the dynamic segment in the given ELF module, if there is one.     */
/*    If there is a dynamic segment, then make a local copy of the dynamic   */
/*    table in the dynamic module object provided, then process the dynamic  */
/*    tag entries in the table.                                              */
/*                                                                           */
/*    If there is no dynamic segment, then we return success from this       */
/*    function, marking the dynamic module as "not relocatable".             */
/*                                                                           */
/*****************************************************************************/
static BOOL dload_dynamic_segment(DLOAD_HANDLE handle,
                                  LOADER_FILE_DESC *fd, 
                                  DLIMP_Dynamic_Module *dyn_module)
{
   /*------------------------------------------------------------------------*/
   /* If we don't find dynamic segment, the relocatable flag will have been  */
   /* set to false to indicate that the module is a static executable.  We   */
   /* still return TRUE from this function so that we can proceed with       */
   /* static loading.                                                        */
   /*------------------------------------------------------------------------*/
   Elf_Word dyn_seg_idx = 0;
   if (!find_dynamic_segment(dyn_module, &dyn_seg_idx))
      return TRUE;

   /*------------------------------------------------------------------------*/
   /* Process the OSABI now, after we know if the module is relocatable.     */
   /*------------------------------------------------------------------------*/
   if (!process_eiosabi(dyn_module))
   {
      DLIF_error(DLET_FILE, "Unsupported EI_OSABI value.\n");
      return FALSE;
   }

   /*------------------------------------------------------------------------*/
   /* Read the dynamic table from the ELF file, then process the dynamic     */
   /* tags in the table.                                                     */
   /*------------------------------------------------------------------------*/
   if (!dload_dynamic_information(fd, dyn_module, dyn_seg_idx))
      return FALSE;

   /*------------------------------------------------------------------------*/
   /* Check to make sure that this module is not already being loaded.  If   */
   /* is, then it will cause a circular dependency to be introduced.         */
   /* Loader should detect circular dependencies and emit an error.          */
   /*------------------------------------------------------------------------*/
   if (!check_circular_dependency(handle, dyn_module->name))
      return FALSE;

   return TRUE;
}

/*****************************************************************************/
/* COPY_SEGMENTS() -                                                         */
/*                                                                           */
/*   Copy all segments into host memory.                                     */
/*****************************************************************************/
static void copy_segments(DLOAD_HANDLE handle, LOADER_FILE_DESC* fp, 
                          DLIMP_Dynamic_Module* dyn_module)
{
   DLIMP_Loaded_Segment* seg =
      (DLIMP_Loaded_Segment*)(dyn_module->loaded_module->loaded_segments.buf);
   int s, seg_size = dyn_module->loaded_module->loaded_segments.size;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;


   for (s=0; s<seg_size; s++)
   {
      struct DLOAD_MEMORY_REQUEST targ_req;
      targ_req.fp = fp;
      targ_req.segment = seg[s].obj_desc;
      targ_req.offset = seg[s].phdr.p_offset;
      targ_req.flags = DLOAD_SF_relocatable;

      if (seg[s].phdr.p_flags & PF_X) targ_req.flags |= DLOAD_SF_executable;
      if (seg[s].phdr.p_flags & PF_W) targ_req.flags |= DLOAD_SF_writable;

      targ_req.align = seg[s].phdr.p_align;

      /*---------------------------------------------------------------------*/
      /* Copy segment data from the file into host buffer where it can       */
      /* be relocated.                                                       */
      /*---------------------------------------------------------------------*/
      DLIF_copy(pHandle->client_handle, &targ_req);
      seg[s].host_address = targ_req.host_address;
   }
}

/*****************************************************************************/
/* WRITE_SEGMENTS() -                                                        */
/*                                                                           */
/*   Write all segments to target memory.                                    */
/*****************************************************************************/
static void write_segments(DLOAD_HANDLE handle,
                          LOADER_FILE_DESC* fp, 
                          DLIMP_Dynamic_Module* dyn_module)
{
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;
   DLIMP_Loaded_Segment* seg =
      (DLIMP_Loaded_Segment*)(dyn_module->loaded_module->loaded_segments.buf);
   int s, seg_size = dyn_module->loaded_module->loaded_segments.size;

   for (s=0; s<seg_size; s++)
   {
      struct DLOAD_MEMORY_REQUEST targ_req;

      targ_req.fp = fp;
      targ_req.segment = seg[s].obj_desc;
      targ_req.offset = seg[s].phdr.p_offset;
      targ_req.flags = DLOAD_SF_relocatable;

      if (seg[s].phdr.p_flags & PF_X) targ_req.flags |= DLOAD_SF_executable;
      if (seg[s].phdr.p_flags & PF_W) targ_req.flags |= DLOAD_SF_writable;

      targ_req.align = seg[s].phdr.p_align;
      targ_req.host_address = seg[s].host_address;

      /*---------------------------------------------------------------------*/
      /* Copy segment data from the file into host buffer where it can       */
      /* be relocated.                                                       */
      /*---------------------------------------------------------------------*/
      DLIF_write(pHandle->client_handle, &targ_req);
   }
}

/*****************************************************************************/
/* SEG_HAS_SPACE_FOR_WRITE() -                                               */
/*                                                                           */
/*   Check if segment has enough space to recieve contents of .args section. */
/*****************************************************************************/
static BOOL seg_has_space_for_write(DLIMP_Loaded_Module* lmodule, int sz) 
{
   DLIMP_Loaded_Segment* seg =
                  (DLIMP_Loaded_Segment*)(lmodule->loaded_segments.buf);
   int s, seg_size = lmodule->loaded_segments.size;

   Elf_Addr write_address = lmodule->c_args;

   for (s=0; s<seg_size; s++)
   {
      Elf_Addr seg_boundary = 
            seg[s].phdr.p_vaddr + seg[s].obj_desc->memsz_in_bytes;

      /*---------------------------------------------------------------------*/
      /* If address to write to is greater than segment addr and less than   */
      /* segment end, it must lie in current segment.                        */
      /*---------------------------------------------------------------------*/
      if ((write_address >= seg[s].phdr.p_vaddr) &&
          (write_address < seg_boundary))
      {
         if ((write_address + sz) > seg_boundary)
         {
#if LOADER_DEBUG
	    if (debugging_on)
	    {
	       DLIF_trace("Write requires "ADDRFMT" bytes\n",
	          write_address + sz);
	       DLIF_trace("Seg boundary at : "ADDRFMT"\n",
	          seg_boundary);
	       DLIF_trace("WARNING - Not enough space in segment\n");
	    }
#endif
            return FALSE;
         }
         else return TRUE;
      }
   }
   /*------------------------------------------------------------------------*/
   /* Given address doesn't belong to any known segment.                     */
   /*------------------------------------------------------------------------*/
   return FALSE;
}


/*****************************************************************************/
/* DLOAD_initialize()                                                        */
/*                                                                           */
/*    Construct and initialize data structures internal to the dynamic       */
/*    loader core.                                                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    This function is deprecated, replaced by DLOAD_create().               */
/*                                                                           */
/*****************************************************************************/
void DLOAD_initialize(DLOAD_HANDLE handle)
{
}

/*****************************************************************************/
/* DLOAD_finalize()                                                          */
/*                                                                           */
/*    Destroy and finalize data structures internal to the dynamic           */
/*    loader core.                                                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    This function is deprecated, replaced by DLOAD_destroy().              */
/*                                                                           */
/*****************************************************************************/
void DLOAD_finalize(DLOAD_HANDLE handle)
{
}

/*****************************************************************************/
/* dload_static_executable()                                                 */
/*                                                                           */
/*    Account for target memory allocated to static executable and wrap up   */
/*    loading.  No relocation is necessary.                                  */
/*                                                                           */
/*****************************************************************************/
static int32_t dload_static_executable(DLOAD_HANDLE handle,
                                       LOADER_FILE_DESC *fd,
                                       DLIMP_Dynamic_Module *dyn_module)
{
   int32_t local_file_handle = 0;

#if LOADER_DEBUG
   if (debugging_on) DLIF_trace("Starting dload_static_executable() ...\n");
#endif

   /*------------------------------------------------------------------------*/
   /* Set entry point for static executable and attempt to allocate target   */
   /* memory for the static executable.                                      */
   /*------------------------------------------------------------------------*/
   dyn_module->loaded_module->entry_point = dyn_module->fhdr.e_entry;
   if (load_static_segment(handle, fd, dyn_module) && 
       load_object(fd, dyn_module))
   {
      /*---------------------------------------------------------------------*/
      /* If successful, we'll want to detach the loaded module object from   */
      /* the dynamic module object that created it.  Take note of the file   */
      /* handle.                                                             */
      /*---------------------------------------------------------------------*/
      DLIMP_Loaded_Module *loaded_module = dyn_module->loaded_module;
      (void)detach_loaded_module(dyn_module);
      local_file_handle = loaded_module->file_handle;
   }

   /*------------------------------------------------------------------------*/
   /* Static load failed.  Flag an error.                                    */
   /*------------------------------------------------------------------------*/
   else
      DLIF_error(DLET_MEMORY, 
                 "Failed to allocate target memory for static executable.\n");

   /*------------------------------------------------------------------------*/
   /* Destruct dynamic module object.                                        */
   /*------------------------------------------------------------------------*/
   delete_DLIMP_Dynamic_Module(handle, &dyn_module);

#if LOADER_DEBUG
   if (debugging_on) DLIF_trace("Finished dload_static_executable()\n");
#endif

   return local_file_handle;
}

#if LOADER_DEBUG || LOADER_PROFILE
int DLREL_relocations;
time_t DLREL_total_reloc_time;
#endif

/*****************************************************************************/
/* process_dynamic_module_relocations()                                      */
/*                                                                           */
/*    Make a host-accessible copy of all of the segments, process all        */
/*    relocation entries associated with the given module within that        */
/*    space, then write the updated segment buffers back out to target       */
/*    memory.                                                                */
/*                                                                           */
/*****************************************************************************/
static void process_dynamic_module_relocations(DLOAD_HANDLE handle,
                                               LOADER_FILE_DESC *fd,
                                               DLIMP_Dynamic_Module *dyn_module)
{
#if LOADER_DEBUG || LOADER_PROFILE
   if(debugging_on || profiling_on)
   {
      DLIF_trace("Running relocate()...\n");
      if (profiling_on) profile_start_clock();
   }
#endif

   /*------------------------------------------------------------------------*/
   /* Copy segments from file to host memory                                 */
   /*------------------------------------------------------------------------*/
   copy_segments(handle, fd, dyn_module);

   /*------------------------------------------------------------------------*/
   /* Process dynamic relocations.                                           */
   /*------------------------------------------------------------------------*/
   DLREL_relocate(handle, fd, dyn_module);

   /*------------------------------------------------------------------------*/
   /* Write segments from host memory to target memory                       */
   /*------------------------------------------------------------------------*/
   write_segments(handle, fd, dyn_module);

#if LOADER_DEBUG || LOADER_PROFILE
   /*------------------------------------------------------------------------*/
   /* Report timing and progress information for relocation step.            */
   /*------------------------------------------------------------------------*/
   if (debugging_on || profiling_on)
   {
      if (profiling_on)
      {
         profile_stop_clock();
         DLIF_trace("Took %lu cycles.\n", 
	                              (unsigned long) profile_cycle_count());
         DLIF_trace("Total reloc time: %lu\n", 
                                      (unsigned long) DLREL_total_reloc_time);
         DLIF_trace("Time per relocation: %ld\n",
         DLREL_relocations ? DLREL_total_reloc_time / DLREL_relocations : 0);
      }

      DLIF_trace("Number of relocations: %d\n", DLREL_relocations);
      DLREL_total_reloc_time = DLREL_relocations = 0;
      if (profiling_on) profile_start_clock();
   }
#endif

}

/*****************************************************************************/
/* store_preinit_data()                                                      */
/*                                                                           */
/*    Given a dynamic module object, store pre-initialization function       */
/*    information. The user may also provide a custom iniitialization        */
/*    function that needs to be executed before the compiler                 */
/*    generated static initialization functions are executed.                */
/*    The dynamic loader will now create a table TI_init_table to store      */
/*    pre-init and init data. This is done because pre-init and              */
/*    init functions could reference as-yet unrelocated symbols from other   */
/*    modules. As such it is safer to store relevant function addresses and  */
/*    execute them only after all modules are relocated (CQ34088).           */
/*                                                                           */
/*****************************************************************************/
static void store_preinit_data(DLIMP_Dynamic_Module *dyn_module)
{
   IF_single_record *preinit_rec = NULL; 
   /*------------------------------------------------------------------------*/
   /* Check for presence of DT_PREINIT_ARRAY and DT_PREINIT_ARRAYSZ          */
   /* dynamic tags associated with this module. The dyn_module object will   */
   /* hold the relevant indices into the local copy of the dynamic table.    */
   /* The value of the DT_INIT_ARRAY tag will have been updated after        */
   /* placement of the  module was completed. Arrays of size 0 will be       */
   /* ignored (CQ36935).                                                     */
   /*------------------------------------------------------------------------*/
   if (dyn_module->preinit_arraysz > 0)
   {
      preinit_rec = (IF_single_record *)DLIF_malloc(sizeof(IF_single_record));
      /*---------------------------------------------------------------------*/
      /* Retrieve the address of the .preinit_array section from the value   */
      /* of the DT_PREINIT_ARRAY tag, and store it in the TI_init_table.     */
      /*---------------------------------------------------------------------*/
      preinit_rec->size = dyn_module->preinit_arraysz;
      preinit_rec->sect_addr = (TARGET_ADDRESS)
                (dyn_module->dyntab[dyn_module->preinit_array_idx].d_un.d_ptr);
   }
 
   if (preinit_rec) IF_table_enqueue(&TI_init_table, preinit_rec);
}

/*****************************************************************************/
/* store_init_data()                                                         */
/*                                                                           */
/*    Given a dynamic module object, save off initialization function(s) for */
/*    all global and static data objects that are defined in the module      */
/*    which require construction. The dynamic loader will now create a table */
/*    TI_init_table to store pre-init and init data. This is done because    */
/*    pre-init and init functions could reference as-yet unrelocated symbols */
/*    from other modules. As such it is safer to store relevant function     */
/*    addresses and execute them only after all modules are relocated.       */
/*                                                                           */
/*****************************************************************************/
static void store_init_data(DLIMP_Dynamic_Module *dyn_module)
{
   /*------------------------------------------------------------------------*/
   /* Check for presence of a DT_INIT dynamic tag associated with this       */
   /* module. The dynamic module will hold the index into the local copy of  */
   /* the dynamic table. This entry in the dynamic table will have been      */
   /* updated after placement of the module is completed.                    */
   /*------------------------------------------------------------------------*/
   if (dyn_module->init_idx != -1)
   {
      IF_single_record *init_rec = 
                   (IF_single_record *)DLIF_malloc(sizeof(IF_single_record));
      /*---------------------------------------------------------------------*/
      /* Retrieve the address of the initialization function from the value  */
      /* of the DT_INIT tag, and get the client to execute the function.     */
      /*---------------------------------------------------------------------*/
      init_rec->size = 0;
      init_rec->sect_addr = (TARGET_ADDRESS)
                         (dyn_module->dyntab[dyn_module->init_idx].d_un.d_ptr);

      IF_table_enqueue(&TI_init_table, init_rec);
   }

   /*------------------------------------------------------------------------*/
   /* Check for presence of a DT_INIT_ARRAY and DT_INIT_ARRAYSZ dynamic tags */
   /* associated with this module. The dyn_module object will hold the       */
   /* relevant indices into the local copy of the dynamic table. The value   */
   /* of the DT_INIT_ARRAY tag will have been updated after placement of the */
   /* module was completed. Arraysz must be a postive number > 0, else it    */
   /* be ignored (CQ36935).                                                  */
   /*------------------------------------------------------------------------*/
   if (dyn_module->init_arraysz > 0)
   {
      IF_single_record *arr_rec = 
                   (IF_single_record *)DLIF_malloc(sizeof(IF_single_record));
      /*---------------------------------------------------------------------*/
      /* Retrieve the address of the .init_array section from the value of   */
      /* DT_INIT_ARRAY tag.                                                  */
      /*---------------------------------------------------------------------*/
      arr_rec->size = dyn_module->init_arraysz;
      arr_rec->sect_addr = (TARGET_ADDRESS)
                   (dyn_module->dyntab[dyn_module->init_array_idx].d_un.d_ptr);

      IF_table_enqueue(&TI_init_table, arr_rec);
   }
}

/*****************************************************************************/
/* execute_module_initialization()                                           */
/*                                                                           */
/*    Given a dynamic module object, execute pre-initialization and          */
/*    initialization function(s) for all global and static data objects that */
/*    are defined in the module which require construction. The user may     */
/*    also provide a custom iniitialization function that needs to be        */
/*    executed before the compiler generated static initialization functions */
/*    are executed.                                                          */
/*    Note that the functions to be executed have already been saved off in  */
/*    the TI_init_table, by store_preinit_data() and store_init_data().      */
/*                                                                           */
/*****************************************************************************/
static void execute_module_initialization(DLOAD_HANDLE handle)
{
   IF_single_record *val = NULL;
   IF_table_Queue_Node *curr_ptr = TI_init_table.front_ptr;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   for (; curr_ptr; curr_ptr = curr_ptr->next_ptr)                               
   {                                                                   
      val = curr_ptr->value;

      /*---------------------------------------------------------------------*/
      /* A size of 0 indicates DT_INIT, otherwise this is an ARRAY.         */
      /*---------------------------------------------------------------------*/
      if (val->size != 0)
      {
         /*------------------------------------------------------------------*/
         /* Now make a loader-accessible copy of the .init_array section.    */
         /*------------------------------------------------------------------*/
         int32_t i;
         int32_t num_init_fcns = val->size/sizeof(TARGET_ADDRESS);
         TARGET_ADDRESS *init_array_buf = (TARGET_ADDRESS *)
                                            DLIF_malloc(val->size);
   
         DLIF_read(pHandle->client_handle, 
	           init_array_buf, 1, val->size, 
		   (TARGET_ADDRESS)val->sect_addr);
   
         /*------------------------------------------------------------------*/
         /* Call each function whose address occupies an entry in array in   */
         /* the order that they appear in the array. The size of the array is*/
         /* provided by the init_arraysz field in the dynamic module (copied */
         /* earlier when the dynamic table was read in). Make sure that      */
         /* function addresses are valid before execution.                   */
         /*------------------------------------------------------------------*/
         for (i = 0; i < num_init_fcns; i++)
            if (init_array_buf[i])
               DLIF_execute(pHandle->client_handle, 
	                    (TARGET_ADDRESS)(init_array_buf[i]));
            else
               DLIF_warning(DLWT_MISC,
                  "DT_INIT_ARRAY/DT_PREINIT_ARRAY function address is NULL!");

         DLIF_free(init_array_buf);
      }
      else
      {
         if (val->sect_addr) 
            DLIF_execute(pHandle->client_handle, 
	                 (TARGET_ADDRESS)(val->sect_addr));
         else 
            DLIF_warning(DLWT_MISC, "DT_INIT function address is NULL!"); 
      }
   }           
}

/*****************************************************************************/
/* adjust_module_init_fini()                                                 */
/*    If the dynamic loader need not process the module initialization       */
/*    and termination (fini section) then adjust the module info so that     */
/*    the respective sizes become zero.                                      */
/*****************************************************************************/
static void adjust_module_init_fini(DLIMP_Dynamic_Module *dm)
{
   /*------------------------------------------------------------------------*/
   /* The C6x RTS boot code has the function _c_int00 which performs         */
   /* the C/C++ initialization. This function processes the .init_array      */
   /* to perform the C/C++ initialization and handles termination through    */
   /* the at_exit functionality. If the dynamic executable we are loading    */
   /* includes _c_int00, the loader assumes that the application code takes  */
   /* care of all initialization and termination. Hence the loader won't     */
   /* perform the initialization and termination.                            */
   /* NOTE: Use of __TI_STACK_SIZE is a hack. The _c_int00 symbol is not     */
   /*       in the dynamic symbol table. The right fix is for the linker     */
   /*       not to generate the init array tags if the build includes RTS    */
   /*       boot routine.                                                    */
   /*------------------------------------------------------------------------*/
   if (dm->fhdr.e_type == ET_EXEC && 
       DLSYM_lookup_local_symtab("__TI_STACK_SIZE", dm->symtab, dm->symnum, 
                                 NULL, dm->strtab))
   {
      dm->init_arraysz   = 0;
      dm->init_array_idx = -1;

      dm->preinit_arraysz   = 0;
      dm->preinit_array_idx = -1;

      dm->loaded_module->fini_arraysz = 0;
      dm->loaded_module->fini_array   = 0;
      dm->loaded_module->fini         = 0;
   }
}

/*****************************************************************************/
/* relocate_dependency_graph_modules()                                       */
/*                                                                           */
/*    For each dynamic module on the dependency stack, process dynamic       */
/*    relocation entries then perform initialization for all global and      */
/*    static objects that are defined in tha given module. The stack is      */
/*    emptied from the top (LIFO).  Each dynamic module object is popped     */
/*    off the top of the stack, the module gets relocated, its global and    */
/*    static objects that need to be constructed will be constructed, and    */
/*    then, after detaching the loaded module object from its dynamic        */
/*    module, the dynamic module object is destructed.                       */
/*                                                                           */
/*****************************************************************************/
static
int32_t relocate_dependency_graph_modules(DLOAD_HANDLE handle,
                                          LOADER_FILE_DESC *fd,
                                          DLIMP_Dynamic_Module *dyn_module)
{
   /*------------------------------------------------------------------------*/
   /* Processing of relocations will only be triggered when this function    */
   /* is called from the top-level object module (at the bottom of the       */
   /* dependency graph stack).                                               */
   /*------------------------------------------------------------------------*/
   int32_t local_file_handle = dyn_module->loaded_module->file_handle;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;
   dynamic_module_ptr_Stack_Node *ptr = 
                           pHandle->DLIMP_dependency_stack.bottom_ptr;
   if (ptr && (ptr->value != dyn_module)) return local_file_handle;

   if (is_dsbt_module(dyn_module))
   {
       /*--------------------------------------------------------------------*/
       /* Assign DSBT indices.                                               */
       /*--------------------------------------------------------------------*/
       DLIF_assign_dsbt_indices();

       /*--------------------------------------------------------------------*/
       /* Update the content of all DSBTs for any module that uses the       */
       /* DSBT model.                                                        */
       /*--------------------------------------------------------------------*/
       DLIF_update_all_dsbts();
   }

   /*------------------------------------------------------------------------*/
   /* Ok, we are ready to process relocations. The relocation tables         */
   /* associated with dependent files will be processed first. Consume       */
   /* dynamic module objects from the dependency graph stack from dependents */
   /* to the root of the dependency graph.                                   */
   /*------------------------------------------------------------------------*/
   while (pHandle->DLIMP_dependency_stack.size > 0)
   {
      DLIMP_Dynamic_Module *dyn_mod_ptr =
                    dynamic_module_ptr_pop(&pHandle->DLIMP_dependency_stack);

      /*---------------------------------------------------------------------*/
      /* Process dynamic relocations associated with this module.            */
      /*---------------------------------------------------------------------*/
      process_dynamic_module_relocations(handle, dyn_mod_ptr->fd, dyn_mod_ptr);

      /*---------------------------------------------------------------------*/
      /* __c_args__ points to the beginning of the .args section, if there   */
      /* is one.  Record this pointer in the ELF file internal data object.  */
      /* Also store this in the loaded module, since this will be needed to  */
      /* write argv, argc to .args at execution time.                        */
      /*---------------------------------------------------------------------*/
      DLSYM_lookup_local_symtab("__c_args__", dyn_mod_ptr->symtab,
                                dyn_mod_ptr->symnum,
                                &dyn_mod_ptr->c_args,
                                dyn_mod_ptr->strtab);
      dyn_mod_ptr->loaded_module->c_args = dyn_mod_ptr->c_args;

      /*---------------------------------------------------------------------*/
      /* Pick up entry point address from ELF file header.                   */
      /*   We currently only support a single entry point into the ELF file. */
      /*   To support Braveheart notion of nodes, with multiple entry points,*/
      /*   we'll need to get the list of entry points associated with a node,*/
      /*   then add capability to the "execute" command to select the entry  */
      /*   point that we want to start executing from.                       */
      /*---------------------------------------------------------------------*/
      dyn_mod_ptr->loaded_module->entry_point = dyn_mod_ptr->fhdr.e_entry;

      /*---------------------------------------------------------------------*/
      /* Copy command-line arguments into args section and deal with DSBT    */
      /* issues (copy DSBT to its run location).                             */
      /* Note that below function is commented out because this doesn't do   */
      /* much as of now.                                                     */
      /*---------------------------------------------------------------------*/
      //load_object(dyn_mod_ptr->fd, dyn_mod_ptr); 

      /*---------------------------------------------------------------------*/
      /* Perform initialization, if needed, for this module.                 */
      /*---------------------------------------------------------------------*/
      store_init_data(dyn_mod_ptr);

      /*---------------------------------------------------------------------*/
      /* Free all dependent file pointers.                                   */
      /*---------------------------------------------------------------------*/
      if (dyn_mod_ptr->fd != fd)
      {
         DLIF_fclose(dyn_mod_ptr->fd);
         dyn_mod_ptr->fd = NULL;
      }

      /*---------------------------------------------------------------------*/
      /* Detach loaded module object from the dynamic module object that     */
      /* created it, then throw away the dynamic module object.              */
      /*---------------------------------------------------------------------*/
      detach_loaded_module(dyn_mod_ptr);
      delete_DLIMP_Dynamic_Module(handle, &dyn_mod_ptr);
   }

   return local_file_handle;
}

/*****************************************************************************/
/* DLOAD_load()                                                              */
/*                                                                           */
/*    Dynamically load the specified file and return a file handle for the   */
/*    loaded file.  If the load fails, this function will return a value of  */
/*    zero (0) for the file handle.                                          */
/*                                                                           */
/*    The core loader must have read access to the file pointed to by fd.    */
/*                                                                           */
/*****************************************************************************/
int32_t DLOAD_load(DLOAD_HANDLE handle, LOADER_FILE_DESC *fd)
{
    int32_t fl_handle;
 
    LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;
    DLIMP_Dynamic_Module *dyn_module = new_DLIMP_Dynamic_Module(fd);

    if (!dyn_module)
        return 0;

#if LOADER_DEBUG
   /*------------------------------------------------------------------------*/
   /* Spit out some loader progress information when we begin loading an     */
   /* object.                                                                */
   /*------------------------------------------------------------------------*/
   if (debugging_on) DLIF_trace("Loading file...\n");
#endif

   /*------------------------------------------------------------------------*/
   /* If no access to a program was provided, there is nothing to do.        */
   /*------------------------------------------------------------------------*/
   if (!fd)
   {
      DLIF_error(DLET_FILE, "Missing file specification.\n");
      delete_DLIMP_Dynamic_Module(handle, &dyn_module);
      return 0;
   }

   /*------------------------------------------------------------------------*/
   /* Read file headers and dynamic information into dynamic module.         */
   /*------------------------------------------------------------------------*/
   if (!dload_headers(fd, dyn_module))
   {
      delete_DLIMP_Dynamic_Module(handle, &dyn_module);
      return 0;
   }

   /*------------------------------------------------------------------------*/
   /* Find the dynamic segment, if there is one, and read dynamic            */
   /* information from the ELF object file into the dynamic module data      */
   /* structure associated with this file.                                   */
   /*------------------------------------------------------------------------*/
   if (!dload_dynamic_segment(handle, fd, dyn_module))
   {
      delete_DLIMP_Dynamic_Module(handle, &dyn_module);
      return 0;
   }

   /*------------------------------------------------------------------------*/
   /* Perform sanity checking on the read-in ELF file.                       */
   /*------------------------------------------------------------------------*/
   if (!is_valid_elf_object_file(fd, dyn_module))
   {
      DLIF_error(DLET_FILE, "Attempt to load invalid ELF file, '%s'.\n",
                    dyn_module->name);
      delete_DLIMP_Dynamic_Module(handle, &dyn_module);
      return 0;
   }

#if LOADER_DEBUG || LOADER_PROFILE
   /*------------------------------------------------------------------------*/
   /* Stop clock on initialization of ELF file information.  Start clock on  */
   /* initialization of ELF module.                                          */
   /*------------------------------------------------------------------------*/
   if (debugging_on || profiling_on)
   {
      DLIF_trace("Finished dload_dynamic_segment.\n");
      if (profiling_on)
      {
         profile_stop_clock();
         DLIF_trace("Took %lu cycles.\n", 
	            (unsigned long) profile_cycle_count());
      }
   }
#endif

   /*------------------------------------------------------------------------*/
   /* Initialize internal ELF module and segment structures.  Sets           */
   /* loaded_module in *dyn_module.  This also deals with assigning a file   */
   /* handle and bumping file handle counter.                                */
   /*------------------------------------------------------------------------*/
   initialize_loaded_module(handle, dyn_module);

   /*------------------------------------------------------------------------*/
   /* Append Module structure to loaded object list.                         */
   /*------------------------------------------------------------------------*/
   loaded_module_ptr_enqueue(&pHandle->DLIMP_loaded_objects, 
                             dyn_module->loaded_module);

   /*------------------------------------------------------------------------*/
   /* Support static loading as special case.                                */
   /*------------------------------------------------------------------------*/
   if (!dyn_module->relocatable)
      return dload_static_executable(handle, fd, dyn_module);

   /*------------------------------------------------------------------------*/
   /* Get space & address for segments, and offset symbols and program       */
   /* header table to reflect the relocated address.  Also offset the        */
   /* addresses in the internal Segment structures used by the Module        */
   /* structure.  Note that this step needs to be performed prior and in     */
   /* addition to the relocation entry processing.                           */
   /*------------------------------------------------------------------------*/
   if (!allocate_dynamic_segments_and_relocate_symbols(handle, fd, dyn_module))
   {
      delete_DLIMP_Dynamic_Module(handle, &dyn_module);
      return 0;
   }

   /*------------------------------------------------------------------------*/
   /* __c_args__ points to the beginning of the .args section, if there is   */
   /* one.  __TI_STATIC_BASE points to the beginning of the DP-relative data */
   /* segment (value to initialize DP). Record these addresses in the ELF    */
   /* file internal data object.                                             */
   /*------------------------------------------------------------------------*/
   DLSYM_lookup_local_symtab("__c_args__", dyn_module->symtab, 
                             dyn_module->symnum, 
                             &dyn_module->c_args,
                             dyn_module->strtab);

   DLSYM_lookup_local_symtab("__TI_STATIC_BASE", dyn_module->symtab, 
                             dyn_module->symnum, 
                             &dyn_module->static_base,
                             dyn_module->strtab);
   dyn_module->loaded_module->static_base = dyn_module->static_base;

   /*------------------------------------------------------------------------*/
   /* If the user application performs initialization and termination,       */
   /* the dynamic loader shouldn't process the init/fini sections.           */
   /* Check and adjust the init/fini information accordingly.                */
   /*------------------------------------------------------------------------*/
   adjust_module_init_fini(dyn_module);

   /*------------------------------------------------------------------------*/
   /* Execute any user defined pre-initialization functions that may be      */
   /* associated with a dynamic executable module.                           */
   /*------------------------------------------------------------------------*/
   if (dyn_module->fhdr.e_type == ET_EXEC)
      store_preinit_data(dyn_module);

   /*------------------------------------------------------------------------*/
   /* Append current ELF file to list of objects currently loading.          */
   /* This is used to detect circular dependencies while we are processing   */
   /* the dependents of this file.                                           */
   /*------------------------------------------------------------------------*/
   AL_append(&pHandle->DLIMP_module_dependency_list, &dyn_module->name);

   /*------------------------------------------------------------------------*/
   /* Push this dynamic module object onto the dependency stack.             */
   /* All of the modules on the stack will get relocated after all of the    */
   /* dependent files have been loaded and allocated.                        */
   /*------------------------------------------------------------------------*/
   dynamic_module_ptr_push(&pHandle->DLIMP_dependency_stack, dyn_module);

   /*------------------------------------------------------------------------*/
   /* If this object file uses the DSBT model, then register a DSBT index    */
   /* request with the client's DSBT support management.                     */
   /*------------------------------------------------------------------------*/
   if (is_dsbt_module(dyn_module) &&
       !DLIF_register_dsbt_index_request(handle, 
                                         dyn_module->name,
                                         dyn_module->loaded_module->file_handle,
                                         dyn_module->dsbt_index))
   {
      delete_DLIMP_Dynamic_Module(handle, &dyn_module);
      return 0;
   }

   /*------------------------------------------------------------------------*/
   /* Load this ELF file's dependees (all files on its DT_NEEDED list).      */
   /* Dependees must be loaded and relocated before processing this module's */
   /* relocations.                                                           */
   /*------------------------------------------------------------------------*/
   if (!dload_and_allocate_dependencies(handle, dyn_module))
   {
      delete_DLIMP_Dynamic_Module(handle, &dyn_module);
      return 0;
   }

   /*------------------------------------------------------------------------*/
   /* Remove the current ELF file from the list of files that are in the     */
   /* process of loading.                                                    */
   /*------------------------------------------------------------------------*/
   pHandle->DLIMP_module_dependency_list.size--;

   /*------------------------------------------------------------------------*/
   /* Process relocation entries.                                            */
   /*------------------------------------------------------------------------*/
   fl_handle = relocate_dependency_graph_modules(handle, fd, dyn_module);

   /*------------------------------------------------------------------------*/
   /* With initialization complete, and all relocations having been resolved */
   /* do module initialization.                                              */
   /*------------------------------------------------------------------------*/
   execute_module_initialization(handle);
  
   return fl_handle;
}

/*****************************************************************************/
/* DLOAD_get_entry_names()                                                   */
/*                                                                           */
/*    Build a list of entry point names for a loaded object.  Currently,     */
/*    any global symbol in the module is considered a valid entry point      */
/*    regardless of whether it is defined in code or associated with a       */
/*    data object.  We would need to process the content of the symbol       */
/*    table entry or its debug information to determine whether it is a      */
/*    valid entry point or not.                                              */
/*                                                                           */
/*****************************************************************************/
BOOL DLOAD_get_entry_names(DLOAD_HANDLE handle, 
                           uint32_t file_handle, 
                           int32_t *entry_pt_cnt, 
                           char ***entry_pt_names)
{
   /*------------------------------------------------------------------------*/
   /* Spin through list of loaded files until we find the file handle we     */
   /* are looking for.  Then build a list of entry points from that file's   */
   /* symbol table.                                                          */
   /*------------------------------------------------------------------------*/
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   loaded_module_ptr_Queue_Node* ptr;
   for (ptr = pHandle->DLIMP_loaded_objects.front_ptr; ptr != NULL; 
                                                          ptr = ptr->next_ptr)
   {
      if (ptr->value->file_handle == file_handle)
      {
         DLIMP_Loaded_Module *module = ptr->value;
         Elf_Sym *symtab = module->gsymtab;
	 const char *strtab = (const char *)module->gstrtab;
         int i;
              
         /*------------------------------------------------------------------*/
         /* Any symbol in our file's symbol table is considered a valid      */
         /* entry point.                                                     */
         /*------------------------------------------------------------------*/
         *entry_pt_cnt = module->gsymnum;
         *entry_pt_names = DLIF_malloc(*entry_pt_cnt * sizeof(char*));
         for (i = 0; i < module->gsymnum; i++)
         {
            const char *sym_name = strtab + symtab[i].st_name;
            **entry_pt_names = DLIF_malloc(strlen(sym_name) + 1);
            strcpy(**entry_pt_names, sym_name);
         }

         return TRUE;
      }
   }

   /*------------------------------------------------------------------------*/
   /* We didn't find the file we were looking for, return false.             */
   /*------------------------------------------------------------------------*/
   return FALSE;
}

/*****************************************************************************/
/* DLOAD_prepare_for_execution()                                             */
/*                                                                           */
/*    Given a file handle, prepare for execution :                           */
/*     - Return entry point associated with that module in the *sym_val      */
/*       output parameter.                                                   */
/*     - Write out the given arguments to the .args section contained in the */
/*       same module.                                                        */
/*     - As a test (for the Reference implementation) read the arguments     */
/*       using the DLIF_read_arguments() function and set global argc,argv.  */
/*                                                                           */
/*****************************************************************************/
BOOL DLOAD_prepare_for_execution(DLOAD_HANDLE handle, uint32_t file_handle, 
                                 TARGET_ADDRESS *sym_val, 
                                 int argc, char** argv)
{
   /*------------------------------------------------------------------------*/
   /* Spin through list of loaded files until we find the file handle we     */
   /* are looking for.  Then return the entry point address associated with  */
   /* that module.                                                           */
   /*------------------------------------------------------------------------*/
   DLIMP_Loaded_Module *ep_loaded_module;
   loaded_module_ptr_Queue_Node* ptr;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   for (ptr = pHandle->DLIMP_loaded_objects.front_ptr; ptr != NULL; 
                                                       ptr = ptr->next_ptr)
      if (ptr->value->file_handle == file_handle)
      {
         *sym_val = (TARGET_ADDRESS)(ptr->value->entry_point);
         ep_loaded_module = ptr->value;

         /*------------------------------------------------------------------*/
         /* Write argc, argv to the .args section in this module.            */
         /*------------------------------------------------------------------*/
         if (!write_arguments_to_args_section(handle, argc, argv, 
	                                      ep_loaded_module))
         {
            DLIF_error(DLET_MISC, "Couldn't write to .args section\n");
            return FALSE;
         }

         /*------------------------------------------------------------------*/
         /* For the Reference Implementation we simulate a "boot" (rts boot  */
         /* routine reads argc, argv from .args), by reading argc, argv from */
         /* .args section. Note that we just wrote these values to the .args */
         /* so this read serves as a test for the Reference Implementation.  */
         /*------------------------------------------------------------------*/
         read_args_from_section(ep_loaded_module);
         return TRUE;
      }

   /*------------------------------------------------------------------------*/
   /* We didn't find the file we were looking for, return false.             */
   /*------------------------------------------------------------------------*/
   return FALSE;
}

/*****************************************************************************/
/* DLOAD_load_arguments()                                                    */
/*                                                                           */
/*    Write out the given arguments to the .args section contained in the    */
/*    same module.                                                           */
/*                                                                           */
/*****************************************************************************/
BOOL DLOAD_load_arguments(DLOAD_HANDLE handle, uint32_t file_handle, 
                           int argc, char** argv)
{
   /*------------------------------------------------------------------------*/
   /* Spin through list of loaded files until we find the file handle we     */
   /* are looking for.  Then return the entry point address associated with  */
   /* that module.                                                           */
   /*------------------------------------------------------------------------*/
   DLIMP_Loaded_Module *ep_loaded_module;
   loaded_module_ptr_Queue_Node* ptr;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   for (ptr = pHandle->DLIMP_loaded_objects.front_ptr; ptr != NULL; 
                                                       ptr = ptr->next_ptr)
      if (ptr->value->file_handle == file_handle)
      {
         ep_loaded_module = ptr->value;

         /*------------------------------------------------------------------*/
         /* Write argc, argv to the .args section in this module.            */
         /*------------------------------------------------------------------*/
         if (!write_arguments_to_args_section(handle, argc, argv, 
	                                      ep_loaded_module))
         {
            DLIF_error(DLET_MISC, "Couldn't write to .args section\n");
            return FALSE;
         }
      }

   /*------------------------------------------------------------------------*/
   /* We didn't find the file we were looking for, return false.             */
   /*------------------------------------------------------------------------*/
   return FALSE;
}

/*****************************************************************************/
/* DLOAD_get_entry_point()                                                   */
/*                                                                           */
/*    Given a file handle, return the entry point associated with that       */
/*    module in the *sym_val output parameter.                               */
/*                                                                           */
/*****************************************************************************/
BOOL DLOAD_get_entry_point(DLOAD_HANDLE handle, uint32_t file_handle, 
                           TARGET_ADDRESS *sym_val) 
{
   /*------------------------------------------------------------------------*/
   /* Spin through list of loaded files until we find the file handle we     */
   /* are looking for.  Then return the entry point address associated with  */
   /* that module.                                                           */
   /*------------------------------------------------------------------------*/
   loaded_module_ptr_Queue_Node* ptr;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   for (ptr = pHandle->DLIMP_loaded_objects.front_ptr; ptr != NULL; 
                                                       ptr = ptr->next_ptr)
      if (ptr->value->file_handle == file_handle)
      {
         *sym_val = (TARGET_ADDRESS)(ptr->value->entry_point);
         return TRUE;
      }

   /*------------------------------------------------------------------------*/
   /* We didn't find the file we were looking for, return false.             */
   /*------------------------------------------------------------------------*/
   return FALSE;
}

/*****************************************************************************/
/* DLOAD_query_symbol()                                                      */
/*                                                                           */
/*    Query the value of a global symbol from a specific file.  The value    */
/*    result will be written to *sym_val.  The function returns TRUE if the  */
/*    symbol was found, and FALSE if it wasn't.                              */
/*                                                                           */
/*****************************************************************************/
BOOL DLOAD_query_symbol(DLOAD_HANDLE handle,
                        uint32_t file_handle, 
                        const char *sym_name, 
                        TARGET_ADDRESS *sym_val)
{
   /*------------------------------------------------------------------------*/
   /* Spin through list of loaded files until we find the file handle we     */
   /* are looking for.  Then return the value (target address) associated    */
   /* with the symbol we are looking for in that file.                       */
   /*------------------------------------------------------------------------*/
   loaded_module_ptr_Queue_Node* ptr;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   for (ptr = pHandle->DLIMP_loaded_objects.front_ptr; ptr != NULL; 
                                                          ptr = ptr->next_ptr)
   {
      if (ptr->value->file_handle == file_handle)
      {
         DLIMP_Loaded_Module *module = ptr->value;
         Elf_Sym *symtab = module->gsymtab;
	 const char *strtab = (const char *)module->gstrtab;
         int i;
              
         /*------------------------------------------------------------------*/
         /* Search through the symbol table by name.                         */
         /*------------------------------------------------------------------*/
         for(i=0; i < module->gsymnum; i++)
         {
            if (!strcmp(sym_name, strtab + symtab[i].st_name))
            {
               *sym_val = (TARGET_ADDRESS) symtab[i].st_value;
               return TRUE;
            }
         }
      }
   }

   /*------------------------------------------------------------------------*/
   /* We didn't find the symbol we were looking for, return false.           */
   /*------------------------------------------------------------------------*/
   return FALSE;
}

/*****************************************************************************/
/* DLOAD_update_symbol()                                                     */
/*                                                                           */
/*    Update the value of a global symbol from a specific file.              */
/*    The function returns TRUE if the symbol was found and updated,         */
/*    and FALSE if it wasn't.                                                */
/*                                                                           */
/*****************************************************************************/
BOOL DLOAD_update_symbol(DLOAD_HANDLE handle,
                        uint32_t file_handle,
                        const char *sym_name,
                        TARGET_ADDRESS sym_val)
{
   /*------------------------------------------------------------------------*/
   /* Spin through list of loaded files until we find the file handle we     */
   /* are looking for.  Then update the value (target address) associated    */
   /* with the symbol we are looking for in that file.                       */
   /*------------------------------------------------------------------------*/
   loaded_module_ptr_Queue_Node* ptr;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   for (ptr = pHandle->DLIMP_loaded_objects.front_ptr; ptr != NULL;
                                                          ptr = ptr->next_ptr)
   {
      if (ptr->value->file_handle == file_handle)
      {
         DLIMP_Loaded_Module *module = ptr->value;
         Elf_Sym *symtab = module->gsymtab;
	 const char *strtab = (const char *)module->gstrtab;
         int i;

         /*------------------------------------------------------------------*/
         /* Search through the symbol table by name.                         */
         /*------------------------------------------------------------------*/
         for(i=0; i < module->gsymnum; i++)
         {
            if (!strcmp(sym_name, strtab + symtab[i].st_name))
            {
               symtab[i].st_value = sym_val;
               return TRUE;
            }
         }
      }
   }

   /*------------------------------------------------------------------------*/
   /* We didn't find the symbol we were looking for, return false.           */
   /*------------------------------------------------------------------------*/
   return FALSE;
}

/*****************************************************************************/
/* unlink_loaded_module()                                                    */
/*                                                                           */
/*    Unlink a loaded module data object from the list of loaded objects,    */
/*    returning a pointer to the object so that it can be deconstructed.     */
/*                                                                           */
/*****************************************************************************/
static DLIMP_Loaded_Module *unlink_loaded_module(DLOAD_HANDLE handle,
                                       loaded_module_ptr_Queue_Node *back_ptr,
                                       loaded_module_ptr_Queue_Node *lm_node)
{
    DLIMP_Loaded_Module *loaded_module = lm_node->value;
    LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;
    loaded_module_ptr_remove(&pHandle->DLIMP_loaded_objects, lm_node->value);
    return loaded_module;
}

/*****************************************************************************/
/* execute_module_termination()                                              */
/*                                                                           */
/*    Execute termination functions associated with this loaded module.      */
/*    Termination functions are called in the reverse order as their         */
/*    corresponding initialization functions.                                */
/*                                                                           */
/*****************************************************************************/
static void execute_module_termination(DLOAD_HANDLE handle,
                                       DLIMP_Loaded_Module *loaded_module)
{
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   /*------------------------------------------------------------------------*/
   /* If a DT_FINI_ARRAY dynamic tag was encountered for this module, spin   */
   /* through the array in reverse order, calling each function address      */
   /* stored in the array.                                                   */
   /*------------------------------------------------------------------------*/
   if (loaded_module->fini_arraysz != 0)
   {
      /*---------------------------------------------------------------------*/
      /* Now make a loader-accessible copy of the .fini_array section.       */
      /*---------------------------------------------------------------------*/
      int32_t i;
      int32_t num_fini_fcns = 
                    loaded_module->fini_arraysz/sizeof(TARGET_ADDRESS);
      TARGET_ADDRESS *fini_array_buf = (TARGET_ADDRESS *)
                                      DLIF_malloc(loaded_module->fini_arraysz);

      DLIF_read(pHandle->client_handle,
                fini_array_buf, 1, loaded_module->fini_arraysz, 
                (TARGET_ADDRESS)loaded_module->fini_array);

      /*---------------------------------------------------------------------*/
      /* Now spin through the array in reverse order, executing each         */
      /* termination function whose address occupies an entry in the array.  */
      /*---------------------------------------------------------------------*/
      for (i = num_fini_fcns - 1; i >= 0; i--)
            DLIF_execute(pHandle->client_handle, 
	                 (TARGET_ADDRESS)(fini_array_buf[i]));

      DLIF_free(fini_array_buf);
   }

   /*------------------------------------------------------------------------*/
   /* If a DT_FINI dynamic tag was encountered for this module, call the     */
   /* function indicated by the tag's value to complete the termination      */
   /* process for this module.                                               */
   /*------------------------------------------------------------------------*/
   if (loaded_module->fini != 0)
         DLIF_execute(pHandle->client_handle, loaded_module->fini);
}

/*****************************************************************************/
/* remove_loaded_module()                                                    */
/*                                                                           */
/*    Find and unlink a loaded module data object from the list of loaded    */
/*    objects, then call its destructor to free the host memory associated   */
/*    with the loaded module and all of its loaded segments.                 */
/*                                                                           */
/*****************************************************************************/
static void remove_loaded_module(DLOAD_HANDLE handle,
                                 loaded_module_ptr_Queue_Node *lm_node)
{
    DLIMP_Loaded_Module *lm_object = NULL;
    loaded_module_ptr_Queue_Node *back_ptr = NULL;
    LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

    if (lm_node != pHandle->DLIMP_loaded_objects.front_ptr)
       for (back_ptr = pHandle->DLIMP_loaded_objects.front_ptr;
            back_ptr->next_ptr != lm_node;
            back_ptr = back_ptr->next_ptr);

    lm_object = unlink_loaded_module(handle, back_ptr, lm_node);

    delete_DLIMP_Loaded_Module(handle, &lm_object);
}

/*****************************************************************************/
/* DLOAD_unload()                                                            */
/*                                                                           */
/*    Unload specified module (identified by its file handle) from target    */
/*    memory.  Free up any target memory that was allocated for the module's */
/*    segments and also any host heap memory that was allocated for the      */
/*    internal module and segment data structures.                           */
/*                                                                           */
/*    Return TRUE if program entry is actually destroyed.  This is a way of  */
/*    communicating to the client when it needs to actually remove debug     */
/*    information associated with this module (so that client does not have  */
/*    to maintain a use count that mirrors the program entry).               */
/*                                                                           */
/*****************************************************************************/
BOOL DLOAD_unload(DLOAD_HANDLE handle, uint32_t file_handle)
{
   loaded_module_ptr_Queue_Node* lm_node;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   for (lm_node = pHandle->DLIMP_loaded_objects.front_ptr; lm_node != NULL;
        lm_node = lm_node->next_ptr)
   {
      if (lm_node->value->file_handle == file_handle)
      {
         --lm_node->value->use_count;
         if (lm_node->value->use_count == 0)
         {
	    DLIMP_Loaded_Module *loaded_module = 
	                                 (DLIMP_Loaded_Module *)lm_node->value;
            int j;
            int *dep_file_handles;

            /*---------------------------------------------------------------*/
	    /* Termination functions need to be executed in the reverse      */
	    /* order as the corresponding initialization functions, so       */
	    /* before we go unload this module's dependents, we need to      */
	    /* perform the user/global/static termination functions          */
	    /* associated with this module.                                  */
            /*---------------------------------------------------------------*/
	    execute_module_termination(handle, loaded_module);

            /*---------------------------------------------------------------*/
	    /* Unload dependent modules via the client. Client needs to know */
	    /* when a dependent gets unloaded so that it can update debug    */
	    /* information.                                                  */
            /*---------------------------------------------------------------*/
            dep_file_handles = (int*)(loaded_module->dependencies.buf);
            for (j = 0; j < loaded_module->dependencies.size; j++)
               DLIF_unload_dependent(pHandle->client_handle, 
                                                  dep_file_handles[j]);

            /*---------------------------------------------------------------*/
            /* Find the predecessor node of the value we're deleting,        */
            /* because its next_ptr will need to be updated.                 */
            /*                                                               */
            /* We can't keep a back pointer around because                   */
            /* DLIF_unload_dependent() might free that node, making our      */
            /* pointer invalid.  Turn the Queue template into a doubly       */
            /* linked list if this overhead becomes a problem.               */
            /*---------------------------------------------------------------*/
            remove_loaded_module(handle, lm_node);

            /*---------------------------------------------------------------*/
            /* Once unloading is done, reset virtual target to NULL.         */
            /*---------------------------------------------------------------*/
            cur_target = NULL;
            
            return TRUE;
         }
      }
   }

   return FALSE;
}

/*****************************************************************************/
/* DLOAD_load_symbols()                                                      */
/*                                                                           */
/*    Load the symbols from the given file and make symbols available for    */
/*    global symbol linkage.                                                 */
/*                                                                           */
/*****************************************************************************/
int32_t DLOAD_load_symbols(DLOAD_HANDLE handle, LOADER_FILE_DESC *fd)
{
   DLIMP_Dynamic_Module *dyn_module = new_DLIMP_Dynamic_Module(fd);
   DLIMP_Loaded_Module *loaded_module = NULL;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   /*------------------------------------------------------------------------*/
   /* Ensure we have a valid dynamic module object from the constructor.     */
   /*------------------------------------------------------------------------*/
   if (!dyn_module)
       return 0;

   /*------------------------------------------------------------------------*/
   /* If no access to a program was provided, there is nothing to do.        */
   /*------------------------------------------------------------------------*/
   if (!fd)
   {
      delete_DLIMP_Dynamic_Module(handle, &dyn_module);
      DLIF_error(DLET_FILE, "Missing file specification.\n");
      return 0;
   }

   /*------------------------------------------------------------------------*/
   /* Record argc and argv pointers with the dynamic module record.          */
   /*------------------------------------------------------------------------*/
   dyn_module->argc = 0;
   dyn_module->argv = NULL;

   /*------------------------------------------------------------------------*/
   /* Read file headers and dynamic information into dynamic module.         */
   /*------------------------------------------------------------------------*/
   if (!dload_headers(fd, dyn_module))
   {
      delete_DLIMP_Dynamic_Module(handle, &dyn_module);
      return 0;
   }

   /*------------------------------------------------------------------------*/
   /* Find the dynamic segment, if there is one, and read dynamic            */
   /* information from the ELF object file into the dynamic module data      */
   /* structure associated with this file.                                   */
   /*------------------------------------------------------------------------*/
   if (!dload_dynamic_segment(handle, fd, dyn_module))
   {
      delete_DLIMP_Dynamic_Module(handle, &dyn_module);
      return 0;
   }

   /*------------------------------------------------------------------------*/
   /* Perform sanity checking on the read-in ELF file.                       */
   /*------------------------------------------------------------------------*/
   if (!is_valid_elf_object_file(fd, dyn_module))
   {
      DLIF_error(DLET_FILE, "Attempt to load invalid ELF file, '%s'.\n",
                    dyn_module->name);
      delete_DLIMP_Dynamic_Module(handle, &dyn_module);
      return 0;
   }

   /*------------------------------------------------------------------------*/
   /* Initialize internal ELF module and segment structures.  Sets           */
   /* loaded_module in *dyn_module.  This also deals with assigning a file   */
   /* handle and bumping file handle counter.                                */
   /*------------------------------------------------------------------------*/
   initialize_loaded_module(handle, dyn_module);

   /*------------------------------------------------------------------------*/
   /* Add this module to the loaded module queue.                            */
   /* Detach the loaded module object from the dynamic module thath created  */
   /* it. Ownership of the host memory allocated for the loaded module       */
   /* object now belongs to the DLIMP_loaded_objects list.                   */
   /*------------------------------------------------------------------------*/
   loaded_module_ptr_enqueue(&pHandle->DLIMP_loaded_objects, 
                             dyn_module->loaded_module);

   /*------------------------------------------------------------------------*/
   /* Register a DSBT index request for this module and update its own copy  */
   /* of the DSBT with the contents of the client's master DSBT.             */
   /*------------------------------------------------------------------------*/
   if (is_dsbt_module(dyn_module))
   {
      dynamic_module_ptr_push(&pHandle->DLIMP_dependency_stack, dyn_module);
      DLIF_register_dsbt_index_request(handle,
                                       dyn_module->name,
                                       dyn_module->loaded_module->file_handle,
                                       dyn_module->dsbt_index);
      DLIF_assign_dsbt_indices();
      DLIF_update_all_dsbts();
      dynamic_module_ptr_pop(&pHandle->DLIMP_dependency_stack);
   }
   
   /*------------------------------------------------------------------------*/
   /* Ownership of the host memory allocated for the loaded module object is */
   /* transferred to the DLIMP_loaded_objects list. Free up the host memory  */
   /* for the dynamic module that created the loaded module object. Just     */
   /* call the destructor function for DLIMP_Dynamic_Module.                 */
   /*------------------------------------------------------------------------*/
   loaded_module = detach_loaded_module(dyn_module);
   if(loaded_module == NULL)
   {
       delete_DLIMP_Dynamic_Module(handle, &dyn_module);
       return 0;
   }
   delete_DLIMP_Dynamic_Module(handle, &dyn_module);

   /*------------------------------------------------------------------------*/
   /* Return a file handle so that the client can match this file to an ID.  */
   /*------------------------------------------------------------------------*/
   return loaded_module->file_handle;
}

/*****************************************************************************/
/* DSBT Support Functions                                                    */
/*****************************************************************************/

/*****************************************************************************/
/* DLOAD_get_dsbt_size()                                                     */
/*                                                                           */
/*    Find the amount of space allocated for the specified module's DSBT.    */
/*    It must be big enough to hold a copy of the master DSBT or the client  */
/*    will flag an error. Those modules whose DSBT size is zero are assumed  */
/*    to not be using the DSBT model.                                        */
/*                                                                           */
/*****************************************************************************/
uint32_t DLOAD_get_dsbt_size(DLOAD_HANDLE handle, int32_t file_handle)
{
   dynamic_module_ptr_Stack_Node *ptr;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   for (ptr = pHandle->DLIMP_dependency_stack.top_ptr; ptr != NULL; 
                                                          ptr = ptr->next_ptr)
   {
      DLIMP_Dynamic_Module *dmp = ptr->value;
      if (dmp->loaded_module->file_handle == file_handle)
         return dmp->dsbt_size;
   }

   return 0;
}

/*****************************************************************************/
/* DLOAD_get_static_base()                                                   */
/*                                                                           */
/*    Look up static base symbol associated with the specified module.       */
/*                                                                           */
/*****************************************************************************/
BOOL DLOAD_get_static_base(DLOAD_HANDLE handle, int32_t file_handle, 
                           TARGET_ADDRESS *static_base)
{
   loaded_module_ptr_Queue_Node* ptr;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   for (ptr = pHandle->DLIMP_loaded_objects.front_ptr; ptr != NULL;
              ptr = ptr->next_ptr)
   {
      DLIMP_Loaded_Module *lmp = ptr->value;
      if (lmp->file_handle == file_handle)
      {
          *static_base = lmp->static_base;
          return TRUE;
      }
   }

   return FALSE;
}

/*****************************************************************************/
/* DLOAD_get_dsbt_base()                                                     */
/*                                                                           */
/*    Look up address of DSBT for the specified module.                      */
/*                                                                           */
/*****************************************************************************/
BOOL DLOAD_get_dsbt_base(DLOAD_HANDLE handle, int32_t file_handle, TARGET_ADDRESS *dsbt_base)
{
   dynamic_module_ptr_Stack_Node *ptr;
   LOADER_OBJECT *pHandle = (LOADER_OBJECT *)handle;

   for (ptr = pHandle->DLIMP_dependency_stack.top_ptr; ptr != NULL; 
                                                          ptr = ptr->next_ptr)
   {
      DLIMP_Dynamic_Module *dmp = ptr->value;
      if (dmp->loaded_module->file_handle == file_handle)
      {
         *dsbt_base = 
	         (TARGET_ADDRESS)dmp->dyntab[dmp->dsbt_base_tagidx].d_un.d_ptr;
         return TRUE;
      }
   }

   return FALSE;
}

/*****************************************************************************/
/* RELOCATE() - Perform RELA and REL type relocations for given ELF object   */
/*      file that we are in the process of loading and relocating.           */
/*****************************************************************************/
void DLREL_relocate(DLOAD_HANDLE handle, LOADER_FILE_DESC* elf_file,
                    DLIMP_Dynamic_Module* dyn_module)

{
   cur_target->relocate(handle, elf_file, dyn_module);
}

/*****************************************************************************/
/* GET_VT_OBJ() - Once file headers have been read, use the e_machine id to  */
/*        figure out the virtul target, so we can access trg specific funcs. */
/*****************************************************************************/
static VIRTUAL_TARGET *get_vt_obj(int given_id)
{
   VIRTUAL_TARGET *ptr;

   for(ptr = vt_arr; ptr->machine_id != EM_NONE ; ptr++)
      if (ptr->machine_id == given_id) return ptr;

   return NULL;
}

#if 0 && LOADER_DEBUG   // enable to make available in debugger
/*****************************************************************************/
/* DEBUG_QUEUE() - Debug function.                                           */
/*****************************************************************************/
static void debug_queue(LOADER_OBJECT *pHandle, char* position)
{
   loaded_module_ptr_Queue_Node* ptr;

   if (!debugging_on) return;

   DLIF_trace ("\nDEBUG QUEUE : %s, pHandle : 0x%x\n\n", position, 
           (uint32_t)pHandle);

   for (ptr = pHandle->DLIMP_loaded_objects.front_ptr; ptr != NULL;
              ptr = ptr->next_ptr)
   {
      DLIF_trace ("ptr->value->name : %s\n",ptr->value->name);
   }
   DLIF_trace ("\n");
}
#endif

/*****************************************************************************/
/* READ_ARGS_FROM_SECTION() - This function reads the argc, argv from the    */
/*         .args section, and is used to test Reference implementation.      */ 
/*****************************************************************************/
static void read_args_from_section(DLIMP_Loaded_Module* ep_module)
{
   /*------------------------------------------------------------------------*/
   /* Before this function in called, the loader has gotten argv/argc from   */
   /* the module and written it out to the .args section. c_args points to   */
   /* the .args section.                                                     */
   /*------------------------------------------------------------------------*/
   ARGS_CONTAINER *pargs = (ARGS_CONTAINER *)(ep_module->c_args);
   if (!pargs || pargs == (ARGS_CONTAINER *)0xFFFFFFFF) 
   {
      global_argc = 0;
      global_argv = NULL;
   }
   else
   {
      global_argc = pargs->argc;
      global_argv = pargs->argv;
   }
}
