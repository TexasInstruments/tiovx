/*
 *
 * Copyright (c) 2024 Texas Instruments Incorporated
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

#include "TI/tivx.h"
#include "TI/tivx_capture.h"
#include "tivx_capture_kernels.h"
#include "TI/tivx_test_kernels_kernels.h"
#include "tivx_kernel_test_target.h"
#include "TI/tivx_target_kernel.h"
#include <stdio.h>

static vx_kernel vx_test_target_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelTestTargetValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelTestTargetInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelTestTarget(vx_context context);
vx_status tivxRemoveKernelTestTarget(vx_context context);

#if defined(LDRA_COVERAGE_ENABLED)
#ifndef PC
#define MAX_LENGTH 64
typedef struct {
    vx_status (*funcPtr)(uint8_t);
    char funcName[MAX_LENGTH];
    vx_status status;
} FuncInfo;

FuncInfo funcArr[];

static vx_status tivxTestKernelsHostUtilsAddKernelTargetDsp(uint8_t id)
{
    vx_context context;
    vx_status status = (vx_status)VX_SUCCESS;
    vx_image img;
    vx_kernel kernel;
    vx_reference ref;

    context = vxCreateContext();
    img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    ref = (vx_reference)img;
    kernel = (vx_kernel)ref;

    if ((vx_status)VX_ERROR_INVALID_REFERENCE != tivxKernelsHostUtilsAddKernelTargetDsp(kernel))
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxKernelsHostUtilsAddKernelTargetMcu returned success with"\
        " reference type set to image\n");
        status = (vx_status)VX_FAILURE;
    }

    vxReleaseImage(&img);

    vxReleaseContext(&context);

    snprintf(funcArr[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestKernelsHostUtilsAddKernelTargetMcu(uint8_t id)
{
    vx_context context;
    vx_status status = (vx_status)VX_SUCCESS;
    vx_image img;
    vx_kernel kernel;
    vx_reference ref;

    context = vxCreateContext();
    img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    ref = (vx_reference)img;
    kernel = (vx_kernel)ref;

    if ((vx_status)VX_ERROR_INVALID_REFERENCE != tivxKernelsHostUtilsAddKernelTargetMcu(kernel))
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxKernelsHostUtilsAddKernelTargetMcu returned success with"\
        " reference type set to image\n");
        status = (vx_status)VX_FAILURE;
    }

    vxReleaseImage(&img);

    vxReleaseContext(&context);

    snprintf(funcArr[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestCommonKernelConfigValidRect(uint8_t id)
{
    vx_context context;
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms, *prms_ptr=NULL;
    vx_image img1;

    vx_rectangle_t roi_rect1 = { 4, 4, 6, 6 };
    vx_rectangle_t roi_rect2 = { 1, 1, 8, 8 };
    vx_rectangle_t roi_rect3 = { 2, 2, 3, 3 };

    context = vxCreateContext();
    tivxKernelValidRectParams_init(&prms);

    if ((vx_status)VX_FAILURE != tivxKernelConfigValidRect(prms_ptr))
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxKernelConfigValidRect returned success with NULL params\n");
        status = (vx_status)VX_FAILURE;
    }

    prms.num_input_images = TIVX_KERNEL_COMMON_VALID_RECT_MAX_IMAGE+1;
    if ((vx_status)VX_FAILURE != tivxKernelConfigValidRect(&prms))
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxKernelConfigValidRect returned success with num_input_images "\
        "greater than TIVX_KERNEL_COMMON_VALID_RECT_MAX_IMAGE\n");
        status = (vx_status)VX_FAILURE;
    }

    prms.num_input_images = 1;
    prms.num_output_images = TIVX_KERNEL_COMMON_VALID_RECT_MAX_IMAGE+1;
    if ((vx_status)VX_FAILURE != tivxKernelConfigValidRect(&prms))
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxKernelConfigValidRect returned success with num_output_images "\
        "greater than TIVX_KERNEL_COMMON_VALID_RECT_MAX_IMAGE\n");
        status = (vx_status)VX_FAILURE;
    }

    prms.num_output_images = 1;
    prms.in_img[0U] = NULL;
    if ((vx_status)VX_FAILURE != tivxKernelConfigValidRect(&prms))
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxKernelConfigValidRect returned success with in_img 0 "\
        "set to NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    prms.in_img[0U] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    prms.out_img[0U] = NULL;
    if ((vx_status)VX_FAILURE != tivxKernelConfigValidRect(&prms))
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxKernelConfigValidRect returned success with out_img 0 "\
        "set to NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    vxReleaseImage(&prms.in_img[0U]);

    #define VX_DF_IMAGE_DEFAULT VX_DF_IMAGE('D','E','F','A')
    prms.in_img[0U] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_DEFAULT);
    prms.out_img[0U] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    if ((vx_status)VX_ERROR_INVALID_REFERENCE != tivxKernelConfigValidRect(&prms))
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxKernelConfigValidRect returned success with invalid in_img 0 "\
        "vx_df_image type\n");
        status = (vx_status)VX_FAILURE;
    }

    vxReleaseImage(&prms.in_img[0U]);
    vxReleaseImage(&prms.out_img[0U]);

    prms.in_img[0U] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    prms.out_img[0U] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_DEFAULT);
    if ((vx_status)VX_ERROR_INVALID_REFERENCE != tivxKernelConfigValidRect(&prms))
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxKernelConfigValidRect returned success with invalid out_img 0 "\
        "vx_df_image type\n");
        status = (vx_status)VX_FAILURE;
    }

    vxReleaseImage(&prms.in_img[0U]);
    vxReleaseImage(&prms.out_img[0U]);

    img1 = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    prms.in_img[0U] = vxCreateImageFromROI(img1, &roi_rect2);
    prms.out_img[0U] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    vxSetImageValidRectangle(prms.in_img[0U], &roi_rect3);
    if ((vx_status)VX_SUCCESS != tivxKernelConfigValidRect(&prms))
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxKernelConfigValidRect returned failure with in_img 0 "\
        "start x and y greater than out_img 0 start x and y - they should have been corrected\n");
        status = (vx_status)VX_FAILURE;
    }

    vxReleaseImage(&img1);
    vxReleaseImage(&prms.in_img[0U]);
    vxReleaseImage(&prms.out_img[0U]);

    img1 = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    prms.in_img[0U] = vxCreateImageFromROI(img1, &roi_rect2);
    prms.out_img[0U] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    vxSetImageValidRectangle(prms.out_img[0U], &roi_rect3);
    if ((vx_status)VX_SUCCESS != tivxKernelConfigValidRect(&prms))
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxKernelConfigValidRect returned failure with out_img 0 "\
        "start x and y greater than out_img 0 start x and y - they should have been corrected\n");
        status = (vx_status)VX_FAILURE;
    }

    vxReleaseImage(&img1);
    vxReleaseImage(&prms.in_img[0U]);
    vxReleaseImage(&prms.out_img[0U]);

    prms.border_mode = (vx_enum)VX_BORDER_CONSTANT;
    prms.in_img[0U] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    prms.out_img[0U] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    if ((vx_status)VX_SUCCESS != tivxKernelConfigValidRect(&prms))
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxKernelConfigValidRect returned failure with valid in_img 0 "\
        "and out_img 0 and border mode constant\n");
        status = (vx_status)VX_FAILURE;
    }

    vxReleaseImage(&prms.in_img[0U]);
    vxReleaseImage(&prms.out_img[0U]);

    prms.border_mode = (vx_enum)VX_BORDER_UNDEFINED;
    prms.left_pad = 258U;
    prms.right_pad = 256U;
    img1 = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    prms.in_img[0U] = vxCreateImageFromROI(img1, &roi_rect1);
    prms.out_img[0U] = vxCreateImageFromROI(img1, &roi_rect1);
    if ((vx_status)VX_ERROR_INVALID_PARAMETERS != tivxKernelConfigValidRect(&prms))
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxKernelConfigValidRect returned success with invalid "\
        "left and right padding set in params\n");
        status = (vx_status)VX_FAILURE;
    }

    vxReleaseImage(&img1);
    vxReleaseImage(&prms.in_img[0U]);
    vxReleaseImage(&prms.out_img[0U]);
    vxReleaseContext(&context);

    snprintf(funcArr[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestKernelValidateParametersNotNull(uint8_t id)
{
    vx_context context;
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint8 max_params = 5u;
    vx_reference prms[max_params];
    vx_uint32 i;

    context = vxCreateContext();

    for (i = 0U; i < max_params; i++)
    {
        prms[i] = NULL;
    }

    if ((vx_status)VX_ERROR_INVALID_PARAMETERS != tivxKernelValidateParametersNotNull(prms, max_params))
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxKernelValidateParametersNotNull returned success with NULL params\n");
        status = (vx_status)VX_FAILURE;
    }

    for (i = 0U; i < max_params; i++)
    {
        prms[i] = (vx_reference)vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    }

    if ((vx_status)VX_SUCCESS != tivxKernelValidateParametersNotNull(prms, max_params))
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxKernelValidateParametersNotNull returned failure with valid params\n");
        status = (vx_status)VX_FAILURE;
    }

    for (i = 0U; i < max_params; i++)
    {
        vxReleaseReference(&prms[i]);
    }

    vxReleaseContext(&context);

    snprintf(funcArr[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

vx_status dummy_add_kernel_fxn(vx_context context)
{
    return (vx_status)VX_FAILURE;
}

static vx_status tivxTestPublishKernels(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_context context;
    vx_uint32 kernel_list_size = 1U;
    Tivx_Host_Kernel_List  test_host_kernel_list[] = {
        {NULL,dummy_add_kernel_fxn}
    };

    context = vxCreateContext();

    tivxPublishKernels(context, test_host_kernel_list, kernel_list_size);

    Tivx_Host_Kernel_List  test_host_kernel_list_null[] = {
        {NULL,NULL}
    };

    tivxPublishKernels(context, test_host_kernel_list_null, kernel_list_size);

    vxReleaseContext(&context);

    snprintf(funcArr[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestUnPublishKernels(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_context context;
    vx_uint32 kernel_list_size = 1U;
    Tivx_Host_Kernel_List  test_host_kernel_list[] = {
        {NULL,dummy_add_kernel_fxn}
    };

    context = vxCreateContext();

    tivxUnPublishKernels(context, test_host_kernel_list, kernel_list_size);

    Tivx_Host_Kernel_List  test_host_kernel_list_null[] = {
        {NULL,NULL}
    };
    tivxUnPublishKernels(context, test_host_kernel_list_null, kernel_list_size);

    vxReleaseContext(&context);

    snprintf(funcArr[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

FuncInfo funcArr[] = {
    {tivxTestKernelsHostUtilsAddKernelTargetDsp,"",VX_SUCCESS},
    {tivxTestKernelsHostUtilsAddKernelTargetMcu,"",VX_SUCCESS},
    {tivxTestCommonKernelConfigValidRect,"",VX_SUCCESS},
    {tivxTestKernelValidateParametersNotNull,"",VX_SUCCESS},
    {tivxTestPublishKernels,"",VX_SUCCESS},
    {tivxTestUnPublishKernels,"",VX_SUCCESS}
};
#endif /* #ifndef PC*/
#endif /* #if defined(LDRA_COVERAGE_ENABLED) */

