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
#include <tivx_kernel_filter_3x3.h>
#include <TI/tivx_target_kernel.h>

typedef struct
{
    /*! \brief ID of the kernel */
    vx_enum                 kernel_id;

    /*! \brief Name of the kernel */
    vx_char                 kernel_name[VX_MAX_KERNEL_NAME];

    /*! \brief Pointer to process function */
    vx_kernel_validate_f    validate_func;

    /*! \brief Pointer to the kernel Registered */
    vx_kernel               kernel;

    /*! \brief number of cpus on which this kernel is supported,
               valid size of the array cpu_id */
    vx_uint32               num_cpu;

    /*! \brief CPU Id on which kernel is supported */
    vx_char                 *cpu_name[TIVX_CPU_ID_MAX];

} tivxFilter3x3KernelInfo;


static vx_status VX_CALLBACK tivxAddKernelFilt3x3Validate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);


static tivxFilter3x3KernelInfo gTivxFilt3x3KernelInfo[] =
{
    {
        VX_KERNEL_ERODE_3x3,
        "org.khronos.openvx.erode_3x3",
        tivxAddKernelFilt3x3Validate,
        NULL,
        2,
        {TIVX_TARGET_DSP1, TIVX_TARGET_DSP2}
    },
    {
        VX_KERNEL_BOX_3x3,
        "org.khronos.openvx.box_3x3",
        tivxAddKernelFilt3x3Validate,
        NULL,
        2,
        {TIVX_TARGET_DSP1, TIVX_TARGET_DSP2}
    },
    {
        VX_KERNEL_DILATE_3x3,
        "org.khronos.openvx.dilate_3x3",
        tivxAddKernelFilt3x3Validate,
        NULL,
        2,
        {TIVX_TARGET_DSP1, TIVX_TARGET_DSP2}
    },
    {
        VX_KERNEL_GAUSSIAN_3x3,
        "org.khronos.openvx.gaussian_3x3",
        tivxAddKernelFilt3x3Validate,
        NULL,
        2,
        {TIVX_TARGET_DSP1, TIVX_TARGET_DSP2}
    },
    {
        VX_KERNEL_MEDIAN_3x3,
        "org.khronos.openvx.median_3x3",
        tivxAddKernelFilt3x3Validate,
        NULL,
        2,
        {TIVX_TARGET_DSP1, TIVX_TARGET_DSP2}
    },
};

