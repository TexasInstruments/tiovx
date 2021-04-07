/*
 *
 * Copyright (c) 2021 Texas Instruments Incorporated
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
#include "TI/j7.h"
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_display_m2m.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"

/* < DEVELOPER_TODO: Uncomment if kernel context is needed > */
#if 0
typedef struct
{
} tivxDisplayM2MParams;

#endif
static tivx_target_kernel vx_display_m2m_target_kernel = NULL;

static vx_status VX_CALLBACK tivxDisplayM2MProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDisplayM2MCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDisplayM2MDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDisplayM2MControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxDisplayM2MProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    /* < DEVELOPER_TODO: Uncomment if kernel context is needed > */
    #if 0
    tivxDisplayM2MParams *prms = NULL;
    #endif
    tivx_obj_desc_user_data_object_t *configuration_desc;
    tivx_obj_desc_image_t *input_desc;
    tivx_obj_desc_image_t *output_desc;

    if ( (num_params != TIVX_KERNEL_DISPLAY_M2M_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        /* < DEVELOPER_TODO: Uncomment if kernel context is needed > */
        #if 0
        uint32_t size;
        #endif
        configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DISPLAY_M2M_CONFIGURATION_IDX];
        input_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DISPLAY_M2M_INPUT_IDX];
        output_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DISPLAY_M2M_OUTPUT_IDX];

        /* < DEVELOPER_TODO: Uncomment if kernel context is needed > */
        #if 0
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);
        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxDisplayM2MParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
        #endif
    }

    if((vx_status)VX_SUCCESS == status)
    {

        void *configuration_target_ptr;
        void *input_target_ptr;
        void *output_target_ptr;

        configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);
        tivxCheckStatus(&status, tivxMemBufferMap(configuration_target_ptr,
           configuration_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
           (vx_enum)VX_READ_ONLY));

        input_target_ptr = tivxMemShared2TargetPtr(&input_desc->mem_ptr[0]);
        tivxCheckStatus(&status, tivxMemBufferMap(input_target_ptr,
           input_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
           (vx_enum)VX_READ_ONLY));

        output_target_ptr = tivxMemShared2TargetPtr(&output_desc->mem_ptr[0]);
        tivxCheckStatus(&status, tivxMemBufferMap(output_target_ptr,
           output_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
           (vx_enum)VX_WRITE_ONLY));



        {
            VXLIB_bufParams2D_t vxlib_input;
            uint8_t *input_addr = NULL;
            VXLIB_bufParams2D_t vxlib_output;
            uint8_t *output_addr = NULL;

            tivxInitBufParams(input_desc, &vxlib_input);
            tivxSetPointerLocation(input_desc, &input_target_ptr, &input_addr);

            tivxInitBufParams(output_desc, &vxlib_output);
            tivxSetPointerLocation(output_desc, &output_target_ptr, &output_addr);

            /* call kernel processing function */

            /* < DEVELOPER_TODO: Add target kernel processing code here > */

            /* kernel processing function complete */

        }
        tivxCheckStatus(&status, tivxMemBufferUnmap(configuration_target_ptr,
           configuration_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));

        tivxCheckStatus(&status, tivxMemBufferUnmap(input_target_ptr,
           input_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));

        tivxCheckStatus(&status, tivxMemBufferUnmap(output_target_ptr,
           output_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_WRITE_ONLY));



    }

    return status;
}

static vx_status VX_CALLBACK tivxDisplayM2MCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    /* < DEVELOPER_TODO: Uncomment if kernel context is needed > */
    #if 0
    tivxDisplayM2MParams *prms = NULL;
    #endif

    /* < DEVELOPER_TODO: (Optional) Add any target kernel create code here (e.g. allocating */
    /*                   local memory buffers, one time initialization, etc) > */
    if ( (num_params != TIVX_KERNEL_DISPLAY_M2M_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        tivx_obj_desc_user_data_object_t *configuration_desc;

        configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DISPLAY_M2M_CONFIGURATION_IDX];

        if (configuration_desc->mem_size != sizeof(tivx_display_m2m_common_params_t))
        {
            VX_PRINT(VX_ZONE_ERROR, "User data object size on target does not match the size on host, possibly due to misalignment in data structure\n");
            status = (vx_status)VX_FAILURE;
        }
        /* < DEVELOPER_TODO: Uncomment if kernel context is needed > */
        #if 0
        prms = tivxMemAlloc(sizeof(tivxDisplayM2MParams), (vx_enum)TIVX_MEM_EXTERNAL);
        if (NULL != prms)
        {

        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxDisplayM2MParams));
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
        }
        #endif
    }

    return status;
}

static vx_status VX_CALLBACK tivxDisplayM2MDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    /* < DEVELOPER_TODO: Uncomment if kernel context is needed > */
    #if 0
    tivxDisplayM2MParams *prms = NULL;
    uint32_t size;
    #endif

    /* < DEVELOPER_TODO: (Optional) Add any target kernel delete code here (e.g. freeing */
    /*                   local memory buffers, etc) > */
    /* < DEVELOPER_TODO: Uncomment if kernel context is needed > */
    #if 0
    if ( (num_params != TIVX_KERNEL_DISPLAY_M2M_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);

        if ((NULL != prms) &&
            (sizeof(tivxDisplayM2MParams) == size))
        {
            tivxMemFree(prms, size, (vx_enum)TIVX_MEM_EXTERNAL);
        }
    }
    #endif

    return status;
}

static vx_status VX_CALLBACK tivxDisplayM2MControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* < DEVELOPER_TODO: (Optional) Add any target kernel control code here (e.g. commands */
    /*                   the user can call to modify the processing of the kernel at run-time) > */

    return status;
}

void tivxAddTargetKernelDisplayM2M(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ( self_cpu == (vx_enum)TIVX_CPU_ID_IPU1_0 )
    {
        strncpy(target_name, TIVX_TARGET_IPU1_0, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        status = (vx_status)VX_FAILURE;
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        vx_display_m2m_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_DISPLAY_M2M_NAME,
                            target_name,
                            tivxDisplayM2MProcess,
                            tivxDisplayM2MCreate,
                            tivxDisplayM2MDelete,
                            tivxDisplayM2MControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelDisplayM2M(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_display_m2m_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_display_m2m_target_kernel = NULL;
    }
}


