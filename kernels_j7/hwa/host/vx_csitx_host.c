/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
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

#ifdef BUILD_CSITX

#include "TI/tivx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_csitx.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_hwa_host_priv.h"

static vx_kernel vx_csitx_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelCsitxValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelCsitxInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelCsitxValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_user_data_object configuration = NULL;
    vx_object_array input = NULL;
    vx_char configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size configuration_size, input_num_items;
    vx_reference obj_arr_element;
    vx_df_image img_fmt;
    vx_enum ref_type;

    if ( (num != TIVX_KERNEL_CSITX_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_CSITX_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_CSITX_INPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_CSITX_CONFIGURATION_IDX];
        input = (vx_object_array)parameters[TIVX_KERNEL_CSITX_INPUT_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));

        tivxCheckStatus(&status, vxQueryObjectArray(input, (vx_enum)VX_OBJECT_ARRAY_NUMITEMS, &input_num_items, sizeof(input_num_items)));

    }

    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((configuration_size != sizeof(tivx_csitx_params_t)) ||
            (strncmp(configuration_name, "tivx_csitx_params_t", sizeof(configuration_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be a user_data_object of type:\n tivx_csitx_params_t \n");
        }

    }

    if ((vx_status)VX_SUCCESS == status)
    {
        obj_arr_element = vxGetObjectArrayItem(input, 0);

        if (NULL != obj_arr_element)
        {
            tivxCheckStatus(&status, vxQueryReference(obj_arr_element, (vx_enum)VX_REFERENCE_TYPE, &ref_type, sizeof(ref_type)));

            if ((vx_status)VX_SUCCESS == status)
            {
                if ( (TIVX_TYPE_RAW_IMAGE != ref_type) &&
                     ((vx_enum)VX_TYPE_IMAGE != ref_type) )
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "input object array must contain either TIVX_TYPE_RAW_IMAGE or VX_TYPE_IMAGE \n");
                }
                else if ((vx_enum)VX_TYPE_IMAGE == ref_type)
                {
                    tivxCheckStatus(&status, vxQueryImage((vx_image)obj_arr_element, (vx_enum)VX_IMAGE_FORMAT, &img_fmt, sizeof(img_fmt)));

                    if (((vx_df_image)VX_DF_IMAGE_RGBX != img_fmt) &&
                        ((vx_df_image)VX_DF_IMAGE_U16 != img_fmt) &&
                        ((vx_df_image)VX_DF_IMAGE_UYVY != img_fmt) &&
                        ((vx_df_image)VX_DF_IMAGE_YUYV != img_fmt))
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "image format is invalid \n");
                    }
                }
                else
                {
                    /* do nothing */
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "query 'input' object array reference failed \n");
            }
            vxReleaseReference(&obj_arr_element);
        }
        else
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input' object array elements are NULL \n");
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelCsitxInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_CSITX_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_CSITX_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_CSITX_INPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    return status;
}

vx_status tivxAddKernelCsitx(vx_context context)
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
                    TIVX_KERNEL_CSITX_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_CSITX_MAX_PARAMS,
                    tivxAddKernelCsitxValidate,
                    tivxAddKernelCsitxInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_uint32 num_bufs = TIVX_CSITX_MIN_PIPEUP_BUFS;

        vxSetKernelAttribute(kernel, (vx_enum)VX_KERNEL_PIPEUP_OUTPUT_DEPTH, &num_bufs, sizeof(num_bufs));

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
                        (vx_enum)VX_TYPE_OBJECT_ARRAY,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_CSITX);
            #if defined(SOC_J721S2) || defined(SOC_J784S4)
            tivxAddKernelTarget(kernel, TIVX_TARGET_CSITX2);
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
    vx_csitx_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelCsitx(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_csitx_kernel;

    status = vxRemoveKernel(kernel);
    vx_csitx_kernel = NULL;

    return status;
}

void tivx_csitx_params_init(tivx_csitx_params_t *prms)
{
    uint32_t cnt, loopCnt;

    if (NULL != prms)
    {
        prms->numInst = 1u;
        prms->numCh   = 0u;

        for (loopCnt = 0U ; loopCnt < TIVX_CSITX_MAX_INST ; loopCnt++)
        {
            prms->instId[loopCnt]                       = loopCnt;
            prms->instCfg[loopCnt].rxCompEnable         = (uint32_t)vx_true_e;
            prms->instCfg[loopCnt].rxv1p3MapEnable      = (uint32_t)vx_true_e;
            prms->instCfg[loopCnt].laneBandSpeed        = TIVX_CSITX_LANE_BAND_SPEED_770_TO_870_MBPS;
            /* This is set to default reserved value to maintain backward comparability */
            prms->instCfg[loopCnt].laneSpeedMbps        = TIVX_CSITX_LANE_SPEED_MBPS_RESERVED;
            prms->instCfg[loopCnt].numDataLanes         = 5u;
            for (cnt = 0u; cnt < prms->instCfg[loopCnt].numDataLanes; cnt ++)
            {
                prms->instCfg[loopCnt].lanePolarityCtrl[cnt]     = 0u;
            }
            prms->instCfg[loopCnt].vBlank = 22U;
            prms->instCfg[loopCnt].hBlank = 40U;
            prms->instCfg[loopCnt].startDelayPeriod = 40U;
        }
        for (cnt = 0u; cnt < TIVX_CSITX_MAX_CH; cnt ++)
        {
            prms->chVcNum[cnt]   = cnt;
            prms->chInstMap[cnt] = 0U;
        }
    }
}
#endif /*BUILD_CSITX*/