/*
*
* Copyright (c) 2021 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

/** \brief Unit test for TI OpenVX TVM target kernel
 * The C7x TVM deployable module used in this unit test is here:
 *     ${VX_TEST_DATA_PATH}/tivx/tvm_models/c7x_deploy_tvm.out
 * It is generated with test_ovx_tvm.py.  The model/network is computing
 * a float 1x480x14x14 input tensor multiplied by a float 1x480x1x1 constant
 * tensor of value 2.0.  The c7x deploy module will be sent to tiovx tvm
 * target kernel on C7x and processed on C7x.
 */


#include <TI/tivx.h>
#include <TI/j7_tvm.h>
#include <TI/tivx_mem.h>
#include "test_engine/test.h"
#include <float.h>
#include <math.h>
#include <TI/tivx_task.h>
#include <inttypes.h>

#define DEBUG_TEST_TVM
/* #define DEBUG_TEST_TVM_VERBOSE */
/* #define TIOVX_TVM_STANDALONE_TEST */

/*
 * This is the size of trace buffer allocated in host memory and
 * shared with target.
 */
#define TIVX_TVM_TRACE_DATA_SIZE  (2 * 1024 * 1024)

#define TEST_TVM_OUT_TENSOR1_ELEMENTS (1*480*14*14)
#define TEST_TVM_OUT_TENSOR2_ELEMENTS (1*480*1*1)

#if defined(SOC_AM62A)
#define TVM_SMALL_TEST "%s/tivx/tvm_models/am62a/ovx_tvm_test_small.out"
#define TVM_LARGE_TEST "%s/tivx/tvm_models/am62a/ovx_tvm_test_large.out"
#else
#define TVM_SMALL_TEST "%s/tivx/tvm_models/j7/ovx_tvm_test_small.out"
#define TVM_LARGE_TEST "%s/tivx/tvm_models/j7/ovx_tvm_test_large.out"
#endif


TESTCASE(tivxTVM, CT_VXContext, ct_setup_vx_context, 0)

static char *testTVM_targets[] = {
  TIVX_TARGET_DSP_C7_1_PRI_1,
  TIVX_TARGET_DSP_C7_1_PRI_2,
  TIVX_TARGET_DSP_C7_1_PRI_3,
  TIVX_TARGET_DSP_C7_1_PRI_4,
  TIVX_TARGET_DSP_C7_1_PRI_5,
  TIVX_TARGET_DSP_C7_1_PRI_6,
  TIVX_TARGET_DSP_C7_1_PRI_7,
  TIVX_TARGET_DSP_C7_1_PRI_8,
#if defined(SOC_J784S4)
  TIVX_TARGET_DSP_C7_2_PRI_1,
  TIVX_TARGET_DSP_C7_2_PRI_2,
  TIVX_TARGET_DSP_C7_2_PRI_3,
  TIVX_TARGET_DSP_C7_2_PRI_4,
  TIVX_TARGET_DSP_C7_2_PRI_5,
  TIVX_TARGET_DSP_C7_2_PRI_6,
  TIVX_TARGET_DSP_C7_2_PRI_7,
  TIVX_TARGET_DSP_C7_2_PRI_8,
  TIVX_TARGET_DSP_C7_3_PRI_1,
  TIVX_TARGET_DSP_C7_3_PRI_2,
  TIVX_TARGET_DSP_C7_3_PRI_3,
  TIVX_TARGET_DSP_C7_3_PRI_4,
  TIVX_TARGET_DSP_C7_3_PRI_5,
  TIVX_TARGET_DSP_C7_3_PRI_6,
  TIVX_TARGET_DSP_C7_3_PRI_7,
  TIVX_TARGET_DSP_C7_3_PRI_8,
  TIVX_TARGET_DSP_C7_4_PRI_1,
  TIVX_TARGET_DSP_C7_4_PRI_2,
  TIVX_TARGET_DSP_C7_4_PRI_3,
  TIVX_TARGET_DSP_C7_4_PRI_4,
  TIVX_TARGET_DSP_C7_4_PRI_5,
  TIVX_TARGET_DSP_C7_4_PRI_6,
  TIVX_TARGET_DSP_C7_4_PRI_7,
  TIVX_TARGET_DSP_C7_4_PRI_8,
#endif
};

static vx_user_data_object createConfig(vx_context context,
                                        int32_t num_input_tensors,
                                        int32_t num_output_tensors,
                                        vx_size in_tensor_size_in_bytes,
                                        vx_size out_tensor_size_in_bytes,
                                        int32_t tvm_rt_debug_level);
