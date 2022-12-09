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

/** \file dsp_load.c
    For integrating DLOAD into PSDK C7x firmware.  See dsp_load.h for details.
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "dload_api.h"
#include "dsp_load.h"
#include "elf32.h"

/** \brief Heap located in DDR */
#define APP_MEM_HEAP_DDR (0u)
/** \brief Heap located in L3 memory (MSMC) */
#define APP_MEM_HEAP_L3         (1u)
/** \brief Heap located in L2 local memory of a CPU */
#define APP_MEM_HEAP_L2         (2u)
/** \brief Heap located in L1 local memory of a CPU */
#define APP_MEM_HEAP_L1         (3u)
/** \brief Heap located in DDR and is used as scratch */
#define APP_MEM_HEAP_DDR_SCRATCH (4u)

/** \brief Max characters to use for heap name */
#define APP_MEM_HEAP_NAME_MAX   (16u)
/**
 * \brief Heap statistics and information
 */
typedef struct {
    uint32_t heap_id;                       /**< Heap ID, see \ref APP_MEM_HEAP */
    char heap_name[APP_MEM_HEAP_NAME_MAX];  /**< Heap name */
    uint32_t heap_size;                     /**< Heap size in bytes */
    uint32_t free_size;                     /**< Free space in bytes */
} app_mem_stats_t;


extern void *appMemAlloc(uint32_t heap_id, uint32_t size, uint32_t align);
extern int32_t appMemFree(uint32_t heap_id, void *ptr, uint32_t size);
extern void  appMemCacheWbInv(void *ptr, uint32_t size);
extern int32_t appMemStats(uint32_t heap_id, app_mem_stats_t *stats);

#define DYN_LOAD_MEM_DEFAULT_ALIGN 128
#define MAX_PTR_SIZE_MAP_SIZE  (1024U)
typedef struct {
  void *ptrs[MAX_PTR_SIZE_MAP_SIZE];
  int  sizes[MAX_PTR_SIZE_MAP_SIZE];
  int  size;
} AllocPtrSizeMap_t;
static AllocPtrSizeMap_t *dlif_alloc_size_map = NULL;

/** \brief client handle returned to external interface */
typedef struct {
  DLOAD_HANDLE  dload_handle;   /**< DLOAD handle */
  int           file_handle;    /**< DLOAD file handle */
  const char   *file_data;      /**< the dynamic exectuable file in memory */
  int           dsp_syms_size;  /**< number of imported firmware symbols */
  const char  **dsp_syms_names; /**< firmware symbol names */
  void        **dsp_syms_addrs; /**< firmware symbol REAL addresses */
  AllocPtrSizeMap_t *dlif_alloc_size_map; /**< DLOAD allocations */
} Dspload_Client_t;

static void dspload_free_all(AllocPtrSizeMap_t *alloc_size_map);

/**\brief error handling */
static jmp_buf dspload_jmpbuf;
static void    dspload_exit(int ecode);

int debugging_on = FALSE;
int profiling_on = FALSE;
#define LOADER_MEM_DEBUG 0

/************************ Below are dsp syms imported from C7x firmware  *****/
static const char *dsp_syms_names[] = {
"TIDL_VISION_FXNS",
"g_l1_mem_addr",
"g_l2_mem_addr",
"g_l3_mem_addr",
"g_l1_mem_size",
"g_l2_mem_size",
"g_l3_mem_size",

"printf",
"puts",
"vprintf",
"snprintf",
"vsnprintf",
"fputs",
"fflush",

"appMemAlloc",
"appMemFree",
"appUdmaGetObj",

"DmaUtilsAutoInc3d_configure",
"DmaUtilsAutoInc3d_convertTrVirtToPhyAddr",
"DmaUtilsAutoInc3d_deconfigure",
"DmaUtilsAutoInc3d_deinit",
"DmaUtilsAutoInc3d_getContextSize",
"DmaUtilsAutoInc3d_getTrMemReq",
"DmaUtilsAutoInc3d_init",
"DmaUtilsAutoInc3d_prepareTr",
"DmaUtilsAutoInc3d_trigger",
"DmaUtilsAutoInc3d_wait",

"TVM_lockInterrupts",
"TVM_unlockInterrupts",
"TVM_cacheWbInv",

};
static void *dsp_syms_addrs[sizeof(dsp_syms_names)/sizeof(char*)];
static void *g_l1_mem_addr;
static void *g_l2_mem_addr;
static void *g_l3_mem_addr;
static uint32_t g_l1_mem_size;
static uint32_t g_l2_mem_size;
static uint32_t g_l3_mem_size;

