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
#include <tivx_kernel_harris_corners.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_bam_kernel_wrapper.h>

typedef struct
{
    tivx_bam_graph_handle graph_handle;

    uint32_t gs;
    uint32_t bs;

    /* Parameters for Harris Corner Score */
    vx_float32 *hcs_score;
    uint32_t hcs_score_size;

    VXLIB_bufParams2D_t vxlib_score;

    /* Parameters for Harris Corner Detect */
    vx_float32 *hcd_sprs;
    uint32_t hcd_sprs_size;

    uint32_t *hcd_corners;
    uint32_t hcd_corners_size;

    vx_float32 *hcd_strength;
    uint32_t hcd_strength_size;

    /* Parameters for NMS */
    uint32_t *nms_corners;
    uint32_t nms_corners_size;

    vx_float32 *nms_strength;
    uint32_t nms_strength_size;

    int8_t *nms_scratch;
    uint32_t nms_scratch_size;

    vx_float32 rad;

} tivxHarrisCornersParams;

#define SOURCE_NODE      0
#define SOBEL_NODE       1
#define SCORE_NODE       2
#define SINK_NODE        3

static tivx_target_kernel vx_harris_corners_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelHarrisCornersProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelHarrisCornersCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelHarrisCornersDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static void tivxHarrisCornersFreeMem(tivxHarrisCornersParams *prms);