static vx_user_data_object readDeployMod(vx_context context, char *mod_file);

static vx_user_data_object createConfig(vx_context context,
                                        int32_t num_input_tensors,
                                        int32_t num_output_tensors,
                                        vx_size in_tensor_size_in_bytes,
                                        vx_size out_tensor_size_in_bytes,
                                        int32_t tvm_rt_debug_level)
{
    vx_status status;
    vx_user_data_object config;
    vx_map_id  map_id;
    tivxTVMJ7Params *tvmParams = NULL;

    config = vxCreateUserDataObject(context, "tivxTVMJ7Params",
                                    sizeof(tivxTVMJ7Params), NULL);
    if (VX_SUCCESS != vxGetStatus((vx_reference)config))
    {
        printf("ERROR: Unable to create tvm config user data object\n");
        return NULL;
    }

    status = vxMapUserDataObject(config, 0, sizeof(tivxTVMJ7Params), &map_id,
                   (void **)&tvmParams, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);
    if ((VX_SUCCESS != status) || (tvmParams == NULL))
    {
        printf("ERROR: Unable to map tvm config user data object\n");
        vxReleaseUserDataObject(&config);
        return NULL;
    }

    memset(tvmParams, 0, sizeof(tivxTVMJ7Params));
    tvmParams->num_input_tensors = num_input_tensors;
    tvmParams->num_output_tensors = num_output_tensors;
    tvmParams->input_names_offset[0] = 0;
    strcpy((char *) tvmParams->input_names, "data");
    tvmParams->tensors_params[0].size_in_bytes = in_tensor_size_in_bytes;
    tvmParams->tensors_params[1].size_in_bytes = out_tensor_size_in_bytes;
    tvmParams->optimize_ivision_activation = 1;
    tvmParams->rt_info.max_preempt_delay = FLT_MAX;
    tvmParams->rt_info.tvm_rt_debug_level = tvm_rt_debug_level;
    tvmParams->rt_info.tvm_rt_trace_node = -1;

    #ifdef DEBUG_TEST_TVM_VERBOSE
    tvmParams->rt_info.tvm_rt_debug_level = 4;
    #endif

    vxUnmapUserDataObject(config, map_id);

    return config;
}

