/*
 *
 * Copyright (c) 2017-2019 Texas Instruments Incorporated
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
#include "tivx_kernel_not_not.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "ti/vxlib/vxlib.h"
#include "tivx_bam_kernel_wrapper.h"

typedef struct
{
    tivx_bam_graph_handle graph_handle;
} tivxNotNotParams;

#define SOURCE_NODE     0
#define NOT1_NODE       1
#define NOT2_NODE       2
#define SINK_NODE       3

static tivx_target_kernel vx_not_not_target_kernel = NULL;

/* The vxlib_not_i8u_o8u is already registered as part of TIOVX, so we
 * are prefixing this one with "test" to register it as a different
 * plugin ID to test the API */
static tivx_bam_plugin_def testNotBamPlugin = {
    &gBAM_VXLIB_not_i8u_o8u_kernel,
    &gBAM_VXLIB_not_i8u_o8u_helperFunc,
    &gBAM_VXLIB_not_i8u_o8u_execFunc,
    "test_vxlib_not_i8u_o8u"
};
static BAM_KernelId testNotKernelId;

static vx_status VX_CALLBACK tivxNotNotProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxNotNotCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxNotNotDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxNotNotProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxNotNotParams *prms = NULL;
    tivx_obj_desc_image_t *input_desc;
    tivx_obj_desc_image_t *output_desc;

    if ( (num_params != TIVX_KERNEL_NOT_NOT_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_NOT_NOT_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_NOT_NOT_OUTPUT_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        uint32_t size;
        input_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_NOT_NOT_INPUT_IDX];
        output_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_NOT_NOT_OUTPUT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);
        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxNotNotParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if(VX_SUCCESS == status)
    {

        void *input_target_ptr;
        void *output_target_ptr;

        input_target_ptr = tivxMemShared2TargetPtr(&input_desc->mem_ptr[0]);
        tivxCheckStatus(&status, tivxMemBufferMap(input_target_ptr,
           input_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY));

        output_target_ptr = tivxMemShared2TargetPtr(&output_desc->mem_ptr[0]);
        tivxCheckStatus(&status, tivxMemBufferMap(output_target_ptr,
           output_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
           VX_WRITE_ONLY));

        {
            void *img_ptrs[2];
            VXLIB_bufParams2D_t vxlib_input;
            uint8_t *input_addr = NULL;
            VXLIB_bufParams2D_t vxlib_output;
            uint8_t *output_addr = NULL;

            tivxInitBufParams(input_desc, &vxlib_input);
            tivxSetPointerLocation(input_desc, &input_target_ptr, &input_addr);

            tivxInitBufParams(output_desc, &vxlib_output);
            tivxSetPointerLocation(output_desc, &output_target_ptr, &output_addr);

            img_ptrs[0] = input_addr;
            img_ptrs[1] = output_addr;
            tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);

        }
        tivxCheckStatus(&status, tivxMemBufferUnmap(input_target_ptr,
           input_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY));

        tivxCheckStatus(&status, tivxMemBufferUnmap(output_target_ptr,
           output_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY));

    }

    return status;
}

