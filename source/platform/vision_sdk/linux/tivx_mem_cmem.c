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
#include <cmem.h>
#include <src/hlos/system/system_priv_openvx.h>

#define MEM_ALLOC_ALIGN      (63U)
#define MEM_BUFFER_ALLOC_ALIGN (64U)

vx_status tivxMemBufferAlloc(
    tivx_shared_mem_ptr_t *mem_ptr, uint32_t size, vx_enum mem_heap_region)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 block_id;
    CMEM_AllocParams prms;

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
        switch (mem_heap_region)
        {
            case TIVX_MEM_EXTERNAL:
                /* Assuming block id 0 is used for external memory */
                block_id = 0;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferAlloc: invalid mem type\n");
                status = VX_FAILURE;
                break;
        }

        if (VX_SUCCESS == status)
        {
            prms.type = CMEM_HEAP;
            prms.flags = CMEM_CACHED;
            prms.alignment = MEM_BUFFER_ALLOC_ALIGN;

            size = (size + MEM_ALLOC_ALIGN) & ~(MEM_ALLOC_ALIGN);
            mem_ptr->host_ptr = CMEM_alloc2(block_id, size, &prms);
            if (NULL != mem_ptr->host_ptr)
            {
                mem_ptr->mem_heap_region = mem_heap_region;
                mem_ptr->shared_ptr = (void *)CMEM_getPhys(mem_ptr->host_ptr);

                memset(mem_ptr->host_ptr, 0, size);
                CMEM_cacheWb(mem_ptr->host_ptr, size);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferAlloc: host pointer could not be allocated\n");
                status = VX_ERROR_NO_MEMORY;
            }
        }
    }

    return (status);
}

void *tivxMemAlloc(vx_uint32 size, vx_enum mem_heap_region)
{
    void *ptr = NULL;
    CMEM_AllocParams prms;

    prms.type = CMEM_HEAP;
    prms.flags = CMEM_CACHED;
    prms.alignment = MEM_BUFFER_ALLOC_ALIGN;

    ptr = CMEM_alloc2(0, size, &prms);

    return (ptr);
}

void tivxMemFree(void *ptr, vx_uint32 size, vx_enum mem_heap_region)
{
    CMEM_AllocParams prms;

    if ((NULL != ptr) && (0 != size))
    {
        prms.type = CMEM_HEAP;
        prms.flags = CMEM_CACHED;
        prms.alignment = MEM_BUFFER_ALLOC_ALIGN;

        CMEM_free(ptr, &prms);
    }
}

vx_status tivxMemBufferFree(tivx_shared_mem_ptr_t *mem_ptr, uint32_t size)
{
    vx_int32 ret_val;
    vx_status status = VX_SUCCESS;
    CMEM_AllocParams prms;

    if ((NULL == mem_ptr) || (0 == size))
    {
        if (NULL == mem_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferFree: Mem pointer is NULL\n");
        }
        if (0 == size)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferFree: size is 0\n");
        }
        status = VX_FAILURE;
    }
    else
    {
        prms.type = CMEM_HEAP;
        prms.flags = CMEM_CACHED;
        prms.alignment = MEM_BUFFER_ALLOC_ALIGN;

        ret_val = CMEM_free(mem_ptr->host_ptr, &prms);

        if (0 == ret_val)
        {
            mem_ptr->host_ptr = NULL;
            mem_ptr->shared_ptr = NULL;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferFree: Host pointer mem free failed\n");
            status = VX_FAILURE;
        }
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
        if (System_ovxIsValidCMemVirtAddr((unsigned int)host_ptr))
        {
            CMEM_cacheInv(host_ptr, size);
        }
        else
        {
            System_ovxCacheInv((unsigned int)host_ptr, size);
        }
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
        if (System_ovxIsValidCMemVirtAddr((unsigned int)host_ptr))
        {
            CMEM_cacheWb(host_ptr, size);
        }
        else
        {
            System_ovxCacheWb((unsigned int)host_ptr, size);
        }
    }
}

uint64_t tivxMemHost2SharedPtr(uint64_t host_ptr, vx_enum mem_heap_region)
{
    void *addr = NULL;

    if (NULL != host_ptr)
    {
        if (System_ovxIsValidCMemVirtAddr((uintptr_t)host_ptr))
        {
            addr = (void *)CMEM_getPhys((void*)(uintptr_t)host_ptr);
        }
        else
        {
            addr = (void *)System_ovxVirt2Phys((uintptr_t)host_ptr);
        }
    }

    return (uint64_t)(uintptr_t)(addr);
}

uint64_t tivxMemShared2HostPtr(uint64_t shared_ptr, vx_enum mem_heap_region)
{
    void *addr = NULL;

    if (NULL != shared_ptr)
    {
        if (!System_ovxIsValidCMemPhysAddr((uintptr_t)shared_ptr))
        {
            addr = (void *)System_ovxPhys2Virt((uintptr_t)shared_ptr);
        }
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
    vx_int32 ret_val, version;
    vx_status status = VX_SUCCESS;

    ret_val = CMEM_init();
    if (ret_val == -1)
    {
        VX_PRINT(VX_ZONE_ERROR, " tivxMemInit: CMEM_init Failed !!!\n");
        status = VX_FAILURE;
    }
    else
    {
        version = CMEM_getVersion();
        if (-1 == version)
        {
            fprintf(stderr, "Failed to retrieve CMEM version\n");
            exit(EXIT_FAILURE);
        }
    }
    return (status);
}

void tivxMemDeInit(void)
{
    vx_int32 ret_val;

    ret_val = CMEM_exit();
    if (ret_val < 0)
    {
        tivxPlatformPrintf(" tivxMemDeInit: CMEM_exit Failed \n");
    }
}
