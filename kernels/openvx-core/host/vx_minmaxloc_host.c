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
#include <tivx_kernel_minmaxloc.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_core_host_priv.h"

static vx_kernel vx_min_max_loc_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelMinMaxLocValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelMinMaxLocInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelMinMaxLocValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_image input = NULL;
    vx_uint32 input_w;
    vx_uint32 input_h;
    vx_df_image input_fmt;

    vx_scalar minval = NULL;
    vx_enum minval_scalar_type;

    vx_scalar maxval = NULL;
    vx_enum maxval_scalar_type;

    vx_array minloc = NULL;
    vx_enum minloc_item_type;
    vx_size minloc_capacity;

    vx_array maxloc = NULL;
    vx_enum maxloc_item_type;
    vx_size maxloc_capacity;

    vx_scalar mincount = NULL;
    vx_enum mincount_scalar_type;

    vx_scalar maxcount = NULL;
    vx_enum maxcount_scalar_type;

    if ( (num != TIVX_KERNEL_MIN_MAX_LOC_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_MIN_MAX_LOC_INPUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_MIN_MAX_LOC_MINVAL_IDX])
        || (NULL == parameters[TIVX_KERNEL_MIN_MAX_LOC_MAXVAL_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        input = (vx_image)parameters[TIVX_KERNEL_MIN_MAX_LOC_INPUT_IDX];
        minval = (vx_scalar)parameters[TIVX_KERNEL_MIN_MAX_LOC_MINVAL_IDX];
        maxval = (vx_scalar)parameters[TIVX_KERNEL_MIN_MAX_LOC_MAXVAL_IDX];
        minloc = (vx_array)parameters[TIVX_KERNEL_MIN_MAX_LOC_MINLOC_IDX];
        maxloc = (vx_array)parameters[TIVX_KERNEL_MIN_MAX_LOC_MAXLOC_IDX];
        mincount = (vx_scalar)parameters[TIVX_KERNEL_MIN_MAX_LOC_MINCOUNT_IDX];
        maxcount = (vx_scalar)parameters[TIVX_KERNEL_MIN_MAX_LOC_MAXCOUNT_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_FORMAT, &input_fmt, sizeof(input_fmt)));
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_WIDTH, &input_w, sizeof(input_w)));
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_HEIGHT, &input_h, sizeof(input_h)));

        tivxCheckStatus(&status, vxQueryScalar(minval, (vx_enum)VX_SCALAR_TYPE, &minval_scalar_type, sizeof(minval_scalar_type)));

        tivxCheckStatus(&status, vxQueryScalar(maxval, (vx_enum)VX_SCALAR_TYPE, &maxval_scalar_type, sizeof(maxval_scalar_type)));

        if (NULL != minloc)
        {
            tivxCheckStatus(&status, vxQueryArray(minloc, (vx_enum)VX_ARRAY_ITEMTYPE, &minloc_item_type, sizeof(minloc_item_type)));
            tivxCheckStatus(&status, vxQueryArray(minloc, (vx_enum)VX_ARRAY_CAPACITY, &minloc_capacity, sizeof(minloc_capacity)));
        }

        if (NULL != maxloc)
        {
            tivxCheckStatus(&status, vxQueryArray(maxloc, (vx_enum)VX_ARRAY_ITEMTYPE, &maxloc_item_type, sizeof(maxloc_item_type)));
            tivxCheckStatus(&status, vxQueryArray(maxloc, (vx_enum)VX_ARRAY_CAPACITY, &maxloc_capacity, sizeof(maxloc_capacity)));
        }

        if (NULL != mincount)
        {
            tivxCheckStatus(&status, vxQueryScalar(mincount, (vx_enum)VX_SCALAR_TYPE, &mincount_scalar_type, sizeof(mincount_scalar_type)));
        }

        if (NULL != maxcount)
        {
            tivxCheckStatus(&status, vxQueryScalar(maxcount, (vx_enum)VX_SCALAR_TYPE, &maxcount_scalar_type, sizeof(maxcount_scalar_type)));
        }
    }


    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (((vx_df_image)VX_DF_IMAGE_U8 != input_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_S16 != input_fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_S16 \n");
        }

        if (((vx_enum)VX_TYPE_UINT8 != minval_scalar_type) &&
            ((vx_enum)VX_TYPE_INT16 != minval_scalar_type))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'minval' should be a scalar of type:\n VX_TYPE_UINT8 or VX_TYPE_INT16 \n");
        }

        if (((vx_enum)VX_TYPE_UINT8 != maxval_scalar_type) &&
            ((vx_enum)VX_TYPE_INT16 != maxval_scalar_type))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'maxval' should be a scalar of type:\n VX_TYPE_UINT8 or VX_TYPE_INT16 \n");
        }

        if (NULL != minloc)
        {
            if ((vx_enum)VX_TYPE_COORDINATES2D != minloc_item_type)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'minloc' should be an array of type:\n VX_TYPE_COORDINATES2D \n");
            }
        }

        if (NULL != maxloc)
        {
            if ((vx_enum)VX_TYPE_COORDINATES2D != maxloc_item_type)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'maxloc' should be an array of type:\n VX_TYPE_COORDINATES2D \n");
            }
        }

        if (NULL != mincount)
        {
            if ((vx_enum)VX_TYPE_UINT32 != mincount_scalar_type)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'mincount' should be a scalar of type:\n VX_TYPE_UINT32 \n");
            }
        }

        if (NULL != maxcount)
        {
            if ((vx_enum)VX_TYPE_UINT32 != maxcount_scalar_type)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'maxcount' should be a scalar of type:\n VX_TYPE_UINT32 \n");
            }
        }
    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (((vx_df_image)VX_DF_IMAGE_U8 == input_fmt) &&
            (minval_scalar_type != (vx_enum)VX_TYPE_UINT8))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'minval' must be type VX_TYPE_UINT8 if 'input' is type VX_DF_IMAGE_U8 \n");
        }

        if (((vx_df_image)VX_DF_IMAGE_S16 == input_fmt) &&
            (minval_scalar_type != (vx_enum)VX_TYPE_INT16))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'minval' must be type VX_TYPE_INT16 if 'input' is type VX_DF_IMAGE_S16 \n");
        }

        if (((vx_df_image)VX_DF_IMAGE_U8 == input_fmt) &&
            (maxval_scalar_type != (vx_enum)VX_TYPE_UINT8))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'maxval' must be type VX_TYPE_UINT8 if 'input' is type VX_DF_IMAGE_U8 \n");
        }

        if (((vx_df_image)VX_DF_IMAGE_S16 == input_fmt) &&
            (maxval_scalar_type != (vx_enum)VX_TYPE_INT16))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'maxval' must be type VX_TYPE_INT16 if 'input' is type VX_DF_IMAGE_S16 \n");
        }
    }


    /* CUSTOM PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if(NULL != minloc)
        {
            if (1U > minloc_capacity)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'minloc' capacity must be 1 or more \n");
            }
        }

        if(NULL != maxloc)
        {
            if (1U > maxloc_capacity)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'maxloc' capacity must be 1 or more \n");
            }
        }
    }

#if 1

    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != metas[TIVX_KERNEL_MIN_MAX_LOC_MINVAL_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MIN_MAX_LOC_MINVAL_IDX], (vx_enum)VX_SCALAR_TYPE, &minval_scalar_type, sizeof(minval_scalar_type));
        }

        if (NULL != metas[TIVX_KERNEL_MIN_MAX_LOC_MAXVAL_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MIN_MAX_LOC_MAXVAL_IDX], (vx_enum)VX_SCALAR_TYPE, &maxval_scalar_type, sizeof(maxval_scalar_type));
        }

        if (NULL != metas[TIVX_KERNEL_MIN_MAX_LOC_MINLOC_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MIN_MAX_LOC_MINLOC_IDX], (vx_enum)VX_ARRAY_ITEMTYPE, &minloc_item_type, sizeof(minloc_item_type));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MIN_MAX_LOC_MINLOC_IDX], (vx_enum)VX_ARRAY_CAPACITY, &minloc_capacity, sizeof(minloc_capacity));
        }

        if (NULL != metas[TIVX_KERNEL_MIN_MAX_LOC_MAXLOC_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MIN_MAX_LOC_MAXLOC_IDX], (vx_enum)VX_ARRAY_ITEMTYPE, &maxloc_item_type, sizeof(maxloc_item_type));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MIN_MAX_LOC_MAXLOC_IDX], (vx_enum)VX_ARRAY_CAPACITY, &maxloc_capacity, sizeof(maxloc_capacity));
        }

        if (NULL != metas[TIVX_KERNEL_MIN_MAX_LOC_MINCOUNT_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MIN_MAX_LOC_MINCOUNT_IDX], (vx_enum)VX_SCALAR_TYPE, &mincount_scalar_type, sizeof(mincount_scalar_type));
        }

        if (NULL != metas[TIVX_KERNEL_MIN_MAX_LOC_MAXCOUNT_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MIN_MAX_LOC_MAXCOUNT_IDX], (vx_enum)VX_SCALAR_TYPE, &maxcount_scalar_type, sizeof(maxcount_scalar_type));
        }
    }

#endif

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelMinMaxLocInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ( (num_params != TIVX_KERNEL_MIN_MAX_LOC_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_MIN_MAX_LOC_INPUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_MIN_MAX_LOC_MINVAL_IDX])
        || (NULL == parameters[TIVX_KERNEL_MIN_MAX_LOC_MAXVAL_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0U] = (vx_image)parameters[TIVX_KERNEL_MIN_MAX_LOC_INPUT_IDX];

        prms.num_input_images = 1U;
        prms.num_output_images = 0U;

        prms.top_pad = 0U;
        prms.bot_pad = 0U;
        prms.left_pad = 0U;
        prms.right_pad = 0U;
        prms.border_mode = (vx_enum)VX_BORDER_UNDEFINED;

        tivxCheckStatus(&status, tivxKernelConfigValidRect(&prms));
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
                (vx_enum)VX_KERNEL_MINMAXLOC,
                NULL,
                TIVX_KERNEL_MIN_MAX_LOC_MAX_PARAMS,
                tivxAddKernelMinMaxLocValidate,
                tivxAddKernelMinMaxLocInitialize,
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
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_SCALAR,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_SCALAR,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_ARRAY,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_ARRAY,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_SCALAR,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_SCALAR,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
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
    vx_min_max_loc_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelMinMaxLoc(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_min_max_loc_kernel;

    status = vxRemoveKernel(kernel);
    vx_min_max_loc_kernel = NULL;

    return status;
}


