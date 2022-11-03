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

#include "test_engine/test.h"

TESTCASE(tivxObjArray, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxObjArray, negativeTestCreateObjectArray)
{
    vx_context context = context_->vx_context_;

    vx_object_array vxoa = NULL;
    vx_convolution cnvl = NULL;
    vx_reference exemplar = NULL;
    vx_size count = 0, columns = 5, rows = 5;

    ASSERT(NULL == (vxoa = vxCreateObjectArray(NULL, exemplar, count)));
    ASSERT_VX_OBJECT(cnvl = vxCreateConvolution(context, columns, rows), VX_TYPE_CONVOLUTION);
    exemplar = (vx_reference)(cnvl);
    ASSERT(NULL == (vxoa = vxCreateObjectArray(context, exemplar, count)));
    VX_CALL(vxReleaseConvolution(&cnvl));
}

TEST(tivxObjArray, negativeTestCreateVirtualObjectArray)
{
    vx_context context = context_->vx_context_;

    vx_object_array vxoa = NULL;
    vx_graph graph = NULL;
    vx_convolution cnvl = NULL;
    vx_reference exemplar = NULL;
    vx_size count = 0, columns = 5, rows = 5;

    ASSERT(NULL == (vxoa = vxCreateVirtualObjectArray(graph, exemplar, count)));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(cnvl = vxCreateConvolution(context, columns, rows), VX_TYPE_CONVOLUTION);
    exemplar = (vx_reference)(cnvl);
    ASSERT(NULL == (vxoa = vxCreateVirtualObjectArray(graph, exemplar, count)));
    VX_CALL(vxReleaseConvolution(&cnvl));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxObjArray, negativeTestGetObjectArrayItem)
{
    vx_context context = context_->vx_context_;

    vx_object_array vxoa = NULL;
    vx_reference ref = NULL;
    vx_uint32 index = 0;

    ASSERT(NULL != (ref = vxGetObjectArrayItem(vxoa, index)));
}

TEST(tivxObjArray, negativeTestQueryObjectArray)
{
    #define VX_OBJECT_ARRAY_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_object_array vxoa = NULL;
    vx_image img = NULL;
    vx_enum attribute = VX_OBJECT_ARRAY_DEFAULT;
    vx_uint32 udata = 0;
    vx_size size = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryObjectArray(vxoa, VX_OBJECT_ARRAY_ITEMTYPE, &udata, size));
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(vxoa = vxCreateObjectArray(context, (vx_reference)(img), 1), VX_TYPE_OBJECT_ARRAY);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryObjectArray(vxoa, VX_OBJECT_ARRAY_ITEMTYPE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryObjectArray(vxoa, VX_OBJECT_ARRAY_NUMITEMS, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryObjectArray(vxoa, attribute, &udata, size));
    VX_CALL(vxReleaseObjectArray(&vxoa));
    VX_CALL(vxReleaseImage(&img));
}

TESTCASE_TESTS(
    tivxObjArray,
    negativeTestCreateObjectArray,
    negativeTestCreateVirtualObjectArray,
    negativeTestGetObjectArrayItem,
    negativeTestQueryObjectArray
)

