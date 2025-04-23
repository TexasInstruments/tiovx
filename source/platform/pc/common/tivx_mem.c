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

#include <vx_internal.h>

#ifndef DISABLE_APP_UTILS_DEP
#include <utils/mem/include/app_mem.h>
#endif // ifndef DISABLE_APP_UTILS_DEP

/*! \brief Default buffer allocation padding
 * \ingroup group_tivx_mem
 * \details HOST_EMULATION will use linux allocations, and the same buffer will be used on simulated
 * DSP.  Some of the optimized DSP functions may do a vector read of multiple bytes of the
 * input (sometimes more than it needs) for optimal fetch.  This has a potential of out of
 * bounds reads which will segfault on HOST_EMULATION (this is not an issue on TARGET mode
 * of operation where DSP is running in an RTOS and is allowed to read past the buffer within
 * the memory carveout).  Therefore, this HOST_EMULATION_ALLOC_PAD variable can prevent
 * HOST_EMULATION segfaults in this situation */
#define HOST_EMULATION_ALLOC_PAD (8U)


/*! \brief Default buffer allocation alignment
 * \ingroup group_tivx_mem
 */
#define TIVX_MEM_BUFFER_ALLOC_ALIGN     (16U)


/*! \brief Default buffer alignment in L2RAM
 * \ingroup group_tivx_mem
 */
#define TIVX_MEM_L2RAM_ALIGN (1024)
#define TIVX_MEM_C7X_L2SRAM_ALIGN (512*1024)


/*! \brief Psuedo L2RAM size for DSP
 * \ingroup group_tivx_mem
 */
#if defined (SOC_J784S4)
#define TIVX_MEM_L2RAM_SIZE (4*10*1024*1024 + 8*1024*1024) /* 4 * 10 MB for 4 cores (L2/MSMC-L1) + 8 MB for MSMC-L3 */
#else
#define TIVX_MEM_L2RAM_SIZE (10*1024*1024)
#endif

/*! \brief Psuedo L2RAM memory for DSP
 * \ingroup group_tivx_mem
 */
static vx_uint8 gL2RAM_mem[TIVX_MEM_L2RAM_SIZE]
__attribute__ ((aligned(TIVX_MEM_L2RAM_ALIGN)))
    ;


/*! \brief Psuedo L2RAM memory allocation offset for DSP
 * \ingroup group_tivx_mem
 */
static vx_uint32 gL2RAM_mem_offset = 0;

