/*
* dload_api.h
*
* Dynamic Loader API Specification
* --------------------------------
*
* Client-side of API is assumed to be platform dependent, but object file
* format independent.
*
* Core Loader side of API is assumed to be platform independent, but
* object file format dependent and target dependent.
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

#ifndef DLOAD_API_H
#define DLOAD_API_H

#if defined(DEVICE_J721E)
  #if !defined(C70_TARGET)
    #define C70_TARGET
  #endif
  #if !defined(ELF64)
    #define ELF64  1
  #endif
#endif

#include <inttypes.h>
#include <stdio.h>
#include "util.h"

extern int debugging_on;

/*****************************************************************************/
/* Specification of Loader File Descriptor.  If client side of the loader    */
/* supports virtual memory, this may need to be updated to facilitate the    */
/* use of mmap().                                                            */
/*****************************************************************************/
#ifndef _SYS_BIOS
typedef FILE LOADER_FILE_DESC;
#else
struct FILE_DESC
{
    int8_t mode;
    int8_t *binary;
    int8_t *orig;
    int8_t *cur;
    uint32_t size;
    uint32_t read_size;
    size_t length;
    FILE *fp;
};
typedef struct FILE_DESC LOADER_FILE_DESC;
#endif

static const int LOADER_SEEK_SET = SEEK_SET;
static const int LOADER_SEEK_CUR = SEEK_CUR;
static const int LOADER_SEEK_END = SEEK_END;

/*****************************************************************************/
/* TARGET_ADDRESS - type suitable for storing target memory address values.  */
/*****************************************************************************/
#if ELF64
typedef uint64_t TARGET_ADDRESS;
#else
typedef uint32_t TARGET_ADDRESS;
#endif

/*****************************************************************************/
/* Define DLOAD Object Handle                                                */ 
/*****************************************************************************/
typedef void * DLOAD_HANDLE;

/*****************************************************************************/
/* Core Loader Provided API Functions (Core Loader Entry Points)             */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/* DLOAD_version()                                                           */
/*                                                                           */
/*    Return a string constant representation for the version ID of the      */
/*    dynamic loader's core loader source code.                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include "version.h"
#define DLOAD_version()	VERSION

/*---------------------------------------------------------------------------*/
/* DLOAD_create()                                                            */
/*                                                                           */
/*    Construct and initialize the dynamic loader core's handle.             */
/*                                                                           */
/*---------------------------------------------------------------------------*/
DLOAD_HANDLE  DLOAD_create(void * client_handle);
                                                                                 
/*---------------------------------------------------------------------------*/
/* DLOAD_destroy()                                                           */
/*                                                                           */
/*    Destroy and finalize the dynamic loader core's handle.                 */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void     DLOAD_destroy(DLOAD_HANDLE handle);

/*---------------------------------------------------------------------------*/
/* DLOAD_initialize()                                                        */
/*                                                                           */
/*    Construct and initialize data structures internal to the dynamic       */
/*    loader core.                                                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void     DLOAD_initialize(DLOAD_HANDLE handle);

/*---------------------------------------------------------------------------*/
/* DLOAD_finalize()                                                          */
/*                                                                           */
/*    Destroy and finalize data structures internal to the dynamic           */
/*    loader core.                                                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void     DLOAD_finalize(DLOAD_HANDLE handle);

/*---------------------------------------------------------------------------*/
/* DLOAD_load_symbols()                                                      */
/*                                                                           */
/*    Load externally visible symbols from the specified file so that they   */
/*    can be linked against when another object file is subsequntly loaded.  */
/*    External symbols will be made available for global symbol linkage.     */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int32_t  DLOAD_load_symbols(DLOAD_HANDLE handle, LOADER_FILE_DESC* fp);

/*---------------------------------------------------------------------------*/
/* DLOAD_load()                                                              */
/*                                                                           */
/*    Dynamically load the specified file and return a file handle for the   */
/*    loaded file.  If the load fails, this function will return a value     */
/*    zero (0).                                                              */
/*                                                                           */
/*    The core loader must have read access to the file pointed by fp.       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int32_t  DLOAD_load(DLOAD_HANDLE handle, LOADER_FILE_DESC* fp);

/*---------------------------------------------------------------------------*/
/* DLOAD_unload()                                                            */
/*                                                                           */
/*    Given a file handle ID, unload all object segments associated with     */
/*    the identified file and any of its dependents that are not still in    */
/*    use.                                                                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLOAD_unload(DLOAD_HANDLE handle, uint32_t pseudopid);

/*---------------------------------------------------------------------------*/
/* DLOAD_get_entry_names_info()                                              */
/*                                                                           */
/*    Given a file handle, return the number entry points that are           */
/*    available in the specified file as well as the max name length.  This  */
/*    can then be used by the client to allocate the appropriate amount of   */
/*    memory needed to call DLOAD_get_entry_names()                          */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLOAD_get_entry_names_info(DLOAD_HANDLE handle, uint32_t file_handle,
                                    int32_t *entry_pt_cnt,
                                    int32_t *entry_pt_max_name_len);

