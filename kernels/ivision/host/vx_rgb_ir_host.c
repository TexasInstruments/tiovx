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
#include <TI/tivx_target_kernel.h>
#include <tivx_kernel_rgb_ir.h>

static vx_kernel vx_rgb_ir_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelRgbIrValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_image img;
    vx_uint32 w, h, i;
    vx_df_image fmt;
    vx_scalar scalar;
    vx_enum stype;
    vx_uint8 intVal;
    vx_float32 value;

    for (i = 0U; i < TIVX_KERNEL_RGB_IR_MAX_PARAMS; i ++)
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
        img = (vx_image)parameters[TIVX_KERNEL_RGB_IR_IN_IMG_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(img, (vx_enum)VX_IMAGE_FORMAT, &fmt,
            sizeof(fmt));

        status |= vxQueryImage(img, (vx_enum)VX_IMAGE_WIDTH, &w, sizeof(w));
        status |= vxQueryImage(img, (vx_enum)VX_IMAGE_HEIGHT, &h, sizeof(h));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Check for validity of data format */
        if ((vx_df_image)VX_DF_IMAGE_U16 != fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        scalar = (vx_scalar)parameters[TIVX_KERNEL_RGB_IR_SENSOR_PHASE_IDX];

        status = vxQueryScalar(scalar, (vx_enum)VX_SCALAR_TYPE, &stype,
            sizeof(stype));
        if ((vx_status)VX_SUCCESS == status)
        {
            if (stype != (vx_enum)VX_TYPE_UINT8)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        }
        else
        {
            status = vxCopyScalar(scalar, &intVal,
                (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST);

            if (status == (vx_status)VX_SUCCESS)
            {
                if (0 != intVal)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        scalar = (vx_scalar)parameters[TIVX_KERNEL_RGB_IR_THR_IDX];

        status = vxQueryScalar(scalar, (vx_enum)VX_SCALAR_TYPE, &stype,
            sizeof(stype));
        if ((vx_status)VX_SUCCESS == status)
        {
            if (stype != (vx_enum)VX_TYPE_UINT16)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        scalar = (vx_scalar)parameters[
                                       TIVX_KERNEL_RGB_IR_ALPHA_R_IDX];

        status = vxQueryScalar(scalar, (vx_enum)VX_SCALAR_TYPE, &stype,
            sizeof(stype));
        if ((vx_status)VX_SUCCESS == status)
        {
            if (stype != (vx_enum)VX_TYPE_FLOAT32)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            else
            {
                status = vxCopyScalar(scalar, &value,
                    (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST);

                if (status == (vx_status)VX_SUCCESS)
                {
                    if (1.0f < value)
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                }
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        scalar = (vx_scalar)parameters[
                                       TIVX_KERNEL_RGB_IR_ALPHA_G_IDX];

        status = vxQueryScalar(scalar, (vx_enum)VX_SCALAR_TYPE, &stype,
            sizeof(stype));
        if ((vx_status)VX_SUCCESS == status)
        {
            if (stype != (vx_enum)VX_TYPE_FLOAT32)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            else
            {
                status = vxCopyScalar(scalar, &value,
                    (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST);

                if (status == (vx_status)VX_SUCCESS)
                {
                    if (1.0f < value)
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                }
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        scalar = (vx_scalar)parameters[
                                       TIVX_KERNEL_RGB_IR_ALPHA_B_IDX];

        status = vxQueryScalar(scalar, (vx_enum)VX_SCALAR_TYPE, &stype,
            sizeof(stype));
        if ((vx_status)VX_SUCCESS == status)
        {
            if (stype != (vx_enum)VX_TYPE_FLOAT32)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            else
            {
                status = vxCopyScalar(scalar, &value,
                    (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST);

                if (status == (vx_status)VX_SUCCESS)
                {
                    if (1.0f < value)
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                }
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        scalar = (vx_scalar)parameters[TIVX_KERNEL_RGB_IR_BORDER_MODE_IDX];

        status = vxQueryScalar(scalar, (vx_enum)VX_SCALAR_TYPE, &stype,
            sizeof(stype));
        if ((vx_status)VX_SUCCESS == status)
        {
            if (stype != (vx_enum)VX_TYPE_UINT8)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        }
        else
        {
            status = vxCopyScalar(scalar, &intVal,
                (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST);

            if (status == (vx_status)VX_SUCCESS)
            {
                if (intVal > 2)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status &&
        ((vx_bool)vx_false_e == tivxIsReferenceVirtual(
            (vx_reference)parameters[TIVX_KERNEL_RGB_IR_OUT_BAYER_IDX])))
    {
        img = (vx_image)parameters[TIVX_KERNEL_RGB_IR_OUT_BAYER_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(img, (vx_enum)VX_IMAGE_FORMAT, &fmt,
            sizeof(fmt));

        status |= vxQueryImage(img, (vx_enum)VX_IMAGE_WIDTH, &w, sizeof(w));
        status |= vxQueryImage(img, (vx_enum)VX_IMAGE_HEIGHT, &h, sizeof(h));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Check for validity of data format */
        if ((vx_df_image)VX_DF_IMAGE_U16 != fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status &&
        ((vx_bool)vx_false_e == tivxIsReferenceVirtual(
            (vx_reference)parameters[TIVX_KERNEL_RGB_IR_OUT_IR_IDX])))
    {
        img = (vx_image)parameters[TIVX_KERNEL_RGB_IR_OUT_IR_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(img, (vx_enum)VX_IMAGE_FORMAT, &fmt,
            sizeof(fmt));

        status |= vxQueryImage(img, (vx_enum)VX_IMAGE_WIDTH, &w, sizeof(w));
        status |= vxQueryImage(img, (vx_enum)VX_IMAGE_HEIGHT, &h, sizeof(h));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Check for validity of data format */
        if ((vx_df_image)VX_DF_IMAGE_U16 != fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    return status;
}

vx_status tivxAddIVisionKernelRgbIr(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "com.ti.ivision.rgb_ir",
                            TIVX_KERNEL_IVISION_RGB_IR,
                            NULL,
                            TIVX_KERNEL_RGB_IR_MAX_PARAMS,
                            tivxAddKernelRgbIrValidate,
                            NULL,
                            NULL);

    status = vxGetStatus((vx_reference)kernel);

    if ( status == (vx_status)VX_SUCCESS)
    {
        index = 0;

        if ( status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                (vx_enum)VX_INPUT,
                (vx_enum)VX_TYPE_IMAGE,
                (vx_enum)VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                (vx_enum)VX_INPUT,
                (vx_enum)VX_TYPE_SCALAR,
                (vx_enum)VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                (vx_enum)VX_INPUT,
                (vx_enum)VX_TYPE_SCALAR,
                (vx_enum)VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                (vx_enum)VX_INPUT,
                (vx_enum)VX_TYPE_SCALAR,
                (vx_enum)VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                (vx_enum)VX_INPUT,
                (vx_enum)VX_TYPE_SCALAR,
                (vx_enum)VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                (vx_enum)VX_INPUT,
                (vx_enum)VX_TYPE_SCALAR,
                (vx_enum)VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                (vx_enum)VX_INPUT,
                (vx_enum)VX_TYPE_SCALAR,
                (vx_enum)VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                (vx_enum)VX_OUTPUT,
                (vx_enum)VX_TYPE_IMAGE,
                (vx_enum)VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == (vx_status)VX_SUCCESS)
        {
                    status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
                        );
                    index++;
        }
        if ( status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_EVE1);
        }

        if ( status == (vx_status)VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        if( status != (vx_status)VX_SUCCESS)
        {
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
    }

    vx_rgb_ir_kernel = kernel;

    return status;
}

vx_status tivxRemoveIVisionKernelRgbIr(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_rgb_ir_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_rgb_ir_kernel = NULL;

    return status;
}





