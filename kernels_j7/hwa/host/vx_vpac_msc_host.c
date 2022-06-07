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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef BUILD_VPAC_MSC

#include "TI/tivx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_vpac_msc.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_hwa_host_priv.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacMscScaleValidate(vx_node node,
    const vx_reference parameters[ ], vx_uint32 num, vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxVpacMscScaleInitialize(vx_node node,
    const vx_reference parameters[ ], vx_uint32 num_params);
static vx_status VX_CALLBACK tivxVpacMscPyramidValidate(vx_node node,
    const vx_reference parameters[ ], vx_uint32 num, vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxVpacMscPyramidInitialize(vx_node node,
    const vx_reference parameters[ ], vx_uint32 num_params);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static vx_kernel vx_vpac_msc_scale_kernel = NULL;
static vx_kernel vx_vpac_msc_pyramid_kernel = NULL;

static uint32_t gmsc_32_phase_gaussian_filter[] =
{
    #include "msc_32_phase_gaussian_filter.txt"
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

vx_status tivxAddKernelVpacMscMultiScale(vx_context context)
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
                    TIVX_KERNEL_VPAC_MSC_MULTI_SCALE_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_VPAC_MSC_SCALE_MAX_PARAMS,
                    tivxVpacMscScaleValidate,
                    tivxVpacMscScaleInitialize,
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
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_MSC1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_MSC2);
            #if defined(SOC_J784S4)
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC2_MSC1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC2_MSC2);
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
    vx_vpac_msc_scale_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelVpacMscMultiScale(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_vpac_msc_scale_kernel;

    status = vxRemoveKernel(kernel);
    vx_vpac_msc_scale_kernel = NULL;

    return status;
}


vx_status tivxAddKernelVpacMscPyramid(vx_context context)
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
                    TIVX_KERNEL_VPAC_MSC_PYRAMID_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PARAMS,
                    tivxVpacMscPyramidValidate,
                    tivxVpacMscPyramidInitialize,
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
                        (vx_enum)VX_TYPE_PYRAMID,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_MSC1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_MSC2);
            #if defined(SOC_J784S4)
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC2_MSC1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC2_MSC2);
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
    vx_vpac_msc_pyramid_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelVpacMscPyramid(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_vpac_msc_pyramid_kernel;

    status = vxRemoveKernel(kernel);
    vx_vpac_msc_pyramid_kernel = NULL;

    return status;
}

