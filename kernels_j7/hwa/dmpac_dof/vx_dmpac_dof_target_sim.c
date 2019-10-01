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
#include "TI/j7.h"
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_dmpac_dof.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "vx_kernels_hwa_target.h"
#include "LibDenseOpticalFlow.h"

#define PYRAMIDAL_CURRENT 0
#define DELAYED_LEFT      1
#define PYRAMIDAL_LEFT    2
#define TEMPORAL          3

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
    int resv4;
} tivxDmpacDofParams;

#else

#include <dlfcn.h>

typedef struct {

    int magic;
    int *input_current[TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS];
    int *input_reference[TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS];
    int *current_prediction;
    int *past_prediction;
    int pyramid_size[TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS][2];
    int confidence_histogram[TIVX_KERNEL_DMPAC_DOF_MAX_CONFIDENCE_HIST_BINS];
    DOFParams dofParams;
    int sof_max_pix_in_row;
    int sof_fv_height;
    int firstFrame;
    int temporalConfig;
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
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status tivxDmpacDofAllocMem(tivxDmpacDofParams *prms);
static void tivxDmpacDofFreeMem(tivxDmpacDofParams *prms);

static vx_status tivxDmpacDofGetErrStatusCmd(
                        tivx_obj_desc_scalar_t *scalar_obj_desc);

static vx_status tivxDmpacDofSetCsPrms(tivxDmpacDofParams *prms,
                        tivx_obj_desc_user_data_object_t *usr_data_obj);

static void tivxDmpacDofSetPredictors(tivxDmpacDofParams *prms,
                        tivx_dmpac_dof_params_t *params);

static vx_status dof_sof(tivxDmpacDofParams *prms, uint8_t *sof_mask, uint32_t sof_mask_stride);

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

    if(prms->dofParams.model[0] != '\0')
    {
        remove(prms->dofParams.model);
    }

    tivxMemFree(prms, sizeof(tivxDmpacDofParams), TIVX_MEM_EXTERNAL);
}

