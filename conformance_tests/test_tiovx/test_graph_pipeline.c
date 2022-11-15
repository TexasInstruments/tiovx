/*
 * Copyright (c) 2012-2018 The Khronos Group Inc.
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
#include <VX/vx.h>
#include <VX/vxu.h>
#include <VX/vx_khr_pipelining.h>
#include <TI/tivx.h>
#include <TI/tivx_config.h>
#include "math.h"
#include <limits.h>
#include <TI/tivx_test_kernels.h>
#include <TI/tivx_capture.h>

TESTCASE(tivxGraphPipeline,  CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(tivxGraphPipelineLdra,  CT_VXContext, ct_setup_vx_context, 0)

#define LOG_RT_TRACE_ENABLE       (0u)

#define MAX_NUM_BUF               (8u)
#define MAX_IMAGE_PLANES          (3u)
#define MAX_NUM_OBJ_ARR_ELEMENTS  (4u)
#define MAX_NUM_PYR_ELEMENTS  (4u)

#define GRAPH_CONSUMED_EVENT      (1u)
#define NODE0_COMPLETED_EVENT     (2u)
#define NODE1_COMPLETED_EVENT     (3u)
#define GRAPH_COMPLETED_EVENT     (4u)

typedef struct {
    const char* testName;
    int width, height;
    int pipe_depth;
    int num_buf;
    int loop_count;
    int measure_perf;
} Arg;

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

#define ADD_LOOP_50(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=50", __VA_ARGS__, 50))

#define ADD_LOOP_1000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=1000", __VA_ARGS__, 1000))

#define ADD_LOOP_100000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=100000", __VA_ARGS__, 100000))

#define ADD_LOOP_1000000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=1000000", __VA_ARGS__, 1000000))

#define MEASURE_PERF_OFF(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/measure_perf=OFF", __VA_ARGS__, 0))

#define MEASURE_PERF_ON(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/measure_perf=ON", __VA_ARGS__, 1))

#define ADD_SIZE_2048x1024(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=2048x1024", __VA_ARGS__, 2048, 1024))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_256x256, ADD_PIPE_3, ADD_BUF_3, ADD_LOOP_10, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_1, ADD_BUF_1, ADD_LOOP_0, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_3, ADD_BUF_3, ADD_LOOP_1, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_3, ADD_LOOP_50, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_MAX, ADD_BUF_3, ADD_LOOP_50, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_1, ADD_BUF_1, ADD_LOOP_50, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_2, ADD_LOOP_50, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_2, ADD_LOOP_50, MEASURE_PERF_ON, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_2048x1024, ADD_PIPE_3, ADD_BUF_3, ADD_LOOP_50, MEASURE_PERF_ON, ARG), \

#if 0
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_3, ADD_BUF_3, ADD_LOOP_0, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_1, ADD_BUF_1, ADD_LOOP_0, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_3, ADD_BUF_3, ADD_LOOP_1, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_3, ADD_LOOP_1000, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_1, ADD_BUF_1, ADD_LOOP_1000, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_2, ADD_LOOP_1000, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_2, ADD_LOOP_100000, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_2, ADD_LOOP_100000, MEASURE_PERF_ON, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_2, ADD_LOOP_1000000, MEASURE_PERF_ON, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_2048x1024, ADD_PIPE_3, ADD_BUF_3, ADD_LOOP_1000, MEASURE_PERF_ON, ARG), \

#endif

/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

/*
 * Utility API to set number of buffers at a node parameter
 * The parameter MUST be a output or bidirectonal parameter for the setting
 * to take effect
 */
static vx_status set_num_buf_by_node_index(vx_node node, vx_uint32 node_parameter_index, vx_uint32 num_buf)
{
    return tivxSetNodeParameterNumBufByIndex(node, node_parameter_index, num_buf);
}

/*
 * Utility API to set pipeline depth for a graph
 */
static vx_status set_graph_pipeline_depth(vx_graph graph, vx_uint32 pipeline_depth)
{
    return tivxSetGraphPipelineDepth(graph, pipeline_depth);
}

/*
 * Utility API to export graph information to file for debug and visualization
 */
static vx_status export_graph_to_file(vx_graph graph, char *filename_prefix)
{
    size_t sz = 0;
    void* buf = 0;
    char filepath[MAXPATHLENGTH];

    sz = snprintf(filepath, MAXPATHLENGTH, "%s/output", ct_get_test_file_path());
    ASSERT_(return 0, (sz < MAXPATHLENGTH));
    return tivxExportGraphToDot(graph, filepath, filename_prefix);
}

/*
 * Utility API to log graph run-time trace
 */
static vx_status log_graph_rt_trace(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    #if LOG_RT_TRACE_ENABLE
    /* If run time logging is needed, update to use tivxLogRtTraceEnable,
     * tivxLogRtTraceDisable and tivxLogRtTraceExportToFile */
    #endif
    return status;
}

static void printGraphPipelinePerformance(vx_graph graph,
            vx_node nodes[], uint32_t num_nodes,
            uint64_t exe_time, uint32_t loop_cnt, uint32_t numPixels)
{
    #define MAX_TEST_NAME (8u)

    vx_perf_t perf_ref;
    char ref_name[MAX_TEST_NAME];
    uint32_t i;
    uint64_t avg_exe_time;

    avg_exe_time = exe_time / loop_cnt;

    for(i=0; i<num_nodes; i++)
    {
         vxQueryNode(nodes[i], VX_NODE_PERFORMANCE, &perf_ref, sizeof(perf_ref));
         snprintf(ref_name,MAX_TEST_NAME, "N%d ", i);
         printPerformance(perf_ref, numPixels, ref_name);
    }

    #if 0
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_ref, sizeof(perf_ref));
    snprintf(ref_name,MAX_TEST_NAME, "G0 ");
    printPerformance(perf_ref, numPixels, ref_name);
    #endif

    printf("[ SYS ] Execution time (avg = %4d.%03d ms, sum = %4d.%03d ms, num = %d)\n",
        (uint32_t)(avg_exe_time/1000u), (uint32_t)(avg_exe_time%1000u),
        (uint32_t)(exe_time/1000u), (uint32_t)(exe_time%1000u),
        loop_cnt
        );
}


static void referenceNot(CT_Image src, CT_Image dst)
{
    uint32_t i, j;

    ASSERT(src && dst);
    ASSERT(src->width == dst->width);
    ASSERT(src->height == dst->height);
    ASSERT(src->format == dst->format && src->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = ~src->data.y[i * src->stride + j];
}

static void reference_mean_stddev(CT_Image src, vx_float32* _mean, vx_float32* _stddev)
{
    uint32_t x, y, width = src ? src->width : 0, height = src ? src->height : 0;
    uint32_t npix, stride;
    double sum = 0, sqsum = 0;
    int format = src ? src->format : VX_DF_IMAGE_U8;

    ASSERT(src);
    ASSERT(src->width > 0 && src->height > 0);
    npix = width*height;
    stride = ct_stride_bytes(src);

#define CASE_MEANSTDDEV(format, type, acctype) \
    case format: \
    { \
        acctype s = 0, s2 = 0; \
        for( y = 0; y < src->height; y++ ) \
        { \
            const type* ptr = (const type*)(src->data.y + stride*y); \
            for( x = 0; x < src->width; x++ ) \
            { \
                type val = ptr[x]; \
                s += val; \
                s2 += (acctype)val*val; \
            } \
        } \
        sum = (double)s; sqsum = (double)s2; \
    } \
    break

    switch(format)
    {
    CASE_MEANSTDDEV(VX_DF_IMAGE_U8, uint8_t, uint64_t);
    default:
        FAIL("Unsupported image format: (%d)", &src->format);
    }

    *_mean = (vx_float32)(sum/npix);
    sqsum = sqsum/npix - (sum/npix)*(sum/npix);
    *_stddev = (vx_float32)sqrt(CT_MAX(sqsum, 0.));
}

static void fillSequence(CT_Image dst, uint32_t seq_init)
{
    uint32_t i, j;
    uint32_t val = seq_init;

    ASSERT(dst);
    ASSERT(dst->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = val;
}

#define TEST_USER_KERNEL_NAME          "test_graph_pipeline.user_kernel"
#define TEST_USER_KERNEL_NUM_PARAMS     (4u)
static vx_kernel test_user_kernel = NULL;

static vx_status test_user_kernel_validate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_scalar scalar[TEST_USER_KERNEL_NUM_PARAMS];
    vx_enum scalar_type[TEST_USER_KERNEL_NUM_PARAMS];
    vx_uint32 i;

    if (num != TEST_USER_KERNEL_NUM_PARAMS)
    {
        printf(" ERROR: Test user kernel: Number of parameters dont match !!!\n");
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    for (i = 0U; i < TEST_USER_KERNEL_NUM_PARAMS; i ++)
    {
        scalar[i] = (vx_scalar)parameters[i];

        if(scalar[i] != NULL)
        {
            /* i.e not a optional parameter */
            status = vxQueryScalar(scalar[i],
                VX_SCALAR_TYPE, &scalar_type[i],
                sizeof(vx_enum));
            if(status==VX_SUCCESS)
            {
                if(scalar_type[i] != VX_TYPE_UINT32)
                {
                    printf(" ERROR: Test user kernel: Scalar type MUST be VX_TYPE_UINT32 !!!\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                vxSetMetaFormatAttribute(metas[i], VX_SCALAR_TYPE, &scalar_type[i],
                    sizeof(scalar_type[i]));
            }
            if(status!=VX_SUCCESS)
            {
                printf(" ERROR: Test user kernel: validate failed !!!\n");
                break;
            }
        }
    }

    return status;
}

static vx_status test_user_kernel_run(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_scalar in1, in2, out1, out2;
    vx_uint32 in1_value = 0, in2_value = 0;
    vx_uint32 out1_value = 0, out2_value = 0;

    /* Any of the parameter can be NULL since parameter is marked
     * as optional during kernel register */
    in1  = (vx_scalar)parameters[0];
    in2  = (vx_scalar)parameters[1];
    out1 = (vx_scalar)parameters[2];
    out2 = (vx_scalar)parameters[3];

    if(in1!=NULL)
    {
        vxCopyScalar(in1,
            &in1_value,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST
            );
    }
    if(in2!=NULL)
    {
        vxCopyScalar(in2,
            &in2_value,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST
            );
    }

    /* just for test
     * out1_value = in1_value + in2_value
     * out2_value = out1_value * 2
     * when in1 reference is not specified (since its optional), in1_value is considered to be 0
     * when in2 reference is not specified (since its optional), in2_value is considered to be 0
     */
    out1_value = in1_value + in2_value;
    out2_value = out1_value*2;

    if(out1!=NULL)
    {
        vxCopyScalar(out1,
            &out1_value,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST
            );
    }
    if(out2!=NULL)
    {
        vxCopyScalar(out2,
            &out2_value,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST
            );
    }

    return status;
}

static vx_status test_user_kernel_register(vx_context context)
{
    vx_kernel kernel = NULL;
    vx_status status;
    uint32_t index;
    vx_enum test_user_kernel_id = 0;

    status = vxAllocateUserKernelId(context, &test_user_kernel_id);
    if(status!=VX_SUCCESS)
    {
        printf(" ERROR: Test user kernel: vxAllocateUserKernelId failed (%d)!!!\n", status);
    }
    if(status==VX_SUCCESS)
    {
        kernel = vxAddUserKernel(
                    context,
                    TEST_USER_KERNEL_NAME,
                    test_user_kernel_id,
                    test_user_kernel_run,
                    TEST_USER_KERNEL_NUM_PARAMS, /* number of parameters objects for this user function */
                    test_user_kernel_validate,
                    NULL,
                    NULL);
    }

    status = vxGetStatus((vx_reference)kernel);
    if ( status == VX_SUCCESS)
    {
        index = 0;

        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_INPUT,
                VX_TYPE_SCALAR,
                VX_PARAMETER_STATE_OPTIONAL
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_INPUT,
                VX_TYPE_SCALAR,
                VX_PARAMETER_STATE_OPTIONAL
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_OUTPUT,
                VX_TYPE_SCALAR,
                VX_PARAMETER_STATE_OPTIONAL
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_OUTPUT,
                VX_TYPE_SCALAR,
                VX_PARAMETER_STATE_OPTIONAL
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        if( status != VX_SUCCESS)
        {
            printf(" ERROR: Test user kernel: vxAddParameterToKernel, vxFinalizeKernel failed (%d)!!!\n", status);
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
        printf(" ERROR: Test user kernel: vxAddUserKernel failed (%d)!!!\n", status);
    }
    if(status==VX_SUCCESS)
    {
        test_user_kernel = kernel;
    }

    return status;
}

static vx_status test_user_kernel_unregister(vx_context context)
{
    vx_status status;

    status = vxRemoveKernel(test_user_kernel);
    test_user_kernel = NULL;

    if(status!=VX_SUCCESS)
    {
        printf(" ERROR: Test user kernel: Unable to remove kernel (%d)!!!\n", status);
    }

    return status;
}

static vx_node test_user_kernel_node(vx_graph graph,
            vx_scalar in1,
            vx_scalar in2,
            vx_scalar out1,
            vx_scalar out2)
{
    vx_node node;

    vx_reference refs[] = {
        (vx_reference)in1,
        (vx_reference)in2,
        (vx_reference)out1,
        (vx_reference)out2};

    node = tivxCreateNodeByKernelName(graph,
                TEST_USER_KERNEL_NAME,
                refs, sizeof(refs)/sizeof(refs[0])
                );

    return node;
}

/*
 *  d0      n0     d2
 *  IMG --  OR -- IMG (*)
 *  (*)     |
 *          d1 (single ref)
 *
 * (*) = queueing enabled
 *
 * This test case test the below
 * - A data reference on which queing is not enabled
 * - No looping
 * - fixed pipeline depth of 2
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testOneNode, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1, d2[MAX_NUM_BUF] = {NULL};
    vx_node n0;

    CT_Image ref_src[MAX_NUM_BUF], vxdst;
    uint32_t width, height, seq_init, num_buf, pipeline_depth;
    uint32_t buf_id, loop_id, loop_cnt;
    uint64_t exe_time;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = arg_->width;
    height = arg_->height;
    loop_cnt = arg_->loop_count;
    pipeline_depth = 2;
    num_buf = 2;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE({
            ref_src[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
            fillSequence(ref_src[buf_id], (uint32_t)(seq_init+buf_id*10));
        });
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    /* create other refs, these are not multiple refs and same refs is fed as parameter to the graph */
    ASSERT_VX_OBJECT(d1    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    /* fill d1 with zero's so that OR operation acts like a NOP */
    {
        vx_imagepatch_addressing_t addr;
        vx_rectangle_t rect;
        void *ptr;
        vx_map_id map_id;

        rect.start_x = rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        VX_CALL(vxMapImagePatch(d1, &rect, 0, &map_id, &addr, &ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X));

        ct_memset(ptr, 0x0, addr.stride_y*addr.dim_y);

        VX_CALL(vxUnmapImagePatch(d1, map_id));
    }

    /* create node, input (index 0) and output (index 2) will be made as graph parameter
     * so that we can enqueue and dequeue refs to it and thus do graph pipelining.
     * d0[0], d2[0] used only for their meta data.
     * Actual input and output used for graph processing will be the
     * refs that are enqueued later
     */
    ASSERT_VX_OBJECT(n0    = vxOrNode(graph, d0[0], d1, d2[0]), VX_TYPE_NODE);

    /* input @ node index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ node index 2, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 2);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_one_node");
    log_graph_rt_trace(graph);

    #if 1
    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(d0[buf_id], ref_src[buf_id]));
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d2[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_image out_img, in_img;
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_img, 1, &num_refs));
        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_img, 1, &num_refs));

        /* A graph execution completed, since we dequeued both input and output refs */
        if(arg_->measure_perf==0)
        {
            /* when measuring performance dont check output since it affects graph performance numbers
             */

            if(loop_cnt > 100)
            {
                ct_update_progress(loop_id, loop_cnt+num_buf);
            }

            ASSERT_NO_FAILURE({
                vxdst = ct_image_from_vx_image(out_img);
            });

            /* compare output */
            ASSERT_EQ_CTIMAGE(ref_src[buf_id], vxdst);
        }

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_img, 1));
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0 };

        printGraphPipelinePerformance(graph, nodes, 1, exe_time, loop_cnt+num_buf, arg_->width*arg_->height);
    }
    #endif

    VX_CALL(vxReleaseNode(&n0));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d2[buf_id]));
    }
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *  d0     n0     d1     n1     d2
 * IMG -- NOT -- IMG -- NOT -- IMG
 *
 * This test case test the below
 * - Single input, single output nodes
 * - Two nodes on two different targets
 * - Number of buffers = pipeline depth
 * - Virtual objects, no hints provided except for pipeline depth
 * - fixed pipeline depth of 2
 * Note: The d1 buffer is not accessible by the application in this
 *       example.  If the application needs access to this buffer,
 *       please refer to the testTwoNodes as an example.
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testTwoNodesBasic, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1, d2[MAX_NUM_BUF] = {NULL};
    vx_node n0, n1;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

    CT_Image ref_src[MAX_NUM_BUF], vxdst;
    uint32_t width, height, seq_init, pipeline_depth;
    uint32_t buf_id, loop_id, loop_cnt, num_buf;
    uint64_t exe_time;


    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = arg_->width;
    height = arg_->height;
    loop_cnt = arg_->loop_count;
    num_buf = 2;
    pipeline_depth = 2;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE({
            ref_src[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
            fillSequence(ref_src[buf_id], (uint32_t)(seq_init+buf_id*10));
        });
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(d1    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(n0    = vxNotNode(graph, d0[0], d1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, d1, d2[0]), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));   
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    #endif

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ n1 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_two_nodes_basic");
    log_graph_rt_trace(graph);
    #if 1

    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(d0[buf_id], ref_src[buf_id]));
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d2[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_image out_img, in_img;
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_img, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_img, 1, &num_refs));

        /* A graph execution completed, since we dequeued both input and output refs */
        if(arg_->measure_perf==0)
        {
            /* when measuring performance dont check output since it affects graph performance numbers
             */

            if(loop_cnt > 100)
            {
                ct_update_progress(loop_id, loop_cnt+num_buf);
            }

            ASSERT_NO_FAILURE({
                vxdst = ct_image_from_vx_image(out_img);
            });

            /* compare output */
            ASSERT_EQ_CTIMAGE(ref_src[buf_id], vxdst);
        }

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_img, 1);
            vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_img, 1);
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0, n1 };

        printGraphPipelinePerformance(graph, nodes, 2, exe_time, loop_cnt+num_buf, arg_->width*arg_->height);
    }
    #endif

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d2[buf_id]));
    }
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *  d0     n0     d1     n1     d2
 * IMG -- NOT -- IMG -- NOT -- IMG
 *  d0     n2     d3     n3     d4
 * IMG -- NOT -- IMG -- NOT -- IMG
 *
 * TIOVX-808
 * This test case test the below
 * - Single input, single output nodes
 * - Two nodes on two different targets
 * - Number of buffers = pipeline depth
 * - Virtual objects, no hints provided except for pipeline depth
 * - fixed pipeline depth of 2
 * Note: The d1 buffer is not accessible by the application in this
 *       example.  If the application needs access to this buffer,
 *       please refer to the testTwoNodes as an example.
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testInputMultipleEnqueue, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1, d2[MAX_NUM_BUF] = {NULL}, d3, d4[MAX_NUM_BUF] = {NULL};
    vx_node n0, n1, n2, n3;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

    CT_Image ref_src[MAX_NUM_BUF], vxdst0, vxdst1;
    uint32_t width, height, seq_init, pipeline_depth;
    uint32_t buf_id, loop_id, loop_cnt, num_buf;
    uint64_t exe_time;


    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = arg_->width;
    height = arg_->height;
    loop_cnt = arg_->loop_count;
    num_buf = 2;
    pipeline_depth = 2;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE({
            ref_src[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
            fillSequence(ref_src[buf_id], (uint32_t)(seq_init+buf_id*10));
        });
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d4[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(d1    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d3    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(n0    = vxNotNode(graph, d0[0], d1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, d1, d2[0]), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n2    = vxNotNode(graph, d0[0], d3), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n3    = vxNotNode(graph, d3, d4[0]), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    #endif

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ n1 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 1);
    /* output @ n1 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n3, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    graph_parameters_queue_params_list[2].graph_parameter_index = 2;
    graph_parameters_queue_params_list[2].refs_list_size = num_buf;
    graph_parameters_queue_params_list[2].refs_list = (vx_reference*)&d4[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                3,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_input_multiple_enqueue");
    log_graph_rt_trace(graph);
    #if 1

    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(d0[buf_id], ref_src[buf_id]));
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d2[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&d4[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_image out_img_n1, out_img_n3, in_img;
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 2, (vx_reference*)&out_img_n3, 1, &num_refs));

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_img_n1, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_img, 1, &num_refs));

        /* A graph execution completed, since we dequeued both input and output refs */
        if(arg_->measure_perf==0)
        {
            /* when measuring performance dont check output since it affects graph performance numbers
             */

            if(loop_cnt > 100)
            {
                ct_update_progress(loop_id, loop_cnt+num_buf);
            }

            ASSERT_NO_FAILURE({
                vxdst0 = ct_image_from_vx_image(out_img_n1);
            });

            ASSERT_NO_FAILURE({
                vxdst1 = ct_image_from_vx_image(out_img_n3);
            });

            /* compare output */
            ASSERT_EQ_CTIMAGE(ref_src[buf_id], vxdst0);

            ASSERT_EQ_CTIMAGE(ref_src[buf_id], vxdst1);
        }

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_img, 1);
            vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_img_n1, 1);
            vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&out_img_n3, 1);
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0, n1, n2, n3 };

        printGraphPipelinePerformance(graph, nodes, 4, exe_time, loop_cnt+num_buf, arg_->width*arg_->height);
    }
    #endif

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n3));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d2[buf_id]));
        VX_CALL(vxReleaseImage(&d4[buf_id]));
    }
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseImage(&d3));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *  d0     n0     d1     n1     d2
 * IMG -- NOT -- IMG -- NOT -- IMG
 *
 * This test case test the below
 * - Single input, single output nodes
 * - Two nodes on two different targets
 * Note: a graph parameter is created from the d1 buffer queue in this application
 *       This is only necessary in the case that the application must access the data
 *       for this buffer.  In the case that this is not needed, please refer to 
 *       testTwoNodesBasic.
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testTwoNodes, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1[MAX_NUM_BUF] = {NULL}, d2[MAX_NUM_BUF] = {NULL};
    vx_node n0, n1;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

    CT_Image ref_src[MAX_NUM_BUF] = {NULL}, vxdst;
    uint32_t width, height, seq_init, pipeline_depth, num_buf;
    uint32_t buf_id, loop_id, loop_cnt;
    uint64_t exe_time;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = arg_->width;
    height = arg_->height;
    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;
    loop_cnt = arg_->loop_count;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE({
            ref_src[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
            fillSequence(ref_src[buf_id], (uint32_t)(seq_init+buf_id*10));
        });
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d1[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(n0    = vxNotNode(graph, d0[0], d1[0]), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, d1[0], d2[0]), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    #endif

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* intermediate output @ n0 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 1);
    /* output @ n1 index 1, becomes graph parameter 2 */
    add_graph_parameter_by_node_index(graph, n1, 1);

    /* set graph schedule config such that graph parameter @ index 0, 1, 2 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d1[0];

    graph_parameters_queue_params_list[2].graph_parameter_index = 2;
    graph_parameters_queue_params_list[2].refs_list_size = num_buf;
    graph_parameters_queue_params_list[2].refs_list = (vx_reference*)&d2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                3,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_two_nodes");
    log_graph_rt_trace(graph);

    #if 1
    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(d0[buf_id], ref_src[buf_id]));
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d1[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&d2[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_image out_img, in_img, intermediate_img;
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 2, (vx_reference*)&out_img, 1, &num_refs));

        /* Get consumed intermediate reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&intermediate_img, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_img, 1, &num_refs));

        /* A graph execution completed, since we dequeued both input and output refs */
        if(arg_->measure_perf==0)
        {
            /* when measuring performance dont check output since it affects graph performance numbers
             */

            if(loop_cnt > 100)
            {
                ct_update_progress(loop_id, loop_cnt+num_buf);
            }

            ASSERT_NO_FAILURE({
                vxdst = ct_image_from_vx_image(out_img);
            });

            /* compare output */
            ASSERT_EQ_CTIMAGE(ref_src[buf_id], vxdst);
        }

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&intermediate_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&out_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_img, 1));
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0, n1 };

        printGraphPipelinePerformance(graph, nodes, 2, exe_time, loop_cnt+num_buf, arg_->width*arg_->height);
    }
    #endif

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d1[buf_id]));
        VX_CALL(vxReleaseImage(&d2[buf_id]));
    }
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *  d0     n0     d1     n1     d2    n2     d3     n3      d5
 * IMG -- NOT -- IMG -- NOT -- IMG -- OR  -- IMG -- AND -- IMG
 *                |                   |              |
 *                +-------------------+             IMG
 *                                                   d4
 *
 * This test case test the below
 * - Same input going to multiple nodes
 * - Outputs from multiple nodes going to a single node
 * - Node taking input from another node as well as from user
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testFourNodes, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1, d2, d3, d4[MAX_NUM_BUF] = {NULL}, d5[MAX_NUM_BUF] = {NULL};
    vx_node  n0, n1, n2, n3;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

    CT_Image ref_src[MAX_NUM_BUF] = {NULL}, vxdst;
    uint32_t width, height, seq_init, pipeline_depth, num_buf, tmp_num_buf, get_num_buf;
    uint32_t buf_id, loop_id, loop_cnt;
    uint64_t exe_time;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = arg_->width;
    height = arg_->height;
    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;
    loop_cnt = arg_->loop_count;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE({
            ref_src[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
            fillSequence(ref_src[buf_id], (uint32_t)(seq_init+buf_id*10));
        });
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d4[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d5[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(d1    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d2    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d3    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(n0    = vxNotNode(graph, d0[0], d1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, d1, d2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n2    = vxOrNode(graph, d1, d2, d3), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n3    = vxAndNode(graph, d3, d4[0], d5[0]), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    #endif

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* input @ n3 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n3, 1);
    /* output @ n3 index 2, becomes graph parameter 2 */
    add_graph_parameter_by_node_index(graph, n3, 2);

    /* set graph schedule config such that graph parameter @ index 0, 1, 2 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d4[0];

    graph_parameters_queue_params_list[2].graph_parameter_index = 2;
    graph_parameters_queue_params_list[2].refs_list_size = num_buf;
    graph_parameters_queue_params_list[2].refs_list = (vx_reference*)&d5[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                3,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    if(arg_->measure_perf==1)
    {
        /* having multiple buffer in order to get higher pipelining and performance */
        tmp_num_buf = 2;
    }
    else
    {
        tmp_num_buf = 1;
        /* intentionally not having multiple buffers just to test graph stalling in absence of buffers */
    }

    /* Validating tivxGetNodeParameterNumBufByIndex API */
    VX_CALL(tivxGetNodeParameterNumBufByIndex(n0, 1, &get_num_buf));

    ASSERT(get_num_buf==0);

    VX_CALL(set_num_buf_by_node_index(n0, 1, tmp_num_buf));

    /* Validating tivxGetNodeParameterNumBufByIndex API */
    VX_CALL(tivxGetNodeParameterNumBufByIndex(n0, 1, &get_num_buf));

    ASSERT(get_num_buf==tmp_num_buf);

    /* n1 and n2 run on different targets hence set output of n1 to have multiple buffers so
     * that n1 and n2 can run in a pipeline
     */
    VX_CALL(set_num_buf_by_node_index(n1, 1, num_buf));

    tmp_num_buf = 1;
    VX_CALL(set_num_buf_by_node_index(n2, 2, tmp_num_buf));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_four_nodes");
    log_graph_rt_trace(graph);
    #if 1
    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(d0[buf_id], ref_src[buf_id]));
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(d4[buf_id], ref_src[buf_id]));
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d4[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&d5[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_image out_img, in1_img, in2_img;
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 2, (vx_reference*)&out_img, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&in2_img, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in1_img, 1, &num_refs));

        /* A graph execution completed, since we dequeued both input and output refs */
        if(arg_->measure_perf==0)
        {
            /* when measuring performance dont check output since it affects graph performance numbers
             */

            if(loop_cnt > 100)
            {
                ct_update_progress(loop_id, loop_cnt+num_buf);
            }

            ASSERT_NO_FAILURE({
                vxdst = ct_image_from_vx_image(out_img);
            });

            /* compare output */
            ASSERT_EQ_CTIMAGE(ref_src[buf_id], vxdst);
        }

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in1_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&in2_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&out_img, 1));
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0, n1, n2, n3 };

        printGraphPipelinePerformance(graph, nodes, 4, exe_time, loop_cnt+num_buf, arg_->width*arg_->height);
    }
    #endif

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n3));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d4[buf_id]));
        VX_CALL(vxReleaseImage(&d5[buf_id]));
    }
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseImage(&d2));
    VX_CALL(vxReleaseImage(&d3));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/* Note: Modifying the data ref queue value depending on node relationship */