static void ** dspload_get_dsp_syms_addrs(void);

static void **
dspload_get_dsp_syms_addrs(void)
{
  /* CHECK_MISRA("-8.12")  -> Disable MISRA C 2004 rule 8.12  */
  /* g_l1/l2/l3_mem are declared externally.  The use is safe here because we get their */
  /*     addresses and query their sizes */
  extern uint8_t g_l1_mem[];
  extern uint8_t g_l2_mem[];
  extern uint8_t g_l3_mem[];
  /* RESET_MISRA("-8.12")  -> Reset MISRA C 2004 rule 8.12  */
  g_l1_mem_addr = g_l1_mem;
  g_l2_mem_addr = g_l2_mem;
  g_l3_mem_addr = g_l3_mem;

  app_mem_stats_t app_mem_stats;
  appMemStats(APP_MEM_HEAP_L1, &app_mem_stats);
  g_l1_mem_size = app_mem_stats.heap_size;
  appMemStats(APP_MEM_HEAP_L2, &app_mem_stats);
  g_l2_mem_size = app_mem_stats.heap_size;
  appMemStats(APP_MEM_HEAP_L3, &app_mem_stats);
  g_l3_mem_size = app_mem_stats.heap_size;

  /* for symbol address only, type does not matter */
  extern int TIDL_VISION_FXNS;
  extern void *appUdmaGetObj(void);
  extern void DmaUtilsAutoInc3d_configure(void);
  extern void DmaUtilsAutoInc3d_convertTrVirtToPhyAddr(void);
  extern void DmaUtilsAutoInc3d_deconfigure(void);
  extern void DmaUtilsAutoInc3d_deinit(void);
  extern void DmaUtilsAutoInc3d_getContextSize(void);
  extern void DmaUtilsAutoInc3d_getTrMemReq(void);
  extern void DmaUtilsAutoInc3d_init(void);
  extern void DmaUtilsAutoInc3d_prepareTr(void);
  extern void DmaUtilsAutoInc3d_trigger(void);
  extern void DmaUtilsAutoInc3d_wait(void);
  extern int32_t TVM_lockInterrupts(void);
  extern void    TVM_unlockInterrupts(void);
  extern void    TVM_cacheWbInv(void);

  int i = 0;
  dsp_syms_addrs[i++] = &TIDL_VISION_FXNS;
  dsp_syms_addrs[i++] = &g_l1_mem_addr;
  dsp_syms_addrs[i++] = &g_l2_mem_addr;
  dsp_syms_addrs[i++] = &g_l3_mem_addr;
  dsp_syms_addrs[i++] = &g_l1_mem_size;
  dsp_syms_addrs[i++] = &g_l2_mem_size;
  dsp_syms_addrs[i++] = &g_l3_mem_size;
  dsp_syms_addrs[i++] = &printf;
  dsp_syms_addrs[i++] = &puts;
  dsp_syms_addrs[i++] = &vprintf;
  dsp_syms_addrs[i++] = &snprintf;
  dsp_syms_addrs[i++] = &vsnprintf;
  dsp_syms_addrs[i++] = &fputs;
  dsp_syms_addrs[i++] = &fflush;
  dsp_syms_addrs[i++] = &appMemAlloc;
  dsp_syms_addrs[i++] = &appMemFree;
  dsp_syms_addrs[i++] = &appUdmaGetObj;
  dsp_syms_addrs[i++] = &DmaUtilsAutoInc3d_configure;
  dsp_syms_addrs[i++] = &DmaUtilsAutoInc3d_convertTrVirtToPhyAddr;
  dsp_syms_addrs[i++] = &DmaUtilsAutoInc3d_deconfigure;
  dsp_syms_addrs[i++] = &DmaUtilsAutoInc3d_deinit;
  dsp_syms_addrs[i++] = &DmaUtilsAutoInc3d_getContextSize;
  dsp_syms_addrs[i++] = &DmaUtilsAutoInc3d_getTrMemReq;
  dsp_syms_addrs[i++] = &DmaUtilsAutoInc3d_init;
  dsp_syms_addrs[i++] = &DmaUtilsAutoInc3d_prepareTr;
  dsp_syms_addrs[i++] = &DmaUtilsAutoInc3d_trigger;
  dsp_syms_addrs[i++] = &DmaUtilsAutoInc3d_wait;
  dsp_syms_addrs[i++] = &TVM_lockInterrupts;
  dsp_syms_addrs[i++] = &TVM_unlockInterrupts;
  dsp_syms_addrs[i++] = &TVM_cacheWbInv;

  return dsp_syms_addrs;
}

