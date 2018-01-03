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

void printPerformance(vx_perf_t performance, uint32_t numPixels, const char* testName)
{
    printf("[ %c%c ] Execution time for %9d pixels (avg = %4d.%-6d ms, sum = %4d.%-6d ms, min = %4d.%-6d ms, max = %4d.%-6d ms, num = %d)\n",
        testName[0], testName[1],
        numPixels,
        (uint32_t)(performance.avg/1000000),
        (uint32_t)(performance.avg%1000000),
        (uint32_t)(performance.sum/1000000),
        (uint32_t)(performance.sum%1000000),
        (uint32_t)(performance.min/1000000),
        (uint32_t)(performance.min%1000000),
        (uint32_t)(performance.max/1000000),
        (uint32_t)(performance.max%1000000),
        (uint32_t)(performance.num)
        );
}