/* Filed TIOVX-676 task to fix this */
/* Note: this test case has been disabled as per bug TIOVX-1023 */
#if (TIVX_GRAPH_MAX_NODES - 2) > TIVX_GRAPH_MAX_DATA_REF_QUEUE
#define PIPELINE_TEST_MAX_DATA_REF_QUEUE TIVX_GRAPH_MAX_DATA_REF_QUEUE
#else
#define PIPELINE_TEST_MAX_DATA_REF_QUEUE (TIVX_GRAPH_MAX_NODES - 2)
#endif
/*
 *  d0     n0     d1
 * IMG -- NOT -- IMG -- NOT --, etc. for TIVX_GRAPH_MAX_DATA_REF_QUEUE length
 *
 *
 * This test case test the below
 * - Tests the limit of TIVX_GRAPH_MAX_DATA_REF_QUEUE
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testMaxDataRef, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d_virt[PIPELINE_TEST_MAX_DATA_REF_QUEUE] = {NULL}, d1[MAX_NUM_BUF] = {NULL};
    vx_node  n[PIPELINE_TEST_MAX_DATA_REF_QUEUE+1];
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

    CT_Image ref_src[MAX_NUM_BUF] = {NULL};
    uint32_t width, height, seq_init, pipeline_depth, num_buf;
    uint32_t buf_id, loop_id, loop_cnt, i;
    uint64_t exe_time;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = arg_->width;
    height = arg_->height;
    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;
    loop_cnt = arg_->loop_count;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE({
            ref_src[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
            fillSequence(ref_src[buf_id], (uint32_t)(seq_init+buf_id*10));
        });
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d1[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    for (i = 0; i < PIPELINE_TEST_MAX_DATA_REF_QUEUE; i++)
    {
        ASSERT_VX_OBJECT(d_virt[i]    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(n[0]    = vxNotNode(graph, d0[0], d_virt[0]), VX_TYPE_NODE);

    for (i = 0; i < PIPELINE_TEST_MAX_DATA_REF_QUEUE-1; i++)
    {
        ASSERT_VX_OBJECT(n[i+1]    = vxNotNode(graph, d_virt[i], d_virt[i+1]), VX_TYPE_NODE);
    }
    ASSERT_VX_OBJECT(n[PIPELINE_TEST_MAX_DATA_REF_QUEUE]    = vxNotNode(graph, d_virt[PIPELINE_TEST_MAX_DATA_REF_QUEUE-1], d1[0]), VX_TYPE_NODE);

    for (i = 0; i < PIPELINE_TEST_MAX_DATA_REF_QUEUE+1; i++)
    {
        VX_CALL(vxSetNodeTarget(n[i], VX_TARGET_STRING, TIVX_TARGET_DSP1));
    }

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n[0], 0);
    /* input @ n3 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n[PIPELINE_TEST_MAX_DATA_REF_QUEUE], 1);

    /* set graph schedule config such that graph parameter @ index 0, 1, 2 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d1[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_four_nodes");
    log_graph_rt_trace(graph);
    #if 1
    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(d0[buf_id], ref_src[buf_id]));
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(d1[buf_id], ref_src[buf_id]));
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d1[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_image out_img, in_img;
        uint32_t num_refs;

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_img, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_img, 1, &num_refs));

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_img, 1));
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    #endif

    for (i = 0; i < PIPELINE_TEST_MAX_DATA_REF_QUEUE+1; i++)
    {
        VX_CALL(vxReleaseNode(&n[i]));
    }
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d1[buf_id]));
    }
    for (i = 0; i < PIPELINE_TEST_MAX_DATA_REF_QUEUE; i++)
    {
        VX_CALL(vxReleaseImage(&d_virt[i]));
    }
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 * This test case test the below
 * - Negative test for the limit of TIVX_GRAPH_MAX_DATA_REF_QUEUE
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, negativeTestGraphMaxDataRef, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d_virt[PIPELINE_TEST_MAX_DATA_REF_QUEUE+1] = {NULL}, d1[MAX_NUM_BUF] = {NULL};
    vx_node  n[PIPELINE_TEST_MAX_DATA_REF_QUEUE+2];
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

    CT_Image ref_src[MAX_NUM_BUF] = {NULL}, vxdst;
    uint32_t width, height, seq_init, pipeline_depth, num_buf, tmp_num_buf;
    uint32_t buf_id, loop_id, loop_cnt, i;
    uint64_t exe_time;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = arg_->width;
    height = arg_->height;
    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;
    loop_cnt = arg_->loop_count;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE({
            ref_src[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
            fillSequence(ref_src[buf_id], (uint32_t)(seq_init+buf_id*10));
        });
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d1[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    for (i = 0; i < PIPELINE_TEST_MAX_DATA_REF_QUEUE+1; i++)
    {
        ASSERT_VX_OBJECT(d_virt[i]    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(n[0]    = vxNotNode(graph, d0[0], d_virt[0]), VX_TYPE_NODE);

    for (i = 0; i < PIPELINE_TEST_MAX_DATA_REF_QUEUE; i++)
    {
        ASSERT_VX_OBJECT(n[i+1]    = vxNotNode(graph, d_virt[i], d_virt[i+1]), VX_TYPE_NODE);
    }
    ASSERT_VX_OBJECT(n[PIPELINE_TEST_MAX_DATA_REF_QUEUE+1]    = vxNotNode(graph, d_virt[PIPELINE_TEST_MAX_DATA_REF_QUEUE], d1[0]), VX_TYPE_NODE);

    for (i = 0; i < PIPELINE_TEST_MAX_DATA_REF_QUEUE+2; i++)
    {
        VX_CALL(vxSetNodeTarget(n[i], VX_TARGET_STRING, TIVX_TARGET_DSP1));
    }

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n[0], 0);
    /* input @ n3 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n[PIPELINE_TEST_MAX_DATA_REF_QUEUE+1], 1);

    /* set graph schedule config such that graph parameter @ index 0, 1, 2 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d1[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    for (i = 0; i < PIPELINE_TEST_MAX_DATA_REF_QUEUE+2; i++)
    {
        VX_CALL(vxReleaseNode(&n[i]));
    }
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d1[buf_id]));
    }
    for (i = 0; i < PIPELINE_TEST_MAX_DATA_REF_QUEUE+1; i++)
    {
        VX_CALL(vxReleaseImage(&d_virt[i]));
    }
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 * This test case test the below
 * - Negative test for the limit of TIVX_DATA_REF_Q_MAX_OBJECTS
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, negativeTestMaxDataRefQ, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d_virt[TIVX_DATA_REF_Q_MAX_OBJECTS] = {NULL}, d1[MAX_NUM_BUF] = {NULL};
    vx_node  n[TIVX_DATA_REF_Q_MAX_OBJECTS+1];
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

    CT_Image ref_src[MAX_NUM_BUF] = {NULL}, vxdst;
    uint32_t width, height, seq_init, pipeline_depth, num_buf, tmp_num_buf;
    uint32_t buf_id, loop_id, loop_cnt, i;
    uint64_t exe_time;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    if (TIVX_DATA_REF_Q_MAX_OBJECTS == 16)
    {

        seq_init = 1;
        width = arg_->width;
        height = arg_->height;
        pipeline_depth = arg_->pipe_depth;
        num_buf = arg_->num_buf;
        loop_cnt = arg_->loop_count;

        ASSERT(num_buf <= MAX_NUM_BUF);

        /* fill reference data */
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            ASSERT_NO_FAILURE({
                ref_src[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
                fillSequence(ref_src[buf_id], (uint32_t)(seq_init+buf_id*10));
            });
        }

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(d1[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        for (i = 0; i < TIVX_DATA_REF_Q_MAX_OBJECTS; i++)
        {
            ASSERT_VX_OBJECT(d_virt[i]    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }

        ASSERT_VX_OBJECT(n[0]    = vxNotNode(graph, d0[0], d_virt[0]), VX_TYPE_NODE);

        for (i = 0; i < TIVX_DATA_REF_Q_MAX_OBJECTS-1; i++)
        {
            ASSERT_VX_OBJECT(n[i+1]    = vxNotNode(graph, d_virt[i], d_virt[i+1]), VX_TYPE_NODE);
        }
        ASSERT_VX_OBJECT(n[TIVX_DATA_REF_Q_MAX_OBJECTS]    = vxNotNode(graph, d_virt[TIVX_DATA_REF_Q_MAX_OBJECTS-1], d1[0]), VX_TYPE_NODE);

        for (i = 0; i < TIVX_DATA_REF_Q_MAX_OBJECTS+1; i++)
        {
            VX_CALL(vxSetNodeTarget(n[i], VX_TARGET_STRING, TIVX_TARGET_DSP1));
        }

        /* input @ n0 index 0, becomes graph parameter 0 */
        add_graph_parameter_by_node_index(graph, n[0], 0);
        /* input @ n3 index 1, becomes graph parameter 1 */
        add_graph_parameter_by_node_index(graph, n[TIVX_DATA_REF_Q_MAX_OBJECTS], 1);

        /* set graph schedule config such that graph parameter @ index 0, 1, 2 are enqueuable */
        graph_parameters_queue_params_list[0].graph_parameter_index = 0;
        graph_parameters_queue_params_list[0].refs_list_size = num_buf;
        graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

        graph_parameters_queue_params_list[1].graph_parameter_index = 1;
        graph_parameters_queue_params_list[1].refs_list_size = num_buf;
        graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d1[0];

        /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        VX_CALL(vxSetGraphScheduleConfig(graph,
                    VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                    2,
                    graph_parameters_queue_params_list
                    ));

        /* explicitly set graph pipeline depth */
        VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

        EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

        for (i = 0; i < TIVX_DATA_REF_Q_MAX_OBJECTS+1; i++)
        {
            VX_CALL(vxReleaseNode(&n[i]));
        }
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            VX_CALL(vxReleaseImage(&d0[buf_id]));
            VX_CALL(vxReleaseImage(&d1[buf_id]));
        }
        for (i = 0; i < TIVX_DATA_REF_Q_MAX_OBJECTS; i++)
        {
            VX_CALL(vxReleaseImage(&d_virt[i]));
        }
        VX_CALL(vxReleaseGraph(&graph));
    }
    else
    {
        printf("In order to properly test the value of TIVX_DATA_REF_Q_MAX_OBJECTS, set it to 16 in tiovx/include/TI/tivx_config.h\n");
    }

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *  d0      n0      d2
 *  IMG --  AND -- IMG
 *          |
 *          d1 (uniform image) filled with 0xFF
 *
 * This test case test the below
 * - Uniform image as input
 * - No looping
 * - fixed pipeline depth of 2
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testUniformImage, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1, d2[MAX_NUM_BUF] = {NULL};
    vx_node n0;
    vx_pixel_value_t pixel_value;

    CT_Image ref_src[MAX_NUM_BUF] = {NULL}, vxdst;
    uint32_t width, height, seq_init, num_buf, pipeline_depth;
    uint32_t buf_id, loop_id, loop_cnt;
    uint64_t exe_time;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = arg_->width;
    height = arg_->height;
    loop_cnt = arg_->loop_count;
    pipeline_depth = 2;
    num_buf = 2;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE({
            ref_src[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
            fillSequence(ref_src[buf_id], (uint32_t)(seq_init+buf_id*10));
        });
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    /* create other refs, these are not multiple refs and same refs is fed as parameter to the graph */
    pixel_value.U8 = 0xFF;
    ASSERT_VX_OBJECT(d1    = vxCreateUniformImage(context, width, height, VX_DF_IMAGE_U8, &pixel_value), VX_TYPE_IMAGE);


    /* create node, input (index 0) and output (index 2) will be made as graph parameter
     * so that we can enqueue and dequeue refs to it and thus do graph pipelining.
     * d0[0], d2[0] used only for their meta data.
     * Actual input and output used for graph processing will be the
     * refs that are enqueued later.
     *
     * d1 also made a graph parameter, however it wont made as enqueable.
     */
    ASSERT_VX_OBJECT(n0    = vxAndNode(graph, d0[0], d1, d2[0]), VX_TYPE_NODE);

    /* input @ node index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ node index 2, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 2);
    /* input @ node index 1, becomes graph parameter 3 */
    add_graph_parameter_by_node_index(graph, n0, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    /* This graph parameter @ index 2 is not enqueue-able */
    VX_CALL(vxSetGraphParameterByIndex(graph, 2, (vx_reference)d1));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_uniform_img");
    log_graph_rt_trace(graph);
    #if 1
    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(d0[buf_id], ref_src[buf_id]));

    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d2[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_image out_img, in_img;
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_img, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_img, 1, &num_refs));

        /* A graph execution completed, since we dequeued both input and output refs */
        if(arg_->measure_perf==0)
        {
            /* when measuring performance dont check output since it affects graph performance numbers
             */

            if(loop_cnt > 100)
            {
                ct_update_progress(loop_id, loop_cnt+num_buf);
            }

            ASSERT_NO_FAILURE({
                vxdst = ct_image_from_vx_image(out_img);
            });

            /* compare output */
            ASSERT_EQ_CTIMAGE(ref_src[buf_id], vxdst);
        }

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_img, 1));
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0 };

        printGraphPipelinePerformance(graph, nodes, 1, exe_time, loop_cnt+num_buf, arg_->width*arg_->height);
    }
    #endif

    VX_CALL(vxReleaseNode(&n0));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d2[buf_id]));
    }
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

