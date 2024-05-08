/*
 *
 * Copyright (c) 2024 Texas Instruments Incorporated
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

#include "test_utils_mem_operations.h"

#define MB (1024 * 1024)
#define KB (1024)

uint32_t link_index = 0;

/* Function to create and push data to the end of the list */
static vx_status pushback_link(tivx_shared_mem_info_t ** head, tivx_shared_mem_ptr_t shared_mem_ptr, vx_uint32 size)
{
    vx_status status = VX_FAILURE;
    tivx_shared_mem_info_t * newLink = tivxMemAlloc(sizeof(tivx_shared_mem_info_t), TIVX_MEM_EXTERNAL);
    if (newLink == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "Cannot alloc new link for the shared mem info linked list!\n");
        status = VX_FAILURE;
    }
    else
    {
        newLink->shared_mem_ptr = shared_mem_ptr;
        newLink->size = size;
        newLink->next = NULL;

        /* First Link */
        if (*head == NULL)
        {
            *head = newLink;
            status = VX_SUCCESS;
        }
        else
        {
            /* Iterate till last node */
            tivx_shared_mem_info_t * current = * head;
            while (current->next != NULL)
            {
                current = current->next;
            }
            current->next = newLink;
            status = VX_SUCCESS;
        }
    }
    if (status == VX_SUCCESS)
    {
        link_index++;
    }
    return status;
}

/* Function to delete the first element of the link */
static vx_status pop_link(tivx_shared_mem_info_t * head)
{
    tivx_shared_mem_info_t * current;
    vx_status status = VX_FAILURE;
    if(head == NULL)
        VX_PRINT(VX_ZONE_ERROR, "List is empty!\n");
    else
    {
        current = head;

        /* Pointing head to the next node */
        head = (*current).next;

        /* Free the popped element's memory */
        status = tivxMemFree(current, sizeof(tivx_shared_mem_info_t), TIVX_MEM_EXTERNAL);
        if (status != VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "Cannot free link from the shared mem info linked list!\n");
        }
    }
    return status;
}

vx_status test_utils_max_out_heap_mem(tivx_shared_mem_info_t** shared_mem_info_array_ret, vx_uint32* num_chunks, vx_enum mheap_region)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 num_allocations = 0;
    vx_uint32 total_alloc_size = 0;

    tivx_shared_mem_ptr_t temp_tivx_shared_mem_ptr;
    vx_uint32 chunk_size = MB;

    tivx_shared_mem_info_t* head = NULL;

    if (status == (vx_status)VX_SUCCESS)
    {
        /* Allocate memory in 1 MB chunks (large chunks) until exhausted */
        while (1)
        {
            status = tivxMemBufferAlloc(&temp_tivx_shared_mem_ptr,
                                            chunk_size, mheap_region);

            if (status == VX_ERROR_NO_MEMORY)
            {
                VX_PRINT(VX_ZONE_ERROR, "Memory exhausted after 1 MB allocation!\n");
                break;
            }
            else
            if (status == VX_SUCCESS)
            {
                total_alloc_size += chunk_size;
                num_allocations++;

                status = pushback_link(&head, temp_tivx_shared_mem_ptr, chunk_size);
                if (status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Memory info couldn't be pushed back to the new link!\n");
                    break;
                }
            }
            else
            {
                break;
            }
        }

        /* If memory exhausted for large chunk allocation, start small chunk (1KB) allocation */
        if (status == (vx_status)VX_ERROR_NO_MEMORY)
        {
            chunk_size = KB;
            /* Allocating memory in 1 KB chunks till memory is completely exhausted */
            while (1)
            {
                status = tivxMemBufferAlloc(&temp_tivx_shared_mem_ptr,
                                                chunk_size, mheap_region);
                if (status == VX_ERROR_NO_MEMORY)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Memory exhausted after 1 KB allocation!\n");
                    break;
                }
                else
                if (status == VX_SUCCESS)
                {
                    total_alloc_size += chunk_size;
                    num_allocations++;

                    status = pushback_link(&head, temp_tivx_shared_mem_ptr, chunk_size);
                    if (status != (vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Memory info couldn't be pushed back to the new link!\n");
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }

    VX_PRINT(VX_ZONE_ERROR,"%d alloc's of %d KB size\n", num_allocations,  total_alloc_size/1024);

    /* If all memory is allocated until exhausted, return success */
    if (status == VX_ERROR_NO_MEMORY)
    {
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        /* do nothing. Return error status */
    }


    *shared_mem_info_array_ret = head;
    *num_chunks = num_allocations;

    return status;
}



vx_status test_utils_release_maxed_out_heap_mem(tivx_shared_mem_info_t* shared_mem_info_array, vx_uint32 num_chunks)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_shared_mem_info_t * head = shared_mem_info_array;
    tivx_shared_mem_info_t * next = NULL;

    uint32_t i=0;
    tivx_shared_mem_ptr_t temp_tivx_shared_mem_ptr;

    while (head != NULL)
    {
        next = head->next;
        status = tivxMemBufferFree(&(head->shared_mem_ptr), head->size);
        if (status != VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,"Memory freeing failed at %u\n", i);
            break;
        }
        status = pop_link(head);
        if (status != VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,"Link deletion failed at %u\n", i);
            break;
        }
        head = next;
        i++;
    }

    if (status == VX_SUCCESS)
    {
        if (num_chunks == i)
        {
            VX_PRINT(VX_ZONE_ERROR,"Released %u/%u chunks successfully.\n", i, num_chunks);
        }
        else
        {
            /* The number of allocations is not equal to the number of frees. This might happen when the
               TIVX_MEM_EXTERNAL is depleted or the dmaFd limit has reached before allocating the final link.
               When the final link allocation is failed, the last memory allocation info couldn't be stored
               and thus it will not be released. This is a corner case, and need to be addressed if it's causing
               failures. */
            VX_PRINT(VX_ZONE_ERROR,"Released only %u/%u chunks successfully. Remaining %u chunk(s)\n", i, num_chunks, num_chunks-i);
            status = VX_FAILURE;
        }
    }

    return status;
}

