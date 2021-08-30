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
#include <tivx_target_kernels_priv.h>
#include <tivx_kernel_convert_depth.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

static tivx_target_kernel vx_convert_depth_target_kernel = NULL;

static vx_status tivxKernelConvertDepth(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    vx_enum kern_type);
static vx_status VX_CALLBACK tivxKernelConvertDepthCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelConvertDepthDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelConvertDepthProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

/* Work on this section */
static vx_status tivxKernelConvertDepth(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    vx_enum kern_type)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src_desc, *dst_desc;
    tivx_obj_desc_scalar_t *sc_desc[2];
    uint32_t i;
    uint8_t *src_addr, *dst_addr;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    uint8_t overflow_policy;

    if (num_params != TIVX_KERNEL_CONVERT_DEPTH_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_CONVERT_DEPTH_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        void *src_desc_target_ptr;
        void *dst_desc_target_ptr;

        src_desc = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_CONVERT_DEPTH_INPUT_IDX];
        dst_desc = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_CONVERT_DEPTH_OUTPUT_IDX];
        sc_desc[0] = (tivx_obj_desc_scalar_t *)
            obj_desc[TIVX_KERNEL_CONVERT_DEPTH_POLICY_IDX];
        sc_desc[1] = (tivx_obj_desc_scalar_t *)
            obj_desc[TIVX_KERNEL_CONVERT_DEPTH_SHIFT_IDX];

        src_desc_target_ptr = tivxMemShared2TargetPtr(&src_desc->mem_ptr[0U]);

        dst_desc_target_ptr = tivxMemShared2TargetPtr(&dst_desc->mem_ptr[0U]);

        tivxCheckStatus(&status, tivxMemBufferMap(src_desc_target_ptr, src_desc->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferMap(dst_desc_target_ptr, dst_desc->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        tivxSetPointerLocation(src_desc, &src_desc_target_ptr, &src_addr);
        tivxSetPointerLocation(dst_desc, &dst_desc_target_ptr, &dst_addr);

        tivxInitBufParams(src_desc, &vxlib_src);
        tivxInitBufParams(dst_desc, &vxlib_dst);

        if ((vx_enum)VX_CONVERT_POLICY_SATURATE == sc_desc[0]->data.enm)
        {
            overflow_policy = VXLIB_CONVERT_POLICY_SATURATE;
        }
        else
        {
            overflow_policy = VXLIB_CONVERT_POLICY_WRAP;
        }

        if ((uint32_t)VXLIB_INT16 == vxlib_dst.data_type)
        {
            status = (vx_status)VXLIB_convertDepth_i8u_o16s((uint8_t *)src_addr,
                        &vxlib_src, (int16_t *)dst_addr, &vxlib_dst, (uint32_t)sc_desc[1]->data.s32);
        }
        else
        {
            status = (vx_status)VXLIB_convertDepth_i16s_o8u((int16_t *)src_addr,
                        &vxlib_src, (uint8_t *)dst_addr, &vxlib_dst, (uint32_t)sc_desc[1]->data.s32, overflow_policy);
        }



        if ((vx_status)VXLIB_SUCCESS != status)
        {
            status = (vx_status)VX_FAILURE;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(src_desc_target_ptr,
            src_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferUnmap(dst_desc_target_ptr,
            dst_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_WRITE_ONLY));
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelConvertDepthCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return ((vx_status)VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelConvertDepthDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return ((vx_status)VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelConvertDepthProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status;

    status = tivxKernelConvertDepth(kernel, obj_desc, num_params, (vx_enum)VX_KERNEL_CONVERTDEPTH);

    return (status);
}

void tivxAddTargetKernelConvertDepth(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_convert_depth_target_kernel = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_CONVERTDEPTH,
            target_name,
            tivxKernelConvertDepthProcess,
            tivxKernelConvertDepthCreate,
            tivxKernelConvertDepthDelete,
            NULL,
            NULL);
    }
}


void tivxRemoveTargetKernelConvertDepth(void)
{
    tivxRemoveTargetKernel(vx_convert_depth_target_kernel);
}