void tivx_vpac_msc_coefficients_params_init(
    tivx_vpac_msc_coefficients_t *coeff,
    vx_enum interpolation)
{
    uint32_t i;
    uint32_t idx;
    uint32_t weight;

    /* Passthrough if using single phase */
    idx = 0;
    coeff->single_phase[0][idx++] = 0;
    coeff->single_phase[0][idx++] = 0;
    coeff->single_phase[0][idx++] = 256;
    coeff->single_phase[0][idx++] = 0;
    coeff->single_phase[0][idx++] = 0;
    idx = 0;
    coeff->single_phase[1][idx++] = 0;
    coeff->single_phase[1][idx++] = 0;
    coeff->single_phase[1][idx++] = 256;
    coeff->single_phase[1][idx++] = 0;
    coeff->single_phase[1][idx++] = 0;

    if((vx_enum)VX_INTERPOLATION_BILINEAR == interpolation)
    {
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            weight = i<<2;
            coeff->multi_phase[0][idx++] = 0;
            coeff->multi_phase[0][idx++] = 0;
            coeff->multi_phase[0][idx++] = 256-(int32_t)weight;
            coeff->multi_phase[0][idx++] = (int32_t)weight;
            coeff->multi_phase[0][idx++] = 0;
        }
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            weight = (i+32u)<<2;
            coeff->multi_phase[1][idx++] = 0;
            coeff->multi_phase[1][idx++] = 0;
            coeff->multi_phase[1][idx++] = 256-(int32_t)weight;
            coeff->multi_phase[1][idx++] = (int32_t)weight;
            coeff->multi_phase[1][idx++] = 0;
        }
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            weight = i<<2;
            coeff->multi_phase[2][idx++] = 0;
            coeff->multi_phase[2][idx++] = 0;
            coeff->multi_phase[2][idx++] = 256-(int32_t)weight;
            coeff->multi_phase[2][idx++] = (int32_t)weight;
            coeff->multi_phase[2][idx++] = 0;
        }
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            weight = (i+32u)<<2;
            coeff->multi_phase[3][idx++] = 0;
            coeff->multi_phase[3][idx++] = 0;
            coeff->multi_phase[3][idx++] = 256-(int32_t)weight;
            coeff->multi_phase[3][idx++] = (int32_t)weight;
            coeff->multi_phase[3][idx++] = 0;
        }
    }
    else if((vx_enum)VX_INTERPOLATION_NEAREST_NEIGHBOR == interpolation)
    {
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            coeff->multi_phase[0][idx++] = 0;
            coeff->multi_phase[0][idx++] = 0;
            coeff->multi_phase[0][idx++] = 256;
            coeff->multi_phase[0][idx++] = 0;
            coeff->multi_phase[0][idx++] = 0;
        }
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            coeff->multi_phase[1][idx++] = 0;
            coeff->multi_phase[1][idx++] = 0;
            coeff->multi_phase[1][idx++] = 0;
            coeff->multi_phase[1][idx++] = 256;
            coeff->multi_phase[1][idx++] = 0;
        }
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            coeff->multi_phase[2][idx++] = 0;
            coeff->multi_phase[2][idx++] = 0;
            coeff->multi_phase[2][idx++] = 256;
            coeff->multi_phase[2][idx++] = 0;
            coeff->multi_phase[2][idx++] = 0;
        }
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            coeff->multi_phase[3][idx++] = 0;
            coeff->multi_phase[3][idx++] = 0;
            coeff->multi_phase[3][idx++] = 0;
            coeff->multi_phase[3][idx++] = 256;
            coeff->multi_phase[3][idx++] = 0;
        }
    }
    else if((vx_enum)TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE == interpolation)
    {
        for(i = 0; i < 2u; i++)
        {
            idx = 0;
            coeff->single_phase[i][idx++] = 16;
            coeff->single_phase[i][idx++] = 64;
            coeff->single_phase[i][idx++] = 96;
            coeff->single_phase[i][idx++] = 64;
            coeff->single_phase[i][idx++] = 16;
        }
        for(i = 0; i < (32u*5u); i++)
        {
            coeff->multi_phase[0][i] = gmsc_32_phase_gaussian_filter[i];
            coeff->multi_phase[1][i] = gmsc_32_phase_gaussian_filter[i];
            coeff->multi_phase[2][i] = gmsc_32_phase_gaussian_filter[i];
            coeff->multi_phase[3][i] = gmsc_32_phase_gaussian_filter[i];
        }
    }
    else
    {
        /* Do nothing */
    }
}

void tivx_vpac_msc_input_params_init(tivx_vpac_msc_input_params_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_vpac_msc_input_params_t));

        prms->kern_sz = 5u;
        prms->src_ln_inc_2 = 0u;
        prms->input_align_12bit = 0u;
    }
}