static inline uint32_t get_ref_src_index(uint32_t num_buf, uint32_t objarr_idx, uint32_t buf_id)
{
    return (objarr_idx*num_buf + buf_id);
}

static inline vx_object_array get_object_array_parent_of_image(vx_image out_img,
                vx_object_array d2[], vx_image img[], vx_uint32 num_buf)
{
    vx_object_array objarr = NULL;
    vx_uint32 buf_id;

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        if(out_img==img[buf_id])
        {
            objarr = d2[buf_id];
            break;
        }
    }
    return objarr;
}

static inline vx_pyramid get_pyramid_parent_of_image(vx_image out_img,
                vx_pyramid d2[], vx_image img[], vx_uint32 num_buf)
{
    vx_pyramid pyr = NULL;
    vx_uint32 buf_id;

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        if(out_img==img[buf_id])
        {
            pyr = d2[buf_id];
            break;
        }
    }
    return pyr;
}


/*
 *  d0     n0     d1     n1     d2
 * OBJ -- NOT -- OBJ -- NOT -- OBJ
 * ARR           ARR           ARR
 * replicate     replicate     replicate
 *
 * This test case test the below
 * - Object array with replicate attribute set
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testReplicateImage, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_object_array d0[MAX_NUM_BUF] = {NULL}, d1, d2[MAX_NUM_BUF] = {NULL};
    vx_image img0[MAX_NUM_BUF] = {NULL}, img1, img2[MAX_NUM_BUF] = {NULL};
    vx_image img_exemplar;
    vx_node n0, n1;
    vx_bool replicate[2] = { vx_true_e, vx_true_e };
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

    CT_Image ref_src[MAX_NUM_BUF*MAX_NUM_OBJ_ARR_ELEMENTS] = {NULL}, vxdst[MAX_NUM_OBJ_ARR_ELEMENTS] = {NULL};
    uint32_t width, height, seq_init, pipeline_depth, num_buf;
    uint32_t buf_id, loop_id, loop_cnt;
    uint32_t idx, objarr_idx, objarr_elements;
    uint64_t exe_time;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = arg_->width;
    height = arg_->height;
    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;
    loop_cnt = arg_->loop_count;
    objarr_elements = 2;

    ASSERT(num_buf <= MAX_NUM_BUF);
    ASSERT(objarr_elements <= MAX_NUM_OBJ_ARR_ELEMENTS);

    /* fill reference data */
    for(objarr_idx=0;objarr_idx<objarr_elements;objarr_idx++)
    {
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            idx = get_ref_src_index(num_buf, objarr_idx, buf_id);

            ASSERT_NO_FAILURE({
                ref_src[idx] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
                fillSequence(ref_src[idx], (uint32_t)(seq_init+(idx)));
            });
        }
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(img_exemplar    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateObjectArray(context, (vx_reference)img_exemplar, objarr_elements), VX_TYPE_OBJECT_ARRAY);
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateObjectArray(context, (vx_reference)img_exemplar, objarr_elements), VX_TYPE_OBJECT_ARRAY);
    }
    ASSERT_VX_OBJECT(d1    = vxCreateObjectArray(context, (vx_reference)img_exemplar, objarr_elements), VX_TYPE_OBJECT_ARRAY);

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        img0[buf_id] = (vx_image)vxGetObjectArrayItem(d0[buf_id], 0);
        img2[buf_id] = (vx_image)vxGetObjectArrayItem(d2[buf_id], 0);
    }
    img1 = (vx_image)vxGetObjectArrayItem(d1, 0);

    ASSERT_VX_OBJECT(n0    = vxNotNode(graph, img0[0], img1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, img1, img2[0]), VX_TYPE_NODE);

    VX_CALL(vxReplicateNode(graph, n0, replicate, 2));
    VX_CALL(vxReplicateNode(graph, n1, replicate, 2));

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    #endif

    /* input @ node0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ node1 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&img0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&img2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    /* set number of buffer at intermediate output */
    VX_CALL(set_num_buf_by_node_index(n0, 1, num_buf));

    /* set pipeline depth explicitly */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_replicate_node");
    log_graph_rt_trace(graph);
    #if 1
    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        for(objarr_idx=0;objarr_idx<objarr_elements;objarr_idx++)
        {
            vx_image image;

            idx = get_ref_src_index(num_buf, objarr_idx, buf_id);

            image = (vx_image)vxGetObjectArrayItem(d0[buf_id], objarr_idx);

            ASSERT_NO_FAILURE(ct_image_copyto_vx_image(image, ref_src[idx]));

            VX_CALL(vxReleaseImage(&image));
        }
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        vx_image image;

        image = (vx_image)vxGetObjectArrayItem(d0[buf_id], 0);
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&image, 1));
        vxReleaseImage(&image);

        image = (vx_image)vxGetObjectArrayItem(d2[buf_id], 0);
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&image, 1));
        vxReleaseImage(&image);
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_image out_img, in_img;
        vx_object_array out_objarr;
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_img, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_img, 1, &num_refs));

        /* A graph execution completed, since we dequeued both input and output refs */
        if(arg_->measure_perf==0)
        {
            /* when measuring performance dont check output since it affects graph performance numbers
             */

            if(loop_cnt > 100)
            {
                ct_update_progress(loop_id, loop_cnt+num_buf);
            }

            out_objarr = (vx_object_array)tivxGetReferenceParent((vx_reference)out_img);

            if (NULL != out_objarr)
            {
                for(objarr_idx=0;objarr_idx<objarr_elements;objarr_idx++)
                {
                    vx_image image;

                    image = (vx_image)vxGetObjectArrayItem(out_objarr, objarr_idx);

                    ASSERT_NO_FAILURE({
                        vxdst[objarr_idx] = ct_image_from_vx_image(image);
                    });

                    VX_CALL(vxReleaseImage(&image));
                }

                for(objarr_idx=0;objarr_idx<objarr_elements;objarr_idx++)
                {
                    idx = get_ref_src_index(num_buf, objarr_idx, buf_id);

                    /* compare output */
                    /* NOT of NOT should give back original image */
                    ASSERT_EQ_CTIMAGE(ref_src[idx], vxdst[objarr_idx]);
                }
            }
        }

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_img, 1));
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0, n1 };

        printGraphPipelinePerformance(graph, nodes, 2, exe_time, loop_cnt+num_buf, arg_->width*arg_->height);
    }
    #endif
    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseImage(&img_exemplar));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&img0[buf_id]));
        VX_CALL(vxReleaseImage(&img2[buf_id]));
    }
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&d0[buf_id]));
        VX_CALL(vxReleaseObjectArray(&d2[buf_id]));
    }
    VX_CALL(vxReleaseImage(&img1));
    VX_CALL(vxReleaseObjectArray(&d1));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *  d0     n0     d1     n1     d2
 * OBJ -- NOT -- OBJ -- NOT -- OBJ
 * ARR -- NOT -- ARR -- NOT -- ARR
 *        n0_1          n1_1
 *
 * This test case test the below
 * - Object array with no replicate nodes
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testDontReplicateImage, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_object_array d0[MAX_NUM_BUF] = {NULL}, d1, d2[MAX_NUM_BUF] = {NULL};
    vx_image img0[MAX_NUM_BUF] = {NULL}, img1, img2[MAX_NUM_BUF] = {NULL};
    vx_image img0_1[MAX_NUM_BUF] = {NULL}, img1_1, img2_1[MAX_NUM_BUF] = {NULL};
    vx_image img_exemplar;
    vx_node n0, n1;
    vx_node n0_1, n1_1;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

    CT_Image ref_src[MAX_NUM_BUF*MAX_NUM_OBJ_ARR_ELEMENTS] = {NULL}, vxdst[MAX_NUM_OBJ_ARR_ELEMENTS] = {NULL};
    uint32_t width, height, seq_init, pipeline_depth, num_buf;
    uint32_t buf_id, loop_id, loop_cnt;
    uint32_t idx, objarr_idx, objarr_elements;
    uint64_t exe_time;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = arg_->width;
    height = arg_->height;
    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;
    loop_cnt = arg_->loop_count;
    objarr_elements = 2;

    ASSERT(num_buf <= MAX_NUM_BUF);
    ASSERT(objarr_elements <= MAX_NUM_OBJ_ARR_ELEMENTS);

    /* fill reference data */
    for(objarr_idx=0;objarr_idx<objarr_elements;objarr_idx++)
    {
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            idx = get_ref_src_index(num_buf, objarr_idx, buf_id);

            ASSERT_NO_FAILURE({
                ref_src[idx] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
                fillSequence(ref_src[idx], (uint32_t)(seq_init+(idx)));
            });
        }
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(img_exemplar    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateObjectArray(context, (vx_reference)img_exemplar, objarr_elements), VX_TYPE_OBJECT_ARRAY);
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateObjectArray(context, (vx_reference)img_exemplar, objarr_elements), VX_TYPE_OBJECT_ARRAY);
    }
    ASSERT_VX_OBJECT(d1    = vxCreateObjectArray(context, (vx_reference)img_exemplar, objarr_elements), VX_TYPE_OBJECT_ARRAY);

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        img0[buf_id] = (vx_image)vxGetObjectArrayItem(d0[buf_id], 0);
        img2[buf_id] = (vx_image)vxGetObjectArrayItem(d2[buf_id], 0);
        img0_1[buf_id] = (vx_image)vxGetObjectArrayItem(d0[buf_id], 1);
        img2_1[buf_id] = (vx_image)vxGetObjectArrayItem(d2[buf_id], 1);
    }
    img1 = (vx_image)vxGetObjectArrayItem(d1, 0);
    img1_1 = (vx_image)vxGetObjectArrayItem(d1, 1);

    ASSERT_VX_OBJECT(n0    = vxNotNode(graph, img0[0], img1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, img1, img2[0]), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n0_1  = vxNotNode(graph, img0_1[0], img1_1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1_1  = vxNotNode(graph, img1_1, img2_1[0]), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n0_1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1_1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    VX_CALL(vxSetNodeTarget(n0_1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1_1, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    #endif

    /* input @ node0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ node1 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&img0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&img2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    /* set number of buffer at intermediate output */
    VX_CALL(set_num_buf_by_node_index(n0, 1, num_buf));
    VX_CALL(set_num_buf_by_node_index(n0_1, 1, num_buf));

    /* set pipeline depth explicitly */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_dont_replicate_node");
    log_graph_rt_trace(graph);
    #if 1
    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        for(objarr_idx=0;objarr_idx<objarr_elements;objarr_idx++)
        {
            vx_image image;

            idx = get_ref_src_index(num_buf, objarr_idx, buf_id);

            image = (vx_image)vxGetObjectArrayItem(d0[buf_id], objarr_idx);

            ASSERT_NO_FAILURE(ct_image_copyto_vx_image(image, ref_src[idx]));

            VX_CALL(vxReleaseImage(&image));
        }
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        vx_image image;

        image = (vx_image)vxGetObjectArrayItem(d0[buf_id], 0);
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&image, 1));
        vxReleaseImage(&image);

        image = (vx_image)vxGetObjectArrayItem(d2[buf_id], 0);
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&image, 1));
        vxReleaseImage(&image);
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_image out_img, in_img;
        vx_object_array out_objarr;
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_img, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_img, 1, &num_refs));

        /* A graph execution completed, since we dequeued both input and output refs */
        if(arg_->measure_perf==0)
        {
            /* when measuring performance dont check output since it affects graph performance numbers
             */

            if(loop_cnt > 100)
            {
                ct_update_progress(loop_id, loop_cnt+num_buf);
            }

            out_objarr = get_object_array_parent_of_image(out_img, d2, img2, num_buf);

            if (NULL != out_objarr)
            {
                for(objarr_idx=0;objarr_idx<objarr_elements;objarr_idx++)
                {
                    vx_image image;

                    image = (vx_image)vxGetObjectArrayItem(out_objarr, objarr_idx);

                    ASSERT_NO_FAILURE({
                        vxdst[objarr_idx] = ct_image_from_vx_image(image);
                    });

                    VX_CALL(vxReleaseImage(&image));
                }

                for(objarr_idx=0;objarr_idx<objarr_elements;objarr_idx++)
                {
                    idx = get_ref_src_index(num_buf, objarr_idx, buf_id);

                    /* compare output */
                    /* NOT of NOT should give back original image */
                    ASSERT_EQ_CTIMAGE(ref_src[idx], vxdst[objarr_idx]);
                }
            }
        }

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_img, 1));
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0, n1 };

        printGraphPipelinePerformance(graph, nodes, 2, exe_time, loop_cnt+num_buf, arg_->width*arg_->height);
    }
    #endif
    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n0_1));
    VX_CALL(vxReleaseNode(&n1_1));
    VX_CALL(vxReleaseImage(&img_exemplar));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&img0[buf_id]));
        VX_CALL(vxReleaseImage(&img2[buf_id]));
        VX_CALL(vxReleaseImage(&img0_1[buf_id]));
        VX_CALL(vxReleaseImage(&img2_1[buf_id]));
    }
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&d0[buf_id]));
        VX_CALL(vxReleaseObjectArray(&d2[buf_id]));
    }
    VX_CALL(vxReleaseImage(&img1));
    VX_CALL(vxReleaseImage(&img1_1));
    VX_CALL(vxReleaseObjectArray(&d1));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

