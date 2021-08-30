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
#include <tivx_kernel_remap.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

static tivx_target_kernel vx_remap_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelRemapProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelRemapCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelRemapDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelRemapProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_scalar_t *sc;
    vx_uint8 *src_addr, *dst_addr;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst, vxlib_remap;
    vx_border_t border;
    tivx_obj_desc_remap_t *remap;

    if (num_params != TIVX_KERNEL_REMAP_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_REMAP_MAX_PARAMS; i ++)
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
        void *src_target_ptr;
        void *dst_target_ptr;
        void *remap_target_ptr;

        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_REMAP_INPUT_IDX];
        sc = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_REMAP_POLICY_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_REMAP_OUTPUT_IDX];
        remap = (tivx_obj_desc_remap_t *)obj_desc[TIVX_KERNEL_REMAP_TABLE_IDX];

        src_target_ptr = tivxMemShared2TargetPtr(&src->mem_ptr[0]);
        dst_target_ptr = tivxMemShared2TargetPtr(&dst->mem_ptr[0]);
        remap_target_ptr = tivxMemShared2TargetPtr(&remap->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(src_target_ptr, src->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferMap(dst_target_ptr, dst->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
        tivxCheckStatus(&status, tivxMemBufferMap(remap_target_ptr, remap->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        tivxInitBufParams(src, &vxlib_src);
        tivxInitBufParams(dst, &vxlib_dst);

        vxlib_remap.dim_x = dst->imagepatch_addr[0].dim_x * 2U;
        vxlib_remap.dim_y = dst->imagepatch_addr[0].dim_y;
        vxlib_remap.stride_y = (int32_t)dst->imagepatch_addr[0].dim_x * 8;
        vxlib_remap.data_type = (uint32_t)VXLIB_FLOAT32;

        tivxSetPointerLocation(src, &src_target_ptr, &src_addr);
        tivxSetPointerLocation(dst, &dst_target_ptr, &dst_addr);

        tivxGetTargetKernelInstanceBorderMode(kernel, &border);

        if ((vx_enum)VX_INTERPOLATION_BILINEAR == sc->data.enm)
        {
            status = (vx_status)VXLIB_remapBilinear_bc_i8u_i32f_o8u(
                src_addr, &vxlib_src, dst_addr, &vxlib_dst,
                remap_target_ptr, &vxlib_remap,
                border.constant_value.U8);
        }
        else if ((vx_enum)VX_INTERPOLATION_NEAREST_NEIGHBOR == sc->data.enm)
        {
            status = (vx_status)VXLIB_remapNearest_bc_i8u_i32f_o8u(
                src_addr, &vxlib_src, dst_addr, &vxlib_dst,
                remap_target_ptr, &vxlib_remap,
                border.constant_value.U8);
        }
        else
        {
            status = (vx_status)VX_FAILURE;
        }

        if ((vx_status)VXLIB_SUCCESS != status)
        {
            status = (vx_status)VX_FAILURE;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(src_target_ptr, src->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferUnmap(dst_target_ptr, dst->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
        tivxCheckStatus(&status, tivxMemBufferUnmap(remap_target_ptr, remap->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelRemapCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return ((vx_status)VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelRemapDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return ((vx_status)VX_SUCCESS);
}

void tivxAddTargetKernelRemap(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_remap_target_kernel = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_REMAP,
            target_name,
            tivxKernelRemapProcess,
            tivxKernelRemapCreate,
            tivxKernelRemapDelete,
            NULL,
            NULL);
    }
}


void tivxRemoveTargetKernelRemap(void)
{
    tivxRemoveTargetKernel(vx_remap_target_kernel);
}
