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
#include <tivx_kernel_canny.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_bam_kernel_wrapper.h>

typedef struct
{
    tivx_bam_graph_handle graph_handle;

    uint32_t *edge_list;
    uint32_t edge_list_size;
    uint32_t gs;

    VXLIB_bufParams2D_t vxlib_dst;
    uint8_t bam_node_num;
} tivxCannyParams;

#define SOURCE_NODE      0
#define SOBEL_NODE       1
#define NORM_NODE        2
#define NMS_NODE         3
#define DBTHRESHOLD_NODE 4
#define SINK_NODE        5

static tivx_target_kernel vx_canny_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelCannyProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelCannyCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelCannyDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

/* Supernode Callbacks */
static vx_status VX_CALLBACK tivxKernelCannyCreateInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    int32_t * bam_node_cnt, void * scratch, int32_t *size);

static vx_status VX_CALLBACK tivxKernelCannyGetNodePort(
    tivx_target_kernel_instance kernel, uint8_t ovx_port, uint8_t plane,
    uint8_t *bam_node, uint8_t *bam_port);

static vx_status VX_CALLBACK tivxKernelCannyAppendInternalEdges(
    tivx_target_kernel_instance kernel, BAM_EdgeParams edge_list[], int32_t *bam_edge_cnt);

static vx_status VX_CALLBACK tivxKernelCannyPostprocessInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, tivx_bam_graph_handle *g_handle, void *priv_arg);

static void tivxCannyFreeMem(tivxCannyParams *prms);


