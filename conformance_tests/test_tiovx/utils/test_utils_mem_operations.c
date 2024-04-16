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


vx_status test_utils_max_out_heap_mem(tivx_shared_mem_info_t** shared_mem_info_array_ret, vx_uint32* num_chunks, vx_enum mheap_region)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 num_allocations = 0;
    vx_uint32 total_alloc_size = 0;

    tivx_shared_mem_info_t* shared_mem_info_array = NULL;
    shared_mem_info_array = malloc(sizeof(tivx_shared_mem_info_t));

    vx_uint32 chunk_size = MB;

    if (shared_mem_info_array == NULL) {
        VX_PRINT(VX_ZONE_ERROR,"Info array allocation failed\n");
        status = (vx_status)VX_FAILURE;
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        /* Allocate memory in 1 MB chunks (large chunks) until exhausted */
        while (1)
        {
            status = tivxMemBufferAlloc(&(shared_mem_info_array[num_allocations].shared_mem_ptr),
                                            chunk_size, mheap_region);

            if (status == VX_ERROR_NO_MEMORY)
            {
                VX_PRINT(VX_ZONE_INFO, "Memory exhausted after 1 MB allocation!\n");
                break;
            }
            else
            {
                shared_mem_info_array[num_allocations].size = chunk_size;
                total_alloc_size += chunk_size;
                num_allocations++;

                shared_mem_info_array = realloc(shared_mem_info_array, (num_allocations + 1) * sizeof(tivx_shared_mem_info_t));

                if (shared_mem_info_array == NULL)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Info array reallocation failed\n");
                    status = (vx_status)VX_FAILURE;
                    break;
                }
            }
        }

        /* If memory exhausted for large chunk allocation, start small chunk (1KB) allocation */
        if (status == (vx_status)VX_ERROR_NO_MEMORY)
        {
            chunk_size = KB;
            /* Allocating memory in 1 KB chunks till memory is completely exhausted */
            while (1)
            {
                status = tivxMemBufferAlloc(&(shared_mem_info_array[num_allocations].shared_mem_ptr),
                                                chunk_size, mheap_region);
                if (status == VX_ERROR_NO_MEMORY)
                {
                    VX_PRINT(VX_ZONE_INFO, "Memory exhausted after 1 KB allocation!\n");
                    break;
                }
                else
                {
                    shared_mem_info_array[num_allocations].size = chunk_size;
                    total_alloc_size += chunk_size;
                    num_allocations++;

                    shared_mem_info_array = realloc(shared_mem_info_array, (num_allocations + 1) * sizeof(tivx_shared_mem_info_t));

                    if (shared_mem_info_array == NULL)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Info array reallocation failed\n");
                        status = (vx_status)VX_FAILURE;
                        break;
                    }
                }
            }
        }
    }

    VX_PRINT(VX_ZONE_INFO,"%d alloc's of %d KB size\n", num_allocations,  total_alloc_size/1024);

    /* If all memory is allocated until exhausted, return success */
    if (status == VX_ERROR_NO_MEMORY)
    {
        status = (vx_status)VX_SUCCESS;
    }

    *shared_mem_info_array_ret = shared_mem_info_array;
    *num_chunks = num_allocations;

    return status;
}

vx_status test_utils_release_maxed_out_heap_mem(tivx_shared_mem_info_t* shared_mem_info_array, vx_uint32 num_chunks)
{
    vx_status status = (vx_status)VX_SUCCESS;

    for (int i = 0; i < num_chunks ; i++)
    {
        status = tivxMemBufferFree(&(shared_mem_info_array[i].shared_mem_ptr), shared_mem_info_array[i].size);
        if (status != VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,"Memory freeing failed at %d\n", i);
            break;
        }
    }
    if (status == VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_INFO,"Released %d chunks\n", num_chunks);
    }
    free(shared_mem_info_array);

    return status;
}

