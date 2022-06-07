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
#ifdef BUILD_DMPAC_DOF

#include "TI/tivx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_dmpac_dof.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_hwa_host_priv.h"

#include <vx_internal.h>

static vx_kernel vx_dmpac_dof_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelDmpacDofValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelDmpacDofInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_bool tivxDmpacDofIsPipelineEnabled(vx_node node);

/* IMPORTANT NOTE:
 * The following way of accessing the graph and node implementation
 * details is not recommended for application code in general, but is used
 * here as this is internal TIOVX framework code.
 *
 * This is an exception for this kernal implementation and copying
 * this style of code should be generally avoided.
 */
static vx_bool tivxDmpacDofIsPipelineEnabled(vx_node node)
{
    tivx_graph_t   *g;
    tivx_node_t    *n;
    vx_bool         status;

    n = (tivx_node_t *)node;
    g = (tivx_graph_t *)n->graph;

    if (g->schedule_mode == (vx_enum)VX_GRAPH_SCHEDULE_MODE_NORMAL)
    {
        status = (vx_bool)vx_false_e;
    }
    else
    {
        status = (vx_bool)vx_true_e;
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelDmpacDofValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status   status = (vx_status)VX_SUCCESS;
    vx_bool     pipelineEnabled;

    vx_user_data_object configuration = NULL;
    vx_char configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size configuration_size;
    tivx_dmpac_dof_params_t params;

    vx_image input_current_base = NULL;
    vx_uint32 input_current_base_w;
    vx_uint32 input_current_base_h;
    vx_df_image input_current_base_fmt;

    vx_image input_reference_base = NULL;
    vx_uint32 input_reference_base_w;
    vx_uint32 input_reference_base_h;
    vx_df_image input_reference_base_fmt;

    vx_pyramid input_current = NULL;
    vx_size input_current_levels;
    vx_float32 input_current_scale;
    vx_uint32 input_current_w;
    vx_uint32 input_current_h;
    vx_df_image input_current_fmt;

    vx_pyramid input_reference = NULL;
    vx_size input_reference_levels;
    vx_float32 input_reference_scale;
    vx_uint32 input_reference_w;
    vx_uint32 input_reference_h;
    vx_df_image input_reference_fmt;

    vx_image flow_vector_in = NULL;
    vx_uint32 flow_vector_in_w;
    vx_uint32 flow_vector_in_h;
    vx_df_image flow_vector_in_fmt;

    vx_user_data_object sparse_of_config = NULL;
    vx_char sparse_of_config_name[VX_MAX_REFERENCE_NAME];
    vx_size sparse_of_config_size;

    vx_image sparse_of_map = NULL;
    vx_uint32 sparse_of_map_w;
    vx_uint32 sparse_of_map_h;
    vx_df_image sparse_of_map_fmt;

    vx_image flow_vector_out = NULL;
    vx_uint32 flow_vector_out_w;
    vx_uint32 flow_vector_out_h;
    vx_df_image flow_vector_out_fmt;

    vx_distribution confidence_histogram = NULL;
    vx_int32 confidence_histogram_offset = 0;
    vx_uint32 confidence_histogram_range = 0;
    vx_size confidence_histogram_numBins = 0;

    if ( (num != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX];
        input_current_base = (vx_image)parameters[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_BASE_IDX];
        input_reference_base = (vx_image)parameters[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_BASE_IDX];
        input_current = (vx_pyramid)parameters[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX];
        input_reference = (vx_pyramid)parameters[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_IDX];
        flow_vector_in = (vx_image)parameters[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_IN_IDX];
        sparse_of_config = (vx_user_data_object)parameters[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_CONFIG_IDX];
        sparse_of_map = (vx_image)parameters[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_MAP_IDX];
        flow_vector_out = (vx_image)parameters[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX];
        confidence_histogram = (vx_distribution)parameters[TIVX_KERNEL_DMPAC_DOF_CONFIDENCE_HISTOGRAM_IDX];
        pipelineEnabled = (vx_bool)tivxDmpacDofIsPipelineEnabled(node);
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));
        tivxCheckStatus(&status, vxCopyUserDataObject(configuration, 0, sizeof(tivx_dmpac_dof_params_t), &params, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));

        if (NULL != input_current_base)
        {
            tivxCheckStatus(&status, vxQueryImage(input_current_base, (vx_enum)VX_IMAGE_WIDTH, &input_current_base_w, sizeof(input_current_base_w)));
            tivxCheckStatus(&status, vxQueryImage(input_current_base, (vx_enum)VX_IMAGE_HEIGHT, &input_current_base_h, sizeof(input_current_base_h)));
            tivxCheckStatus(&status, vxQueryImage(input_current_base, (vx_enum)VX_IMAGE_FORMAT, &input_current_base_fmt, sizeof(input_current_base_fmt)));
        }

        if (NULL != input_reference_base)
        {
            tivxCheckStatus(&status, vxQueryImage(input_reference_base, (vx_enum)VX_IMAGE_WIDTH, &input_reference_base_w, sizeof(input_reference_base_w)));
            tivxCheckStatus(&status, vxQueryImage(input_reference_base, (vx_enum)VX_IMAGE_HEIGHT, &input_reference_base_h, sizeof(input_reference_base_h)));
            tivxCheckStatus(&status, vxQueryImage(input_reference_base, (vx_enum)VX_IMAGE_FORMAT, &input_reference_base_fmt, sizeof(input_reference_base_fmt)));
        }

        tivxCheckStatus(&status, vxQueryPyramid(input_current, (vx_enum)VX_PYRAMID_LEVELS, &input_current_levels, sizeof(input_current_levels)));
        tivxCheckStatus(&status, vxQueryPyramid(input_current, (vx_enum)VX_PYRAMID_SCALE, &input_current_scale, sizeof(input_current_scale)));
        tivxCheckStatus(&status, vxQueryPyramid(input_current, (vx_enum)VX_PYRAMID_WIDTH, &input_current_w, sizeof(input_current_w)));
        tivxCheckStatus(&status, vxQueryPyramid(input_current, (vx_enum)VX_PYRAMID_HEIGHT, &input_current_h, sizeof(input_current_h)));
        tivxCheckStatus(&status, vxQueryPyramid(input_current, (vx_enum)VX_PYRAMID_FORMAT, &input_current_fmt, sizeof(input_current_fmt)));

        tivxCheckStatus(&status, vxQueryPyramid(input_reference, (vx_enum)VX_PYRAMID_LEVELS, &input_reference_levels, sizeof(input_reference_levels)));
        tivxCheckStatus(&status, vxQueryPyramid(input_reference, (vx_enum)VX_PYRAMID_SCALE, &input_reference_scale, sizeof(input_reference_scale)));
        tivxCheckStatus(&status, vxQueryPyramid(input_reference, (vx_enum)VX_PYRAMID_WIDTH, &input_reference_w, sizeof(input_reference_w)));
        tivxCheckStatus(&status, vxQueryPyramid(input_reference, (vx_enum)VX_PYRAMID_HEIGHT, &input_reference_h, sizeof(input_reference_h)));
        tivxCheckStatus(&status, vxQueryPyramid(input_reference, (vx_enum)VX_PYRAMID_FORMAT, &input_reference_fmt, sizeof(input_reference_fmt)));

        if (NULL != flow_vector_in)
        {
            tivxCheckStatus(&status, vxQueryImage(flow_vector_in, (vx_enum)VX_IMAGE_WIDTH, &flow_vector_in_w, sizeof(flow_vector_in_w)));
            tivxCheckStatus(&status, vxQueryImage(flow_vector_in, (vx_enum)VX_IMAGE_HEIGHT, &flow_vector_in_h, sizeof(flow_vector_in_h)));
            tivxCheckStatus(&status, vxQueryImage(flow_vector_in, (vx_enum)VX_IMAGE_FORMAT, &flow_vector_in_fmt, sizeof(flow_vector_in_fmt)));
        }

        if (NULL != sparse_of_config)
        {
            tivxCheckStatus(&status, vxQueryUserDataObject(sparse_of_config, (vx_enum)VX_USER_DATA_OBJECT_NAME, &sparse_of_config_name, sizeof(sparse_of_config_name)));
            tivxCheckStatus(&status, vxQueryUserDataObject(sparse_of_config, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &sparse_of_config_size, sizeof(sparse_of_config_size)));
        }

        if (NULL != sparse_of_map)
        {
            tivxCheckStatus(&status, vxQueryImage(sparse_of_map, (vx_enum)VX_IMAGE_WIDTH, &sparse_of_map_w, sizeof(sparse_of_map_w)));
            tivxCheckStatus(&status, vxQueryImage(sparse_of_map, (vx_enum)VX_IMAGE_HEIGHT, &sparse_of_map_h, sizeof(sparse_of_map_h)));
            tivxCheckStatus(&status, vxQueryImage(sparse_of_map, (vx_enum)VX_IMAGE_FORMAT, &sparse_of_map_fmt, sizeof(sparse_of_map_fmt)));
        }

        tivxCheckStatus(&status, vxQueryImage(flow_vector_out, (vx_enum)VX_IMAGE_WIDTH, &flow_vector_out_w, sizeof(flow_vector_out_w)));
        tivxCheckStatus(&status, vxQueryImage(flow_vector_out, (vx_enum)VX_IMAGE_HEIGHT, &flow_vector_out_h, sizeof(flow_vector_out_h)));
        tivxCheckStatus(&status, vxQueryImage(flow_vector_out, (vx_enum)VX_IMAGE_FORMAT, &flow_vector_out_fmt, sizeof(flow_vector_out_fmt)));

        if (NULL != confidence_histogram)
        {
            tivxCheckStatus(&status, vxQueryDistribution(confidence_histogram, (vx_enum)VX_DISTRIBUTION_BINS, &confidence_histogram_numBins, sizeof(confidence_histogram_numBins)));
            tivxCheckStatus(&status, vxQueryDistribution(confidence_histogram, (vx_enum)VX_DISTRIBUTION_RANGE, &confidence_histogram_range, sizeof(confidence_histogram_range)));
            tivxCheckStatus(&status, vxQueryDistribution(confidence_histogram, (vx_enum)VX_DISTRIBUTION_OFFSET, &confidence_histogram_offset, sizeof(confidence_histogram_offset)));
        }
    }

    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((configuration_size != sizeof(tivx_dmpac_dof_params_t)) ||
            (strncmp(configuration_name, "tivx_dmpac_dof_params_t", sizeof(configuration_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be a user_data_object of type:\n tivx_dmpac_dof_params_t \n");
        }

        if (NULL != input_current_base)
        {
            if( ((vx_df_image)VX_DF_IMAGE_U8 != input_current_base_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_U16 != input_current_base_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 != input_current_base_fmt))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'input_current_base' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != input_reference_base)
        {
            if( ((vx_df_image)VX_DF_IMAGE_U8 != input_reference_base_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_U16 != input_reference_base_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 != input_reference_base_fmt))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'input_reference_base' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if( ((vx_df_image)VX_DF_IMAGE_U8 != input_current_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U16 != input_current_fmt) &&
            ((vx_df_image)TIVX_DF_IMAGE_P12 != input_current_fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input_current' should be a pyramid of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
        }

        if( ((vx_df_image)VX_DF_IMAGE_U8 != input_reference_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U16 != input_reference_fmt) &&
            ((vx_df_image)TIVX_DF_IMAGE_P12 != input_reference_fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input_reference' should be a pyramid of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
        }

        if (NULL != flow_vector_in)
        {
            if( (vx_df_image)VX_DF_IMAGE_U32 != flow_vector_in_fmt)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'flow_vector_in' should be an image of type:\n VX_DF_IMAGE_U32 \n");
            }
        }

        if (NULL != sparse_of_config)
        {
            if ((sparse_of_config_size != sizeof(tivx_dmpac_dof_sof_params_t)) ||
                (strncmp(sparse_of_config_name, "tivx_dmpac_dof_sof_params_t", sizeof(sparse_of_config_name)) != 0))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'sparse_of_config' should be a user_data_object of type:\n tivx_dmpac_dof_sof_params_t \n");
            }
        }

        if (NULL != sparse_of_map)
        {
            if ((vx_df_image)VX_DF_IMAGE_U8 != sparse_of_map_fmt)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'sparse_of_map' should be an image of type:\n VX_DF_IMAGE_U8 \n");
            }
        }

        if( ((vx_df_image)VX_DF_IMAGE_U16 != flow_vector_out_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U32 != flow_vector_out_fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'flow_vector_out' should be an image of type:\n VX_DF_IMAGE_U16 or VX_DF_IMAGE_U32 \n");
        }

    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (input_current_levels != input_reference_levels)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current' and 'input_reference' should have the same value for VX_PYRAMID_LEVELS\n");
        }
        if (input_current_scale != input_reference_scale)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current' and 'input_reference' should have the same value for VX_PYRAMID_SCALE\n");
        }
        if (input_current_w != input_reference_w)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current' and 'input_reference' should have the same value for VX_PYRAMID_WIDTH\n");
        }
        if (input_current_h != input_reference_h)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current' and 'input_reference' should have the same value for VX_PYRAMID_HEIGHT\n");
        }
        if (input_current_fmt != input_reference_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current' and 'input_reference' should have the same value for VX_PYRAMID_FORMAT\n");
        }

        if (NULL == sparse_of_map)
        {
            if (input_current_w != flow_vector_out_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current' and 'flow_vector_out' should have the same value for VX_IMAGE_WIDTH\n");
            }
            if (input_current_h != flow_vector_out_h)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current' and 'flow_vector_out' should have the same value for VX_IMAGE_HEIGHT\n");
            }
        }
        else
        {
            if (input_current_w < flow_vector_out_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'input_current' width should be greater that or equal to 'flow_vector_out' width\n");
            }
            if (input_current_h < flow_vector_out_h)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'input_current' height should be greater that or equal to 'flow_vector_out' height\n");
            }
        }

        if (NULL != flow_vector_in)
        {
            if (flow_vector_in_w != flow_vector_out_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'flow_vector_in' and 'flow_vector_out' should have the same value for VX_IMAGE_WIDTH\n");
            }
            if (flow_vector_in_h != flow_vector_out_h)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'flow_vector_in' and 'flow_vector_out' should have the same value for VX_IMAGE_HEIGHT\n");
            }
            if (flow_vector_in_h != flow_vector_out_h)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'flow_vector_in' and 'flow_vector_out' should have the same value for VX_IMAGE_HEIGHT\n");
            }
        }

        if (NULL != sparse_of_map)
        {
            if ((sparse_of_map_w*8U) != input_current_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'sparse_of_map' width should be == 'input_current' width / 8\n");
            }
            if (sparse_of_map_h != input_current_h)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'sparse_of_map' and 'input_current' should have the same value for VX_IMAGE_HEIGHT\n");
            }
        }

        if ((NULL != input_current_base) && (NULL != input_reference_base))
        {
            if (input_current_base_w != input_reference_base_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current_base' and 'input_reference_base' should have the same value for VX_IMAGE_WIDTH\n");
            }
            if (input_current_base_h != input_reference_base_h)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current_base' and 'input_reference_base' should have the same value for VX_IMAGE_HEIGHT\n");
            }
            if (input_current_base_fmt != input_reference_base_fmt)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current_base' and 'input_reference_base' should have the same value for VX_IMAGE_FORMAT\n");
            }
        }
    }

    /* CUSTOM PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (((NULL != input_current_base) && (NULL == input_reference_base)) ||
            ((NULL == input_current_base) && (NULL != input_reference_base)))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current_base' and 'input_reference_base' should both be NULL or not NULL !!!\n");
        }

        if( input_current_base == NULL )
        {
            if (input_current_levels > TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Number of total pyramid levels %d exceeds max supported values of %d !!!\n",
                    input_current_levels,
                    TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS
                    );
            }
        }
        else
        {
            if ( (input_current_levels+1U) > TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Number of total pyramid levels %d exceeds max supported values of %d !!!\n",
                    input_current_levels+1U,
                    TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS
                    );
            }
        }

        if(input_current_scale != VX_SCALE_PYRAMID_HALF )
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Scale of pyramid %3.1f does not match required scale of %3.1f!!!\n",
                input_current_scale,
                VX_SCALE_PYRAMID_HALF
                );
        }

        if((input_current_w > 2048U) || (input_current_h > 1024U))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Pyramid base image WxH exceeds max supported limit !!!\n"
                );
        }

        if (NULL != input_current_base)
        {
            if((input_current_base_w > 2048U) || (input_current_base_h > 1024U))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Pyramid base image WxH exceeds max supported limit !!!\n");
            }

            if((input_current_base_w != (input_current_w * 2U)) ||
               (input_current_base_h != (input_current_h * 2U)))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Base image WxH is not twice WxH of pyramid base size !!!\n");
            }
        }

        if( ((input_current_w % ((vx_uint32)1U<<input_current_levels)) != 0U) ||
            ((input_current_h % ((vx_uint32)1U<<input_current_levels)) != 0U)
          )
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Pyramid base image WxH of %d %d MUST be integer divisible by %d pixels !!!\n",
                input_current_w,
                input_current_h,
                1U<<(uint32_t)input_current_levels
                );
        }

        if( ((input_current_w / ((vx_uint32)1U<<(input_current_levels-1U))) < 32U) ||
            ((input_current_h / ((vx_uint32)1U<<(input_current_levels-1U))) < 16U)
          )
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: The highest level must have minimum dimensions WxH of 32x16 !!!\n");
        }

        if(confidence_histogram != NULL)
        {
            if((confidence_histogram_numBins!=TIVX_KERNEL_DMPAC_DOF_MAX_CONFIDENCE_HIST_BINS) ||
               (confidence_histogram_range!=TIVX_KERNEL_DMPAC_DOF_MAX_CONFIDENCE_HIST_BINS) ||
               (confidence_histogram_offset!=0))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Confidence measure histogram meta properties "
                    "(bins=%d, offset=%d, range=%d) do NOT match expected meta properties (bins=%d, offset=0, range=%d)!!!\n",
                    confidence_histogram_numBins, confidence_histogram_range, confidence_histogram_offset,
                    TIVX_KERNEL_DMPAC_DOF_MAX_CONFIDENCE_HIST_BINS, TIVX_KERNEL_DMPAC_DOF_MAX_CONFIDENCE_HIST_BINS
                    );
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if((confidence_histogram != NULL) &&
           ((vx_df_image)VX_DF_IMAGE_U16 == flow_vector_out_fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'confidence_histogram' should be NULL when the flow vector output is set to VX_DF_IMAGE_U16 type\n");
        }

        if(((sparse_of_config != NULL) && (sparse_of_map == NULL)) ||
           ((sparse_of_config == NULL) && (sparse_of_map != NULL)))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'sparse_of_config' and 'sparse_of_map' should both be NULL, or both be non-NULL\n");
        }

        if((params.inter_predictor[0] == TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL) ||
           (params.inter_predictor[1] == TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL is not a supported option for configuration.inter_predictor[n]\n");
        }

        if((params.base_predictor[0] == TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL) ||
           (params.base_predictor[1] == TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL))
        {
            /* Temporal predictor is ON. */
            if (params.flow_vector_internal_delay_num >
                            TIVX_DMPAC_DOF_MAX_FLOW_VECTOR_DELAY)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Invalid value for "
                         "'flow_vector_internal_delay_num'.\n");
            }
            else if (pipelineEnabled == (vx_bool)vx_false_e)
            {
                /* Pipeline is OFF. */
                if (params.flow_vector_internal_delay_num != 0)
                {
                    /* Temporal predictor is ON and pipeline is OFF. The flow
                     * vector input is expected to be handled by the caller and
                     * hence the flow_vector_internal_delay_num should be 0.
                     */
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR,
                             "Temporal predictor is ON and pipeline is OFF. The "
                             "flow vector input is expected to be handled by the "
                             "caller and hence the 'flow_vector_internal_delay_num' "
                             "should be 0.\n");
                }
            }
            else
            {
                /* Pipeline is ON. */
                if ((NULL == flow_vector_in) && (params.flow_vector_internal_delay_num == 0))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR,
                             "Parameter configuration.base_predictor[n] set to "
                             "TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL, parameter "
                             "'flow_vector_in' is NULL, graph is pipelined, "
                             "and 'flow_vector_internal_delay_num' is ZERO.\n");
                    VX_PRINT(VX_ZONE_ERROR,
                             "   Either 'flow_vector_in' should be "
                             "non-NULL or the internal delay shoud be non-zero, "
                             "when temporal prediction is enabled\n");
                }
                else if ((NULL != flow_vector_in) && (params.flow_vector_internal_delay_num != 0))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR,
                             "Parameter configuration.base_predictor[n] set to "
                             "TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL, parameter "
                             "'flow_vector_in' is non-NULL, graph is pipelined, "
                             "and 'flow_vector_internal_delay_num' is non-ZERO.\n");
                    VX_PRINT(VX_ZONE_ERROR,
                             "   Either 'flow_vector_in' should be "
                             "NULL or the internal delay shoud be zero, "
                             "when temporal prediction is enabled\n");
                }
            }

            if (NULL != sparse_of_map)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameter configuration.base_predictor[n] set to TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL, but parameter 'sparse_of_map' is not NULL\n");
                VX_PRINT(VX_ZONE_ERROR, "   Sparse optical flow is not supported when temporal prediction is enabled\n");
            }

            if ((vx_df_image)VX_DF_IMAGE_U16 == flow_vector_out_fmt)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameter configuration.base_predictor[n] set to TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL, but parameter 'flow_vector_out_fmt' is VX_DF_IMAGE_U16\n");
                VX_PRINT(VX_ZONE_ERROR, "   'flow_vector_out' format of VX_DF_IMAGE_U16 is not supported when temporal prediction is enabled\n");
            }
        }
        else
        {
            /* Temporal predictor is OFF. */
            /* Ignore flow_vector_internal_delay_num and no check needs to be
             * performed.
             */
            if (NULL != flow_vector_in)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameter 'flow_vector_in' is non-NULL, but configuration.base_predictor[n] set not set to TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL\n");
                VX_PRINT(VX_ZONE_ERROR, "   If temporal prediction is disabled, then 'flow_vector_in' should be NULL\n");
            }
        }

        if ((62U < params.vertical_search_range[0U]) ||
            (62U < params.vertical_search_range[1U]))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter vertical_search_range values should be between 0 and 62 inclusive\n");
        }
        else if (191U < params.horizontal_search_range)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter horizontal_search_range should be between 0 and 191 inclusive\n");
        }
        else if ((191U == params.horizontal_search_range) &&
            (112U < (params.vertical_search_range[0U] + params.vertical_search_range[1U])))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter vertical_search_range should sum to no greater than 112 if horizontal_search_range is 191\n");
        }
        else if (((62U == params.vertical_search_range[0U]) && (62U == params.vertical_search_range[1U])) &&
            (170U < (params.horizontal_search_range)))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter horizontal_search_range should be no greater than 170 if vertical_search_range values are 62\n");
        }
        else
        {
            /* do nothing */
        }
        if (1U < params.median_filter_enable)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter median_filter_enable should be either 0 or 1\n");
        }
        if (31U < params.motion_smoothness_factor)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter motion_smoothness_factor should be a value between 0 and 31 inclusive\n");
        }
        if (3U < params.motion_direction)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter motion_direction should be a value between 0 and 3 inclusive\n");
        }
        if ((1U > params.iir_filter_alpha) ||
            (255U < params.iir_filter_alpha))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter iir_filter_alpha should be a value between 1 and 255 inclusive\n");
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelDmpacDofInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (num_params != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS)
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

