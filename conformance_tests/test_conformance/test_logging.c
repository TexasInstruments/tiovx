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

#include "test_engine/test.h"
#include <VX/vx.h>
#include <VX/vxu.h>
#include <string.h>

TESTCASE(Logging, CT_VoidContext, 0, 0)

static vx_bool log_callback_is_called = vx_false_e;
static void VX_CALLBACK test_log_callback(vx_context context, vx_reference ref, vx_status status, const vx_char string[])
{
    log_callback_is_called = vx_true_e;
    ASSERT_(printf("\tActual: %d < %d\n\n", (int)strlen(string), (int)VX_MAX_LOG_MESSAGE_LEN), strlen(string) < VX_MAX_LOG_MESSAGE_LEN);
}

TEST(Logging, Cummulative)
{
    vx_image image = 0;
    vx_context context = vxCreateContext();

    ASSERT_VX_OBJECT(context, VX_TYPE_CONTEXT);
    CT_RegisterForGarbageCollection(context, ct_destroy_vx_context, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    // normall logging
    vxRegisterLogCallback(context, test_log_callback, vx_false_e);
    log_callback_is_called = vx_false_e;
    vxAddLogEntry((vx_reference)image, VX_FAILURE, "hello world", 1, 2, 3);
    ASSERT(log_callback_is_called);

    // clear callback
    vxRegisterLogCallback(context, NULL, vx_true_e);
    log_callback_is_called = vx_false_e;
    vxAddLogEntry((vx_reference)image, VX_FAILURE, "hello world", 4, 5, 6);
    ASSERT(!log_callback_is_called);

    // restore callback
    vxRegisterLogCallback(context, test_log_callback, vx_true_e);

    // disable logs for image
    VX_CALL(vxDirective((vx_reference)image, VX_DIRECTIVE_DISABLE_LOGGING));
    log_callback_is_called = vx_false_e;
    vxAddLogEntry((vx_reference)image, VX_FAILURE, "hello world", 4, 5, 6);
    ASSERT(!log_callback_is_called);

    // turn on logs once again
    VX_CALL(vxDirective((vx_reference)image, VX_DIRECTIVE_ENABLE_LOGGING));
    log_callback_is_called = vx_false_e;
    vxAddLogEntry((vx_reference)image, VX_FAILURE, "%*s", VX_MAX_LOG_MESSAGE_LEN + 20, ""); // 20 symbols longer string than limit
    ASSERT(log_callback_is_called);

    VX_CALL(vxReleaseImage(&image));
    ASSERT(image == 0);
}


TESTCASE_TESTS(Logging,
        Cummulative
        )
