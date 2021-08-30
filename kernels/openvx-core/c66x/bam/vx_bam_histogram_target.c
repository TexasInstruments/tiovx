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
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_target_kernels_priv.h>
#include <tivx_kernel_histogram.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_bam_kernel_wrapper.h>

typedef struct
{
    tivx_bam_graph_handle graph_handle;
    uint8_t bam_node_num;
} tivxHistogramParams;

static tivx_target_kernel vx_histogram_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelHistogramProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelHistogramCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelHistogramDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

/* Supernode Callbacks */
static vx_status VX_CALLBACK tivxKernelHistogramCreateInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    int32_t * bam_node_cnt, void * scratch, int32_t *size);

static vx_status VX_CALLBACK tivxKernelHistogramGetNodePort(
    tivx_target_kernel_instance kernel, uint8_t ovx_port, uint8_t plane,
    uint8_t *bam_node, uint8_t *bam_port);

static vx_status VX_CALLBACK tivxKernelHistogramPreprocessInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, tivx_bam_graph_handle *g_handle , void *priv_arg);

static vx_status VX_CALLBACK tivxKernelHistogramPostprocessInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, tivx_bam_graph_handle *g_handle, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelHistogramProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxHistogramParams *prms = NULL;
    tivx_obj_desc_image_t *src;
    tivx_obj_desc_distribution_t *dst;
    uint8_t *src_addr;
    uint32_t size;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_HISTOGRAM_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HISTOGRAM_INPUT_IDX];
        dst = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_HISTOGRAM_DISTRIBUTION_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxHistogramParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *img_ptrs[1];
        void *src_target_ptr;
        void *dst_target_ptr;

        src_target_ptr = tivxMemShared2TargetPtr(&src->mem_ptr[0]);
        dst_target_ptr = tivxMemShared2TargetPtr(&dst->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(dst_target_ptr, dst->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        tivxSetPointerLocation(src, &src_target_ptr, &src_addr);

        img_ptrs[0] = src_addr;
        tivxBamUpdatePointers(prms->graph_handle, 1U, 0U, img_ptrs);

        tivxBamControlNode(prms->graph_handle, 0,
                           VXLIB_HISTOGRAM_I8U_O32U_CMD_SET_DIST_PTR,
                           dst_target_ptr);

        status  = tivxBamProcessGraph(prms->graph_handle);

        tivxCheckStatus(&status, tivxMemBufferUnmap(dst_target_ptr, dst->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelHistogramCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src;
    tivx_obj_desc_distribution_t *dst;
    tivxHistogramParams *prms = NULL;
    tivx_bam_kernel_details_t kernel_details;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_HISTOGRAM_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxBamInitKernelDetails(&kernel_details, 1, kernel);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_HISTOGRAM_INPUT_IDX];
        dst = (tivx_obj_desc_distribution_t *)obj_desc[
            TIVX_KERNEL_HISTOGRAM_DISTRIBUTION_IDX];

        prms = tivxMemAlloc(sizeof(tivxHistogramParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            BAM_VXLIB_histogram_i8u_o32u_params kernel_params;
            VXLIB_bufParams2D_t vxlib_src;
            VXLIB_bufParams2D_t *buf_params[1];

            memset(prms, 0, sizeof(tivxHistogramParams));

            tivxInitBufParams(src, &vxlib_src);

            /* Fill in the frame level sizes of buffers here. If the port
             * is optionally disabled, put NULL */
            buf_params[0] = &vxlib_src;

            kernel_params.offset          = (uint8_t)dst->offset;
            kernel_params.range           = (uint16_t)dst->range;
            kernel_params.numBins         = (uint16_t)dst->num_bins;

            kernel_details.compute_kernel_params = (void*)&kernel_params;

            BAM_VXLIB_histogram_i8u_o32u_getKernelInfo( &kernel_params,
                                                        &kernel_details.kernel_info);

            status = tivxBamCreateHandleSingleNode(BAM_KERNELID_VXLIB_HISTOGRAM_I8U_O32U,
                                                   buf_params, &kernel_details,
                                                   &prms->graph_handle);
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxHistogramParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxHistogramParams), (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelHistogramDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t size;
    tivxHistogramParams *prms = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_HISTOGRAM_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxHistogramParams) == size))
        {
            if(NULL != prms->graph_handle)
            {
                tivxBamDestroyHandle(prms->graph_handle);
            }
            tivxMemFree(prms, sizeof(tivxHistogramParams), (vx_enum)TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

void tivxAddTargetKernelBamHistogram(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_histogram_target_kernel = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_HISTOGRAM,
            target_name,
            tivxKernelHistogramProcess,
            tivxKernelHistogramCreate,
            tivxKernelHistogramDelete,
            NULL,
            NULL);

        tivxEnableKernelForSuperNode(vx_histogram_target_kernel,
            tivxKernelHistogramCreateInBamGraph,
            tivxKernelHistogramGetNodePort,
            NULL,
            tivxKernelHistogramPreprocessInBamGraph,
            tivxKernelHistogramPostprocessInBamGraph,
            (int32_t)sizeof(BAM_VXLIB_histogram_i8u_o32u_params),
            NULL);
    }
}


void tivxRemoveTargetKernelBamHistogram(void)
{
    tivxRemoveTargetKernel(vx_histogram_target_kernel);
}

static vx_status VX_CALLBACK tivxKernelHistogramCreateInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    int32_t * bam_node_cnt, void * scratch, int32_t *size)
{

    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_distribution_t *dist;
    tivxHistogramParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_HISTOGRAM_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        dist = (tivx_obj_desc_distribution_t *)obj_desc[
            TIVX_KERNEL_HISTOGRAM_DISTRIBUTION_IDX];

        prms = tivxMemAlloc(sizeof(tivxHistogramParams), (vx_enum)TIVX_MEM_EXTERNAL);

        BAM_VXLIB_histogram_i8u_o32u_params *kernel_params = (BAM_VXLIB_histogram_i8u_o32u_params*)scratch;

        if ((NULL == kernel_params) || (NULL == prms) ||
            ((int32_t)sizeof(BAM_VXLIB_histogram_i8u_o32u_params) != *size))
        {
            status = (vx_status)VX_FAILURE;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            memset(prms, 0, sizeof(tivxHistogramParams));

            node_list[*bam_node_cnt].nodeIndex = (uint8_t)*bam_node_cnt;
            node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_HISTOGRAM_I8U_O32U;
            node_list[*bam_node_cnt].kernelArgs = NULL;

            kernel_params->offset          = (uint8_t)dist->offset;
            kernel_params->range           = (uint16_t)dist->range;
            kernel_params->numBins         = (uint16_t)dist->num_bins;

            kernel_details[*bam_node_cnt].compute_kernel_params = (void*)kernel_params;

            BAM_VXLIB_histogram_i8u_o32u_getKernelInfo(kernel_params,
                &kernel_details[*bam_node_cnt].kernel_info);

            prms->bam_node_num = (uint8_t)*bam_node_cnt;
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxHistogramParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxHistogramParams), (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelHistogramGetNodePort(
    tivx_target_kernel_instance kernel,
    uint8_t ovx_port, uint8_t plane, uint8_t *bam_node, uint8_t *bam_port)
{
    tivxHistogramParams *prms = NULL;
    uint32_t size;

    vx_status status = tivxGetTargetKernelInstanceContext(kernel,
                        (void **)&prms, &size);

    if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
        (sizeof(tivxHistogramParams) == size))
    {
        switch (ovx_port)
        {
            case TIVX_KERNEL_HISTOGRAM_INPUT_IDX:
                *bam_node = prms->bam_node_num;
                *bam_port = (uint8_t)BAM_VXLIB_HISTOGRAM_I8U_O32U_INPUT_IMAGE_PORT;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "non existing index queried by tivxKernelSupernodeCreate.tivxGetNodePort()\n");
                status = (vx_status)VX_FAILURE;
                break;
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelHistogramPreprocessInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, tivx_bam_graph_handle *g_handle, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxHistogramParams *prms = NULL;
    tivx_obj_desc_distribution_t *dst;
    uint32_t size;

    dst = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_HISTOGRAM_DISTRIBUTION_IDX];

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&prms, &size);

    if (((vx_status)VX_SUCCESS != status) || (NULL == prms) || (NULL == g_handle) ||
        (sizeof(tivxHistogramParams) != size))
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *dst_target_ptr;

        dst_target_ptr = tivxMemShared2TargetPtr(&dst->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(dst_target_ptr, dst->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        status = tivxBamControlNode(*g_handle, prms->bam_node_num,
                           VXLIB_HISTOGRAM_I8U_O32U_CMD_SET_DIST_PTR,
                           dst_target_ptr);
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelHistogramPostprocessInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, tivx_bam_graph_handle *g_handle, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_distribution_t *dst;

    dst = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_HISTOGRAM_DISTRIBUTION_IDX];

    if (NULL == g_handle)
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *dst_target_ptr;

        dst_target_ptr = tivxMemShared2TargetPtr(&dst->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferUnmap(dst_target_ptr, dst->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
    }

    return status;
}