/************************ Below are dsp_load APIs        *********************/
/** \brief Load dsp dynamic executable
 */
void *
dspload_load_program(void *file_data, int file_size)
{
  Dspload_Client_t* dspload_client = (Dspload_Client_t*) appMemAlloc(
        APP_MEM_HEAP_DDR, sizeof(Dspload_Client_t), DYN_LOAD_MEM_DEFAULT_ALIGN);
  if (NULL == dspload_client)
  {
    printf("ERROR: dsp_load: fail to allocate Dspload_Client_t!\n");
  }
  else
  {
    dlif_alloc_size_map = NULL;

    /* CHECK_MISRA("-20.7")  -> Disable MISRA C 2004 rule 20.7  */
    /* setjmp is used to handle errors occuring in DLOAD and clean up gracefully.   */
    if (! setjmp(dspload_jmpbuf))
    /* RESET_MISRA("-20.7")  -> Reset MISRA C 2004 rule 20.7  */
    {
      dspload_client->dload_handle   = DLOAD_create(dspload_client);
      dspload_client->file_data      = file_data;
      dspload_client->dsp_syms_size  = sizeof(dsp_syms_names)/sizeof(char*);
      dspload_client->dsp_syms_names = dsp_syms_names;
      dspload_client->dsp_syms_addrs = dspload_get_dsp_syms_addrs();

      LOADER_FILE_DESC f;
      f.binary = (int8_t *) file_data;
      f.cur = f.orig = f.binary;
      f.length = f.size = file_size;
      f.read_size = 0;

      dspload_client->file_handle = DLOAD_load(dspload_client->dload_handle, &f);
    }
    else
    {
      dspload_client->file_handle = 0;
    }
    dspload_client->dlif_alloc_size_map = dlif_alloc_size_map;

    if (dspload_client->file_handle == 0)
    {
      dspload_free_all(dspload_client->dlif_alloc_size_map);
      appMemFree(APP_MEM_HEAP_DDR, dspload_client, sizeof(Dspload_Client_t));
      dspload_client = NULL;
    }
  }

  return (void *) dspload_client;
}

/** \brief Query symbol address
 */
void *
dspload_query_symbol(void *dspload_handle, const char *sym_name)
{
  void *sym_addr = NULL;
  if (dspload_handle != NULL)
  {
    Dspload_Client_t* dspload_client = (Dspload_Client_t*) dspload_handle;
    DLOAD_HANDLE dload_handle = dspload_client->dload_handle;
    DLOAD_query_symbol(dload_handle, dspload_client->file_handle, sym_name,
                       (TARGET_ADDRESS *) &sym_addr);
  }
  return sym_addr;
}

/** \brief Unload dsp dynamic executable
 *  dspload_handle becomes invalid afterwards
 */
void
dspload_unload_program(void *dspload_handle)
{
  Dspload_Client_t* dspload_client = (Dspload_Client_t*) dspload_handle;
  dlif_alloc_size_map = dspload_client->dlif_alloc_size_map;
  DLOAD_unload(dspload_client->dload_handle, dspload_client->file_handle);
  DLOAD_destroy(dspload_client->dload_handle);
  dspload_free_all(dspload_client->dlif_alloc_size_map);
  appMemFree(APP_MEM_HEAP_DDR, dspload_client, sizeof(Dspload_Client_t));
}

/** \brief Error handling for dspload
*/
static void
dspload_exit(int ecode)
{
  /* CHECK_MISRA("-20.7")  -> Disable MISRA C 2004 rule 20.7  */
  /* longjmp is used to handle errors occuring in DLOAD and clean up gracefully.   */
  longjmp(dspload_jmpbuf, -1);
  /* RESET_MISRA("-20.7")  -> Reset MISRA C 2004 rule 20.7  */
}

/** \brief Clean up all memory allocations in case anything goes wrong
 */
