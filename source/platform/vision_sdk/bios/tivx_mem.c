/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <vx_internal.h>

#include <xdc/std.h>
#include <osal/bsp_osal.h>

#include <src/rtos/utils_common/include/utils_mem_if.h>


/*! \brief Default buffer allocation alignment
 * \ingroup group_tivx_mem
 */
#define TIVX_MEM_BUFFER_ALLOC_ALIGN     (16U)


vx_status tivxMemBufferAlloc(
    tivx_shared_mem_ptr_t *mem_ptr, uint32_t size, vx_enum mem_type)
{
    vx_status status = VX_SUCCESS;
    Utils_HeapId heap_id;

    if ((NULL == mem_ptr) || (0U == size))
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
        switch (mem_type)
        {
            case TIVX_MEM_EXTERNAL:
                heap_id = UTILS_HEAPID_DDR_CACHED_SR;
                break;
            case TIVX_MEM_INTERNAL_L3:
                /* Since there is no L3 memory, so using OCMC memory */
                heap_id = UTILS_HEAPID_OCMC_SR;
                break;
            case TIVX_MEM_INTERNAL_L2:
                heap_id = UTILS_HEAPID_L2_LOCAL;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferAlloc: Invalid memtype\n");
                status = VX_FAILURE;
                break;
        }

        if (VX_SUCCESS == status)
        {
            mem_ptr->shared_ptr = Utils_memAlloc(
                heap_id, size, TIVX_MEM_BUFFER_ALLOC_ALIGN);

            if (NULL != mem_ptr->shared_ptr)
            {
                mem_ptr->mem_type = mem_type;
                mem_ptr->host_ptr = tivxMemShared2HostPtr(
                    mem_ptr->shared_ptr, mem_type);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferAlloc: Shared mem ptr allocation failed\n");
                status = VX_ERROR_NO_MEMORY;
            }
        }
    }

    return (status);
}

void *tivxMemAlloc(vx_uint32 size, vx_enum mem_type)
{
    vx_status status = VX_SUCCESS;
    Utils_HeapId heap_id;
    void *ptr = NULL;

    switch (mem_type)
    {
        case TIVX_MEM_EXTERNAL:
            heap_id = UTILS_HEAPID_DDR_CACHED_SR;
            break;
        case TIVX_MEM_INTERNAL_L3:
            /* Since there is no L3 memory, so using OCMC memory */
            heap_id = UTILS_HEAPID_OCMC_SR;
            break;
        case TIVX_MEM_INTERNAL_L2:
            heap_id = UTILS_HEAPID_L2_LOCAL;
            break;
        default:
            VX_PRINT(VX_ZONE_ERROR, "tivxMemAlloc: Invalid memtype\n");
            status = VX_FAILURE;
            break;
    }

    if (VX_SUCCESS == status)
    {
        ptr = Utils_memAlloc(heap_id, size, TIVX_MEM_BUFFER_ALLOC_ALIGN);
    }

    return (ptr);
}

void tivxMemFree(void *ptr, vx_uint32 size, vx_enum mem_type)
{
    vx_status status = VX_SUCCESS;
    Utils_HeapId heap_id;

    if ((NULL != ptr) && (0U != size))
    {
        switch (mem_type)
        {
            case TIVX_MEM_EXTERNAL:
                heap_id = UTILS_HEAPID_DDR_CACHED_SR;
                break;
            case TIVX_MEM_INTERNAL_L3:
                /* Since there is no L3 memory, so using OCMC memory */
                heap_id = UTILS_HEAPID_OCMC_SR;
                break;
            case TIVX_MEM_INTERNAL_L2:
                heap_id = UTILS_HEAPID_L2_LOCAL;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "tivxMemAlloc: Invalid memtype\n");
                status = VX_FAILURE;
                break;
        }

        if (VX_SUCCESS == status)
        {
            Utils_memFree(heap_id, ptr, size);
        }
    }
}