/*---------------------------------------------------------------------------*/
/* DLOAD_get_entry_names()                                                   */
/*                                                                           */
/*    Given a file handle, build a list of entry point names that are        */
/*    available in the specified file.  This can be used when querying       */
/*    the list of global functions available in a shared library.            */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLOAD_get_entry_names(DLOAD_HANDLE handle, uint32_t file_handle, 
                               int32_t* entry_pt_cnt, char*** entry_pt_names);

/*---------------------------------------------------------------------------*/
/* DLOAD_query_symbol()                                                      */
/*                                                                           */
/*    Query the value of a symbol that is defined by an object file that     */
/*    has previously been loaded.  Boolean return value will be false if     */
/*    the symbol is not found.                                               */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLOAD_query_symbol(DLOAD_HANDLE handle, uint32_t file_handle, 
                            const char *sym_name, TARGET_ADDRESS *sym_val);

/*---------------------------------------------------------------------------*/
/* DLOAD_update_symbol()                                                     */
/*                                                                           */
/*    Update the value of a symbol that is defined by an object file that    */
/*    has previously been loaded.  Boolean return value will be false if     */
/*    the symbol is not found.                                               */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLOAD_update_symbol(DLOAD_HANDLE handle, uint32_t file_handle,
                            const char *sym_name, TARGET_ADDRESS sym_val);

/*---------------------------------------------------------------------------*/
/* DLOAD_get_entry_point()                                                   */
/*                                                                           */
/*    Given a file handle, return the entry point target address associated  */
/*    with that object file.  The entry point address value is written to    */
/*    *sym_val.  The return value of the function indicates whether the      */
/*    file with the specified handle was found or not.                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLOAD_get_entry_point(DLOAD_HANDLE handle, uint32_t file_handle, 
                               TARGET_ADDRESS *sym_val);

/*---------------------------------------------------------------------------*/
/* DLOAD_load_arguments()                                                    */
/*                                                                           */
/*    Given a file handle, find the object file assicated with that handle   */
/*    and copy the argc/argv information from the client into that object    */
/*    file's .args section. The return value indicates whether the operation */
/*    was successful. If there are no loaded object files which match the    */
/*    handle or if there is insufficient space in the .args section to hold  */
/*    the specified argc/argv information, the function will return false.   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL DLOAD_load_arguments(DLOAD_HANDLE handle, uint32_t file_handle, 
                           int argc, char** argv);

/*---------------------------------------------------------------------------*/
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
/*    The return value of the function indicates whether the file with the   */
/*    specified handle was found or not.                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLOAD_prepare_for_execution(DLOAD_HANDLE handle, uint32_t file_handle, 
                               TARGET_ADDRESS *sym_val, 
                               int argc, char** argv);


/*****************************************************************************/
/* Client Provided API Functions                                             */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/* File I/O                                                                  */
/*                                                                           */
/*    The client side of the dynamic loader must provide basic file I/O      */
/*    capabilities so that the core loader has random access into any        */
/*    object file that it is asked to load.                                  */
/*                                                                           */
/*    The client side of the dynamic loader must provide a definition of     */
/*    the LOADER_FILE_DESC in dload_filedefs.h.  This allows the core loader */
/*    to be independent of how the client accesses raw data in an object     */
/*    file.                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* DLIF_fseek()                                                              */
/*                                                                           */
/*    Seek to a position in a file (accessed via 'stream') based on the      */
/*    values for offset and origin.                                          */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int      DLIF_fseek(LOADER_FILE_DESC *stream, int32_t offset, int origin);

/*---------------------------------------------------------------------------*/
/* DLIF_ftell()                                                              */
/*                                                                           */
/*    Return the current file position in the file identified in the         */
/*    LOADER_FILE_DESC pointed to by 'stream'.                               */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int32_t  DLIF_ftell(LOADER_FILE_DESC *stream);

