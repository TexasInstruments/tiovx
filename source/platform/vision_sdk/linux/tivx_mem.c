/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <vx_internal.h>
#include <tivx_platform_vision_sdk.h>
#include <sys/types.h>
#include <src/hlos/system/system_priv_openvx.h>

vx_status tivxMemBufferAlloc(
    tivx_shared_mem_ptr_t *mem_ptr, uint32_t size, vx_enum mem_heap_region)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((NULL == mem_ptr) || (0U == size))
    {
        if (NULL == mem_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "Mem pointer is NULL\n");
        }
        if (0 == size)
        {
            VX_PRINT(VX_ZONE_ERROR, "size is 0\n");
        }
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        mem_ptr->host_ptr = (uintptr_t)tivxMemAlloc(size, mem_heap_region);
        if ((uintptr_t)NULL != mem_ptr->host_ptr)
        {
            mem_ptr->mem_heap_region = mem_heap_region;
            mem_ptr->shared_ptr = (uint64_t)tivxMemHost2SharedPtr(mem_ptr->host_ptr, mem_heap_region);

            memset((void *)(uintptr_t)mem_ptr->host_ptr, 0, size);
            System_ovxCacheWb((unsigned int)mem_ptr->host_ptr, size);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Mem host pointer was not allocated\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    return (status);
}

void *tivxMemAlloc(vx_uint32 size, vx_enum mem_heap_region)
{
    void *ptr = NULL;

    ptr = System_ovxAllocMem(size);

    return (ptr);
}

void tivxMemFree(void *ptr, vx_uint32 size, vx_enum mem_heap_region)
{
    if ((NULL != ptr) && (0 != size))
    {
        System_ovxFreeMem(ptr, size);
    }
}

vx_status tivxMemBufferFree(tivx_shared_mem_ptr_t *mem_ptr, uint32_t size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((NULL == mem_ptr) || (mem_ptr->host_ptr == (uintptr_t)NULL) || (0U == size))
    {
        if (NULL == mem_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "Mem pointer is NULL\n");
        }
        if (mem_ptr->host_ptr == (uintptr_t)NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "Mem host pointer is NULL\n");
        }
        if (0 == size)
        {
            VX_PRINT(VX_ZONE_ERROR, "size is 0\n");
        }
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        tivxMemFree((void *)(uintptr_t)mem_ptr->host_ptr, size, mem_ptr->mem_heap_region);

        mem_ptr->host_ptr = (uintptr_t)NULL;
        mem_ptr->shared_ptr = (uintptr_t)NULL;
    }

    return (status);
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
            System_ovxCacheInv((unsigned int)host_ptr, size);
        }
    }
    else
    {
        status = (vx_status)VX_FAILURE;
        VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferMap failed (either pointer is NULL or size is 0)\n");
    }

    return status;
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
    }
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
            System_ovxCacheWb((unsigned int)host_ptr, size);
        }
    }
    else
    {
        status = (vx_status)VX_FAILURE;
        VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferUnmap failed (either pointer is NULL or size is 0)\n");
    }

    return status;
}

uint64_t tivxMemHost2SharedPtr(uint64_t host_ptr, vx_enum mem_heap_region)
{
    void *addr = NULL;

    if ((uint64_t)(uintptr_t)NULL != host_ptr)
    {
        addr = (void *)System_ovxVirt2Phys((uintptr_t)host_ptr);
    }

    return (uint64_t)(uintptr_t)(addr);
}

void* tivxMemShared2TargetPtr(const tivx_shared_mem_ptr_t *shared_ptr)
{
    return (void*)(uintptr_t)(shared_ptr->host_ptr);
}

uint64_t tivxMemShared2PhysPtr(uint64_t shared_ptr, vx_enum mem_heap_region)
{
    /* Currently it is same as shared pointer for bios */
    return (shared_ptr);
}

vx_status tivxMemInit(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return (status);
}

void tivxMemDeInit(void)
{
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

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        /* TODO: Review this assumption. */
        *fd      = (uint64_t *)virtAddr;
        *phyAddr = (void*)(uintptr_t)tivxMemHost2SharedPtr((uint64_t)virtAddr,
                                                           TIVX_MEM_EXTERNAL);
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
    else if (phyAddr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'phyAddr' is NULL.\n");
        vxStatus = (vx_status)VX_FAILURE;
    }

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        /* TODO: Review this assumption. */
        *virtAddr = (void*)(uintptr_t)dmaBufFd;
        *phyAddr = (void*)(uintptr_t)tivxMemHost2SharedPtr(dmaBufFd,
                                                           TIVX_MEM_EXTERNAL);
    }

    return vxStatus;
}

void tivxEnableL1DandL2CacheWb()
{
}
