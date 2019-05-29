/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

#include <utils/mem/include/app_mem.h>


/*! \brief Default buffer allocation alignment
 * \ingroup group_tivx_mem
 */
#define TIVX_MEM_BUFFER_ALLOC_ALIGN     (1024U)

vx_status tivxMemBufferAlloc(
    tivx_shared_mem_ptr_t *mem_ptr, uint32_t size, vx_enum mem_heap_region)
{
    vx_status status = VX_SUCCESS;
    uint32_t heap_id;

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
        status = VX_FAILURE;
    }
    else
    {
        switch (mem_heap_region)
        {
            case TIVX_MEM_EXTERNAL:
                heap_id = APP_MEM_HEAP_DDR;
                break;
            case TIVX_MEM_INTERNAL_L3:
                heap_id = APP_MEM_HEAP_L3;
                break;
            case TIVX_MEM_INTERNAL_L2:
                heap_id = APP_MEM_HEAP_L2;
                break;
            case TIVX_MEM_INTERNAL_L1:
                heap_id = APP_MEM_HEAP_L1;
                break;
            case TIVX_MEM_EXTERNAL_SCRATCH:
                heap_id = APP_MEM_HEAP_DDR_SCRATCH;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid memtype\n");
                status = VX_FAILURE;
                break;
        }

        if (VX_SUCCESS == status)
        {
            mem_ptr->host_ptr = (uintptr_t)appMemAlloc(
                heap_id, size, TIVX_MEM_BUFFER_ALLOC_ALIGN);

            if ((uintptr_t)NULL != mem_ptr->host_ptr)
            {
                mem_ptr->mem_heap_region = mem_heap_region;
                mem_ptr->shared_ptr = (uint64_t)tivxMemHost2SharedPtr(
                    mem_ptr->host_ptr, mem_heap_region);
                mem_ptr->dma_buf_fd = (int32_t)appMemGetDmaBufFd(
                    (void*)(uintptr_t)(mem_ptr->host_ptr));
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Shared mem ptr allocation failed\n");
                status = VX_ERROR_NO_MEMORY;
            }
        }
    }

    return (status);
}

void *tivxMemAlloc(vx_uint32 size, vx_enum mem_heap_region)
{
    vx_status status = VX_SUCCESS;
    uint32_t heap_id;
    void *ptr = NULL;

    switch (mem_heap_region)
    {
        case TIVX_MEM_EXTERNAL:
            heap_id = APP_MEM_HEAP_DDR;
            break;
        case TIVX_MEM_INTERNAL_L3:
            heap_id = APP_MEM_HEAP_L3;
            break;
        case TIVX_MEM_INTERNAL_L2:
            heap_id = APP_MEM_HEAP_L2;
            break;
        case TIVX_MEM_INTERNAL_L1:
            heap_id = APP_MEM_HEAP_L1;
            break;
        case TIVX_MEM_EXTERNAL_SCRATCH:
            heap_id = APP_MEM_HEAP_DDR_SCRATCH;
            break;
        default:
            VX_PRINT(VX_ZONE_ERROR, "Invalid memtype\n");
            status = VX_FAILURE;
            break;
    }

    if (VX_SUCCESS == status)
    {
        ptr = appMemAlloc(heap_id, size, TIVX_MEM_BUFFER_ALLOC_ALIGN);
    }

    return (ptr);
}

void tivxMemFree(void *ptr, vx_uint32 size, vx_enum mem_heap_region)
{
    vx_status status = VX_SUCCESS;
    uint32_t heap_id;

    switch (mem_heap_region)
    {
        case TIVX_MEM_EXTERNAL:
            heap_id = APP_MEM_HEAP_DDR;
            break;
        case TIVX_MEM_INTERNAL_L3:
            heap_id = APP_MEM_HEAP_L3;
            break;
        case TIVX_MEM_INTERNAL_L2:
            heap_id = APP_MEM_HEAP_L2;
            break;
        case TIVX_MEM_INTERNAL_L1:
            heap_id = APP_MEM_HEAP_L1;
            break;
        case TIVX_MEM_EXTERNAL_SCRATCH:
            heap_id = APP_MEM_HEAP_DDR_SCRATCH;
            break;
        default:
            VX_PRINT(VX_ZONE_ERROR, "Invalid memtype\n");
            status = VX_FAILURE;
            break;
    }

    if (VX_SUCCESS == status)
    {
        appMemFree(heap_id, ptr, size);
    }
}

