/*
*
* Copyright (c) 2025 Texas Instruments Incorporated
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

#include <utils/mem/include/app_mem.h>


/*! \brief Default buffer allocation alignment
 * \ingroup group_tivx_mem
 */
#define TIVX_MEM_BUFFER_ALLOC_ALIGN     (1024U)

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

            /* Waiver here: leaving in so that if someone adds a new type it gets flagged */
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
    if (status == (vx_status)VX_SUCCESS)
    {
        enabled = (appMemRegionQuery(heap_id)) ? (vx_bool)vx_true_e : (vx_bool)vx_false_e;
    }

    return enabled;
}

vx_status tivxMemBufferAlloc(
    tivx_shared_mem_ptr_t *mem_ptr, uint32_t size, vx_enum mem_heap_region)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t heap_id;

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
        status = (vx_status)tivxMemRegionTranslate((uint32_t)mem_heap_region, &heap_id);

        /* Waiver here: leaving in so that if someone adds a new type it gets flagged */
        if ((vx_status)VX_SUCCESS == status)
        {
            mem_ptr->host_ptr = (uintptr_t)appMemAlloc(
                heap_id, size, TIVX_MEM_BUFFER_ALLOC_ALIGN);

            if ((uint64_t)0 != mem_ptr->host_ptr)
            {
                mem_ptr->mem_heap_region = (uint32_t)mem_heap_region;
                mem_ptr->shared_ptr = (uint64_t)tivxMemHost2SharedPtr(
                    mem_ptr->host_ptr, mem_heap_region);
                mem_ptr->dma_buf_fd = (int32_t)appMemGetDmaBufFd(
                    (void*)(uintptr_t)(mem_ptr->host_ptr), &mem_ptr->dma_buf_fd_offset);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Shared mem ptr allocation failed\n");
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }
    }

    return (status);
}

void *tivxMemAlloc(vx_uint32 size, vx_enum mem_heap_region)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t heap_id;
    void *ptr = NULL;

    status = (vx_status)tivxMemRegionTranslate((uint32_t)mem_heap_region, &heap_id);

    if ((vx_status)VX_SUCCESS == status)
    {
        ptr = appMemAlloc(heap_id, size, TIVX_MEM_BUFFER_ALLOC_ALIGN);
    }

    return (ptr);
}

vx_status tivxMemFree(void *ptr, vx_uint32 size, vx_enum mem_heap_region)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t heap_id;

    status = (vx_status)tivxMemRegionTranslate((uint32_t)mem_heap_region, &heap_id);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = appMemFree(heap_id, ptr, size);

        if (0 != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Mem free failed\n");
        }
    }

    return status;
}

vx_status tivxMemBufferFree(tivx_shared_mem_ptr_t *mem_ptr, uint32_t size)
{
    int32_t ret_val;
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t heap_id;

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
        status = (vx_status)tivxMemRegionTranslate((uint32_t)mem_ptr->mem_heap_region, &heap_id);

        if ((vx_status)VX_SUCCESS == status)
        {
            ret_val = appMemFree(
                heap_id, (void*)(uintptr_t)mem_ptr->host_ptr, size);

            if (0 == ret_val)
            {
                #if defined(x86_64) || defined(QNX)
                appMemCloseDmaBufFd((uint64_t)mem_ptr->dma_buf_fd);
                #else
                appMemCloseDmaBufFd((uint32_t)mem_ptr->dma_buf_fd);
                #endif
                mem_ptr->dma_buf_fd = (int32_t)-1;
                mem_ptr->dma_buf_fd_offset = (uint32_t)0U;
                mem_ptr->host_ptr = 0;
                mem_ptr->shared_ptr = 0;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Shared ptr mem free failed\n");
                status = (vx_status)VX_FAILURE;
            }
        }
    }

    return (status);
}

void tivxMemStats(tivx_mem_stats *stats, vx_enum mem_heap_region)
{
    int32_t ret_val;
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t heap_id;
    app_mem_stats_t heap_stats;

    if (NULL == stats)
    {

    }
    else
    {
        stats->mem_size = 0;
        stats->free_size = 0;

        status = (vx_status)tivxMemRegionTranslate((uint32_t)mem_heap_region, &heap_id);

        if ((vx_status)VX_SUCCESS == status)
        {
            ret_val = appMemStats(heap_id, &heap_stats);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_MEM_UBR001
<justification end> */
            if (0 == ret_val)
            {
                stats->mem_size = heap_stats.heap_size;
                stats->free_size = heap_stats.free_size;
            }
/* LDRA_JUSTIFY_END */
        }
    }
}

vx_status tivxMemBufferMap(
    void *host_ptr, uint32_t size, vx_enum mem_type, vx_enum maptype)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* Note: Technically, we might be able to avoid a cache invalidate
     * if the maptype == VX_WRITE_ONLY, however if the mapping boundary splits
     * a cache line, then stale data outside the mapping, but on a cache
     * line that was mapped, could inadvertently be written back.  Therefore,
     * to be safe, we still perform invalidate even in WRITE only mode. */
    if ((NULL != host_ptr) && (0U != size))
    {
        if ((vx_enum)TIVX_MEMORY_TYPE_DMA != mem_type)
        {
            #if defined(SOC_AM62A)
            appMemCacheInv(host_ptr, size);
            #else
            #ifndef A72
            appMemCacheInv(host_ptr, size);
            #endif
            #endif
        }
    }
    else
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

    #if defined(QNX)
    status = tivxMemBufferMap(host_ptr, size, mem_type, maptype);
    #endif

    return status;
}

