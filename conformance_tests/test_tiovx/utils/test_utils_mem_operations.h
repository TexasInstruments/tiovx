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

/**
 * \file test_utils_mem_operations.h Utility APIs to allocate all available memory
 *       and free the allocated memory using low level tivxMemBuffer APIs.
 */

#ifndef TEST_UTILS_MEM_OPERATIONS_H
#define TEST_UTILS_MEM_OPERATIONS_H

#include <VX/vx.h>
#include <TI/tivx.h>

#include "test_engine/test.h"
#include <inttypes.h>

typedef struct tivx_shared_mem_info {
    tivx_shared_mem_ptr_t shared_mem_ptr;
    vx_uint32 size;
    bool is_used;
    struct tivx_shared_mem_info* next;
} tivx_shared_mem_info_t;

/**
 * \brief Allocate all available specified heap memory until memory is exhausted
 *
 *  1 MB chunks of memory will be allocated first until it's is not possible and
 *  then 1 KB chunks will be allocated until complete memory region is exhausted.
 *
 *  Due to the limits set in limits.conf file, the total number of dmaBufFd's
 *  in the system is limited. Thus the larger chunks are allocated first to
 *  allocate maximum memory using minimal allocations, and then small chunks
 *  to allocate the remaining memory, to avoid running into this limit.
 *
 * \param shared_mem_info_array [out] tivx_shared_mem_info array which will be
*                                       populated with the tivx_shared_mem_ptr_t and size data.
 * \param num_chunks [out] number of total allocation.
 * \param mheap_region [in] The heap region to be filled.
 *
 * \return VX_SUCCESS if successfully exhausted the memory region. Error code otherwise.
 *
 * \ingroup group_tivx_ext_host_utils
 */
vx_status test_utils_max_out_heap_mem(tivx_shared_mem_info_t** shared_mem_info_array, vx_uint32* num_chunks, vx_enum mheap_region);

/**
 * \brief Free the allocated memory
 *
 * \param shared_mem_info_array [in] tivx_shared_mem_info array with the tivx_shared_mem_ptr_t and size data.
 * \param num_chunks [in] number of total allocation / number of elements of shared_mem_info_array.
 *
 * \return VX_SUCCESS if successfully freed all the chunks in the given shared_mem_info_array.
 *
 * \ingroup group_tivx_ext_host_utils
 */
vx_status test_utils_release_maxed_out_heap_mem(tivx_shared_mem_info_t* shared_mem_info_array, vx_uint32 num_chunks);

/**
 * \brief Free the allocated memory from a single (tail) link
 *
 * \param shared_mem_info_array [in] tivx_shared_mem_info array with the tivx_shared_mem_ptr_t and size data.
 * \param num_chunks [in/out] number of total allocation / number of elements of shared_mem_info_array will be updated.
 *
 * \return VX_SUCCESS if successfully freed the tail chunk in the given shared_mem_info_array.
 *
 * \ingroup group_tivx_ext_host_utils
 */
vx_status test_utils_single_release_heap_mem(tivx_shared_mem_info_t** shared_mem_info_array, vx_uint32* num_chunks);

#endif /* TEST_UTILS_MEM_OPERATIONS_H */