typedef enum
{
    ADD = 0,
    SUB,
    MUL

} OWN_OPERATION_TYPE;

static CT_Image own_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
        image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static void tst_replicate_op_2(vx_context context, vx_image input1_img, vx_image input2_img, vx_pyramid input1, vx_pyramid input2, vx_pyramid output, OWN_OPERATION_TYPE op)
{
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_image src1 = 0;
    vx_image src2 = 0;
    vx_image dst = 0;
    vx_enum policy = VX_CONVERT_POLICY_SATURATE;
    vx_size levels = 0;
    vx_enum type = VX_TYPE_INVALID;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    {
        ASSERT_VX_OBJECT(src1 = vxGetPyramidLevel((vx_pyramid)input1, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src2 = vxGetPyramidLevel((vx_pyramid)input2, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst = vxGetPyramidLevel((vx_pyramid)output, 0), VX_TYPE_IMAGE);

        VX_CALL(vxQueryPyramid((vx_pyramid)input1, VX_PYRAMID_LEVELS, &levels, sizeof(levels)));

        ASSERT_VX_OBJECT(node1 = vxGaussianPyramidNode(graph, input1_img, input1), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node2 = vxGaussianPyramidNode(graph, input2_img, input2), VX_TYPE_NODE);
        vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_true_e };
        ASSERT_VX_OBJECT(node3 = vxAddNode(graph, src1, src2, policy, dst), VX_TYPE_NODE);
        VX_CALL(vxReplicateNode(graph, node3, replicate, 4));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&dst));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    return;
}

static void ref_replicate_op_2(vx_context context, vx_pyramid input1, vx_pyramid input2, vx_pyramid output, OWN_OPERATION_TYPE op)
{
    vx_uint32 i, k;
    vx_size levels = 0;
    vx_enum type = VX_TYPE_INVALID;

    {
        VX_CALL(vxQueryPyramid((vx_pyramid)input1, VX_PYRAMID_LEVELS, &levels, sizeof(vx_size)));

        // add, sub, mul
        for (k = 0; k < levels; k++)
        {
            vx_image src1 = 0;
            vx_image src2 = 0;
            vx_image dst = 0;
            vx_enum policy = VX_CONVERT_POLICY_SATURATE;
            vx_float32 scale_val = 1.0f;
            vx_enum rounding = VX_ROUND_POLICY_TO_ZERO;

            ASSERT_VX_OBJECT(src1 = vxGetPyramidLevel((vx_pyramid)input1, k), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(src2 = vxGetPyramidLevel((vx_pyramid)input2, k), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(dst = vxGetPyramidLevel((vx_pyramid)output, k), VX_TYPE_IMAGE);

            VX_CALL(vxuAdd(context, src1, src2, policy, dst));

            VX_CALL(vxReleaseImage(&src1));
            VX_CALL(vxReleaseImage(&src2));
            VX_CALL(vxReleaseImage(&dst));
        }
    }

    return;
}

static void check_replicas_2(vx_pyramid ref, vx_pyramid tst, vx_border_t border)
{
    vx_uint32 i;
    vx_size ref_levels = 0;
    vx_size tst_levels = 0;
    vx_uint32 ref_width1, ref_width2;
    vx_enum type = VX_TYPE_INVALID;

    {
        vx_float32 scale;
        VX_CALL(vxQueryPyramid((vx_pyramid)ref, VX_PYRAMID_LEVELS, &ref_levels, sizeof(vx_size)));
        VX_CALL(vxQueryPyramid((vx_pyramid)tst, VX_PYRAMID_LEVELS, &tst_levels, sizeof(vx_size)));
        VX_CALL(vxQueryPyramid((vx_pyramid)ref, VX_PYRAMID_SCALE, &scale, sizeof(scale)));
        EXPECT_EQ_INT(ref_levels, tst_levels);

        for (i = 0; i < ref_levels; i++)
        {
            vx_image src1 = 0;
            vx_image src2 = 0;
            CT_Image img1 = 0;
            CT_Image img2 = 0;

            ASSERT_VX_OBJECT(src1 = vxGetPyramidLevel((vx_pyramid)ref, i), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(src2 = vxGetPyramidLevel((vx_pyramid)tst, i), VX_TYPE_IMAGE);

            VX_CALL(vxQueryImage(src1, VX_IMAGE_WIDTH, &ref_width1, sizeof(ref_width1)));
            VX_CALL(vxQueryImage(src2, VX_IMAGE_WIDTH, &ref_width2, sizeof(ref_width2)));

            ASSERT_NO_FAILURE(img1 = ct_image_from_vx_image(src1));
            ASSERT_NO_FAILURE(img2 = ct_image_from_vx_image(src2));

            if (VX_BORDER_UNDEFINED == border.mode)
            {
                if (i > 0)
                {
                    if (VX_SCALE_PYRAMID_ORB == scale)
                    {
                        ct_adjust_roi(img1, 2, 2, 2, 2);
                        ct_adjust_roi(img2, 2, 2, 2, 2);
                    }
                    else if (VX_SCALE_PYRAMID_HALF == scale)
                    {
                        int roi_adj = ceil((double)(1+i)/2);
                        ct_adjust_roi(img1, roi_adj, roi_adj, roi_adj, roi_adj);
                        ct_adjust_roi(img2, roi_adj, roi_adj, roi_adj, roi_adj);
                    }
                }
            }

            /* Currently only passing on base level of pyramid */
            //if (0 == i)
            {
                EXPECT_EQ_CTIMAGE(img1, img2);
            }

            VX_CALL(vxReleaseImage(&src1));
            VX_CALL(vxReleaseImage(&src2));
        }
    }

    return;
}

/*
 *  d0     n0     d1     n1     d2
 * OBJ -- NOT -- OBJ -- NOT -- OBJ
 * ARR           ARR           ARR
 * replicate     replicate     replicate
 *
 * This test case test the below
 * - Object array with replicate attribute set
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testReplicateImage2, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    CT_Image src = 0;
    vx_pyramid src1_0 = 0, src1_0_ref = 0;
    vx_pyramid src2_0 = 0, src2_0_ref = 0;
    vx_pyramid ref;
    vx_pyramid tst[MAX_NUM_BUF] = {NULL};
    vx_image input1_0[MAX_NUM_BUF] = {NULL};
    vx_image input2_0[MAX_NUM_BUF] = {NULL};
    vx_pixel_value_t value1, value2;
    vx_border_t border;

    CT_Image ref_src[MAX_NUM_PYR_ELEMENTS], vxdst[MAX_NUM_PYR_ELEMENTS];
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_image pyr_src1 = 0;
    vx_image pyr_src2 = 0;
    vx_image pyr_dst[MAX_NUM_BUF] = {NULL};
    vx_enum policy = VX_CONVERT_POLICY_SATURATE;
    vx_size levels = 0;
    vx_enum type = VX_TYPE_INVALID;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

    uint32_t width, height, seq_init, pipeline_depth, num_buf;
    uint32_t buf_id, loop_id, loop_cnt;
    uint32_t idx, pyr_idx;
    uint64_t exe_time;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    levels = 4;
    seq_init = 1;
    width = arg_->width;
    height = arg_->height;
    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;
    loop_cnt = arg_->loop_count;

    ASSERT(num_buf <= MAX_NUM_BUF);

    ASSERT_NO_FAILURE(src = own_generate_random(NULL, width, height));

    {
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            value1.reserved[0] = 50*buf_id + 10;
            value2.reserved[0] = 50*buf_id + 20;
            ASSERT_VX_OBJECT(input1_0[buf_id] = vxCreateUniformImage(context, width, height, VX_DF_IMAGE_U8, &value1), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(input2_0[buf_id] = vxCreateUniformImage(context, width, height, VX_DF_IMAGE_U8, &value2), VX_TYPE_IMAGE);
        }

        ASSERT_VX_OBJECT(src1_0 = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(src2_0 = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(pyr_src1 = vxGetPyramidLevel(src1_0, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(pyr_src2 = vxGetPyramidLevel(src2_0, 0), VX_TYPE_IMAGE);

        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            ASSERT_VX_OBJECT(tst[buf_id] = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
            ASSERT_VX_OBJECT(pyr_dst[buf_id] = vxGetPyramidLevel(tst[buf_id], 0), VX_TYPE_IMAGE);
        }

        ASSERT_VX_OBJECT(node1 = vxGaussianPyramidNode(graph, input1_0[0], src1_0), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node2 = vxGaussianPyramidNode(graph, input2_0[0], src2_0), VX_TYPE_NODE);

        vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_true_e };
        ASSERT_VX_OBJECT(node3 = vxAddNode(graph, pyr_src1, pyr_src2, policy, pyr_dst[0]), VX_TYPE_NODE);
        VX_CALL(vxReplicateNode(graph, node3, replicate, 4));

        /* Pipelining stuff below */
        #if defined(SOC_AM62A)
        VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, TIVX_TARGET_DSP1));
        VX_CALL(vxSetNodeTarget(node3, VX_TARGET_STRING, TIVX_TARGET_DSP1));
        #else
        VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, TIVX_TARGET_DSP2));
        VX_CALL(vxSetNodeTarget(node3, VX_TARGET_STRING, TIVX_TARGET_DSP2));
        #endif

        /* input @ node1 index 0, becomes graph parameter 0 */
        add_graph_parameter_by_node_index(graph, node1, 0);
        /* input @ node2 index 0, becomes graph parameter 1 */
        add_graph_parameter_by_node_index(graph, node2, 0);
        /* output @ node3 index 3, becomes graph parameter 2 */
        add_graph_parameter_by_node_index(graph, node3, 3);

        /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
        graph_parameters_queue_params_list[0].graph_parameter_index = 0;
        graph_parameters_queue_params_list[0].refs_list_size = num_buf;
        graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&input1_0[0];

        graph_parameters_queue_params_list[1].graph_parameter_index = 1;
        graph_parameters_queue_params_list[1].refs_list_size = num_buf;
        graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&input2_0[0];

        graph_parameters_queue_params_list[2].graph_parameter_index = 1;
        graph_parameters_queue_params_list[2].refs_list_size = num_buf;
        graph_parameters_queue_params_list[2].refs_list = (vx_reference*)&pyr_dst[0];

        /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        VX_CALL(vxSetGraphScheduleConfig(graph,
                    VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                    3,
                    graph_parameters_queue_params_list
                    ));

        /* set number of buffer at intermediate output */
        VX_CALL(set_num_buf_by_node_index(node1, 1, num_buf));

        /* set number of buffer at intermediate output */
        VX_CALL(set_num_buf_by_node_index(node2, 1, num_buf));

        /* set pipeline depth explicitly */
        VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

        VX_CALL(vxVerifyGraph(graph));

        export_graph_to_file(graph, "test_graph_pipeline_replicate_node2");
        log_graph_rt_trace(graph);

        #if 1

        exe_time = tivxPlatformGetTimeInUsecs();

        /* enqueue input and output references,
         * input and output can be enqueued in any order
         * can be enqueued all together, here they are enqueue one by one just as a example
         */
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            vx_image image;
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&input1_0[buf_id], 1));

            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&input2_0[buf_id], 1));

            image = (vx_image)vxGetPyramidLevel(tst[buf_id], 0);
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&image, 1));
            VX_CALL(vxReleaseImage(&image));
        }

        buf_id = 0;

        /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
        for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
        {
            vx_image out_img, in_img1, in_img2;
            vx_pyramid out_pyr;
            uint32_t num_refs;

            /* Get output reference, waits until a reference is available */
            VX_CALL(vxGraphParameterDequeueDoneRef(graph, 2, (vx_reference*)&out_img, 1, &num_refs));

            /* Get consumed input reference, waits until a reference is available
             */
            VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&in_img2, 1, &num_refs));
            VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_img1, 1, &num_refs));

            /* A graph execution completed, since we dequeued both input and output refs */
            if(arg_->measure_perf==0)
            {

                if(loop_cnt > 100)
                {
                    ct_update_progress(loop_id, loop_cnt+num_buf);
                }

                out_pyr = get_pyramid_parent_of_image(out_img, tst, pyr_dst, num_buf);

                ASSERT_VX_OBJECT(src1_0_ref = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
                ASSERT_VX_OBJECT(src2_0_ref = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

                ASSERT_VX_OBJECT(ref = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

                tst_replicate_op_2(context, in_img1, in_img2, src1_0_ref, src2_0_ref, ref, ADD);

                /* Checking values */
                VX_CALL(vxQueryContext(context, VX_CONTEXT_IMMEDIATE_BORDER, &border, sizeof(border)));
                check_replicas_2(ref, out_pyr, border);

                VX_CALL(vxReleasePyramid(&src1_0_ref));
                VX_CALL(vxReleasePyramid(&src2_0_ref));
                VX_CALL(vxReleasePyramid(&ref));
            }

            buf_id = (buf_id+1)%num_buf;

            /* recycles dequeued input and output refs 'loop_cnt' times */
            if(loop_id<loop_cnt)
            {
                /* input and output can be enqueued in any order */
                VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_img1, 1));
                VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&in_img2, 1));
                VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&out_img, 1));
            }
        }

        /* ensure all graph processing is complete */
        VX_CALL(vxWaitGraph(graph));

        /* Pipelining stuff above */

        exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

        if(arg_->measure_perf==1)
        {
            vx_node nodes[] = { node1, node2, node3 };

            printGraphPipelinePerformance(graph, nodes, 3, exe_time, loop_cnt+num_buf, arg_->width*arg_->height);
        }
        #endif

        VX_CALL(vxReleaseImage(&pyr_src1));
        VX_CALL(vxReleaseImage(&pyr_src2));

        VX_CALL(vxReleaseNode(&node1));
        VX_CALL(vxReleaseNode(&node2));
        VX_CALL(vxReleaseNode(&node3));
        VX_CALL(vxReleaseGraph(&graph));

        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            VX_CALL(vxReleaseImage(&input1_0[buf_id]));
            VX_CALL(vxReleaseImage(&input2_0[buf_id]));
        }

        VX_CALL(vxReleasePyramid(&src1_0));
        VX_CALL(vxReleasePyramid(&src2_0));
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            VX_CALL(vxReleaseImage(&pyr_dst[buf_id]));
            VX_CALL(vxReleasePyramid(&tst[buf_id]));
        }
    }

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *
 * IMAGE -> MeanStdDev -> MEAN (SCALAR)
 *              |
 *              +-------> STD_DEV (SCALAR)
 *
 * This test case test the below
 * - Scalar with pipeline with scalar at output
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testScalarOutput, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL};
    vx_scalar mean_s[MAX_NUM_BUF] = {NULL}, stddev_s[MAX_NUM_BUF] = {NULL};
    vx_node n0;
    vx_float32 mean_out=0.0, stddev_out=0.0;
    vx_float32 mean_tolerance=1e-4, stddev_tolerance=1e-4;
    vx_float32 mean_diff, stddev_diff;
    CT_Image ref_src[MAX_NUM_BUF] = {NULL};
    vx_float32 mean_ref[MAX_NUM_BUF] = {0.0f};
    vx_float32 stddev_ref[MAX_NUM_BUF] = {0.0f};
    uint32_t width, height, pipeline_depth, num_buf;
    uint32_t buf_id, loop_id, loop_cnt;
    uint64_t rng;
    uint64_t exe_time;
    int a = 0, b = 256;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

    tivx_clr_debug_zone(VX_ZONE_INFO);

    width = arg_->width;
    height = arg_->height;
    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;
    loop_cnt = arg_->loop_count;

    rng = CT()->seed_;
    mean_tolerance *= b;
    stddev_tolerance *= b;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE({
            ref_src[buf_id] = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &rng, a, b);
        });
        reference_mean_stddev(ref_src[buf_id], &mean_ref[buf_id], &stddev_ref[buf_id]);
        //printf("Ref %d: mean=%5.3f, stddev=%5.3f\n", buf_id, (float)mean_ref[buf_id], (float)stddev_ref[buf_id]);
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]       = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(mean_s[buf_id]   = vxCreateScalar(context, VX_TYPE_FLOAT32, &mean_out), VX_TYPE_SCALAR);
        ASSERT_VX_OBJECT(stddev_s[buf_id] = vxCreateScalar(context, VX_TYPE_FLOAT32, &stddev_out), VX_TYPE_SCALAR);
    }
    ASSERT_VX_OBJECT(n0    = vxMeanStdDevNode(graph, d0[0], mean_s[0], stddev_s[0]), VX_TYPE_NODE);

    /* input @ node0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ node0 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 1);
    /* output @ node0 index 2, becomes graph parameter 2 */
    add_graph_parameter_by_node_index(graph, n0, 2);

    /* set graph schedule config such that graph parameter @ index 0 and 1 and 2 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&mean_s[0];

    graph_parameters_queue_params_list[2].graph_parameter_index = 1;
    graph_parameters_queue_params_list[2].refs_list_size = num_buf;
    graph_parameters_queue_params_list[2].refs_list = (vx_reference*)&stddev_s[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                3,
                graph_parameters_queue_params_list
                ));

    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_scalar_output");
    log_graph_rt_trace(graph);
    #if 1
    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(d0[buf_id], ref_src[buf_id]));
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&mean_s[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&stddev_s[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_image in_img;
        vx_scalar out_mean, out_stddev;
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 2, (vx_reference*)&out_stddev, 1, &num_refs));

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_mean, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_img, 1, &num_refs));

        /* A graph execution completed, since we dequeued both input and output refs */
        if(arg_->measure_perf==0)
        {
            /* when measuring performance dont check output since it affects graph performance numbers
             */

            if(loop_cnt > 100)
            {
                ct_update_progress(loop_id, loop_cnt+num_buf);
            }

        VX_CALL(vxCopyScalar(out_mean, &mean_out, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyScalar(out_stddev, &stddev_out, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
            /* compare output with reference */
            mean_diff = fabs(mean_ref[buf_id] - mean_out);
            stddev_diff = fabs(stddev_ref[buf_id] - stddev_out);

            //printf("Out %d: mean=%5.3f, stddev=%5.3f\n", loop_id, (float)mean_out, (float)stddev_out);

            #if 1
            if( mean_diff > mean_tolerance ||
                stddev_diff > stddev_tolerance )
            {
                CT_RecordFailureAtFormat("Test case %d. width=%d, height=%d,\n"
                                         "\tExpected: mean=%.5g, stddev=%.5g\n"
                                         "\tActual:   mean=%.5g (diff=%.5g %s %.5g), stddev=%.5f (diff=%.5g %s %.5g)\n",
                                         __FUNCTION__, __FILE__, __LINE__,
                                         loop_id, width, height,
                                         mean_ref[buf_id], stddev_ref[buf_id],
                                         mean_out, mean_diff, mean_diff > mean_tolerance ? ">" : "<=", mean_tolerance,
                                         stddev_out, stddev_diff, stddev_diff > stddev_tolerance ? ">" : "<=", stddev_tolerance);
            }
            #endif
        }

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_mean, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&out_stddev, 1));
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;
    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0 };

        printGraphPipelinePerformance(graph, nodes, 1, exe_time, loop_cnt+num_buf, arg_->width*arg_->height);
    }
    #endif
    VX_CALL(vxReleaseNode(&n0));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseScalar(&mean_s[buf_id]));
        VX_CALL(vxReleaseScalar(&stddev_s[buf_id]));
    }
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *  d0     n0     d1     n1     d2
 * IMG -- NOT -- IMG -- NOT -- IMG
 *
 * This test case test the below
 * - Single input, single output nodes
 * - Two nodes on two different targets
 * - Events are used to enqueue/dequeue buffers
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testEventHandling, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1, d2[MAX_NUM_BUF] = {NULL};
    vx_node n0, n1;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

    CT_Image ref_src[MAX_NUM_BUF] = {NULL}, vxdst;
    uint32_t width, height, seq_init, pipeline_depth, num_buf, loop_cnt;
    uint32_t buf_id, loop_id, in_q_cnt;
    uint64_t exe_time;

    vx_bool done;
    vx_event_t event;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = arg_->width;
    height = arg_->height;
    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;
    loop_cnt = arg_->loop_count;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE({
            ref_src[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
            fillSequence(ref_src[buf_id], (uint32_t)(seq_init+buf_id*10));
        });
    }

    VX_CALL(vxEnableEvents(context));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id] = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d2[buf_id] = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(d1    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(n0    = vxNotNode(graph, d0[0], d1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, d1, d2[0]), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    #endif

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ n1 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    /* set number of buffer at intermediate output */
    VX_CALL(set_num_buf_by_node_index(n0, 1, num_buf));

    VX_CALL(vxRegisterEvent((vx_reference)graph, VX_EVENT_GRAPH_PARAMETER_CONSUMED, 0, GRAPH_CONSUMED_EVENT));
    VX_CALL(vxRegisterEvent((vx_reference)n0, VX_EVENT_NODE_COMPLETED, 0, NODE0_COMPLETED_EVENT));
    VX_CALL(vxRegisterEvent((vx_reference)n1, VX_EVENT_NODE_COMPLETED, 0, NODE1_COMPLETED_EVENT));
    VX_CALL(vxRegisterEvent((vx_reference)graph, VX_EVENT_GRAPH_COMPLETED, 0, GRAPH_COMPLETED_EVENT));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_event_handling");
    log_graph_rt_trace(graph);
    #if 1
    /* clear pending events */
    while( vxWaitEvent(context, &event, vx_true_e) == VX_SUCCESS);

    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(d0[buf_id], ref_src[buf_id]));
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d2[buf_id], 1));
    }

    buf_id = 0;

    done = vx_false_e;
    loop_id = 0;
    in_q_cnt = 0;
    while(!done)
    {
        VX_CALL(vxWaitEvent(context, &event, vx_false_e));

        if(event.app_value==GRAPH_CONSUMED_EVENT)
        {
            vx_image in_img;
            uint32_t num_refs;

            /* input should be free at this point */
            /* recycle input buffer, input data is not changed in this test */
            VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_img, 1, &num_refs));
            if(in_q_cnt<loop_cnt)
            {
                VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_img, 1));
            }
            in_q_cnt++;
        }
        else
        if(event.app_value==NODE1_COMPLETED_EVENT)
        {
            vx_image out_img;
            uint32_t num_refs;

            /* Get output reference, waits until a reference is available */
            VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_img, 1, &num_refs));

            /* A graph execution completed, since we dequeued both input and output refs */
            if(arg_->measure_perf==0)
            {
                /* when measuring performance dont check output since it affects graph performance numbers
                 */

                if(loop_cnt > 100)
                {
                    ct_update_progress(loop_id, loop_cnt+num_buf);
                }

                ASSERT_NO_FAILURE({
                    vxdst = ct_image_from_vx_image(out_img);
                });

                /* compare output */
                ASSERT_EQ_CTIMAGE(ref_src[buf_id], vxdst);
            }

            buf_id = (buf_id+1)%num_buf;

            /* recycles dequeued input and output refs 'loop_cnt' times */
            if(loop_id<loop_cnt)
            {
                /* input and output can be enqueued in any order */
                VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_img, 1));
            }
            else
            {
                /* send user event to exit, id is 0xDEADBEAF, and parameter is some known pattern 0x12345678 */
                VX_CALL(vxSendUserEvent(context, 0xDEADBEAFu, (void*)0x12345678u));
            }

            loop_id++;
        }
        else
        if((event.type==VX_EVENT_USER)
            && (event.app_value == 0xDEADBEAFu)
            && (event.event_info.user_event.user_event_parameter == (void*)0x12345678u)
            )
        {
            done = vx_true_e;
        }
    }

    VX_CALL(vxWaitGraph(graph));

    /* handle last few buffers */
    done = vx_false_e;
    while(!done)
    {
        vx_image in_img, out_img;
        vx_uint32 in_num_refs, out_num_refs;

        /* recycle and access output data */
        VX_CALL(vxGraphParameterCheckDoneRef(graph, 1, &out_num_refs));
        if(out_num_refs>0)
        {
            VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_img, 1, &out_num_refs));
            if(arg_->measure_perf==0)
            {
                ASSERT_NO_FAILURE({
                    vxdst = ct_image_from_vx_image(out_img);
                });
                /* compare output */
                /* NOT of NOT should give back original image */
                ASSERT_EQ_CTIMAGE(ref_src[buf_id], vxdst);
            }
        }
        /* recycle and access input data */
        VX_CALL(vxGraphParameterCheckDoneRef(graph, 0, &in_num_refs));
        if(in_num_refs>0)
        {
            VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_img, 1, &in_num_refs));
        }
        buf_id = (buf_id+1)%num_buf;

        if(in_num_refs == 0 && out_num_refs == 0)
        {
            done = vx_true_e;
        }
    }

    /* clear pending events */
    while( vxWaitEvent(context, &event, vx_true_e) == VX_SUCCESS);

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;
    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0, n1 };

        printGraphPipelinePerformance(graph, nodes, 2, exe_time, loop_cnt+num_buf, arg_->width*arg_->height);
    }
    #endif
    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d2[buf_id]));
    }
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 * TIOVX-814
 * Tries to create an event after verifying the graph.
 * This will throw an error.
 *
 */