static vx_user_data_object readDeployMod(vx_context context, char *mod_file)
{
    vx_status status;

    vx_user_data_object  deploy_mod;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void      *mod_buffer = NULL;
    vx_size read_count;

    FILE *fp_mod;

    #ifdef DEBUG_TEST_TVM
    printf("Reading C7x deployable module file %s ...\n", mod_file);
    #endif

    fp_mod = fopen(mod_file, "rb");

    if (fp_mod == NULL)
    {
        printf("ERROR: Unable to open c7x deploy mod file %s \n", mod_file);
        return NULL;
    }

    fseek(fp_mod, 0, SEEK_END);
    capacity = ftell(fp_mod);
    fseek(fp_mod, 0, SEEK_SET);

    deploy_mod = vxCreateUserDataObject(context, "tivxTVMJ7DeployMod", capacity, NULL);

    status = vxGetStatus((vx_reference)deploy_mod);

    if (VX_SUCCESS == status)
    {
        status = vxMapUserDataObject(deploy_mod, 0, capacity, &map_id,
                            (void **)&mod_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if (VX_SUCCESS == status)
        {
            if(mod_buffer) {
                read_count = fread(mod_buffer, capacity, 1, fp_mod);
                if(read_count != 1)
                {
                    printf("ERROR: Unable to read c7x deploy mod file\n");
                }
            } else {
                printf("ERROR: Unable to allocate memory for reading c7x deploy mod file of %d bytes\n", capacity);
            }

            vxUnmapUserDataObject(deploy_mod, map_id);

            #ifdef DEBUG_TEST_TVM
            printf("Finished reading c7x deploy mod file of %d bytes\n", capacity);
            #endif
        }
    }

    fclose(fp_mod);

    return deploy_mod;
}


static void ct_teardown_tvm_kernels(void/*vx_context*/ **context_)
{
    vx_context context = (vx_context)*context_;
    if (context == NULL)
    {
        return;
    }

    if (CT_HasFailure())
    {
        tivxTVMUnLoadKernels(context);
    }
}

TEST(tivxTVM, testTVM)
{
    #ifdef DEBUG_TEST_TVM
    printf("tivxTVM testTVM ...\n");
    #endif

    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_kernel kernel = 0;

    vx_user_data_object  config;
    vx_user_data_object  deploy_mod;
    vx_user_data_object  traceData = NULL;
    void *trace_ptr = NULL;
    int32_t trace_written_size = 0;

    vx_tensor input_tensors[1];
    vx_tensor output_tensors[1];

    char filepath[MAXPATHLENGTH];
    size_t sz;
    vx_size starts[1] = { 0 };
    vx_size ends[1] = { TEST_TVM_OUT_TENSOR1_ELEMENTS * sizeof(float) };
    vx_size strides[1] = { sizeof(vx_uint8) };
    void   *ptr;
    int32_t i, j, num_targets;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DSP_C7_1))
    {
        vx_status status = VX_SUCCESS;
        uint32_t num_input_tensors  = 1;
        uint32_t num_output_tensors = 1;
        vx_map_id map_id;

        tivxTVMLoadKernels(context);
        #ifndef TIOVX_TVM_STANDALONE_TEST
        CT_RegisterForGarbageCollection(context, ct_teardown_tvm_kernels, CT_GC_OBJECT);
        #endif

        /* Create userdata object for TVMRT configuration parameters */
        config = createConfig(context, num_input_tensors, num_output_tensors,
                              ends[0], ends[0], 3 /*tvm_rt_debug_level*/);
        ASSERT(VX_SUCCESS == vxGetStatus((vx_reference)config));
        #ifdef DEBUG_TEST_TVM
        printf("Finished setting TVM config\n");
        #endif

        /* Create userdata object for c7x depolyable module */
        sz = snprintf(filepath, MAXPATHLENGTH, TVM_SMALL_TEST, ct_get_test_file_path());
        ASSERT(sz < MAXPATHLENGTH);
        deploy_mod = readDeployMod(context, filepath);
        ASSERT(VX_SUCCESS == vxGetStatus((vx_reference)deploy_mod));
        #ifdef DEBUG_TEST_TVM
        printf("Finished setting c7x deployable module\n");
        #endif

        /* Create trace data */
        ASSERT_VX_OBJECT(traceData = vxCreateUserDataObject(context, "TVM_traceData", TIVX_TVM_TRACE_DATA_SIZE, NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        #ifdef DEBUG_TEST_TVM
        printf("Finished setting c7x TVM trace\n");
        #endif

        /* Create vx_tensor objects for input/output tensors */
        ASSERT_VX_OBJECT(input_tensors[0] = vxCreateTensor(context, 1,
                ends, VX_TYPE_UINT8, 0), (enum vx_type_e)VX_TYPE_TENSOR);
        ASSERT_VX_OBJECT(output_tensors[0] = vxCreateTensor(context, 1,
                ends, VX_TYPE_UINT8, 0), (enum vx_type_e)VX_TYPE_TENSOR);
        ASSERT(VX_SUCCESS == tivxMapTensorPatch(input_tensors[0], 1, starts,
             ends, &map_id, strides, &ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        for (i = 0; i < TEST_TVM_OUT_TENSOR1_ELEMENTS; i++)
        {
            ((float *)ptr)[i] = i;
        }
        tivxUnmapTensorPatch(input_tensors[0], map_id);
        #ifdef DEBUG_TEST_TVM
        printf("Finished setting input/output tensors\n");
        #endif

        kernel = tivxAddKernelTVM(context, num_input_tensors, num_output_tensors);

        vx_reference params[] = {
                (vx_reference)config,
                (vx_reference)deploy_mod,
                (vx_reference)traceData
        };

        num_targets = sizeof(testTVM_targets) / sizeof(char*);
        for (j = 0; j < num_targets; j++)
        {
            if (vx_true_e == tivxIsTargetEnabled(testTVM_targets[j]))
            {
                #ifdef DEBUG_TEST_TVM
                printf("Testing on target %s ...\n", testTVM_targets[j]);
                #endif

                ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

                ASSERT_VX_OBJECT(node = tivxTVMNode(graph, kernel, params,
                        input_tensors, output_tensors), VX_TYPE_NODE);

                VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, testTVM_targets[j]));

                #ifdef DEBUG_TEST_TVM
                printf("Verifying graph ...\n");
                #endif
                VX_CALL(vxVerifyGraph(graph));

                #ifdef DEBUG_TEST_TVM
                printf("Running graph ...\n");
                #endif
                VX_CALL(vxProcessGraph(graph));

                #ifdef DEBUG_TEST_TVM
                printf("Verifying output ...\n");
                #endif
                ASSERT(VX_SUCCESS == tivxMapTensorPatch(output_tensors[0], 1, starts,
                      ends, &map_id, strides, &ptr, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST));
                for (i = 0; i < TEST_TVM_OUT_TENSOR1_ELEMENTS; i++)
                {
                    if (((float *) ptr)[i] != (i * 2))
                    {
                      printf("ERROR: Result [%d] differ: %f\n", i,  ((float *) ptr)[i]);
                      status = VX_FAILURE;
                      break;
                    }
                    ((float *) ptr)[i] = 0.0f;  /* reset for testing next target */
                }
                tivxUnmapTensorPatch(output_tensors[0], map_id);

                /* Checking trace */
                ASSERT(VX_SUCCESS == vxMapUserDataObject(traceData, 0, TIVX_TVM_TRACE_DATA_SIZE, &map_id,
                   (void **)&trace_ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
                memcpy(&trace_written_size, trace_ptr, sizeof(int32_t));
                #ifdef DEBUG_TEST_TVM
                printf("Trace written size: %d\n", trace_written_size);
                #endif
                ASSERT(trace_written_size > 0);
                vxUnmapUserDataObject(traceData, map_id);

                VX_CALL(vxReleaseNode(&node));
                VX_CALL(vxReleaseGraph(&graph));
            }
        }

        #ifdef DEBUG_TEST_TVM
        printf("Tearing down ...\n");
        #endif

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseUserDataObject(&config));
        VX_CALL(vxReleaseUserDataObject(&deploy_mod));
        VX_CALL(vxReleaseUserDataObject(&traceData));
        VX_CALL(vxReleaseTensor(&input_tensors[0]));
        VX_CALL(vxReleaseTensor(&output_tensors[0]));

        ASSERT(config == 0);
        ASSERT(deploy_mod == 0);
        ASSERT(input_tensors[0]  == 0);
        ASSERT(output_tensors[0] == 0);

        tivxTVMUnLoadKernels(context);
        VX_CALL(vxRemoveKernel(kernel));

        ASSERT(VX_SUCCESS == status);
    }
    tivx_clr_debug_zone(VX_ZONE_INFO);
}

TEST(tivxTVM, testTVMPreempt)
{
    #ifdef DEBUG_TEST_TVM
    printf("tivxTVM testTVMPreempt ...\n");
    #endif

    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_graph graph_large = 0;
    vx_node node = 0;
    vx_node node_large = 0;
    vx_kernel kernel = 0;

    vx_perf_t perf_small, perf_1, perf_2;

    vx_user_data_object  config, config_large;
    vx_user_data_object  deploy_mod, deploy_mod_large;
    vx_user_data_object  traceData = NULL;

    vx_tensor input_tensors[1];
    vx_tensor input_tensors_large[1];
    vx_tensor output_tensors[1];
    vx_tensor output_tensors_large[1];

    char filepath[MAXPATHLENGTH];
    size_t sz;
    vx_size starts[1] = { 0 };
    vx_size ends[1] = { TEST_TVM_OUT_TENSOR1_ELEMENTS * sizeof(float) };
    vx_size ends_large[1] = { TEST_TVM_OUT_TENSOR2_ELEMENTS * sizeof(float) };
    vx_size strides[1] = { sizeof(vx_uint8) };
    void   *ptr;
    int32_t i;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DSP_C7_1)) &&
        (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DSP_C7_1_PRI_2)))
    {
        vx_status status = VX_SUCCESS;
        uint32_t num_input_tensors  = 1;
        uint32_t num_output_tensors = 1;
        vx_map_id map_id;

        tivxTVMLoadKernels(context);
        #ifndef TIOVX_TVM_STANDALONE_TEST
        CT_RegisterForGarbageCollection(context, ct_teardown_tvm_kernels, CT_GC_OBJECT);
        #endif

        /* Create userdata object for TVMRT configuration parameters */
        config = createConfig(context, num_input_tensors, num_output_tensors,
                              ends[0], ends[0], 0 /*tvm_rt_debug_level*/);
        ASSERT(VX_SUCCESS == vxGetStatus((vx_reference)config));
        config_large = createConfig(context, num_input_tensors, num_output_tensors,
                              ends[0], ends_large[0], 0 /*tvm_rt_debug_level*/);
        ASSERT(VX_SUCCESS == vxGetStatus((vx_reference)config_large));
        #ifdef DEBUG_TEST_TVM
        printf("Finished setting TVM config\n");
        #endif

        /* Create userdata object for c7x depolyable module */
        sz = snprintf(filepath, MAXPATHLENGTH, TVM_SMALL_TEST, ct_get_test_file_path());
        ASSERT(sz < MAXPATHLENGTH);
        deploy_mod = readDeployMod(context, filepath);
        ASSERT(VX_SUCCESS == vxGetStatus((vx_reference)deploy_mod));
        sz = snprintf(filepath, MAXPATHLENGTH, TVM_LARGE_TEST, ct_get_test_file_path());
        ASSERT(sz < MAXPATHLENGTH);
        deploy_mod_large = readDeployMod(context, filepath);
        ASSERT(VX_SUCCESS == vxGetStatus((vx_reference)deploy_mod_large));
        #ifdef DEBUG_TEST_TVM
        printf("Finished setting c7x deployable module\n");
        #endif

        /* Create vx_tensor objects for input/output tensors */
        ASSERT_VX_OBJECT(input_tensors[0] = vxCreateTensor(context, 1,
                ends, VX_TYPE_UINT8, 0), (enum vx_type_e)VX_TYPE_TENSOR);
        ASSERT_VX_OBJECT(input_tensors_large[0] = vxCreateTensor(context, 1,
                ends, VX_TYPE_UINT8, 0), (enum vx_type_e)VX_TYPE_TENSOR);
        ASSERT_VX_OBJECT(output_tensors[0] = vxCreateTensor(context, 1,
                ends, VX_TYPE_UINT8, 0), (enum vx_type_e)VX_TYPE_TENSOR);
        ASSERT_VX_OBJECT(output_tensors_large[0] = vxCreateTensor(context, 1,
                ends_large, VX_TYPE_UINT8, 0), (enum vx_type_e)VX_TYPE_TENSOR);
        ASSERT(VX_SUCCESS == tivxMapTensorPatch(input_tensors[0], 1, starts,
             ends, &map_id, strides, &ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        for (i = 0; i < TEST_TVM_OUT_TENSOR1_ELEMENTS; i++)
        {
            ((float *) ptr)[i] = i;
        }
        tivxUnmapTensorPatch(input_tensors[0], map_id);
        ASSERT(VX_SUCCESS == tivxMapTensorPatch(input_tensors_large[0], 1, starts,
             ends, &map_id, strides, &ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        for (i = 0; i < TEST_TVM_OUT_TENSOR1_ELEMENTS; i++)
        {
            ((float *) ptr)[i] = 1;
        }
        tivxUnmapTensorPatch(input_tensors_large[0], map_id);
        #ifdef DEBUG_TEST_TVM
        printf("Finished setting input/output tensors\n");
        #endif

        kernel = tivxAddKernelTVM(context, num_input_tensors, num_output_tensors);

        vx_reference params[] = {
                (vx_reference)config,
                (vx_reference)deploy_mod,
                (vx_reference)traceData
        };
        vx_reference params_large[] = {
                (vx_reference)config_large,
                (vx_reference)deploy_mod_large,
                (vx_reference)traceData
        };

        /* test: running two graphs */
        {
            ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
            ASSERT_VX_OBJECT(graph_large = vxCreateGraph(context), VX_TYPE_GRAPH);

            ASSERT_VX_OBJECT(node = tivxTVMNode(graph, kernel, params,
                    input_tensors, output_tensors), VX_TYPE_NODE);
            ASSERT_VX_OBJECT(node_large = tivxTVMNode(graph_large, kernel, params_large,
                    input_tensors_large, output_tensors_large), VX_TYPE_NODE);

            VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_1_PRI_1));
            VX_CALL(vxSetNodeTarget(node_large, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_1_PRI_2));

            #ifdef DEBUG_TEST_TVM
            printf("Verifying graph ...\n");
            #endif
            VX_CALL(vxVerifyGraph(graph));
            VX_CALL(vxVerifyGraph(graph_large));

            #ifdef DEBUG_TEST_TVM
            printf("Running graph ...\n");
            #endif
            /* first run: large graph only */
            VX_CALL(vxScheduleGraph(graph_large));
            VX_CALL(vxWaitGraph(graph_large));
            VX_CALL(vxQueryNode(node_large, VX_NODE_PERFORMANCE, &perf_1, sizeof(perf_1)));

            /* second run, large graph first, preempted by small graph */
            VX_CALL(vxScheduleGraph(graph_large));
            for (i = 0; i < 5; i++)
            {
                tivxTaskWaitMsecs(1);
                VX_CALL(vxScheduleGraph(graph));
                VX_CALL(vxWaitGraph(graph));
            }
            VX_CALL(vxWaitGraph(graph_large));

            #ifdef DEBUG_TEST_TVM
            printf("Get timing ...\n");
            #endif

            VX_CALL(vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_small, sizeof(perf_small)));
            VX_CALL(vxQueryNode(node_large, VX_NODE_PERFORMANCE, &perf_2, sizeof(perf_2)));

            printf("first run large perf = %" PRIu64 "\n", perf_1.avg);
            printf("small perf.avg = %" PRIu64 ", min = %" PRIu64 ", max = %"
                   PRIu64 "\n", perf_small.avg, perf_small.min, perf_small.max);
            printf("second run large perf = %" PRIu64 "\n", perf_2.sum - perf_1.avg);
            /* first large run time + small run time < second large run time */
            ASSERT(perf_1.avg + perf_small.min < perf_2.sum - perf_1.avg);

            #ifdef DEBUG_TEST_TVM
            printf("Verifying output ...\n");
            #endif
            ASSERT(VX_SUCCESS == tivxMapTensorPatch(output_tensors[0], 1, starts,
                  ends, &map_id, strides, &ptr, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST));
            for (i = 0; i < TEST_TVM_OUT_TENSOR1_ELEMENTS; i++)
            {
                if (((float *) ptr)[i] != (i * 2))
                {
                  printf("ERROR: Small result [%d] differ: %f\n", i,  ((float *) ptr)[i]);
                  status = VX_FAILURE;
                  break;
                }
                ((float *) ptr)[i] = 0.0f;  /* reset for testing next target */
            }
            tivxUnmapTensorPatch(output_tensors[0], map_id);
            ASSERT(VX_SUCCESS == tivxMapTensorPatch(output_tensors_large[0], 1, starts,
                  ends_large, &map_id, strides, &ptr, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST));
            for (i = 0; i < TEST_TVM_OUT_TENSOR2_ELEMENTS; i++)
            {
                if (((float *) ptr)[i] != 16384.0f)
                {
                  printf("ERROR: Large result [%d] differ: %f\n", i,  ((float *) ptr)[i]);
                  status = VX_FAILURE;
                  break;
                }
                ((float *) ptr)[i] = 0.0f;  /* reset for testing next target */
            }
            tivxUnmapTensorPatch(output_tensors_large[0], map_id);

            VX_CALL(vxReleaseNode(&node));
            VX_CALL(vxReleaseGraph(&graph));
            VX_CALL(vxReleaseNode(&node_large));
            VX_CALL(vxReleaseGraph(&graph_large));
        }

        #ifdef DEBUG_TEST_TVM
        printf("Tearing down ...\n");
        #endif

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(node_large == 0);
        ASSERT(graph_large == 0);

        VX_CALL(vxReleaseUserDataObject(&config));
        VX_CALL(vxReleaseUserDataObject(&config_large));
        VX_CALL(vxReleaseUserDataObject(&deploy_mod));
        VX_CALL(vxReleaseUserDataObject(&deploy_mod_large));
        VX_CALL(vxReleaseTensor(&input_tensors[0]));
        VX_CALL(vxReleaseTensor(&input_tensors_large[0]));
        VX_CALL(vxReleaseTensor(&output_tensors[0]));
        VX_CALL(vxReleaseTensor(&output_tensors_large[0]));

        ASSERT(config == 0);
        ASSERT(config_large == 0);
        ASSERT(deploy_mod == 0);
        ASSERT(deploy_mod_large == 0);
        ASSERT(input_tensors[0]  == 0);
        ASSERT(input_tensors_large[0]  == 0);
        ASSERT(output_tensors[0] == 0);
        ASSERT(output_tensors_large[0] == 0);

        tivxTVMUnLoadKernels(context);
        VX_CALL(vxRemoveKernel(kernel));

        ASSERT(VX_SUCCESS == status);
    }
    tivx_clr_debug_zone(VX_ZONE_INFO);
}

TESTCASE_TESTS(tivxTVM, testTVM, testTVMPreempt)