vx_status tivxMemBufferFree(tivx_shared_mem_ptr_t *mem_ptr, uint32_t size)
{
    int32_t ret_val;
    vx_status status = VX_SUCCESS;
    uint32_t heap_id;

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
        status = VX_FAILURE;
    }
    else
    {
        switch (mem_ptr->mem_heap_region)
        {
            case TIVX_MEM_EXTERNAL:
                heap_id = APP_MEM_HEAP_DDR;
                break;
            case TIVX_MEM_INTERNAL_L3:
                heap_id = APP_MEM_HEAP_L3;
                break;
            case TIVX_MEM_INTERNAL_L2:
                heap_id = APP_MEM_HEAP_L2;
                break;
            case TIVX_MEM_INTERNAL_L1:
                heap_id = APP_MEM_HEAP_L1;
                break;
            case TIVX_MEM_EXTERNAL_SCRATCH:
                heap_id = APP_MEM_HEAP_DDR_SCRATCH;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid memtype\n");
                status = VX_FAILURE;
                break;
        }

        if (VX_SUCCESS == status)
        {
            ret_val = appMemFree(
                heap_id, (void*)(uintptr_t)mem_ptr->host_ptr, size);

            if (0 == ret_val)
            {
                appMemCloseDmaBufFd(mem_ptr->dma_buf_fd);
                mem_ptr->dma_buf_fd = (int32_t)-1;
                mem_ptr->host_ptr = (uintptr_t)NULL;
                mem_ptr->shared_ptr = (uintptr_t)NULL;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Shared ptr mem free failed\n");
                status = VX_FAILURE;
            }
        }
    }

    return (status);
}

void tivxMemStats(tivx_mem_stats *stats, vx_enum mem_heap_region)
{
    int32_t ret_val;
    vx_status status = VX_SUCCESS;
    uint32_t heap_id;
    app_mem_stats_t heap_stats;

    if (NULL == stats)
    {

    }
    else
    {
        stats->mem_size = 0;
        stats->free_size = 0;

        switch (mem_heap_region)
        {
            case TIVX_MEM_EXTERNAL:
                heap_id = APP_MEM_HEAP_DDR;
                break;
            case TIVX_MEM_INTERNAL_L3:
                heap_id = APP_MEM_HEAP_L3;
                break;
            case TIVX_MEM_INTERNAL_L2:
                heap_id = APP_MEM_HEAP_L2;
                break;
            case TIVX_MEM_INTERNAL_L1:
                heap_id = APP_MEM_HEAP_L1;
                break;
            case TIVX_MEM_EXTERNAL_SCRATCH:
                heap_id = APP_MEM_HEAP_DDR_SCRATCH;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid memtype\n");
                status = VX_FAILURE;
                break;
        }

        if (VX_SUCCESS == status)
        {
            ret_val = appMemStats(heap_id, &heap_stats);

            if (0 == ret_val)
            {
                stats->mem_size = heap_stats.heap_size;
                stats->free_size = heap_stats.free_size;
            }
        }
    }
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
        appMemCacheInv(
            host_ptr,
            size);
    }
}

void tivxMemBufferUnmap(
    void *host_ptr, uint32_t size, vx_enum mem_type, vx_enum maptype)
{
    if ((NULL != host_ptr) && (0U != size) && (TIVX_MEMORY_TYPE_DMA != mem_type) &&
        ((VX_WRITE_ONLY == maptype) || (VX_READ_AND_WRITE == maptype)))
    {
        appMemCacheWb(
            host_ptr,
            size);
    }
}

uint64_t tivxMemHost2SharedPtr(uint64_t host_ptr, vx_enum mem_heap_region)
{
    uint32_t heap_id;
    vx_status status = VX_SUCCESS;
    uint64_t phys = 0;

    switch (mem_heap_region)
    {
        case TIVX_MEM_EXTERNAL:
            heap_id = APP_MEM_HEAP_DDR;
            break;
        case TIVX_MEM_INTERNAL_L3:
            heap_id = APP_MEM_HEAP_L3;
            break;
        case TIVX_MEM_INTERNAL_L2:
            heap_id = APP_MEM_HEAP_L2;
            break;
        case TIVX_MEM_INTERNAL_L1:
            heap_id = APP_MEM_HEAP_L1;
            break;
        case TIVX_MEM_EXTERNAL_SCRATCH:
            heap_id = APP_MEM_HEAP_DDR_SCRATCH;
            break;
        default:
            VX_PRINT(VX_ZONE_ERROR, "Invalid memtype\n");
            status = VX_FAILURE;
            break;
    }
    if(status == VX_SUCCESS)
    {
        phys = appMemGetVirt2PhyBufPtr(host_ptr, heap_id);
    }
    return phys;
}

void* tivxMemShared2TargetPtr(uint64_t shared_ptr, vx_enum mem_heap_region)
{
    /* TODO need to fill for Linux
     */
    return (void*)(uintptr_t)(shared_ptr);
}

uint64_t tivxMemShared2PhysPtr(uint64_t shared_ptr, vx_enum mem_heap_region)
{
    /* Currently it is same as shared pointer for bios */
    return (shared_ptr);
}

