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

/*! \brief Addresses and sizes of OCMC memories in the SOC
 * \ingroup group_tivx_mem
 */
#define OCMC_1_BASE_ADDRESS         (0x40300000U)
#define OCMC_1_SIZE                 (512 * 1024)

#define OCMC_2_BASE_ADDRESS         (0x40400000U)
#define OCMC_2_SIZE                 (1024 * 1024)

#define OCMC_3_BASE_ADDRESS         (0x40500000U)
#define OCMC_3_SIZE                 (1024 * 1024)


vx_status tivxMemBufferAlloc(
    tivx_shared_mem_ptr_t *mem_ptr, uint32_t size, vx_enum mem_heap_region)
{
    vx_status status = (vx_status)VX_SUCCESS;
    Utils_HeapId heap_id;

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
        switch (mem_heap_region)
        {
            case (vx_enum)TIVX_MEM_EXTERNAL:
            case (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH:
                heap_id = UTILS_HEAPID_DDR_CACHED_SR;
                break;
            case (vx_enum)TIVX_MEM_INTERNAL_L3:
                /* Since there is no L3 memory, so using OCMC memory */
                heap_id = UTILS_HEAPID_OCMC_SR;
                break;
            case (vx_enum)TIVX_MEM_INTERNAL_L1:
            case (vx_enum)TIVX_MEM_INTERNAL_L2:
                heap_id = UTILS_HEAPID_L2_LOCAL;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid memtype\n");
                status = (vx_status)VX_FAILURE;
                break;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            mem_ptr->host_ptr = (uintptr_t)Utils_memAlloc(
                heap_id, size, TIVX_MEM_BUFFER_ALLOC_ALIGN);

            if ((uintptr_t)NULL != mem_ptr->host_ptr)
            {
                mem_ptr->mem_heap_region = mem_heap_region;
                mem_ptr->shared_ptr = (uint64_t)tivxMemHost2SharedPtr(
                    mem_ptr->host_ptr, mem_heap_region);
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
    Utils_HeapId heap_id;
    tivx_cpu_id_e cpuId;
    void *ptr = NULL;

    switch (mem_heap_region)
    {
    case (vx_enum)TIVX_MEM_EXTERNAL:
    case (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH:
        heap_id = UTILS_HEAPID_DDR_CACHED_SR;
        break;
    case (vx_enum)TIVX_MEM_INTERNAL_L3:
        /* In case of EVE, L3 memory correspond to one of the OCMC memory.
         * We use hardcoded addresses because on EVE, only TI-DL use case needs allocation
         * in OCMC and the TI_DL implementation assumes that the entire OCMC memory is available for its consumption.
         * The only issue that can arise from using hard-coded address is in case On-the-fly capture from VIP is enabled
         * because it also uses OCMC_1. However there is no use-case up-to-date that combines both TI-DL and OTF capture.
         */
        cpuId= (tivx_cpu_id_e)tivxGetSelfCpuId();

        if (cpuId== (vx_enum)TIVX_CPU_ID_EVE1)
        {
            ptr = (void *)OCMC_1_BASE_ADDRESS;
            goto exit; /* Jump to exit-point because we don't want to call Utils_memAlloc */
        }
        else if (cpuId== (vx_enum)TIVX_CPU_ID_EVE2)
        {
            ptr = (void *)OCMC_2_BASE_ADDRESS;
            goto exit;
        }
        else if (cpuId== (vx_enum)TIVX_CPU_ID_EVE3)
        {
            ptr = (void *)OCMC_3_BASE_ADDRESS;
            goto exit;
        }
        else if (cpuId== (vx_enum)TIVX_CPU_ID_EVE4)
        {
            ptr = (void *)(OCMC_3_BASE_ADDRESS + (OCMC_3_SIZE/2));
            goto exit;
        }
        else if ((cpuId== (vx_enum)TIVX_CPU_ID_DSP1) || (cpuId== (vx_enum)TIVX_CPU_ID_DSP2))
        {
            heap_id = UTILS_HEAPID_DDR_CACHED_SR;
        }
        else
        {
            /* Since there is no L3 memory, so using OCMC memory */
            heap_id = UTILS_HEAPID_OCMC_SR;
        }
        break;
    case (vx_enum)TIVX_MEM_INTERNAL_L1:
    case (vx_enum)TIVX_MEM_INTERNAL_L2:
        heap_id = UTILS_HEAPID_L2_LOCAL;
        break;
    default:
        VX_PRINT(VX_ZONE_ERROR, "Invalid memtype\n");
        status = (vx_status)VX_FAILURE;
        break;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        ptr = Utils_memAlloc(heap_id, size, TIVX_MEM_BUFFER_ALLOC_ALIGN);
    }

    exit:
    return (ptr);
}

void tivxMemFree(void *ptr, vx_uint32 size, vx_enum mem_heap_region)
{
    vx_status status = (vx_status)VX_SUCCESS;
    Utils_HeapId heap_id;
    tivx_cpu_id_e cpuId;

    if ((NULL != ptr) && (0U != size))
    {
        switch (mem_heap_region)
        {
        case (vx_enum)TIVX_MEM_EXTERNAL:
        case (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH:
            heap_id = UTILS_HEAPID_DDR_CACHED_SR;
            break;
        case (vx_enum)TIVX_MEM_INTERNAL_L3:
            /* In case of EVE, L3 memory correspond to one of the OCMC memory.
             * We had used hardcoded addresses for the allocation and thus no call to Utils_memFree() must be made
             * */
            cpuId= (tivx_cpu_id_e)tivxGetSelfCpuId();
            if ((cpuId== (vx_enum)TIVX_CPU_ID_EVE1) || (cpuId== (vx_enum)TIVX_CPU_ID_EVE2) || (cpuId== (vx_enum)TIVX_CPU_ID_EVE3) || (cpuId== (vx_enum)TIVX_CPU_ID_EVE4))
            {
                goto exit;
            }
            else if ((cpuId== (vx_enum)TIVX_CPU_ID_DSP1) || (cpuId== (vx_enum)TIVX_CPU_ID_DSP2))
            {
                heap_id = UTILS_HEAPID_DDR_CACHED_SR;
            }
            else {
                /* Since there is no L3 memory, so using OCMC memory */
                heap_id = UTILS_HEAPID_OCMC_SR;
            }
            break;
        case (vx_enum)TIVX_MEM_INTERNAL_L1:
        case (vx_enum)TIVX_MEM_INTERNAL_L2:
            heap_id = UTILS_HEAPID_L2_LOCAL;
            break;
        default:
            VX_PRINT(VX_ZONE_ERROR, "Invalid memtype\n");
            status = (vx_status)VX_FAILURE;
            break;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            Utils_memFree(heap_id, ptr, size);
        }
    }
    exit:
    return;
}

vx_status tivxMemBufferFree(tivx_shared_mem_ptr_t *mem_ptr, uint32_t size)
{
    int32_t ret_val;
    vx_status status = (vx_status)VX_SUCCESS;
    Utils_HeapId heap_id;

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
        switch (mem_ptr->mem_heap_region)
        {
            case (vx_enum)TIVX_MEM_EXTERNAL:
            case (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH:
                heap_id = UTILS_HEAPID_DDR_CACHED_SR;
                break;
            case (vx_enum)TIVX_MEM_INTERNAL_L3:
                heap_id = UTILS_HEAPID_OCMC_SR;
                break;
            case (vx_enum)TIVX_MEM_INTERNAL_L1:
            case (vx_enum)TIVX_MEM_INTERNAL_L2:
                heap_id = UTILS_HEAPID_L2_LOCAL;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid memtype\n");
                status = (vx_status)VX_FAILURE;
                break;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            ret_val = Utils_memFree(
                heap_id, (void*)(uintptr_t)mem_ptr->shared_ptr, size);

            if (0 == ret_val)
            {
                mem_ptr->host_ptr = (uintptr_t)NULL;
                mem_ptr->shared_ptr = (uintptr_t)NULL;
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
    Utils_HeapId heap_id;
    Utils_MemHeapStats heap_stats;

    if (NULL == stats)
    {

    }
    else
    {
        stats->mem_size = 0;
        stats->free_size = 0;

        switch (mem_heap_region)
        {
            case (vx_enum)TIVX_MEM_EXTERNAL:
            case (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH:
                heap_id = UTILS_HEAPID_DDR_CACHED_SR;
                break;
            case (vx_enum)TIVX_MEM_INTERNAL_L3:
                heap_id = UTILS_HEAPID_OCMC_SR;
                break;
            case (vx_enum)TIVX_MEM_INTERNAL_L1:
            case (vx_enum)TIVX_MEM_INTERNAL_L2:
                heap_id = UTILS_HEAPID_L2_LOCAL;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid memtype\n");
                status = (vx_status)VX_FAILURE;
                break;
        }

        if ((vx_status)VX_SUCCESS == status)
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
            BspOsal_cacheInv(
                host_ptr,
                size,
                BSP_OSAL_CT_ALLD,
                BSP_OSAL_WAIT_FOREVER);
        }
    }
    else
    {
        status = (vx_status)VX_FAILURE;
        VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferMap failed (either pointer is NULL or size is 0)\n");
    }

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
            BspOsal_cacheWb(
                host_ptr,
                size,
                BSP_OSAL_CT_ALLD,
                BSP_OSAL_WAIT_FOREVER);
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
    /* For Bios implementation, host and shared pointers are same */
    return (host_ptr);
}

void* tivxMemShared2TargetPtr(const tivx_shared_mem_ptr_t *shared_ptr)
{
    /* For Bios implementation, host and shared pointers are same
     * However when used in Linux+BIOS mode, a translation maybe required
     * Utils_physToVirt abstracts this translation
     */
    return Utils_memPhysToVirt((void*)(uintptr_t)shared_ptr->shared_ptr);
}

uint64_t tivxMemShared2PhysPtr(uint64_t shared_ptr, vx_enum mem_heap_region)
{
    /* Currently it is same as shared pointer for bios */
    return (shared_ptr);
}


int32_t tivxMemResetScratchHeap(vx_enum mem_heap_region)
{
    vx_status status = (vx_status)VX_FAILURE;

    if ((vx_enum)TIVX_MEM_EXTERNAL_SCRATCH == mem_heap_region)
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

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        *fd      = (uint32_t)virtAddr;
        *phyAddr = (void*)(uintptr_t)tivxMemHost2SharedPtr((uint32_t)virtAddr,
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
        *virtAddr = (void*)(uintptr_t)dmaBufFd;
        *phyAddr = (void*)(uintptr_t)tivxMemHost2SharedPtr(dmaBufFd,
                                                           TIVX_MEM_EXTERNAL);
    }

    return vxStatus;
}

void tivxEnableL1DandL2CacheWb()
{
}
