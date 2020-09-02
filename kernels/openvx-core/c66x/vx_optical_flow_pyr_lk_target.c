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
#include <tivx_kernel_optical_flow_pyr_lk.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <math.h>

static tivx_target_kernel vx_optical_flow_pyr_lk_target_kernel = NULL;

typedef struct
{
    tivx_obj_desc_image_t *img_obj_desc_old[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    tivx_obj_desc_image_t *img_obj_desc_new[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    int16_t *pSchX, *pSchY;
    uint32_t sch_size;
    __float2_t *oldPoints;
    __float2_t *newPoints;
    uint32_t points_size;
    uint8_t *tracking;
    uint32_t tracking_buff_size;
    uint8_t *scratch;
    uint32_t scratch_buff_size;
    VXLIB_bufParams2D_t old_params, new_params, scharrGrad_params;
} tivxOpticalFlowPyrLkParams;

static vx_status VX_CALLBACK tivxOpticalFlowPyrLk(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxOpticalFlowPyrLkCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxOpticalFlowPyrLkDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static void tivxOpticalFlowPyrLKFreeMem(tivxOpticalFlowPyrLkParams *prms);

static vx_status VX_CALLBACK tivxOpticalFlowPyrLk(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_pyramid_t *old_pyramid_desc;
    tivx_obj_desc_pyramid_t *new_pyramid_desc;
    tivx_obj_desc_array_t *prevpts_desc;
    tivx_obj_desc_array_t *estimatedpts_desc;
    tivx_obj_desc_array_t *nextpts_desc;
    tivx_obj_desc_scalar_t *termination_desc;
    tivx_obj_desc_scalar_t *epsilon_desc;
    tivx_obj_desc_scalar_t *num_iterations_desc;
    tivx_obj_desc_scalar_t *use_initial_estimate_desc;
    tivx_obj_desc_scalar_t *window_dimension_desc;
    tivx_obj_desc_image_t *old_image, *new_image;
    tivxOpticalFlowPyrLkParams *prms = NULL;
    uint32_t size, levels;
    uint8_t *old_image_addr, *new_image_addr;

    uint8_t termination_value;
    VXLIB_F32 epsilon_value;
    uint32_t num_iterations_value;
    bool use_initial_estimate_value;
    size_t window_dimension_value;

    vx_int32 level;
    size_t num_levels;
    vx_float32 pyramid_scale, scale;
    vx_size list_length;
    vx_keypoint_t *oldPts_addr;
    vx_keypoint_t *estPts_addr;
    vx_keypoint_t *newPts_addr;
    vx_size list_indx;

    if ((num_params != TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_OLD_IMAGES_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_IMAGES_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_OLD_POINTS_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_POINTS_ESTIMATES_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_POINTS_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_TERMINATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_EPSILON_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NUM_ITERATIONS_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_USE_INITIAL_ESTIMATE_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_WINDOW_DIMENSION_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        old_pyramid_desc = (tivx_obj_desc_pyramid_t *)obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_OLD_IMAGES_IDX];
        new_pyramid_desc = (tivx_obj_desc_pyramid_t *)obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_IMAGES_IDX];
        prevpts_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_OLD_POINTS_IDX];
        estimatedpts_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_POINTS_ESTIMATES_IDX];
        nextpts_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_POINTS_IDX];
        termination_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_TERMINATION_IDX];
        epsilon_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_EPSILON_IDX];
        num_iterations_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NUM_ITERATIONS_IDX];
        use_initial_estimate_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_USE_INITIAL_ESTIMATE_IDX];
        window_dimension_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_WINDOW_DIMENSION_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((vx_status)VX_SUCCESS == status)
        {
            if ((NULL != prms) && (size == sizeof(tivxOpticalFlowPyrLkParams)))
            {
                tivxGetObjDescList(old_pyramid_desc->obj_desc_id,
                    (tivx_obj_desc_t **)prms->img_obj_desc_old,
                    old_pyramid_desc->num_levels);

                for (levels = 0U; levels < old_pyramid_desc->num_levels;
                            levels ++)
                {
                    if (NULL == prms->img_obj_desc_old[levels])
                    {
                        status = (vx_status)VX_FAILURE;
                        break;
                    }
                }

                tivxGetObjDescList(new_pyramid_desc->obj_desc_id,
                    (tivx_obj_desc_t **)prms->img_obj_desc_new,
                    new_pyramid_desc->num_levels);

                for (levels = 0U; levels < new_pyramid_desc->num_levels;
                        levels ++)
                {
                    if (NULL == prms->img_obj_desc_new[levels])
                    {
                        status = (vx_status)VX_FAILURE;
                        break;
                    }
                }
            }
            else
            {
                status = (vx_status)VX_FAILURE;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *prevpts_target_ptr;
        void *estimatedpts_target_ptr;
        void *nextpts_target_ptr;

        prevpts_target_ptr = tivxMemShared2TargetPtr(&prevpts_desc->mem_ptr);
        estimatedpts_target_ptr = tivxMemShared2TargetPtr(&estimatedpts_desc->mem_ptr);
        nextpts_target_ptr = tivxMemShared2TargetPtr(&nextpts_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(prevpts_target_ptr,
           prevpts_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferMap(estimatedpts_target_ptr,
           estimatedpts_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferMap(nextpts_target_ptr,
           nextpts_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_WRITE_ONLY));

        termination_value = termination_desc->data.u08;
        epsilon_value = epsilon_desc->data.f32;
        num_iterations_value = num_iterations_desc->data.u32;
        use_initial_estimate_value = (bool)use_initial_estimate_desc->data.boolean;
        window_dimension_value = window_dimension_desc->data.size;

        num_levels = old_pyramid_desc->num_levels;
        pyramid_scale = old_pyramid_desc->scale;
        list_length = prevpts_desc->num_items;

        oldPts_addr = (vx_keypoint_t *)prevpts_target_ptr;
        estPts_addr = oldPts_addr;
        newPts_addr = (vx_keypoint_t *)nextpts_target_ptr;
        nextpts_desc->num_items = prevpts_desc->num_items;

        /* if Use initial estimate is true, use the estimated pts */
        if (use_initial_estimate_value)
        {
            estPts_addr = (vx_keypoint_t *)estimatedpts_target_ptr;
        }

        /* Before processing any levels, copy from data structure to arrays for kernel processing (also cast to float) */
        /* Consider making this a VXLIB function (init function) */
        for(list_indx = 0; list_indx < list_length; list_indx++)
        {
            prms->oldPoints[list_indx] = _ftof2((VXLIB_F32)oldPts_addr[list_indx].y, (VXLIB_F32)oldPts_addr[list_indx].x);
            prms->newPoints[list_indx] = _ftof2((VXLIB_F32)estPts_addr[list_indx].y, (VXLIB_F32)estPts_addr[list_indx].x);
            prms->tracking[list_indx] = (uint8_t)estPts_addr[list_indx].tracking_status;

            /* As per spec, these are passthrough */
            newPts_addr[list_indx].strength    = oldPts_addr[list_indx].strength;
            newPts_addr[list_indx].scale       = oldPts_addr[list_indx].scale;
            newPts_addr[list_indx].orientation = oldPts_addr[list_indx].orientation;
            newPts_addr[list_indx].error       = oldPts_addr[list_indx].error;
        }

        /* We are done with external prevpts and estimatedpts buffers since we have copied what we need internally s*/
        tivxCheckStatus(&status, tivxMemBufferUnmap(prevpts_target_ptr,
           prevpts_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferUnmap(estimatedpts_target_ptr,
           estimatedpts_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));

        /* Set the scale for the lowest resolution level for first iteration */
        scale = (vx_float32)pow((vx_float64)pyramid_scale, (vx_float64)((vx_float64)num_levels-(vx_float64)1.0f));

        /* call kernel processing functions for each level of the pyramid */
        for( level = (vx_int32)num_levels-1; level >= 0; level-- )
        {
            vx_uint32 width;
            vx_uint32 height;
            vx_rectangle_t rect;
            vx_uint32 temp_status;
            void *old_image_target_ptr;
            void *new_image_target_ptr;

            old_image = prms->img_obj_desc_old[level];
            new_image = prms->img_obj_desc_new[level];

            /* Map Old image */
            old_image_target_ptr = tivxMemShared2TargetPtr(&old_image->mem_ptr[0]);
            tivxCheckStatus(&status, tivxMemBufferMap(old_image_target_ptr,
                old_image->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));

            old_image_addr = (uint8_t *)((uintptr_t)old_image_target_ptr +
                tivxComputePatchOffset(0, 0, &old_image->imagepatch_addr[0U]));

            /* Map New image */
            new_image_target_ptr = tivxMemShared2TargetPtr(&new_image->mem_ptr[0]);
            tivxCheckStatus(&status, tivxMemBufferMap(new_image_target_ptr,
                new_image->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY)); /* Not sure if this is READ_ONLY */

            new_image_addr = (uint8_t *)((uintptr_t)new_image_target_ptr +
                tivxComputePatchOffset(0, 0, &new_image->imagepatch_addr[0U]));

            /* Fetch ROI */
            rect = old_image->valid_roi;
            width  = rect.end_x - rect.start_x;
            height = rect.end_y - rect.start_y;

            /* Prepare and execute scharr gradient function */

            prms->old_params.dim_x     = width;
            prms->old_params.dim_y     = height;
            prms->old_params.stride_y  = old_image->imagepatch_addr[0U].stride_y;
            prms->old_params.data_type = (uint32_t)VXLIB_UINT8;

            prms->scharrGrad_params.dim_x = width;
            prms->scharrGrad_params.dim_y = height - 2U;
            prms->scharrGrad_params.stride_y = (int32_t)width*2;
            prms->scharrGrad_params.data_type = (uint32_t)VXLIB_INT16;

            temp_status = (vx_uint32)status;
            temp_status |= (vx_uint32)VXLIB_scharr_3x3_i8u_o16s_o16s(old_image_addr, &prms->old_params,
                                                     &prms->pSchX[width+1U], &prms->scharrGrad_params,
                                                     &prms->pSchY[width+1U], &prms->scharrGrad_params);

            /* Prepare and execute tracking function */

            prms->scharrGrad_params.dim_y = height;

            prms->new_params.dim_x      = width;
            prms->new_params.dim_y      = height;
            prms->new_params.stride_y  = new_image->imagepatch_addr[0U].stride_y;
            prms->new_params.data_type = (uint32_t)VXLIB_UINT8;

            /* Adjust scale for all but smallest resoltion */
            if (level != ((vx_int32)num_levels-1)) {
                scale = 1.0f/pyramid_scale;
            }

            temp_status |= (vx_uint32)VXLIB_trackFeaturesLK_i8u(old_image_addr, &prms->old_params,
                                                 new_image_addr, &prms->new_params,
                                                 prms->pSchX, &prms->scharrGrad_params,
                                                 prms->pSchY, &prms->scharrGrad_params,
                                                 prms->oldPoints, prms->newPoints, prms->tracking, list_length,
                                                 num_iterations_value, epsilon_value, scale, (uint8_t)window_dimension_value, (uint8_t)level,
                                                 termination_value, prms->scratch, prms->scratch_buff_size);
            status = (vx_status)temp_status;

            tivxCheckStatus(&status, tivxMemBufferUnmap(old_image_target_ptr,
               old_image->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
               (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferUnmap(new_image_target_ptr,
               new_image->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
               (vx_enum)VX_READ_ONLY));

        }

        /* After all level processing, copy results back to output array */
        for(list_indx = 0; list_indx < list_length; list_indx++)
        {
            vx_float32 temp_x = _lof2(prms->newPoints[list_indx]) + 0.5f;
            vx_float32 temp_y = _hif2(prms->newPoints[list_indx]) + 0.5f;
            newPts_addr[list_indx].x = (int32_t)temp_x;
            newPts_addr[list_indx].y = (int32_t)temp_y;
            newPts_addr[list_indx].tracking_status = (int32_t)prms->tracking[list_indx];
        }

        /* kernel processing function complete */
        tivxCheckStatus(&status, tivxMemBufferUnmap(nextpts_target_ptr,
           nextpts_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_WRITE_ONLY));
    }

    if(status != VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Process Callback Failed\n");
    }

    return status;
}

static vx_status VX_CALLBACK tivxOpticalFlowPyrLkCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    tivx_obj_desc_scalar_t *window_dimension_desc;
    tivx_obj_desc_pyramid_t *old_pyramid_desc;
    size_t window_dimension_value;
    tivx_obj_desc_array_t *prevpts_desc;
    tivx_obj_desc_image_t *img_obj_desc_old[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    vx_size list_length;
    tivxOpticalFlowPyrLkParams *prms = NULL;
    tivx_obj_desc_image_t *old_image;
    vx_rectangle_t rect;
    vx_uint32 width;
    vx_uint32 height;

    if (num_params != TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        status = tivxMemResetScratchHeap(TIVX_MEM_EXTERNAL_SCRATCH);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        prevpts_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_OLD_POINTS_IDX];
        window_dimension_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_WINDOW_DIMENSION_IDX];
        old_pyramid_desc = (tivx_obj_desc_pyramid_t *)obj_desc[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_OLD_IMAGES_IDX];
        window_dimension_value = window_dimension_desc->data.size;

        list_length = prevpts_desc->capacity;

        tivxGetObjDescList(old_pyramid_desc->obj_desc_id,
            (tivx_obj_desc_t **)img_obj_desc_old, old_pyramid_desc->num_levels);

        /* Using the base level of the pyramid to get sizes b/c that will be the largest */
        old_image = img_obj_desc_old[0];
        rect = old_image->valid_roi;

        width  = rect.end_x - rect.start_x;
        height = rect.end_y - rect.start_y;

        prms = tivxMemAlloc(sizeof(tivxOpticalFlowPyrLkParams),
            (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxOpticalFlowPyrLkParams));

            prms->scratch_buff_size = window_dimension_value*window_dimension_value*8U*sizeof(uint8_t);

            prms->scratch = tivxMemAlloc(prms->scratch_buff_size,
                (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);

            if (NULL == prms->scratch)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->tracking_buff_size = list_length*sizeof(uint8_t);

                prms->tracking = tivxMemAlloc(prms->tracking_buff_size,
                    (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->tracking)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->points_size = list_length*sizeof(__float2_t);

                prms->oldPoints = tivxMemAlloc(prms->points_size,
                    (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->oldPoints)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->newPoints = tivxMemAlloc(prms->points_size,
                    (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->newPoints)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->sch_size = width*height*sizeof(int16_t);

                prms->pSchX = tivxMemAlloc(prms->sch_size,
                    (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->pSchX)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->pSchY = tivxMemAlloc(prms->sch_size,
                    (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->pSchY)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                tivxSetTargetKernelInstanceContext(kernel, prms,
                    sizeof(tivxOpticalFlowPyrLkParams));
            }
            else
            {
                tivxOpticalFlowPyrLKFreeMem(prms);
            }
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxOpticalFlowPyrLkDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivxOpticalFlowPyrLkParams *prms = NULL;

    if (num_params != TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
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
            (sizeof(tivxOpticalFlowPyrLkParams) == size))
        {
            tivxOpticalFlowPyrLKFreeMem(prms);
        }
    }

    return (status);
}

static void tivxOpticalFlowPyrLKFreeMem(tivxOpticalFlowPyrLkParams *prms)
{
    if (NULL != prms)
    {
        if (NULL != prms->scratch)
        {
            tivxMemFree(prms->scratch, prms->scratch_buff_size,
                (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
        }
        if (NULL != prms->tracking)
        {
            tivxMemFree(prms->tracking, prms->tracking_buff_size,
                (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
        }
        if (NULL != prms->oldPoints)
        {
            tivxMemFree(prms->oldPoints, prms->points_size,
                (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
        }
        if (NULL != prms->newPoints)
        {
            tivxMemFree(prms->newPoints, prms->points_size,
                (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
        }
        if (NULL != prms->pSchX)
        {
            tivxMemFree(prms->pSchX, prms->sch_size, (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
        }
        if (NULL != prms->pSchY)
        {
            tivxMemFree(prms->pSchY, prms->sch_size, (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
        }

        tivxMemFree(prms, sizeof(tivxOpticalFlowPyrLkParams), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}

void tivxAddTargetKernelOpticalFlowPyrLk(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ( self_cpu == (vx_enum)TIVX_CPU_ID_DSP1 )
    {
        strncpy(target_name, TIVX_TARGET_DSP1, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    else
    if ( self_cpu == (vx_enum)TIVX_CPU_ID_DSP2 )
    {
        strncpy(target_name, TIVX_TARGET_DSP2, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        status = (vx_status)VX_FAILURE;
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        vx_optical_flow_pyr_lk_target_kernel = tivxAddTargetKernel(
                            (vx_enum)VX_KERNEL_OPTICAL_FLOW_PYR_LK,
                            target_name,
                            tivxOpticalFlowPyrLk,
                            tivxOpticalFlowPyrLkCreate,
                            tivxOpticalFlowPyrLkDelete,
                            NULL,
                            NULL);
    }
}

void tivxRemoveTargetKernelOpticalFlowPyrLk(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_optical_flow_pyr_lk_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_optical_flow_pyr_lk_target_kernel = NULL;
    }
}


