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
#include <tivx_kernel_filter_3x3.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

typedef VXLIB_STATUS (*VxLib_Filt3x3_Fxn)(const uint8_t *,
    const VXLIB_bufParams2D_t *, uint8_t *, const VXLIB_bufParams2D_t *);

typedef struct
{
    /*! \brief ID of the kernel */
    vx_enum                 kernel_id;

    /*! \brief Pointer to the kernel Registered */
    tivx_target_kernel      target_kernel;

    /*! \brief Pointer to filter function */
    VxLib_Filt3x3_Fxn       filter_func;
} tivxFilter3x3KernelInfo;

static tivxFilter3x3KernelInfo gTivxFilt3x3KernelInfo[] =
{
    {
        VX_KERNEL_ERODE_3x3,
        NULL,
        VXLIB_erode_3x3_i8u_o8u
    },
    {
        VX_KERNEL_BOX_3x3,
        NULL,
        VXLIB_box_3x3_i8u_o8u
    },
    {
        VX_KERNEL_DILATE_3x3,
        NULL,
        VXLIB_dilate_3x3_i8u_o8u
    },
    {
        VX_KERNEL_GAUSSIAN_3x3,
        NULL,
        VXLIB_gaussian_3x3_i8u_o8u
    },
    {
        VX_KERNEL_MEDIAN_3x3,
        NULL,
        VXLIB_median_3x3_i8u_o8u
    },
};

vx_status VX_CALLBACK tivxProcess3x3Filter(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src_desc, *dst_desc;
    uint8_t *src_addr, *dst_addr;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    tivxFilter3x3KernelInfo *kern_info;
    void *src_desc_target_ptr;
    void *dst_desc_target_ptr;

    if ((num_params != 2U) || (NULL == obj_desc[0U]) ||
        (NULL == obj_desc[1U]) || (NULL == kernel) ||
        (NULL == priv_arg))
    {
        status = VX_FAILURE;
    }
    else
    {
        /* Get the Src and Dst descriptors */
        src_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_FILT3x3_IN_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_FILT3x3_OUT_IMG_IDX];

        if ((NULL == src_desc) || (NULL == dst_desc))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        kern_info = (tivxFilter3x3KernelInfo *)priv_arg;

        /* Get the target pointer from the shared pointer for all
           three buffers */
        src_desc_target_ptr = tivxMemShared2TargetPtr(
            src_desc->mem_ptr[0].shared_ptr, src_desc->mem_ptr[0].mem_heap_region);
        dst_desc_target_ptr = tivxMemShared2TargetPtr(
            dst_desc->mem_ptr[0].shared_ptr, dst_desc->mem_ptr[0].mem_heap_region);

        if ((NULL == src_desc_target_ptr) ||
            (NULL == dst_desc_target_ptr))
        {
            status = VX_ERROR_INVALID_REFERENCE;
        }

        if ((src_desc->imagepatch_addr[0U].stride_y <
                src_desc->imagepatch_addr[0U].dim_x) ||
            (dst_desc->imagepatch_addr[0U].stride_y <
                dst_desc->imagepatch_addr[0U].dim_x))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        /* Map all buffers, which invalidates the cache */
        tivxMemBufferMap(src_desc_target_ptr,
            src_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferMap(dst_desc_target_ptr,
            dst_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

        tivxInitBufParams(src_desc, &vxlib_src);
        tivxInitBufParams(dst_desc, &vxlib_dst);

        tivxSetPointerLocation(src_desc, &src_desc_target_ptr, &src_addr);
        tivxSetPointerLocation(dst_desc, &dst_desc_target_ptr, &dst_addr);

        /* All 3x3 filter reduces the output size, therefore reduce output
         * height, but leave output width the same (DSP optimization) */
        vxlib_dst.dim_x = vxlib_src.dim_x;

        if (kern_info->filter_func)
        {
            status = kern_info->filter_func(src_addr, &vxlib_src, dst_addr,
                &vxlib_dst);
        }
        if (VXLIB_SUCCESS != status)
        {
            status = VX_FAILURE;
        }

        tivxMemBufferUnmap(src_desc_target_ptr,
            src_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(dst_desc_target_ptr,
            dst_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);
    }

    return (status);
}

vx_status VX_CALLBACK tivxFilter3x3Create(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status VX_CALLBACK tivxFilter3x3Delete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status VX_CALLBACK tivxFilter3x3Control(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelErode3x3(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;
    vx_uint32 i;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_DSP1) || (self_cpu == TIVX_CPU_ID_DSP2))
    {
        if (self_cpu == TIVX_CPU_ID_DSP1)
        {
            strncpy(target_name, TIVX_TARGET_DSP1,
                TIVX_TARGET_MAX_NAME);
        }
        else
        {
            strncpy(target_name, TIVX_TARGET_DSP2,
                TIVX_TARGET_MAX_NAME);
        }

        for (i = 0; i < (sizeof(gTivxFilt3x3KernelInfo)/
                sizeof(tivxFilter3x3KernelInfo)); i ++)
        {
            gTivxFilt3x3KernelInfo[i].target_kernel = tivxAddTargetKernel(
                        gTivxFilt3x3KernelInfo[i].kernel_id,
                        target_name,
                        tivxProcess3x3Filter,
                        tivxFilter3x3Create,
                        tivxFilter3x3Delete,
                        tivxFilter3x3Control,
                        (void *)&gTivxFilt3x3KernelInfo[i]);
            if (NULL == gTivxFilt3x3KernelInfo[i].target_kernel)
            {
                break;
            }
        }
    }
}

void tivxRemoveTargetKernelErode3x3(void)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;

    for (i = 0; i < (sizeof(gTivxFilt3x3KernelInfo)/
            sizeof(tivxFilter3x3KernelInfo)); i ++)
    {
        status = tivxRemoveTargetKernel(gTivxFilt3x3KernelInfo[i].target_kernel);

        if (VX_SUCCESS == status)
        {
            gTivxFilt3x3KernelInfo[i].target_kernel = NULL;
        }
    }
}