static void
dspload_free_all(AllocPtrSizeMap_t *alloc_size_map)
{
  if (alloc_size_map != NULL)
  {
    int i = 0;
    for (i = 0; i < alloc_size_map->size; i++)
    {
      void *ptr = alloc_size_map->ptrs[i];
      int  size = alloc_size_map->sizes[i];
      if (size > 0)
      {
        appMemFree(APP_MEM_HEAP_DDR, ptr, size);
      }
    }
    appMemFree(APP_MEM_HEAP_DDR, alloc_size_map, sizeof(AllocPtrSizeMap_t));
  }
}

/************************ Below are DLIF implementations *********************/

void DLIF_exit(int code)
{
  dspload_exit(code);
}

void DLIF_warning(LOADER_WARNING_TYPE wtype, const char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt);
  printf("<< D L O A D >> WARNING: ");
  vprintf(fmt,ap);
  va_end(ap);
}

void DLIF_error(LOADER_ERROR_TYPE etype, const char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt);
  printf("<< D L O A D >> ERROR: ");
  vprintf(fmt,ap);
  va_end(ap);
  DLIF_exit(-1);
}

void DLIF_trace(const char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt);
  vprintf(fmt,ap);
  va_end(ap);
}

int DLIF_fseek(LOADER_FILE_DESC *stream, int32_t offset, int origin)
{
  int status = 0;
  switch(origin){
    case SEEK_SET:
      stream->cur = stream->orig;
      stream->cur += offset;
      stream->read_size = offset;
      break;
    case SEEK_CUR:
      stream->cur += offset;
      stream->read_size += offset;
      break;
    case SEEK_END:
      stream->cur = stream->orig + stream->size;
      stream->read_size = stream->size;
      break;
    default:
      DLIF_exit(-1);
      status = -1;
      break;
  }
  return status;
}

size_t DLIF_fread(void *ptr, size_t size, size_t nmemb,
                  LOADER_FILE_DESC *stream)
{
  memcpy(ptr, stream->cur, size*nmemb);
  stream->cur += (size*nmemb);
  stream->read_size += (size*nmemb);
  return nmemb;
}

int32_t DLIF_ftell(LOADER_FILE_DESC *stream)
{
  return (int32_t)(stream->cur - stream->orig);
}

int32_t DLIF_fclose(LOADER_FILE_DESC *fd)
{
  return 0;
}

void*   DLIF_malloc(size_t size)
{
  void *ptr = NULL;

  if (NULL == dlif_alloc_size_map)
  {
    dlif_alloc_size_map = (AllocPtrSizeMap_t *) appMemAlloc(APP_MEM_HEAP_DDR,
                        sizeof(AllocPtrSizeMap_t), DYN_LOAD_MEM_DEFAULT_ALIGN);
    if (NULL != dlif_alloc_size_map)
    {
      memset(dlif_alloc_size_map, 0, sizeof(AllocPtrSizeMap_t));
    }
  }
  if (NULL != dlif_alloc_size_map)
  {
    ptr = appMemAlloc(APP_MEM_HEAP_DDR, size, DYN_LOAD_MEM_DEFAULT_ALIGN);
    if (dlif_alloc_size_map->size < MAX_PTR_SIZE_MAP_SIZE)
    {
      dlif_alloc_size_map->ptrs[dlif_alloc_size_map->size] = ptr;
      dlif_alloc_size_map->sizes[dlif_alloc_size_map->size] = size;
      dlif_alloc_size_map->size += 1;
    }
    else
    {
      DLIF_warning(DLWT_MISC, "DLIF_malloc() ptr_size_map overflow: %d\n",
                   dlif_alloc_size_map->size);
    }
  }

  if (NULL == ptr)
  {
    DLIF_error(DLET_MEMORY, "DLIF_malloc() failed for size %d.\n", (int) size);
  }

  return ptr;
}

void    DLIF_free(void* ptr)
{
  if (dlif_alloc_size_map != NULL)
  {
    int i, size = 0;
    for (i = 0; i < dlif_alloc_size_map->size; i++)
    {
      if (dlif_alloc_size_map->ptrs[i] == ptr)
      {
        size = dlif_alloc_size_map->sizes[i];
        dlif_alloc_size_map->ptrs[i] = dlif_alloc_size_map->ptrs[
                                        dlif_alloc_size_map->size - 1];
        dlif_alloc_size_map->sizes[i] = dlif_alloc_size_map->sizes[
                                        dlif_alloc_size_map->size - 1];
        dlif_alloc_size_map->size -= 1;
        break;
      }
    }
    if (size > 0)
    {
      appMemFree(APP_MEM_HEAP_DDR, ptr, size);
    }
    else
    {
      DLIF_warning(DLWT_MISC, "DLIF_free() %p size unknown, not freed\n", ptr);
    }
  }
}