static vx_status VX_CALLBACK tivxKernelCannyProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    tivxCannyParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    uint8_t *src_addr, *dst_addr;
    uint8_t *border_addr_tl, *border_addr_tr, *border_addr_bl;
    vx_rectangle_t rect;
    uint32_t size, num_edge_trace_out = 0;
    uint32_t num_dbl_thr_items = 0;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_CANNY_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CANNY_INPUT_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CANNY_OUTPUT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxCannyParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *img_ptrs[2];
        void *src_target_ptr;
        void *dst_target_ptr;

        src_target_ptr = tivxMemShared2TargetPtr(&src->mem_ptr[0]);
        dst_target_ptr = tivxMemShared2TargetPtr(&dst->mem_ptr[0]);
        /* Get the correct offset of the images from the valid roi parameter */
        rect = src->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)src_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));

        rect = dst->valid_roi;

        dst_addr = (uint8_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[0U]));

        /* Get the correct offset of the images from the valid roi parameter */
        rect = src->valid_roi;

        border_addr_tl = (uint8_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x + (prms->gs / 2U), rect.start_y + (prms->gs / 2U),
            &dst->imagepatch_addr[0U]));
        border_addr_tr = (uint8_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x + (prms->gs / 2U) + 1U + prms->vxlib_dst.dim_x, rect.start_y + (prms->gs / 2U),
            &dst->imagepatch_addr[0U]));
        border_addr_bl = (uint8_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x + (prms->gs / 2U), rect.start_y + (prms->gs / 2U) + 1U + prms->vxlib_dst.dim_y,
            &dst->imagepatch_addr[0U]));

        img_ptrs[0] = src_addr;
        img_ptrs[1] = dst_addr;
        tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

        status  = tivxBamProcessGraph(prms->graph_handle);

        tivxBamControlNode(prms->graph_handle, DBTHRESHOLD_NODE,
                           VXLIB_DOUBLETHRESHOLD_I16U_I8U_CMD_GET_NUM_EDGES,
                           &num_dbl_thr_items);

        /* Edge Tracing requires 1 pixel border of zeros */
        memset(border_addr_tl, 0, prms->vxlib_dst.dim_x+2U);
        memset(border_addr_bl, 0, prms->vxlib_dst.dim_x+2U);
        for(i=0; i<(prms->vxlib_dst.dim_y+2U); i++)
        {
            border_addr_tl[(int32_t)i*prms->vxlib_dst.stride_y] = 0U;
            border_addr_tr[(int32_t)i*prms->vxlib_dst.stride_y] = 0U;
        }

        if ((vx_status)VXLIB_SUCCESS == status)
        {
            status = (vx_status)VXLIB_edgeTracing_i8u(dst_addr, &prms->vxlib_dst,
                prms->edge_list, prms->edge_list_size, num_dbl_thr_items,
                &num_edge_trace_out);
        }
        if ((vx_status)VXLIB_SUCCESS == status)
        {
            status = (vx_status)VXLIB_thresholdBinary_i8u_o8u(dst_addr,
                &prms->vxlib_dst, dst_addr, &prms->vxlib_dst, 128, 255, 0);
        }

        if (status != (vx_status)VXLIB_SUCCESS)
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelCannyCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_threshold_t *thr;
    tivxCannyParams *prms = NULL;
    tivx_obj_desc_scalar_t *sc_gs, *sc_norm;
    tivx_bam_kernel_details_t kernel_details[6];

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_CANNY_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxMemResetScratchHeap((vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxBamInitKernelDetails(&kernel_details[0], 6, kernel);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CANNY_INPUT_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CANNY_OUTPUT_IDX];
        thr = (tivx_obj_desc_threshold_t *)obj_desc[
            TIVX_KERNEL_CANNY_HYST_IDX];
        sc_gs = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_CANNY_GRADIENT_SIZE_IDX];
        sc_norm = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_CANNY_NORM_TYPE_IDX];

        prms = tivxMemAlloc(sizeof(tivxCannyParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            BAM_VXLIB_doubleThreshold_i16u_i8u_params dbThreshold_kernel_params;
            VXLIB_bufParams2D_t vxlib_src;
            VXLIB_bufParams2D_t *buf_params[2];

            memset(prms, 0, sizeof(tivxCannyParams));

            BAM_NodeParams node_list[] = { \
                {SOURCE_NODE, (uint32_t)BAM_KERNELID_DMAREAD_AUTOINCREMENT, NULL}, \
                {SOBEL_NODE, (uint32_t)BAM_KERNELID_VXLIB_SOBEL_7X7_I8U_O16S_O16S, NULL}, \
                {NORM_NODE, (uint32_t)BAM_KERNELID_VXLIB_NORML2_I16S_I16S_O16U, NULL}, \
                {NMS_NODE, (uint32_t)BAM_KERNELID_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U, NULL}, \
                {DBTHRESHOLD_NODE, (uint32_t)BAM_KERNELID_VXLIB_DOUBLETHRESHOLD_I16S_I8U, NULL}, \
                {SINK_NODE, (uint32_t)BAM_KERNELID_DMAWRITE_AUTOINCREMENT, NULL}, \
                {BAM_END_NODE_MARKER,   0,                          NULL},\
            };

            prms->gs = (uint32_t)sc_gs->data.s32;

            /* Update the Sobel type accordingly */
            if(3U == prms->gs)
            {
                node_list[SOBEL_NODE].kernelId = (uint32_t)BAM_KERNELID_VXLIB_SOBEL_3X3_I8U_O16S_O16S;
                BAM_VXLIB_sobel_3x3_i8u_o16s_o16s_getKernelInfo( NULL,
                                                                 &kernel_details[SOBEL_NODE].kernel_info);
            }
            else if(5U == prms->gs)
            {
                node_list[SOBEL_NODE].kernelId = (uint32_t)BAM_KERNELID_VXLIB_SOBEL_5X5_I8U_O16S_O16S;
                BAM_VXLIB_sobel_5x5_i8u_o16s_o16s_getKernelInfo( NULL,
                                                                 &kernel_details[SOBEL_NODE].kernel_info);
            }
            else
            {
                BAM_VXLIB_sobel_7x7_i8u_o16s_o16s_getKernelInfo( NULL,
                                                                 &kernel_details[SOBEL_NODE].kernel_info);
            }

            /* Update the Norm type accordingly */
            if((vx_enum)VX_NORM_L1 == sc_norm->data.enm)
            {
                node_list[NORM_NODE].kernelId = (uint32_t)BAM_KERNELID_VXLIB_NORML1_I16S_I16S_O16U;
                BAM_VXLIB_normL1_i16s_i16s_o16u_getKernelInfo( NULL,
                                                                 &kernel_details[NORM_NODE].kernel_info);
            }
            else
            {
                BAM_VXLIB_normL2_i16s_i16s_o16u_getKernelInfo( NULL,
                                                                 &kernel_details[NORM_NODE].kernel_info);
            }

            BAM_EdgeParams edge_list[]= {\
                {{SOURCE_NODE, 0},
                    {SOBEL_NODE, (uint8_t)BAM_VXLIB_SOBEL_7X7_I8U_O16S_O16S_INPUT_IMAGE_PORT}},\

                {{SOBEL_NODE, (uint8_t)BAM_VXLIB_SOBEL_7X7_I8U_O16S_O16S_OUTPUT_X_PORT},
                    {NORM_NODE, (uint8_t)BAM_VXLIB_NORML2_I16S_I16S_O16U_INPUT_X_PORT}},\

                {{SOBEL_NODE, (uint8_t)BAM_VXLIB_SOBEL_7X7_I8U_O16S_O16S_OUTPUT_X_PORT},
                    {NMS_NODE, (uint8_t)BAM_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U_INPUT_X_PORT}},\

                {{SOBEL_NODE, (uint8_t)BAM_VXLIB_SOBEL_7X7_I8U_O16S_O16S_OUTPUT_Y_PORT},
                    {NORM_NODE, (uint8_t)BAM_VXLIB_NORML2_I16S_I16S_O16U_INPUT_Y_PORT}},\

                {{SOBEL_NODE, (uint8_t)BAM_VXLIB_SOBEL_7X7_I8U_O16S_O16S_OUTPUT_Y_PORT},
                    {NMS_NODE, (uint8_t)BAM_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U_INPUT_Y_PORT}},\

                {{NORM_NODE, (uint8_t)BAM_VXLIB_NORML2_I16S_I16S_O16U_OUTPUT_PORT},
                    {NMS_NODE, (uint8_t)BAM_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U_INPUT_MAG_PORT}},\

                {{NORM_NODE, (uint8_t)BAM_VXLIB_NORML2_I16S_I16S_O16U_OUTPUT_PORT},
                    {DBTHRESHOLD_NODE, (uint8_t)BAM_VXLIB_DOUBLETHRESHOLD_I16S_I8U_INPUT_MAG_PORT}},\

                {{NMS_NODE, (uint8_t)BAM_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U_OUTPUT_PORT},
                    {DBTHRESHOLD_NODE, (uint8_t)BAM_VXLIB_DOUBLETHRESHOLD_I16S_I8U_INPUT_EDGEMAP_PORT}},\

                {{NMS_NODE, (uint8_t)BAM_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U_OUTPUT_PORT},
                    {SINK_NODE, 0}},\

                {{BAM_END_NODE_MARKER, 0},
                    {BAM_END_NODE_MARKER, 0}},\
            };

            tivxInitBufParams(src, &vxlib_src);
            tivxInitBufParams(dst, &prms->vxlib_dst);

            prms->edge_list_size = prms->vxlib_dst.dim_x * prms->vxlib_dst.dim_y;

            prms->edge_list = tivxMemAlloc(prms->edge_list_size * 4u,
                (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
            if (NULL == prms->edge_list)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                /* Fill in the frame level sizes of buffers here. If the port
                 * is optionally disabled, put NULL */
                buf_params[0] = &vxlib_src;
                buf_params[1] = &prms->vxlib_dst;

                dbThreshold_kernel_params.edgeMapLineOffset   = (uint16_t)prms->vxlib_dst.stride_y;
                dbThreshold_kernel_params.edgeList            = prms->edge_list;
                dbThreshold_kernel_params.edgeListCapacity    = prms->edge_list_size;
                dbThreshold_kernel_params.loThreshold         = (uint32_t)thr->lower;
                dbThreshold_kernel_params.hiThreshold         = (uint32_t)thr->upper;

                BAM_VXLIB_cannyNMS_i16s_i16s_i16u_o8u_getKernelInfo( NULL,
                                                                     &kernel_details[NMS_NODE].kernel_info);
                BAM_VXLIB_doubleThreshold_i16u_i8u_getKernelInfo( &dbThreshold_kernel_params,
                                                                  &kernel_details[DBTHRESHOLD_NODE].kernel_info);

                kernel_details[SOURCE_NODE].compute_kernel_params = NULL;
                kernel_details[SOBEL_NODE].compute_kernel_params = NULL;
                kernel_details[NORM_NODE].compute_kernel_params = NULL;
                kernel_details[NMS_NODE].compute_kernel_params = NULL;
                kernel_details[DBTHRESHOLD_NODE].compute_kernel_params = &dbThreshold_kernel_params;
                kernel_details[SINK_NODE].compute_kernel_params = NULL;

                status = tivxBamCreateHandleMultiNode(node_list,
                    sizeof(node_list)/sizeof(BAM_NodeParams),
                    edge_list,
                    sizeof(edge_list)/sizeof(BAM_EdgeParams),
                    buf_params, kernel_details,
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
                sizeof(tivxCannyParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxCannyFreeMem(prms);
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelCannyDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t size;
    tivxCannyParams *prms = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_CANNY_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxCannyParams) == size))
        {
            if(NULL != prms->graph_handle)
            {
                tivxBamDestroyHandle(prms->graph_handle);
            }
            tivxCannyFreeMem(prms);
        }
    }

    return (status);
}

static void tivxCannyFreeMem(tivxCannyParams *prms)
{
    if (NULL != prms)
    {
        if (NULL != prms->edge_list)
        {
            tivxMemFree(prms->edge_list, prms->edge_list_size * 4u,
                (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
            prms->edge_list = NULL;
        }

        tivxMemFree(prms, sizeof(tivxCannyParams), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}

void tivxAddTargetKernelBamCannyEd(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_canny_target_kernel = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_CANNY_EDGE_DETECTOR,
            target_name,
            tivxKernelCannyProcess,
            tivxKernelCannyCreate,
            tivxKernelCannyDelete,
            NULL,
            NULL);

        tivxEnableKernelForSuperNode(vx_canny_target_kernel,
            tivxKernelCannyCreateInBamGraph,
            tivxKernelCannyGetNodePort,
            tivxKernelCannyAppendInternalEdges,
            NULL,
            tivxKernelCannyPostprocessInBamGraph,
            (int32_t)sizeof(BAM_VXLIB_doubleThreshold_i16u_i8u_params),
            NULL);
    }
}

void tivxRemoveTargetKernelBamCannyEd(void)
{
    tivxRemoveTargetKernel(vx_canny_target_kernel);
}

static vx_status VX_CALLBACK tivxKernelCannyCreateInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    int32_t * bam_node_cnt, void * scratch, int32_t *size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *dst;
    tivx_obj_desc_threshold_t *thr;
    tivxCannyParams *prms = NULL;
    tivx_obj_desc_scalar_t *sc_gs, *sc_norm;

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_CANNY_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CANNY_OUTPUT_IDX];
        thr = (tivx_obj_desc_threshold_t *)obj_desc[
            TIVX_KERNEL_CANNY_HYST_IDX];
        sc_gs = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_CANNY_GRADIENT_SIZE_IDX];
        sc_norm = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_CANNY_NORM_TYPE_IDX];

        prms = tivxMemAlloc(sizeof(tivxCannyParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxCannyParams));

            tivxInitBufParams(dst, &prms->vxlib_dst);
            prms->gs = (uint32_t)sc_gs->data.s32;

            node_list[*bam_node_cnt].nodeIndex = (uint8_t)*bam_node_cnt;
            node_list[*bam_node_cnt].kernelArgs = NULL;

            if(3U == prms->gs)
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_SOBEL_3X3_I8U_O16S_O16S;
                BAM_VXLIB_sobel_3x3_i8u_o16s_o16s_getKernelInfo( NULL,
                                                                 &kernel_details[*bam_node_cnt].kernel_info);
            }
            else if(5U == prms->gs)
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_SOBEL_5X5_I8U_O16S_O16S;
                BAM_VXLIB_sobel_5x5_i8u_o16s_o16s_getKernelInfo( NULL,
                                                                 &kernel_details[*bam_node_cnt].kernel_info);
            }
            else
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_SOBEL_7X7_I8U_O16S_O16S;
                BAM_VXLIB_sobel_7x7_i8u_o16s_o16s_getKernelInfo( NULL,
                                                                 &kernel_details[*bam_node_cnt].kernel_info);
            }
            kernel_details[*bam_node_cnt].compute_kernel_params = NULL;
            *bam_node_cnt = *bam_node_cnt + 1;

            node_list[*bam_node_cnt].nodeIndex = (uint8_t)*bam_node_cnt;
            node_list[*bam_node_cnt].kernelArgs = NULL;

            if((vx_enum)VX_NORM_L1 == sc_norm->data.enm)
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_NORML1_I16S_I16S_O16U;
                BAM_VXLIB_normL1_i16s_i16s_o16u_getKernelInfo( NULL,
                                                                 &kernel_details[*bam_node_cnt].kernel_info);
            }
            else
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_NORML2_I16S_I16S_O16U;
                BAM_VXLIB_normL2_i16s_i16s_o16u_getKernelInfo( NULL,
                                                                 &kernel_details[*bam_node_cnt].kernel_info);
            }
            kernel_details[*bam_node_cnt].compute_kernel_params = NULL;
            *bam_node_cnt = *bam_node_cnt + 1;

            node_list[*bam_node_cnt].nodeIndex = (uint8_t)*bam_node_cnt;
            node_list[*bam_node_cnt].kernelArgs = NULL;
            node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U;
            BAM_VXLIB_cannyNMS_i16s_i16s_i16u_o8u_getKernelInfo( NULL,
                                                                 &kernel_details[*bam_node_cnt].kernel_info);
            kernel_details[*bam_node_cnt].compute_kernel_params = NULL;
            *bam_node_cnt = *bam_node_cnt + 1;

            prms->edge_list_size = prms->vxlib_dst.dim_x * prms->vxlib_dst.dim_y;
            prms->edge_list = tivxMemAlloc(prms->edge_list_size * 4u, (vx_enum)TIVX_MEM_EXTERNAL);

            if (NULL == prms->edge_list)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                BAM_VXLIB_doubleThreshold_i16u_i8u_params *kernel_params = (BAM_VXLIB_doubleThreshold_i16u_i8u_params*)scratch;

                if ((NULL != kernel_params) &&
                    (*size >= (int32_t)sizeof(BAM_VXLIB_doubleThreshold_i16u_i8u_params)))
                {
                    node_list[*bam_node_cnt].nodeIndex = (uint8_t)*bam_node_cnt;
                    node_list[*bam_node_cnt].kernelArgs = NULL;
                    node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_DOUBLETHRESHOLD_I16S_I8U;

                    kernel_params->edgeMapLineOffset   = (uint16_t)prms->vxlib_dst.stride_y;
                    kernel_params->edgeList            = prms->edge_list;
                    kernel_params->edgeListCapacity    = prms->edge_list_size;
                    kernel_params->loThreshold         = (uint32_t)thr->lower;
                    kernel_params->hiThreshold         = (uint32_t)thr->upper;

                    kernel_details[*bam_node_cnt].compute_kernel_params = kernel_params;

                    BAM_VXLIB_doubleThreshold_i16u_i8u_getKernelInfo(kernel_params,
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
                sizeof(tivxCannyParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxCannyParams), (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelCannyAppendInternalEdges(
    tivx_target_kernel_instance kernel, BAM_EdgeParams edge_list[], int32_t *bam_edge_cnt)
{
    tivxCannyParams *prms = NULL;
    uint32_t size;

    vx_status status = tivxGetTargetKernelInstanceContext(kernel,
                        (void **)&prms, &size);

    if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
        (sizeof(tivxCannyParams) == size))
    {
        edge_list[*bam_edge_cnt].upStreamNode.id = prms->bam_node_num - 3U;
        edge_list[*bam_edge_cnt].upStreamNode.port = (uint8_t)BAM_VXLIB_SOBEL_7X7_I8U_O16S_O16S_OUTPUT_X_PORT;
        edge_list[*bam_edge_cnt].downStreamNode.id = prms->bam_node_num - 2U;
        edge_list[*bam_edge_cnt].downStreamNode.port = (uint8_t)BAM_VXLIB_NORML2_I16S_I16S_O16U_INPUT_X_PORT;
        *bam_edge_cnt = *bam_edge_cnt + 1;

        edge_list[*bam_edge_cnt].upStreamNode.id = prms->bam_node_num - 3U;
        edge_list[*bam_edge_cnt].upStreamNode.port = (uint8_t)BAM_VXLIB_SOBEL_7X7_I8U_O16S_O16S_OUTPUT_X_PORT;
        edge_list[*bam_edge_cnt].downStreamNode.id = prms->bam_node_num - 1U;
        edge_list[*bam_edge_cnt].downStreamNode.port = (uint8_t)BAM_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U_INPUT_X_PORT;
        *bam_edge_cnt = *bam_edge_cnt + 1;

        edge_list[*bam_edge_cnt].upStreamNode.id = prms->bam_node_num - 3U;
        edge_list[*bam_edge_cnt].upStreamNode.port = (uint8_t)BAM_VXLIB_SOBEL_7X7_I8U_O16S_O16S_OUTPUT_Y_PORT;
        edge_list[*bam_edge_cnt].downStreamNode.id = prms->bam_node_num - 2U;
        edge_list[*bam_edge_cnt].downStreamNode.port = (uint8_t)BAM_VXLIB_NORML2_I16S_I16S_O16U_INPUT_Y_PORT;
        *bam_edge_cnt = *bam_edge_cnt + 1;

        edge_list[*bam_edge_cnt].upStreamNode.id = prms->bam_node_num - 3U;
        edge_list[*bam_edge_cnt].upStreamNode.port = (uint8_t)BAM_VXLIB_SOBEL_7X7_I8U_O16S_O16S_OUTPUT_Y_PORT;
        edge_list[*bam_edge_cnt].downStreamNode.id = prms->bam_node_num - 1U;
        edge_list[*bam_edge_cnt].downStreamNode.port = (uint8_t)BAM_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U_INPUT_Y_PORT;
        *bam_edge_cnt = *bam_edge_cnt + 1;

        edge_list[*bam_edge_cnt].upStreamNode.id = prms->bam_node_num - 2U;
        edge_list[*bam_edge_cnt].upStreamNode.port = (uint8_t)BAM_VXLIB_NORML2_I16S_I16S_O16U_OUTPUT_PORT;
        edge_list[*bam_edge_cnt].downStreamNode.id = prms->bam_node_num - 1U;
        edge_list[*bam_edge_cnt].downStreamNode.port = (uint8_t)BAM_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U_INPUT_MAG_PORT;
        *bam_edge_cnt = *bam_edge_cnt + 1;

        edge_list[*bam_edge_cnt].upStreamNode.id = prms->bam_node_num - 2U;
        edge_list[*bam_edge_cnt].upStreamNode.port = (uint8_t)BAM_VXLIB_NORML2_I16S_I16S_O16U_OUTPUT_PORT;
        edge_list[*bam_edge_cnt].downStreamNode.id = prms->bam_node_num;
        edge_list[*bam_edge_cnt].downStreamNode.port = (uint8_t)BAM_VXLIB_DOUBLETHRESHOLD_I16S_I8U_INPUT_MAG_PORT;
        *bam_edge_cnt = *bam_edge_cnt + 1;

        edge_list[*bam_edge_cnt].upStreamNode.id = prms->bam_node_num - 1U;
        edge_list[*bam_edge_cnt].upStreamNode.port = (uint8_t)BAM_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U_OUTPUT_PORT;
        edge_list[*bam_edge_cnt].downStreamNode.id = prms->bam_node_num;
        edge_list[*bam_edge_cnt].downStreamNode.port = (uint8_t)BAM_VXLIB_DOUBLETHRESHOLD_I16S_I8U_INPUT_EDGEMAP_PORT;
        *bam_edge_cnt = *bam_edge_cnt + 1;
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelCannyGetNodePort(
    tivx_target_kernel_instance kernel,
    uint8_t ovx_port, uint8_t plane, uint8_t *bam_node, uint8_t *bam_port)
{
    tivxCannyParams *prms = NULL;
    uint32_t size;

    vx_status status = tivxGetTargetKernelInstanceContext(kernel,
                        (void **)&prms, &size);

    if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
        (sizeof(tivxCannyParams) == size))
    {
        switch (ovx_port)
        {
            case TIVX_KERNEL_CANNY_INPUT_IDX:
                *bam_node = prms->bam_node_num - 3U;
                *bam_port = (uint8_t)BAM_VXLIB_SOBEL_3X3_I8U_O16S_O16S_INPUT_IMAGE_PORT;
                break;
            case TIVX_KERNEL_CANNY_OUTPUT_IDX:
                *bam_node = prms->bam_node_num - 1U;
                *bam_port = (uint8_t)BAM_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U_OUTPUT_PORT;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "non existing index queried by tivxKernelSupernodeCreate.tivxGetNodePort()\n");
                status = (vx_status)VX_FAILURE;
                break;
        }
    }

    return status;
}


static vx_status VX_CALLBACK tivxKernelCannyPostprocessInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, tivx_bam_graph_handle *g_handle, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    tivxCannyParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    uint8_t *dst_addr;
    uint8_t *border_addr_tl, *border_addr_tr, *border_addr_bl;
    vx_rectangle_t rect;
    uint32_t size, num_edge_trace_out = 0;
    uint32_t num_dbl_thr_items = 0;

    src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CANNY_INPUT_IDX];
    dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CANNY_OUTPUT_IDX];

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&prms, &size);

    if (((vx_status)VX_SUCCESS != status) || (NULL == prms) || (NULL == g_handle) ||
        (sizeof(tivxCannyParams) != size))
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *dst_target_ptr;

        dst_target_ptr = tivxMemShared2TargetPtr(&dst->mem_ptr[0]);

        rect = dst->valid_roi;

        dst_addr = (uint8_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[0U]));

        /* Get the correct offset of the images from the valid roi parameter */
        rect = src->valid_roi;

        border_addr_tl = (uint8_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x + (prms->gs / 2U), rect.start_y + (prms->gs / 2U),
            &dst->imagepatch_addr[0U]));
        border_addr_tr = (uint8_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x + (prms->gs / 2U) + 1U + prms->vxlib_dst.dim_x, rect.start_y + (prms->gs / 2U),
            &dst->imagepatch_addr[0U]));
        border_addr_bl = (uint8_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x + (prms->gs / 2U), rect.start_y + (prms->gs / 2U) + 1U + prms->vxlib_dst.dim_y,
            &dst->imagepatch_addr[0U]));

        tivxBamControlNode(*g_handle, prms->bam_node_num,
                           VXLIB_DOUBLETHRESHOLD_I16U_I8U_CMD_GET_NUM_EDGES,
                           &num_dbl_thr_items);

        /* Edge Tracing requires 1 pixel border of zeros */
        memset(border_addr_tl, 0, prms->vxlib_dst.dim_x+2U);
        memset(border_addr_bl, 0, prms->vxlib_dst.dim_x+2U);
        for(i=0; i<(prms->vxlib_dst.dim_y+2U); i++)
        {
            border_addr_tl[(int32_t)i*prms->vxlib_dst.stride_y] = 0U;
            border_addr_tr[(int32_t)i*prms->vxlib_dst.stride_y] = 0U;
        }

        if ((vx_status)VXLIB_SUCCESS == status)
        {
            status = (vx_status)VXLIB_edgeTracing_i8u(dst_addr, &prms->vxlib_dst,
                prms->edge_list, prms->edge_list_size, num_dbl_thr_items,
                &num_edge_trace_out);
        }
        if ((vx_status)VXLIB_SUCCESS == status)
        {
            status = (vx_status)VXLIB_thresholdBinary_i8u_o8u(dst_addr,
                &prms->vxlib_dst, dst_addr, &prms->vxlib_dst, 128, 255, 0);
        }

        if (status != (vx_status)VXLIB_SUCCESS)
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    return status;
}

