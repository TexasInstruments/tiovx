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

#include "uc_superset1.h"

vx_status uc_superset1_create(uc_superset1 usecase)
{
    vx_status status = VX_SUCCESS;

    memset(usecase, 0, sizeof(uc_superset1_t));

    if (status == VX_SUCCESS)
    {
        usecase->context = vxCreateContext();
        if (usecase->context == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->graph_0 = vxCreateGraph(usecase->context);
        if (usecase->graph_0 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        status = uc_superset1_data_create(usecase);
    }
    if (status == VX_SUCCESS)
    {
        status = uc_superset1_graph_0_create(usecase);
    }

    return status;
}

vx_status uc_superset1_verify(uc_superset1 usecase)
{
    vx_status status = VX_SUCCESS;

    if (status == VX_SUCCESS)
    {
        status = uc_superset1_graph_0_verify(usecase);
    }

    return status;
}

vx_status uc_superset1_run(uc_superset1 usecase)
{
    vx_status status = VX_SUCCESS;

    if (status == VX_SUCCESS)
    {
        status = uc_superset1_graph_0_run(usecase);
    }

    return status;
}

vx_status uc_superset1_delete(uc_superset1 usecase)
{
    vx_status status = VX_SUCCESS;

    if (status == VX_SUCCESS)
    {
        status = uc_superset1_graph_0_delete(usecase);
    }
    if (status == VX_SUCCESS)
    {
        status = uc_superset1_data_delete(usecase);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseContext(&usecase->context);
    }

    return status;
}

vx_status uc_superset1_data_create(uc_superset1 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_context context = usecase->context;

    if (status == VX_SUCCESS)
    {
        usecase->image_1 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_1 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_1, "image_1");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_2 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_2 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_2, "image_2");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_3 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_3 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_3, "image_3");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_4 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_4 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_4, "image_4");
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_CONVERT_POLICY_WRAP;

        usecase->scalar_59 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_59 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_5 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_5 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_5, "image_5");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_6 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_6 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_6, "image_6");
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_CONVERT_POLICY_WRAP;

        usecase->scalar_61 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_61 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_7 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_7 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_7, "image_7");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_8 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_8 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_8, "image_8");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_9 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_9 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_9, "image_9");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_10 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_10 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_10, "image_10");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_11 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_11 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_11, "image_11");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_12 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_12 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_12, "image_12");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_13 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_13 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_13, "image_13");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_14 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_14 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_14, "image_14");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_15 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_15 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_15, "image_15");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->convolution_16 = vxCreateConvolution(context, 3, 3);
        if (usecase->convolution_16 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_17 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_17 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_17, "image_17");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_18 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_18 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_18, "image_18");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_19 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_19 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_19, "image_19");
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_NONLINEAR_FILTER_MIN;

        usecase->scalar_71 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_71 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->matrix_20 = vxCreateMatrix(context, VX_TYPE_UINT8, 3, 3);
        if (usecase->matrix_20 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_21 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_21 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_21, "image_21");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_22 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_22 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_22, "image_22");
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_float32 value = 1;

        usecase->scalar_23 = vxCreateScalar(context, VX_TYPE_FLOAT32, &value);
        if (usecase->scalar_23 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_CONVERT_POLICY_WRAP;

        usecase->scalar_73 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_73 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_ROUND_POLICY_TO_ZERO;

        usecase->scalar_74 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_74 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_24 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_24 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_24, "image_24");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_25 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16);
        if (usecase->image_25 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_25, "image_25");
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_CONVERT_POLICY_WRAP;

        usecase->scalar_76 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_76 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_int32 value = 0;

        usecase->scalar_28 = vxCreateScalar(context, VX_TYPE_INT32, &value);
        if (usecase->scalar_28 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_26 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16);
        if (usecase->image_26 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_26, "image_26");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_27 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16);
        if (usecase->image_27 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_27, "image_27");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_29 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16);
        if (usecase->image_29 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_29, "image_29");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_30 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_30 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_30, "image_30");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_31 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_31 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_31, "image_31");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_32 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_32 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_32, "image_32");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_33 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_33 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_33, "image_33");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_34 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_RGBX);
        if (usecase->image_34 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_34, "image_34");
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_CHANNEL_R;

        usecase->scalar_81 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_81 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_35 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_35 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_35, "image_35");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_36 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_36 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_36, "image_36");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->lut_37 = vxCreateLUT(context, VX_TYPE_UINT8, 255);
        if (usecase->lut_37 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_38 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_38 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_38, "image_38");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_39 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_39 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_39, "image_39");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_40 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16);
        if (usecase->image_40 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_40, "image_40");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_41 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16);
        if (usecase->image_41 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_41, "image_41");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_42 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_42 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_42, "image_42");
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_CONVERT_POLICY_WRAP;

        usecase->scalar_87 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_87 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->matrix_44 = vxCreateMatrix(context, VX_TYPE_FLOAT32, 2, 3);
        if (usecase->matrix_44 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_INTERPOLATION_BILINEAR;

        usecase->scalar_89 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_89 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_43 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_43 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_43, "image_43");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->matrix_46 = vxCreateMatrix(context, VX_TYPE_FLOAT32, 3, 3);
        if (usecase->matrix_46 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_INTERPOLATION_BILINEAR;

        usecase->scalar_91 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_91 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_45 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_45 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_45, "image_45");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->threshold_47 = vxCreateThreshold(context, VX_THRESHOLD_TYPE_BINARY, VX_TYPE_UINT8);
        if (usecase->threshold_47 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_48 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_48 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_48, "image_48");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->remap_49 = vxCreateRemap(context, 640, 480, 1280, 720);
        if (usecase->remap_49 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_INTERPOLATION_BILINEAR;

        usecase->scalar_94 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_94 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_50 = vxCreateImage(context, 1280, 720, VX_DF_IMAGE_U8);
        if (usecase->image_50 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_50, "image_50");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_51 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_51 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_51, "image_51");
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_INTERPOLATION_BILINEAR;

        usecase->scalar_96 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_96 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->threshold_52 = vxCreateThreshold(context, VX_THRESHOLD_TYPE_BINARY, VX_TYPE_UINT8);
        if (usecase->threshold_52 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_int32 value = 3;

        usecase->scalar_98 = vxCreateScalar(context, VX_TYPE_INT32, &value);
        if (usecase->scalar_98 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_NORM_L1;

        usecase->scalar_99 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_99 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_53 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_53 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_53, "image_53");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_54 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U32);
        if (usecase->image_54 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_54, "image_54");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->pyramid_55 = vxCreatePyramid(context, 5, VX_SCALE_PYRAMID_HALF, 640, 480, VX_DF_IMAGE_S16);
        if (usecase->pyramid_55 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_56 = vxCreateImage(context, 20, 15, VX_DF_IMAGE_U8);
        if (usecase->image_56 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_56, "image_56");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_57 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_57 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_57, "image_57");
        }
    }

    return status;
}