vx_status tivxAddKernelDmpacDof(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    if(status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        kernel = vxAddUserKernel(
                    context,
                    TIVX_KERNEL_DMPAC_DOF_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS,
                    tivxAddKernelDmpacDofValidate,
                    tivxAddKernelDmpacDofInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        index = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_PYRAMID,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_PYRAMID,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_DISTRIBUTION,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_DMPAC_DOF);
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        if (status != (vx_status)VX_SUCCESS)
        {
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
    }
    vx_dmpac_dof_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelDmpacDof(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_dmpac_dof_kernel;

    status = vxRemoveKernel(kernel);
    vx_dmpac_dof_kernel = NULL;

    return status;
}

void tivx_dmpac_dof_params_init(tivx_dmpac_dof_params_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_dmpac_dof_params_t));

        /* Set Search range */
        prms->vertical_search_range[0] = 48;
        prms->vertical_search_range[1] = 48;
        prms->horizontal_search_range = 191;

        prms->median_filter_enable = 1;
        prms->motion_smoothness_factor = 24;
        prms->motion_direction = 1; /* Forward Motion */

        /* Predictors */
        prms->base_predictor[0] = TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL;
        prms->base_predictor[1] = TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT;
        prms->inter_predictor[0] = TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED;
        prms->inter_predictor[1] = TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT;

        prms->iir_filter_alpha = 0x66;
    }
}