TEST(tivxGraphPipeline, testEventHandlingGraphVerify)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1, d2[MAX_NUM_BUF] = {NULL};
    vx_node n0, n1;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

    CT_Image ref_src[MAX_NUM_BUF] = {NULL}, vxdst;
    uint32_t width, height, seq_init, pipeline_depth, num_buf, loop_cnt;
    uint32_t buf_id, loop_id, in_q_cnt;
    uint64_t exe_time;

    vx_bool done;
    vx_event_t event;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = 64;
    height = 48;
    pipeline_depth = 1;
    num_buf = 1;
    loop_cnt = 1;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE({
            ref_src[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
            fillSequence(ref_src[buf_id], (uint32_t)(seq_init+buf_id*10));
        });
    }

    VX_CALL(vxEnableEvents(context));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id] = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d2[buf_id] = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(d1    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(n0    = vxNotNode(graph, d0[0], d1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, d1, d2[0]), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    #endif

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ n1 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    /* set number of buffer at intermediate output */
    VX_CALL(set_num_buf_by_node_index(n0, 1, num_buf));

    VX_CALL(vxVerifyGraph(graph));

    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)graph, VX_EVENT_GRAPH_PARAMETER_CONSUMED, 0, GRAPH_CONSUMED_EVENT));
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)n0, VX_EVENT_NODE_COMPLETED, 0, NODE0_COMPLETED_EVENT));
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)n1, VX_EVENT_NODE_COMPLETED, 0, NODE1_COMPLETED_EVENT));
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)graph, VX_EVENT_GRAPH_COMPLETED, 0, GRAPH_COMPLETED_EVENT));

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d2[buf_id]));
    }
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *
 * This test case test the below
 * - Disable of events and reenable of events
 *
 */
TEST(tivxGraphPipeline, testEventHandlingDisableEvents)
{
    vx_context context = context_->vx_context_;
    vx_event_t event;

    /* NOTE: events are enabled by default */

    /* send one user event, this should be received */
    VX_CALL(vxSendUserEvent(context, 0x1u, NULL));

    /* disable events and send another event */
    VX_CALL(vxDisableEvents(context));
    /* this event should get dropped and send event API should return failure */
    ASSERT(vxSendUserEvent(context, 0x2u, NULL)!=VX_SUCCESS);

    /* re-enable events and send another event */
    VX_CALL(vxEnableEvents(context));
    /* this event should get received */
    VX_CALL(vxSendUserEvent(context, 0x3u, NULL));

    /* wait for one event, this should be the first one */
    VX_CALL(vxWaitEvent(context, &event, vx_true_e));
    ASSERT(event.type==VX_EVENT_USER && event.app_value==0x1u);

    /* wait for one more event, this should be the third one */
    VX_CALL(vxWaitEvent(context, &event, vx_true_e));
    ASSERT(event.type==VX_EVENT_USER && event.app_value==0x3u);

    /* wait for one more event, there should be no more events */
    ASSERT(vxWaitEvent(context, &event, vx_true_e) != VX_SUCCESS);
}