vx_status uc_superset1_data_delete(uc_superset1 usecase)
{
    vx_status status = VX_SUCCESS;

    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_1);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_2);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_3);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_4);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_59);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_5);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_6);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_61);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_7);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_8);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_9);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_10);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_11);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_12);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_13);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_14);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_15);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->convolution_16);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_17);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_18);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_19);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_71);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->matrix_20);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_21);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_22);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_23);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_73);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_74);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_24);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_25);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_76);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_28);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_26);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_27);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_29);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_30);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_31);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_32);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_33);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_34);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_81);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_35);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_36);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->lut_37);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_38);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_39);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_40);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_41);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_42);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_87);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->matrix_44);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_89);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_43);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->matrix_46);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_91);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_45);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->threshold_47);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_48);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->remap_49);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_94);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_50);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_51);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_96);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->threshold_52);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_98);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_99);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_53);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_54);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->pyramid_55);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_56);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_57);
    }

    return status;
}

static vx_node usecase_node_create_node_58 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.absdiff");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 3);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_60 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_scalar scalar_2 ,
  vx_image image_3 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)scalar_2 ,
          (vx_reference)image_3 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.add");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 4);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_62 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_scalar scalar_2 ,
  vx_image image_3 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)scalar_2 ,
          (vx_reference)image_3 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.subtract");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 4);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_63 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.and");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 3);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_64 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.or");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 3);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_65 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.xor");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 3);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_66 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.not");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 2);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_67 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.box_3x3");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 2);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_68 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_convolution convolution_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)convolution_1 ,
          (vx_reference)image_2 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.custom_convolution");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 3);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_69 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.dilate_3x3");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 2);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_70 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.gaussian_3x3");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 2);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_72 (
  vx_graph graph ,
  vx_scalar scalar_0 ,
  vx_image image_1 ,
  vx_matrix matrix_2 ,
  vx_image image_3 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)scalar_0 ,
          (vx_reference)image_1 ,
          (vx_reference)matrix_2 ,
          (vx_reference)image_3 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.non_linear_filter");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 4);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_75 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_scalar scalar_2 ,
  vx_scalar scalar_3 ,
  vx_scalar scalar_4 ,
  vx_image image_5 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)scalar_2 ,
          (vx_reference)scalar_3 ,
          (vx_reference)scalar_4 ,
          (vx_reference)image_5 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.multiply");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 6);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_77 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_scalar scalar_2 ,
  vx_scalar scalar_3 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)scalar_2 ,
          (vx_reference)scalar_3 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.convertdepth");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 4);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_78 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.magnitude");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 3);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_79 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.phase");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 3);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_80 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_image image_2 ,
  vx_image image_3 ,
  vx_image image_4 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2 ,
          (vx_reference)image_3 ,
          (vx_reference)image_4 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.channel_combine");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 5);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_82 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_scalar scalar_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)scalar_1 ,
          (vx_reference)image_2 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.channel_extract");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 3);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_83 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.equalize_histogram");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 2);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_84 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_lut lut_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)lut_1 ,
          (vx_reference)image_2 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.table_lookup");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 3);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_85 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.median_3x3");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 2);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_86 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.sobel_3x3");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 3);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_88 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_scalar scalar_2 ,
  vx_scalar scalar_3 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)scalar_2 ,
          (vx_reference)scalar_3 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.convertdepth");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 4);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_90 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_matrix matrix_1 ,
  vx_scalar scalar_2 ,
  vx_image image_3 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)matrix_1 ,
          (vx_reference)scalar_2 ,
          (vx_reference)image_3 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.warp_affine");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 4);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_92 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_matrix matrix_1 ,
  vx_scalar scalar_2 ,
  vx_image image_3 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)matrix_1 ,
          (vx_reference)scalar_2 ,
          (vx_reference)image_3 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.warp_perspective");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 4);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_93 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_threshold threshold_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)threshold_1 ,
          (vx_reference)image_2 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.threshold");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 3);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_95 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_remap remap_1 ,
  vx_scalar scalar_2 ,
  vx_image image_3 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)remap_1 ,
          (vx_reference)scalar_2 ,
          (vx_reference)image_3 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.remap");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 4);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_97 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_scalar scalar_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)scalar_2 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.scale_image");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 3);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_100 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_threshold threshold_1 ,
  vx_scalar scalar_2 ,
  vx_scalar scalar_3 ,
  vx_image image_4 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)threshold_1 ,
          (vx_reference)scalar_2 ,
          (vx_reference)scalar_3 ,
          (vx_reference)image_4 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.canny_edge_detector");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 5);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_101 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.integral_image");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 2);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

