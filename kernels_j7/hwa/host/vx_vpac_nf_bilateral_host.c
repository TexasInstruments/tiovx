/*
 *
 * Copyright (c) 2017-2018 Texas Instruments Incorporated
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

#ifdef BUILD_VPAC_NF

#include "TI/tivx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_vpac_nf_bilateral.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_hwa_host_priv.h"

static vx_kernel vx_vpac_nf_bilateral_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelVpacNfBilateralValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelVpacNfBilateralInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelVpacNfBilateralValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_user_data_object configuration = NULL;
    vx_char configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size configuration_size;
    tivx_vpac_nf_bilateral_params_t bilateral_prms;

    vx_image input = NULL;
    vx_df_image input_fmt;
    vx_uint32 input_w, input_h;

    vx_user_data_object sigmas = NULL;
    vx_char sigmas_name[VX_MAX_REFERENCE_NAME];
    vx_size sigmas_size;
    tivx_vpac_nf_bilateral_sigmas_t bilateral_sigmas;

    vx_image output = NULL;
    vx_df_image output_fmt;
    vx_uint32 output_w, output_h;

    if ( (num != TIVX_KERNEL_VPAC_NF_BILATERAL_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_VPAC_NF_BILATERAL_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_NF_BILATERAL_INPUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_NF_BILATERAL_SIGMAS_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_NF_BILATERAL_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_VPAC_NF_BILATERAL_CONFIGURATION_IDX];
        input = (vx_image)parameters[TIVX_KERNEL_VPAC_NF_BILATERAL_INPUT_IDX];
        sigmas = (vx_user_data_object)parameters[TIVX_KERNEL_VPAC_NF_BILATERAL_SIGMAS_IDX];
        output = (vx_image)parameters[TIVX_KERNEL_VPAC_NF_BILATERAL_OUTPUT_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));
        tivxCheckStatus(&status, vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_nf_bilateral_params_t), &bilateral_prms, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));

        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_FORMAT, &input_fmt, sizeof(input_fmt)));
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_WIDTH, &input_w, sizeof(input_w)));
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_HEIGHT, &input_h, sizeof(input_h)));

        tivxCheckStatus(&status, vxQueryUserDataObject(sigmas, (vx_enum)VX_USER_DATA_OBJECT_NAME, &sigmas_name, sizeof(sigmas_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(sigmas, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &sigmas_size, sizeof(sigmas_size)));
        tivxCheckStatus(&status, vxCopyUserDataObject(sigmas, 0, sizeof(tivx_vpac_nf_bilateral_sigmas_t), &bilateral_sigmas, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));

        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_FORMAT, &output_fmt, sizeof(output_fmt)));
        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_WIDTH, &output_w, sizeof(output_w)));
        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_HEIGHT, &output_h, sizeof(output_h)));
    }

    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((configuration_size != sizeof(tivx_vpac_nf_bilateral_params_t)) ||
            (strncmp(configuration_name, "tivx_vpac_nf_bilateral_params_t", sizeof(configuration_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be a user_data_object of type:\n tivx_vpac_nf_bilateral_params_t \n");
        }

        if( ((vx_df_image)VX_DF_IMAGE_U8 != input_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U16 != input_fmt) &&
            ((vx_df_image)TIVX_DF_IMAGE_P12 != input_fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
        }

        if ((sigmas_size != sizeof(tivx_vpac_nf_bilateral_sigmas_t)) ||
            (strncmp(sigmas_name, "tivx_vpac_nf_bilateral_sigmas_t", sizeof(sigmas_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'sigmas' should be a user_data_object of type:\n tivx_vpac_nf_bilateral_sigmas_t \n");
        }

        if( ((vx_df_image)VX_DF_IMAGE_U8 != output_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U16 != output_fmt) &&
            ((vx_df_image)TIVX_DF_IMAGE_P12 != output_fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'output' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
        }
    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (input_w != output_w)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'input' and 'output' should have the same value for VX_IMAGE_WIDTH\n");
        }
        if (input_h != output_h)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'input' and 'output' should have the same value for VX_IMAGE_HEIGHT\n");
        }
    }

    /* CUSTOM PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (1U < bilateral_prms.params.input_interleaved)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter params.input_interleaved should be either 0 or 1\n");
        }
        if ((-8 > bilateral_prms.params.output_downshift) ||
            (7 < bilateral_prms.params.output_downshift))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter params.output_downshift should be a value between -8 and 7 inclusive\n");
        }
        if (4095U < bilateral_prms.params.output_offset)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter params.output_offset should be between 0 and 4095 inclusive\n");
        }
        if (1U < bilateral_prms.params.output_pixel_skip)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter params.output_pixel_skip should be either 0 or 1\n");
        }
        if (1U < bilateral_prms.params.output_pixel_skip_odd)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter params.output_pixel_skip_odd should be either 0 or 1\n");
        }
        if (4U < bilateral_prms.params.kern_ln_offset)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter params.kern_ln_offset should be between 0 and 4 inclusive\n");
        }
        if ((1U > bilateral_prms.params.kern_sz_height) ||
            (5U < bilateral_prms.params.kern_sz_height))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter params.kern_sz_height should be a value between 1 and 5 inclusive\n");
        }
        if (1U < bilateral_prms.params.src_ln_inc_2)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter params.src_ln_inc_2 should be either 0 or 1\n");
        }
        if (1U < bilateral_prms.adaptive_mode)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter adaptive_mode should be either 0 or 1\n");
        }
        if (7U < bilateral_prms.sub_table_select)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter sub_table_select should be between 0 and 7 inclusive\n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((1U != bilateral_sigmas.num_sigmas) &&
            (2U != bilateral_sigmas.num_sigmas) &&
            (4U != bilateral_sigmas.num_sigmas) &&
            (8U != bilateral_sigmas.num_sigmas))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter num_sigmas should be either 1, 2, 4, or 8\n");
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelVpacNfBilateralInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ( (num_params != TIVX_KERNEL_VPAC_NF_BILATERAL_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_VPAC_NF_BILATERAL_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_NF_BILATERAL_INPUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_NF_BILATERAL_SIGMAS_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_NF_BILATERAL_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0U] = (vx_image)parameters[TIVX_KERNEL_VPAC_NF_BILATERAL_INPUT_IDX];
        prms.out_img[0U] = (vx_image)parameters[TIVX_KERNEL_VPAC_NF_BILATERAL_OUTPUT_IDX];

        prms.num_input_images = 1;
        prms.num_output_images = 1;

        status = tivxKernelConfigValidRect(&prms);
    }

    return status;
}

vx_status tivxAddKernelVpacNfBilateral(vx_context context)
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
                    TIVX_KERNEL_VPAC_NF_BILATERAL_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_VPAC_NF_BILATERAL_MAX_PARAMS,
                    tivxAddKernelVpacNfBilateralValidate,
                    tivxAddKernelVpacNfBilateralInitialize,
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
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
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
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_NF);
            #if defined(SOC_J784S4)
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC2_NF);
            #endif
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
    vx_vpac_nf_bilateral_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelVpacNfBilateral(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_vpac_nf_bilateral_kernel;

    status = vxRemoveKernel(kernel);
    vx_vpac_nf_bilateral_kernel = NULL;

    return status;
}

void tivx_vpac_nf_bilateral_params_init(tivx_vpac_nf_bilateral_params_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_vpac_nf_bilateral_params_t));
        prms->params.input_interleaved = 0u;
        prms->params.output_downshift = 0;
        prms->params.output_offset = 0u;
        prms->params.output_pixel_skip = 0u;
        prms->params.output_pixel_skip_odd = 0u;
        prms->params.kern_ln_offset = 0u;
        prms->params.kern_sz_height = 5u;
        prms->params.src_ln_inc_2 = 0u;
        prms->adaptive_mode = 0u;
        prms->sub_table_select = 0u;
    }
}

void tivx_vpac_nf_bilateral_sigmas_init(tivx_vpac_nf_bilateral_sigmas_t *sigmas)
{
    uint32_t i;
    if (NULL != sigmas)
    {
        memset(sigmas, 0x0, sizeof(tivx_vpac_nf_bilateral_sigmas_t));

        sigmas->num_sigmas = 1u;

        for(i = 0u; i < 8u; i++)
        {
            sigmas->sigma_space[i] = 1.0f;
            sigmas->sigma_range[i] = 128.0f;
        }
    }
}

#endif