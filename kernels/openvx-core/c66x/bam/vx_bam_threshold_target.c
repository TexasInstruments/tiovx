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



#include <TI/tivx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_target_kernels_priv.h>
#include <tivx_kernel_threshold.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_bam_kernel_wrapper.h>

typedef struct
{
    tivx_bam_graph_handle graph_handle;
    uint8_t bam_node_num;
} tivxThresholdParams;

static tivx_target_kernel vx_threshold_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelThresholdProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelThresholdCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelThresholdDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

/* Supernode Callbacks */
static vx_status VX_CALLBACK tivxKernelThresholdCreateInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    int32_t * bam_node_cnt, void * scratch, int32_t *size);

static vx_status VX_CALLBACK tivxKernelThresholdGetNodePort(
    tivx_target_kernel_instance kernel, uint8_t ovx_port, uint8_t plane,
    uint8_t *bam_node, uint8_t *bam_port);


static vx_status VX_CALLBACK tivxKernelThresholdProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxThresholdParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    vx_uint8 *src_addr, *dst_addr;
    uint32_t size;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_THRESHOLD_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_THRESHOLD_INPUT_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_THRESHOLD_OUTPUT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxThresholdParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *img_ptrs[2];
        void *src_target_ptr;
        void *dst_target_ptr;

        src_target_ptr = tivxMemShared2TargetPtr(&src->mem_ptr[0U]);
        dst_target_ptr = tivxMemShared2TargetPtr(&dst->mem_ptr[0U]);

        tivxSetPointerLocation(src, &src_target_ptr, &src_addr);
        tivxSetPointerLocation(dst, &dst_target_ptr, &dst_addr);

        img_ptrs[0] = src_addr;
        img_ptrs[1] = dst_addr;
        tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

        status  = tivxBamProcessGraph(prms->graph_handle);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelThresholdCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_threshold_t *thr;
    tivxThresholdParams *prms = NULL;
    tivx_bam_kernel_details_t kernel_details;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_THRESHOLD_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxBamInitKernelDetails(&kernel_details, 1, kernel);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_THRESHOLD_INPUT_IDX];
        thr = (tivx_obj_desc_threshold_t *)obj_desc[
            TIVX_KERNEL_THRESHOLD_THRESH_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_THRESHOLD_OUTPUT_IDX];

        prms = tivxMemAlloc(sizeof(tivxThresholdParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
            VXLIB_bufParams2D_t *buf_params[2];

            memset(prms, 0, sizeof(tivxThresholdParams));

            tivxInitBufParams(src, &vxlib_src);
            tivxInitBufParams(dst, &vxlib_dst);

            /* Fill in the frame level sizes of buffers here. If the port
             * is optionally disabled, put NULL */
            buf_params[0] = &vxlib_src;
            buf_params[1] = &vxlib_dst;

            if ((vx_enum)VX_THRESHOLD_TYPE_BINARY == thr->type)
            {
                BAM_VXLIB_thresholdBinary_i8u_o8u_params kernel_params;
                kernel_params.threshold  = (uint8_t)thr->value;
                kernel_params.trueValue  = (uint8_t)thr->true_value;
                kernel_params.falseValue = (uint8_t)thr->false_value;

                kernel_details.compute_kernel_params = (void*)&kernel_params;

                BAM_VXLIB_thresholdBinary_i8u_o8u_getKernelInfo( &kernel_params,
                                                             &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(BAM_KERNELID_VXLIB_THRESHOLDBINARY_I8U_O8U,
                                                           buf_params, &kernel_details,
                                                           &prms->graph_handle);
            }
            else
            {
                BAM_VXLIB_thresholdRange_i8u_o8u_params kernel_params;
                kernel_params.upper  = (uint8_t)thr->upper;
                kernel_params.lower  = (uint8_t)thr->lower;
                kernel_params.trueValue  = (uint8_t)thr->true_value;
                kernel_params.falseValue = (uint8_t)thr->false_value;

                kernel_details.compute_kernel_params = (void*)&kernel_params;

                BAM_VXLIB_thresholdRange_i8u_o8u_getKernelInfo( &kernel_params,
                                                             &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(BAM_KERNELID_VXLIB_THRESHOLDRANGE_I8U_O8U,
                                                           buf_params, &kernel_details,
                                                           &prms->graph_handle);
            }
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxThresholdParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxThresholdParams), (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelThresholdDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t size;
    tivxThresholdParams *prms = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_THRESHOLD_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxThresholdParams) == size))
        {
            if(NULL != prms->graph_handle)
            {
                tivxBamDestroyHandle(prms->graph_handle);
            }
            tivxMemFree(prms, sizeof(tivxThresholdParams), (vx_enum)TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

void tivxAddTargetKernelBamThreshold(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_threshold_target_kernel = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_THRESHOLD,
            target_name,
            tivxKernelThresholdProcess,
            tivxKernelThresholdCreate,
            tivxKernelThresholdDelete,
            NULL,
            NULL);

        tivxEnableKernelForSuperNode(vx_threshold_target_kernel,
            tivxKernelThresholdCreateInBamGraph,
            tivxKernelThresholdGetNodePort,
            NULL,
            NULL,
            NULL,
            MAX2((int32_t)sizeof(BAM_VXLIB_thresholdBinary_i8u_o8u_params),
                 (int32_t)sizeof(BAM_VXLIB_thresholdRange_i8u_o8u_params)),
            NULL);
    }
}


void tivxRemoveTargetKernelBamThreshold(void)
{
    tivxRemoveTargetKernel(vx_threshold_target_kernel);
}

static vx_status VX_CALLBACK tivxKernelThresholdCreateInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    int32_t * bam_node_cnt, void * scratch, int32_t *size)
{

    vx_status status = (vx_status)VX_SUCCESS;
    tivxThresholdParams *prms = NULL;
    tivx_obj_desc_threshold_t *thr;

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_THRESHOLD_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        thr = (tivx_obj_desc_threshold_t *)obj_desc[
            TIVX_KERNEL_THRESHOLD_THRESH_IDX];

        prms = tivxMemAlloc(sizeof(tivxThresholdParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxThresholdParams));

            node_list[*bam_node_cnt].nodeIndex = (uint8_t)*bam_node_cnt;
            node_list[*bam_node_cnt].kernelArgs = NULL;

            if ((vx_enum)VX_THRESHOLD_TYPE_BINARY == thr->type)
            {
                BAM_VXLIB_thresholdBinary_i8u_o8u_params *kernel_params = (BAM_VXLIB_thresholdBinary_i8u_o8u_params*)scratch;

                if ((NULL != kernel_params) &&
                    (*size >= (int32_t)sizeof(BAM_VXLIB_thresholdBinary_i8u_o8u_params)))
                {
                    node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_THRESHOLDBINARY_I8U_O8U;

                    kernel_params->threshold  = (uint8_t)thr->value;
                    kernel_params->trueValue  = (uint8_t)thr->true_value;
                    kernel_params->falseValue = (uint8_t)thr->false_value;

                    kernel_details[*bam_node_cnt].compute_kernel_params = (void*)kernel_params;

                    BAM_VXLIB_thresholdBinary_i8u_o8u_getKernelInfo(kernel_params,
                        &kernel_details[*bam_node_cnt].kernel_info);
                }
                else
                {
                    status = (vx_status)VX_FAILURE;
                }
            }
            else
            {
                BAM_VXLIB_thresholdRange_i8u_o8u_params *kernel_params = (BAM_VXLIB_thresholdRange_i8u_o8u_params*)scratch;

                if ((NULL != kernel_params) &&
                    (*size >= (int32_t)sizeof(BAM_VXLIB_thresholdRange_i8u_o8u_params)))
                {
                    node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_THRESHOLDRANGE_I8U_O8U;

                    kernel_params->upper  = (uint8_t)thr->upper;
                    kernel_params->lower  = (uint8_t)thr->lower;
                    kernel_params->trueValue  = (uint8_t)thr->true_value;
                    kernel_params->falseValue = (uint8_t)thr->false_value;

                    kernel_details[*bam_node_cnt].compute_kernel_params = (void*)kernel_params;

                    BAM_VXLIB_thresholdRange_i8u_o8u_getKernelInfo(kernel_params,
                                                                 &kernel_details[*bam_node_cnt].kernel_info);
                }
                else
                {
                    status = (vx_status)VX_FAILURE;
                }
            }
            prms->bam_node_num = (uint8_t)*bam_node_cnt;
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxThresholdParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxThresholdParams), (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelThresholdGetNodePort(
    tivx_target_kernel_instance kernel,
    uint8_t ovx_port, uint8_t plane, uint8_t *bam_node, uint8_t *bam_port)
{
    tivxThresholdParams *prms = NULL;
    uint32_t size;

    vx_status status = tivxGetTargetKernelInstanceContext(kernel,
                        (void **)&prms, &size);

    if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
        (sizeof(tivxThresholdParams) == size))
    {
        switch (ovx_port)
        {
            case TIVX_KERNEL_THRESHOLD_INPUT_IDX:
                *bam_node = prms->bam_node_num;
                *bam_port = (uint8_t)BAM_VXLIB_THRESHOLDBINARY_I8U_O8U_INPUT_IMAGE_PORT;
                break;
            case TIVX_KERNEL_THRESHOLD_OUTPUT_IDX:
                *bam_node = prms->bam_node_num;
                *bam_port = (uint8_t)BAM_VXLIB_THRESHOLDBINARY_I8U_O8U_OUTPUT_PORT;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "non existing index queried by tivxKernelSupernodeCreate.tivxGetNodePort()\n");
                status = (vx_status)VX_FAILURE;
                break;
        }
    }

    return status;
}
