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



#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_absdiff.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include "tivx_target_kernels_priv.h"

static tivx_target_kernel vx_absdiff_target_kernel = NULL;

static vx_status VX_CALLBACK tivxAbsDiff(tivx_target_kernel_instance kernel,
                                         tivx_obj_desc_t *obj_desc[],
                                         uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxAbsDiffDelete(tivx_target_kernel_instance kernel,
                 tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxAbsDiffCreate(tivx_target_kernel_instance kernel,
                 tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxAbsDiff(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src0_desc, *src1_desc, *dst_desc;
    uint8_t *src0_addr, *src1_addr, *dst_addr;
    VXLIB_bufParams2D_t vxlib_src0, vxlib_src1, vxlib_dst;

    if ((num_params != 3U) || (NULL == obj_desc[0U]) || (NULL == obj_desc[1U]) ||
        (NULL == obj_desc[2U]))
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        void *src0_desc_target_ptr;
        void *src1_desc_target_ptr;
        void *dst_desc_target_ptr;

        /* Get the Src and Dst descriptors */
        src0_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_ABSDIFF_IN1_IDX];
        src1_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_ABSDIFF_IN2_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_ABSDIFF_OUT_IDX];

        /* Get the target pointer from the shared pointer for all
           buffers */
        src0_desc_target_ptr = tivxMemShared2TargetPtr(&src0_desc->mem_ptr[0]);
        src1_desc_target_ptr = tivxMemShared2TargetPtr(&src1_desc->mem_ptr[0]);
        dst_desc_target_ptr = tivxMemShared2TargetPtr(&dst_desc->mem_ptr[0]);

        /* Map all buffers, which invalidates the cache */
        tivxCheckStatus(&status, tivxMemBufferMap(src0_desc_target_ptr,
            src0_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferMap(src1_desc_target_ptr,
            src1_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferMap(dst_desc_target_ptr,
            dst_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_WRITE_ONLY));

        tivxSetTwoPointerLocation(src0_desc, src1_desc, &src0_desc_target_ptr, &src1_desc_target_ptr, &src0_addr, &src1_addr);
        tivxSetPointerLocation(dst_desc, &dst_desc_target_ptr, &dst_addr);

        tivxInitTwoBufParams(src0_desc, src1_desc, &vxlib_src0, &vxlib_src1);
        tivxInitBufParams(dst_desc, &vxlib_dst);

        if ((uint32_t)VXLIB_UINT8 == vxlib_dst.data_type)
        {
            status = (vx_status)VXLIB_absDiff_i8u_i8u_o8u(src0_addr, &vxlib_src0, src1_addr,
                &vxlib_src1, dst_addr, &vxlib_dst);
        }
        else
        {
            status = (vx_status)VXLIB_absDiff_i16s_i16s_o16s(
                (int16_t *)src0_addr, &vxlib_src0, (int16_t *)src1_addr,
                &vxlib_src1, (int16_t *)dst_addr, &vxlib_dst);
        }
        if ((vx_status)VXLIB_SUCCESS != status)
        {
            status = (vx_status)VX_FAILURE;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(src0_desc_target_ptr,
            src0_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferUnmap(src1_desc_target_ptr,
            src1_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferUnmap(dst_desc_target_ptr,
            dst_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_WRITE_ONLY));

    }

    return (status);
}

static vx_status VX_CALLBACK tivxAbsDiffCreate(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return status;
}

static vx_status VX_CALLBACK tivxAbsDiffDelete(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return status;
}


void tivxAddTargetKernelAbsDiff(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == (vx_enum)TIVX_CPU_ID_DSP1) || (self_cpu == (vx_enum)TIVX_CPU_ID_DSP2))
    {
        if (self_cpu == (vx_enum)TIVX_CPU_ID_DSP1)
        {
            strncpy(target_name, TIVX_TARGET_DSP1,
                TIVX_TARGET_MAX_NAME);
        }
        else
        {
            strncpy(target_name, TIVX_TARGET_DSP2,
                TIVX_TARGET_MAX_NAME);
        }

        vx_absdiff_target_kernel = tivxAddTargetKernel(
                    (vx_enum)VX_KERNEL_ABSDIFF,
                    target_name,
                    tivxAbsDiff,
                    tivxAbsDiffCreate,
                    tivxAbsDiffDelete,
                    NULL,
                    NULL);
    }
}

void tivxRemoveTargetKernelAbsDiff(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_absdiff_target_kernel);

    if ((vx_status)VX_SUCCESS == status)
    {
        vx_absdiff_target_kernel = NULL;
    }
}

