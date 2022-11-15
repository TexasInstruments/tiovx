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

#include <VX/vx.h>
#include <TI/tivx_debug.h>

#include "test_engine/test.h"

TESTCASE(tivxDebug, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxDebug, negativeTestSetDebugZone)
{
    vx_context context = context_->vx_context_;

    vx_enum zone = -1;

    tivx_set_debug_zone(zone);
    tivx_set_debug_zone(VX_ZONE_MAX);
}

TEST(tivxDebug, negativeTestClrDebugZone)
{
    vx_context context = context_->vx_context_;

    vx_enum zone = -1;

    tivx_clr_debug_zone(zone);
    tivx_clr_debug_zone(VX_ZONE_MAX);
}

TEST(tivxDebug, negativeTestGetDebugZone)
{
    vx_context context = context_->vx_context_;

    vx_enum zone = -1;

    ASSERT(vx_false_e == tivx_get_debug_zone(zone));
    ASSERT(vx_false_e == tivx_get_debug_zone(VX_ZONE_MAX));
    ASSERT(vx_false_e == tivx_get_debug_zone(0));
    ASSERT(vx_true_e == tivx_get_debug_zone(1));
}

TESTCASE_TESTS(
    tivxDebug,
    negativeTestSetDebugZone,
    negativeTestClrDebugZone,
    negativeTestGetDebugZone
)

