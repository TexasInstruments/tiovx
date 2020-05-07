/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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
 * \file tivx_utils_checksum.h Utility APIs to provide checksums for output correctness
 */

#ifndef TIVX_UTILS_CHECKSUM_H
#define TIVX_UTILS_CHECKSUM_H


#include <VX/vx.h>
#include <VX/vx_khr_user_data_object.h>
#include <TI/tivx_tensor.h>

/**
 * \brief Returns a uint32_t of the sum of all words within a rect of an image's buffer.
 *
 *        If the buffer size contains a number of bytes not divisible by 4, the last
 *        1, 2, or 3 bytes are rightshifted so that the last byte aligns with the LSB.
 *
 * \param image [in] input image
 *
 * \param plane_id [in] plane number for multi plane image format
 *
 * \param rect [in] rectangular region to generate the checksum for
 *
 * \return sum of words in the specified region
 *
 * \ingroup group_tivx_ext_host_utils
 */
uint32_t tivx_utils_simple_image_checksum(vx_image image, uint8_t plane_id, vx_rectangle_t rect);

/**
 * \brief Returns a uint32_t of the sum of all words within a range of the user data object buffer.
 *
 *        If the buffer size contains a number of bytes not divisible by 4, the last
 *        1, 2, or 3 bytes are rightshifted so that the last byte aligns with the LSB.
 *
 * \param user_data_object [in] input user data object
 *
 * \param offset_byte [in] starting offset (in bytes)
 *
 * \param num_bytes [in] number of bytes to generate the checksum for
 *
 * \return sum of words in the specified range
 *
 * \ingroup group_tivx_ext_host_utils
 */
uint32_t tivx_utils_user_data_object_checksum(vx_user_data_object user_data_object, uint32_t offset_byte, uint32_t num_bytes);

/**
 * \brief Returns a uint32_t of the sum of all words within a range of the tensor buffer.
 *
 *        If the buffer size contains a number of bytes not divisible by 4, the last
 *        1, 2, or 3 bytes are rightshifted so that the last byte aligns with the LSB.
 *
 * \param tensor_object [in] input tensor object
 *
 * \param number_of_dimensions [in] number of tensor dimensions
 *
 * \param view_start [in] starting offset for each dimension (in elements)
 *
 * \param view_end [in] ending offset for each dimension (in elements)
 *
 * \param user_stride [in] stride for each dimension (in bytes)
 *
 * \return sum of words in the specified range
 *
 * \ingroup group_tivx_ext_host_utils
 */
uint32_t tivx_utils_tensor_checksum(vx_tensor tensor_object, vx_size number_of_dimensions, vx_size * view_start, vx_size * view_end, vx_size * user_stride);

#endif