vx_status tivxMemBufferFree(tivx_shared_mem_ptr_t *mem_ptr, uint32_t size)
{
    int32_t ret_val;
    vx_status status = VX_SUCCESS;
    Utils_HeapId heap_id;

    if ((NULL == mem_ptr) || (0U == size))
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
        switch (mem_ptr->mem_type)
        {
            case TIVX_MEM_EXTERNAL:
                heap_id = UTILS_HEAPID_DDR_CACHED_SR;
                break;
            case TIVX_MEM_INTERNAL_L3:
                heap_id = UTILS_HEAPID_OCMC_SR;
                break;
            case TIVX_MEM_INTERNAL_L2:
                heap_id = UTILS_HEAPID_L2_LOCAL;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferFree: Invalid memtype\n");
                status = VX_FAILURE;
                break;
        }

        if (VX_SUCCESS == status)
        {
            ret_val = Utils_memFree(
                heap_id, mem_ptr->shared_ptr, size);

            if (0 == ret_val)
            {
                mem_ptr->host_ptr = NULL;
                mem_ptr->shared_ptr = NULL;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferFree: Shared ptr mem free failed\n");
                status = VX_FAILURE;
            }
        }
    }

    return (status);
}

void tivxMemStats(tivx_mem_stats *stats, vx_enum mem_type)
{
    int32_t ret_val;
    vx_status status = VX_SUCCESS;
    Utils_HeapId heap_id;
    Utils_MemHeapStats heap_stats;

    if (NULL == stats)
    {

    }
    else
    {
        stats->mem_size = 0;
        stats->free_size = 0;

        switch (mem_type)
        {
            case TIVX_MEM_EXTERNAL:
                heap_id = UTILS_HEAPID_DDR_CACHED_SR;
                break;
            case TIVX_MEM_INTERNAL_L3:
                heap_id = UTILS_HEAPID_OCMC_SR;
                break;
            case TIVX_MEM_INTERNAL_L2:
                heap_id = UTILS_HEAPID_L2_LOCAL;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "tivxMemStats: Invalid memtype\n");
                status = VX_FAILURE;
                break;
        }

        if (VX_SUCCESS == status)
        {
            ret_val = Utils_memGetHeapStats(heap_id, &heap_stats);

            if (0 == ret_val)
            {
                stats->mem_size = heap_stats.heapSize;
                stats->free_size = heap_stats.freeSize;
            }
        }
    }
}

void tivxMemBufferMap(
    void *host_ptr, uint32_t size, vx_enum mem_type, vx_enum maptype)
{
    if ((NULL != host_ptr) && (0U != size))
    {
        BspOsal_cacheInv(
            host_ptr,
            size,
            BSP_OSAL_CT_ALLD,
            BSP_OSAL_WAIT_FOREVER);
    }
}

void tivxMemBufferUnmap(
    void *host_ptr, uint32_t size, vx_enum mem_type, vx_enum maptype)
{
    if ((NULL != host_ptr) && (0U != size) &&
        ((VX_WRITE_ONLY == maptype) || (VX_READ_AND_WRITE == maptype)))
    {
        BspOsal_cacheWb(
            host_ptr,
            size,
            BSP_OSAL_CT_ALLD,
            BSP_OSAL_WAIT_FOREVER);
    }
}

void *tivxMemHost2SharedPtr(void *host_ptr, vx_enum mem_type)
{
    /* For Bios implementation, host and shared pointers are same */
    return (host_ptr);
}

void *tivxMemShared2HostPtr(void *shared_ptr, vx_enum mem_type)
{
    /* For Bios implementation, host and shared pointers are same */
    return (shared_ptr);
}

void* tivxMemShared2TargetPtr(void *shared_ptr, vx_enum mem_type)
{
    /* For Bios implementation, host and shared pointers are same
     * However when used in Linux+BIOS mode, a translation maybe required
     * Utils_physToVirt abstracts this translation
     */
    return Utils_memPhysToVirt(shared_ptr);
}

void* tivxMemTarget2SharedPtr(void *target_ptr, vx_enum mem_type)
{
    return (target_ptr);
}

