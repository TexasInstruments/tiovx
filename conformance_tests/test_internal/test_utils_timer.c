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
#include <TI/tivx_debug.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include "TI/tivx.h"
#include <VX/vx_types.h>
#include <utils/mem/include/app_mem.h>

#if defined(A72) || defined(A53)
#include <utils/timer/include/app_timer.h>

TESTCASE(tivxTimer, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxTimer, testappLogWaitMsecs)
{
    uint32_t time_in_msecs = 100U;

    appLogWaitMsecs(time_in_msecs);
}

TESTCASE_TESTS(
    tivxTimer,
    testappLogWaitMsecs
    )
#endif
