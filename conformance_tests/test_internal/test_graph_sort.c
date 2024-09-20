/*

 * Copyright (c) 2012-2017 The Khronos Group Inc.
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

#include "test_tiovx.h"
#include <vx_internal.h>

TESTCASE(tivxInternalGraphSort, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalGraphSort, negativeGraphTopologicalSort)
{
    ownGraphTopologicalSort(NULL, NULL, TIVX_GRAPH_MAX_NODES+1, 0);
}

TESTCASE_TESTS(tivxInternalGraphSort,
    negativeGraphTopologicalSort
    )