void tivx_vpac_msc_output_params_init(tivx_vpac_msc_output_params_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_vpac_msc_output_params_t));

        prms->signed_data = 0u;
        prms->filter_mode = 1u;
        prms->coef_shift = 8u;
        prms->saturation_mode = 0u;
        prms->offset_x = TIVX_VPAC_MSC_AUTOCOMPUTE;
        prms->offset_y = TIVX_VPAC_MSC_AUTOCOMPUTE;
        prms->output_align_12bit = 0u;

        prms->single_phase.horz_coef_src = 0u;
        prms->single_phase.horz_coef_sel = 0u;
        prms->single_phase.vert_coef_src = 0u;
        prms->single_phase.vert_coef_sel = 0u;

        prms->multi_phase.phase_mode = 0u;
        prms->multi_phase.horz_coef_sel = 0u;
        prms->multi_phase.vert_coef_sel = 0u;
        prms->multi_phase.init_phase_x = TIVX_VPAC_MSC_AUTOCOMPUTE;
        prms->multi_phase.init_phase_y = TIVX_VPAC_MSC_AUTOCOMPUTE;
    }
}

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacMscScaleValidate(vx_node node,
    const vx_reference parameters[ ], vx_uint32 num_params, vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_uint32 cnt;

    vx_image in_img = NULL;
    vx_df_image in_img_fmt;
    vx_uint32 in_img_w, in_img_h;

    vx_image out_img[TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT] = {NULL};
    vx_df_image out_img_fmt[TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT] = {VX_DF_IMAGE('0','0','0','0'),
                                                                      VX_DF_IMAGE('0','0','0','0'),
                                                                      VX_DF_IMAGE('0','0','0','0'),
                                                                      VX_DF_IMAGE('0','0','0','0'),
                                                                      VX_DF_IMAGE('0','0','0','0')};
    vx_uint32 out_img_w[TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT] = {0};
    vx_uint32 out_img_h[TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT] = {0};

    if ((num_params != TIVX_KERNEL_VPAC_MSC_SCALE_MAX_PARAMS) ||
        (NULL == parameters[TIVX_KERNEL_VPAC_MSC_SCALE_IN_IMG_IDX]) ||
        (NULL == parameters[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX]))
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR,
            "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        in_img = (vx_image)parameters[TIVX_KERNEL_VPAC_MSC_SCALE_IN_IMG_IDX];

        for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
        {
            out_img[cnt] = (vx_image)parameters
                [TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX + cnt];
        }
    }

    /* PARAMETER ATTRIBUTE FETCH */
    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(in_img, (vx_enum)VX_IMAGE_FORMAT,
            &in_img_fmt, sizeof(in_img_fmt)));
        tivxCheckStatus(&status, vxQueryImage(in_img, (vx_enum)VX_IMAGE_WIDTH,
            &in_img_w, sizeof(in_img_w)));
        tivxCheckStatus(&status, vxQueryImage(in_img, (vx_enum)VX_IMAGE_HEIGHT,
            &in_img_h, sizeof(in_img_h)));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
        {
            if (NULL != out_img[cnt])
            {
                tivxCheckStatus(&status, vxQueryImage(out_img[cnt],
                    (vx_enum)VX_IMAGE_FORMAT, &out_img_fmt[cnt],
                    sizeof(out_img_fmt[cnt])));
                tivxCheckStatus(&status, vxQueryImage(out_img[cnt],
                    (vx_enum)VX_IMAGE_WIDTH, &out_img_w[cnt], sizeof(out_img_w[cnt])));
                tivxCheckStatus(&status, vxQueryImage(out_img[cnt],
                    (vx_enum)VX_IMAGE_HEIGHT, &out_img_h[cnt], sizeof(out_img_h[cnt])));
            }
            if ((vx_status)VX_SUCCESS != status)
            {
                break;
            }
        }
    }

    if (NULL != in_img)
    {
        if( ((vx_df_image)VX_DF_IMAGE_NV12 != in_img_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U8 != in_img_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U16 != in_img_fmt) &&
            ((vx_df_image)TIVX_DF_IMAGE_P12 != in_img_fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Incorrect in_img fmt \n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        for (cnt = 0u; (cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT) && (status == (vx_status)VX_SUCCESS); cnt ++)
        {
            if (NULL == out_img[cnt])
            {
                /* all images must be continuous */
                break;
            }
            else
            {
                if (((vx_df_image)VX_DF_IMAGE_NV12 != out_img_fmt[cnt]) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 != out_img_fmt[cnt]) &&
                    ((vx_df_image)VX_DF_IMAGE_U16 != out_img_fmt[cnt]) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 != out_img_fmt[cnt]))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' should be an image of type:\n VX_DF_IMAGE_NV12 or VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12\n", cnt);
                }

                if (((vx_df_image)VX_DF_IMAGE_NV12 == out_img_fmt[cnt]) &&
                    ((vx_df_image)VX_DF_IMAGE_NV12 != in_img_fmt))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' should be an image of type:\n VX_DF_IMAGE_NV12 when in_img is VX_DF_IMAGE_NV12\n", cnt);
                }

                if ((out_img_w[cnt] > in_img_w))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' width should not be larger than the input width\n", cnt);
                }

                if ((out_img_h[cnt] > in_img_h))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' height should not be larger than the input height\n", cnt);
                }

                if (cnt > 0u)
                {
                    if (out_img_fmt[cnt] != out_img_fmt[cnt-1u])
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' format should be same as 'out_img[%d]' format\n", cnt, cnt-1u);
                        VX_PRINT(VX_ZONE_ERROR, "Format must be same for all outputs\n");
                    }
                }
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacMscScaleInitialize(vx_node node,
    const vx_reference parameters[ ], vx_uint32 num_params)
{
    return (vx_status)VX_SUCCESS;
}