/*
 *  d0           n0           d1            n1         d2
 * SCALAR -- USER_KERNEL -- SCALAR -- USER_KERNEL -- SCALAR
 *                            |            |
 *                            + -----------+
 *
 * This test case test the below
 * - User kernel nodes
 * - Nodes with optional parameters
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testUserKernel, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_scalar d0[MAX_NUM_BUF] = {NULL}, d1, d2[MAX_NUM_BUF] = {NULL};
    vx_scalar in_scalar, out_scalar;
    vx_node n0, n1;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];
    vx_uint32 in_value[MAX_NUM_BUF] = {0}, ref_out_value[MAX_NUM_BUF] = {0};
    vx_uint32 tmp_value = 0;

    uint32_t pipeline_depth, num_buf;
    uint32_t buf_id, loop_id, loop_cnt;
    uint64_t exe_time;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    test_user_kernel_register(context);

    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;
    loop_cnt = arg_->loop_count;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        in_value[buf_id] = 10*(buf_id+1);
        ref_out_value[buf_id] = 2 * in_value[buf_id];
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
    }
    ASSERT_VX_OBJECT(d1    = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n0    = test_user_kernel_node(graph, d0[0], NULL, d1, NULL), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1    = test_user_kernel_node(graph, d1, d1, d2[0], NULL), VX_TYPE_NODE);

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ n1 index 2, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 2);

    /* set graph schedule config such that graph parameter @ index 0, 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    VX_CALL(set_num_buf_by_node_index(n0, 2, num_buf));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_user_kernel");
    log_graph_rt_trace(graph);

    #if 1
    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(
            vxCopyScalar(d0[buf_id],
            &in_value[buf_id],
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d2[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_scalar, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_scalar, 1, &num_refs));

        /* A graph execution completed, since we dequeued both input and output refs */
        if(arg_->measure_perf==0)
        {
            /* when measuring performance dont check output since it affects graph performance numbers
             */

            if(loop_cnt > 100)
            {
                ct_update_progress(loop_id, loop_cnt+num_buf);
            }
        }

        VX_CALL(vxCopyScalar(out_scalar, &tmp_value, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        /* compare output */
        ASSERT_EQ_INT(tmp_value, ref_out_value[buf_id]);

        /* clear value in output */
        tmp_value = 0;
        VX_CALL(vxCopyScalar(out_scalar, &tmp_value, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_scalar, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_scalar, 1));
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0, n1 };

        printGraphPipelinePerformance(graph, nodes, 2, exe_time, loop_cnt+num_buf, 1);
    }
    #endif

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseScalar(&d0[buf_id]));
        VX_CALL(vxReleaseScalar(&d2[buf_id]));
    }
    VX_CALL(vxReleaseScalar(&d1));
    VX_CALL(vxReleaseGraph(&graph));

    test_user_kernel_unregister(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *  d0     n0     d1     n1     d2
 * IMG -- NOT -- IMG -- NOT -- IMG
 *
 * This test case test the below
 * - Single input, single output nodes
 * - Two nodes on two different targets
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testManualSchedule, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1, d2[MAX_NUM_BUF] = {NULL};
    vx_node n0, n1;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

    CT_Image ref_src[MAX_NUM_BUF], vxdst;
    uint32_t width, height, seq_init, pipeline_depth, num_buf;
    uint32_t buf_id, loop_id, loop_cnt;
    uint64_t exe_time;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = arg_->width;
    height = arg_->height;
    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;
    loop_cnt = arg_->loop_count;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE({
            ref_src[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
            fillSequence(ref_src[buf_id], (uint32_t)(seq_init+buf_id*10));
        });
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(d1    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(n0    = vxNotNode(graph, d0[0], d1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, d1, d2[0]), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    #endif

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ n1 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 1);

    /* set graph schedule config such that graph parameter @ index 0, 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    /* Schedule mode manual is used, here we need to call vxScheduleGraph
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL,
                2,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_manual_schedule_mode");
    log_graph_rt_trace(graph);

    #if 1
    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(d0[buf_id], ref_src[buf_id]));
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    loop_cnt = (loop_cnt + num_buf) / num_buf;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt); loop_id++)
    {
        vx_image out_img[MAX_NUM_BUF], in_img[MAX_NUM_BUF];
        uint32_t num_refs_in, num_refs_out;

        /* enqueue input and output references,
         * input and output can be enqueued in any order
         * can be enqueued all together, here they are enqueue one by one just as a example
         */
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d2[buf_id], 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
        }

        VX_CALL(vxScheduleGraph(graph));
        VX_CALL(vxWaitGraph(graph));

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)out_img, num_buf, &num_refs_in));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)in_img, num_buf, &num_refs_out));

        ASSERT_EQ_INT(num_refs_in, num_buf);
        ASSERT_EQ_INT(num_refs_out, num_buf);

        /* A graph execution completed, since we dequeued both input and output refs */
        if(arg_->measure_perf==0)
        {
            /* when measuring performance dont check output since it affects graph performance numbers
             */

            if(loop_cnt > 100)
            {
                ct_update_progress(loop_id, loop_cnt);
            }

            for(buf_id=0; buf_id<num_buf; buf_id++)
            {
                ASSERT_NO_FAILURE({
                    vxdst = ct_image_from_vx_image(out_img[buf_id]);
                });

                /* compare output */
                ASSERT_EQ_CTIMAGE(ref_src[buf_id], vxdst);
            }
        }
    }

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0, n1 };

        printGraphPipelinePerformance(graph, nodes, 2, exe_time, loop_cnt*num_buf, arg_->width*arg_->height);
    }
    #endif

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d2[buf_id]));
    }
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *  d0                        n0           d2
 * SCALAR -- delay (0) --  USER_KERNEL -- SCALAR
 *               |            |
 *           delay (-1) ------+
 *
 *
 * This test case test the below
 * - Delay objects
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testDelay1, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_scalar d0[MAX_NUM_BUF] = {NULL}, d2[MAX_NUM_BUF] = {NULL}, exemplar;
    vx_delay delay;
    vx_scalar in_scalar, out_scalar;
    vx_node n0;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];
    vx_uint32 in_value[MAX_NUM_BUF], ref_out_value[MAX_NUM_BUF];
    vx_uint32 tmp_value = 0;

    uint32_t pipeline_depth, num_buf;
    uint32_t buf_id, loop_id, loop_cnt;
    uint64_t exe_time;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    test_user_kernel_register(context);

    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;
    loop_cnt = arg_->loop_count;

    /* since delay is of 2 slots, num_buf MUST be >= 2 at input atleast */
    if(num_buf < 2)
    {
        num_buf = 2;
    }

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        in_value[buf_id] = 10*(buf_id+1);
    }
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ref_out_value[buf_id] = in_value[buf_id]
                          + in_value[ (num_buf + buf_id-1)%num_buf ];
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
    }

    /* allocate output, delay slot 0 is d0[0], delay slot -1 is d0[num_buf-1]
     * allocate other objects in between
     */
    ASSERT_VX_OBJECT(exemplar  = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(delay     = vxCreateDelay(context, (vx_reference)exemplar, 2), VX_TYPE_DELAY);
    d0[0] = (vx_scalar)vxGetReferenceFromDelay(delay, 0);
    for(buf_id=1; buf_id<num_buf-1; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
    }
    d0[num_buf-1] = (vx_scalar)vxGetReferenceFromDelay(delay, -1);
    vxReleaseScalar(&exemplar);

    ASSERT_VX_OBJECT(n0    = test_user_kernel_node(graph, d0[0], d0[num_buf-1], d2[0], NULL), VX_TYPE_NODE);

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ n0 index 2, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 2);

    /* set graph schedule config such that graph parameter @ index 0, 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    /* always auto age delay in pipelined graph */
    VX_CALL(vxRegisterAutoAging(graph, delay));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_delay1");
    log_graph_rt_trace(graph);

    #if 1
    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(
            vxCopyScalar(d0[buf_id],
            &in_value[buf_id],
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d2[buf_id], 1));
    }
    /* last buf is already set at the delay slot -1 so dont enqueue that ref */
    for(buf_id=0; buf_id<num_buf-1; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf-1); loop_id++)
    {
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_scalar, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_scalar, 1, &num_refs));

        /* A graph execution completed, since we dequeued both input and output refs */
        if(arg_->measure_perf==0)
        {
            /* when measuring performance dont check output since it affects graph performance numbers
             */

            if(loop_cnt > 100)
            {
                ct_update_progress(loop_id, loop_cnt+num_buf);
            }
        }

        VX_CALL(vxCopyScalar(out_scalar, &tmp_value, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        /* compare output */
        //printf(" %d: out = %d ref = %d\n", loop_id, tmp_value, ref_out_value[buf_id]);
        ASSERT_EQ_INT(tmp_value, ref_out_value[buf_id]);

        /* clear value in output */
        tmp_value = 0;
        VX_CALL(vxCopyScalar(out_scalar, &tmp_value, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_scalar, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_scalar, 1));
        }
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0 };

        printGraphPipelinePerformance(graph, nodes, 1, exe_time, loop_cnt+num_buf, 1);
    }
    #endif

    VX_CALL(vxReleaseNode(&n0));
    for(buf_id=1; buf_id<num_buf-1; buf_id++)
    {
        VX_CALL(vxReleaseScalar(&d0[buf_id]));
    }
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseScalar(&d2[buf_id]));
    }
    VX_CALL(vxReleaseDelay(&delay));
    VX_CALL(vxReleaseGraph(&graph));

    test_user_kernel_unregister(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *  d0                        n0           d2
 * SCALAR -- delay (0) --  USER_KERNEL -- SCALAR
 *               |            |
 *           delay (-1) ------+--- USER_KERNEL -- null
 *               |                     n1
 *               |                     |
 *           delay (-2) ---------------+
 *
 * This test case test the below
 * - Delay objects with 3 delay slots
 * - Delay slot connected to two inputs
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testDelay2, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_scalar d0[MAX_NUM_BUF] = {NULL}, d2[MAX_NUM_BUF] = {NULL}, exemplar;
    vx_delay delay;
    vx_scalar in_scalar, out_scalar;
    vx_node n0, n1;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];
    vx_uint32 in_value[MAX_NUM_BUF], ref_out_value[MAX_NUM_BUF];
    vx_uint32 tmp_value = 0;

    uint32_t pipeline_depth, num_buf;
    uint32_t buf_id, loop_id, loop_cnt, k;
    uint64_t exe_time;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    test_user_kernel_register(context);

    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;
    loop_cnt = arg_->loop_count;

    /* since delay is of 3 slots, num_buf MUST be >= 3 at input atleast */
    if(num_buf < 3)
    {
        num_buf = 3;
    }

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        in_value[buf_id] = 10*(buf_id+1);
    }
    k=0;
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ref_out_value[buf_id] = in_value[k]
                          + in_value[ (num_buf + k - 2)%num_buf ];
        k = (k+2)%num_buf;
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
    }

    /* allocate output, delay slot 0 is d0[0], delay slot -1 is d0[num_buf-1]
     * allocate other objects in between
     */
    ASSERT_VX_OBJECT(exemplar  = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(delay     = vxCreateDelay(context, (vx_reference)exemplar, 3), VX_TYPE_DELAY);
    d0[0] = (vx_scalar)vxGetReferenceFromDelay(delay, 0);
    for(buf_id=1; buf_id<num_buf-2; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
    }
    d0[num_buf-2] = (vx_scalar)vxGetReferenceFromDelay(delay, -1);
    d0[num_buf-1] = (vx_scalar)vxGetReferenceFromDelay(delay, -2);
    VX_CALL(vxReleaseScalar(&exemplar));

    ASSERT_VX_OBJECT(n0    = test_user_kernel_node(graph, d0[0], d0[num_buf-2], d2[0], NULL), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1    = test_user_kernel_node(graph, d0[num_buf-1], d0[num_buf-2], NULL, NULL), VX_TYPE_NODE);

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ n0 index 2, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 2);

    /* set graph schedule config such that graph parameter @ index 0, 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    /* always auto age delay in pipelined graph */
    VX_CALL(vxRegisterAutoAging(graph, delay));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_delay2");
    log_graph_rt_trace(graph);

    #if 1
    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(
            vxCopyScalar(d0[buf_id],
            &in_value[buf_id],
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d2[buf_id], 1));
    }
    /* last buf is already set at the delay slot -1 so dont enqueue that ref */
    for(buf_id=0; buf_id<num_buf-2; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf-2); loop_id++)
    {
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_scalar, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_scalar, 1, &num_refs));

        /* A graph execution completed, since we dequeued both input and output refs */
        if(arg_->measure_perf==0)
        {
            /* when measuring performance dont check output since it affects graph performance numbers
             */

            if(loop_cnt > 100)
            {
                ct_update_progress(loop_id, loop_cnt+num_buf);
            }
        }

        VX_CALL(vxCopyScalar(out_scalar, &tmp_value, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        /* compare output */
        //printf(" %d: out = %d ref = %d\n", loop_id, tmp_value, ref_out_value[buf_id]);
        ASSERT_EQ_INT(tmp_value, ref_out_value[buf_id]);

        /* clear value in output */
        tmp_value = 0;
        VX_CALL(vxCopyScalar(out_scalar, &tmp_value, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_scalar, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_scalar, 1));
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0, n1 };

        printGraphPipelinePerformance(graph, nodes, 2, exe_time, loop_cnt+num_buf, 1);
    }
    #endif

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    for(buf_id=1; buf_id<num_buf-2; buf_id++)
    {
        VX_CALL(vxReleaseScalar(&d0[buf_id]));
    }
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseScalar(&d2[buf_id]));
    }
    VX_CALL(vxReleaseDelay(&delay));
    VX_CALL(vxReleaseGraph(&graph));

    test_user_kernel_unregister(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *  d0           n0                           n1            d2
 * SCALAR -- USER_KERNEL -- delay (-1) --  USER_KERNEL -- SCALAR
 *                             |            |
 *                         delay (-2) ------+--- USER_KERNEL -- null
 *                             |                     n2
 *                             |                     |
 *                         delay (0)  ---------------+
 *
 * This test case test the below
 * - Delay objects with 3 delay slots
 * - Delay slot connected to two inputs
 * - Delay intermediate to a graph, no graph parameter at any delay slot
 * - node output to delay slot -1 (instead of typical slot 0)
 * - multiple buffers at output of n0 i.e delay slot -1
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testDelay3, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_scalar d0[MAX_NUM_BUF] = {NULL}, d2[MAX_NUM_BUF] = {NULL}, exemplar;
    vx_delay delay;
    vx_scalar in_scalar, out_scalar;
    vx_node n0, n1, n2;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];
    vx_uint32 in_value[MAX_NUM_BUF], ref_out_value[MAX_NUM_BUF];
    vx_uint32 tmp_value = 0;

    uint32_t pipeline_depth, num_buf;
    uint32_t buf_id, loop_id, loop_cnt;
    uint64_t exe_time;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    test_user_kernel_register(context);

    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;
    loop_cnt = arg_->loop_count;

    /* since delay is of 3 slots, num_buf MUST be >= 3 at input atleast */
    if(num_buf < 3)
    {
        num_buf = 3;
    }

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        in_value[buf_id] = 10*(buf_id+1);
    }

    {
        uint32_t tmp_value[3];

        tmp_value[0] = 0;
        tmp_value[1] = in_value[num_buf-1];
        tmp_value[2] = in_value[num_buf-2];

        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            ref_out_value[buf_id] = in_value[buf_id]
                              + tmp_value[ 1 ];

            tmp_value[ 0 ] = tmp_value[ 2 ];
            tmp_value[ 2 ] = tmp_value[ 1 ];
            tmp_value[ 1 ] = in_value[buf_id];
        }
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
    }

    /* allocate delay
     */
    ASSERT_VX_OBJECT(exemplar  = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(delay     = vxCreateDelay(context, (vx_reference)exemplar, 3), VX_TYPE_DELAY);
    vxReleaseScalar(&exemplar);

    ASSERT_VX_OBJECT(
        n0 = test_user_kernel_node( graph,
                d0[0], NULL,
                (vx_scalar)vxGetReferenceFromDelay(delay, -1), NULL),
                VX_TYPE_NODE);
    ASSERT_VX_OBJECT(
        n1 = test_user_kernel_node(graph,
                (vx_scalar)vxGetReferenceFromDelay(delay, -1), (vx_scalar)vxGetReferenceFromDelay(delay, -2),
                d2[0], NULL),
                VX_TYPE_NODE);
    ASSERT_VX_OBJECT(
        n2 = test_user_kernel_node(graph,
                (vx_scalar)vxGetReferenceFromDelay(delay, -2), (vx_scalar)vxGetReferenceFromDelay(delay, 0),
                NULL, NULL),
                VX_TYPE_NODE);

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ n1 index 2, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 2);

    /* set graph schedule config such that graph parameter @ index 0, 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    VX_CALL(set_num_buf_by_node_index(n0, 2, 2));

    /* always auto age delay in pipelined graph */
    VX_CALL(vxRegisterAutoAging(graph, delay));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_delay3");
    log_graph_rt_trace(graph);

    #if 1
    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(
            vxCopyScalar(d0[buf_id],
            &in_value[buf_id],
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
    }
    {
        vx_scalar tmp_scalar;

        tmp_scalar = (vx_scalar)vxGetReferenceFromDelay(delay, 0);
        ASSERT_NO_FAILURE(
            vxCopyScalar(tmp_scalar,
            &in_value[num_buf-2],
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        tmp_scalar = (vx_scalar)vxGetReferenceFromDelay(delay, -2);
        ASSERT_NO_FAILURE(
            vxCopyScalar(tmp_scalar,
            &in_value[num_buf-1],
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d2[buf_id], 1));
    }
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_scalar, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_scalar, 1, &num_refs));

        /* A graph execution completed, since we dequeued both input and output refs */
        if(arg_->measure_perf==0)
        {
            /* when measuring performance dont check output since it affects graph performance numbers
             */

            if(loop_cnt > 100)
            {
                ct_update_progress(loop_id, loop_cnt+num_buf);
            }
        }

        VX_CALL(vxCopyScalar(out_scalar, &tmp_value, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        /* compare output */
        //printf(" %d: out = %d ref = %d\n", loop_id, tmp_value, ref_out_value[buf_id]);
        ASSERT_EQ_INT(tmp_value, ref_out_value[buf_id]);

        /* clear value in output */
        tmp_value = 0;
        VX_CALL(vxCopyScalar(out_scalar, &tmp_value, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_scalar, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_scalar, 1));
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0, n1, n2 };

        printGraphPipelinePerformance(graph, nodes, 3, exe_time, loop_cnt+num_buf, 1);
    }
    #endif

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseScalar(&d0[buf_id]));
        VX_CALL(vxReleaseScalar(&d2[buf_id]));
    }
    VX_CALL(vxReleaseDelay(&delay));
    VX_CALL(vxReleaseGraph(&graph));

    test_user_kernel_unregister(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *  d0           n0                           n1            d2
 * SCALAR -- USER_KERNEL -- delay (0) --  USER_KERNEL -- SCALAR
 *                             |            |
 *                          delay (-1)      |
 *                             |            |
 *                          delay (-2)      |
 *                             |            |
 *                          delay (-3) -----+
 *
 * This test case test the below
 * - Delay objects with 4 delay slots
 * - Delay intermediate to a graph, no graph parameter at any delay slot
 * - Delay with slot's not connected to any input - tests auto age at these slots
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testDelay4, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_scalar d0[MAX_NUM_BUF] = {NULL}, d2[MAX_NUM_BUF] = {NULL}, exemplar;
    vx_delay delay;
    vx_scalar in_scalar, out_scalar;
    vx_node n0, n1;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];
    vx_uint32 in_value[MAX_NUM_BUF], ref_out_value;
    vx_uint32 ref_delay_value[MAX_NUM_BUF];
    vx_uint32 tmp_value = 0;

    uint32_t pipeline_depth, num_buf;
    uint32_t buf_id, loop_id, loop_cnt;
    uint64_t exe_time;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    test_user_kernel_register(context);

    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;
    loop_cnt = arg_->loop_count;

    /* since delay is of 4 slots, num_buf MUST be >= 4 at input atleast */
    if(num_buf < 4)
    {
        num_buf = 4;
    }

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        in_value[buf_id] = 10*(buf_id+1);
    }

    ref_delay_value[0] = 0;
    ref_delay_value[1] = in_value[num_buf-3];
    ref_delay_value[2] = in_value[num_buf-2];
    ref_delay_value[3] = in_value[num_buf-1];

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
    }

    /* allocate delay
     */
    ASSERT_VX_OBJECT(exemplar  = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(delay     = vxCreateDelay(context, (vx_reference)exemplar, 4), VX_TYPE_DELAY);
    vxReleaseScalar(&exemplar);

    ASSERT_VX_OBJECT(
        n0 = test_user_kernel_node( graph,
                d0[0], NULL,
                (vx_scalar)vxGetReferenceFromDelay(delay, 0), NULL),
                VX_TYPE_NODE);
    ASSERT_VX_OBJECT(
        n1 = test_user_kernel_node(graph,
                (vx_scalar)vxGetReferenceFromDelay(delay, 0), (vx_scalar)vxGetReferenceFromDelay(delay, -3),
                d2[0], NULL),
                VX_TYPE_NODE);

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ n1 index 2, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 2);

    /* set graph schedule config such that graph parameter @ index 0, 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    /* always auto age delay in pipelined graph */
    VX_CALL(vxRegisterAutoAging(graph, delay));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_delay4");
    log_graph_rt_trace(graph);

    #if 1
    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(
            vxCopyScalar(d0[buf_id],
            &in_value[buf_id],
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
    }
    {
        vx_scalar tmp_scalar;

        tmp_scalar = (vx_scalar)vxGetReferenceFromDelay(delay, -1);
        ASSERT_NO_FAILURE(
            vxCopyScalar(tmp_scalar,
            &in_value[num_buf-3],
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        tmp_scalar = (vx_scalar)vxGetReferenceFromDelay(delay, -2);
        ASSERT_NO_FAILURE(
            vxCopyScalar(tmp_scalar,
            &in_value[num_buf-2],
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        tmp_scalar = (vx_scalar)vxGetReferenceFromDelay(delay, -3);
        ASSERT_NO_FAILURE(
            vxCopyScalar(tmp_scalar,
            &in_value[num_buf-1],
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d2[buf_id], 1));
    }
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_scalar, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_scalar, 1, &num_refs));

        /* A graph execution completed, since we dequeued both input and output refs */
        if(arg_->measure_perf==0)
        {
            /* when measuring performance dont check output since it affects graph performance numbers
             */

            if(loop_cnt > 100)
            {
                ct_update_progress(loop_id, loop_cnt+num_buf);
            }
        }

        VX_CALL(vxCopyScalar(out_scalar, &tmp_value, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        ref_out_value = in_value[buf_id] + ref_delay_value[ 3 ];

        ref_delay_value[ 3 ] = ref_delay_value[ 2 ];
        ref_delay_value[ 2 ] = ref_delay_value[ 1 ];
        ref_delay_value[ 1 ] = in_value[buf_id];

        /* compare output */
        //printf(" %d: out = %d ref = %d\n", loop_id, tmp_value, ref_out_value);
        ASSERT_EQ_INT(tmp_value, ref_out_value);

        /* clear value in output */
        tmp_value = 0;
        VX_CALL(vxCopyScalar(out_scalar, &tmp_value, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_scalar, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_scalar, 1));
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0, n1 };

        printGraphPipelinePerformance(graph, nodes, 2, exe_time, loop_cnt+num_buf, 1);
    }
    #endif

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseScalar(&d0[buf_id]));
        VX_CALL(vxReleaseScalar(&d2[buf_id]));
    }
    VX_CALL(vxReleaseDelay(&delay));
    VX_CALL(vxReleaseGraph(&graph));

    test_user_kernel_unregister(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

TEST(tivxGraphPipeline, negativeTestPipelineDepth)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* explicitly set graph pipeline depth */
    ASSERT_NE_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, TIVX_GRAPH_MAX_PIPELINE_DEPTH));

    VX_CALL(vxReleaseGraph(&graph));
}