static vx_status VX_CALLBACK tivxKernelHarrisCornersProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    tivxHarrisCornersParams *prms = NULL;
    tivx_obj_desc_image_t *src;
    tivx_obj_desc_array_t *arr;
    uint8_t *src_addr;
    uint32_t size, num_corners, num_corners_hcd;
    tivx_obj_desc_scalar_t *sc_thr, *sc_cnt;
    vx_keypoint_t *kp;
    vx_rectangle_t rect;

    if (num_params != TIVX_KERNEL_HARRIS_CORNERS_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HARRIS_CORNERS_MAX_PARAMS; i ++)
        {
            if ((NULL == obj_desc[i]) &&
                (i != TIVX_KERNEL_HARRIS_CORNERS_NUM_CORNERS_IDX))
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HARRIS_CORNERS_INPUT_IDX];
        arr = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_HARRIS_CORNERS_CORNERS_IDX];
        sc_thr = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRIS_CORNERS_STRENGTH_THRESH_IDX];
        sc_cnt = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRIS_CORNERS_NUM_CORNERS_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxHarrisCornersParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *img_ptrs[2];
        void *src_target_ptr;
        void *arr_target_ptr;
        VXLIB_STATUS status_vxlib = (VXLIB_STATUS)VXLIB_SUCCESS;

        src_target_ptr = tivxMemShared2TargetPtr(&src->mem_ptr[0]);
        arr_target_ptr = tivxMemShared2TargetPtr(&arr->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(arr_target_ptr, arr->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        /* Get the correct offset of the images from the valid roi parameter */
        rect = src->valid_roi;

        tivxSetPointerLocation(src, &src_target_ptr, &src_addr);

        img_ptrs[0] = src_addr;
        img_ptrs[1] = prms->hcs_score;

        tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

        status  = tivxBamProcessGraph(prms->graph_handle);

        if ((vx_status)VX_SUCCESS == status)
        {
            status_vxlib = VXLIB_harrisCornersDetect_i32f(prms->hcs_score, &prms->vxlib_score,
                prms->hcd_corners, prms->hcd_strength,
                prms->vxlib_score.dim_x * prms->vxlib_score.dim_y,
                &num_corners_hcd, sc_thr->data.f32,
                ((uint16_t)prms->gs/2U) + ((uint16_t)prms->bs/2U), ((uint16_t)prms->gs/2U) + ((uint16_t)prms->bs/2U));

            if ((vx_status)status_vxlib != (vx_status)VXLIB_SUCCESS)
            {
                status = (vx_status)VX_FAILURE;
            }
        }
        if ((vx_status)VX_SUCCESS == status)
        {
            if(prms->rad >= 2.0f)
            {
                status_vxlib = VXLIB_harrisCornersNMS_i32f(prms->hcd_corners,
                    prms->hcd_strength, num_corners_hcd, prms->nms_corners,
                    prms->nms_strength, num_corners_hcd, &num_corners,
                    prms->nms_scratch, prms->nms_scratch_size, prms->rad, NULL);

                if ((vx_status)status_vxlib != (vx_status)VXLIB_SUCCESS)
                {
                    status = (vx_status)VX_FAILURE;
                }
            }
            else
            {
                num_corners = num_corners_hcd;
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            if (NULL != sc_cnt)
            {
                sc_cnt->data.u32 = num_corners;
            }

            /* Copy array and other parameters */
            if (num_corners > arr->capacity)
            {
                num_corners = arr->capacity;
            }
            arr->num_items = num_corners;

            kp = (vx_keypoint_t *)arr_target_ptr;
            for (i = 0; i < num_corners; i ++)
            {
                uint32_t temp_x, temp_y;
                temp_x = rect.start_x + ((uint32_t)prms->nms_corners[i] & (uint32_t)0xFFFFU);
                kp->x = (int32_t)temp_x;
                temp_y = rect.start_y + (((uint32_t)prms->nms_corners[i] & (uint32_t)0xFFFF0000U) >> 16);
                kp->y = (int32_t)temp_y;
                kp->strength = prms->nms_strength[i];
                kp->scale = 0.0f;
                kp->orientation = 0.0f;
                kp->tracking_status = 1;
                kp->error = 0.0f;

                kp ++;
            }
        }
        tivxCheckStatus(&status, tivxMemBufferUnmap(arr_target_ptr, arr->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelHarrisCornersCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    tivx_obj_desc_image_t *img;
    tivxHarrisCornersParams *prms = NULL;
    tivx_bam_kernel_details_t kernel_details[4];
    tivx_obj_desc_scalar_t *sc_dist, *sc_gs, *sc_bs, *sc_sens;

    if (num_params != TIVX_KERNEL_HARRIS_CORNERS_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HARRIS_CORNERS_MAX_PARAMS; i ++)
        {
            if ((NULL == obj_desc[i]) &&
                (i != TIVX_KERNEL_HARRIS_CORNERS_NUM_CORNERS_IDX))
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxMemResetScratchHeap((vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxBamInitKernelDetails(&kernel_details[0], 4, kernel);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        img = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_HARRIS_CORNERS_INPUT_IDX];
        sc_bs = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRIS_CORNERS_BLOCK_SIZE_IDX];
        sc_gs = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRIS_CORNERS_GRADIENT_SIZE_IDX];
        sc_sens = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRIS_CORNERS_SENSITIVITY_IDX];
        sc_dist = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRIS_CORNERS_MIN_DISTANCE_IDX];

        prms = tivxMemAlloc(sizeof(tivxHarrisCornersParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            BAM_VXLIB_harrisCornersScore_i16s_i16s_o32f_params harris_score_kernel_params;
            VXLIB_bufParams2D_t vxlib_src;
            VXLIB_bufParams2D_t *buf_params[2];

            memset(prms, 0, sizeof(tivxHarrisCornersParams));

            BAM_NodeParams node_list[] = { \
                {SOURCE_NODE, (uint32_t)BAM_KERNELID_DMAREAD_AUTOINCREMENT, NULL}, \
                {SOBEL_NODE, (uint32_t)BAM_KERNELID_VXLIB_SOBEL_3X3_I8U_O16S_O16S, NULL}, \
                {SCORE_NODE, (uint32_t)BAM_KERNELID_VXLIB_HARRISCORNERSSCORE_I16S_I16S_O32F, NULL}, \
                {SINK_NODE, (uint32_t)BAM_KERNELID_DMAWRITE_AUTOINCREMENT, NULL}, \
                {BAM_END_NODE_MARKER,   0,                          NULL},\
            };

            prms->gs = (uint32_t)sc_gs->data.s32;
            prms->bs = (uint32_t)sc_bs->data.s32;

            harris_score_kernel_params.sensitivity   = sc_sens->data.f32;
            harris_score_kernel_params.gradient_size = (uint8_t)prms->gs;
            harris_score_kernel_params.block_size    = (uint8_t)prms->bs;

            /* Update the Sobel and harris score type accordingly */
            if(3U == prms->gs)
            {
                node_list[SOBEL_NODE].kernelId = (uint32_t)BAM_KERNELID_VXLIB_SOBEL_3X3_I8U_O16S_O16S;
                BAM_VXLIB_sobel_3x3_i8u_o16s_o16s_getKernelInfo( NULL,
                    &kernel_details[SOBEL_NODE].kernel_info);

                BAM_VXLIB_harrisCornersScore_i16s_i16s_o32f_getKernelInfo(
                    &harris_score_kernel_params,
                    &kernel_details[SCORE_NODE].kernel_info);
            }
            else if(5U == prms->gs)
            {
                node_list[SOBEL_NODE].kernelId = (uint32_t)BAM_KERNELID_VXLIB_SOBEL_5X5_I8U_O16S_O16S;
                BAM_VXLIB_sobel_5x5_i8u_o16s_o16s_getKernelInfo( NULL,
                    &kernel_details[SOBEL_NODE].kernel_info);

                BAM_VXLIB_harrisCornersScore_i16s_i16s_o32f_getKernelInfo(
                    &harris_score_kernel_params,
                    &kernel_details[SCORE_NODE].kernel_info);
            }
            else
            {
                node_list[SOBEL_NODE].kernelId = (uint32_t)BAM_KERNELID_VXLIB_SOBEL_7X7_I8U_O32S_O32S;
                BAM_VXLIB_sobel_7x7_i8u_o32s_o32s_getKernelInfo( NULL,
                    &kernel_details[SOBEL_NODE].kernel_info);

                node_list[SCORE_NODE].kernelId = (uint32_t)BAM_KERNELID_VXLIB_HARRISCORNERSSCORE_I32S_I32S_O32F;
                BAM_VXLIB_harrisCornersScore_i32s_i32s_o32f_getKernelInfo(
                    (BAM_VXLIB_harrisCornersScore_i32s_i32s_o32f_params*)&harris_score_kernel_params,
                    &kernel_details[SCORE_NODE].kernel_info);
            }

            BAM_EdgeParams edge_list[]= {\
                {{SOURCE_NODE, 0},
                    {SOBEL_NODE, (uint8_t)BAM_VXLIB_SOBEL_3X3_I8U_O16S_O16S_INPUT_IMAGE_PORT}},\

                {{SOBEL_NODE, (uint8_t)BAM_VXLIB_SOBEL_3X3_I8U_O16S_O16S_OUTPUT_X_PORT},
                    {SCORE_NODE, (uint8_t)BAM_VXLIB_HARRISCORNERSSCORE_I16S_I16S_O32F_INPUT_X_PORT}},\

                {{SOBEL_NODE, (uint8_t)BAM_VXLIB_SOBEL_3X3_I8U_O16S_O16S_OUTPUT_Y_PORT},
                    {SCORE_NODE, (uint8_t)BAM_VXLIB_HARRISCORNERSSCORE_I16S_I16S_O32F_INPUT_Y_PORT}},\

                {{SCORE_NODE, (uint8_t)BAM_VXLIB_HARRISCORNERSSCORE_I16S_I16S_O32F_OUTPUT_PORT},
                    {SINK_NODE, 0}},\

                {{BAM_END_NODE_MARKER, 0},
                    {BAM_END_NODE_MARKER, 0}},\
            };

            kernel_details[SOURCE_NODE].compute_kernel_params = NULL;
            kernel_details[SOBEL_NODE].compute_kernel_params = NULL;
            kernel_details[SCORE_NODE].compute_kernel_params = &harris_score_kernel_params;
            kernel_details[SINK_NODE].compute_kernel_params = NULL;

            tivxInitBufParams(img, &vxlib_src);

            prms->vxlib_score.dim_x = img->imagepatch_addr[0].dim_x -
                (prms->gs - 1U) - (prms->bs - 1U);
            prms->vxlib_score.dim_y = img->imagepatch_addr[0].dim_y -
                (prms->gs - 1U) - (prms->bs - 1U);
            prms->vxlib_score.stride_y =
                (int32_t)img->imagepatch_addr[0].dim_x * (int32_t)sizeof((uint32_t)VXLIB_FLOAT32);
            prms->vxlib_score.data_type = (uint32_t)VXLIB_FLOAT32;

            prms->hcs_score_size = (uint32_t)prms->vxlib_score.stride_y *
                prms->vxlib_score.dim_y;

            prms->hcs_score = tivxMemAlloc(prms->hcs_score_size,
                (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);

            if (NULL == prms->hcs_score)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                /* Fill in the frame level sizes of buffers here. If the port
                 * is optionally disabled, put NULL */
                buf_params[0] = &vxlib_src;
                buf_params[1] = &prms->vxlib_score;

                status = tivxBamCreateHandleMultiNode(node_list,
                    sizeof(node_list)/sizeof(BAM_NodeParams),
                    edge_list,
                    sizeof(edge_list)/sizeof(BAM_EdgeParams),
                    buf_params, kernel_details,
                    &prms->graph_handle);
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                /* TODO: reduce size to save memory */
                prms->hcd_sprs_size = (uint32_t)prms->vxlib_score.stride_y *
                    img->imagepatch_addr[0].dim_y;

                prms->hcd_sprs = tivxMemAlloc(prms->hcd_sprs_size,
                    (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
                if (NULL == prms->hcd_sprs)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }


            if ((vx_status)VX_SUCCESS == status)
            {
                prms->hcd_corners_size = img->imagepatch_addr[0].dim_x *
                    img->imagepatch_addr[0].dim_y * sizeof(uint32_t);

                prms->hcd_corners = tivxMemAlloc(prms->hcd_corners_size,
                    (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
                if (NULL == prms->hcd_corners)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->hcd_strength_size = img->imagepatch_addr[0].dim_x *
                    img->imagepatch_addr[0].dim_y * sizeof(vx_float32);

                prms->hcd_strength = tivxMemAlloc(prms->hcd_strength_size,
                    (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
                if (NULL == prms->hcd_strength)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                vx_float32 radius = sc_dist->data.f32;

                prms->rad = radius;
                if (prms->rad >= 2.0f)
                {
                    /* We need to run the NMS function, so allocate buffers */

                    prms->nms_scratch_size = (img->imagepatch_addr[0].dim_x *
                        img->imagepatch_addr[0].dim_y) +
                            ((((uintptr_t)prms->rad+1U)*2U)*
                            (((uintptr_t)prms->rad+1U)*2U)*2U);

                    prms->nms_scratch = tivxMemAlloc(prms->nms_scratch_size,
                        (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
                    if (NULL == prms->nms_scratch)
                    {
                        status = (vx_status)VX_ERROR_NO_MEMORY;
                    }

                    if ((vx_status)VX_SUCCESS == status)
                    {
                        prms->nms_corners_size = img->imagepatch_addr[0].dim_x *
                            img->imagepatch_addr[0].dim_y * sizeof(uint32_t);

                        prms->nms_corners = tivxMemAlloc(prms->nms_corners_size,
                            (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
                        if (NULL == prms->nms_corners)
                        {
                            status = (vx_status)VX_ERROR_NO_MEMORY;
                        }
                    }

                    if ((vx_status)VX_SUCCESS == status)
                    {
                        prms->nms_strength_size = img->imagepatch_addr[0].dim_x *
                            img->imagepatch_addr[0].dim_y * sizeof(vx_float32);

                        prms->nms_strength = tivxMemAlloc(prms->nms_strength_size,
                            (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
                        if (NULL == prms->nms_strength)
                        {
                            status = (vx_status)VX_ERROR_NO_MEMORY;
                        }
                    }
                }
                else
                {
                    /* We don't need to run the NMS operation since distance is within 3x3 neighborhood which "detect" already covers */
                    /* Set output pointers to be same as input pointers */

                    prms->nms_corners = prms->hcd_corners;
                    prms->nms_strength = prms->hcd_strength;
                }
            }
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxHarrisCornersParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxHarrisCornersFreeMem(prms);
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelHarrisCornersDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivxHarrisCornersParams *prms = NULL;

    if (num_params != TIVX_KERNEL_HARRIS_CORNERS_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HARRIS_CORNERS_MAX_PARAMS; i ++)
        {
            if ((NULL == obj_desc[i]) &&
                (i != TIVX_KERNEL_HARRIS_CORNERS_NUM_CORNERS_IDX))
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxHarrisCornersParams) == size))
        {
            tivxBamDestroyHandle(prms->graph_handle);
            tivxHarrisCornersFreeMem(prms);
        }
    }

    return (status);
}

static void tivxHarrisCornersFreeMem(tivxHarrisCornersParams *prms)
{
    if (NULL != prms)
    {
        if (NULL != prms->hcs_score)
        {
            tivxMemFree(prms->hcs_score, prms->hcs_score_size,
                (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
            prms->hcs_score = NULL;
        }
        if (NULL != prms->hcd_sprs)
        {
            tivxMemFree(prms->hcd_sprs, prms->hcd_sprs_size, (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
            prms->hcd_sprs = NULL;
        }
        if (NULL != prms->hcd_corners)
        {
            tivxMemFree(prms->hcd_corners, prms->hcd_corners_size,
                (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
            prms->hcd_corners = NULL;
        }
        if (NULL != prms->hcd_strength)
        {
            tivxMemFree(prms->hcd_strength, prms->hcd_strength_size,
                (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
            prms->hcd_strength = NULL;
        }
        if(prms->rad >= 2.0f)
        {
            if (NULL != prms->nms_corners)
            {
                tivxMemFree(prms->nms_corners, prms->nms_corners_size,
                    (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
                prms->nms_corners = NULL;
            }
            if (NULL != prms->nms_strength)
            {
                tivxMemFree(prms->nms_strength, prms->nms_strength_size,
                    (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
                prms->nms_strength = NULL;
            }
            if (NULL != prms->nms_scratch)
            {
                tivxMemFree(prms->nms_scratch, prms->nms_scratch_size,
                    (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
                prms->nms_scratch = NULL;
            }
        }

        tivxMemFree(prms, sizeof(tivxHarrisCornersParams), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}

void tivxAddTargetKernelBamHarrisCorners(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_harris_corners_target_kernel = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_HARRIS_CORNERS,
            target_name,
            tivxKernelHarrisCornersProcess,
            tivxKernelHarrisCornersCreate,
            tivxKernelHarrisCornersDelete,
            NULL,
            NULL);
    }
}

void tivxRemoveTargetKernelBamHarrisCorners(void)
{
    tivxRemoveTargetKernel(vx_harris_corners_target_kernel);
}
