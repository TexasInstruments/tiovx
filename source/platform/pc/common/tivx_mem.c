/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <vx_internal.h>

/*! \brief Default buffer allocation alignment
 * \ingroup group_tivx_mem
 */
#define TIVX_MEM_BUFFER_ALLOC_ALIGN     (16U)


vx_status tivxMemBufferAlloc(
    tivx_shared_mem_ptr_t *mem_ptr, uint32_t size, vx_enum mem_type)
{
    vx_status status = VX_SUCCESS;

    mem_ptr->mem_type = mem_type;

    mem_ptr->host_ptr = tivxMemAlloc(size, TIVX_MEM_EXTERNAL);

    mem_ptr->shared_ptr = mem_ptr->host_ptr;
    mem_ptr->target_ptr = mem_ptr->host_ptr;

    if(mem_ptr->host_ptr==NULL)
    {
        status = VX_ERROR_NO_MEMORY;
    }

    return (status);
}

void *tivxMemAlloc(vx_uint32 size, vx_enum mem_type)
{
    void *ptr = NULL;

    ptr = malloc(size);

    return (ptr);
}

void tivxMemFree(void *ptr, vx_uint32 size, vx_enum mem_type)
{
    if(ptr)
    {
        free(ptr);
    }
}

vx_status tivxMemBufferFree(tivx_shared_mem_ptr_t *mem_ptr, uint32_t size)
{
    vx_status status = VX_SUCCESS;

    if(mem_ptr->host_ptr)
    {
        tivxMemFree(mem_ptr->host_ptr, size, TIVX_MEM_EXTERNAL);
    }

    return (status);
}

void tivxMemBufferMap(
    void *host_ptr, uint32_t size, vx_enum mem_type, vx_enum maptype)
{
}

void tivxMemBufferUnmap(
    void *host_ptr, uint32_t size, vx_enum mem_type, vx_enum maptype)
{
}

void *tivxMemHost2SharedPtr(void *host_ptr, vx_enum mem_type)
{
    return (host_ptr);
}

void *tivxMemShared2HostPtr(void *shared_ptr, vx_enum mem_type)
{
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