static vx_status VX_CALLBACK tivxAddKernelFilt3x3Validate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_uint32 w[TIVX_KERNEL_FILT3x3_MAX_PARAMS];
    vx_uint32 h[TIVX_KERNEL_FILT3x3_MAX_PARAMS];
    vx_uint32 out_w, out_h, i;
    vx_df_image fmt[TIVX_KERNEL_FILT3x3_MAX_PARAMS], out_fmt;
    vx_image img[TIVX_KERNEL_FILT3x3_MAX_PARAMS];
    vx_border_t border;

    if (num != TIVX_KERNEL_FILT3x3_MAX_PARAMS)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    for (i = 0U; i < TIVX_KERNEL_FILT3x3_MAX_PARAMS; i ++)
    {
        img[i] = (vx_image)parameters[i];

        /* Check for NULL */
        if (NULL == img[i])
        {
            status = VX_ERROR_NO_MEMORY;
            break;
        }
    }
    if (VX_SUCCESS == status)
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[TIVX_KERNEL_FILT3x3_IN_IMG_IDX],
            VX_IMAGE_FORMAT, &fmt[TIVX_KERNEL_FILT3x3_IN_IMG_IDX],
            sizeof(vx_df_image));
        status |= vxQueryImage(img[TIVX_KERNEL_FILT3x3_IN_IMG_IDX],
            VX_IMAGE_WIDTH, &w[TIVX_KERNEL_FILT3x3_IN_IMG_IDX],
            sizeof(vx_uint32));
        status |= vxQueryImage(img[TIVX_KERNEL_FILT3x3_IN_IMG_IDX],
            VX_IMAGE_HEIGHT, &h[TIVX_KERNEL_FILT3x3_IN_IMG_IDX],
            sizeof(vx_uint32));
    }
    if ((VX_SUCCESS == status) &&
        (vx_false_e == tivxIsReferenceVirtual(
            parameters[TIVX_KERNEL_FILT3x3_OUT_IMG_IDX])))
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[TIVX_KERNEL_FILT3x3_OUT_IMG_IDX],
            VX_IMAGE_FORMAT, &fmt[TIVX_KERNEL_FILT3x3_OUT_IMG_IDX],
            sizeof(vx_df_image));
        status |= vxQueryImage(img[TIVX_KERNEL_FILT3x3_OUT_IMG_IDX],
            VX_IMAGE_WIDTH, &w[TIVX_KERNEL_FILT3x3_OUT_IMG_IDX],
            sizeof(vx_uint32));
        status |= vxQueryImage(img[TIVX_KERNEL_FILT3x3_OUT_IMG_IDX],
            VX_IMAGE_HEIGHT, &h[TIVX_KERNEL_FILT3x3_OUT_IMG_IDX],
            sizeof(vx_uint32));
    }

    if (VX_SUCCESS == status)
    {
        if (VX_DF_IMAGE_U8 != fmt[TIVX_KERNEL_FILT3x3_IN_IMG_IDX])
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if (vx_false_e == tivxIsReferenceVirtual(
            parameters[TIVX_KERNEL_FILT3x3_OUT_IMG_IDX]))
        {
            if ((w[TIVX_KERNEL_FILT3x3_IN_IMG_IDX] !=
                 w[TIVX_KERNEL_FILT3x3_OUT_IMG_IDX]) ||
                (h[TIVX_KERNEL_FILT3x3_IN_IMG_IDX] !=
                 h[TIVX_KERNEL_FILT3x3_OUT_IMG_IDX]))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }

            if (fmt[TIVX_KERNEL_FILT3x3_OUT_IMG_IDX] !=
                fmt[TIVX_KERNEL_FILT3x3_IN_IMG_IDX])
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        status = vxQueryNode(node, VX_NODE_BORDER, &border, sizeof(border));
        if (VX_SUCCESS == status)
        {
            if (border.mode != VX_BORDER_UNDEFINED)
            {
                status = VX_ERROR_NOT_SUPPORTED;
                VX_PRINT(VX_ZONE_ERROR, "Only undefined border mode is supported for filtering kernels\n");
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        out_fmt = VX_DF_IMAGE_U8;
        out_w = w[TIVX_KERNEL_FILT3x3_IN_IMG_IDX];
        out_h = h[TIVX_KERNEL_FILT3x3_IN_IMG_IDX];

        tivxKernelSetMetas(metas, TIVX_KERNEL_FILT3x3_MAX_PARAMS, out_fmt, out_w, out_h);
    }

    return status;
}

vx_status tivxAddKernelErode3x3(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;
    vx_uint32 i, j;

    for (i = 0; i < (sizeof(gTivxFilt3x3KernelInfo)/
            sizeof(tivxFilter3x3KernelInfo)); i ++)
    {
        kernel = vxAddUserKernel(
                                context,
                                gTivxFilt3x3KernelInfo[i].kernel_name,
                                gTivxFilt3x3KernelInfo[i].kernel_id,
                                NULL,
                                2,
                                gTivxFilt3x3KernelInfo[i].validate_func,
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
                    VX_TYPE_IMAGE,
                    VX_PARAMETER_STATE_REQUIRED
                    );
                index++;
            }
            if ( status == VX_SUCCESS)
            {
                for (j = 0; j < gTivxFilt3x3KernelInfo[i].num_cpu; j ++)
                {
                    /* add supported target's */
                    status = tivxAddKernelTarget(kernel,
                        gTivxFilt3x3KernelInfo[i].cpu_name[j]);

                    if (VX_SUCCESS != status)
                    {
                        break;
                    }
                }
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

        gTivxFilt3x3KernelInfo[i].kernel = kernel;
    }

    return status;
}

vx_status tivxRemoveKernelErode3x3(vx_context context)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;

    for (i = 0; i < (sizeof(gTivxFilt3x3KernelInfo)/
            sizeof(tivxFilter3x3KernelInfo)); i ++)
    {
        if (NULL != gTivxFilt3x3KernelInfo[i].kernel)
        {
            status = vxRemoveKernel(gTivxFilt3x3KernelInfo[i].kernel);

            if (VX_SUCCESS == status)
            {
                gTivxFilt3x3KernelInfo[i].kernel = NULL;
            }
            else
            {
                /* Error */
                break;
            }
        }
    }

    return status;
}