/*
 *  d0     n0     d2     n1     d3     n2     d4
 * IMG -- ADD -- IMG -- NOT -- IMG -- NOT -- IMG
 *         |                    |
 *         |                    |
 *         +--------------------+
 *
 * This test case test for loop carried dependency functional correctness
 *
 */
TEST_WITH_ARG(tivxGraphPipeline, testLoopCarriedDependency, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_delay delay;
    vx_image delay_image;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d2[MAX_NUM_BUF] = {NULL}, d4[MAX_NUM_BUF] = {NULL};
    vx_node n0, n1, n2;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

    int i;
    vx_graph graph_1 = 0;
    vx_image images[4];
    vx_node nodes[3];
    vx_delay delay_1 = 0;
    vx_image delay_image_0 = 0;
    vx_image delay_image_1 = 0;
    vx_image delay_image_0_nopipeline = 0;
    vx_image delay_image_1_nopipeline = 0;
    vx_imagepatch_addressing_t addr;
    vx_uint8 *pdata = 0;
    vx_rectangle_t rect = {0, 0, arg_->width, arg_->height};
    vx_map_id map_id;

    CT_Image ref_src[MAX_NUM_BUF] = {NULL}, ref_src1[MAX_NUM_BUF] = {NULL}, vxdst0, vxdst1;
    uint32_t width, height, seq_init, pipeline_depth, num_buf;
    uint32_t buf_id, loop_id, loop_cnt;
    uint64_t exe_time;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = arg_->width;
    height = arg_->height;
    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;
    loop_cnt = arg_->loop_count;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE({
            ref_src[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
            fillSequence(ref_src[buf_id], (uint32_t)(seq_init+buf_id*10));
        });
        ASSERT_NO_FAILURE({
            ref_src1[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
            fillSequence(ref_src1[buf_id], (uint32_t)(seq_init+buf_id*10));
        });
    }

    /* Non-pipelining graph */
    ASSERT_VX_OBJECT(graph_1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(images[0] = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(images[1] = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(images[2] = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(images[3] = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_image_copyto_vx_image(images[0], ref_src[0]));

    ASSERT_VX_OBJECT(delay_1 = vxCreateDelay(context, (vx_reference)images[3], 2), VX_TYPE_DELAY);

    ASSERT_VX_OBJECT(delay_image_0_nopipeline = (vx_image)vxGetReferenceFromDelay(delay_1, 0), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(delay_image_1_nopipeline = (vx_image)vxGetReferenceFromDelay(delay_1,-1), VX_TYPE_IMAGE);

    /* Filling reference data */
    pdata = NULL;
    VX_CALL(vxMapImagePatch(delay_image_0_nopipeline, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    for (i = 0; i < width*height; i++)
    {
        *(pdata+i) = 1;
    }
    VX_CALL(vxUnmapImagePatch(delay_image_0_nopipeline, map_id));

    pdata = NULL;
    VX_CALL(vxMapImagePatch(delay_image_1_nopipeline, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    for (i = 0; i < width*height; i++)
    {
        *(pdata+i) = 1;
    }
    VX_CALL(vxUnmapImagePatch(delay_image_1_nopipeline, map_id));

    ASSERT_VX_OBJECT(nodes[0] = vxAddNode(graph_1, images[0], (vx_image)vxGetReferenceFromDelay(delay_1, -1), VX_CONVERT_POLICY_WRAP, images[1]), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(nodes[1] = vxNotNode(graph_1, images[1], (vx_image)vxGetReferenceFromDelay(delay_1, 0)), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(nodes[2] = vxNotNode(graph_1, (vx_image)vxGetReferenceFromDelay(delay_1, 0), images[2]), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(nodes[0], VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(nodes[1], VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #else
    VX_CALL(vxSetNodeTarget(nodes[0], VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(nodes[1], VX_TARGET_STRING, TIVX_TARGET_DSP2));
    #endif

    VX_CALL(vxRegisterAutoAging(graph_1, delay_1));
    VX_CALL(vxVerifyGraph(graph_1));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d4[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(delay_image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)delay_image, 2), VX_TYPE_DELAY);

    ASSERT_VX_OBJECT(delay_image_0 = (vx_image)vxGetReferenceFromDelay(delay, 0), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(delay_image_1 = (vx_image)vxGetReferenceFromDelay(delay, -1), VX_TYPE_IMAGE);

    /* Filling reference data */
    pdata = NULL;
    VX_CALL(vxMapImagePatch(delay_image_0, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    for (i = 0; i < width*height; i++)
    {
        *(pdata+i) = 1;
    }
    VX_CALL(vxUnmapImagePatch(delay_image_0, map_id));

    pdata = NULL;
    VX_CALL(vxMapImagePatch(delay_image_1, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    for (i = 0; i < width*height; i++)
    {
        *(pdata+i) = 1;
    }
    VX_CALL(vxUnmapImagePatch(delay_image_1, map_id));


    ASSERT_VX_OBJECT(n0    = vxAddNode(graph, d0[0], (vx_image)vxGetReferenceFromDelay(delay, -1), VX_CONVERT_POLICY_WRAP, d2[0]), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, d2[0], (vx_image)vxGetReferenceFromDelay(delay, 0)), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n2    = vxNotNode(graph, (vx_image)vxGetReferenceFromDelay(delay, 0), d4[0]), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    #endif

    /* input0 @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ n0 index 2, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 2);
    /* output @ n2 index 1, becomes graph parameter 2 */
    add_graph_parameter_by_node_index(graph, n2, 1);

    /* set graph schedule config such that graph parameter @ index 0, 1, 2 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    graph_parameters_queue_params_list[2].graph_parameter_index = 2;
    graph_parameters_queue_params_list[2].refs_list_size = num_buf;
    graph_parameters_queue_params_list[2].refs_list = (vx_reference*)&d4[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                3,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    /* always auto age delay in pipelined graph */
    VX_CALL(vxRegisterAutoAging(graph, delay));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_two_nodes");
    log_graph_rt_trace(graph);

    #if 1
    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(d0[buf_id], ref_src[0]));
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d2[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&d4[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_image add_in0_img, add_out_img, not_out1_img;
        uint32_t num_refs;

        VX_CALL(vxProcessGraph(graph_1));

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 2, (vx_reference*)&not_out1_img, 1, &num_refs));

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&add_out_img, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&add_in0_img, 1, &num_refs));

        /* when measuring performance dont check output since it affects graph performance numbers
         */

        if(loop_cnt > 100)
        {
            ct_update_progress(loop_id, loop_cnt+num_buf);
        }

        if(arg_->measure_perf==0)
        {
            ASSERT_NO_FAILURE({
                vxdst0 = ct_image_from_vx_image(not_out1_img);
            });

            ASSERT_NO_FAILURE({
                vxdst1 = ct_image_from_vx_image(images[2]);
            });

            ASSERT_EQ_CTIMAGE(vxdst1, vxdst0);
        }

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */

        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&add_out_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&add_in0_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&not_out1_img, 1));
        }

    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    if(arg_->measure_perf==1)
    {
        vx_node nodes[] = { n0, n1, n2 };

        printGraphPipelinePerformance(graph, nodes, 3, exe_time, loop_cnt+num_buf, arg_->width*arg_->height);
    }
    #endif

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d2[buf_id]));
        VX_CALL(vxReleaseImage(&d4[buf_id]));
    }
    VX_CALL(vxReleaseImage(&delay_image));
    VX_CALL(vxReleaseDelay(&delay));
    VX_CALL(vxReleaseGraph(&graph));

    for (i = 0; i < (sizeof(nodes)/sizeof(nodes[0])); i++)
    {
        VX_CALL(vxReleaseNode(&nodes[i]));
    }

    for (i = 0; i < (sizeof(images)/sizeof(images[0])); i++)
    {
        VX_CALL(vxReleaseImage(&images[i]));
    }

    VX_CALL(vxReleaseGraph(&graph_1));
    VX_CALL(vxReleaseDelay(&delay_1));

    ASSERT(graph_1 == 0);
    ASSERT(delay_1 == 0);

    CT_CollectGarbage(CT_GC_ALL);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}


/*
 * This test case is a negative test that tries to create a data ref queue of images with inconsistent
 * values for the width and height of images
 *
 */
TEST(tivxGraphPipeline, negativeTestImageInconsistentRefs)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1, d2[MAX_NUM_BUF] = {NULL};
    vx_node n0;

    uint32_t initial_width, width, initial_height, height, num_buf, buf_id;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

    initial_width = 64;
    initial_height = 64;
    width = 128;
    height = 128;
    num_buf = 2;

    ASSERT(num_buf <= MAX_NUM_BUF);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    ASSERT_VX_OBJECT(d0[0]    = vxCreateImage(context, initial_width, initial_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d2[0]    = vxCreateImage(context, initial_width, initial_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(d0[1]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d2[1]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    /* create other refs, these are not multiple refs and same refs is fed as parameter to the graph */
    ASSERT_VX_OBJECT(d1    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    /* create node, input (index 0) and output (index 2) will be made as graph parameter
     * so that we can enqueue and dequeue refs to it and thus do graph pipelining.
     * d0[0], d2[0] used only for their meta data.
     * Actual input and output used for graph processing will be the
     * refs that are enqueued later
     */
    ASSERT_VX_OBJECT(n0    = vxOrNode(graph, d0[0], d1, d2[0]), VX_TYPE_NODE);

    /* input @ node index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ node index 2, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 2);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    /* This should fail because the queue params have inconsistent values for width and height
     */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    VX_CALL(vxReleaseNode(&n0));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d2[buf_id]));
    }
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 * This test case is a negative test that tries to create a data ref queue of images with inconsistent
 * values for parameters of object arrays
 *
 */
TEST(tivxGraphPipeline, negativeTestObjectArrayInconsistentRefs)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    uint32_t buf_id, num_buf;
    vx_node n0;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_exemplar;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    vx_object_array obj_array_source[MAX_NUM_BUF];

    /* Setting to num buf of capture node */
    num_buf = 3;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_exemplar  = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
         ASSERT_VX_OBJECT(obj_array_source[buf_id] = vxCreateObjectArray(context, (vx_reference)scalar_exemplar, buf_id+1), VX_TYPE_OBJECT_ARRAY);
    }

    VX_CALL(vxReleaseScalar(&scalar_exemplar));

    ASSERT_VX_OBJECT(n0 = tivxScalarSourceObjArrayNode(graph, obj_array_source[0]), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n0, "Source_node");

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 0);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&obj_array_source[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            1,
            graph_parameters_queue_params_list
            ));

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseGraph(&graph));

    /* since buffers could be held by source node, first release graph
     * to delete source node, then free the buffers
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&obj_array_source[buf_id]));
    }
    tivxTestKernelsUnLoadKernels(context);
}

/*
 * This test case is a negative test that tries to create a data ref queue of images with inconsistent
 * values for parameters of pyramids
 *
 */
TEST(tivxGraphPipeline, negativeTestPyramidInconsistentRefs)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node n1;
    vx_pyramid pyr_1[MAX_NUM_BUF];
    uint32_t width, height, i, j, val;
    uint32_t pipeline_depth, num_buf;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];

    width = 16;
    height = 16;
    num_buf = 3;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for (i = 0; i < num_buf; i++)
    {
        ASSERT_VX_OBJECT(pyr_1[i] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, width+i, height+i, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    }

    ASSERT_VX_OBJECT(n1 = tivxPyramidSourceNode(graph, pyr_1[0]), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 0);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&pyr_1[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            1,
            graph_parameters_queue_params_list
            ));

    for (i = 0; i < num_buf; i++)
    {
        VX_CALL(vxReleasePyramid(&pyr_1[i]));
    }
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

TEST(tivxGraphPipeline, testGraphPipelineQuery)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1, d2[MAX_NUM_BUF] = {NULL};
    vx_node n0;
    vx_uint32 pipe_depth_query;

    uint32_t width, height, num_buf, pipeline_depth, buf_id;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

    tivx_clr_debug_zone(VX_ZONE_INFO);

    width = 64;
    height = 48;
    pipeline_depth = 3;
    num_buf = 2;

    ASSERT(num_buf <= MAX_NUM_BUF);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    /* create other refs, these are not multiple refs and same refs is fed as parameter to the graph */
    ASSERT_VX_OBJECT(d1    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(n0    = vxOrNode(graph, d0[0], d1, d2[0]), VX_TYPE_NODE);

    /* input @ node index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ node index 2, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 2);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxQueryGraph(graph, TIVX_GRAPH_PIPELINE_DEPTH, &pipe_depth_query, sizeof(pipe_depth_query)));

    ASSERT(pipe_depth_query==pipeline_depth);

    VX_CALL(vxReleaseNode(&n0));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d2[buf_id]));
    }
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/* Test case for TIOVX-902: Querying serial graph pipe depth */
/*
 *  d0     n0     d1     n1     d2    n2     d3     n3      d5
 * IMG -- NOT -- IMG -- NOT -- IMG -- OR  -- IMG -- AND -- IMG
 *                |                   |              |
 *                +-------------------+             IMG
 *                                                   d4
 *
 * This test case test the below
 * - Same input going to multiple nodes
 * - Outputs from multiple nodes going to a single node
 * - Node taking input from another node as well as from user
 *
 */
TEST(tivxGraphPipeline, testGraphPipelineDepthDetectionSerial)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1, d2, d3, d4[MAX_NUM_BUF] = {NULL}, d5[MAX_NUM_BUF] = {NULL};
    vx_node  n0, n1, n2, n3;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

    CT_Image ref_src[MAX_NUM_BUF] = {NULL}, vxdst;
    uint32_t width, height, seq_init, pipe_depth_query, num_buf, tmp_num_buf, expected_pipe_depth = 4;
    uint32_t buf_id, loop_id, loop_cnt;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = 64;
    height = 48;
    num_buf = 2;
    loop_cnt = 1;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE({
            ref_src[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
            fillSequence(ref_src[buf_id], (uint32_t)(seq_init+buf_id*10));
        });
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d4[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d5[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(d1    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d2    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d3    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(n0    = vxNotNode(graph, d0[0], d1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, d1, d2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n2    = vxOrNode(graph, d1, d2, d3), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n3    = vxAndNode(graph, d3, d4[0], d5[0]), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    #endif

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* input @ n3 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n3, 1);
    /* output @ n3 index 2, becomes graph parameter 2 */
    add_graph_parameter_by_node_index(graph, n3, 2);

    /* set graph schedule config such that graph parameter @ index 0, 1, 2 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d4[0];

    graph_parameters_queue_params_list[2].graph_parameter_index = 2;
    graph_parameters_queue_params_list[2].refs_list_size = num_buf;
    graph_parameters_queue_params_list[2].refs_list = (vx_reference*)&d5[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                3,
                graph_parameters_queue_params_list
                ));

    tmp_num_buf = 2;
    VX_CALL(set_num_buf_by_node_index(n0, 1, tmp_num_buf));

    /* n1 and n2 run on different targets hence set output of n1 to have multiple buffers so
     * that n1 and n2 can run in a pipeline
     */
    VX_CALL(set_num_buf_by_node_index(n1, 1, num_buf));

    tmp_num_buf = 1;
    VX_CALL(set_num_buf_by_node_index(n2, 2, tmp_num_buf));

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxQueryGraph(graph, TIVX_GRAPH_PIPELINE_DEPTH, &pipe_depth_query, sizeof(pipe_depth_query)));

    ASSERT(pipe_depth_query==4);

    #if 1
    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(d0[buf_id], ref_src[buf_id]));
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(d4[buf_id], ref_src[buf_id]));
    }

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d4[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&d5[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_image out_img, in1_img, in2_img;
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 2, (vx_reference*)&out_img, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&in2_img, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in1_img, 1, &num_refs));

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in1_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&in2_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&out_img, 1));
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    #endif

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n3));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d4[buf_id]));
        VX_CALL(vxReleaseImage(&d5[buf_id]));
    }
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseImage(&d2));
    VX_CALL(vxReleaseImage(&d3));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/* Test case for TIOVX-902: Querying parallel graph pipe depth */
/*
 *  d0     n0     d1     n1     d2    n2     d3     n3      d5
 * IMG -- NOT -- IMG -- NOT -- IMG -- OR -- IMG -- AND -- IMG
 *                |                   |      d4     |
 *                |--------------------     IMG ----
 *                |      n4            n5
 *                +  -- NOT -- IMG -- NOT -- IMG
 *                             d6            d7
 */
TEST(tivxGraphPipeline, testGraphPipelineDepthDetectionParallel)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1, d2, d3, d4[MAX_NUM_BUF] = {NULL}, d5[MAX_NUM_BUF] = {NULL}, d6, d7[MAX_NUM_BUF] = {NULL};
    vx_node  n0, n1, n2, n3, n4, n5;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[4];

    CT_Image ref_src[MAX_NUM_BUF] = {NULL}, vxdst;
    uint32_t width, height, seq_init, pipe_depth_query, num_buf, expected_pipe_depth = 4;
    uint32_t buf_id, loop_id, loop_cnt;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = 64;
    height = 48;
    num_buf = 3;
    loop_cnt = 1;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE({
            ref_src[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
            fillSequence(ref_src[buf_id], (uint32_t)(seq_init+buf_id*10));
        });
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d4[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d5[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d7[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(d1    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d2    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d3    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d6    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(n0    = vxNotNode(graph, d0[0], d1), VX_TYPE_NODE);
    VX_CALL(vxSetReferenceName( (vx_reference)n0, "n0"));
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, d1, d2), VX_TYPE_NODE);
    VX_CALL(vxSetReferenceName( (vx_reference)n1, "n1"));
    ASSERT_VX_OBJECT(n2    = vxOrNode(graph, d1, d2, d3), VX_TYPE_NODE);
    VX_CALL(vxSetReferenceName( (vx_reference)n2, "n2"));
    ASSERT_VX_OBJECT(n3    = vxAndNode(graph, d3, d4[0], d5[0]), VX_TYPE_NODE);
    VX_CALL(vxSetReferenceName( (vx_reference)n3, "n3"));

    /* Parallel branch of graph */
    ASSERT_VX_OBJECT(n4    = vxNotNode(graph, d1, d6), VX_TYPE_NODE);
    VX_CALL(vxSetReferenceName( (vx_reference)n4, "n4"));
    ASSERT_VX_OBJECT(n5    = vxNotNode(graph, d6, d7[0]), VX_TYPE_NODE);
    VX_CALL(vxSetReferenceName( (vx_reference)n5, "n5"));

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n4, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n5, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    VX_CALL(vxSetNodeTarget(n4, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n5, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #endif

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* input @ n3 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n3, 1);
    /* output @ n3 index 2, becomes graph parameter 2 */
    add_graph_parameter_by_node_index(graph, n3, 2);
    /* output @ n3 index 2, becomes graph parameter 2 */
    add_graph_parameter_by_node_index(graph, n5, 1);

    /* set graph schedule config such that graph parameter @ index 0, 1, 2 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d4[0];

    graph_parameters_queue_params_list[2].graph_parameter_index = 2;
    graph_parameters_queue_params_list[2].refs_list_size = num_buf;
    graph_parameters_queue_params_list[2].refs_list = (vx_reference*)&d5[0];

    graph_parameters_queue_params_list[3].graph_parameter_index = 3;
    graph_parameters_queue_params_list[3].refs_list_size = num_buf;
    graph_parameters_queue_params_list[3].refs_list = (vx_reference*)&d7[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                4,
                graph_parameters_queue_params_list
                ));

    VX_CALL(set_num_buf_by_node_index(n0, 1, num_buf));

    /* n1 and n2 run on different targets hence set output of n1 to have multiple buffers so
     * that n1 and n2 can run in a pipeline
     */
    VX_CALL(set_num_buf_by_node_index(n1, 1, num_buf));

    VX_CALL(set_num_buf_by_node_index(n2, 2, num_buf));

    VX_CALL(set_num_buf_by_node_index(n4, 1, num_buf));

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxQueryGraph(graph, TIVX_GRAPH_PIPELINE_DEPTH, &pipe_depth_query, sizeof(pipe_depth_query)));

    ASSERT(pipe_depth_query==4);

    #if 1
    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d4[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&d5[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 3, (vx_reference*)&d7[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_image out_img1, out_img2, in1_img, in2_img;
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 3, (vx_reference*)&out_img2, 1, &num_refs));

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 2, (vx_reference*)&out_img1, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&in2_img, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in1_img, 1, &num_refs));

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in1_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&in2_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&out_img1, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 3, (vx_reference*)&out_img2, 1));
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    #endif

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n3));
    VX_CALL(vxReleaseNode(&n4));
    VX_CALL(vxReleaseNode(&n5));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d4[buf_id]));
        VX_CALL(vxReleaseImage(&d5[buf_id]));
        VX_CALL(vxReleaseImage(&d7[buf_id]));
    }
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseImage(&d2));
    VX_CALL(vxReleaseImage(&d3));
    VX_CALL(vxReleaseImage(&d6));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *  d0     n0     d1     n1     d2    n2     d3     n4      d5
 * IMG -- NOT -- IMG -- NOT -- IMG -- NOT -- IMG -- AND -- IMG
 *                |      n3                          |
 *                +  -- NOT -- IMG ------------------
 *                             d4
 */
