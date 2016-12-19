/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <TI/tivx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_minmaxloc.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_mml_kernel = NULL;

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
                            NULL,
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





