/*
 *
 * Copyright (c) 2024 Texas Instruments Incorporated
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

#include "test_engine/test.h"
#include "test_tiovx.h"

#include <VX/vx.h>
#include <TI/tivx_test_kernels.h>
#include <TI/tivx_capture_nodes.h>
#include <TI/tivx_task.h>

#define OUTPUT_FILE     "tiovx_overhead.html"

#define ITERATIONS      100u

static FILE *file = NULL;
static uint8_t g_core_counter = 0U;

static void openFile()
{
    char filepath[MAXPATHLENGTH];
    size_t sz;

    if (!file)
    {
        sz = snprintf(filepath, MAXPATHLENGTH, "%s/output/%s", ct_get_test_file_path(),
            OUTPUT_FILE);
        ASSERT(sz < MAXPATHLENGTH);

        printf ("Open file: %s\n", filepath);
        file = fopen(filepath, "wb");
        if (!file)
        {
            printf("Cannot open file: %s\n", filepath);
        }

        ASSERT(file);
        fprintf(file, "%s\n", "<html>");
        fprintf(file, "%s\n", "<head>");
        fprintf(file, "%s\n", "  <meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">");
        fprintf(file, "%s\n", "  <title>TIOVX Framework Overhead Benchmark</title>");
        fprintf(file, "%s\n", "</head>");

        fprintf(file, "<p>TIOVX Framework Overhead Benchmark<p>");
        fprintf(file, "<p>Average values are reported after performing %d iterations on each target. Framework overhead is calculated \
                       by subtracting the execution time of a single node from the total execution time of its graph. Execution \
                       time is measured by querying each graph and node after calling vxProcessGraph.<p>", ITERATIONS);

        fprintf(file, "%s\n", "<body>");
        fprintf(file, "%s\n", "  <table frame=\"box\" rules=\"all\" cellspacing=\"0\" width=\"75%\" border=\"1\" cellpadding=\"3\">");
        fprintf(file, "%s\n", "    <tr bgcolor=\"lightgrey\">");
        fprintf(file, "%s\n", "<td width=\"10%\" align=\"center\"><b>Core</b></td>");
        fprintf(file, "%s\n", "<td width=\"10%\" align=\"center\"><b>Target</b></td>");
        fprintf(file, "%s\n", "<td width=\"20%\" align=\"center\"><b>Average Graph Performance (msec) </b></td>");
        fprintf(file, "%s\n", "<td width=\"20%\" align=\"center\"><b>Average Node Performance (msec) </b></td>");
        fprintf(file, "%s\n", "<td width=\"20%\" align=\"center\"><b>Average Overhead [Graph - Node] (msec) </b></td>");
        fprintf(file, "%s\n", "<td width=\"20%\" align=\"center\"><b>Maximum Overhead (msec) </b></td>");
        fprintf(file, "%s\n", "<td width=\"20%\" align=\"center\"><b>Minimum Overhead (msec) </b></td>");
        fprintf(file, "%s\n", "</tr>");
    }
}

static void printOverhead(float gMax[], float nMax[], const char* targetName, const char* coreName)
{
    float gAvg=0, nAvg=0, oAvg=0, oMin=999999, oMax=0, over;
    uint32_t i;

    for (i = 0; i < ITERATIONS; i++)
    {
        over = gMax[i] - nMax[i];
        gAvg += gMax[i];
        nAvg += nMax[i];
        oAvg += over;

        if (over > oMax)
        {
            oMax = over;
        }
        if  (over < oMin)
        {
            oMin = over;
        }
    }
    gAvg = gAvg / ITERATIONS;
    nAvg = nAvg / ITERATIONS;
    oAvg = oAvg / ITERATIONS;

    printf("\n********************************************\n");
    printf("  TIOVX OVERHEAD PERFORMANCE TEST\n");
    printf("  CORE: %s\n", coreName);
    printf("  TARGET: %s\n", targetName);
    printf("  ITERATIONS: %d\n", ITERATIONS);
    printf("  AVERAGE GRAPH PERFORMANCE (MSEC): %f\n", gAvg);
    printf("  AVERAGE NODE PERFORMANCE (MSEC): %f\n", nAvg);
    printf("  AVERAGE OVERHEAD [GRAPH - NODE] (MSEC): %f\n", oAvg);
    printf("  MAXIMUM OVERHEAD (MSEC): %f\n", oMax);
    printf("  MINIMUM OVERHEAD (MSEC): %f\n", oMin);
    printf("********************************************\n\n");

    if (file == NULL)
    {
        openFile();
    }

    if (file)
    {
        ASSERT(file);

        fprintf(file, "%s\n", " <tr align=\"center\">");
        fprintf(file, "%s%s%s\n", "<td>", coreName, "</td>");
        fprintf(file, "%s%s%s\n", "<td>", targetName, "</td>");
        fprintf(file, "%s%4.6f%s\n", "<td>", gAvg, "</td>");
        fprintf(file, "%s%4.6f%s\n", "<td>", nAvg, "</td>");
        fprintf(file, "%s%4.6f%s\n", "<td>", oAvg, "</td>");
        fprintf(file, "%s%4.6f%s\n", "<td>", oMax, "</td>");
        fprintf(file, "%s%4.6f%s\n", "<td>", oMin, "</td>");
        fprintf(file, "%s\n", " </tr>");
    }
}

TESTCASE(tivxTiovxOverhead,  CT_VXContext, ct_setup_vx_context, 0)

typedef struct
{
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    char *target_string;

} SetTarget_Arg;

#if defined(SOC_AM62A)
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MPU_0", __VA_ARGS__, TIVX_TARGET_MPU_0)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MCU1_0", __VA_ARGS__, TIVX_TARGET_MCU1_0)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP1", __VA_ARGS__, TIVX_TARGET_DSP1))
#define CORE_LIST ((char const*[]){"A53", "R5F", "C7x"})
#elif defined(SOC_J721E)
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MPU_0", __VA_ARGS__, TIVX_TARGET_MPU_0)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MCU2_0", __VA_ARGS__, TIVX_TARGET_MCU2_0)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP1", __VA_ARGS__, TIVX_TARGET_DSP1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP_C7_1", __VA_ARGS__, TIVX_TARGET_DSP_C7_1))
#define CORE_LIST ((char const*[]){"A72", "R5F", "C66", "C7x"})
#else
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MPU_0", __VA_ARGS__, TIVX_TARGET_MPU_0)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MCU2_0", __VA_ARGS__, TIVX_TARGET_MCU2_0)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP1", __VA_ARGS__, TIVX_TARGET_DSP1))
#define CORE_LIST ((char const*[]){"A72", "R5F", "C7x"})
#endif

#define SET_NODE_TARGET_PARAMETERS \
    CT_GENERATE_PARAMETERS("tiovx_overhead", ADD_SET_TARGET_PARAMETERS, ARG, NULL)

TEST_WITH_ARG(tivxTiovxOverhead, testTiovxOverhead, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  in_scalar_val = 73, out_scalar_val=20;
    vx_scalar scalar_in, scalar_out;
    vx_node node;
    vx_perf_t perf_node, perf_graph;
    float gMax[ITERATIONS], nMax[ITERATIONS];
    uint32_t i, idx=0;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_in  = vxCreateScalar(context, VX_TYPE_UINT8, &in_scalar_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(scalar_out  = vxCreateScalar(context, VX_TYPE_UINT8, &out_scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(node = tivxTiovxOverheadNode(graph, scalar_in, scalar_out), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)node, "test_tiovx_overhead");

    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    VX_CALL(vxVerifyGraph(graph));

    for (i = 0; i < ITERATIONS; i++)
    {
        VX_CALL(vxProcessGraph(graph));

        vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
        vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

        if (idx < ITERATIONS)
        {
            gMax[idx] = perf_graph.max/1000000.0;
            nMax[idx] = perf_node.max/1000000.0;
            idx++;
        }
    }

    VX_CALL(vxCopyScalar(scalar_in, &out_scalar_val, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    ASSERT(in_scalar_val == out_scalar_val);

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseScalar(&scalar_in));
    VX_CALL(vxReleaseScalar(&scalar_out));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);

    printOverhead(gMax, nMax, arg_->target_string, CORE_LIST[g_core_counter]);
    g_core_counter++;
}

TEST(tivxTiovxOverhead, testCloseFile)
{
    if (file)
    {
        fprintf(file, "%s\n", "</table>");
        fprintf(file, "%s\n", "</body>");
        fprintf(file, "%s\n", "</html>");
        fclose(file);
        file = NULL;
    }
}

TESTCASE_TESTS(tivxTiovxOverhead,
    testTiovxOverhead,
    testCloseFile)