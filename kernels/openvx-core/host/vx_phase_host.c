/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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
#include <tivx_kernel_phase.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_core_host_priv.h"

static vx_kernel vx_phase_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelPhaseValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelPhaseInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelPhaseValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_image grad_x = NULL;
    vx_uint32 grad_x_w;
    vx_uint32 grad_x_h;
    vx_df_image grad_x_fmt;

    vx_image grad_y = NULL;
    vx_uint32 grad_y_w;
    vx_uint32 grad_y_h;
    vx_df_image grad_y_fmt;

    vx_image orientation = NULL;
    vx_uint32 orientation_w;
    vx_uint32 orientation_h;
    vx_df_image orientation_fmt;

    vx_bool is_virtual = (vx_bool)vx_false_e;

    if ( (num != TIVX_KERNEL_PHASE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_PHASE_GRAD_X_IDX])
        || (NULL == parameters[TIVX_KERNEL_PHASE_GRAD_Y_IDX])
        || (NULL == parameters[TIVX_KERNEL_PHASE_ORIENTATION_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        grad_x = (vx_image)parameters[TIVX_KERNEL_PHASE_GRAD_X_IDX];
        grad_y = (vx_image)parameters[TIVX_KERNEL_PHASE_GRAD_Y_IDX];
        orientation = (vx_image)parameters[TIVX_KERNEL_PHASE_ORIENTATION_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(grad_x, (vx_enum)VX_IMAGE_WIDTH, &grad_x_w, sizeof(grad_x_w)));
        tivxCheckStatus(&status, vxQueryImage(grad_x, (vx_enum)VX_IMAGE_HEIGHT, &grad_x_h, sizeof(grad_x_h)));
        tivxCheckStatus(&status, vxQueryImage(grad_x, (vx_enum)VX_IMAGE_FORMAT, &grad_x_fmt, sizeof(grad_x_fmt)));

        tivxCheckStatus(&status, vxQueryImage(grad_y, (vx_enum)VX_IMAGE_WIDTH, &grad_y_w, sizeof(grad_y_w)));
        tivxCheckStatus(&status, vxQueryImage(grad_y, (vx_enum)VX_IMAGE_HEIGHT, &grad_y_h, sizeof(grad_y_h)));
        tivxCheckStatus(&status, vxQueryImage(grad_y, (vx_enum)VX_IMAGE_FORMAT, &grad_y_fmt, sizeof(grad_y_fmt)));

        tivxCheckStatus(&status, vxQueryImage(orientation, (vx_enum)VX_IMAGE_WIDTH, &orientation_w, sizeof(orientation_w)));
        tivxCheckStatus(&status, vxQueryImage(orientation, (vx_enum)VX_IMAGE_HEIGHT, &orientation_h, sizeof(orientation_h)));
        tivxCheckStatus(&status, vxQueryImage(orientation, (vx_enum)VX_IMAGE_FORMAT, &orientation_fmt, sizeof(orientation_fmt)));

#if 1

        is_virtual = tivxIsReferenceVirtual((vx_reference)orientation);

#endif

    }


    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_df_image)VX_DF_IMAGE_S16 != grad_x_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'grad_x' should be an image of type:\n VX_DF_IMAGE_S16 \n");
        }

        if ((vx_df_image)VX_DF_IMAGE_S16 != grad_y_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'grad_y' should be an image of type:\n VX_DF_IMAGE_S16 \n");
        }

        if ((vx_bool)vx_false_e == is_virtual)
        {
            if ((vx_df_image)VX_DF_IMAGE_U8 != orientation_fmt)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'orientation' should be an image of type:\n VX_DF_IMAGE_U8 \n");
            }
        }
    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if((vx_status)VX_SUCCESS == status)
    {
        if (grad_x_w != grad_y_w)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'grad_x' and 'grad_y' should have the same value for VX_IMAGE_WIDTH \n");
        }

        if (grad_x_h != grad_y_h)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'grad_x' and 'grad_y' should have the same value for VX_IMAGE_WIDTH \n");
        }

        if ((vx_bool)vx_false_e == is_virtual)
        {
            if (grad_x_w != orientation_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'grad_x' and 'orientation' should have the same value for VX_IMAGE_WIDTH \n");
            }

            if (grad_x_h != orientation_h)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'grad_x' and 'orientation' should have the same value for VX_IMAGE_HEIGHT \n");
            }
        }
    }

#if 1

    if((vx_status)VX_SUCCESS == status)
    {
        orientation_fmt = (vx_df_image)VX_DF_IMAGE_U8;

        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_PHASE_ORIENTATION_IDX], (vx_enum)VX_IMAGE_FORMAT, &orientation_fmt, sizeof(orientation_fmt));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_PHASE_ORIENTATION_IDX], (vx_enum)VX_IMAGE_WIDTH, &grad_x_w, sizeof(grad_x_w));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_PHASE_ORIENTATION_IDX], (vx_enum)VX_IMAGE_HEIGHT, &grad_x_h, sizeof(grad_x_h));
    }

#endif

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelPhaseInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ( (num_params != TIVX_KERNEL_PHASE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_PHASE_GRAD_X_IDX])
        || (NULL == parameters[TIVX_KERNEL_PHASE_GRAD_Y_IDX])
        || (NULL == parameters[TIVX_KERNEL_PHASE_ORIENTATION_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0U] = (vx_image)parameters[TIVX_KERNEL_PHASE_GRAD_X_IDX];
        prms.in_img[1U] = (vx_image)parameters[TIVX_KERNEL_PHASE_GRAD_Y_IDX];
        prms.out_img[0U] = (vx_image)parameters[TIVX_KERNEL_PHASE_ORIENTATION_IDX];

        prms.num_input_images = 2U;
        prms.num_output_images = 1U;

        prms.top_pad = 0U;
        prms.bot_pad = 0U;
        prms.left_pad = 0U;
        prms.right_pad = 0U;
        prms.border_mode = (vx_enum)VX_BORDER_UNDEFINED;

        tivxCheckStatus(&status, tivxKernelConfigValidRect(&prms));
    }

    return status;
}

vx_status tivxAddKernelPhase(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                context,
                "org.khronos.openvx.phase",
                (vx_enum)VX_KERNEL_PHASE,
                NULL,
                TIVX_KERNEL_PHASE_MAX_PARAMS,
                tivxAddKernelPhaseValidate,
                tivxAddKernelPhaseInitialize,
                NULL);

    status = vxGetStatus((vx_reference)kernel);

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
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxKernelsHostUtilsAddKernelTargetDsp(kernel);
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
    vx_phase_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelPhase(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_phase_kernel;

    status = vxRemoveKernel(kernel);
    vx_phase_kernel = NULL;

    return status;
}


