/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
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
    tivx_shared_mem_ptr_t *mem_ptr, uint32_t size, vx_enum mem_heap_region)
{
    vx_status status = VX_SUCCESS;

    mem_ptr->mem_heap_region = mem_heap_region;

    mem_ptr->host_ptr = (uint64_t)tivxMemAlloc(size, TIVX_MEM_EXTERNAL);

    mem_ptr->shared_ptr = mem_ptr->host_ptr;

    if(mem_ptr->host_ptr==(uint64_t)NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferAlloc: Memory was not allocated\n");
        status = VX_ERROR_NO_MEMORY;
    }

    return (status);
}

void *tivxMemAlloc(vx_uint32 size, vx_enum mem_heap_region)
{
    void *ptr = NULL;

    if(mem_heap_region==TIVX_MEM_INTERNAL_L2)
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

void tivxMemFree(void *ptr, vx_uint32 size, vx_enum mem_heap_region)
{
    if(ptr)
    {
        if(mem_heap_region==TIVX_MEM_INTERNAL_L2)
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

    if(mem_ptr->host_ptr!=(uint64_t)NULL)
    {
        tivxMemFree((void*)mem_ptr->host_ptr, size, TIVX_MEM_EXTERNAL);
    }

    return (status);
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

        if(mem_heap_region==TIVX_MEM_INTERNAL_L2)
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

uint64_t tivxMemHost2SharedPtr(uint64_t host_ptr, vx_enum mem_heap_region)
{
    return (host_ptr);
}

uint64_t tivxMemShared2HostPtr(uint64_t shared_ptr, vx_enum mem_heap_region)
{
    return (shared_ptr);
}

void* tivxMemShared2TargetPtr(uint64_t shared_ptr, vx_enum mem_heap_region)
{
    return (void*)(shared_ptr);
}

uint64_t tivxMemTarget2SharedPtr(void *target_ptr, vx_enum mem_heap_region)
{
    return (uint64_t)(target_ptr);
}

