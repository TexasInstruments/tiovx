/*

 * Copyright (c) 2015-2017 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * Copyright (c) 2022 Texas Instruments Incorporated
 */

#include <TI/tivx_target_kernel.h>

#include "test_engine/test.h"

tivx_target_kernel_instance g_tki[TIVX_TARGET_KERNEL_INSTANCE_MAX+1];

vx_status c_function(tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);
vx_status p_function(tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);

TESTCASE(tivxTgKnl, CT_VXContext, ct_setup_vx_context, 0)

vx_status c_function(tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg) {

    return (VX_SUCCESS);
}

vx_status p_function(tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg) {

    return (VX_SUCCESS);
}

TEST(tivxTgKnl, negativeTestAddTargetKernelInternal)
{
    vx_context context = context_->vx_context_;

    tivx_target_kernel ttk = NULL;
    tivx_target_kernel_instance tki = NULL;
    vx_enum kernel_id = 0;
    char tname[] = {'t', 'i', 'o', 'v', 'x'};
    tivx_target_kernel_f process_func = NULL, create_func = NULL, delete_func = NULL;
    tivx_target_kernel_control_f control_func = NULL;
    vx_uint32 priv_arg = 0;
    int32_t i = 0;
    vx_uint32 num_target_kernels;

    ASSERT(NULL == (ttk = tivxAddTargetKernel(kernel_id, NULL, NULL, NULL, NULL, NULL, (void *)(&priv_arg))));
    ASSERT(NULL == (ttk = tivxAddTargetKernel(kernel_id, tname, NULL, c_function, NULL, NULL, (void *)(&priv_arg))));

    VX_CALL(tivxQueryNumTargetKernel(&num_target_kernels));

    for (i=num_target_kernels; i<TIVX_TARGET_KERNEL_MAX; i++) {
        kernel_id = (vx_enum)(i);
        ASSERT(NULL != (ttk = tivxAddTargetKernel(kernel_id, tname, p_function, c_function, NULL, NULL, (void *)(&priv_arg))));
    }

    kernel_id = TIVX_TARGET_KERNEL_MAX;

    /* Trying to allocate TIVX_TARGET_KERNEL_MAX+1 */
    ASSERT(NULL == (ttk = tivxAddTargetKernel(kernel_id, tname, p_function, c_function, NULL, NULL, (void *)(&priv_arg))));
}

TEST(tivxTgKnl, negativeTestRemoveTargetKernel)
{
    vx_context context = context_->vx_context_;

    tivx_target_kernel ttk = NULL;
    vx_uint32 ttkaddress = 0;

    ASSERT_EQ_VX_STATUS(VX_FAILURE, tivxRemoveTargetKernel(ttk));
    ASSERT_EQ_VX_STATUS(VX_FAILURE, tivxRemoveTargetKernel((tivx_target_kernel)(&ttkaddress)));
}

TESTCASE_TESTS(
    tivxTgKnl,
    negativeTestAddTargetKernelInternal,
    negativeTestRemoveTargetKernel
)

