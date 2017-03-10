/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */
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

#include <src/utils_common/include/utils_mem_if.h>


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
                status = VX_FAILURE;
            }
        }
    }

    return (status);
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
    return (shared_ptr);
}

void* tivxMemTarget2SharedPtr(void *target_ptr, vx_enum mem_type)
{
    return (target_ptr);
}

