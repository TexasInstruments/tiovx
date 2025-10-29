/*
 * Copyright (c) 2012-2025 The Khronos Group Inc.
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

#include "test_engine/test.h"
#include "test_tiovx/test_tiovx.h"

#include <VX/vx.h>
#include <TI/tivx_test_kernels.h>
#include <TI/tivx_target_kernel.h>
#include <test_kernels_utils.h>
#include "utils/include/tivx_utils.h"

TESTCASE(tivxFileio,  CT_VXContext, ct_setup_vx_context, 0)

TEST_WITH_ARG(tivxFileio, testFileio, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar;
    vx_user_data_object path;
    vx_node n0;
    vx_reference ref[1];
    const char *envPtr = tivx_utils_get_test_file_dir();

    ASSERT(envPtr != NULL);
    ASSERT(strlen(envPtr) > 0);

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar  = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(path  = vxCreateUserDataObject(context, "test_data_path", strlen(envPtr)+1, envPtr), VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(n0 = tivxFileioNode(graph, path), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n0, "Fileio Test Node");

    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, arg_->target_string));

    VX_CALL(vxVerifyGraph(graph));

    ref[0] = (vx_reference)scalar;

    tivxNodeSendCommand(n0, 0, 0, ref, 1u);

    VX_CALL(vxCopyScalar(scalar, &scalar_val, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    ASSERT(scalar_val==0);

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseUserDataObject(&path));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}

TESTCASE_TESTS(tivxFileio,
               testFileio)

