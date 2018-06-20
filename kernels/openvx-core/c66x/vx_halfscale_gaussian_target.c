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
#include <tivx_kernel_halfscale_gaussian.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

static tivx_target_kernel vx_halfscale_gaussian_target_kernel = NULL;

vx_status VX_CALLBACK tivxHalfscaleGaussian(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src_desc;
    tivx_obj_desc_image_t *dst_desc;
    tivx_obj_desc_scalar_t *gsize_desc;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    uint8_t *src_addr, *dst_addr;

    if ((num_params != TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_SRC_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_DST_IDX]))
    {
        status = VX_FAILURE;
    }
    else
    {
        int32_t gsize_value = 1;
        void *src_desc_target_ptr;
        void *dst_desc_target_ptr;

        src_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_SRC_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_DST_IDX];
        gsize_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_GSIZE_IDX];

        src_desc_target_ptr = tivxMemShared2TargetPtr(
          src_desc->mem_ptr[0].shared_ptr, src_desc->mem_ptr[0].mem_heap_region);
        dst_desc_target_ptr = tivxMemShared2TargetPtr(
          dst_desc->mem_ptr[0].shared_ptr, dst_desc->mem_ptr[0].mem_heap_region);

        tivxMemBufferMap(src_desc_target_ptr,
           src_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferMap(dst_desc_target_ptr,
           dst_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

        tivxSetPointerLocation(src_desc, &src_desc_target_ptr, &src_addr);
        tivxSetPointerLocation(dst_desc, &dst_desc_target_ptr, &dst_addr);

        if( gsize_desc != NULL)
        {
            gsize_value = gsize_desc->data.s32;
        }

        if(gsize_value == 1)
        {
            tivxInitBufParams(src_desc, &vxlib_src);
            tivxInitBufParams(dst_desc, &vxlib_dst);
            status |= VXLIB_scaleImageNearest_i8u_o8u(src_addr, &vxlib_src,
                                                      dst_addr, &vxlib_dst,
                                                      2, 2, 0, 0, 0, 0);
        }
        else if ((gsize_value == 3) || (gsize_value == 5))
        {
            VXLIB_bufParams2D_t gauss_params;
            uint8_t *pGauss;

            tivxInitBufParams(src_desc, &vxlib_src);
            tivxInitBufParams(dst_desc, &vxlib_dst);

            gauss_params.dim_x    = vxlib_src.dim_x-(gsize_value-1);
            gauss_params.dim_y    = vxlib_src.dim_y-(gsize_value-1);
            gauss_params.stride_y = vxlib_src.stride_y;

            if (gsize_value == 3) {
                void *gaussOut;
                uint32_t gaussOut_size;
                status = tivxGetTargetKernelInstanceContext(kernel, &gaussOut, &gaussOut_size);
                if (VX_SUCCESS == status)
                {
                    pGauss = (uint8_t*)(gaussOut);
                    status |= VXLIB_gaussian_3x3_i8u_o8u(src_addr, &vxlib_src,
                                                     pGauss, &gauss_params);
                    status |= VXLIB_scaleImageNearest_i8u_o8u((uint8_t*)gaussOut, &gauss_params,
                                                          dst_addr, &vxlib_dst,
                                                          2, 2, 0, 0, 0, 0);
                }
            } else {
                status |= VXLIB_halfScaleGaussian_5x5_i8u_o8u(src_addr, &vxlib_src,
                                                              dst_addr, &vxlib_dst);
            }
        }
        else
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

    return status;
}

vx_status VX_CALLBACK tivxHalfscaleGaussianCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    void *temp_ptr;
    tivx_obj_desc_image_t *src_desc;
    tivx_obj_desc_scalar_t *gsize_desc;
    int32_t gsize_value = 0;

    if (num_params != TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {

        gsize_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_GSIZE_IDX];

        if( gsize_desc != NULL)
        {
            gsize_value = gsize_desc->data.s32;
        }

        if (gsize_value == 3)
        {

            src_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_SRC_IDX];

            temp_ptr = tivxMemAlloc(src_desc->imagepatch_addr[0].stride_y *
                src_desc->imagepatch_addr[0].dim_y, TIVX_MEM_EXTERNAL);

            if (NULL == temp_ptr)
            {
                status = VX_ERROR_NO_MEMORY;
            }
            else
            {
                memset(temp_ptr, 0, src_desc->imagepatch_addr[0].stride_y *
                    src_desc->imagepatch_addr[0].dim_y);
                tivxSetTargetKernelInstanceContext(kernel, temp_ptr,
                    (src_desc->imagepatch_addr[0].stride_y *
                    src_desc->imagepatch_addr[0].dim_y) );
            }
        }
    }

    return (status);
}

vx_status VX_CALLBACK tivxHalfscaleGaussianDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    void *temp_ptr;
    uint32_t temp_ptr_size;
    tivx_obj_desc_scalar_t *gsize_desc;
    int32_t gsize_value = 0;

    if (num_params != TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {

        gsize_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_GSIZE_IDX];

        if( gsize_desc != NULL)
        {
            gsize_value = gsize_desc->data.s32;
        }

        if (gsize_value == 3)
        {

            status = tivxGetTargetKernelInstanceContext(kernel, &temp_ptr, &temp_ptr_size);

            if (VXLIB_SUCCESS != status)
            {
                status = VX_ERROR_NO_MEMORY;
            }
            else
            {
                tivxMemFree(temp_ptr, temp_ptr_size, TIVX_MEM_EXTERNAL);
            }
        }
    }

    return (status);
}

vx_status VX_CALLBACK tivxHalfscaleGaussianControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelHalfscaleGaussian(void)
{
    vx_status status = VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ( self_cpu == TIVX_CPU_ID_DSP1 )
    {
        strncpy(target_name, TIVX_TARGET_DSP1, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
    else
    if ( self_cpu == TIVX_CPU_ID_DSP2 )
    {
        strncpy(target_name, TIVX_TARGET_DSP2, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
    else
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        vx_halfscale_gaussian_target_kernel = tivxAddTargetKernel(
                            VX_KERNEL_HALFSCALE_GAUSSIAN,
                            target_name,
                            tivxHalfscaleGaussian,
                            tivxHalfscaleGaussianCreate,
                            tivxHalfscaleGaussianDelete,
                            tivxHalfscaleGaussianControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelHalfscaleGaussian(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_halfscale_gaussian_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_halfscale_gaussian_target_kernel = NULL;
    }
}


