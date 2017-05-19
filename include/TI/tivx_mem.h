/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_MEM_H_
#define TIVX_MEM_H_

#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to Memory allocation and deallocation APIs
 */

/*!
 * \brief Enum that list all possible memory regions from which allocations are
 *        possible
 *
 * \ingroup group_tivx_mem
 */
typedef enum _tivx_mem_type_e
{
    /*! \brief External memory.
     *
     *  Typically large in size and can be used by kernels.
     *  as well as applications.
     */
    TIVX_MEM_EXTERNAL,

    /*! \brief Internal memory at L3 level.
     *
     *  Typically visiable to all CPUs, limited in size.
     *  Typically used by kernels and in very rare cases by applications.
     */
    TIVX_MEM_INTERNAL_L3,

    /*! \brief Internal memory at L2 level.
     *
     *  Typically local to CPU, very limited in size.
     *  Typically used by kernels.
     *
     *  This is used as scratch memory by kernels, i.e
     *  memory contents are not preserved across kernel function calls
     *
     *  tivxMemAlloc() API will linearly allocate from this memory
     *  segement. After each allocatation an internal
     *  offset will be incremented.
     *
     *  tivxMemFree() resets this offset to zero.
     *  i.e tivxMemAlloc() and tivxMemFree() are not heap like memory
     *  alloc and free functions.
     *
     *  NOT to be used by applications.
     */
    TIVX_MEM_INTERNAL_L2

} tivx_mem_type_e;

/*!
 * \brief Structure describing a shared memory pointer
 *
 * \ingroup group_tivx_mem
 */
typedef struct _tivx_shared_mem_ptr_t {

    /*! \brief Memory region to which this pointer belongs, see \ref tivx_mem_type_e */
    vx_enum mem_type;

    /*! \brief Value of pointer as seen in shared memory
     *         All CPUs will have method to convert from shared memory pointer
     *         to CPU local memory pointer
     */
    void *shared_ptr;

    /*! \brief Value of pointer as seen as by host CPU
     *         Host CPU will have method to convert to/from shared memory
     *         pointer
     */
    void *host_ptr;

    /*! \brief Value of pointer as seen as by target CPU
     *         Target CPU will have method to convert to/from shared memory
     *         pointer
     */
    void *target_ptr;

} tivx_shared_mem_ptr_t;


typedef struct _tivx_mem_stats_t {

    /*! \brief Total size of memory segment
     *         Set to 0 when memory segment size cannot be determined.
     */
    vx_uint32 mem_size;

    /*! \brief Max free block in memoru heap segment
     *         Set to 0 when free size cannot be determined.
     */
    vx_uint32 free_size;

} tivx_mem_stats;

/*!
 * \brief Alloc buffer from shared memory
 *
 * \param [in] mem_type Memory type to which this allocation belongs, see \ref tivx_mem_type_e
 * \param [out] mem_ptr Allocated memory pointer
 * \param [in] size Size of memory to allocate in bytes
 *
 * \ingroup group_tivx_mem
 */
vx_status tivxMemBufferAlloc(tivx_shared_mem_ptr_t *mem_ptr, uint32_t size, vx_enum mem_type);

/*!
 * \brief Free buffer from shared memory
 *
 * \param [out] mem_ptr Allocated memory pointer
 * \param [in] size Size of memory allocated in bytes
 *
 * \ingroup group_tivx_mem
 */
vx_status tivxMemBufferFree(tivx_shared_mem_ptr_t *mem_ptr, uint32_t size);

/*!
 * \brief Map a allocated buffer address
 *
 *        This is to ensure the memory pointed by the buffer is
 *        accesible to the caller and brought to a coherent state wrt caller.
 *
 * \param [in] host_ptr Buffer memory to map
 * \param [in] size Size of memory to map in units of bytes
 * \param [in] mem_type Memory type to which this pointer belongs, see \ref tivx_mem_type_e
 * \param [in] maptype Mapping type as defined by \ref vx_accessor_e
 *
 * \ingroup group_tivx_mem
 */
void tivxMemBufferMap(void *host_ptr, uint32_t size, vx_enum mem_type, vx_enum maptype);

/*!
 * \brief UnMap a buffer address
 *
 *        This is to ensure the memory pointed by the buffer pointer is
 *        made coherent with other possible readers of this buffer
 *
 * \param [in] host_ptr Buffer memory to unmap
 * \param [in] size Size of memory to unmap in units of bytes
 * \param [in] mem_type Memory type to which this pointer belongs, see \ref tivx_mem_type_e
 * \param [in] maptype Mapping type as defined by \ref vx_accessor_e
 *
 * \ingroup group_tivx_mem
 */
void tivxMemBufferUnmap(void *host_ptr, uint32_t size, vx_enum mem_type, vx_enum maptype);

/*!
 * \brief Convert Host pointer to shared pointer
 *
 * \param [in] host_ptr Host memory pointer
 * \param [in] mem_type Memory type to which this pointer belongs, see \ref tivx_mem_type_e
 *
 * \return Converted shared memory pointer
 *
 * \ingroup group_tivx_mem
 */
void* tivxMemHost2SharedPtr(void *host_ptr, vx_enum mem_type);

/*!
 * \brief Convert Shared memory pointer to host pointer
 *
 * \param [in] shared_ptr Shared memory pointer
 * \param [in] mem_type Memory type to which this pointer belongs, see \ref tivx_mem_type_e
 *
 * \return Converted host memory pointer
 *
 * \ingroup group_tivx_mem
 */
void* tivxMemShared2HostPtr(void *shared_ptr, vx_enum mem_type);

/*!
 * \brief Convert shared pointer to target pointer
 *
 * \param [in] shared_ptr Host memory pointer
 * \param [in] mem_type Memory type to which this pointer belongs, see \ref tivx_mem_type_e
 *
 * \return Converted shared memory pointer
 *
 * \ingroup group_tivx_mem
 */
void* tivxMemShared2TargetPtr(void *shared_ptr, vx_enum mem_type);

/*!
 * \brief Convert target memory pointer to shared pointer
 *
 * \param [in] target_ptr Target memory pointer
 * \param [in] mem_type Memory type to which this pointer belongs, see \ref tivx_mem_type_e
 *
 * \return Converted host memory pointer
 *
 * \ingroup group_tivx_mem
 */
void* tivxMemTarget2SharedPtr(void *target_ptr, vx_enum mem_type);

/*!
 * \brief Allocates memory of given size
 *
 * \param [in] size     size of the memory to be allocated
 *
 * \return Pointer to the allocated memory
 *
 * \ingroup group_tivx_mem
 */
void *tivxMemAlloc(vx_uint32 size, vx_enum mem_type);

/*!
 * \brief Frees already allocated memory
 *
 * \param [in] ptr  Pointer to the memory
 * \param [in] size size of the memory to be freed
 *
 * \ingroup group_tivx_mem
 */
void tivxMemFree(void *ptr, vx_uint32 size, vx_enum mem_type);

/*!
 * \brief Get memory segement information
 *
 * \param [out] stats Memory segment information
 * \param [in] mem_type Memory segment ID
 *
 * \ingroup group_tivx_mem
 */
void tivxMemStats(tivx_mem_stats *stats, vx_enum mem_type);

#ifdef __cplusplus
}
#endif

#endif
