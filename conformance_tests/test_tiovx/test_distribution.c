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

#include <math.h>
#include <VX/vx.h>
#include <VX/vxu.h>

#include "test_engine/test.h"

TESTCASE(tivxDistribution, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxDistribution, negativeTestQueryDistribution)
{
    #define VX_DISTRIBUTION_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_distribution dist = NULL;
    vx_enum attribute = VX_DISTRIBUTION_DEFAULT;
    vx_size num_bins = 1, size = 0;
    vx_int32 offset = 1;
    vx_uint32 range = 5, pvalue = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryDistribution(dist, attribute, &pvalue, size));
    ASSERT_VX_OBJECT(dist = vxCreateDistribution(context, num_bins, offset, range), VX_TYPE_DISTRIBUTION);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryDistribution(dist, VX_DISTRIBUTION_DIMENSIONS, &pvalue, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryDistribution(dist, VX_DISTRIBUTION_OFFSET, &pvalue, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryDistribution(dist, VX_DISTRIBUTION_RANGE, &pvalue, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryDistribution(dist, VX_DISTRIBUTION_BINS, &pvalue, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryDistribution(dist, VX_DISTRIBUTION_WINDOW, &pvalue, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryDistribution(dist, VX_DISTRIBUTION_SIZE, &pvalue, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryDistribution(dist, attribute, &pvalue, size));
    VX_CALL(vxReleaseDistribution(&dist));
}

TEST(tivxDistribution, negativeTestCopyDistribution)
{
    vx_context context = context_->vx_context_;

    vx_distribution dist = NULL;
    int32_t udata[256];
    vx_enum usage = VX_READ_ONLY, user_mem_type = VX_MEMORY_TYPE_NONE;
    vx_size num_bins = 1;
    vx_int32 offset = 1;
    vx_uint32 range = 5;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxCopyDistribution(dist, udata, usage, user_mem_type));
    ASSERT_VX_OBJECT(dist = vxCreateDistribution(context, num_bins, offset, range), VX_TYPE_DISTRIBUTION);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxCopyDistribution(dist, NULL, usage, user_mem_type));
    VX_CALL(vxReleaseDistribution(&dist));
}

TEST(tivxDistribution, negativeTestMapDistribution)
{
    vx_context context = context_->vx_context_;

    vx_distribution dist = NULL;
    vx_map_id mapid;
    int32_t udata[256];
    vx_enum usage = VX_READ_ONLY, user_mem_type = VX_MEMORY_TYPE_NONE;
    vx_bitfield bfield = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxMapDistribution(dist, &mapid, &udata, usage, user_mem_type, bfield));
}

TEST(tivxDistribution, negativeTestUnmapDistribution)
{
    vx_context context = context_->vx_context_;

    vx_distribution dist = NULL;
    vx_map_id mapid;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxUnmapDistribution(dist, mapid));
}

TESTCASE_TESTS(
    tivxDistribution,
    negativeTestQueryDistribution,
    negativeTestCopyDistribution,
    negativeTestMapDistribution,
    negativeTestUnmapDistribution
)

