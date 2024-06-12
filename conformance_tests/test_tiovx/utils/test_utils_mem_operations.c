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

#define LARGE_CHUNK MB
#define SMALL_CHUNK KB

uint32_t link_index = 0;

/* Function to create and push data to the end of the list */
static vx_status pushback_link(tivx_shared_mem_info_t ** head, tivx_shared_mem_ptr_t shared_mem_ptr, vx_uint32 size)
{
    vx_status status = VX_FAILURE;

    /* Iterate till last node */
    tivx_shared_mem_info_t * last = * head;
    while (last->next != NULL)
    {
        last = last->next;
    }

    /* Populate the link with memory info */
    if (last->is_used == false)
    {
        last->shared_mem_ptr = shared_mem_ptr;
        last->size = size;
        last->is_used = true;
        status = VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Error. Link is already in use!\n");
    }

    return status;
}

/* Function to create and push data to the end of the list */
static vx_status create_link(tivx_shared_mem_info_t ** head)
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
        newLink->next = NULL;
        newLink->is_used = false;

        /* First Link */
        if (*head == NULL)
        {
            *head = newLink;
            status = VX_SUCCESS;
        }
        else
        {
            tivx_shared_mem_info_t * current = * head;

            /* Iterate till last node */
            while (current->next != NULL)
            {
                current = current->next;
            }
            /* Create new node if the last node is used, else do not create new link */
            if (current->is_used == true)
            {
                current->next = newLink;
                status = VX_SUCCESS;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Last link is not used. No need to create new link!\n");
                /* Free the temp link */
                status = tivxMemFree(newLink, sizeof(tivx_shared_mem_info_t), TIVX_MEM_EXTERNAL);
                if (status != VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Temporary link buffer cannot be freed!\n");
                }
            }

        }
    }
    if (status == VX_SUCCESS)
    {
        link_index++;
    }
    return status;
}

/* Function to delete the first element of the link */
static vx_status pop_head(tivx_shared_mem_info_t * head)
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

/* Function to delete the last element of the link */
static vx_status pop_tail(tivx_shared_mem_info_t ** head)
{
    tivx_shared_mem_info_t * tail = * head;
    tivx_shared_mem_info_t * previous = NULL;
    vx_status status = VX_FAILURE;


    if(tail == NULL)
        VX_PRINT(VX_ZONE_ERROR, "List is empty!\n");
    else
    {
        /* To ensure this is the last link, iterate till last node */
        while (tail->next != NULL)
        {
            previous = tail;
            tail = tail->next;
        }

        if (previous == NULL)
        {
            /* Only the head remains */
            VX_PRINT(VX_ZONE_INFO, "Only the head remains!\n");
            status = tivxMemFree(tail, sizeof(tivx_shared_mem_info_t), TIVX_MEM_EXTERNAL);
            tail = NULL;
            *head = NULL;
        }
        else
        {
            /* Set the previous node as tail */
            previous->next = NULL;
            status = tivxMemFree(tail, sizeof(tivx_shared_mem_info_t), TIVX_MEM_EXTERNAL);
        }

        /* Free the popped element's memory */
        if (status != VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "Cannot free link from the shared mem info linked list!\n");
        }
    }
    return status;
}

