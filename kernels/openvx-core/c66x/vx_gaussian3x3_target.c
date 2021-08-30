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
#include <tivx_kernel_gaussian3x3.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

typedef VXLIB_STATUS (*VxLib_Filt3x3_Fxn)(const uint8_t *src_addr,
    const VXLIB_bufParams2D_t *vxlib_src, uint8_t *dst_addr, const VXLIB_bufParams2D_t *vxlib_dst);

typedef struct
{
    /*! \brief ID of the kernel */
    vx_enum                 kernel_id;

    /*! \brief Pointer to the kernel Registered */
    tivx_target_kernel      target_kernel;

    /*! \brief Pointer to filter function */
    VxLib_Filt3x3_Fxn       filter_func;
} tivxGaussian3X3KernelInfo;

static tivxGaussian3X3KernelInfo gTivxGaussian3X3KernelInfo =
{
    (vx_enum)VX_KERNEL_GAUSSIAN_3x3,
    NULL,
    VXLIB_gaussian_3x3_i8u_o8u

};

static vx_status VX_CALLBACK tivxGaussian3X3Create(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxGaussian3X3Delete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxProcessGaussian3X3(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxProcessGaussian3X3(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src_desc, *dst_desc;
    uint8_t *src_addr, *dst_addr;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    tivxGaussian3X3KernelInfo *kern_info;
    void *src_desc_target_ptr;
    void *dst_desc_target_ptr;

    if ((num_params != 2U) || (NULL == obj_desc[0U]) ||
        (NULL == obj_desc[1U]) || (NULL == kernel) ||
        (NULL == priv_arg))
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        /* Get the Src and Dst descriptors */
        src_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_GAUSSIAN3X3_INPUT_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_GAUSSIAN3X3_OUTPUT_IDX];

        if ((NULL == src_desc) || (NULL == dst_desc))
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        kern_info = (tivxGaussian3X3KernelInfo *)priv_arg;

        /* Get the target pointer from the shared pointer for all
           three buffers */
        src_desc_target_ptr = tivxMemShared2TargetPtr(&src_desc->mem_ptr[0]);
        dst_desc_target_ptr = tivxMemShared2TargetPtr(&dst_desc->mem_ptr[0]);

        if ((NULL == src_desc_target_ptr) ||
            (NULL == dst_desc_target_ptr))
        {
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        }

        if ((src_desc->imagepatch_addr[0U].stride_y <
                (int32_t)src_desc->imagepatch_addr[0U].dim_x) ||
            (dst_desc->imagepatch_addr[0U].stride_y <
                (int32_t)dst_desc->imagepatch_addr[0U].dim_x))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Map all buffers, which invalidates the cache */
        tivxCheckStatus(&status, tivxMemBufferMap(src_desc_target_ptr,
            src_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferMap(dst_desc_target_ptr,
            dst_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_WRITE_ONLY));

        tivxInitBufParams(src_desc, &vxlib_src);
        tivxInitBufParams(dst_desc, &vxlib_dst);

        tivxSetPointerLocation(src_desc, &src_desc_target_ptr, &src_addr);
        tivxSetPointerLocation(dst_desc, &dst_desc_target_ptr, &dst_addr);

        /* All 3x3 filter reduces the output size, therefore reduce output
         * height, but leave output width the same (DSP optimization) */
        vxlib_dst.dim_x = vxlib_src.dim_x;

        if (kern_info->filter_func != NULL)
        {
            status = (vx_status)kern_info->filter_func(src_addr, &vxlib_src, dst_addr,
                &vxlib_dst);
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

static vx_status VX_CALLBACK tivxGaussian3X3Create(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return status;
}

static vx_status VX_CALLBACK tivxGaussian3X3Delete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelGaussian3X3(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        gTivxGaussian3X3KernelInfo.target_kernel = tivxAddTargetKernel(
                    gTivxGaussian3X3KernelInfo.kernel_id,
                    target_name,
                    tivxProcessGaussian3X3,
                    tivxGaussian3X3Create,
                    tivxGaussian3X3Delete,
                    NULL,
                    (void *)&gTivxGaussian3X3KernelInfo);
    }
}

void tivxRemoveTargetKernelGaussian3X3(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

        status = tivxRemoveTargetKernel(gTivxGaussian3X3KernelInfo.target_kernel);

        if ((vx_status)VX_SUCCESS == status)
        {
            gTivxGaussian3X3KernelInfo.target_kernel = NULL;
        }
}