static vx_status VX_CALLBACK tivxDmpacDofProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *configuration_desc;
    tivx_obj_desc_image_t   *input_current_base_desc = NULL;
    tivx_obj_desc_image_t   *input_reference_base_desc = NULL;
    tivx_obj_desc_pyramid_t *input_current_desc;
    tivx_obj_desc_pyramid_t *input_reference_desc;
    tivx_obj_desc_image_t *flow_vector_in_desc;
    tivx_obj_desc_image_t *sparse_of_map_desc;
    tivx_obj_desc_image_t *flow_vector_out_desc;
    tivx_obj_desc_distribution_t *confidence_histogram_desc;
    tivx_obj_desc_image_t *img_current_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    tivx_obj_desc_image_t *img_reference_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    uint32_t i, total_pyramid_lvl;
    tivxDmpacDofParams *prms = NULL;
    int *past_prediction = NULL;

    if ( (num_params != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
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
        void *img_curr_base_target_ptr = NULL;
        void *img_ref_base_target_ptr = NULL;

        /* point to descriptors with correct type */
        configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX];
        input_current_base_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_BASE_IDX];
        input_reference_base_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_BASE_IDX];
        input_current_desc = (tivx_obj_desc_pyramid_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX];
        input_reference_desc = (tivx_obj_desc_pyramid_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_IDX];
        flow_vector_in_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_IN_IDX];
        sparse_of_map_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_MAP_IDX];
        flow_vector_out_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX];
        confidence_histogram_desc = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIDENCE_HISTOGRAM_IDX];

        tivxGetObjDescList(input_current_desc->obj_desc_id, (tivx_obj_desc_t**)img_current_desc, input_current_desc->num_levels);
        tivxGetObjDescList(input_reference_desc->obj_desc_id, (tivx_obj_desc_t**)img_reference_desc, input_reference_desc->num_levels);


        if(NULL != input_current_base_desc)
        {
            total_pyramid_lvl = input_current_desc->num_levels + 1;

            /* convert shared pointer to target/local pointer for base images */
            img_curr_base_target_ptr = tivxMemShared2TargetPtr(&input_current_base_desc->mem_ptr[0]);
            img_ref_base_target_ptr = tivxMemShared2TargetPtr(&input_reference_base_desc->mem_ptr[0]);
        }
        else
        {
            total_pyramid_lvl = input_current_desc->num_levels;
        }

        /* convert shared pointer to target/local pointer for rest of parametes*/
        configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);

        if(flow_vector_in_desc != NULL)
        {
            flow_vector_in_target_ptr = tivxMemShared2TargetPtr(&flow_vector_in_desc->mem_ptr[0]);
        }

        if( sparse_of_map_desc != NULL)
        {
            sparse_of_map_target_ptr = tivxMemShared2TargetPtr(&sparse_of_map_desc->mem_ptr[0]);
        }

        flow_vector_out_target_ptr = tivxMemShared2TargetPtr(&flow_vector_out_desc->mem_ptr[0]);

        if( confidence_histogram_desc != NULL)
        {
            confidence_histogram_target_ptr = tivxMemShared2TargetPtr(&confidence_histogram_desc->mem_ptr);
        }

        for(i=0; i<input_current_desc->num_levels ; i++)
        {
            img_current_target_ptr[i] = tivxMemShared2TargetPtr(&img_current_desc[i]->mem_ptr[0]);

            img_reference_target_ptr[i] = tivxMemShared2TargetPtr(&img_reference_desc[i]->mem_ptr[0]);
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

        if(NULL != input_current_base_desc)
        {
            tivxMemBufferMap(img_curr_base_target_ptr,
               input_current_base_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);

            tivxMemBufferMap(img_ref_base_target_ptr,
               input_reference_base_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);
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
        for(i=0; i<total_pyramid_lvl; i++)
        {
            if(NULL == input_current_base_desc)
            {
                lse_reformat_in_dof(img_current_desc[i],
                                    img_current_target_ptr[i],
                                    prms->input_current[i]);
                lse_reformat_in_dof(img_reference_desc[i],
                                    img_reference_target_ptr[i],
                                    prms->input_reference[i]);
            }
            else
            {
                if(0 == i)
                {
                    lse_reformat_in_dof(input_current_base_desc,
                                    img_curr_base_target_ptr,
                                    prms->input_current[i]);
                    lse_reformat_in_dof(input_reference_base_desc,
                                    img_ref_base_target_ptr,
                                    prms->input_reference[i]);
                }
                else
                {
                    lse_reformat_in_dof(img_current_desc[i-1],
                                    img_current_target_ptr[i-1],
                                    prms->input_current[i]);
                    lse_reformat_in_dof(img_reference_desc[i-1],
                                    img_reference_target_ptr[i-1],
                                    prms->input_reference[i]);
                }
            }
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
        {
                /* VLAB HWA are 64b models in backend, but int* pointers are 32b on R5F core
                 * Hence need to trick VLAB to think we are passing 64b pointer
                 */
                int *input_current_org[TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS];
                int *input_reference_org[TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS];

                /* save orignal 32b pointers */
                for(i=0; i<TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS; i++)
                {
                    input_current_org[i] = prms->input_current[i];
                    input_reference_org[i] = prms->input_reference[i];
                }
                /* set pointers to how VLAB 64b C models will see it */
                for(i=0; i<TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS; i++)
                {
                    prms->input_current[2*i] = input_current_org[i];
                    prms->input_current[2*i+1] = NULL;
                    prms->input_reference[2*i] = input_reference_org[i];
                    prms->input_reference[2*i+1] = NULL;
                }

                status = vlab_hwa_process(DMPAC_DOF_BASE_ADDRESS, "DMPAC_DOF", sizeof(tivxDmpacDofParams), prms);
                prms->past_prediction = past_prediction;

                /* restore pointer to how 32b R5F will see it  */
                for(i=0; i<TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS; i++)
                {
                    prms->input_current[i] = input_current_org[i];
                    prms->input_reference[i] = input_reference_org[i];
                }
                for(i=TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS; i<TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS*2; i++)
                {
                    prms->input_current[i] = NULL;
                    prms->input_reference[i] = NULL;
                }
        }

#else
        int (*dofProcess)();
        void *handle = dlopen("libDOF.so", RTLD_LOCAL|RTLD_LAZY|RTLD_DEEPBIND );

        if(handle == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Unable to load libDOF.so\n");
            status = VX_FAILURE;
        }
        else
        {
            dlerror();
            dofProcess = dlsym(handle, "dofProcess");

            if (dlerror() != NULL)  {
                VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Unable to load dofProcess symbol from libDOF.so\n");
                status = VX_FAILURE;
            }
            else
            {
                dofProcess(
                   &prms->dofParams,
                   prms->input_current,
                   prms->input_reference,
                   past_prediction,
                   prms->pyramid_size,
                   prms->current_prediction,
                   prms->confidence_histogram);

                dlclose(handle);
            }
        }

#endif

        if(prms->firstFrame == 1)
        {
            prms->firstFrame = 0;
            prms->dofParams.baseLayerPredictorConfiguration[TEMPORAL] = prms->temporalConfig;
        }

        if( sparse_of_map_desc != NULL)
        {
            dof_sof(prms, (uint8_t *)sparse_of_map_target_ptr, sparse_of_map_desc->imagepatch_addr[0].stride_y);
        }

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
        if(NULL != input_current_base_desc)
        {
            tivxMemBufferUnmap(img_curr_base_target_ptr,
               input_current_base_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);

            tivxMemBufferUnmap(img_ref_base_target_ptr,
               input_reference_base_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
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

    if ( (num_params != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS)
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
            tivx_obj_desc_user_data_object_t *params_array = (tivx_obj_desc_user_data_object_t*)obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX];
            tivx_obj_desc_image_t *img[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
            tivx_dmpac_dof_params_t *params;
            uint32_t i;
            void *params_array_target_ptr;
            tivx_obj_desc_image_t *input_current_base_desc = (tivx_obj_desc_image_t*)obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_BASE_IDX];
            tivx_obj_desc_image_t *flow_vector_out_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX];

            memset(prms, 0, sizeof(tivxDmpacDofParams));

            if(NULL != input_current_base_desc)
            {
                prms->dofParams.numberOfPyramidLevels = pyr1->num_levels + 1;
            }
            else
            {
                prms->dofParams.numberOfPyramidLevels = pyr1->num_levels;
            }

            tivxGetObjDescList(pyr1->obj_desc_id, (tivx_obj_desc_t**)img, pyr1->num_levels);

            for(i=0; i<prms->dofParams.numberOfPyramidLevels; i++)
            {
                if(NULL == input_current_base_desc)
                {
                    prms->pyramid_size[i][0] = img[i]->height;
                    prms->pyramid_size[i][1] = img[i]->width;
                }
                else
                {
                    if(0 == i)
                    {
                        prms->pyramid_size[i][0] = input_current_base_desc->height;
                        prms->pyramid_size[i][1] = input_current_base_desc->width;
                    }
                    else
                    {
                        prms->pyramid_size[i][0] = img[i-1]->height;
                        prms->pyramid_size[i][1] = img[i-1]->width;
                    }
                }
            }

            params_array_target_ptr = tivxMemShared2TargetPtr(&params_array->mem_ptr);

            tivxMemBufferMap(params_array_target_ptr, params_array->mem_size,
                VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

            params = (tivx_dmpac_dof_params_t *)params_array_target_ptr;

            prms->magic = 0xC0DEFACE;
            prms->dofParams.direction = params->motion_direction ;
            prms->dofParams.flowPostFiltering = params->median_filter_enable ;
            prms->dofParams.nonBaseLevelFlowPostFiltering = params->median_filter_enable ;
            prms->dofParams.msf = params->motion_smoothness_factor ;
            prms->dofParams.verticalSearchRange[0] = params->vertical_search_range[0] ;
            prms->dofParams.verticalSearchRange[1] = params->vertical_search_range[1] ;
            prms->dofParams.horizontalSearchRange = params->horizontal_search_range ;
            prms->dofParams.confidenceFeatureIIRFilterCoeffQ8 = params->iir_filter_alpha ;
            prms->dofParams.baseLevelConfidenceScorePacked =
                                (flow_vector_out_desc->format == VX_DF_IMAGE_U16) ? 0 : 1;
            prms->dofParams.model[0] = '\0';
            prms->sof_max_pix_in_row = params->sof_max_pix_in_row;
            prms->sof_fv_height = params->sof_fv_height;
            prms->firstFrame = 1;



            tivxDmpacDofSetPredictors(prms, params);

            /* For first frame, turn off temporal predictor */
            prms->dofParams.baseLayerPredictorConfiguration[TEMPORAL] = 0;

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
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = VX_SUCCESS;

    uint32_t size;
    tivxDmpacDofParams *prms = NULL;

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&prms, &size);

    if (VX_SUCCESS == status)
    {
        switch (node_cmd_id)
        {
            case TIVX_DMPAC_DOF_CMD_CS_PARAMS:
            {
                status = tivxDmpacDofSetCsPrms(prms,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_DMPAC_DOF_CMD_SET_HTS_BW_LIMIT_PARAMS:
            {
                /* Does not apply for cmodel version */
                break;
            }
            case TIVX_DMPAC_DOF_CMD_GET_ERR_STATUS:
            {
                status = tivxDmpacDofGetErrStatusCmd(
                    (tivx_obj_desc_scalar_t *)obj_desc[0U]);
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR,
                        "tivxDmpacDofControl: Invalid Input\n");
                status = VX_FAILURE;
                break;
            }
        }
    }

    return (status);
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

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */

static vx_status tivxDmpacDofGetErrStatusCmd(
    tivx_obj_desc_scalar_t *scalar_obj_desc)
{
    vx_status                           status = VX_SUCCESS;

    if (NULL != scalar_obj_desc)
    {
        /* Cmodel doesn't give same error output as HW */
        scalar_obj_desc->data.u32 = 0;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxDmpacDofGetErrStatus: Null argument\n");
        status = VX_FAILURE;
    }

    return (status);
}

static vx_status tivxDmpacDofSetCsPrms(tivxDmpacDofParams *prms,
                        tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    uint32_t                            idx;
    vx_status                           status = VX_SUCCESS;
    tivx_dmpac_dof_cs_tree_params_t    *cs_prms;
    void                               *target_ptr;
    FILE                               *fout;

    if(NULL == usr_data_obj)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxDmpacDofSetCsPrms: Invalid Input\n");
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        if (sizeof(tivx_dmpac_dof_cs_tree_params_t) ==
                usr_data_obj->mem_size)
        {
            cs_prms = (tivx_dmpac_dof_cs_tree_params_t *)target_ptr;

            prms->dofParams.confidenceGainFactor = cs_prms->cs_gain;

            sprintf(prms->dofParams.model, "/tmp/.ti_ovx_dmpac_dof_cs_tree_%p.cfg", (void *)cs_prms);

            fout = fopen(prms->dofParams.model, "w");

            fprintf(fout, "#AdaboostCompactDescriptor: 16 2 10 7\n");
            fprintf(fout, "###################### Depth = 0 Node = 0 ######################\n");
            for(idx=0; idx<16; idx++) fprintf(fout, "%9d", cs_prms->decision_tree_index[idx][0]); fprintf(fout, "  #FIDS(FeatureIds)\n");
            for(idx=0; idx<16; idx++) fprintf(fout, "%9d", cs_prms->decision_tree_threshold[idx][0]); fprintf(fout, "  #TH(Thresholds)\n");
            fprintf(fout, "###################### Depth = 1 Node = 1 ######################\n");
            for(idx=0; idx<16; idx++) fprintf(fout, "%9d", cs_prms->decision_tree_index[idx][1]); fprintf(fout, "  #FIDS(FeatureIds)\n");
            for(idx=0; idx<16; idx++) fprintf(fout, "%9d", cs_prms->decision_tree_threshold[idx][1]); fprintf(fout, "  #TH(Thresholds)\n");
            fprintf(fout, "###################### Depth = 1 Node = 2 ######################\n");
            for(idx=0; idx<16; idx++) fprintf(fout, "%9d", cs_prms->decision_tree_index[idx][2]); fprintf(fout, "  #FIDS(FeatureIds)\n");
            for(idx=0; idx<16; idx++) fprintf(fout, "%9d", cs_prms->decision_tree_threshold[idx][2]); fprintf(fout, "  #TH(Thresholds)\n");
            fprintf(fout, "###################### Depth = 2 Node = 3 ######################\n");
            for(idx=0; idx<16; idx++) fprintf(fout, "%9d", cs_prms->decision_tree_weight[idx][0]); fprintf(fout, "  #WT(Weights)\n");
            fprintf(fout, "###################### Depth = 2 Node = 4 ######################\n");
            for(idx=0; idx<16; idx++) fprintf(fout, "%9d", cs_prms->decision_tree_weight[idx][1]); fprintf(fout, "  #WT(Weights)\n");
            fprintf(fout, "###################### Depth = 2 Node = 5 ######################\n");
            for(idx=0; idx<16; idx++) fprintf(fout, "%9d", cs_prms->decision_tree_weight[idx][2]); fprintf(fout, "  #WT(Weights)\n");
            fprintf(fout, "###################### Depth = 2 Node = 6 ######################\n");
            for(idx=0; idx<16; idx++) fprintf(fout, "%9d", cs_prms->decision_tree_weight[idx][3]); fprintf(fout, "  #WT(Weights)\n");

            fclose(fout);

        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxDmpacDofSetCsPrms: Invalid Argument\n");
            status = VX_FAILURE;
        }
        tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxDmpacDofSetCsPrms: Null Argument\n");
    }

    return (status);
}

/* This section is meant to match the driver in vhwa_m2mDofApi.c */
static void tivxDmpacDofSetPredictors(tivxDmpacDofParams *prms,
                        tivx_dmpac_dof_params_t *params)
{
    /* Check Base layer (all 4 are valid) */
    if((params->base_predictor[0] == TIVX_DMPAC_DOF_PREDICTOR_DELEY_LEFT) ||
       (params->base_predictor[1] == TIVX_DMPAC_DOF_PREDICTOR_DELEY_LEFT))
    {
        prms->dofParams.baseLayerPredictorConfiguration[DELAYED_LEFT] = 1;
    }

    if((params->base_predictor[0] == TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL) ||
       (params->base_predictor[1] == TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL))
    {
        prms->dofParams.baseLayerPredictorConfiguration[TEMPORAL] = 1;
        prms->temporalConfig = 1;
    }

    if((params->base_predictor[0] == TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT) ||
       (params->base_predictor[1] == TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT))
    {
        prms->dofParams.baseLayerPredictorConfiguration[PYRAMIDAL_LEFT] = 1;
    }

    if((params->base_predictor[0] == TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED) ||
       (params->base_predictor[1] == TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED))
    {
        prms->dofParams.baseLayerPredictorConfiguration[PYRAMIDAL_CURRENT] = 1;
    }

    /* Check Inter layer (3 are valid) */
    if((params->inter_predictor[0] == TIVX_DMPAC_DOF_PREDICTOR_DELEY_LEFT) ||
       (params->inter_predictor[1] == TIVX_DMPAC_DOF_PREDICTOR_DELEY_LEFT))
    {
        prms->dofParams.nonBaseNonTopLayerPredictorConfiguration[DELAYED_LEFT] = 1;
    }

    if((params->inter_predictor[0] == TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT) ||
       (params->inter_predictor[1] == TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT))
    {
        prms->dofParams.nonBaseNonTopLayerPredictorConfiguration[PYRAMIDAL_LEFT] = 1;
    }

    if((params->inter_predictor[0] == TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED) ||
       (params->inter_predictor[1] == TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED))
    {
        prms->dofParams.nonBaseNonTopLayerPredictorConfiguration[PYRAMIDAL_CURRENT] = 1;
    }

    /* Set top layer (1 is fixed in driver, so we will match it here) */
    prms->dofParams.topLayerPredictorConfiguration[DELAYED_LEFT] = 1;
}

static vx_status dof_sof(tivxDmpacDofParams *prms, uint8_t *sof_mask, uint32_t sof_mask_stride)
{
    vx_status status = VX_SUCCESS;
    int i, j, k, bitCnt;

    int *input_ptr = prms->current_prediction;
    int *output_ptr = prms->current_prediction;
    int width = prms->pyramid_size[0][1];
    int height = prms->pyramid_size[0][0];
    int flow_width = prms->sof_max_pix_in_row;
    int flow_height = prms->sof_fv_height;
    uint32_t out_line_cnt = 0;

    if(prms->current_prediction != NULL)
    {
        for(j = 0; j<height; j+=2)
        {
            uint32_t flag_valid_line = 0;

            for(i=0; i<width/8; i++)
            {
                /* Check if there are any outputs on this line */
                if(sof_mask[j*sof_mask_stride + i])
                {
                    flag_valid_line = 1;
                    break;
                }
                if(sof_mask[(j+1)*sof_mask_stride + i])
                {
                    flag_valid_line = 1;
                    break;
                }
            }

            if(flag_valid_line)
            {
                for(k=0; k<2; k++)
                {
                    uint32_t col_cnt = 0;

                    for(i=0; i<width/8; i++)
                    {
                        uint8_t maskVal = sof_mask[(j+k)*sof_mask_stride + i];

                        for(bitCnt=0; bitCnt<8; bitCnt++)
                        {
                            if(col_cnt < flow_width)
                            {
                                if(maskVal & 1)
                                {
                                    output_ptr[flow_width*out_line_cnt + col_cnt] = input_ptr[(j+k)*width + (i*8) + bitCnt];
                                    col_cnt++;
                                }
                                maskVal >>= 1;
                            }
                        }
                    }
                    for(;col_cnt < flow_width; col_cnt++)
                    {
                        output_ptr[flow_width*out_line_cnt + col_cnt] = 0;
                    }
                    out_line_cnt++;

                    if(out_line_cnt >= flow_height)
                    {
                        break;
                    }
                }
            }
            if(out_line_cnt >= flow_height)
            {
                break;
            }
        }
    }
    else
    {
        status = VX_FAILURE;
    }

    return status;
}