TEST(tivxGraphPipeline, testGraphPipelineDepthDetectionBranches)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1, d2, d3, d4, d5[MAX_NUM_BUF] = {NULL};
    vx_node  n0, n1, n2, n3, n4;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

    CT_Image ref_src[MAX_NUM_BUF] = {NULL}, vxdst;
    uint32_t width, height, pipe_depth_query, num_buf, expected_pipe_depth = 4;
    uint32_t buf_id, loop_id, loop_cnt;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    width = 64;
    height = 48;
    num_buf = 3;
    loop_cnt = 1;

    ASSERT(num_buf <= MAX_NUM_BUF);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d5[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(d1    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d2    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d3    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d4    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(n0    = vxNotNode(graph, d0[0], d1), VX_TYPE_NODE);
    VX_CALL(vxSetReferenceName( (vx_reference)n0, "n0"));
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, d1, d2), VX_TYPE_NODE);
    VX_CALL(vxSetReferenceName( (vx_reference)n1, "n1"));
    ASSERT_VX_OBJECT(n2    = vxNotNode(graph, d2, d3), VX_TYPE_NODE);
    VX_CALL(vxSetReferenceName( (vx_reference)n2, "n2"));
    ASSERT_VX_OBJECT(n3    = vxNotNode(graph, d1, d4), VX_TYPE_NODE);
    VX_CALL(vxSetReferenceName( (vx_reference)n3, "n3"));
    ASSERT_VX_OBJECT(n4    = vxAndNode(graph, d3, d4, d5[0]), VX_TYPE_NODE);
    VX_CALL(vxSetReferenceName( (vx_reference)n4, "n4"));

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n4, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    VX_CALL(vxSetNodeTarget(n4, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #endif

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ n4 index 2, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n4, 2);

    /* set graph schedule config such that graph parameter @ index 0, 1, 2 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d5[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    VX_CALL(set_num_buf_by_node_index(n0, 1, num_buf));

    VX_CALL(set_num_buf_by_node_index(n1, 1, num_buf));

    VX_CALL(set_num_buf_by_node_index(n2, 1, num_buf));

    VX_CALL(set_num_buf_by_node_index(n3, 1, num_buf));

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxQueryGraph(graph, TIVX_GRAPH_PIPELINE_DEPTH, &pipe_depth_query, sizeof(pipe_depth_query)));

    ASSERT(pipe_depth_query==4);

    #if 1

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&d0[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&d5[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_image out_img, in_img;
        uint32_t num_refs;

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_img, 1, &num_refs));

        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_img, 1, &num_refs));

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_img, 1));
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    #endif

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n3));
    VX_CALL(vxReleaseNode(&n4));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d5[buf_id]));
    }
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseImage(&d2));
    VX_CALL(vxReleaseImage(&d3));
    VX_CALL(vxReleaseImage(&d4));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/* Test case for TIOVX-726 */
TEST(tivxGraphPipeline, testGraphPipelineErrorDetection)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1[MAX_NUM_BUF] = {NULL}, d2[MAX_NUM_BUF] = {NULL};
    vx_node n0, n1;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

    CT_Image ref_src[MAX_NUM_BUF] = {NULL}, vxdst;
    uint32_t width, height, seq_init, pipeline_depth, num_buf;
    uint32_t buf_id;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    width = 64;
    height = 48;
    pipeline_depth = 3;
    num_buf = 3;

    ASSERT(num_buf <= MAX_NUM_BUF);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d1[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(n0    = vxNotNode(graph, d0[0], d1[0]), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, d1[0], d2[0]), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    #endif

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* intermediate output @ n0 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 1);
    /* output @ n1 index 1, becomes graph parameter 2 */
    add_graph_parameter_by_node_index(graph, n1, 1);

    /* set graph schedule config such that graph parameter @ index 0, 1, 2 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d1[0];

    graph_parameters_queue_params_list[2].graph_parameter_index = 2;
    graph_parameters_queue_params_list[2].refs_list_size = num_buf;
    graph_parameters_queue_params_list[2].refs_list = (vx_reference*)&d2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                3,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    VX_CALL(set_num_buf_by_node_index(n0, 1, num_buf));

    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d1[buf_id]));
        VX_CALL(vxReleaseImage(&d2[buf_id]));
    }
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *  d0     n0     d1     n1     d2
 * IMG -- NOT -- IMG -- NOT -- IMG
 *
 * Expectation is that framework sets d1 to use 2 buffers
 */
TEST(tivxGraphPipeline, testBufferDepthDetection1)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1, d2[MAX_NUM_BUF] = {NULL};
    vx_node n0, n1;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

    uint32_t width, height, pipeline_depth;
    uint32_t buf_id, loop_id, num_buf, get_num_buf;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    width = 64;
    height = 48;
    num_buf = 2;
    pipeline_depth = 2;

    ASSERT(num_buf <= MAX_NUM_BUF);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(d1    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(n0    = vxNotNode(graph, d0[0], d1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, d1, d2[0]), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    #endif

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ n1 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    /* Validating tivxGetNodeParameterNumBufByIndex API */
    VX_CALL(tivxGetNodeParameterNumBufByIndex(n0, 1, &get_num_buf));

    ASSERT(get_num_buf==0);

    VX_CALL(vxVerifyGraph(graph));

    /* Validating tivxGetNodeParameterNumBufByIndex API */
    VX_CALL(tivxGetNodeParameterNumBufByIndex(n0, 1, &get_num_buf));

    ASSERT(get_num_buf==2);

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d2[buf_id]));
    }
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *  d0     n0     d1     n1     d2
 * IMG -- NOT -- IMG -- NOT -- IMG
 *                 |
 *                 |     n2     d3
 *                 | -- NOT -- IMG
 *
 * Expectation is that framework sets d1 to use 3 buffers
 *
 */
TEST(tivxGraphPipeline, testBufferDepthDetection2)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d0[MAX_NUM_BUF] = {NULL}, d1, d2[MAX_NUM_BUF] = {NULL}, d3[MAX_NUM_BUF] = {NULL};
    vx_node n0, n1, n2;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

    uint32_t width, height, pipeline_depth;
    uint32_t buf_id, loop_id, num_buf, get_num_buf;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    width = 64;
    height = 48;
    num_buf = 2;
    pipeline_depth = 2;

    ASSERT(num_buf <= MAX_NUM_BUF);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(d0[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d2[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(d3[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(d1    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(n0    = vxNotNode(graph, d0[0], d1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, d1, d2[0]), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n2    = vxNotNode(graph, d1, d3[0]), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    #endif

    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ n1 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 1);
    /* output @ n2 index 1, becomes graph parameter 2 */
    add_graph_parameter_by_node_index(graph, n2, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&d0[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&d2[0];

    graph_parameters_queue_params_list[2].graph_parameter_index = 2;
    graph_parameters_queue_params_list[2].refs_list_size = num_buf;
    graph_parameters_queue_params_list[2].refs_list = (vx_reference*)&d3[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                3,
                graph_parameters_queue_params_list
                ));

    /* explicitly set graph pipeline depth */
    VX_CALL(set_graph_pipeline_depth(graph, pipeline_depth));

    /* Validating tivxGetNodeParameterNumBufByIndex API */
    VX_CALL(tivxGetNodeParameterNumBufByIndex(n0, 1, &get_num_buf));

    ASSERT(get_num_buf==0);

    VX_CALL(vxVerifyGraph(graph));

    /* Validating tivxGetNodeParameterNumBufByIndex API */
    VX_CALL(tivxGetNodeParameterNumBufByIndex(n0, 1, &get_num_buf));

    ASSERT(get_num_buf==3);

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&d0[buf_id]));
        VX_CALL(vxReleaseImage(&d2[buf_id]));
        VX_CALL(vxReleaseImage(&d3[buf_id]));
    }
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseGraph(&graph));

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

TEST(tivxGraphPipelineLdra, negativeTestSetGraphScheduleConfig)
{
    #define VX_GRAPH_SCHEDULE_MODE_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_graph graph = NULL, graph1 = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;
    vx_parameter param = NULL;
    vx_enum graph_schedule_mode = VX_GRAPH_SCHEDULE_MODE_NORMAL;
    vx_uint32 graph_parameters_list_size = 0;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2] = {0};

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetGraphScheduleConfig(graph, graph_schedule_mode, graph_parameters_list_size, graph_parameters_queue_params_list));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    graph_schedule_mode = VX_GRAPH_SCHEDULE_MODE_NORMAL;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetGraphScheduleConfig(graph, graph_schedule_mode, graph_parameters_list_size, graph_parameters_queue_params_list));

    graph_schedule_mode = VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO;
    graph_parameters_list_size = 1;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetGraphScheduleConfig(graph, graph_schedule_mode, graph_parameters_list_size, graph_parameters_queue_params_list));

    graph_schedule_mode = VX_GRAPH_SCHEDULE_MODE_DEFAULT;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetGraphScheduleConfig(graph, graph_schedule_mode, graph_parameters_list_size, graph_parameters_queue_params_list));

    VX_CALL(vxVerifyGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetGraphScheduleConfig(graph, graph_schedule_mode, graph_parameters_list_size, graph_parameters_queue_params_list));

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph1, kernel), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(node, 0), VX_TYPE_PARAMETER);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxAddParameterToGraph(graph1, param));
    graph_schedule_mode = VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetGraphScheduleConfig(graph1, graph_schedule_mode, graph_parameters_list_size, graph_parameters_queue_params_list));
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = 0;
    graph_parameters_queue_params_list[0].refs_list = NULL;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetGraphScheduleConfig(graph1, graph_schedule_mode, graph_parameters_list_size, graph_parameters_queue_params_list));

    VX_CALL(vxReleaseParameter(&param));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph1));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxGraphPipelineLdra, negativeTestGraphParameterCheckDoneRef)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_uint32 graph_parameter_index = 0;
    vx_uint32 *num_refs = NULL;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxGraphParameterCheckDoneRef(graph, graph_parameter_index, num_refs));
}

TEST(tivxGraphPipelineLdra, negativeTestGraphParameterDequeueDoneRef)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_uint32 graph_parameter_index = 0;
    vx_image image;
    vx_uint32 max_refs = 1;
    vx_uint32 num_refs;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxGraphParameterDequeueDoneRef(graph, graph_parameter_index, (vx_reference *)&image, max_refs, &num_refs));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxGraphParameterDequeueDoneRef(graph, graph_parameter_index, (vx_reference *)&image, max_refs, &num_refs));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxGraphPipelineLdra, negativeTestGraphParameterEnqueueReadyRef)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_reference ref = NULL;
    vx_uint32 gpi = 0, num_refs = 0, flags = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxGraphParameterEnqueueReadyRef(graph, gpi, &ref, num_refs, flags));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxGraphParameterEnqueueReadyRef(graph, gpi, &ref, 1, flags));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxGraphPipelineLdra, negativeTestSetGraphPipelineDepth)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_image input = NULL, accum = NULL;
    vx_node node = NULL;
    vx_uint32 pipeline_depth = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, tivxSetGraphPipelineDepth(graph, pipeline_depth));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(input = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(accum = vxCreateImage(context, 128, 128, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node = vxAccumulateImageNode(graph, input, accum), VX_TYPE_NODE);
    VX_CALL(vxVerifyGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, tivxSetGraphPipelineDepth(graph, pipeline_depth));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseImage(&accum));
    VX_CALL(vxReleaseImage(&input));
    VX_CALL(vxReleaseGraph(&graph));
}

TESTCASE_TESTS(tivxGraphPipeline,
    testOneNode,
    testTwoNodesBasic,
    testInputMultipleEnqueue,
    testTwoNodes,
    testFourNodes,
    /*testMaxDataRef,
    negativeTestGraphMaxDataRef,*/
    negativeTestMaxDataRefQ,
    testUniformImage,
    testScalarOutput,
    testEventHandling,
    testEventHandlingGraphVerify,
    testEventHandlingDisableEvents,
    testReplicateImage,
    DISABLED_testDontReplicateImage,
    testReplicateImage2,
    testUserKernel,
    testManualSchedule,
    testDelay1,
    testDelay2,
    testDelay3,
    testDelay4,
    negativeTestPipelineDepth,
    testLoopCarriedDependency,
    negativeTestImageInconsistentRefs,
    negativeTestObjectArrayInconsistentRefs,
    negativeTestPyramidInconsistentRefs,
    testGraphPipelineQuery,
    testGraphPipelineDepthDetectionSerial,
    testGraphPipelineDepthDetectionParallel,
    testGraphPipelineDepthDetectionBranches,
    testGraphPipelineErrorDetection,
    testBufferDepthDetection1,
    testBufferDepthDetection2
)

TESTCASE_TESTS(
    tivxGraphPipelineLdra,
    negativeTestSetGraphScheduleConfig,
    negativeTestGraphParameterCheckDoneRef,
    negativeTestGraphParameterDequeueDoneRef,
    negativeTestGraphParameterEnqueueReadyRef,
    negativeTestSetGraphPipelineDepth
)

