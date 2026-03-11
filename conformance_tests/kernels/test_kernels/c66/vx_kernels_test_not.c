/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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
#include "TI/tivx_test_kernels.h"
#include "VX/vx.h"
#include "tivx_kernel_test_not.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"

static tivx_target_kernel vx_test_not_target_kernel[C7X_COUNT] = {NULL};

static vx_enum additional_dsp_target_ids[] = {
#if defined(SOC_J721S2)
    TIVX_CPU_ID_DSP1
#elif (C7X_COUNT > 1)
    TIVX_CPU_ID_DSP_C7_2,
#endif
#if (C7X_COUNT > 2)
    TIVX_CPU_ID_DSP_C7_3,
#endif
#if (C7X_COUNT > 3)
    TIVX_CPU_ID_DSP_C7_4,
#endif
};

static char additional_dsp_targets[][TIVX_TARGET_MAX_NAME] = {
#if defined(SOC_J721S2)
    TIVX_TARGET_DSP1,
#elif (C7X_COUNT > 1)
    TIVX_TARGET_DSP_C7_2,
#endif
#if (C7X_COUNT > 2)
    TIVX_TARGET_DSP_C7_3,
#endif
#if (C7X_COUNT > 3)
    TIVX_TARGET_DSP_C7_4
#endif
};

static vx_status VX_CALLBACK tivxTestNotProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxTestNotCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxTestNotDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxTestNotControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxTestNotProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *input_desc, *output_desc;

    if ( (num_params != TIVX_KERNEL_TEST_NOT_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_TEST_NOT_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_TEST_NOT_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        uint8_t *input_desc_target_ptr;
        uint8_t *output_desc_target_ptr;

        /* Get the Src and Dst descriptors */
        input_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_TEST_NOT_INPUT_IDX];
        output_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_TEST_NOT_OUTPUT_IDX];

        /* Get the target pointer from the shared pointer for all
           buffers */
        input_desc_target_ptr = (uint8_t *)tivxMemShared2TargetPtr(&input_desc->mem_ptr[0]);
        output_desc_target_ptr = (uint8_t *)tivxMemShared2TargetPtr(&output_desc->mem_ptr[0]);

        /* Map all buffers, which invalidates the cache */
        tivxCheckStatus(&status, tivxMemBufferMap(input_desc_target_ptr,
           input_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
           (vx_enum)VX_READ_ONLY));

        tivxCheckStatus(&status, tivxMemBufferMap(output_desc_target_ptr,
           output_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
           (vx_enum)VX_WRITE_ONLY));

        /* Get image dimensions and stride from descriptor */
        uint32_t width = input_desc->imagepatch_addr[0].dim_x;
        uint32_t height = input_desc->imagepatch_addr[0].dim_y;
        uint32_t input_stride = input_desc->imagepatch_addr[0].stride_y;
        uint32_t output_stride = output_desc->imagepatch_addr[0].stride_y;
        uint32_t x, y, in_offset, out_offset;

        /* Stride through the image, inverting each pixel */
        for (y = 0; y < height; y++)
            for (x = 0; x < width; x++)
            {
                in_offset = (y * input_stride) + x;
                out_offset = (y * output_stride) + x;
                output_desc_target_ptr[out_offset] = ~input_desc_target_ptr[in_offset];
            }

        tivxCheckStatus(&status, tivxMemBufferUnmap(input_desc_target_ptr,
           input_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));

        tivxCheckStatus(&status, tivxMemBufferUnmap(output_desc_target_ptr,
           output_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_WRITE_ONLY));
    }

    return status;
}

static vx_status VX_CALLBACK tivxTestNotCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return status;
}

static vx_status VX_CALLBACK tivxTestNotDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return status;
}

static vx_status VX_CALLBACK tivxTestNotControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelTestNot(void)
{
    vx_enum self_cpu;
    int8_t i;

    self_cpu = tivxGetSelfCpuId();
    if (self_cpu == (vx_enum)TIVX_CPU_ID_DSP_C7_1)
    {
        vx_test_not_target_kernel[0] = tivxAddTargetKernelByName(
                            TIVX_KERNEL_TEST_NOT_NAME,
                            TIVX_TARGET_DSP_C7_1,
                            tivxTestNotProcess,
                            tivxTestNotCreate,
                            tivxTestNotDelete,
                            tivxTestNotControl,
                            NULL);
    }
    else
    {
        for (i = 0; i < C7X_COUNT - 1; i++)
        {
            if (self_cpu == (vx_enum)additional_dsp_target_ids[i])
            {
                vx_test_not_target_kernel[i + 1] = tivxAddTargetKernelByName(
                                    TIVX_KERNEL_TEST_NOT_NAME,
                                    additional_dsp_targets[i],
                                    tivxTestNotProcess,
                                    tivxTestNotCreate,
                                    tivxTestNotDelete,
                                    tivxTestNotControl,
                                    NULL);
            }
        }
    }
}

void tivxRemoveTargetKernelTestNot(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint8_t i;

    for (i = 0; i < C7X_COUNT; i++)
    {
        status = tivxRemoveTargetKernel(vx_test_not_target_kernel[i]);
        if (status == VX_SUCCESS)
        {
            vx_test_not_target_kernel[i] = NULL;
        }
    }
}
