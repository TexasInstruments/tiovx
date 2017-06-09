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




#ifndef TIVX_TARGET_KERNELS_UTILS_
#define TIVX_TARGET_KERNELS_UTILS_

#include <tivx_kernel_utils.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief A utility API to initialize VXLIB buf parameters based
 *        on the provided valid rectangle and given object descriptor.
 *
 *        This API takes valid rectangle and object descriptor as an argument
 *        uses them to initialize VXLIB buf descriptor. It uses valid
 *        rectangle to initialize dimensions of the frame and object
 *        descriptor to initialize stride and data type.
 *        While initializing frame dimensions, it also takes into account
 *        the padding requirement of the calling kernel. If the kernel
 *        requires few pixels/lines on all sides of the kernels, this api
 *        increases the valid rectangle and then initializes vxlib buf
 *        descriptor.
 *
 *        If the valid rectangle is not provided, this API uses valid
 *        rectangle from the object descriptor.
 *
 * \param prms [in] Valid Rectangle Parameters
 */
void ownInitBufParams(
    tivx_obj_desc_image_t *obj_desc,
    vx_rectangle_t *rect,
    VXLIB_bufParams2D_t buf_params[],
    uint8_t *addr[],
    uint32_t lpad, uint32_t tpad, uint32_t rpad, uint32_t bpad);

/*!
 * \brief Reserve L2MEM within C66x for usage with BAM framework
 *
 */
void ownReserveC66xL2MEM(void);

#ifdef __cplusplus
}
#endif

#endif