BOOL DLIF_read(void* client_handle, 
               void *ptr, size_t size, size_t nmemb, TARGET_ADDRESS src)
{
  DLIF_error(DLET_MISC, "DLIF_read should not be called\n");
  return FALSE;
}

BOOL DLIF_memcpy(void* client_handle, 
                 void *to, void *from, size_t size)
{
  return (!memcpy(to, from, size)) ? FALSE : TRUE;
}

int32_t DLIF_execute(void* client_handle,
                     TARGET_ADDRESS exec_addr)
{
  DLIF_error(DLET_MISC, "DLIF_execute should not be called");
  return -1;
}

BOOL DLIF_register_dsbt_index_request(DLOAD_HANDLE handle,
                                      const char *requestor_name,
				      int32_t     requestor_file_handle,
				      int32_t     requested_dsbt_index)
{
  DLIF_error(DLET_MISC,
             "DLIF_register_dsbt_index_request should not be called\n");
  return FALSE;
}

void DLIF_assign_dsbt_indices(void)
{
  DLIF_error(DLET_MISC, "DLIF_assign_dsbt_indices should not be called\n");
}

int32_t DLIF_get_dsbt_index(int32_t file_handle)
{
  DLIF_error(DLET_MISC, "DLIF_get_dsbt_index should not be called\n");
  return DSBT_INDEX_INVALID;
}

BOOL DLIF_update_all_dsbts(void)
{
  DLIF_error(DLET_MISC, "DLIF_update_all_dsbts should not be called\n");
  return FALSE;
}

/*****************************************************************************/
/* DLIF_ALLOCATE() - Return the load address of the segment/section          */
/*      described in its parameters and record the run address in            */
/*      run_address field of DLOAD_MEMORY_REQUEST.                           */
/*****************************************************************************/
BOOL DLIF_allocate(void* client_handle, struct DLOAD_MEMORY_REQUEST *req)
{
  /*------------------------------------------------------------------------*/
  /* Get pointers to API segment and file descriptors.                      */
  /* For TVM dynamic loader, target address must be in DDR                  */
  /*------------------------------------------------------------------------*/
  struct DLOAD_MEMORY_SEGMENT* obj_desc = req->segment;
  void* addr = DLIF_malloc(obj_desc->memsz_in_bytes);

  #if LOADER_MEM_DEBUG
  printf("DLIF_allocate: %d bytes starting at %p (relocated from 0x%llx)\n",
                      obj_desc->memsz_in_bytes, addr,
                      (uint64_t)obj_desc->target_address);
  #endif

  obj_desc->target_address = (TARGET_ADDRESS) addr;

  /*------------------------------------------------------------------------*/
  /* Target memory request was successful.                                  */
  /*------------------------------------------------------------------------*/
  return (addr == NULL) ? FALSE : TRUE;
}

/*****************************************************************************/
/* DLIF_RELEASE() - Unmap or free target memory that was previously          */
/*      allocated by DLIF_allocate().                                        */
/*****************************************************************************/
BOOL DLIF_release(void* client_handle, struct DLOAD_MEMORY_SEGMENT* ptr)
{
  DLIF_free((void*) ptr->target_address);

  #if LOADER_MEM_DEBUG
  printf("DLIF_free: %d bytes starting at %p\n",
                      ptr->memsz_in_bytes, (void*) ptr->target_address);
  #endif
  return TRUE;
}

/*****************************************************************************/
/* DLIF_COPY() - Copy data from file to host-accessible memory.              */
/*      Returns a host pointer to the data in the host_address field of the  */
/*      DLOAD_MEMORY_REQUEST object.                                         */
/* C7x load to C7x: use the same buffer for target_address and host_address  */
/*****************************************************************************/
BOOL DLIF_copy(void* client_handle, struct DLOAD_MEMORY_REQUEST* req)
{
  struct DLOAD_MEMORY_SEGMENT* obj_desc = req->segment;
  LOADER_FILE_DESC* f = req->fp;

  int result = 1;
  void *buf = NULL;
  if (obj_desc->objsz_in_bytes)
  {
    buf = (void *) obj_desc->target_address;

    if (buf == NULL)
    {
      DLIF_error(DLET_MEMORY, "DLIF_copy allocation failure.\n");
      result = 0;
    }
    else
    {
      DLIF_fseek(f, req->offset, SEEK_SET);
      result = DLIF_fread(buf, obj_desc->objsz_in_bytes, 1, f);

      if (result != 1)
      {
        DLIF_error(DLET_FILE, "DLIF_fread returns %s != 1\n", result);
      }
    }
  }

  req->host_address = buf;

  return (result == 1) ? TRUE : FALSE;
}


