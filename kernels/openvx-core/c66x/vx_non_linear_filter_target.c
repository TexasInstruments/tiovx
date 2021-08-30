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
#include <tivx_kernel_non_linear_filter.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

static tivx_target_kernel vx_non_linear_filter_target_kernel = NULL;

static vx_status VX_CALLBACK tivxNonLinearFilter(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxNonLinearFilterCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxNonLinearFilterDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxNonLinearFilter(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_scalar_t *function_desc;
    tivx_obj_desc_image_t *src_desc;
    tivx_obj_desc_matrix_t *mask_desc;
    tivx_obj_desc_image_t *dst_desc;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst, mask_params;
    uint8_t *src_addr;
    uint8_t *dst_addr;
    uint8_t *mask_addr;

    if ((num_params != TIVX_KERNEL_NON_LINEAR_FILTER_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_FUNCTION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_MASK_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_OUTPUT_IDX]))
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        void *src_desc_target_ptr;
        void *mask_desc_target_ptr;
        void *dst_desc_target_ptr;

        function_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_FUNCTION_IDX];
        src_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_INPUT_IDX];
        mask_desc = (tivx_obj_desc_matrix_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_MASK_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_OUTPUT_IDX];

        src_desc_target_ptr = tivxMemShared2TargetPtr(&src_desc->mem_ptr[0]);
        mask_desc_target_ptr = tivxMemShared2TargetPtr(&mask_desc->mem_ptr);
        dst_desc_target_ptr = tivxMemShared2TargetPtr(&dst_desc->mem_ptr[0]);

        tivxCheckStatus(&status, tivxMemBufferMap(src_desc_target_ptr,
           src_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferMap(mask_desc_target_ptr,
           mask_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferMap(dst_desc_target_ptr,
           dst_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_WRITE_ONLY));

        mask_addr = (uint8_t *)((uintptr_t)mask_desc_target_ptr);

        tivxSetPointerLocation(src_desc, &src_desc_target_ptr, &src_addr);
        tivxSetPointerLocation(dst_desc, &dst_desc_target_ptr, &dst_addr);

        tivxInitBufParams(src_desc, &vxlib_src);
        tivxInitBufParams(dst_desc, &vxlib_dst);

        mask_params.dim_x    = mask_desc->columns;
        mask_params.dim_y    = mask_desc->rows;
        mask_params.stride_y = (int32_t)mask_desc->columns;
        mask_params.data_type = (uint32_t)VXLIB_UINT8;

        if ((vx_enum)VX_NONLINEAR_FILTER_MIN == function_desc->data.enm)
        {
            vx_uint32 temp_status = (vx_uint32)status;
            temp_status |= (vx_uint32)VXLIB_erode_MxN_i8u_i8u_o8u(src_addr, &vxlib_src,
                                                  dst_addr, &vxlib_dst,
                                                  mask_addr, &mask_params);
            status = (vx_status)temp_status;
        }
        else if ((vx_enum)VX_NONLINEAR_FILTER_MAX == function_desc->data.enm)
        {
            vx_uint32 temp_status = (vx_uint32)status;
            temp_status |= (vx_uint32)VXLIB_dilate_MxN_i8u_i8u_o8u(src_addr, &vxlib_src,
                                                   dst_addr, &vxlib_dst,
                                                   mask_addr, &mask_params);
            status = (vx_status)temp_status;
        }
        else
        {
            void *scratch;
            uint32_t scratch_size;
            status = tivxGetTargetKernelInstanceContext(
                            kernel,
                            &scratch, &scratch_size);

            if(status==(vx_status)VX_SUCCESS)
            {
                vx_uint32 temp_status = (vx_uint32)status;
                temp_status |= (vx_uint32)VXLIB_median_MxN_i8u_i8u_o8u(
                                        src_addr, &vxlib_src,
                                        dst_addr, &vxlib_dst,
                                        mask_addr, &mask_params,
                                        (int64_t*)scratch, scratch_size);
                status = (vx_status)temp_status;
            }
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(src_desc_target_ptr,
           src_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferUnmap(mask_desc_target_ptr,
           mask_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferUnmap(dst_desc_target_ptr,
           dst_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_WRITE_ONLY));


    }

    return status;
}

static vx_status VX_CALLBACK tivxNonLinearFilterCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    void *temp_ptr;
    tivx_obj_desc_scalar_t *function_desc;
    tivx_obj_desc_matrix_t *mask_desc;

    if (num_params != TIVX_KERNEL_NON_LINEAR_FILTER_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_NON_LINEAR_FILTER_MAX_PARAMS; i ++)
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
        function_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_FUNCTION_IDX];

        if ( ((vx_enum)VX_NONLINEAR_FILTER_MIN != function_desc->data.enm) &&
             ((vx_enum)VX_NONLINEAR_FILTER_MAX != function_desc->data.enm) )
        {
            mask_desc = (tivx_obj_desc_matrix_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_MASK_IDX];

            temp_ptr = tivxMemAlloc(mask_desc->columns*mask_desc->rows*2U*
                sizeof(int64_t), (vx_enum)TIVX_MEM_EXTERNAL);

            if (NULL == temp_ptr)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
            else
            {
                memset(temp_ptr, 0, mask_desc->columns*mask_desc->rows*2U*sizeof(int64_t));
                tivxSetTargetKernelInstanceContext(kernel, temp_ptr,
                    mask_desc->columns*mask_desc->rows*2U*sizeof(int64_t));
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxNonLinearFilterDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    void *temp_ptr;
    tivx_obj_desc_scalar_t *function_desc;
    uint32_t temp_ptr_size;

    if (num_params != TIVX_KERNEL_NON_LINEAR_FILTER_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_NON_LINEAR_FILTER_MAX_PARAMS; i ++)
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
        function_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_FUNCTION_IDX];

        if ( ((vx_enum)VX_NONLINEAR_FILTER_MIN != function_desc->data.enm) &&
             ((vx_enum)VX_NONLINEAR_FILTER_MAX != function_desc->data.enm) )
        {
            status = tivxGetTargetKernelInstanceContext(kernel, &temp_ptr, &temp_ptr_size);

            if ((vx_status)VXLIB_SUCCESS != status)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
            else
            {
                tivxMemFree(temp_ptr, temp_ptr_size, (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return (status);
}

void tivxAddTargetKernelNonLinearFilter(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_non_linear_filter_target_kernel = tivxAddTargetKernel(
                            (vx_enum)VX_KERNEL_NON_LINEAR_FILTER,
                            target_name,
                            tivxNonLinearFilter,
                            tivxNonLinearFilterCreate,
                            tivxNonLinearFilterDelete,
                            NULL,
                            NULL);
    }
}

void tivxRemoveTargetKernelNonLinearFilter(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_non_linear_filter_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_non_linear_filter_target_kernel = NULL;
    }
}


