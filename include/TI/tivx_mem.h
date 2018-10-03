/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
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
typedef enum _tivx_mem_heap_region_e
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

} tivx_mem_heap_region_e;

/*! \brief An enumeration of TI extension memory import types.
 * \ingroup group_tivx_mem
 */
typedef enum _tivx_memory_type_e {
    /*! \brief Memory type when a DMA will access the memory rather than the HOST. */
    TIVX_MEMORY_TYPE_DMA = VX_ENUM_BASE(VX_ID_TI, VX_ENUM_MEMORY_TYPE) + 0x0,
} tivx_memory_type_e;

/*!
 * \brief Structure describing a shared memory pointer
 *
 * \ingroup group_tivx_mem
 */
typedef struct _tivx_shared_mem_ptr_t {

    /*! \brief Memory region to which this pointer belongs, see \ref tivx_mem_heap_region_e */
    uint32_t mem_heap_region;
    
    /*! \brief reserved field to make this structure a multiple of 64b */    
    uint32_t rsv1[1];

    /*! \brief Value of pointer as seen in shared memory
     *         All CPUs will have method to convert from shared memory pointer
     *         to CPU local memory pointer
     */
    uint64_t shared_ptr;

    /*! \brief Value of pointer as seen as by host CPU
     *         Host CPU will have method to convert to/from shared memory
     *         pointer
     */
    uint64_t host_ptr;

} tivx_shared_mem_ptr_t;

/*!
 * \brief Structure describing a memory stats pointer
 *
 * \ingroup group_tivx_mem
 */
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
 * \param [in] mem_heap_region Memory region to which this allocation belongs, see \ref tivx_mem_heap_region_e
 * \param [out] mem_ptr Allocated memory pointer
 * \param [in] size Size of memory to allocate in bytes
 *
 * \ingroup group_tivx_mem
 */
vx_status tivxMemBufferAlloc(tivx_shared_mem_ptr_t *mem_ptr, uint32_t size, vx_enum mem_heap_region);

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
 * \param [in] mem_type Memory type to which this pointer belongs, see \ref vx_memory_type_e and \ref tivx_memory_type_e
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
 * \param [in] mem_type Memory type to which this pointer belongs, see \ref vx_memory_type_e and \ref tivx_memory_type_e
 * \param [in] maptype Mapping type as defined by \ref vx_accessor_e
 *
 * \ingroup group_tivx_mem
 */
void tivxMemBufferUnmap(void *host_ptr, uint32_t size, vx_enum mem_type, vx_enum maptype);

/*!
 * \brief Convert Host pointer to shared pointer
 *
 * \param [in] host_ptr Host memory pointer
 * \param [in] mem_heap_region Memory region to which this pointer belongs, see \ref tivx_mem_heap_region_e
 *
 * \return Converted shared memory pointer
 *
 * \ingroup group_tivx_mem
 */
uint64_t tivxMemHost2SharedPtr(uint64_t host_ptr, vx_enum mem_heap_region);

/*!
 * \brief Convert Shared memory pointer to host pointer
 *
 * \param [in] shared_ptr Shared memory pointer
 * \param [in] mem_heap_region Memory region to which this pointer belongs, see \ref tivx_mem_heap_region_e
 *
 * \return Converted host memory pointer
 *
 * \ingroup group_tivx_mem
 */
uint64_t tivxMemShared2HostPtr(uint64_t shared_ptr, vx_enum mem_heap_region);

/*!
 * \brief Convert shared pointer to target pointer
 *
 * \param [in] shared_ptr Host memory pointer
 * \param [in] mem_heap_region Memory region to which this pointer belongs, see \ref tivx_mem_heap_region_e
 *
 * \return Converted shared memory pointer
 *
 * \ingroup group_tivx_mem
 */
void* tivxMemShared2TargetPtr(uint64_t shared_ptr, vx_enum mem_heap_region);

/*!
 * \brief Convert target memory pointer to shared pointer
 *
 * \param [in] target_ptr Target memory pointer
 * \param [in] mem_heap_region Memory region to which this pointer belongs, see \ref tivx_mem_heap_region_e
 *
 * \return Converted host memory pointer
 *
 * \ingroup group_tivx_mem
 */
uint64_t tivxMemTarget2SharedPtr(void *target_ptr, vx_enum mem_heap_region);

/*!
 * \brief Allocates memory of given size
 *
 * \param [in] size     size of the memory to be allocated
 *
 * \return Pointer to the allocated memory
 *
 * \ingroup group_tivx_mem
 */
void *tivxMemAlloc(vx_uint32 size, vx_enum mem_heap_region);

/*!
 * \brief Frees already allocated memory
 *
 * \param [in] ptr  Pointer to the memory
 * \param [in] size size of the memory to be freed
 * \param [in] mem_heap_region Memory segment ID
 *
 * \ingroup group_tivx_mem
 */
void tivxMemFree(void *ptr, vx_uint32 size, vx_enum mem_heap_region);

/*!
 * \brief Get memory segement information
 *
 * \param [out] stats Memory segment information
 * \param [in] mem_heap_region Memory segment ID
 *
 * \ingroup group_tivx_mem
 */
void tivxMemStats(tivx_mem_stats *stats, vx_enum mem_type);

#ifdef __cplusplus
}
#endif

#endif