/*---------------------------------------------------------------------------*/
/* DLIF_fread()                                                              */
/*                                                                           */
/*    Read 'size' * 'nmemb' bytes of data from the file identified in the    */
/*    LOADER_FILE_DESC object pointed to by 'stream', and write that data    */
/*    into the memory accessed via 'ptr'.                                    */
/*                                                                           */
/*---------------------------------------------------------------------------*/
size_t   DLIF_fread(void *ptr, size_t size, size_t nmemb,
                    LOADER_FILE_DESC *stream);

/*---------------------------------------------------------------------------*/
/* DLIF_fclose()                                                             */
/*                                                                           */
/*    Close a file that was opened on behalf of the core loader. Ownership   */
/*    of the file pointer in question belongs to the core loader, but the    */
/*    client has exclusive access to the file system.                        */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int32_t  DLIF_fclose(LOADER_FILE_DESC *fd);

/*---------------------------------------------------------------------------*/
/* Host Memory Management                                                    */
/*                                                                           */
/*    Allocate and free host memory as needed for the dynamic loader's       */
/*    internal data structures.  If the dynamic loader resides on the        */
/*    target architecture, then this memory is allocated from a target       */
/*    memory heap that must be managed separately from memory that is        */
/*    allocated for a dynamically loaded object file.                        */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* DLIF_malloc()                                                             */
/*                                                                           */
/*    Allocate 'size' bytes of memory space that is usable as scratch space  */
/*    (appropriate for the loader's internal data structures) by the dynamic */
/*    loader.                                                                */
/*                                                                           */
/*    If allocation fails, this function must not return.                    */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void*    DLIF_malloc(size_t size);

/*---------------------------------------------------------------------------*/
/* DLIF_free()                                                               */
/*                                                                           */
/*    Free memory space that was previously allocated by DLIF_malloc().      */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void     DLIF_free(void* ptr);

/*---------------------------------------------------------------------------*/
/* Target Memory Allocator Interface                                         */
/*                                                                           */
/*    The client side of the dynamic loader must create and maintain an      */
/*    infrastructure to manage target memory.  The client must keep track    */
/*    of what target memory is associated with each object segment,          */
/*    allocating target memory for newly loaded objects and release target   */
/*    memory that is associated with objects that are being unloaded from    */
/*    the target architecture.                                               */
/*                                                                           */
/*    The two client-supplied functions, DLIF_allocate() and DLIF_release(), */
/*    are used by the core loader to interface into the client side's        */
/*    target memory allocator infrastructure.                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* DLOAD_SEGMENT_FLAGS - segment characteristics.                            */
/*---------------------------------------------------------------------------*/
typedef uint32_t DLOAD_SEGMENT_FLAGS;
static const int DLOAD_SF_executable  = 0x1;  /* Memory must be executable   */
static const int DLOAD_SF_relocatable = 0x2;  /* Segment must be relocatable */
static const int DLOAD_SF_writable    = 0x4;  /* Memory must be writable     */

/*---------------------------------------------------------------------------*/
/* DLOAD_MEMORY_SEGMENT - Define structure to represent placement and size   */
/*      details of a segment to be loaded.                                   */
/*---------------------------------------------------------------------------*/
struct DLOAD_MEMORY_SEGMENT
{
   uint32_t       target_page;          /* requested/returned memory page    */
   TARGET_ADDRESS target_address;       /* requested/returned address        */
   uint32_t       objsz_in_bytes;       /* size of init'd part of segment    */
   uint32_t       memsz_in_bytes;       /* size of memory block for segment  */
//   DLOAD_SEGMENT_FLAGS flags;           /* allocation request flags          */
};

/*---------------------------------------------------------------------------*/
/* DLOAD_MEMORY_REQUEST - Define structure to represent a target memory      */
/*      request made by the core loader on behalf of a segment that the      */
/*      loader needs to relocate and write into target memory.               */
/*---------------------------------------------------------------------------*/
struct DLOAD_MEMORY_REQUEST
{
   LOADER_FILE_DESC            *fp;           /* file being loaded           */
   struct DLOAD_MEMORY_SEGMENT *segment;      /* obj for req/ret alloc       */
   void                        *host_address; /* ret hst ptr from DLIF_copy()*/
   BOOL                         is_loaded;    /* returned as true if segment */
                                              /* is already in target memory */
   uint32_t                     offset;       /* file offset of segment's    */
                                              /* raw data                    */
   uint32_t                     flip_endian;  /* endianness of trg opp host  */
   DLOAD_SEGMENT_FLAGS          flags;        /* allocation request flags    */
   uint32_t                     align;        /* align of trg memory block   */
};

