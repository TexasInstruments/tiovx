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
#include <TI/tivx.h>

#define ADD_SIZE_18x18(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=18x18", __VA_ARGS__, 18, 18))

#define ADD_SIZE_640x480(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=640x480", __VA_ARGS__, 640, 480))

#define ADD_SIZE_644x258(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=644x258", __VA_ARGS__, 644, 258))

#define ADD_SIZE_1600x1200(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=1600x1200", __VA_ARGS__, 1600, 1200))

#define ADD_VX_BORDERS_REQUIRE_REPLICATE_ONLY(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_REPLICATE", __VA_ARGS__, { VX_BORDER_REPLICATE, {{ 0 }} }))

#define ADD_VX_BORDERS_REQUIRE_CONSTANT_ONLY(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_CONSTANT=0", __VA_ARGS__, { VX_BORDER_CONSTANT, {{ 0 }} }))


#define __STDC_FORMAT_MACROS
#include <inttypes.h>

void printPerformance(vx_perf_t performance, uint32_t numPixels, const char* testName);

