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
#include <tivx_openvx_core_kernels.h>
#include <tivx_target_kernels_priv.h>
#include <tivx_kernel_halfscale_gaussian.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_bam_kernel_wrapper.h>

/* In the case of gaussian size == 3, use 2 separate graphs, one for
 * gaussian 3x3, and another for half scale.  A single graph for these
 * two is not yet supported. */
#define TWO_BAM_GRAPHS   1

#define SOURCE_NODE      0
#define GAUSSIAN_NODE    1
#define SCALE_NODE       2
#define SINK_NODE        3

typedef struct
{
    tivx_bam_graph_handle graph_handle;
#if TWO_BAM_GRAPHS
    tivx_bam_graph_handle graph_handleScale;
    uint8_t *scratch;
    uint32_t scratchSize;
#endif
} tivxHalfScaleGaussianParams;

static tivx_target_kernel vx_halfscale_gaussian_target_kernel = NULL;

static vx_status VX_CALLBACK tivxBamKernelHalfScaleGaussianProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelHalfScaleGaussianCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelHalfScaleGaussianDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelHalfScaleGaussianControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelHalfScaleGaussianProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxHalfScaleGaussianParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_scalar_t *gsize;
    vx_uint8 *src_addr, *dst_addr;
    vx_rectangle_t rect;
    uint32_t size, gsize_value = 1;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_SRC_IDX];
        gsize = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_GSIZE_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_DST_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxHalfScaleGaussianParams) != size))
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
            src->mem_ptr[0U].shared_ptr, src->mem_ptr[0U].mem_heap_region);
        dst_target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0U].shared_ptr, dst->mem_ptr[0U].mem_heap_region);

        /* Get the correct offset of the images from the valid roi parameter,
           Assuming valid Roi is same for src0 and src1 images */
        rect = src->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)src_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));

        dst_addr = (uint8_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[0]));

        if( gsize != NULL)
        {
            gsize_value = gsize->data.s32;
        }

        if (gsize_value == 1)
        {
            img_ptrs[1] = dst_addr;
        }
        else if (gsize_value == 3)
        {
#if TWO_BAM_GRAPHS
            img_ptrs[1] = (vx_uint8*)(prms->scratch + src->imagepatch_addr[0].stride_y + 1);
#else
            img_ptrs[1] = dst_addr;
#endif

        }
        else if (gsize_value == 5)
        {
            dst_addr = (vx_uint8*)(dst_addr + dst->imagepatch_addr[0].stride_y + 1);
            img_ptrs[1] = dst_addr;
        }
        else
        {
        }

        img_ptrs[0] = src_addr;
        tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

        status  = tivxBamProcessGraph(prms->graph_handle);

#if TWO_BAM_GRAPHS
        if(gsize_value == 3)
        {
            img_ptrs[0] = prms->scratch;
            img_ptrs[1] = dst_addr;
            tivxBamUpdatePointers(prms->graph_handleScale, 1U, 1U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handleScale);
        }
#endif
    }

    return (status);
}