/*---------------------------------------------------------------------------*/
/* DLIF_initMem()                                                            */
/*                                                                           */
/*    Given an address and size, initialize the memory used to load the      */
/*    dynamic segments.  This should be called by the client before          */
/*    beginning dynamic loading.                                             */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLIF_initMem(void* client_handle, uint32_t dynMemAddr, uint32_t size);
                                                                                 
/*---------------------------------------------------------------------------*/
/* DLIF_deinitMem()                                                          */
/*                                                                           */
/*    De-initialize the memory used to load the dynamic segments.            */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLIF_deinitMem(void* client_handle);

/*---------------------------------------------------------------------------*/
/* DLIF_allocate()                                                           */
/*                                                                           */
/*    Given a DLOAD_MEMORY_REQUEST created by the core loader, allocate      */
/*    target memory to fulfill the request using the target memory           */
/*    management infrastrucutre on the client side of the dynamic loader.    */
/*    The contents of the DLOAD_MEMORY_REQUEST will be updated per the       */
/*    details of a successful allocation.  The allocated page and address    */
/*    can be found in the DLOAD_MEMORY_SEGMENT attached to the request.      */
/*    The boolean return value reflects whether the allocation was           */
/*    successful or not.                                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLIF_allocate(void* client_handle, struct DLOAD_MEMORY_REQUEST *req);

/*---------------------------------------------------------------------------*/
/* DLIF_release()                                                            */
/*                                                                           */
/*    Given a DLOAD_MEMORY_SEGMENT description, free the target memory       */
/*    associated with the segment using the target memory management         */
/*    infrastructure on the client side of the dynamic loader.               */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLIF_release(void* client_handle, struct DLOAD_MEMORY_SEGMENT* ptr);

/*---------------------------------------------------------------------------*/
/* Target Memory Access / Write Services                                     */
/*                                                                           */
/*    The client side's target memory allocator infrastructure communicates  */
/*    with the core loader through the DLOAD_MEMORY_REQUEST and              */
/*    DLOAD_MEMORY_SEGMENT data structures defined above.  To complete the   */
/*    loading of an object segment, the segment may need to be relocated     */
/*    before it is actually written to target memory in the space that was   */
/*    allocated for it by DLIF_allocate().                                   */
/*                                                                           */
/*    The client side of the dynamic loader provides two functions to help   */
/*    complete the process of loading an object segment, DLIF_copy() and     */
/*    DLIF_write().                                                          */
/*                                                                           */
/*    These functions help to make the core loader truly independent of      */
/*    whether it is running on the host or target architecture and how the   */
/*    client provides for reading/writing from/to target memory.             */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* DLIF_copy()                                                               */
/*                                                                           */
/*    Copy segment data from the object file described in the 'fp' and       */
/*    'offset' of the DLOAD_MEMORY_REQUEST into host accessible memory so    */
/*    that it can relocated or otherwise manipulated by the core loader.     */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLIF_copy(void* client_handle, struct DLOAD_MEMORY_REQUEST* req);

/*---------------------------------------------------------------------------*/
/* DLIF_write()                                                              */
/*                                                                           */
/*    Once the segment data described in the DLOAD_MEMORY_REQUEST is ready   */
/*    (relocated, if needed), write the segment contents to the target       */
/*    memory identified in the DLOAD_MEMORY_SEGMENT attached to the request. */
/*                                                                           */
/*    After the segment contents have been written to target memory, the     */
/*    core loader should discard the DLOAD_MEMORY_REQUEST object, but retain */
/*    the DLOAD_MEMORY_SEGMENT object so that the target memory associated   */
/*    with the segment can be releases when the segment is unloaded.         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLIF_write(void* client_handle, struct DLOAD_MEMORY_REQUEST* req);

/*---------------------------------------------------------------------------*/
/* DLIF_read()                                                               */
/*                                                                           */
/*    Given a host accessible buffer, read content of indicated target       */
/*    memory address into the buffer.                                        */
/*---------------------------------------------------------------------------*/
BOOL     DLIF_read(void* client_handle, 
                   void *ptr, size_t size, size_t nmemb, TARGET_ADDRESS src);

/*---------------------------------------------------------------------------*/
/* DLIF_memcpy()                                                             */
/*                                                                           */
/*    Copy memory within host memory.                                        */
/*---------------------------------------------------------------------------*/
BOOL     DLIF_memcpy(void* client_handle, void *to, void *from, size_t size);

