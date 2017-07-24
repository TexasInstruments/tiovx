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
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_minmaxloc.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_mml_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelMmlValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);

static vx_status VX_CALLBACK tivxAddKernelMmlInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelMmlValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img;
    vx_uint32 w, h;
    vx_df_image fmt;
    vx_scalar scalar;
    vx_enum stype;
    vx_array arr;
    vx_size min_size = 0, max_size = 0;

    if ((NULL == parameters[TIVX_KERNEL_MML_IN_IMG_IDX]) ||
        (NULL == parameters[TIVX_KERNEL_MML_OUT_MIN_SC_IDX]) ||
        (NULL == parameters[TIVX_KERNEL_MML_OUT_MAX_SC_IDX]))
    {
        status = VX_ERROR_NO_MEMORY;
    }

    if (VX_SUCCESS == status)
    {
        img = (vx_image)parameters[TIVX_KERNEL_MML_IN_IMG_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(img, VX_IMAGE_FORMAT, &fmt,
            sizeof(fmt));

        status |= vxQueryImage(img, VX_IMAGE_WIDTH, &w, sizeof(w));
        status |= vxQueryImage(img, VX_IMAGE_HEIGHT, &h, sizeof(h));
    }

    if (VX_SUCCESS == status)
    {
        /* Check for validity of data format */
        if ((VX_DF_IMAGE_U8 != fmt) && (VX_DF_IMAGE_S16 != fmt))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        scalar = (vx_scalar)parameters[TIVX_KERNEL_MML_OUT_MIN_SC_IDX];

        status = vxQueryScalar(scalar, VX_SCALAR_TYPE, &stype,
            sizeof(stype));
        if (VX_SUCCESS == status)
        {
            if ((VX_DF_IMAGE_U8 == fmt) && (stype != VX_TYPE_UINT8))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            if ((VX_DF_IMAGE_S16 == fmt) && (stype != VX_TYPE_INT16))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        scalar = (vx_scalar)parameters[TIVX_KERNEL_MML_OUT_MAX_SC_IDX];

        status = vxQueryScalar(scalar, VX_SCALAR_TYPE, &stype,
            sizeof(stype));
        if (VX_SUCCESS == status)
        {
            if ((VX_DF_IMAGE_U8 == fmt) && (stype != VX_TYPE_UINT8))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            if ((VX_DF_IMAGE_S16 == fmt) && (stype != VX_TYPE_INT16))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if ((VX_SUCCESS == status) &&
        (NULL != parameters[TIVX_KERNEL_MML_OUT_MIN_ARR_IDX]))
    {
        arr = (vx_array)parameters[TIVX_KERNEL_MML_OUT_MIN_ARR_IDX];

        status = vxQueryArray(arr, VX_ARRAY_ITEMTYPE, &stype,
            sizeof(stype));
        if (VX_SUCCESS == status)
        {
            if (stype != VX_TYPE_COORDINATES2D)
            {
                status = VX_ERROR_INVALID_TYPE;
            }
            else
            {
                status = vxQueryArray(arr, VX_ARRAY_CAPACITY, &min_size,
                    sizeof(min_size));
                if ((VX_SUCCESS == status) && (min_size < 1))
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
            }
        }
        else
        {
            status = VX_ERROR_INVALID_TYPE;
        }
    }

    if ((VX_SUCCESS == status) &&
        (NULL != parameters[TIVX_KERNEL_MML_OUT_MAX_ARR_IDX]))
    {
        arr = (vx_array)parameters[TIVX_KERNEL_MML_OUT_MAX_ARR_IDX];

        status = vxQueryArray(arr, VX_ARRAY_ITEMTYPE, &stype,
            sizeof(stype));
        if (VX_SUCCESS == status)
        {
            if (stype != VX_TYPE_COORDINATES2D)
            {
                status = VX_ERROR_INVALID_TYPE;
            }
            else
            {
                status = vxQueryArray(arr, VX_ARRAY_CAPACITY, &max_size,
                    sizeof(max_size));
                if ((VX_SUCCESS == status) && (max_size < 1))
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
            }
        }
        else
        {
            status = VX_ERROR_INVALID_TYPE;
        }
    }

    if ((VX_SUCCESS == status) &&
        (NULL != parameters[TIVX_KERNEL_MML_OUT_MIN_SC_C_IDX]))
    {
        scalar = (vx_scalar)parameters[TIVX_KERNEL_MML_OUT_MIN_SC_C_IDX];

        status = vxQueryScalar(scalar, VX_SCALAR_TYPE, &stype,
            sizeof(stype));
        if (VX_SUCCESS == status)
        {
            if (VX_TYPE_UINT32 != stype)
            {
                status = VX_ERROR_INVALID_TYPE;
            }
        }
        else
        {
            status = VX_ERROR_INVALID_TYPE;
        }
    }

    if ((VX_SUCCESS == status) &&
        (NULL != parameters[TIVX_KERNEL_MML_OUT_MAX_SC_C_IDX]))
    {
        scalar = (vx_scalar)parameters[TIVX_KERNEL_MML_OUT_MAX_SC_C_IDX];

        status = vxQueryScalar(scalar, VX_SCALAR_TYPE, &stype,
            sizeof(stype));
        if (VX_SUCCESS == status)
        {
            if (VX_TYPE_UINT32 != stype)
            {
                status = VX_ERROR_INVALID_TYPE;
            }
        }
        else
        {
            status = VX_ERROR_INVALID_TYPE;
        }
    }

    if (VX_SUCCESS == status)
    {
        if (VX_DF_IMAGE_U8 == fmt)
        {
            stype = VX_TYPE_UINT8;
        }
        else
        {
            stype = VX_TYPE_INT16;
        }

        if (NULL != metas[TIVX_KERNEL_MML_OUT_MIN_SC_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MML_OUT_MIN_SC_IDX],
                VX_SCALAR_TYPE, &stype, sizeof(stype));
        }
        if (NULL != metas[TIVX_KERNEL_MML_OUT_MAX_SC_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MML_OUT_MAX_SC_IDX],
                VX_SCALAR_TYPE, &stype, sizeof(stype));
        }

        stype = VX_TYPE_COORDINATES2D;
        if (NULL != metas[TIVX_KERNEL_MML_OUT_MIN_ARR_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MML_OUT_MIN_ARR_IDX],
                VX_ARRAY_ITEMTYPE, &stype, sizeof(stype));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MML_OUT_MIN_ARR_IDX],
                VX_ARRAY_CAPACITY, &min_size, sizeof(min_size));
        }
        if (NULL != metas[TIVX_KERNEL_MML_OUT_MAX_ARR_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MML_OUT_MAX_ARR_IDX],
                VX_ARRAY_ITEMTYPE, &stype, sizeof(stype));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MML_OUT_MAX_ARR_IDX],
                VX_ARRAY_CAPACITY, &max_size, sizeof(max_size));
        }

        stype = VX_TYPE_UINT32;
        if (NULL != metas[TIVX_KERNEL_MML_OUT_MIN_SC_C_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MML_OUT_MIN_SC_C_IDX],
                VX_SCALAR_TYPE, &stype, sizeof(stype));
        }
        if (NULL != metas[TIVX_KERNEL_MML_OUT_MAX_SC_C_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MML_OUT_MAX_SC_C_IDX],
                VX_SCALAR_TYPE, &stype, sizeof(stype));
        }
    }
    return status;
}

