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

#include "TI/tivx.h"
#include "TI/tda4x.h"
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_dmpac_dof.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "vx_kernels_hwa_target.h"
#include "LibDenseOpticalFlow.h"

#ifdef VLAB_HWA

typedef struct {

    int magic;
    int resv1;
    int *input_current[TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS*2];
    int *input_reference[TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS*2];
    int *current_prediction;
    int resv2;
    int *past_prediction;
    int resv3;
    int pyramid_size[TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS][2];
    int confidence_histogram[TIVX_KERNEL_DMPAC_DOF_MAX_CONFIDENCE_HIST_BINS];
    DOFParams dofParams;
} tivxDmpacDofParams;

#else

typedef struct {

    int magic;
    int *input_current[TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS];
    int *input_reference[TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS];
    int *current_prediction;
    int *past_prediction;
    int pyramid_size[TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS][2];
    int confidence_histogram[TIVX_KERNEL_DMPAC_DOF_MAX_CONFIDENCE_HIST_BINS];
    DOFParams dofParams;
} tivxDmpacDofParams;

#endif

static tivx_target_kernel vx_dmpac_dof_target_kernel = NULL;

static vx_status VX_CALLBACK tivxDmpacDofProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDmpacDofCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDmpacDofDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDmpacDofControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status tivxDmpacDofAllocMem(tivxDmpacDofParams *prms);
static void tivxDmpacDofFreeMem(tivxDmpacDofParams *prms);



static vx_status tivxDmpacDofAllocMem(tivxDmpacDofParams *prms)
{
    uint32_t i, size;
    vx_status status = VX_SUCCESS;

    for(i=0; i<TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS; i++)
    {
        prms->input_current[i] = NULL;
        prms->input_reference[i] = NULL;
    }
    prms->current_prediction = NULL;
    prms->past_prediction = NULL;

    for(i=0; i<prms->dofParams.numberOfPyramidLevels; i++)
    {
        size = (prms->pyramid_size[i][0])*(prms->pyramid_size[i][1])*sizeof(int);


        if(status==VX_SUCCESS)
        {
            prms->input_current[i] = tivxMemAlloc( size, TIVX_MEM_EXTERNAL);
            if(prms->input_current[i]==NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Unable to allocate memory\n");
                status = VX_FAILURE;
            }
        }
        if(status==VX_SUCCESS)
        {
            prms->input_reference[i] = tivxMemAlloc( size, TIVX_MEM_EXTERNAL);
            if(prms->input_reference[i]==NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Unable to allocate memory\n");
                status = VX_FAILURE;
            }
        }
    }
    size = prms->pyramid_size[0][0]*prms->pyramid_size[0][1]*sizeof(int);
    if(status==VX_SUCCESS)
    {
        prms->current_prediction = tivxMemAlloc( size, TIVX_MEM_EXTERNAL);
        if(prms->current_prediction==NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Unable to allocate memory\n");
            status = VX_FAILURE;
        }
    }
    if(status==VX_SUCCESS)
    {
        prms->past_prediction = tivxMemAlloc( size, TIVX_MEM_EXTERNAL);
        if(prms->past_prediction==NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Unable to allocate memory\n");
            status = VX_FAILURE;
        }
    }

    return status;
}

static void tivxDmpacDofFreeMem(tivxDmpacDofParams *prms)
{
    uint32_t i, size;

    for(i=0; i<prms->dofParams.numberOfPyramidLevels; i++)
    {
        size = (prms->pyramid_size[i][0])*(prms->pyramid_size[i][1])*sizeof(int);

        if(prms->input_current[i] != NULL)
        {
            tivxMemFree( prms->input_current[i], size, TIVX_MEM_EXTERNAL);
            prms->input_current[i] = NULL;
        }
        if(prms->input_reference[i] != NULL)
        {
            tivxMemFree( prms->input_reference[i], size, TIVX_MEM_EXTERNAL);
            prms->input_reference[i] = NULL;
        }
    }
    size = prms->pyramid_size[0][0]*prms->pyramid_size[0][1]*sizeof(int);
    if(prms->current_prediction!=NULL)
    {
        tivxMemFree( prms->current_prediction, size, TIVX_MEM_EXTERNAL);
        prms->current_prediction = NULL;
    }
    if(prms->past_prediction!=NULL)
    {
        tivxMemFree( prms->past_prediction, size, TIVX_MEM_EXTERNAL);
        prms->past_prediction = NULL;
    }
    tivxMemFree(prms, size, TIVX_MEM_EXTERNAL);
}