/*---------------------------------------------------------------------------*/
/* DLIF_execute()                                                            */
/*                                                                           */
/*    Start execution on the target architecture from given 'exec_addr'.     */
/*    If the dynamic loader is running on the target architecture, this can  */
/*    be effected as a simple function call.                                 */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int32_t  DLIF_execute(void* client_handle, TARGET_ADDRESS exec_addr);

/*---------------------------------------------------------------------------*/
/* Loading and Unloading of Dependent Files                                  */
/*                                                                           */
/*    The dynamic loader core loader must coordinate loading and unloading   */
/*    dependent object files with the client side of the dynamic loader.     */
/*    This allows the client to keep its bookkeeping information up to date  */
/*    with what is currently loaded on the target architecture.              */
/*                                                                           */
/*    For instance, the client may need to interact with a file system or    */
/*    registry.  The client may also need to update debug information in     */
/*    synch with the loading and unloading of shared objects.                */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* DLIF_load_dependent()                                                     */
/*                                                                           */
/*    Ask client to find and open a dependent file identified by the         */
/*    'so_name' parameter, then, if necessary, initiate a DLOAD_load()       */
/*    call to actually load the shared object onto the target.  A            */
/*    successful load will return a file handle ID that the client can       */
/*    associate with the newly loaded file.                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int      DLIF_load_dependent(void* client_handle, const char* so_name);

/*---------------------------------------------------------------------------*/
/* DLIF_unload_dependent()                                                   */
/*                                                                           */
/*    Ask client to unload a dependent file identified by the 'file_handle'  */
/*    parameter.  Initiate a call to DLOAD_unload() to actually free up      */
/*    the target memory that was occupied by the object file.                */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void     DLIF_unload_dependent(void* client_handle, uint32_t file_handle);

/*---------------------------------------------------------------------------*/
/* Error/Warning Registration Functions                                      */
/*                                                                           */
/*    The client will maintain an error/warning log.  This will allow the    */
/*    core loader to register errors and warnings in the load during a       */
/*    given dynamic load.  The client is required to check the log after     */
/*    each load attempt to report any problems.                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Loader Warning Types                                                      */
/*---------------------------------------------------------------------------*/
typedef enum {
    DLWT_MISC = 0,              /* Miscellaneous warning                     */
    DLWT_FILE                   /* Warning missing/invalid file information  */
} LOADER_WARNING_TYPE;

/*---------------------------------------------------------------------------*/
/* DLIF_warning()                                                            */
/*                                                                           */
/*    Log a warning message with the client's error/warning handling         */
/*    infrastructure.                                                        */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void     DLIF_warning(LOADER_WARNING_TYPE wtype, const char *fmt, ...);

/*---------------------------------------------------------------------------*/
/* Loader Error Types                                                        */
/*---------------------------------------------------------------------------*/
typedef enum {
    DLET_MISC = 0,              /* Miscellaneous error                       */
    DLET_FILE,                  /* Error reading/processing file             */
    DLET_SYMBOL,                /* Symbol resolution error                   */
    DLET_RELOC,                 /* Relocation error                          */
    DLET_MEMORY,                /* Host memory allocation/free error         */
    DLET_TRGMEM,                /* Target memory allocation/free error       */
    DLET_DEBUG                  /* Shared object or DLL debug error          */
} LOADER_ERROR_TYPE;

/*---------------------------------------------------------------------------*/
/* DLIF_error()                                                              */
/*                                                                           */
/*    Log an error message with the client's error/warning handling          */
/*    infrastructure.                                                        */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void     DLIF_error(LOADER_ERROR_TYPE etype, const char *fmt, ...);

/*---------------------------------------------------------------------------*/
/* DLIF_exit()                                                               */
/*                                                                           */
/*    Abort the loader following a fatal error.                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void     DLIF_exit(int code);

/*---------------------------------------------------------------------------*/
/* DLIF_trace()                                                              */
/*                                                                           */
/*    Log a message with the client's trace handling infrastructure.         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void     DLIF_trace(const char *fmt, ...);

/*---------------------------------------------------------------------------*/
/* Dynamic Static Base Table (DSBT) Support Functions                        */
/*---------------------------------------------------------------------------*/
#define DSBT_INDEX_INVALID        -1
#define DSBT_DSBT_BASE_INVALID     0
#define DSBT_STATIC_BASE_INVALID   0