vx_status tivxMemRegionTranslate (uint32_t mem_heap_region, uint32_t *heap_id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    switch (mem_heap_region)
        {
            case (vx_enum)TIVX_MEM_EXTERNAL:
                *heap_id = APP_MEM_HEAP_DDR;
                break;
            case (vx_enum)TIVX_MEM_INTERNAL_L3:
                *heap_id = APP_MEM_HEAP_L3;
                break;
            case (vx_enum)TIVX_MEM_INTERNAL_L2:
                *heap_id = APP_MEM_HEAP_L2;
                break;
            case (vx_enum)TIVX_MEM_INTERNAL_L1:
                *heap_id = APP_MEM_HEAP_L1;
                break;
            case (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH:
                *heap_id = APP_MEM_HEAP_DDR_SCRATCH;
                break;
            case (vx_enum)TIVX_MEM_EXTERNAL_PERSISTENT_NON_CACHEABLE:
                *heap_id = APP_MEM_HEAP_DDR_NON_CACHE;
                break;
            case (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH_NON_CACHEABLE:
                *heap_id = APP_MEM_HEAP_DDR_NON_CACHE_SCRATCH;
                break;
            case (vx_enum)TIVX_MEM_EXTERNAL_CACHEABLE_WT:
                *heap_id = APP_MEM_HEAP_DDR_WT_CACHE;
                break;

            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid memtype\n");
                status = (vx_status)VX_FAILURE;
                break;
        }
    return (vx_status)status;
}

vx_bool tivxMemRegionQuery (vx_enum mem_heap_region)
{
    vx_bool enabled = (vx_bool)vx_false_e;
    uint32_t heap_id = 0U;
    vx_status status = (vx_status)VX_SUCCESS;
    status = tivxMemRegionTranslate((uint32_t)mem_heap_region, &heap_id);
    if (status == VX_SUCCESS)
    {
        enabled = (appMemRegionQuery(heap_id)) ? (vx_bool)vx_true_e : (vx_bool)vx_false_e;
    }

    return enabled;
}

vx_status tivxMemBufferAlloc(
    tivx_shared_mem_ptr_t *mem_ptr, uint32_t size, vx_enum mem_heap_region)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t alloc_size = size+HOST_EMULATION_ALLOC_PAD;

    if ((NULL == mem_ptr) || (0U == size))
    {
        if (NULL == mem_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "Mem pointer is NULL\n");
        }
        if (0U == size)
        {
            VX_PRINT(VX_ZONE_ERROR, "size is 0\n");
        }
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        mem_ptr->mem_heap_region = mem_heap_region;

        mem_ptr->host_ptr = (uint64_t)(uintptr_t)tivxMemAlloc(alloc_size, (vx_enum)TIVX_MEM_EXTERNAL);

        mem_ptr->shared_ptr = mem_ptr->host_ptr;

        if(mem_ptr->host_ptr==(uint64_t)(uintptr_t)NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "Memory was not allocated\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    return (status);
}

void *tivxMemAlloc(vx_uint32 size, vx_enum mem_heap_region)
{
    void *ptr = NULL;

    if( ((vx_enum)(vx_enum)TIVX_MEM_EXTERNAL != mem_heap_region) && ((vx_enum)(vx_enum)TIVX_MEM_EXTERNAL_SCRATCH != mem_heap_region) &&
        ((vx_enum)(vx_enum)TIVX_MEM_EXTERNAL_PERSISTENT_NON_CACHEABLE != mem_heap_region) &&
        ((vx_enum)(vx_enum)TIVX_MEM_EXTERNAL_SCRATCH_NON_CACHEABLE != mem_heap_region) )
    {
        uint32_t mem_offset;

        if(TIVX_MEM_INTERNAL_L2 == mem_heap_region)
        {
            vx_uint64 base_ptr = (vx_uint64)&gL2RAM_mem[gL2RAM_mem_offset];
            vx_uint64 align_ptr = TIVX_ALIGN(base_ptr, (uint64_t)TIVX_MEM_C7X_L2SRAM_ALIGN);
            base_ptr = (vx_uint64)&gL2RAM_mem[0];
            mem_offset = align_ptr - base_ptr;
        }
        else
        {
            mem_offset = TIVX_ALIGN(gL2RAM_mem_offset, (uint32_t)TIVX_MEM_L2RAM_ALIGN);
        }
        /* L2RAM is used as scratch memory and allocation is linear offset based allocation */
        if((size + mem_offset) <= (uint32_t)TIVX_MEM_L2RAM_SIZE)
        {
            ptr = &gL2RAM_mem[mem_offset];
            gL2RAM_mem_offset = mem_offset + size;
        }
    }
    else
    {
#ifdef DISABLE_APP_UTILS_DEP
        ptr = malloc(size);
#else
        ptr = appMemAlloc(mem_heap_region, size, TIVX_MEM_BUFFER_ALLOC_ALIGN);
#endif // DISABLE_APP_UTILS_DEP
    }

    return (ptr);
}

vx_status tivxMemFree(void *ptr, vx_uint32 size, vx_enum mem_heap_region)
{
    vx_status status = VX_SUCCESS;

    if( ((vx_enum)(vx_enum)TIVX_MEM_EXTERNAL != mem_heap_region) && ((vx_enum)(vx_enum)TIVX_MEM_EXTERNAL_SCRATCH != mem_heap_region) )
    {
        /* L2RAM is used as scratch memory and allocation is linear offset based allocation
         * Free in this case resets the offset to 0
         */
        gL2RAM_mem_offset = 0;
    }
    else
    {
        if(ptr)
        {
#ifdef DISABLE_APP_UTILS_DEP
            free(ptr);
#else
            status = appMemFree(mem_heap_region, ptr, size);
#endif
        }
    }

    return status;
}

vx_status tivxMemBufferFree(tivx_shared_mem_ptr_t *mem_ptr, uint32_t size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t alloc_size = size+HOST_EMULATION_ALLOC_PAD;

    if ((NULL == mem_ptr) || (0U == size))
    {
        if (NULL == mem_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "Mem pointer is NULL\n");
        }
        if (0U == size)
        {
            VX_PRINT(VX_ZONE_ERROR, "size is 0\n");
        }
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        if(mem_ptr->host_ptr!=(uint64_t)(uintptr_t)NULL)
        {
            status = tivxMemFree((void*)(uintptr_t)mem_ptr->host_ptr, alloc_size, (vx_enum)TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

void tivxMemStats(tivx_mem_stats *stats, vx_enum mem_heap_region)
{
    if (NULL == stats)
    {

    }
    else
    {
        /* when memory segment information is not known set it to
         * 0
         */
        stats->mem_size = 0;
        stats->free_size = 0;

        if( ((vx_enum)(vx_enum)TIVX_MEM_EXTERNAL != mem_heap_region) && ((vx_enum)(vx_enum)TIVX_MEM_EXTERNAL_SCRATCH != mem_heap_region) )
        {
            stats->mem_size = TIVX_MEM_L2RAM_SIZE;
            stats->free_size = (uint32_t)TIVX_MEM_L2RAM_SIZE - gL2RAM_mem_offset;
        }
    }
}

vx_status tivxMemBufferMap(
    void *host_ptr, uint32_t size, vx_enum mem_type, vx_enum maptype)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((NULL == host_ptr) || (0U == size))
    {
        status = (vx_status)VX_FAILURE;
        VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferMap failed (either pointer is NULL or size is 0)\n");
    }

    return status;
}

vx_status ownMemBufferMap(
    void *host_ptr, uint32_t size, vx_enum mem_type, vx_enum maptype)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxMemBufferMap(host_ptr, size, mem_type, maptype);

    return status;
}

vx_status tivxMemBufferUnmap(
    void *host_ptr, uint32_t size, vx_enum mem_type, vx_enum maptype)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((NULL == host_ptr) || (0U == size))
    {
        status = (vx_status)VX_FAILURE;
        VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferUnmap failed (either pointer is NULL or size is 0)\n");
    }

    return status;
}

vx_status ownMemBufferUnmap(
    void *host_ptr, uint32_t size, vx_enum mem_type, vx_enum maptype)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxMemBufferUnmap(host_ptr, size, mem_type, maptype);

    return status;
}

uint64_t tivxMemHost2SharedPtr(uint64_t host_ptr, vx_enum mem_heap_region)
{
    uint64_t phys;

    phys = appMemGetVirt2PhyBufPtr(host_ptr, mem_heap_region);

    return phys;
}

void* tivxMemShared2TargetPtr(const tivx_shared_mem_ptr_t *shared_ptr)
{
    return (void*)(uintptr_t)(shared_ptr->shared_ptr);
}

#ifdef DISABLE_APP_UTILS_DEP
void* Utils_memPhysToVirt(uint64_t phys_ptr)
{
    return (void*)(uintptr_t)(phys_ptr);
}
#endif

uint64_t tivxMemShared2PhysPtr(uint64_t shared_ptr, vx_enum mem_heap_region)
{
    return (shared_ptr);
}

int32_t tivxMemResetScratchHeap(vx_enum mem_heap_region)
{
    vx_status status = (vx_status)VX_FAILURE;

    if (((vx_enum)TIVX_MEM_EXTERNAL_SCRATCH == mem_heap_region) || ((vx_enum)TIVX_MEM_EXTERNAL_SCRATCH_NON_CACHEABLE == mem_heap_region))
    {
        /* Return success since there is not scratch mem region on PC */
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "TIVX_MEM_EXTERNAL_SCRATCH is the only memory region supported\n");
    }

    return status;
}