static vx_status VX_CALLBACK tivxVpacMscPyramidValidate(vx_node node,
    const vx_reference parameters[ ], vx_uint32 num, vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_image img;
    vx_pyramid pmd;
    vx_uint32 w, h, i;
    vx_uint32 p_w, p_h;
    vx_size num_levels;
    vx_df_image fmt, p_fmt;
    vx_float32 scale;
    vx_border_t border;

    /* Check only half scale pyramid is supported */
    /* Check NV12, U8, U16 and P12 data format support for input and output */
    /* Check to see all pyramid levels have same format */
    /* Atleast for the first level, scaling factor cannot be less than 1/4x */

    for (i = 0U; i < TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PARAMS; i ++)
    {
        /* Check for NULL */
        if (NULL == parameters[i])
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
            break;
        }
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        img = (vx_image)parameters[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX];
        pmd = (vx_pyramid)parameters[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX];

        /* Get the image width/heigh and format */
        tivxCheckStatus(&status, vxQueryImage(img, (vx_enum)VX_IMAGE_FORMAT,
            &fmt, sizeof(fmt)));
        tivxCheckStatus(&status, vxQueryImage(img, (vx_enum)VX_IMAGE_WIDTH,
            &w, sizeof(w)));
        tivxCheckStatus(&status, vxQueryImage(img, (vx_enum)VX_IMAGE_HEIGHT,
            &h, sizeof(h)));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Check for validity of data format */
        if( ((vx_df_image)VX_DF_IMAGE_NV12 != fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U8 != fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U16 != fmt) &&
            ((vx_df_image)TIVX_DF_IMAGE_P12 != fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Input Image Format \n");
        }
    }

    if (((vx_status)VX_SUCCESS == status) &&
        ((vx_bool)vx_false_e == tivxIsReferenceVirtual((vx_reference)pmd)))
    {
        tivxCheckStatus(&status, vxQueryPyramid(pmd, (vx_enum)VX_PYRAMID_WIDTH, &p_w, sizeof(p_w)));
        tivxCheckStatus(&status, vxQueryPyramid(pmd, (vx_enum)VX_PYRAMID_HEIGHT, &p_h, sizeof(p_h)));
        tivxCheckStatus(&status, vxQueryPyramid(pmd, (vx_enum)VX_PYRAMID_FORMAT, &p_fmt, sizeof(p_fmt)));

        /* Check for frame sizes */
        if (((w / TIVX_VPAC_MSC_MAX_DS_FACTOR) > p_w) ||
            ((h / TIVX_VPAC_MSC_MAX_DS_FACTOR) > p_h))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Scale out of range [1.0 downto 0.25]\n");
        }

        /* Check for validity of data format */
        if( ((vx_df_image)VX_DF_IMAGE_NV12 != p_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U8 != p_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U16 != p_fmt) &&
            ((vx_df_image)TIVX_DF_IMAGE_P12 != p_fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Pyramid Format\n");
        }

        if (((vx_df_image)VX_DF_IMAGE_NV12 == p_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_NV12 != fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Pyramid Format MisMatch\n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryPyramid(pmd, (vx_enum)VX_PYRAMID_SCALE, &scale, sizeof(scale)));
        tivxCheckStatus(&status, vxQueryPyramid(pmd, (vx_enum)VX_PYRAMID_LEVELS, &num_levels, sizeof(num_levels)));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = vxQueryNode(node, (vx_enum)VX_NODE_BORDER, &border, sizeof(border));
        if ((vx_status)VX_SUCCESS == status)
        {
            if ((border.mode != (vx_enum)VX_BORDER_REPLICATE) &&
                (border.mode != (vx_enum)VX_BORDER_UNDEFINED))
            {
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                VX_PRINT(VX_ZONE_ERROR, "Only replicate/Undefined border mode is supported \n");
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != metas[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX],
                (vx_enum)VX_PYRAMID_WIDTH, &p_w, sizeof(p_w));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX],
                (vx_enum)VX_PYRAMID_HEIGHT, &p_h, sizeof(p_h));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX],
                (vx_enum)VX_PYRAMID_FORMAT, &p_fmt, sizeof(p_fmt));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX],
                (vx_enum)VX_PYRAMID_LEVELS, &num_levels, sizeof(num_levels));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX],
                (vx_enum)VX_PYRAMID_SCALE, &scale, sizeof(scale));
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxVpacMscPyramidInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (num_params != TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PARAMS)
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}


/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */

#endif