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

#include "TI/tivx.h"
#include "TI/tivx_capture.h"
#include "VX/vx.h"
#include "tivx_capture_kernels.h"
#include "tivx_kernel_image_intermediate.h"
#include "TI/tivx_target_kernel.h"
#include <TI/tivx_task.h>
#include "tivx_kernels_target_utils.h"

#define TEST_MODE_CREATE_FAILURE_VAL  10
#define TEST_MODE_PROCESS_FAILURE_VAL 200
#define TEST_MODE_DELETE_FAILURE_VAL  30

static tivx_target_kernel vx_image_intermediate_target_kernel = NULL;

static vx_status VX_CALLBACK tivxImageIntermediateProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxImageIntermediateCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxImageIntermediateDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxImageIntermediateControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status test_init_buf_params(
       tivx_obj_desc_image_t *obj_desc);

static vx_status test_init_buf_params(
       tivx_obj_desc_image_t *obj_desc)
{
    vx_status status = (vx_status)VX_SUCCESS;
    VXLIB_bufParams2D_t buf_params;
    vx_uint32 format_count = 15U, i, base_scale;
    vx_enum base_format;
    vx_enum format_list[] = {
        VX_DF_IMAGE_NV12,
        VX_DF_IMAGE_NV21,
        VX_DF_IMAGE_IYUV,
        VX_DF_IMAGE_YUV4,
        VX_DF_IMAGE_U8,
        VX_DF_IMAGE_U16,
        VX_DF_IMAGE_S16,
        VX_DF_IMAGE_RGBX,
        TIVX_DF_IMAGE_BGRX,
        VX_DF_IMAGE_U32,
        VX_DF_IMAGE_S32,
        VX_DF_IMAGE_RGB,
        VX_DF_IMAGE_YUYV,
        VX_DF_IMAGE_UYVY,
        TIVX_DF_IMAGE_RGB565,
        VX_DF_IMAGE_VIRT
    };
    uint32_t type_list[] = {
        VXLIB_UINT8,
        VXLIB_UINT8,
        VXLIB_UINT8,
        VXLIB_UINT8,
        VXLIB_UINT8,
        VXLIB_UINT16,
        VXLIB_INT16,
        VXLIB_UINT32,
        VXLIB_UINT32,
        VXLIB_UINT32,
        VXLIB_INT32,
        VXLIB_UINT24,
        VXLIB_UINT16,
        VXLIB_UINT16,
        VXLIB_UINT16
    };

    base_scale = obj_desc->imagepatch_addr[0].scale_x;
    base_format = obj_desc->format;

    obj_desc->imagepatch_addr[0].scale_x = 512U;
    for (i = 0U; i < format_count; i++)
    {
        obj_desc->format = format_list[i];
        tivxInitBufParams(obj_desc, &buf_params);
        if (buf_params.data_type != type_list[i])
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "Buf params data type does not match obj desc format\n");
        }
    }

    /* Set format of obj desc to invalid value*/
    obj_desc->format = format_list[format_count];
    tivxInitBufParams(obj_desc, &buf_params);

    /* Reset format and scale_x of obj desc*/
    obj_desc->format = base_format;
    obj_desc->imagepatch_addr[0].scale_x = base_scale;
    tivxInitBufParams(obj_desc, &buf_params);

    return status;
}

static vx_status VX_CALLBACK tivxImageIntermediateProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    tivx_obj_desc_image_t *src_desc;
    tivx_obj_desc_image_t *dst_desc;
    uint8_t *input_addr, *output_addr;

    if ( (num_params != TIVX_KERNEL_IMAGE_INTERMEDIATE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_IMAGE_INTERMEDIATE_IN_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_IMAGE_INTERMEDIATE_OUT_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        void *src_desc_target_ptr;
        void *dst_desc_target_ptr;

        /* Get the Src and Dst descriptors */
        src_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_IMAGE_INTERMEDIATE_IN_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_IMAGE_INTERMEDIATE_OUT_IDX];

        /* Get the target pointer from the shared pointer for all
           buffers */
        src_desc_target_ptr = tivxMemShared2TargetPtr(&src_desc->mem_ptr[0]);
        dst_desc_target_ptr = tivxMemShared2TargetPtr(&dst_desc->mem_ptr[0]);

        /* Map all buffers, which invalidates the cache */
        tivxCheckStatus(&status, tivxMemBufferMap(src_desc_target_ptr,
            src_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferMap(dst_desc_target_ptr,
            dst_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_WRITE_ONLY));

        tivxSetPointerLocation(src_desc, &src_desc_target_ptr, &input_addr);
        tivxSetPointerLocation(dst_desc, &dst_desc_target_ptr, &output_addr);

        memcpy(output_addr, input_addr, dst_desc->imagepatch_addr[0].stride_y*dst_desc->imagepatch_addr[0].dim_y);

        tivxCheckStatus(&status, tivxMemBufferUnmap(src_desc_target_ptr,
            src_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY));

        tivxCheckStatus(&status, tivxMemBufferUnmap(dst_desc_target_ptr,
           dst_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
           VX_WRITE_ONLY));
        
        status = test_init_buf_params(src_desc);
    }

    return status;
}

static vx_status VX_CALLBACK tivxImageIntermediateCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    #if 0
    tivx_obj_desc_image_t *in_desc;

    if ( (num_params != TIVX_KERNEL_IMAGE_INTERMEDIATE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_IMAGE_INTERMEDIATE_IN_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_IMAGE_INTERMEDIATE_OUT_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        in_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_IMAGE_INTERMEDIATE_IN_IDX];
    }

    if(VX_SUCCESS == status)
    {
        vx_uint8 in_value;

        in_value = in_desc->data.u08;

        if (in_value == TEST_MODE_CREATE_FAILURE_VAL)
        {
            status = VX_FAILURE;
        }
    }
    #endif

    return status;
}

static vx_status VX_CALLBACK tivxImageIntermediateDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    #if 0
    tivx_obj_desc_image_t *in_desc;

    if ( (num_params != TIVX_KERNEL_IMAGE_INTERMEDIATE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_IMAGE_INTERMEDIATE_IN_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_IMAGE_INTERMEDIATE_OUT_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        in_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_IMAGE_INTERMEDIATE_IN_IDX];
    }

    if(VX_SUCCESS == status)
    {
        vx_uint8 in_value;

        in_value = in_desc->data.u08;

        if (in_value == TEST_MODE_DELETE_FAILURE_VAL)
        {
            status = VX_FAILURE;
        }
    }
    #endif

    return status;
}

static vx_status VX_CALLBACK tivxImageIntermediateControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelImageIntermediate(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameMcu(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameMpu(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameC7x(target_name)))
    {
        vx_image_intermediate_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_IMAGE_INTERMEDIATE_NAME,
                            target_name,
                            tivxImageIntermediateProcess,
                            tivxImageIntermediateCreate,
                            tivxImageIntermediateDelete,
                            tivxImageIntermediateControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelImageIntermediate(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_image_intermediate_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_image_intermediate_target_kernel = NULL;
    }
}


