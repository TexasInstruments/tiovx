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
    vx_status status = VX_SUCCESS;

    if ((NULL == mem_ptr) || (0 == size))
    {
        if (NULL == mem_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferAlloc: Mem pointer is NULL\n");
        }
        if (0 == size)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferAlloc: size is 0\n");
        }
        status = VX_FAILURE;
    }
    else
    {
        mem_ptr->host_ptr = tivxMemAlloc(size, mem_heap_region);
        if (NULL != mem_ptr->host_ptr)
        {
            mem_ptr->mem_heap_region = mem_heap_region;
            mem_ptr->shared_ptr = (void *)tivxMemHost2SharedPtr(mem_ptr->host_ptr, mem_heap_region);

            memset(mem_ptr->host_ptr, 0, size);
            System_ovxCacheWb((unsigned int)mem_ptr->host_ptr, size);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferAlloc: Mem host pointer was not allocated\n");
            status = VX_ERROR_NO_MEMORY;
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
    vx_status status = VX_SUCCESS;

    if ((NULL == mem_ptr) || (mem_ptr->host_ptr == NULL) || (0 == size))
    {
        if (NULL == mem_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferAlloc: Mem pointer is NULL\n");
        }
        if (mem_ptr->host_ptr == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferAlloc: Mem host pointer is NULL\n");
        }
        if (0 == size)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferAlloc: size is 0\n");
        }
        status = VX_FAILURE;
    }
    else
    {
        tivxMemFree(mem_ptr->host_ptr, size, mem_ptr->mem_heap_region);

        mem_ptr->host_ptr = NULL;
        mem_ptr->shared_ptr = NULL;
    }

    return (status);
}

void tivxMemBufferMap(
    void *host_ptr, uint32_t size, vx_enum mem_type, vx_enum maptype)
{
    /* Note: Technically, we might be able to avoid a cache invalidate
     * if the maptype == VX_WRITE_ONLY, however if the mapping boundary splits
     * a cache line, then stale data outside the mapping, but on a cache
     * line that was mapped, could inadvertently be written back.  Therefore,
     * to be safe, we still perform invalidate even in WRITE only mode. */
    if ((NULL != host_ptr) && (0U != size) && (TIVX_MEMORY_TYPE_DMA != mem_type))
    {
        System_ovxCacheInv((unsigned int)host_ptr, size);
    }
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

void tivxMemBufferUnmap(
    void *host_ptr, uint32_t size, vx_enum mem_type, vx_enum maptype)
{
    if ((NULL != host_ptr) && (0 != size) && (TIVX_MEMORY_TYPE_DMA != mem_type) &&
        ((VX_WRITE_ONLY == maptype) || (VX_READ_AND_WRITE == maptype)))
    {
        System_ovxCacheWb((unsigned int)host_ptr, size);
    }
}

uint64_t tivxMemHost2SharedPtr(uint64_t host_ptr, vx_enum mem_heap_region)
{
    void *addr = NULL;

    if ((uint64_t)NULL != host_ptr)
    {
        addr = (void *)System_ovxVirt2Phys((uintptr_t)host_ptr);
    }

    return (uint64_t)(uintptr_t)(addr);
}

uint64_t tivxMemShared2HostPtr(uint64_t shared_ptr, vx_enum mem_heap_region)
{
    void *addr = NULL;

    if ((uint64_t)NULL != shared_ptr)
    {
        addr = (void *)System_ovxPhys2Virt((uintptr_t)shared_ptr);
    }

    return (uint64_t)(uintptr_t)(addr);
}

void* tivxMemShared2TargetPtr(uint64_t shared_ptr, vx_enum mem_heap_region)
{
    return (void*)(shared_ptr);
}

uint64_t tivxMemTarget2SharedPtr(void *target_ptr, vx_enum mem_heap_region)
{
    return (uint64_t)(target_ptr);
}

vx_status tivxMemInit(void)
{
    vx_status status = VX_SUCCESS;

    return (status);
}

void tivxMemDeInit(void)
{
}