static vx_node usecase_node_create_node_102 (
  vx_graph graph ,
  vx_pyramid pyramid_0 ,
  vx_image image_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)pyramid_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2 
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.laplacian_reconstruct");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 3);
        }
        vxReleaseKernel(&kernel);
    }

    return node;
}

vx_status uc_superset1_graph_0_create(uc_superset1 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_graph graph = usecase->graph_0;

    if (status == VX_SUCCESS)
    {
        usecase->node_58 = usecase_node_create_node_58 (
            graph ,
            usecase->image_1 ,
            usecase->image_2 ,
            usecase->image_3 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_58, "node_58");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_58, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_60 = usecase_node_create_node_60 (
            graph ,
            usecase->image_3 ,
            usecase->image_4 ,
            usecase->scalar_59 ,
            usecase->image_5 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_60, "node_60");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_60, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_62 = usecase_node_create_node_62 (
            graph ,
            usecase->image_5 ,
            usecase->image_6 ,
            usecase->scalar_61 ,
            usecase->image_7 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_62, "node_62");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_62, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_63 = usecase_node_create_node_63 (
            graph ,
            usecase->image_7 ,
            usecase->image_8 ,
            usecase->image_9 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_63, "node_63");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_63, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_64 = usecase_node_create_node_64 (
            graph ,
            usecase->image_9 ,
            usecase->image_10 ,
            usecase->image_11 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_64, "node_64");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_64, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_65 = usecase_node_create_node_65 (
            graph ,
            usecase->image_11 ,
            usecase->image_12 ,
            usecase->image_13 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_65, "node_65");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_65, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_66 = usecase_node_create_node_66 (
            graph ,
            usecase->image_13 ,
            usecase->image_14 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_66, "node_66");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_66, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_67 = usecase_node_create_node_67 (
            graph ,
            usecase->image_14 ,
            usecase->image_15 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_67, "node_67");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_67, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_68 = usecase_node_create_node_68 (
            graph ,
            usecase->image_15 ,
            usecase->convolution_16 ,
            usecase->image_17 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_68, "node_68");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_68, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_69 = usecase_node_create_node_69 (
            graph ,
            usecase->image_17 ,
            usecase->image_18 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_69, "node_69");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_69, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_70 = usecase_node_create_node_70 (
            graph ,
            usecase->image_18 ,
            usecase->image_19 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_70, "node_70");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_70, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_72 = usecase_node_create_node_72 (
            graph ,
            usecase->scalar_71 ,
            usecase->image_19 ,
            usecase->matrix_20 ,
            usecase->image_21 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_72, "node_72");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_72, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_75 = usecase_node_create_node_75 (
            graph ,
            usecase->image_21 ,
            usecase->image_22 ,
            usecase->scalar_23 ,
            usecase->scalar_73 ,
            usecase->scalar_74 ,
            usecase->image_24 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_75, "node_75");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_75, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_77 = usecase_node_create_node_77 (
            graph ,
            usecase->image_24 ,
            usecase->image_25 ,
            usecase->scalar_76 ,
            usecase->scalar_28 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_77, "node_77");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_77, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_78 = usecase_node_create_node_78 (
            graph ,
            usecase->image_25 ,
            usecase->image_26 ,
            usecase->image_27 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_78, "node_78");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_78, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_79 = usecase_node_create_node_79 (
            graph ,
            usecase->image_27 ,
            usecase->image_29 ,
            usecase->image_30 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_79, "node_79");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_79, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_80 = usecase_node_create_node_80 (
            graph ,
            usecase->image_30 ,
            usecase->image_31 ,
            usecase->image_32 ,
            usecase->image_33 ,
            usecase->image_34 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_80, "node_80");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_80, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_82 = usecase_node_create_node_82 (
            graph ,
            usecase->image_34 ,
            usecase->scalar_81 ,
            usecase->image_35 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_82, "node_82");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_82, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_83 = usecase_node_create_node_83 (
            graph ,
            usecase->image_35 ,
            usecase->image_36 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_83, "node_83");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_83, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_84 = usecase_node_create_node_84 (
            graph ,
            usecase->image_36 ,
            usecase->lut_37 ,
            usecase->image_38 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_84, "node_84");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_84, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_85 = usecase_node_create_node_85 (
            graph ,
            usecase->image_38 ,
            usecase->image_39 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_85, "node_85");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_85, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_86 = usecase_node_create_node_86 (
            graph ,
            usecase->image_39 ,
            usecase->image_40 ,
            usecase->image_41 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_86, "node_86");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_86, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_88 = usecase_node_create_node_88 (
            graph ,
            usecase->image_41 ,
            usecase->image_42 ,
            usecase->scalar_87 ,
            usecase->scalar_28 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_88, "node_88");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_88, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_90 = usecase_node_create_node_90 (
            graph ,
            usecase->image_42 ,
            usecase->matrix_44 ,
            usecase->scalar_89 ,
            usecase->image_43 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_90, "node_90");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_90, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_92 = usecase_node_create_node_92 (
            graph ,
            usecase->image_43 ,
            usecase->matrix_46 ,
            usecase->scalar_91 ,
            usecase->image_45 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_92, "node_92");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_92, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_93 = usecase_node_create_node_93 (
            graph ,
            usecase->image_45 ,
            usecase->threshold_47 ,
            usecase->image_48 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_93, "node_93");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_93, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_95 = usecase_node_create_node_95 (
            graph ,
            usecase->image_48 ,
            usecase->remap_49 ,
            usecase->scalar_94 ,
            usecase->image_50 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_95, "node_95");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_95, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_97 = usecase_node_create_node_97 (
            graph ,
            usecase->image_50 ,
            usecase->image_51 ,
            usecase->scalar_96 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_97, "node_97");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_97, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_100 = usecase_node_create_node_100 (
            graph ,
            usecase->image_51 ,
            usecase->threshold_52 ,
            usecase->scalar_98 ,
            usecase->scalar_99 ,
            usecase->image_53 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_100, "node_100");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_100, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_101 = usecase_node_create_node_101 (
            graph ,
            usecase->image_53 ,
            usecase->image_54 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_101, "node_101");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_101, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_102 = usecase_node_create_node_102 (
            graph ,
            usecase->pyramid_55 ,
            usecase->image_56 ,
            usecase->image_57 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_102, "node_102");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_102, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }

    if (status == VX_SUCCESS)
    {
        status = vxSetReferenceName( (vx_reference)usecase->graph_0, "graph_0");
    }

    return status;
}

vx_status uc_superset1_graph_0_delete(uc_superset1 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_graph graph = usecase->graph_0;

    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_58 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_60 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_62 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_63 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_64 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_65 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_66 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_67 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_68 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_69 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_70 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_72 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_75 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_77 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_78 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_79 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_80 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_82 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_83 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_84 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_85 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_86 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_88 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_90 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_92 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_93 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_95 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_97 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_100 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_101 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_102 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseGraph(&graph);
    }

    usecase->graph_0 = graph;

    return status;
}

vx_status uc_superset1_graph_0_verify(uc_superset1 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_graph graph = usecase->graph_0;

    if (status == VX_SUCCESS)
    {
        status = vxVerifyGraph(graph);
    }

    return status;
}

vx_status uc_superset1_graph_0_run(uc_superset1 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_graph graph = usecase->graph_0;

    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    return status;
}


