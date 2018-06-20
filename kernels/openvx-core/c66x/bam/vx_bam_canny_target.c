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

static vx_status VX_CALLBACK tivxKernelCannyControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static void tivxCannyFreeMem(tivxCannyParams *prms);


static vx_status VX_CALLBACK tivxKernelCannyProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivxCannyParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    uint8_t *src_addr, *dst_addr;
    uint8_t *border_addr_tl, *border_addr_tr, *border_addr_bl;
    vx_rectangle_t rect;
    uint32_t size, num_edge_trace_out = 0;
    uint32_t num_dbl_thr_items = 0;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_CNED_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CNED_IN_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CNED_OUT_IMG_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxCannyParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        void *img_ptrs[2];
        void *src_target_ptr;
        void *dst_target_ptr;

        src_target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_heap_region);
        dst_target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_heap_region);
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
            tivxComputePatchOffset(rect.start_x + (prms->gs / 2), rect.start_y + (prms->gs / 2),
            &dst->imagepatch_addr[0U]));
        border_addr_tr = (uint8_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x + (prms->gs / 2) + 1 + prms->vxlib_dst.dim_x, rect.start_y + (prms->gs / 2),
            &dst->imagepatch_addr[0U]));
        border_addr_bl = (uint8_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x + (prms->gs / 2), rect.start_y + (prms->gs / 2) + 1 + prms->vxlib_dst.dim_y,
            &dst->imagepatch_addr[0U]));

        img_ptrs[0] = src_addr;
        img_ptrs[1] = dst_addr;
        tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

        status  = tivxBamProcessGraph(prms->graph_handle);

        tivxBamControlNode(prms->graph_handle, DBTHRESHOLD_NODE,
                           VXLIB_DOUBLETHRESHOLD_I16U_I8U_CMD_GET_NUM_EDGES,
                           &num_dbl_thr_items);

        /* Edge Tracing requires 1 pixel border of zeros */
        memset(border_addr_tl, 0, prms->vxlib_dst.dim_x+2);
        memset(border_addr_bl, 0, prms->vxlib_dst.dim_x+2);
        for(i=0; i<(prms->vxlib_dst.dim_y+2); i++)
        {
            border_addr_tl[i*prms->vxlib_dst.stride_y] = 0;
            border_addr_tr[i*prms->vxlib_dst.stride_y] = 0;
        }

        if (VXLIB_SUCCESS == status)
        {
            status = VXLIB_edgeTracing_i8u(dst_addr, &prms->vxlib_dst,
                prms->edge_list, prms->edge_list_size, num_dbl_thr_items,
                &num_edge_trace_out);
        }
        if (VXLIB_SUCCESS == status)
        {
            status = VXLIB_thresholdBinary_i8u_o8u(dst_addr,
                &prms->vxlib_dst, dst_addr, &prms->vxlib_dst, 128, 255, 0);
        }

        if (status != VXLIB_SUCCESS)
        {
            status = VX_FAILURE;
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelCannyCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_threshold_t *thr;
    tivxCannyParams *prms = NULL;
    tivx_obj_desc_scalar_t *sc_gs, *sc_norm;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_CNED_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CNED_IN_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CNED_OUT_IMG_IDX];
        thr = (tivx_obj_desc_threshold_t *)obj_desc[
            TIVX_KERNEL_CNED_IN_THR_IDX];
        sc_gs = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_CNED_IN_SC_GS_IDX];
        sc_norm = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_CNED_IN_SC_NORM_IDX];

        prms = tivxMemAlloc(sizeof(tivxCannyParams), TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            tivx_bam_kernel_details_t kernel_details[6];
            BAM_VXLIB_doubleThreshold_i16u_i8u_params dbThreshold_kernel_params;
            VXLIB_bufParams2D_t vxlib_src;
            VXLIB_bufParams2D_t *buf_params[2];

            memset(prms, 0, sizeof(tivxCannyParams));

            BAM_NodeParams node_list[] = { \
                {SOURCE_NODE, BAM_KERNELID_DMAREAD_AUTOINCREMENT, NULL}, \
                {SOBEL_NODE, BAM_KERNELID_VXLIB_SOBEL_7X7_I8U_O16S_O16S, NULL}, \
                {NORM_NODE, BAM_KERNELID_VXLIB_NORML2_I16S_I16S_O16U, NULL}, \
                {NMS_NODE, BAM_KERNELID_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U, NULL}, \
                {DBTHRESHOLD_NODE, BAM_KERNELID_VXLIB_DOUBLETHRESHOLD_I16S_I8U, NULL}, \
                {SINK_NODE, BAM_KERNELID_DMAWRITE_AUTOINCREMENT, NULL}, \
                {BAM_END_NODE_MARKER,   0,                          NULL},\
            };

            prms->gs = sc_gs->data.s32;

            /* Update the Sobel type accordingly */
            if(3 == prms->gs)
            {
                node_list[SOBEL_NODE].kernelId = BAM_KERNELID_VXLIB_SOBEL_3X3_I8U_O16S_O16S;
                BAM_VXLIB_sobel_3x3_i8u_o16s_o16s_getKernelInfo( NULL,
                                                                 &kernel_details[SOBEL_NODE].kernel_info);
            }
            else if(5 == prms->gs)
            {
                node_list[SOBEL_NODE].kernelId = BAM_KERNELID_VXLIB_SOBEL_5X5_I8U_O16S_O16S;
                BAM_VXLIB_sobel_5x5_i8u_o16s_o16s_getKernelInfo( NULL,
                                                                 &kernel_details[SOBEL_NODE].kernel_info);
            }
            else
            {
                BAM_VXLIB_sobel_7x7_i8u_o16s_o16s_getKernelInfo( NULL,
                                                                 &kernel_details[SOBEL_NODE].kernel_info);
            }

            /* Update the Norm type accordingly */
            if(VX_NORM_L1 == sc_norm->data.enm)
            {
                node_list[NORM_NODE].kernelId = BAM_KERNELID_VXLIB_NORML1_I16S_I16S_O16U;
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
                    {SOBEL_NODE, BAM_VXLIB_SOBEL_7X7_I8U_O16S_O16S_INPUT_IMAGE_PORT}},\

                {{SOBEL_NODE, BAM_VXLIB_SOBEL_7X7_I8U_O16S_O16S_OUTPUT_X_PORT},
                    {NORM_NODE, BAM_VXLIB_NORML2_I16S_I16S_O16U_INPUT_X_PORT}},\

                {{SOBEL_NODE, BAM_VXLIB_SOBEL_7X7_I8U_O16S_O16S_OUTPUT_X_PORT},
                    {NMS_NODE, BAM_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U_INPUT_X_PORT}},\

                {{SOBEL_NODE, BAM_VXLIB_SOBEL_7X7_I8U_O16S_O16S_OUTPUT_Y_PORT},
                    {NORM_NODE, BAM_VXLIB_NORML2_I16S_I16S_O16U_INPUT_Y_PORT}},\

                {{SOBEL_NODE, BAM_VXLIB_SOBEL_7X7_I8U_O16S_O16S_OUTPUT_Y_PORT},
                    {NMS_NODE, BAM_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U_INPUT_Y_PORT}},\

                {{NORM_NODE, BAM_VXLIB_NORML2_I16S_I16S_O16U_OUTPUT_PORT},
                    {NMS_NODE, BAM_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U_INPUT_MAG_PORT}},\

                {{NORM_NODE, BAM_VXLIB_NORML2_I16S_I16S_O16U_OUTPUT_PORT},
                    {DBTHRESHOLD_NODE, BAM_VXLIB_DOUBLETHRESHOLD_I16S_I8U_INPUT_MAG_PORT}},\

                {{NMS_NODE, BAM_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U_OUTPUT_PORT},
                    {DBTHRESHOLD_NODE, BAM_VXLIB_DOUBLETHRESHOLD_I16S_I8U_INPUT_EDGEMAP_PORT}},\

                {{NMS_NODE, BAM_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U_OUTPUT_PORT},
                    {SINK_NODE, 0}},\

                {{BAM_END_NODE_MARKER, 0},
                    {BAM_END_NODE_MARKER, 0}},\
            };

            tivxInitBufParams(src, &vxlib_src);
            tivxInitBufParams(dst, &prms->vxlib_dst);

            prms->edge_list_size = prms->vxlib_dst.dim_x * prms->vxlib_dst.dim_y;

            prms->edge_list = tivxMemAlloc(prms->edge_list_size * 4u,
                TIVX_MEM_EXTERNAL);
            if (NULL == prms->edge_list)
            {
                status = VX_ERROR_NO_MEMORY;
            }

            if (VX_SUCCESS == status)
            {
                /* Fill in the frame level sizes of buffers here. If the port
                 * is optionally disabled, put NULL */
                buf_params[0] = &vxlib_src;
                buf_params[1] = &prms->vxlib_dst;

                dbThreshold_kernel_params.edgeMapLineOffset   = prms->vxlib_dst.stride_y;
                dbThreshold_kernel_params.edgeList            = prms->edge_list;
                dbThreshold_kernel_params.edgeListCapacity    = prms->edge_list_size;
                dbThreshold_kernel_params.loThreshold         = thr->lower;
                dbThreshold_kernel_params.hiThreshold         = thr->upper;

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
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
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
    vx_status status = VX_SUCCESS;
    uint32_t size;
    tivxCannyParams *prms = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_CNED_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxCannyParams) == size))
        {
            tivxBamDestroyHandle(prms->graph_handle);
            tivxCannyFreeMem(prms);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelCannyControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static void tivxCannyFreeMem(tivxCannyParams *prms)
{
    if (NULL != prms)
    {
        if (NULL != prms->edge_list)
        {
            tivxMemFree(prms->edge_list, prms->edge_list_size * 4u,
                TIVX_MEM_EXTERNAL);
            prms->edge_list = NULL;
        }

        tivxMemFree(prms, sizeof(tivxCannyParams), TIVX_MEM_EXTERNAL);
    }
}

void tivxAddTargetKernelBamCannyEd(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_DSP1) || (self_cpu == TIVX_CPU_ID_DSP2))
    {
        if (self_cpu == TIVX_CPU_ID_DSP1)
        {
            strncpy(target_name, TIVX_TARGET_DSP1,
                TIVX_TARGET_MAX_NAME);
        }
        else
        {
            strncpy(target_name, TIVX_TARGET_DSP2,
                TIVX_TARGET_MAX_NAME);
        }

        vx_canny_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_CANNY_EDGE_DETECTOR,
            target_name,
            tivxKernelCannyProcess,
            tivxKernelCannyCreate,
            tivxKernelCannyDelete,
            tivxKernelCannyControl,
            NULL);
    }
}

void tivxRemoveTargetKernelBamCannyEd(void)
{
    tivxRemoveTargetKernel(vx_canny_target_kernel);
}