void tivx_dmpac_dof_sof_params_init(tivx_dmpac_dof_sof_params_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_dmpac_dof_sof_params_t));
    }
}

static tivx_dmpac_dof_cs_tree_params_t cs_tree_init_values =
{
    /*cs_gain*/
    224,

    /* decision_tree_index[16][3] */
    {
        {7, 6, 0},   {7, 5, 7},  {7, 5, 0},  {0, 5, 5},
        {6, 5, 7},   {6, 5, 5},  {6, 7, 6},  {7, 5, 7},
        {7, 0, 0},   {6, 7, 0},  {6, 4, 5},  {0, 5, 5},
        {6, 4, 5},   {5, 5, 6},  {4, 1, 7},  {5, 5, 5}
    },

    /* decision_tree_threshold[16][3] */
    {
        { 11,   16,    0}, {  27,  82,   1}, {    27,   82,   0}, { 596, 175, 495},
        {  3,  532,   11}, {  10, 556,  82}, {    16,   11,  35}, {   6, 572,  27},
        { 17,  692, 1343}, {  16,  11, 741}, {     3, 4417, 399}, { 500, 483, 443},
        {  3, 4764,  291}, { 175,  18, 106}, { 11866,    1, 198}, { 163, 163, 734}
    },

    /* decision_tree_weight[16][4] */
    {
        {             917,  (uint32_t)-917, (uint32_t)-917,   (uint32_t)-917},
        { (uint32_t)-5120,            5120,           5120,  (uint32_t)-5120},
        {            4734, (uint32_t)-4734,           4734,             4734},
        {  (uint32_t)-221,             221, (uint32_t)-221,              221},
        {             171,  (uint32_t)-171, (uint32_t)-171,              171},
        {             166,  (uint32_t)-166,            166,   (uint32_t)-166},
        {  (uint32_t)-195,             195,            195,   (uint32_t)-195},
        {             137,  (uint32_t)-137, (uint32_t)-137,              137},
        {             114,  (uint32_t)-114, (uint32_t)-114,              114},
        {  (uint32_t)-129,             129,            129,   (uint32_t)-129},
        {  (uint32_t)-123,             123, (uint32_t)-123,              123},
        {             130,  (uint32_t)-130, (uint32_t)-130,              130},
        {   (uint32_t)-72,              72,  (uint32_t)-72,               72},
        {   (uint32_t)-75,              75,  (uint32_t)-75,               75},
        {   (uint32_t)-77,              77,  (uint32_t)-77,               77},
        {(uint32_t)-16384,               0,              0, (uint32_t)-16384}
    }
};


void tivx_dmpac_dof_cs_tree_params_init(tivx_dmpac_dof_cs_tree_params_t *prms)
{
    if (NULL != prms)
    {
        memcpy(prms, &cs_tree_init_values, sizeof(tivx_dmpac_dof_cs_tree_params_t));
    }
}

void tivx_dmpac_dof_hts_bw_limit_params_init(
                                    tivx_dmpac_dof_hts_bw_limit_params_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_dmpac_dof_hts_bw_limit_params_t));
    }
}

#endif /* BUILD_DMPAC_DOF */