static vx_status VX_CALLBACK tivxAddKernelMmlInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    tivxKernelValidRectParams prms;

    /* Check for NULL */
    if (NULL == parameters[TIVX_KERNEL_MML_IN_IMG_IDX])
    {
        status = VX_ERROR_NO_MEMORY;
    }

    if (VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0] = (vx_image)parameters[TIVX_KERNEL_MML_IN_IMG_IDX];

        prms.num_input_images = 1;
        prms.num_output_images = 0;

        prms.top_pad = 0;
        prms.bot_pad = 0;
        prms.left_pad = 0;
        prms.right_pad = 0;
        prms.border_mode = VX_BORDER_UNDEFINED;

        status = tivxKernelConfigValidRect(&prms);
    }

    return status;
}


vx_status tivxAddKernelMinMaxLoc(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.minmaxloc",
                            VX_KERNEL_MINMAXLOC,
                            NULL,
                            7,
                            tivxAddKernelMmlValidate,
                            tivxAddKernelMmlInitialize,
                            NULL);

    status = vxGetStatus((vx_reference)kernel);

    if ( status == VX_SUCCESS)
    {
        index = 0;

        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_INPUT,
                VX_TYPE_IMAGE,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_OUTPUT,
                VX_TYPE_SCALAR,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_OUTPUT,
                VX_TYPE_SCALAR,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_OUTPUT,
                VX_TYPE_ARRAY,
                VX_PARAMETER_STATE_OPTIONAL
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_OUTPUT,
                VX_TYPE_ARRAY,
                VX_PARAMETER_STATE_OPTIONAL
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_OUTPUT,
                VX_TYPE_SCALAR,
                VX_PARAMETER_STATE_OPTIONAL
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_OUTPUT,
                VX_TYPE_SCALAR,
                VX_PARAMETER_STATE_OPTIONAL
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP2);
        }

        if ( status == VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        if( status != VX_SUCCESS)
        {
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
    }

    vx_mml_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelMinMaxLoc(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_mml_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_mml_kernel = NULL;

    return status;
}





