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
#include "test_tiovx/test_tiovx.h"

#include <VX/vx.h>
#include <VX/vxu.h>
#include <VX/vx_khr_pipelining.h>
#include <TI/tivx.h>
#include <TI/tivx_test_kernels.h>
#include <TI/tivx_config.h>
#include <TI/tivx_capture.h>
#include <TI/tivx_task.h>
#include <TI/tivx_target_kernel.h>
#include "math.h"
#include <limits.h>

#define MAX_LINE_LEN   (256U)
#define NUM_CAMERAS    (4U)

#define LOG_RT_TRACE_ENABLE       (0u)

#define MAX_NUM_BUF               (8u)
#define MAX_IMAGE_PLANES          (3u)
#define MAX_NUM_OBJ_ARR_ELEMENTS  (4u)

#define NODE1_EVENT     (1u)
#define NODE2_EVENT     (2u)

#define ADD_BUF_1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/buf=1", __VA_ARGS__, 1))

#define ADD_BUF_2(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/buf=2", __VA_ARGS__, 2))

#define ADD_BUF_3(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/buf=3", __VA_ARGS__, 3))

#define ADD_PIPE_1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pipe_depth=1", __VA_ARGS__, 1))

#define ADD_PIPE_3(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pipe_depth=3", __VA_ARGS__, 3))

#define ADD_PIPE_6(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pipe_depth=6", __VA_ARGS__, 6))

#define ADD_PIPE_MAX(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pipe_depth=MAX", __VA_ARGS__, TIVX_GRAPH_MAX_PIPELINE_DEPTH-1))

#define ADD_LOOP_0(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=0", __VA_ARGS__, 0))

#define ADD_LOOP_1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=1", __VA_ARGS__, 1))

#define ADD_LOOP_10(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=10", __VA_ARGS__, 10))

#define ADD_LOOP_100(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=100", __VA_ARGS__, 100))

#define ADD_LOOP_1000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=1000", __VA_ARGS__, 1000))

#define ADD_LOOP_100000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=100000", __VA_ARGS__, 100000))

#define ADD_LOOP_1000000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=1000000", __VA_ARGS__, 1000000))

#define ADD_STREAM_TIME_100(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/stream_time=100", __VA_ARGS__, 100))

#define ADD_STREAM_TIME_1000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/stream_time=1000", __VA_ARGS__, 1000))

#define ADD_STREAM_TIME_10000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/stream_time=10000", __VA_ARGS__, 10000))

#define MEASURE_PERF_OFF(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/measure_perf=OFF", __VA_ARGS__, 0))

#define MEASURE_PERF_ON(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/measure_perf=ON", __VA_ARGS__, 1))

#define ADD_SIZE_2048x1024(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=2048x1024", __VA_ARGS__, 2048, 1024))

typedef struct {
    const char* testName;
    int width, height;
    int pipe_depth;
    int num_buf;
    int measure_perf;
    char *target_string;
} Pipeline_Arg;

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_3, ADD_BUF_3, MEASURE_PERF_OFF, ADD_SET_TARGET_PARAMETERS, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_1, ADD_BUF_1, MEASURE_PERF_OFF, ADD_SET_TARGET_PARAMETERS, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_3, ADD_BUF_3, MEASURE_PERF_OFF, ADD_SET_TARGET_PARAMETERS, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_3, MEASURE_PERF_OFF, ADD_SET_TARGET_PARAMETERS, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_MAX, ADD_BUF_3, MEASURE_PERF_OFF, ADD_SET_TARGET_PARAMETERS, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_1, ADD_BUF_1, MEASURE_PERF_OFF, ADD_SET_TARGET_PARAMETERS, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_2, MEASURE_PERF_OFF, ADD_SET_TARGET_PARAMETERS, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_2, MEASURE_PERF_ON, ADD_SET_TARGET_PARAMETERS, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_2048x1024, ADD_PIPE_3, ADD_BUF_3, MEASURE_PERF_ON, ADD_SET_TARGET_PARAMETERS, ARG), \

#if defined(SOC_AM62A)
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MPU_0", __VA_ARGS__, TIVX_TARGET_MPU_0)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MCU1_0", __VA_ARGS__, TIVX_TARGET_MCU1_0)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP1", __VA_ARGS__, TIVX_TARGET_DSP1))
#elif defined(SOC_J721E)
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MPU_0", __VA_ARGS__, TIVX_TARGET_MPU_0)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MCU2_0", __VA_ARGS__, TIVX_TARGET_MCU2_0)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP1", __VA_ARGS__, TIVX_TARGET_DSP1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP_C7_1", __VA_ARGS__, TIVX_TARGET_DSP_C7_1))
#else
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MPU_0", __VA_ARGS__, TIVX_TARGET_MPU_0)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MCU2_0", __VA_ARGS__, TIVX_TARGET_MCU2_0)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP1", __VA_ARGS__, TIVX_TARGET_DSP1))
#endif

#if defined(SOC_AM62A)
#define ADD_SET_ADDITIONAL_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MCU1_0", __VA_ARGS__, TIVX_TARGET_MCU1_0))
#elif defined(SOC_J722S)
#define ADD_SET_ADDITIONAL_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MCU2_0", __VA_ARGS__, TIVX_TARGET_MCU2_0))
#else
#define ADD_SET_ADDITIONAL_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MCU2_1", __VA_ARGS__, TIVX_TARGET_MCU2_1))
#endif

typedef struct {
    const char* testName;
    int stream_time;
    char *target_string;
    char *additional_target_string;
} Arg;

#define STREAMING_PARAMETERS \
    CT_GENERATE_PARAMETERS("streaming", ADD_SET_TARGET_PARAMETERS, ADD_SET_ADDITIONAL_TARGET_PARAMETERS, ARG, 1000)
