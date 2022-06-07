/*
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
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

#ifdef BUILD_DISPLAY

#include "TI/tivx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_display.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_hwa_host_priv.h"

static vx_kernel vx_display_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelDisplayValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelDisplayInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelDisplayValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_user_data_object configuration = NULL;
    vx_char configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size configuration_size;

    vx_image in_image = NULL;
    vx_df_image in_image_fmt;
    vx_uint32 in_image_w, in_image_h;

    if((num != TIVX_KERNEL_DISPLAY_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_DISPLAY_CONFIGURATION_IDX]))
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_DISPLAY_CONFIGURATION_IDX];
        in_image      = (vx_image)parameters[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX];
    }

    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));

        if (NULL != in_image)
        {
            tivxCheckStatus(&status, vxQueryImage(in_image, (vx_enum)VX_IMAGE_FORMAT, &in_image_fmt, sizeof(in_image_fmt)));
            tivxCheckStatus(&status, vxQueryImage(in_image, (vx_enum)VX_IMAGE_WIDTH, &in_image_w, sizeof(in_image_w)));
            tivxCheckStatus(&status, vxQueryImage(in_image, (vx_enum)VX_IMAGE_HEIGHT, &in_image_h, sizeof(in_image_h)));
        }
    }

    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((configuration_size != sizeof(tivx_display_params_t)) ||
            (strncmp(configuration_name, "tivx_display_params_t", sizeof(configuration_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be an user_data_object of type:\n tivx_display_params_t \n");
        }

        if (NULL != in_image)
        {
            if( ((vx_df_image)VX_DF_IMAGE_RGB != in_image_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_RGBX != in_image_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_BGRX != in_image_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_UYVY != in_image_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_YUYV != in_image_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_NV12 != in_image_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_U16 != in_image_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_U8 != in_image_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_RGB565 != in_image_fmt) )
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'in_image' should be an image of type:\n VX_DF_IMAGE_RGB or VX_DF_IMAGE_RGBX or TIVX_DF_IMAGE_BGRX or VX_DF_IMAGE_UYVY or VX_DF_IMAGE_NV12 or TIVX_DF_IMAGE_RGB565 or TIVX_DF_IMAGE_U16 or TIVX_DF_IMAGE_U8\n");
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelDisplayInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ((num_params != TIVX_KERNEL_DISPLAY_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_DISPLAY_CONFIGURATION_IDX]))
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        if(NULL != parameters[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX])
        {
            prms.in_img[0U] = (vx_image)parameters[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX];
        }
        else
        {
            /* Do nothing */
        }

        prms.num_input_images = 1;
        prms.num_output_images = 0;

        status = tivxKernelConfigValidRect(&prms);
    }

    return status;
}

vx_status tivxAddKernelDisplay(vx_context context)
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
                    TIVX_KERNEL_DISPLAY_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_DISPLAY_MAX_PARAMS,
                    tivxAddKernelDisplayValidate,
                    tivxAddKernelDisplayInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    if (status == VX_SUCCESS)
    {
        tivxSetKernelSinkDepth(kernel, 2);

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
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_DISPLAY1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_DISPLAY2);
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
    vx_display_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelDisplay(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_display_kernel;

    status = vxRemoveKernel(kernel);
    vx_display_kernel = NULL;

    return status;
}

#endif /* BUILD_DISPLAY */