vx_status tivxMemTranslateVirtAddr(const void *virtAddr, uint64_t *fd, void **phyAddr)
{
    vx_status   vxStatus = (vx_status)VX_SUCCESS;

    if (fd == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'fd' is NULL.\n");
        vxStatus = (vx_status)VX_FAILURE;
    }
    else if (phyAddr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'phyAddr' is NULL.\n");
        vxStatus = (vx_status)VX_FAILURE;
    }

#ifdef DISABLE_APP_UTILS_DEP
    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        *fd      = (uint64_t)virtAddr;
        *phyAddr = (void *)virtAddr;
    }
#else
    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        uint32_t dmaBufFdOffset;

        *fd = appMemGetDmaBufFd((void*)virtAddr, &dmaBufFdOffset);
        *phyAddr = (void *)(uintptr_t)tivxMemHost2SharedPtr((uint64_t)virtAddr,
                                                            TIVX_MEM_EXTERNAL);

        if ((*fd == (uint32_t)-1) || (*phyAddr == 0))
        {
            vxStatus = (vx_status)VX_FAILURE;
        }
    }
#endif

    return vxStatus;
}

vx_status tivxMemTranslateFd(uint64_t dmaBufFd, uint32_t size, void **virtAddr, void **phyAddr)
{
    vx_status   vxStatus = (vx_status)VX_SUCCESS;

    if (virtAddr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'virtAddr' is NULL.\n");
        vxStatus = (vx_status)VX_FAILURE;
    }
    else if (phyAddr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'phyAddr' is NULL.\n");
        vxStatus = (vx_status)VX_FAILURE;
    }

#ifdef DISABLE_APP_UTILS_DEP
    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        *virtAddr = (void*)(uintptr_t)dmaBufFd;
        *phyAddr  = (void*)(uintptr_t)dmaBufFd;
    }
#else
    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        int32_t status;

        status = appMemTranslateDmaBufFd(dmaBufFd,
                                         size,
                                         (uint64_t*)virtAddr,
                                         (uint64_t*)phyAddr);

        if (status < 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "appMemTranslateDmaBufFd() failed.\n");
            vxStatus = (vx_status)VX_FAILURE;
        }
    }
#endif

    return vxStatus;
}

vx_bool tivxMemCompareFd(uint64_t dmaBufFd1, uint64_t dmaBufFd2, uint32_t size1, uint32_t size2)
{
    vx_bool ret = (vx_bool)vx_false_e;

    if (size1 == size2)
    {
        vx_status status;
        void  *phyAddr1,  *phyAddr2;
        void  *virtAddr1, *virtAddr2;

        status  = tivxMemTranslateFd(dmaBufFd1, size1, &virtAddr1, &phyAddr1);
        status |= tivxMemTranslateFd(dmaBufFd2, size2, &virtAddr2, &phyAddr2);

        if ((vx_status)VX_SUCCESS == status)
        {
            if (phyAddr1 == phyAddr2)
            {
                ret = (vx_bool)vx_true_e;
            }
            else
            {
                ret = (vx_bool)vx_false_e;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMemTranslateFd() failed.\n");
        }
    }

    return ret;
}

void tivxEnableL1DandL2CacheWb()
{
}