/*****************************************************************************/
/* DLIF_WRITE() - Write updated (relocated) segment contents to target       */
/*      memory.                                                              */
/* C7x load to C7x: target address and host address point to the same buffer */
/*                  no need to copy.  Perform cache WbInv on the segment     */
/*****************************************************************************/
BOOL DLIF_write(void* client_handle, struct DLOAD_MEMORY_REQUEST* req)
{
  /* no more uses after DLIF_write */
  if (req->host_address)
  {
    req->host_address = NULL;
  }

  /* cache WbInv on the dloaded segment, Data Cache -> DDR (-> Program Cache) */
  struct DLOAD_MEMORY_SEGMENT* obj_desc = req->segment;
  appMemCacheWbInv((void *)obj_desc->target_address, obj_desc->memsz_in_bytes);

  #if LOADER_MEM_DEBUG
  printf("DLIF_write: %d bytes starting at %p\n",
         obj_desc->memsz_in_bytes, obj_desc->target_address);
  #endif

  return TRUE;
}

/******************************************************************************
* DLIF_LOAD_DEPENDENT()
* C7x load to C7x: retrieve from client_handle, "dsp_syms.out" in memory buffer,
*                  dsp_syms_names[] and dsp_syms_addrs[]
******************************************************************************/
int DLIF_load_dependent(void* client_handle, const char* so_name)
{
  int file_handle = 0;
  if (strcmp(so_name, "dsp_syms.out") == 0)
  {
    /*----------------------------------------------------------------------
    * Similar to C7x OpenCL, dsp_syms.out is the dependent executable
    * and contains dummy addresses for the C7x firmware symbols at
    * the dynamic executable build time.  After these symbols are loaded
    * into the symbol table, we need to update them to real addresses
    * obtained from the C7x firmware.  Then the real addresses
    * can be used for resolving/relocating the dynamic executable.
    *---------------------------------------------------------------------*/
    Dspload_Client_t* dspload_client = (Dspload_Client_t*) client_handle;
    DLOAD_HANDLE dload_handle = dspload_client->dload_handle;

    const char* data = dspload_client->file_data;
    struct Elf64_Ehdr *ehdr = (struct Elf64_Ehdr *) data;
    struct Elf64_Shdr *shdr = (struct Elf64_Shdr *) (data + ehdr->e_shoff +
                                       ehdr->e_shstrndx * ehdr->e_shentsize);
    const char* names_start = data + shdr->sh_offset;
    int i;
    for (i = 0; i < ehdr->e_shnum; i++)
    {
      shdr = (struct Elf64_Shdr *) (data + ehdr->e_shoff +
                                    i * ehdr->e_shentsize);
      if (strncmp(".dsp_syms_out", (names_start + shdr->sh_name), 13) == 0)
      {
          break;
      }
    }

    LOADER_FILE_DESC f;
    f.binary = (int8_t*) (data + shdr->sh_offset);
    f.cur = f.orig = f.binary;
    f.length = f.size = shdr->sh_size;
    f.read_size = 0;
    file_handle = DLOAD_load_symbols(dload_handle, &f);
    for (i = 0; i < dspload_client->dsp_syms_size; i++)
    {
      DLOAD_update_symbol(dload_handle, file_handle,
                          dspload_client->dsp_syms_names[i],
         (TARGET_ADDRESS) dspload_client->dsp_syms_addrs[i]);
    }
  }
  return file_handle;
}

/******************************************************************************
* DLIF_UNLOAD_DEPENDENT()
******************************************************************************/
void DLIF_unload_dependent(void* client_handle, uint32_t file_handle)
{
  Dspload_Client_t* dspload_client = (Dspload_Client_t*) client_handle;
  DLOAD_HANDLE dload_handle = dspload_client->dload_handle;
  DLOAD_unload(dload_handle, file_handle);
}

