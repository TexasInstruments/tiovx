/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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
#include "tivx_test_kernels_kernels.h"
#include "tivx_kernel_pyramid_source.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include <TI/tivx_task.h>

typedef struct
{
    uint8_t local_val;
} tivxPyramidSourceParams;

static tivx_target_kernel vx_pyramid_source_target_kernel = NULL;

static vx_status VX_CALLBACK tivxPyramidSourceProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxPyramidSourceCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxPyramidSourceDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxPyramidSourceControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxPyramidSourceProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_pyramid_t *output_desc;
    tivx_obj_desc_image_t *img_output_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    vx_uint32 i;
    tivxPyramidSourceParams *prms = NULL;
    uint32_t size;
    vx_enum state;
    uint8_t *output_addr[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];

    if ( (num_params != TIVX_KERNEL_PYRAMID_SOURCE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_PYRAMID_SOURCE_OUTPUT_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        output_desc = (tivx_obj_desc_pyramid_t *)obj_desc[TIVX_KERNEL_PYRAMID_SOURCE_OUTPUT_IDX];

    }

    if(VX_SUCCESS == status)
    {

        void *output_target_ptr[TIVX_PYRAMID_MAX_LEVEL_OBJECTS] = {NULL};

        tivxGetObjDescList(output_desc->obj_desc_id, (tivx_obj_desc_t**)img_output_desc, output_desc->num_levels);
        for(i=0; i<output_desc->num_levels; i++)
        {
            output_target_ptr[i] = tivxMemShared2TargetPtr(&img_output_desc[i]->mem_ptr[0]);
            tivxCheckStatus(&status, tivxMemBufferMap(output_target_ptr[i],
               img_output_desc[i]->mem_size[0], VX_MEMORY_TYPE_HOST,
               VX_WRITE_ONLY));
        }



        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (VX_SUCCESS == status)
        {
            status = tivxGetTargetKernelInstanceState(kernel, &state);

            if (VX_SUCCESS == status)
            {
                if (VX_NODE_STATE_STEADY == state)
                {
                    if (255 == prms->local_val)
                    {
                        prms->local_val = 0;
                    }
                    else
                    {
                        prms->local_val++;
                    }

                    tivxTaskWaitMsecs(1);

                    for(i=0; i<output_desc->num_levels; i++)
                    {
                        tivxSetPointerLocation(img_output_desc[i], &output_target_ptr[i], &output_addr[i]);

                        memset(output_addr[i], prms->local_val, img_output_desc[i]->imagepatch_addr[0].stride_y*img_output_desc[i]->imagepatch_addr[0].dim_y);
                    }
                }
            }
        }

        for(i=0; i<output_desc->num_levels; i++)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(output_target_ptr[i],
               img_output_desc[i]->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY));
        }



    }

    return status;
}

static vx_status VX_CALLBACK tivxPyramidSourceCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxPyramidSourceParams *prms = NULL;

    prms = tivxMemAlloc(sizeof(tivxPyramidSourceParams), TIVX_MEM_EXTERNAL);

    tivxSetTargetKernelInstanceContext(kernel, prms,
       sizeof(tivxPyramidSourceParams));

    prms->local_val = 0;

    return status;
}

static vx_status VX_CALLBACK tivxPyramidSourceDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxPyramidSourceParams *prms = NULL;
    uint32_t size;

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&prms, &size);

    tivxMemFree(prms, sizeof(tivxPyramidSourceParams), TIVX_MEM_EXTERNAL);

    return status;
}

static vx_status VX_CALLBACK tivxPyramidSourceControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    /* < DEVELOPER_TODO: (Optional) Add any target kernel control code here (e.g. commands */
    /*                   the user can call to modify the processing of the kernel at run-time) > */

    return status;
}

void tivxAddTargetKernelPyramidSource(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    #if defined(SOC_AM62A)
    if ((self_cpu == TIVX_CPU_ID_MCU1_0))
    {
        strncpy(target_name, TIVX_TARGET_MCU1_0, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    #else
    if ( (self_cpu == TIVX_CPU_ID_MCU2_0) ||
          (self_cpu == TIVX_CPU_ID_MCU2_1))
    {
        if (self_cpu == TIVX_CPU_ID_MCU2_0)
        {
            strncpy(target_name, TIVX_TARGET_MCU2_0, TIVX_TARGET_MAX_NAME);
            status = (vx_status)VX_SUCCESS;
        }
        else if (self_cpu == TIVX_CPU_ID_MCU2_1)
        {
            strncpy(target_name, TIVX_TARGET_MCU2_1, TIVX_TARGET_MAX_NAME);
            status = (vx_status)VX_SUCCESS;
        }
    }
    #endif
    else
    {
        status = (vx_status)VX_FAILURE;
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        vx_pyramid_source_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_PYRAMID_SOURCE_NAME,
                            target_name,
                            tivxPyramidSourceProcess,
                            tivxPyramidSourceCreate,
                            tivxPyramidSourceDelete,
                            tivxPyramidSourceControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelPyramidSource(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_pyramid_source_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_pyramid_source_target_kernel = NULL;
    }
}


