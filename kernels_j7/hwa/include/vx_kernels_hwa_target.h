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

#ifndef VX_KERNELS_HWA_TARGET_
#define VX_KERNELS_HWA_TARGET_

#include "TI/tivx.h"

#ifdef VLAB_HWA
#include "vlab_hwa.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for all target kernels
 */

void lse_reformat_in(const tivx_obj_desc_image_t *src, void *src_target_ptr, uint16_t src16[], uint8_t ch, uint8_t out_bit_alignment);
void lse_reformat_out(const tivx_obj_desc_image_t *src, const tivx_obj_desc_image_t *dst, void *dst_target_ptr, uint16_t dst16[], uint16_t input_bits, uint8_t ch);
void lse_reformat_in_dof(const tivx_obj_desc_image_t *src, void *src_target_ptr, uint32_t *src32);
void lse_reformat_out_dof(const tivx_obj_desc_image_t *src, const tivx_obj_desc_image_t *dst, void *dst_target_ptr, const int32_t *dst32);
void lse_reformat_in_viss(const tivx_obj_desc_raw_image_t *src, void* src_target_ptr, uint16_t src16[], uint32_t exposure);
void lse_reformat_out_viss(const tivx_obj_desc_raw_image_t *src, tivx_obj_desc_image_t *dst, void *dst0_target_ptr, void *dst1_target_ptr, uint16_t dst16_0[], uint16_t dst16_1[], uint16_t input_bits);
void lse_interleave_422(const tivx_obj_desc_image_t *src, const tivx_obj_desc_image_t *dst, void *dst_target_ptr, uint16_t dst16_0[], uint16_t dst16_1[], uint16_t input_bits);
void lse_deinterleave_422(const tivx_obj_desc_image_t *src, void *src_target_ptr, uint16_t src16_0[], uint16_t src16_1[], uint16_t input_bits);

#ifdef VLAB_HWA
vx_status vlab_hwa_process(uint32_t base_address, char *kernel_prefix, uint32_t config_size, void *pConfig);
#endif

#ifdef __cplusplus
}
#endif

#endif