static vx_status VX_CALLBACK tivxDmpacDofProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_array_t *configuration_desc;
    tivx_obj_desc_pyramid_t *input_current_desc;
    tivx_obj_desc_pyramid_t *input_ref_desc;
    tivx_obj_desc_image_t *flow_vector_in_desc;
    tivx_obj_desc_image_t *sparse_of_map_desc;
    tivx_obj_desc_image_t *flow_vector_out_desc;
    tivx_obj_desc_distribution_t *confidence_histogram_desc;
    tivx_obj_desc_image_t *img_current_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    tivx_obj_desc_image_t *img_reference_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    uint32_t i;
    tivxDmpacDofParams *prms = NULL;
    int *past_prediction = NULL;

    if ( num_params != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX])
    )
    {
        status = VX_FAILURE;
    }
    if(status==VX_SUCCESS)
    {
        uint32_t size;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxDmpacDofParams) != size))
        {
            status = VX_FAILURE;
        }
    }
    if(status==VX_SUCCESS)
    {
        void *configuration_target_ptr;
        void *flow_vector_in_target_ptr = NULL;
        void *sparse_of_map_target_ptr = NULL;
        void *flow_vector_out_target_ptr;
        void *confidence_histogram_target_ptr = NULL;
        void *img_current_target_ptr[TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS];
        void *img_reference_target_ptr[TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS];

        /* point to descriptors with correct type */
        configuration_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX];
        input_current_desc = (tivx_obj_desc_pyramid_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX];
        input_ref_desc = (tivx_obj_desc_pyramid_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_IDX];
        flow_vector_in_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_IN_IDX];
        sparse_of_map_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_MAP_IDX];
        flow_vector_out_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX];
        confidence_histogram_desc = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIDENCE_HISTOGRAM_IDX];

        tivxGetObjDescList(input_current_desc->obj_desc_id, (tivx_obj_desc_t**)img_current_desc, input_current_desc->num_levels);
        tivxGetObjDescList(input_ref_desc->obj_desc_id, (tivx_obj_desc_t**)img_reference_desc, input_ref_desc->num_levels);

        /* convert shared pointer to target/local pointer */
        configuration_target_ptr = tivxMemShared2TargetPtr(
          configuration_desc->mem_ptr.shared_ptr, configuration_desc->mem_ptr.mem_heap_region);

        if(flow_vector_in_desc != NULL)
        {
            flow_vector_in_target_ptr = tivxMemShared2TargetPtr(
                flow_vector_in_desc->mem_ptr[0].shared_ptr, flow_vector_in_desc->mem_ptr[0].mem_heap_region);
        }

        if( sparse_of_map_desc != NULL)
        {
            sparse_of_map_target_ptr = tivxMemShared2TargetPtr(
              sparse_of_map_desc->mem_ptr[0].shared_ptr, sparse_of_map_desc->mem_ptr[0].mem_heap_region);
        }

        flow_vector_out_target_ptr = tivxMemShared2TargetPtr(
          flow_vector_out_desc->mem_ptr[0].shared_ptr, flow_vector_out_desc->mem_ptr[0].mem_heap_region);

        if( confidence_histogram_desc != NULL)
        {
            confidence_histogram_target_ptr = tivxMemShared2TargetPtr(
              confidence_histogram_desc->mem_ptr.shared_ptr, confidence_histogram_desc->mem_ptr.mem_heap_region);
        }

        for(i=0; i<input_current_desc->num_levels ; i++)
        {
            img_current_target_ptr[i] = tivxMemShared2TargetPtr(
                img_current_desc[i]->mem_ptr[0].shared_ptr, img_current_desc[i]->mem_ptr[0].mem_heap_region);

            img_reference_target_ptr[i] = tivxMemShared2TargetPtr(
                img_reference_desc[i]->mem_ptr[0].shared_ptr, img_reference_desc[i]->mem_ptr[0].mem_heap_region);
        }

        /* map buffers */

        tivxMemBufferMap(configuration_target_ptr,
           configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        if(flow_vector_in_desc != NULL)
        {
            tivxMemBufferMap(flow_vector_in_target_ptr,
                flow_vector_in_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);
        }
        if( sparse_of_map_desc != NULL)
        {
            tivxMemBufferMap(sparse_of_map_target_ptr,
               sparse_of_map_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);
        }
        tivxMemBufferMap(flow_vector_out_target_ptr,
           flow_vector_out_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);
        if( confidence_histogram_desc != NULL)
        {
            tivxMemBufferMap(confidence_histogram_target_ptr,
               confidence_histogram_desc->mem_size, VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }

        for(i=0; i<input_current_desc->num_levels ; i++)
        {
            tivxMemBufferMap(img_current_target_ptr[i],
               img_current_desc[i]->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);

            tivxMemBufferMap(img_reference_target_ptr[i],
               img_reference_desc[i]->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);
        }

        /* copy input */
        for(i=0; i<input_current_desc->num_levels ; i++)
        {
            lse_reformat_in_dof(img_current_desc[i], img_current_target_ptr[i], prms->input_current[i]);
            lse_reformat_in_dof(img_reference_desc[i], img_reference_target_ptr[i], prms->input_reference[i]);
        }
        /* when NULL past prediction not used */
        past_prediction = NULL;
        if(flow_vector_in_desc != NULL)
        {
            lse_reformat_in_dof(flow_vector_in_desc, flow_vector_in_target_ptr, prms->past_prediction);
            past_prediction = prms->past_prediction;
        }

        /* call kernel processing function */
#ifdef VLAB_HWA

        status = vlab_hwa_process(DMPAC_DOF_BASE_ADDRESS, "DMPAC_DOF", sizeof(tivxDmpacDofParams), &prms);
        prms->past_prediction = past_prediction;

#else

        dofProcess(
           &prms->dofParams,
           prms->input_current,
           prms->input_reference,
           past_prediction,
           prms->pyramid_size,
           prms->current_prediction,
           prms->confidence_histogram);

#endif

        /* kernel processing function complete */

        /* copy output */
        lse_reformat_out_dof(flow_vector_out_desc, flow_vector_out_desc, flow_vector_out_target_ptr, prms->current_prediction);
        if(confidence_histogram_desc!=NULL)
        {
            uint32_t *confidence_histogram = confidence_histogram_target_ptr;
            for(i=0; i<TIVX_KERNEL_DMPAC_DOF_MAX_CONFIDENCE_HIST_BINS; i++)
            {
                confidence_histogram[i] = prms->confidence_histogram[i];
            }
        }

        /* unmap buffers */

        tivxMemBufferUnmap(configuration_target_ptr,
           configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        if(flow_vector_in_desc != NULL)
        {
            tivxMemBufferUnmap(flow_vector_in_target_ptr,
               flow_vector_in_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);
        }
        if( sparse_of_map_desc != NULL)
        {
            tivxMemBufferUnmap(sparse_of_map_target_ptr,
               sparse_of_map_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);
        }
        tivxMemBufferUnmap(flow_vector_out_target_ptr,
           flow_vector_out_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);
        if( confidence_histogram_desc != NULL)
        {
            tivxMemBufferUnmap(confidence_histogram_target_ptr,
               confidence_histogram_desc->mem_size, VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }
        for(i=0; i<input_current_desc->num_levels ; i++)
        {
            tivxMemBufferUnmap(img_current_target_ptr[i],
               img_current_desc[i]->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);

            tivxMemBufferUnmap(img_reference_target_ptr[i],
               img_reference_desc[i]->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxDmpacDofCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    if ( num_params != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX])
        )
    {
        status = VX_FAILURE;
    }
    if (VX_SUCCESS == status)
    {
        tivxDmpacDofParams *prms = NULL;

        prms = tivxMemAlloc(sizeof(tivxDmpacDofParams), TIVX_MEM_EXTERNAL);
        if (NULL != prms)
        {
            tivx_obj_desc_pyramid_t *pyr1 = (tivx_obj_desc_pyramid_t*)obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX];
            tivx_obj_desc_array_t *params_array = (tivx_obj_desc_array_t*)obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX];
            tivx_obj_desc_image_t *img[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
            tivx_dmpac_dof_params_t *params;
            uint32_t i;
            void *params_array_target_ptr;

            memset(prms, 0, sizeof(tivxDmpacDofParams));

            prms->dofParams.numberOfPyramidLevels = pyr1->num_levels;

            tivxGetObjDescList(pyr1->obj_desc_id, (tivx_obj_desc_t**)img, pyr1->num_levels);

            for(i=0; i<prms->dofParams.numberOfPyramidLevels; i++)
            {
                prms->pyramid_size[i][0] = img[i]->height;
                prms->pyramid_size[i][1] = img[i]->width;
            }

            params_array_target_ptr = tivxMemShared2TargetPtr(
                params_array->mem_ptr.shared_ptr, params_array->mem_ptr.mem_heap_region);

            tivxMemBufferMap(params_array_target_ptr, params_array->mem_size,
                VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

            params = (tivx_dmpac_dof_params_t *)params_array_target_ptr;

            prms->magic = 0xC0DEFACE;
            prms->dofParams.direction = params->motion_direction ;
            prms->dofParams.flowPostFiltering = params->median_filter_enable ;
            prms->dofParams.msf = params->motion_smoothness_factor ;
            prms->dofParams.verticalSearchRange[0] = params->vertical_search_range[0] ;
            prms->dofParams.verticalSearchRange[1] = params->vertical_search_range[1] ;
            prms->dofParams.horizontalSearchRange = params->horizontal_search_range ;

            tivxMemBufferUnmap(params_array_target_ptr, params_array->mem_size,
                VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

            status = tivxDmpacDofAllocMem(prms);
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxDmpacDofParams));
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Unable to create target kernel\n");
            if (NULL != prms)
            {
                tivxDmpacDofFreeMem(prms);
            }
        }
    }
    return status;
}

static vx_status VX_CALLBACK tivxDmpacDofDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    if ( num_params != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS )
    {
        status = VX_FAILURE;
    }

    if (VX_SUCCESS == status)
    {
        uint32_t size;
        tivxDmpacDofParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxDmpacDofParams) == size))
        {
            tivxDmpacDofFreeMem(prms);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxDmpacDofControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelDmpacDof(void)
{
    vx_status status = VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ( self_cpu == TIVX_CPU_ID_IPU1_0 )
    {
        strncpy(target_name, TIVX_TARGET_DMPAC_DOF, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
    else
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        vx_dmpac_dof_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_DMPAC_DOF_NAME,
                            target_name,
                            tivxDmpacDofProcess,
                            tivxDmpacDofCreate,
                            tivxDmpacDofDelete,
                            tivxDmpacDofControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelDmpacDof(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_dmpac_dof_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_dmpac_dof_target_kernel = NULL;
    }
}