vx_status tivxMemBufferUnmap(
    void *host_ptr, uint32_t size, vx_enum mem_type, vx_enum maptype)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((NULL != host_ptr) && (0U != size))
    {
        if (((vx_enum)TIVX_MEMORY_TYPE_DMA != mem_type) &&
            (((vx_enum)VX_WRITE_ONLY == maptype) || ((vx_enum)VX_READ_AND_WRITE == maptype)))
        {
            #if defined(SOC_AM62A)
            appMemCacheWb(host_ptr, size);
            #else
            #ifndef A72
            appMemCacheWb(host_ptr, size);
            #endif
            #endif
        }
    }
    else
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

    #if defined(QNX)
    status = tivxMemBufferUnmap(host_ptr, size, mem_type, maptype);
    #endif

    return status;
}

uint64_t tivxMemHost2SharedPtr(uint64_t host_ptr, vx_enum mem_heap_region)
{
    uint32_t heap_id;
    uint64_t phys = 0;

    if((vx_status)VX_SUCCESS == tivxMemRegionTranslate((uint32_t)mem_heap_region, &heap_id))
    {
        phys = appMemGetVirt2PhyBufPtr(host_ptr, heap_id);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid mem_heap_region %d\n", mem_heap_region);
    }

    return phys;
}

void* tivxMemShared2TargetPtr(const tivx_shared_mem_ptr_t *shared_ptr)
{
#ifdef TARGET_HLOS
    return (void*)(uintptr_t)(shared_ptr->host_ptr);
#else
    return (void*)(uintptr_t)(appMemShared2TargetPtr(shared_ptr->shared_ptr));
#endif
}

uint64_t tivxMemShared2PhysPtr(uint64_t shared_ptr, vx_enum mem_heap_region)
{
    uint32_t heap_id;
    uint64_t phys = 0;

    if((vx_status)VX_SUCCESS == tivxMemRegionTranslate((uint32_t)mem_heap_region, &heap_id))
    {
        phys = appMemShared2PhysPtr(shared_ptr, heap_id);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid mem_heap_region %d\n", mem_heap_region);
    }
    return phys;
}

vx_status tivxMemResetScratchHeap(vx_enum mem_heap_region)
{
    vx_status status = (vx_status)VX_FAILURE;

    if ((vx_enum)TIVX_MEM_EXTERNAL_SCRATCH == mem_heap_region)
    {
        status = appMemResetScratchHeap(APP_MEM_HEAP_DDR_SCRATCH);
    }
    else if ((vx_enum)TIVX_MEM_EXTERNAL_SCRATCH_NON_CACHEABLE == mem_heap_region)
    {
        status = appMemResetScratchHeap(APP_MEM_HEAP_DDR_NON_CACHE_SCRATCH);
    }
    else if ((vx_enum)TIVX_MEM_INTERNAL_L2 == mem_heap_region)
    {
        status = appMemResetScratchHeap(APP_MEM_HEAP_L2);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Memory region is not supported\n");
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
    if (phyAddr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'phyAddr' is NULL.\n");
        vxStatus = (vx_status)VX_FAILURE;
    }

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        uint32_t dmaBufFdOffset;

        *fd = appMemGetDmaBufFd((void*)virtAddr, &dmaBufFdOffset);
        *phyAddr = (void *)(uintptr_t)tivxMemHost2SharedPtr((uint64_t)virtAddr,
                                                            (vx_enum)TIVX_MEM_EXTERNAL);

        if ((*fd == (uint32_t)-1) || /* TIOVX-1959- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_MEM_UBR003 */
        /* TIOVX-1959- LDRA Uncovered Branch Id: TIOVX_CODE_COVERAGE_RTOS_MEM_UM001 */
        (*phyAddr == (void*)0)) /* TIOVX-1959- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_MEM_UBR004 */
        {
            vxStatus = (vx_status)VX_FAILURE;
        }
    }

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
    if (phyAddr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'phyAddr' is NULL.\n");
        vxStatus = (vx_status)VX_FAILURE;
    }

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        int32_t status;
        #if defined(x86_64) || defined(QNX)
        status = (int32_t)appMemTranslateDmaBufFd(dmaBufFd,
                                         size,
                                         (uint64_t*)virtAddr,
                                         (uint64_t*)phyAddr);
        #else
        status = (int32_t)appMemTranslateDmaBufFd((uint32_t)dmaBufFd,
                                         size,
                                         (uint64_t*)virtAddr,
                                         (uint64_t*)phyAddr);
        #endif

#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_RTOS_TIVX_MEM_UBR005
<justification end> */
#endif
        if (status < 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "appMemTranslateDmaBufFd() failed.\n");
            vxStatus = (vx_status)VX_FAILURE;
        }
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_END */
#endif
    }

    return vxStatus;
}

void tivxEnableL1DandL2CacheWb(void)
{
    #if defined(__C7120__) && (defined(SOC_J784S4) || defined(SOC_J742S2))
    appMemEnableL1DandL2CacheWb();
    #endif
}

vx_bool tivxMemCompareFd(uint64_t dmaBufFd1, uint64_t dmaBufFd2, uint32_t size1, uint32_t size2)
{
    vx_bool ret = (vx_bool)vx_false_e;

    if (size1 == size2)
    {
        vx_status status;
        uint32_t temp_status;
        void  *phyAddr1,  *phyAddr2;
        void  *virtAddr1, *virtAddr2;

        status  = tivxMemTranslateFd(dmaBufFd1, size1, &virtAddr1, &phyAddr1);
        temp_status = (uint32_t)status | (uint32_t)tivxMemTranslateFd(dmaBufFd2, size2, &virtAddr2, &phyAddr2);
        status = (vx_status)temp_status;

#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_HOST_ONLY_MEM_UM001
<justification end> */
#endif
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
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_HOST_ONLY_MEM_UM001
<justification end> */
#endif
        else
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_END */
#endif
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMemTranslateFd() failed.\n");
        }
    }

    return ret;
}