/*****************************************************************************/
/* Core Loader Side of DSBT Support                                          */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/* DLOAD_get_dsbt_size()                                                     */
/*                                                                           */
/*    Query the size of the DSBT associated with a specified file. The       */
/*    client will check the size of a module's DSBT before it writes a copy  */
/*    of the master DSBT to the module's DSBT. If the module's DSBT is not   */
/*    big enough, an error will be emitted and the load will fail.           */
/*                                                                           */
/*---------------------------------------------------------------------------*/
uint32_t  DLOAD_get_dsbt_size(DLOAD_HANDLE handle, int32_t file_handle);

/*---------------------------------------------------------------------------*/
/* DLOAD_get_dsbt_base()                                                     */
/*                                                                           */
/*    Find DSBT address for specified file. The client will query for this   */
/*    address after allocation and symbol relocation has been completed.     */
/*    The client will write a copy of the master DSBT to the returned DSBT   */
/*    address if the module's DSBT size is big enough.                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLOAD_get_dsbt_base(DLOAD_HANDLE handle, int32_t file_handle, 
                             TARGET_ADDRESS *dsbt_base);

/*---------------------------------------------------------------------------*/
/* DLOAD_get_static_base()                                                   */
/*                                                                           */
/*    Find static base for a specified file. The client will query for this  */
/*    address after allocation and symbol relocation has been completed.     */
/*    The client will use the returned static base value to fill the slot    */
/*    in the master DSBT that is associated with this module.                */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLOAD_get_static_base(DLOAD_HANDLE handle, int32_t file_handle, 
                               TARGET_ADDRESS *static_base);


/*****************************************************************************/
/* Client Side of DSBT Support                                               */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/* DLIF_register_dsbt_index_request()                                        */
/*                                                                           */
/*    Register a request for a DSBT index with the client. A module can      */
/*    make a specific DSBT index request or it can allow the client to       */
/*    assign a DSBT index on its behalf (requested_dsbt_index == -1). The    */
/*    client implementation of this function must check that a specific DSBT */
/*    index request does not conflict with a previous specific DSBT index    */
/*    request.                                                               */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLIF_register_dsbt_index_request(DLOAD_HANDLE handle,
                                          const char *requestor_name,
                                          int32_t     requestor_file_handle,
				          int32_t     requested_dsbt_index);

/*---------------------------------------------------------------------------*/
/* DLIF_assign_dsbt_indices()                                                */
/*                                                                           */
/*    Bind each module that registered a request for a DSBT index to a       */
/*    specific slot in the DSBT. Specific requests for DSBT indices will be  */
/*    honored first. Any general requests that remain will be assigned to    */
/*    the first available slot in the DSBT.                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void     DLIF_assign_dsbt_indices(void);

/*---------------------------------------------------------------------------*/
/* DLIF_get_dsbt_index()                                                     */
/*                                                                           */
/*    Given a module that uses the DSBT model, return the identity of the    */
/*    DSBT slot that was assigned to it by the client. This function can     */
/*    only be called after the client has assigned DSBT indices to all       */
/*    loaded object modules that use the DSBT model. The implementation of   */
/*    this function will check that a proper DSBT index has been assigned to */
/*    the specified module and an invalid index (-1) if there is a problem.  */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int32_t  DLIF_get_dsbt_index(int32_t file_handle);

/*---------------------------------------------------------------------------*/
/* DLIF_update_all_dsbts()                                                   */
/*                                                                           */
/*    Populate the client's model of the master DSBT with the static base    */
/*    for each assigned slot in the DSBT, then write a copy of the master    */
/*    DSBT to each module's DSBT location. The implementation of this        */
/*    function must check the size of each module's DSBT to make sure that   */
/*    it is large enough to hold a copy of the master DSBT. The function     */
/*    will return FALSE if there is a problem.                               */
/*                                                                           */
/*---------------------------------------------------------------------------*/
BOOL     DLIF_update_all_dsbts(void);
/*---------------------------------------------------------------------------*/
/* Abstract printf format specfiers for formatting addresses.                */
/* Example:  printf("address=" ADDRFMT "\n", my_target_address)              */
/*---------------------------------------------------------------------------*/
#if ELF64
  /* Specialize OF_ADDRFMT* for C7x as it has 48-bit addressing (not 64-bit) */
  #if defined(C70_TARGET)
    #define ADDRFMT    "0x%012" PRIx64
  #else 
    #define ADDRFMT    "0x%016" PRIx64
  #endif
#else
  #define ADDRFMT  "0x%08" PRIx32
#endif

#endif
