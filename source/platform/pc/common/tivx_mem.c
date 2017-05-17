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

/*! \brief Psuedo L2RAM size for DSP
 * \ingroup group_tivx_mem
 */
#define TIVX_MEM_L2RAM_SIZE (256*1024)

/*! \brief Psuedo L2RAM memory for DSP
 * \ingroup group_tivx_mem
 */
static vx_uint8 gL2RAM_mem[TIVX_MEM_L2RAM_SIZE];

/*! \brief Psuedo L2RAM memory allocation offset for DSP
 * \ingroup group_tivx_mem
 */
static vx_uint32 gL2RAM_mem_offset = 0;

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

    if(mem_type==TIVX_MEM_INTERNAL_L2)
    {
        /* L2RAM is used as scratch memory and allocation is linear offset based allocation */
        if(size+gL2RAM_mem_offset <= TIVX_MEM_L2RAM_SIZE)
        {
            ptr = &gL2RAM_mem[gL2RAM_mem_offset];

            gL2RAM_mem_offset += size;
        }
    }
    else
    {
        ptr = malloc(size);
    }

    return (ptr);
}

void tivxMemFree(void *ptr, vx_uint32 size, vx_enum mem_type)
{
    if(ptr)
    {
        if(mem_type==TIVX_MEM_INTERNAL_L2)
        {
            /* L2RAM is used as scratch memory and allocation is linear offset based allocation
             * Free in this case resets the offset to 0
             */
            gL2RAM_mem_offset = 0;
        }
        else
        {
            free(ptr);
        }
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

void tivxMemStats(tivx_mem_stats *stats, vx_enum mem_type)
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

        if(mem_type==TIVX_MEM_INTERNAL_L2)
        {
            stats->mem_size = TIVX_MEM_L2RAM_SIZE;
            stats->free_size = TIVX_MEM_L2RAM_SIZE - gL2RAM_mem_offset;
        }
    }
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