vx_status test_utils_max_out_heap_mem(tivx_shared_mem_info_t** shared_mem_info_array_ret, vx_uint32* num_chunks, vx_enum mheap_region)
{
    vx_status status, ret_status = (vx_status)VX_SUCCESS;
    vx_uint32 num_allocations = 0;
    vx_uint32 total_alloc_size = 0;

    tivx_shared_mem_ptr_t temp_tivx_shared_mem_ptr;
    vx_uint32 chunk_size = LARGE_CHUNK;

    tivx_shared_mem_info_t* head = NULL;

    /* Allocate memory in 1 MB chunks (large chunks) until exhausted, and then allocate small chunks until exhausted */
    while (1)
    {
        status = create_link(&head);
        if (status == VX_FAILURE)
        {
            /* Memory might be exhausted if memory region is TIVX_MEM_EXTERNAL */
            break;
        }
        status = tivxMemBufferAlloc(&temp_tivx_shared_mem_ptr,
                                        chunk_size, mheap_region);
        /* If memory is exhausted */
        if (status == VX_ERROR_NO_MEMORY)
        {
            if (chunk_size == LARGE_CHUNK)
            {
                VX_PRINT(VX_ZONE_ERROR, "Memory exhausted after Large chunk allocation! Starting Small chunk allocation.\n");
                /* Try small chunk allocation */
                chunk_size = SMALL_CHUNK;
            }
            else
            if (chunk_size == SMALL_CHUNK)
            {
                VX_PRINT(VX_ZONE_ERROR, "Memory exhausted after Small chunk allocation!\n");
                /* Break out of the loop after small chunk exhaustion */
                break;
            }
        }
        /* Memory not exhausted */
        else
        if (status == VX_SUCCESS)
        {
            total_alloc_size += chunk_size;
            num_allocations++;

            /* Populate info to the last link */
            status = pushback_link(&head, temp_tivx_shared_mem_ptr, chunk_size);
            if (status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "Memory info couldn't be pushed back to the last link!\n");
                break;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferAlloc failed with status %d!\n", status);
            break;
        }
    }

    VX_PRINT(VX_ZONE_ERROR,"%d alloc's of %d KB size\n", num_allocations,  total_alloc_size/1024);


    /* Ensuring no memory is remaining in the given region */
    tivx_shared_mem_ptr_t temp_final_tivx_shared_mem_ptr;

    status = tivxMemBufferAlloc(&temp_final_tivx_shared_mem_ptr,
                                            SMALL_CHUNK, mheap_region);


    /* If all memory is allocated until exhausted, return success */
    if (status == VX_ERROR_NO_MEMORY)
    {
        ret_status = (vx_status)VX_SUCCESS;
    }
    else
    if (status == VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Could allocate memory after exhaustion! Memory not exhausted fully! Releasing all allocated memory!\n", status);
        /* Last small allocation passed. Memory is not exhausted. Free all allocated memory and return failure */
        status = tivxMemBufferFree(&temp_final_tivx_shared_mem_ptr, SMALL_CHUNK);
        if (status != VX_SUCCESS)
        {
            /* Couldn't free last allocated small buffer */
            ret_status = VX_FAILURE;
        }
        /* Free all allocated buffers */
        status = test_utils_release_maxed_out_heap_mem(head, num_allocations);
        if (status != VX_SUCCESS)
        {
            /* Couldn't free all allocated buffers */
            ret_status = VX_FAILURE;
        }
    }

    if (ret_status == VX_SUCCESS)
    {
        *shared_mem_info_array_ret = head;
        *num_chunks = num_allocations;
    }
    else
    {
        *shared_mem_info_array_ret = NULL;
        *num_chunks = 0;
    }

    return ret_status;
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
        if (head->is_used == true)
        {
            next = head->next;
            status = tivxMemBufferFree(&(head->shared_mem_ptr), head->size);
            if (status != VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,"Memory freeing failed at %u\n", i);
                break;
            }
            status = pop_head(head);
            if (status != VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,"Link deletion failed at %u\n", i);
                break;
            }
            head = next;
            i++;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"The link %u is not used.\n", i);
            status = pop_head(head);
            break;
        }
    }

    if (status == VX_SUCCESS)
    {
        if (num_chunks == i)
        {
            VX_PRINT(VX_ZONE_ERROR,"Released %u/%u chunks successfully.\n", i, num_chunks);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"Released only %u/%u chunks successfully. Remaining %u chunk(s)\n", i, num_chunks, num_chunks-i);
            status = VX_FAILURE;
        }
    }

    return status;
}

vx_status test_utils_single_release_heap_mem(tivx_shared_mem_info_t** shared_mem_info_array, vx_uint32* num_chunks)
{
    vx_status status = (vx_status)VX_FAILURE;
    tivx_shared_mem_info_t * head = * shared_mem_info_array;
    tivx_shared_mem_info_t * next = NULL;

    tivx_shared_mem_ptr_t temp_tivx_shared_mem_ptr;

    tivx_shared_mem_info_t * tail = head;
    if (tail != NULL)
    {

        /* Iterate till last node */
        while (tail->next != NULL)
        {
            tail = tail->next;
        }
        /* Create new node if the last node is used, else do not create new link */
        if (tail->is_used == true)
        {
            status = tivxMemBufferFree(&(tail->shared_mem_ptr), tail->size);
            if (status != VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,"Memory freeing failed for chunk %u of size %u\n", *num_chunks, tail->size);
            }

            status = pop_tail(&head);
            if (status != VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,"Tail link deletion failed at %u\n", *num_chunks);
            }

            if (head == NULL)
            {
                VX_PRINT(VX_ZONE_INFO,"Last node has been released. Resetting the shared_mem_info_array to NULL!\n");
                *shared_mem_info_array = NULL;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"The link %u is not used\n", *num_chunks);

            status = pop_tail(&head);
            if (status != VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,"Link deletion failed at link: %u\n", *num_chunks);
            }
        }

        if (status == VX_SUCCESS)
        {
            if (head != NULL)
            {
                *num_chunks = *num_chunks - 1;
            }
        }
    }
    return status;
}