static vx_status VX_CALLBACK tivxAddKernelTestTargetValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ( (num != TIVX_KERNEL_TEST_TARGET_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_TEST_TARGET_INPUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_TEST_TARGET_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelTestTargetInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_TEST_TARGET_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_TEST_TARGET_INPUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_TEST_TARGET_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

#if defined(LDRA_COVERAGE_ENABLED)
    if((vx_status)VX_SUCCESS == status)
    {
        uint8_t i = 0;
        uint8_t pcount = 0;
        uint8_t fcount = 0;
        vx_status status1 = (vx_status)VX_SUCCESS;
        uint32_t size= sizeof(funcArr)/sizeof(funcArr[0]);
        tivx_set_debug_zone(VX_ZONE_INFO);

        VX_PRINT(VX_ZONE_INFO,"------------------TEST KERNEL INITIALIZE CALLBACK TESTCASES-------------------------\n");
        VX_PRINT(VX_ZONE_INFO,"[  TO DO  ] %d test(s) from 1 test case(s)\n",size);
        VX_PRINT(VX_ZONE_INFO,"------------------TEST KERNEL INITIALIZE CALLBACK TESTCASES-------------------------\n");

        if((vx_status)VX_SUCCESS == status)
        {
            for(i=0;i<size;i++)
            {
                status1 = funcArr[i].funcPtr(i);
                if((vx_status)VX_SUCCESS != status1)
                {
                    VX_PRINT(VX_ZONE_ERROR,"[ !FAILED! ] TARGET TESTCASE: %s\n",funcArr[i].funcName);
                    funcArr[i].status=status1;
                    status = status1;
                    fcount++;
                }
                else
                {
                    VX_PRINT(VX_ZONE_INFO,"[ PASSED ] TARGET TESTCASE: %s\n",funcArr[i].funcName);
                    pcount++;
                }
            }
        }
        VX_PRINT(VX_ZONE_INFO,"------------------TEST KERNEL INITIALIZE CALLBACK TESTCASES-------------------------\n");
        VX_PRINT(VX_ZONE_INFO,"[ ALL DONE ] %d test(s) from 1 test case(s) ran\n",i);
        VX_PRINT(VX_ZONE_INFO,"[  PASSED  ] %d test(s)\n",pcount);
        if(fcount>0)
        {
            i=0;
            VX_PRINT(VX_ZONE_INFO,"[  FAILED  ] %d test(s), listed below:\n",fcount);
            while(i<size)
            {
                if(funcArr[i].status!= VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_INFO,"[  FAILED  ] %s\n",funcArr[i].funcName);
                }
                i++;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO,"[  FAILED  ] %d test(s)\n",fcount);
        }
        VX_PRINT(VX_ZONE_INFO,"------------------------------------------------------------------------------------\n");
        tivx_clr_debug_zone(VX_ZONE_INFO);
    }
#endif /* #if defined(LDRA_COVERAGE_ENABLED) */

    return status;
}

vx_status tivxAddKernelTestTarget(vx_context context)
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
                    TIVX_KERNEL_TEST_TARGET_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_TEST_TARGET_MAX_PARAMS,
                    tivxAddKernelTestTargetValidate,
                    tivxAddKernelTestTargetInitialize,
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
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxKernelsHostUtilsAddKernelTargetMcu(kernel);
            tivxKernelsHostUtilsAddKernelTargetDsp(kernel);
            tivxAddKernelTarget(kernel, TIVX_TARGET_MPU_0);
            #if defined(SOC_J721E)
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_1);
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
    vx_test_target_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelTestTarget(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_test_target_kernel;

    status = vxRemoveKernel(kernel);
    vx_test_target_kernel = NULL;

    return status;
}