static vx_status VX_CALLBACK tivxBamKernelHalfScaleGaussianCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_scalar_t *gsize;
    tivxHalfScaleGaussianParams *prms = NULL;
    int32_t gsize_value = 1;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_HALFSCALE_GAUSSIAN_SRC_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_HALFSCALE_GAUSSIAN_DST_IDX];
        gsize = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HALFSCALE_GAUSSIAN_GSIZE_IDX];

        prms = tivxMemAlloc(sizeof(tivxHalfScaleGaussianParams), TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            tivx_bam_kernel_details_t kernel_details;
            VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
            VXLIB_bufParams2D_t *buf_params[2];

            memset(prms, 0, sizeof(tivxHalfScaleGaussianParams));

            vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
            vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
            vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;
            vxlib_src.data_type = VXLIB_UINT8;

            vxlib_dst.dim_x = dst->imagepatch_addr[0].dim_x;
            vxlib_dst.dim_y = dst->imagepatch_addr[0].dim_y;
            vxlib_dst.stride_y = dst->imagepatch_addr[0].stride_y;
            vxlib_dst.data_type = VXLIB_UINT8;

            if( gsize != NULL)
            {
                gsize_value = gsize->data.s32;
            }

            /* Fill in the frame level sizes of buffers here. If the port
             * is optionally disabled, put NULL */
            buf_params[0] = &vxlib_src;
            buf_params[1] = &vxlib_dst;

            if (1 == gsize_value)
            {
                BAM_VXLIB_scaleImageNearest_i8u_o8u_params kernel_params;

                kernel_params.xScale = 2;
                kernel_params.yScale = 2;
                kernel_params.srcOffsetX = 0;
                kernel_params.srcOffsetY = 0;
                kernel_params.dstOffsetX = 0;
                kernel_params.dstOffsetY = 0;

                kernel_details.compute_kernel_params = (void*)&kernel_params;

                BAM_VXLIB_scaleImageNearest_i8u_o8u_getKernelInfo(
                    &kernel_params, &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_SCALEIMAGENEAREST_I8U_O8U,
                    buf_params, &kernel_details,
                    &prms->graph_handle);
            }
            else if (3 == gsize_value)
            {
#if TWO_BAM_GRAPHS
                VXLIB_bufParams2D_t vxlib_int;
                BAM_VXLIB_scaleImageNearest_i8u_o8u_params kernel_params;

                prms->scratchSize = vxlib_src.dim_y*vxlib_src.stride_y;
                prms->scratch = tivxMemAlloc(prms->scratchSize, TIVX_MEM_EXTERNAL);

                kernel_params.xScale = 2;
                kernel_params.yScale = 2;
                kernel_params.srcOffsetX = 0;
                kernel_params.srcOffsetY = 0;
                kernel_params.dstOffsetX = 0;
                kernel_params.dstOffsetY = 0;

                kernel_details.compute_kernel_params = (void*)&kernel_params;

                vxlib_int.dim_x = src->imagepatch_addr[0].dim_x-2;
                vxlib_int.dim_y = src->imagepatch_addr[0].dim_y-2;
                vxlib_int.stride_y = src->imagepatch_addr[0].stride_y;
                vxlib_int.data_type = VXLIB_UINT8;

                buf_params[1] = &vxlib_int;

                BAM_VXLIB_gaussian_3x3_i8u_o8u_getKernelInfo(
                    NULL, &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_GAUSSIAN_3X3_I8U_O8U,
                    buf_params, &kernel_details,
                    &prms->graph_handle);

                vxlib_int.dim_x = src->imagepatch_addr[0].dim_x;
                vxlib_int.dim_y = src->imagepatch_addr[0].dim_y;

                buf_params[0] = &vxlib_int;
                buf_params[1] = &vxlib_dst;

                BAM_VXLIB_scaleImageNearest_i8u_o8u_getKernelInfo(
                    &kernel_params, &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_SCALEIMAGENEAREST_I8U_O8U,
                    buf_params, &kernel_details,
                    &prms->graph_handleScale);

#else
                tivx_bam_kernel_details_t kernel_details_g3[4];
                BAM_VXLIB_scaleImageNearest_i8u_o8u_params kernel_params;

                BAM_NodeParams node_list[] = { \
                    {SOURCE_NODE, BAM_KERNELID_DMAREAD_AUTOINCREMENT, NULL}, \
                    {GAUSSIAN_NODE, BAM_KERNELID_VXLIB_GAUSSIAN_3X3_I8U_O8U, NULL}, \
                    {SCALE_NODE, BAM_KERNELID_VXLIB_SCALEIMAGENEARES_I8U_O8U, NULL}, \
                    {SINK_NODE, BAM_KERNELID_DMAWRITE_AUTOINCREMENT, NULL}, \
                    {BAM_END_NODE_MARKER,   0,                          NULL},\
                };

                BAM_EdgeParams edge_list[]= {\
                    {{SOURCE_NODE, 0},
                        {GAUSSIAN_NODE, BAM_VXLIB_GAUSSIAN_3X3_I8U_O8U_INPUT_IMAGE_PORT}},\

                    {{GAUSSIAN_NODE, BAM_VXLIB_GAUSSIAN_3X3_I8U_O8U_OUTPUT_IMAGE_PORT},
                        {SCALE_NODE, BAM_VXLIB_SCALEIMAGENEAREST_I8U_O8U_INPUT_IMAGE_PORT}},\

                    {{SCALE_NODE, BAM_VXLIB_SCALEIMAGENEAREST_I8U_O8U_OUTPUT_IMAGE_PORT},
                        {SINK_NODE, 0}},\

                    {{BAM_END_NODE_MARKER, 0},
                        {BAM_END_NODE_MARKER, 0}},\
                };

                vxlib_dst.dim_x -= 2;
                vxlib_dst.dim_y -= 2;

                kernel_params.xScale = 2;
                kernel_params.yScale = 2;
                kernel_params.srcOffsetX = 1;
                kernel_params.srcOffsetY = 1;
                kernel_params.dstOffsetX = 1;
                kernel_params.dstOffsetY = 1;

                BAM_VXLIB_gaussian_3x3_i8u_o8u_getKernelInfo(
                    NULL, &kernel_details_g3[GAUSSIAN_NODE].kernel_info);

                BAM_VXLIB_scaleImageNearest_i8u_o8u_getKernelInfo(
                    &kernel_params, &kernel_details_g3[SCALE_NODE].kernel_info);

                kernel_details_g3[SOURCE_NODE].compute_kernel_params = NULL;
                kernel_details_g3[GAUSSIAN_NODE].compute_kernel_params = NULL;
                kernel_details_g3[SCALE_NODE].compute_kernel_params = (void*)&kernel_params;
                kernel_details_g3[SINK_NODE].compute_kernel_params = NULL;

                status = tivxBamCreateHandleMultiNode(node_list, edge_list,
                                                      buf_params, kernel_details_g3,
                                                      &prms->graph_handle);
#endif
            }
            else if (5 == gsize_value)
            {
                vxlib_dst.dim_x -= 2;
                vxlib_dst.dim_y -= 2;

                kernel_details.compute_kernel_params = NULL;

                BAM_VXLIB_halfScaleGaussian_5x5_i8u_o8u_getKernelInfo(
                    NULL, &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_HALFSCALEGAUSSIAN_5x5_I8U_O8U,
                    buf_params, &kernel_details,
                    &prms->graph_handle);
            }
            else
            {
                status = VX_FAILURE;
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxHalfScaleGaussianParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxHalfScaleGaussianParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxBamKernelHalfScaleGaussianDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t size;
    tivxHalfScaleGaussianParams *prms = NULL;
    tivx_obj_desc_scalar_t *gsize;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxHalfScaleGaussianParams) == size))
        {
            tivxBamDestroyHandle(prms->graph_handle);

#if TWO_BAM_GRAPHS
            gsize = (tivx_obj_desc_scalar_t *)obj_desc[
                TIVX_KERNEL_HALFSCALE_GAUSSIAN_GSIZE_IDX];

            if( gsize != NULL)
            {
                if(gsize->data.s32 == 3)
                {
                    tivxBamDestroyHandle(prms->graph_handleScale);
                    tivxMemFree(prms->scratch, prms->scratchSize, TIVX_MEM_EXTERNAL);
                }
            }
#endif
            tivxMemFree(prms, sizeof(tivxHalfScaleGaussianParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxBamKernelHalfScaleGaussianControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBamHalfscaleGaussian(void)
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

        vx_halfscale_gaussian_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_HALFSCALE_GAUSSIAN,
            target_name,
            tivxBamKernelHalfScaleGaussianProcess,
            tivxBamKernelHalfScaleGaussianCreate,
            tivxBamKernelHalfScaleGaussianDelete,
            tivxBamKernelHalfScaleGaussianControl,
            NULL);
    }
}


void tivxRemoveTargetKernelBamHalfscaleGaussian(void)
{
    tivxRemoveTargetKernel(vx_halfscale_gaussian_target_kernel);
}
