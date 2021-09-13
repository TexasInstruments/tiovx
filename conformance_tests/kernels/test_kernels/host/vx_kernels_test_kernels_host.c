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

#include <TI/tivx.h>
#include <TI/tivx_test_kernels.h>
#include "tivx_test_kernels_kernels.h"
#include "tivx_capture_kernels.h"
#include "tivx_kernels_host_utils.h"

static vx_status VX_CALLBACK publishKernels(vx_context context);
static vx_status VX_CALLBACK unPublishKernels(vx_context context);

static uint32_t gIsTestKernelsKernelsLoad = 0u;

vx_status tivxAddKernelNotNot(vx_context context);
vx_status tivxAddKernelScalarSink(vx_context context);
vx_status tivxAddKernelScalarSource(vx_context context);
vx_status tivxAddKernelScalarSink2(vx_context context);
vx_status tivxAddKernelScalarSource2(vx_context context);
vx_status tivxAddKernelScalarIntermediate(vx_context context);
vx_status tivxAddKernelScalarSourceError(vx_context context);
vx_status tivxAddKernelScalarSourceObjArray(vx_context context);
vx_status tivxAddKernelScalarSinkObjArray(vx_context context);
vx_status tivxAddKernelPyramidIntermediate(vx_context context);
vx_status tivxAddKernelPyramidSource(vx_context context);
vx_status tivxAddKernelCmdTimeoutTest(vx_context context);
vx_status tivxAddKernelScalarIntermediate2(vx_context context);

vx_status tivxRemoveKernelScalarSink(vx_context context);
vx_status tivxRemoveKernelScalarSource(vx_context context);
vx_status tivxRemoveKernelScalarSink2(vx_context context);
vx_status tivxRemoveKernelScalarSource2(vx_context context);
vx_status tivxRemoveKernelScalarIntermediate(vx_context context);
vx_status tivxRemoveKernelNotNot(vx_context context);
vx_status tivxRemoveKernelScalarSourceError(vx_context context);
vx_status tivxRemoveKernelScalarSourceObjArray(vx_context context);
vx_status tivxRemoveKernelScalarSinkObjArray(vx_context context);
vx_status tivxRemoveKernelPyramidIntermediate(vx_context context);
vx_status tivxRemoveKernelPyramidSource(vx_context context);
vx_status tivxRemoveKernelCmdTimeoutTest(vx_context context);
vx_status tivxRemoveKernelScalarIntermediate2(vx_context context);

static Tivx_Host_Kernel_List  gTivx_host_kernel_list[] = {
    {&tivxAddKernelNotNot, &tivxRemoveKernelNotNot},
    {&tivxAddKernelScalarSink, &tivxRemoveKernelScalarSink},
    {&tivxAddKernelScalarSource, &tivxRemoveKernelScalarSource},
    {&tivxAddKernelScalarSink2, &tivxRemoveKernelScalarSink2},
    {&tivxAddKernelScalarSource2, &tivxRemoveKernelScalarSource2},
    {&tivxAddKernelScalarIntermediate, &tivxRemoveKernelScalarIntermediate},
    {&tivxAddKernelScalarSourceError, &tivxRemoveKernelScalarSourceError},
    {&tivxAddKernelScalarSourceObjArray, &tivxRemoveKernelScalarSourceObjArray},
    {&tivxAddKernelScalarSinkObjArray, &tivxRemoveKernelScalarSinkObjArray},
    {&tivxAddKernelPyramidIntermediate, &tivxRemoveKernelPyramidIntermediate},
    {&tivxAddKernelPyramidSource, &tivxRemoveKernelPyramidSource},
    {&tivxAddKernelCmdTimeoutTest, &tivxRemoveKernelCmdTimeoutTest},
    {&tivxAddKernelScalarIntermediate2, &tivxRemoveKernelScalarIntermediate2},
};

static vx_status VX_CALLBACK publishKernels(vx_context context)
{
    return tivxPublishKernels(context, gTivx_host_kernel_list, dimof(gTivx_host_kernel_list));
}

static vx_status VX_CALLBACK unPublishKernels(vx_context context)
{
    return tivxUnPublishKernels(context, gTivx_host_kernel_list, dimof(gTivx_host_kernel_list));
}

void tivxRegisterTestKernelsKernels(void)
{
    tivxRegisterModule(TIVX_MODULE_NAME_TEST_KERNELS, publishKernels, unPublishKernels);
}

void tivxUnRegisterTestKernelsKernels(void)
{
    tivxUnRegisterModule(TIVX_MODULE_NAME_TEST_KERNELS);
}

void tivxTestKernelsLoadKernels(vx_context context)
{
    if ((0 == gIsTestKernelsKernelsLoad) && (NULL != context))
    {
        tivxRegisterTestKernelsKernels();
        vxLoadKernels(context, TIVX_MODULE_NAME_TEST_KERNELS);

#if defined(A72) || defined(A15)
        tivxRegisterTestKernelsTargetArmKernels();
#endif

        gIsTestKernelsKernelsLoad = 1U;
    }
}

void tivxTestKernelsUnLoadKernels(vx_context context)
{
    if ((1u == gIsTestKernelsKernelsLoad) && (NULL != context))
    {
        vxUnloadKernels(context, TIVX_MODULE_NAME_TEST_KERNELS);
        tivxUnRegisterTestKernelsKernels();

#if defined(A72) || defined(A15)
        tivxUnRegisterTestKernelsTargetArmKernels();
#endif

        gIsTestKernelsKernelsLoad = 0U;
    }
}

void tivxTestKernelsUnSetLoadKernelsFlag(void)
{
    if (1u == gIsTestKernelsKernelsLoad)
    {
        gIsTestKernelsKernelsLoad = 0U;
    }
}