static vx_status VX_CALLBACK tivxNotNotCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxNotNotParams *prms = NULL;
    tivx_obj_desc_image_t *input_desc;
    tivx_obj_desc_image_t *output_desc;
    BAM_KernelId localBamKernelId;
    tivx_bam_kernel_details_t kernel_details[4];

    if ( (num_params != TIVX_KERNEL_NOT_NOT_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_NOT_NOT_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_NOT_NOT_OUTPUT_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        {
            /* The logic in these brackets is not usally all needed, but added here for testing */
            status = tivxBamGetKernelIdFromName("test_negative_vxlib_not_i8u_o8u", &localBamKernelId);

            if (VX_SUCCESS == status)
            {
                if ( (BAM_KernelId)BAM_TI_KERNELID_UNDEFINED != localBamKernelId )
                {
                    status = VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR, "tivxBamGetKernelIdFromName failed negative test\n");
                }
            }

            if (VX_SUCCESS == status)
            {
                status = tivxBamGetKernelIdFromName("test_vxlib_not_i8u_o8u", &localBamKernelId);
                if ( testNotKernelId != localBamKernelId )
                {
                    status = VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR, "tivxBamGetKernelIdFromName failed to retrieve same id as when it was registered\n");
                }
            }

            if (VX_SUCCESS == status)
            {
                if ( testNotKernelId < BAM_KERNELID_EXTERNAL_START )
                {
                    status = VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR, "tivxBamGetKernelIdFromName failed to retrieve id in expected range\n");
                }
            }
        }

        if (VX_SUCCESS == status)
        {
            status = tivxBamInitKernelDetails(&kernel_details[0], 4, kernel);
        }

        if (VX_SUCCESS == status)
        {

            input_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_NOT_NOT_INPUT_IDX];
            output_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_NOT_NOT_OUTPUT_IDX];

            prms = tivxMemAlloc(sizeof(tivxNotNotParams), TIVX_MEM_EXTERNAL);
            if (NULL == prms)
            {
                status = VX_ERROR_NO_MEMORY;
                VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
            }
            else
            {
                VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
                VXLIB_bufParams2D_t *buf_params[2];

                memset(prms, 0, sizeof(tivxNotNotParams));

                BAM_NodeParams node_list[] = { \
                    {SOURCE_NODE, BAM_KERNELID_DMAREAD_AUTOINCREMENT, NULL}, \
                    {NOT1_NODE, localBamKernelId, NULL}, \
                    {NOT2_NODE, localBamKernelId, NULL}, \
                    {SINK_NODE, BAM_KERNELID_DMAWRITE_AUTOINCREMENT, NULL}, \
                    {BAM_END_NODE_MARKER,   0,                          NULL},\
                };

                BAM_VXLIB_not_i8u_o8u_getKernelInfo( NULL, &kernel_details[NOT1_NODE].kernel_info);
                BAM_VXLIB_not_i8u_o8u_getKernelInfo( NULL, &kernel_details[NOT2_NODE].kernel_info);

                BAM_EdgeParams edge_list[]= {\
                    {{SOURCE_NODE, 0},
                        {NOT1_NODE, BAM_VXLIB_NOT_I8U_O8U_INPUT_IMAGE_PORT}},\

                    {{NOT1_NODE, BAM_VXLIB_NOT_I8U_O8U_OUTPUT_PORT},\
                        {NOT2_NODE, BAM_VXLIB_NOT_I8U_O8U_INPUT_IMAGE_PORT}},\

                    {{NOT2_NODE, BAM_VXLIB_NOT_I8U_O8U_OUTPUT_PORT},\
                        {SINK_NODE, 0}},\

                    {{BAM_END_NODE_MARKER, 0},
                        {BAM_END_NODE_MARKER, 0}},\
                };

                tivxInitBufParams(input_desc, &vxlib_src);
                tivxInitBufParams(output_desc, &vxlib_dst);

                if (VX_SUCCESS == status)
                {
                    /* Fill in the frame level sizes of buffers here. If the port
                     * is optionally disabled, put NULL */
                    buf_params[0] = &vxlib_src;
                    buf_params[1] = &vxlib_dst;

                    kernel_details[SOURCE_NODE].compute_kernel_params = NULL;
                    kernel_details[NOT1_NODE].compute_kernel_params = NULL;
                    kernel_details[NOT2_NODE].compute_kernel_params = NULL;
                    kernel_details[SINK_NODE].compute_kernel_params = NULL;

                    status = tivxBamCreateHandleMultiNode(node_list,
                        sizeof(node_list)/sizeof(BAM_NodeParams),
                        edge_list,
                        sizeof(edge_list)/sizeof(BAM_EdgeParams),
                        buf_params, kernel_details,
                        &prms->graph_handle);

                    if (VX_SUCCESS == status)
                    {
                        tivxSetTargetKernelInstanceContext(kernel, prms,
                            sizeof(tivxNotNotParams));
                    }
                    else
                    {
                        status = VX_ERROR_NO_MEMORY;
                        VX_PRINT(VX_ZONE_ERROR, "Unable to create BAM handle\n");
                    }
                }
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxNotNotDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxNotNotParams *prms = NULL;
    uint32_t size;

    /* < DEVELOPER_TODO: (Optional) Add any target kernel delete code here (e.g. freeing */
    /*                   local memory buffers, etc) > */
    if ( (num_params != TIVX_KERNEL_NOT_NOT_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_NOT_NOT_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_NOT_NOT_OUTPUT_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);

        if ((NULL != prms) &&
            (sizeof(tivxNotNotParams) == size))
        {
            /* < DEVELOPER_TODO: Uncomment once BAM graph has been created > */
            tivxBamDestroyHandle(prms->graph_handle);
            tivxMemFree(prms, size, TIVX_MEM_EXTERNAL);
        }
    }

    return status;
}

void tivxAddTargetKernelBamNotNot(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_not_not_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_NOT_NOT_NAME,
                            target_name,
                            tivxNotNotProcess,
                            tivxNotNotCreate,
                            tivxNotNotDelete,
                            NULL,
                            NULL);
    }

    if (status == VX_SUCCESS)
    {
        tivxBamRegisterPlugin(&testNotBamPlugin, &testNotKernelId);
    }
}

void tivxRemoveTargetKernelBamNotNot(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_not_not_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_not_not_target_kernel = NULL;
    }
}


