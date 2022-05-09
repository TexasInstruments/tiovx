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
#include "test_tiovx.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <VX/vx.h>
#include <VX/vxu.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include "shared_functions.h"

#define ENABLE_SUPERNODE   1
#define WIDTH           (640)
#define HEIGHT          (480)
#define DEFAULT_BLOCK_WIDTH  64
#define DEFAULT_BLOCK_HEIGHT 48
#define OUTPUT_FILE     "tiovx_performance.html"
#define SUPERNODE_OFILE "tiovx_sperformance.html"
#define MAX_CONV_SIZE   (15)
#define MAX_PMD_LEVELS  (7)
#define MAX_HISTOGRAMS_BINS        (256)
#ifndef M_PI
#define M_PIF   3.14159265358979323846f
#else
#define M_PIF   (vx_float32)M_PI
#endif

#define ONE_2_0 1.0f
#define ONE_2_0_STR "(1/2^0)"

#define VX_MAP_IDENT         0
#define VX_MAP_SCALE         1
#define VX_MAP_SCALE_ROTATE  2
#define VX_MAP_RANDOM        3

#define RND_FLT(low, high)      (vx_float32)CT_RNG_NEXT_REAL(CT()->seed_, low, high);

#define LEVELS_COUNT_MAX    7

#define MAX_POINTS 100
#define MAX_NODES  16

static FILE *perf_file = NULL;
static uint32_t gTiovxKernIdx;
static FILE *spernode_perf_file = NULL;
static uint32_t sTiovxKernIdx;

TESTCASE(tiovxPerformance, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(tiovxPerformance2, CT_VXContext, ct_setup_vx_context, 0)

#ifdef BUILD_BAM
TESTCASE(tiovxSupernodePerformance, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(tiovxSupernodePerformance2, CT_VXContext, ct_setup_vx_context, 0)
#endif

static void ctSetTileSize(vx_node node)
{
#if !defined(SYSBIOS) && !defined(FREERTOS) && !defined(SAFERTOS)
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxSetNodeTileSize(node,
        (getenv("TIOVX_BLOCK_WIDTH") == NULL) ? DEFAULT_BLOCK_WIDTH : (vx_uint32)atoi(getenv("TIOVX_BLOCK_WIDTH")),
        (getenv("TIOVX_BLOCK_HEIGHT") == NULL) ? DEFAULT_BLOCK_HEIGHT : (vx_uint32)atoi(getenv("TIOVX_BLOCK_HEIGHT"))));
#endif
}

static void openFile()
{
    char filepath[MAXPATHLENGTH];
    size_t sz;

    if (!perf_file)
    {
        sz = snprintf(filepath, MAXPATHLENGTH, "%s/output/%s", ct_get_test_file_path(),
            OUTPUT_FILE);
        ASSERT(sz < MAXPATHLENGTH);

        printf ("Open file: %s\n", filepath);
        perf_file = fopen(filepath, "wb");
        if (!perf_file)
        {
            printf("Cannot open file: %s\n", filepath);
        }
        ASSERT(perf_file);

        fprintf(perf_file, "%s\n", "<html>");
        fprintf(perf_file, "%s\n", "<head>");
        fprintf(perf_file, "%s\n", "  <meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">");
        fprintf(perf_file, "%s\n", "  <title>TIOVX Kernel Performance</title>");
        fprintf(perf_file, "%s\n", "</head>");

        fprintf(perf_file, "%s\n", "<body>");
        fprintf(perf_file, "%s\n", "  <table frame=\"box\" rules=\"all\" cellspacing=\"0\" width=\"50%\" border=\"1\" cellpadding=\"3\">");
        fprintf(perf_file, "%s\n", "    <tr bgcolor=\"lightgrey\">");
        fprintf(perf_file, "%s\n", "<td width=\"5\" align=\"center\"><b>Index</b></td>");
        fprintf(perf_file, "%s\n", "<td width=\"25%\" align=\"center\"><b>Kernel</b></td>");
        fprintf(perf_file, "%s\n", "<td width=\"25%\" align=\"center\"><b>Variant</b></td>");
        fprintf(perf_file, "%s\n", "<td width=\"20%\" align=\"center\"><b>Frame Size (Pixels) </b></td>");
        fprintf(perf_file, "%s\n", "<td width=\"25%\" align=\"center\"><b>Graph Performance (msec) </b></td>");
        fprintf(perf_file, "%s\n", "<td width=\"25%\" align=\"center\"><b>Node Performance (msec) </b></td>");
        fprintf(perf_file, "%s\n", "</tr>");

        gTiovxKernIdx = 0;
    }
}

TEST(tiovxPerformance, tiovxPerfOpenFile)
{
    openFile();
}

TEST(tiovxPerformance2, tiovxPerfCloseFile)
{
    if (perf_file)
    {
        fprintf(perf_file, "%s\n", "</table>");
        fprintf(perf_file, "%s\n", "</body>");
        fprintf(perf_file, "%s\n", "</html>");
        fprintf(perf_file, "<p>Footnote 1: All optional parameters for relevant kernels are used<p>");
        fprintf(perf_file, "<p>Footnote 2: All image sizes are for input parameters unless noted with an asterisk (*) in which case these are the output parameter sizes<p>");
        fclose(perf_file);
        perf_file = NULL;
    }
}

void PrintPerf(vx_perf_t graph, vx_perf_t node, uint32_t width,  uint32_t height, const char* testName, const char* variant, int asterisk)
{
    if(perf_file == NULL)
    {
        openFile();
    }

    if(perf_file)
    {
        gTiovxKernIdx ++;
        fprintf(perf_file, "%s\n", " <tr align=\"center\">");
        fprintf(perf_file, "%s%d%s\n", "<td>", gTiovxKernIdx, "</td>");
        fprintf(perf_file, "%s%s%s\n", "<td>", testName, "</td>");
        fprintf(perf_file, "%s%s%s\n", "<td>", variant, "</td>");
        if (asterisk == 1)
            fprintf(perf_file, "%s*%dx%d  (%d)%s\n", "<td>", width, height, width*height, "</td>");
        else
            fprintf(perf_file, "%s%dx%d  (%d)%s\n", "<td>", width, height, width*height, "</td>");
        fprintf(perf_file, "%s%4.6f%s\n", "<td>", graph.max/1000000.0, "</td>");
        fprintf(perf_file, "%s%4.6f%s\n", "<td>", node.max/1000000.0, "</td>");
        fprintf(perf_file, "%s\n", " </tr>");
    }
}

static void openSupernodeFile()
{
    char filepath[MAXPATHLENGTH];
    size_t sz;

    if (!spernode_perf_file)
    {
        sz = snprintf(filepath, MAXPATHLENGTH, "%s/output/%s", ct_get_test_file_path(),
            SUPERNODE_OFILE);
        ASSERT(sz < MAXPATHLENGTH);

        spernode_perf_file = fopen(filepath, "wb");

        if (!spernode_perf_file)
        {
            printf("Cannot open file: %s\n", filepath);
        }
        ASSERT(spernode_perf_file);
        fprintf(spernode_perf_file, "%s\n", "<html>");
        fprintf(spernode_perf_file, "%s\n", "<head>");
        fprintf(spernode_perf_file, "%s\n", "  <meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">");
        fprintf(spernode_perf_file, "%s\n", "  <title>TIOVX Kernel Performance</title>");
        fprintf(spernode_perf_file, "%s\n", "</head>");

        fprintf(spernode_perf_file, "%s\n", "<body>");
        fprintf(spernode_perf_file, "%s\n", "  <table frame=\"box\" rules=\"all\" cellspacing=\"0\" width=\"50%\" border=\"1\" cellpadding=\"3\">");
        fprintf(spernode_perf_file, "%s\n", "    <tr bgcolor=\"lightgrey\">");
        fprintf(spernode_perf_file, "%s\n", "<td width=\"5\" align=\"center\"><b>Index</b></td>");
        fprintf(spernode_perf_file, "%s\n", "<td width=\"25%\" align=\"center\"><b>Kernel</b></td>");
        fprintf(spernode_perf_file, "%s\n", "<td width=\"25%\" align=\"center\"><b>Variant</b></td>");
        fprintf(spernode_perf_file, "%s\n", "<td width=\"20%\" align=\"center\"><b>Frame Size (Pixels) </b></td>");
        fprintf(spernode_perf_file, "%s\n", "<td width=\"25%\" align=\"center\"><b>Graph Performance (msec) </b></td>");
        if (ENABLE_SUPERNODE)
        {
            fprintf(spernode_perf_file, "%s\n", "<td width=\"25%\" align=\"center\"><b>Supernode Performance (msec) </b></td>");
        }
        else
        {
            fprintf(spernode_perf_file, "%s\n", "<td width=\"25%\" align=\"center\"><b>Node1 Performance (msec) </b></td>");
            fprintf(spernode_perf_file, "%s\n", "<td width=\"25%\" align=\"center\"><b>Node2 Performance (msec) </b></td>");
            fprintf(spernode_perf_file, "%s\n", "<td width=\"25%\" align=\"center\"><b>Node3 Performance (msec) </b></td>");
        }
        fprintf(spernode_perf_file, "%s\n", "</tr>");

        sTiovxKernIdx = 0;
    }
}

#ifdef BUILD_BAM

TEST(tiovxSupernodePerformance, tiovxSupernodePerfOpenFile)
{
    openSupernodeFile();
}

TEST(tiovxSupernodePerformance2, tiovxSupernodePerfCloseFile)
{
    if (spernode_perf_file)
    {
        fprintf(spernode_perf_file, "%s\n", "</table>");
        fprintf(spernode_perf_file, "%s\n", "</body>");
        fprintf(spernode_perf_file, "%s\n", "</html>");
        fprintf(spernode_perf_file, "<p>Footnote 1: All optional parameters for relevant kernels are used<p>");
        fprintf(spernode_perf_file, "<p>Footnote 2: All image sizes are for input parameters unless noted with an asterisk (*) in which case these are the output parameter sizes<p>");
        fclose(spernode_perf_file);
        spernode_perf_file = NULL;
    }
}

void PrintSupernodePerf(vx_perf_t graph, vx_perf_t super_node, vx_perf_t node1, vx_perf_t node2, vx_perf_t node3,
                        uint32_t width,  uint32_t height, const char* testName, const char* variant, int asterisk)
{
    if(spernode_perf_file == NULL)
    {
        openSupernodeFile();
    }

    if(spernode_perf_file)
    {
        sTiovxKernIdx ++;
        fprintf(spernode_perf_file, "%s\n", " <tr align=\"center\">");
        fprintf(spernode_perf_file, "%s%d%s\n", "<td>", sTiovxKernIdx, "</td>");
        fprintf(spernode_perf_file, "%s%s%s\n", "<td>", testName, "</td>");
        fprintf(spernode_perf_file, "%s%s%s\n", "<td>", variant, "</td>");
        if (asterisk == 1)
            fprintf(spernode_perf_file, "%s*%dx%d  (%d)%s\n", "<td>", width, height, width*height, "</td>");
        else
            fprintf(spernode_perf_file, "%s%dx%d  (%d)%s\n", "<td>", width, height, width*height, "</td>");
        fprintf(spernode_perf_file, "%s%4.6f%s\n", "<td>", graph.max/1000000.0, "</td>");
        if (ENABLE_SUPERNODE) fprintf(spernode_perf_file, "%s%4.6f%s\n", "<td>", super_node.max/1000000.0, "</td>");
        if (!ENABLE_SUPERNODE) fprintf(spernode_perf_file, "%s%4.6f%s\n", "<td>", node1.max/1000000.0, "</td>");
        if (!ENABLE_SUPERNODE) fprintf(spernode_perf_file, "%s%4.6f%s\n", "<td>", node2.max/1000000.0, "</td>");
        if (!ENABLE_SUPERNODE) fprintf(spernode_perf_file, "%s%4.6f%s\n", "<td>", node3.max/1000000.0, "</td>");
        fprintf(spernode_perf_file, "%s\n", " </tr>");
    }
}

#endif

TEST(tiovxPerformance, tiovxPerfAccumulate)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, accum = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(input = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(input, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(accum, &CT()->seed_));

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxAccumulateImageNode(graph, input, accum), VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&accum));
    VX_CALL(vxReleaseImage(&input));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(accum == 0);
    ASSERT(input == 0);

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Accumulate", "", 0);
}

TEST(tiovxPerformance, tiovxPerfAccumulateSquare)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, accum = 0;
    vx_uint32 shift = 8;
    vx_scalar shift_scalar = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(input = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(shift_scalar = vxCreateScalar(context, VX_TYPE_UINT32, &shift), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(ct_fill_image_random(input, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(accum, &CT()->seed_));

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxAccumulateSquareImageNode(graph, input, shift_scalar, accum), VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&accum));
    VX_CALL(vxReleaseImage(&input));
    VX_CALL(vxReleaseScalar(&shift_scalar));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(accum == 0);
    ASSERT(input == 0);

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "AccumulateSquare", "", 0);
}

TEST(tiovxPerformance, tiovxPerfAccumulateWeighted)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, accum = 0;
    vx_float32 alpha = 0.5f;
    vx_scalar alpha_scalar = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(input = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(alpha_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &alpha), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(ct_fill_image_random(input, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(accum, &CT()->seed_));

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxAccumulateWeightedImageNode(graph, input, alpha_scalar, accum), VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&accum));
    VX_CALL(vxReleaseImage(&input));
    VX_CALL(vxReleaseScalar(&alpha_scalar));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(accum == 0);
    ASSERT(input == 0);
    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "AccumulateWeighted", "", 0);
}

TEST(tiovxPerformance, tiovxPerfAdd888)
{
    vx_context context = context_->vx_context_;
    vx_image input1 = 0, input2 = 0, output;
    vx_float32 alpha = 0.5f;
    vx_scalar alpha_scalar = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(input1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(input1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(input2, &CT()->seed_));

    ASSERT_VX_OBJECT(output = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxAddNode(graph, input1, input2, VX_ENUM_OVERFLOW, output), VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&input2));
    VX_CALL(vxReleaseImage(&input1));
    VX_CALL(vxReleaseImage(&output));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(input1 == 0);
    ASSERT(input2 == 0);
    ASSERT(output == 0);
    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Addition", "U8+U8=U8", 0);
}

TEST(tiovxPerformance, tiovxPerfAdd8816)
{
    vx_context context = context_->vx_context_;
    vx_image input1 = 0, input2 = 0, output;
    vx_float32 alpha = 0.5f;
    vx_scalar alpha_scalar = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(input1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(input1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(input2, &CT()->seed_));

    ASSERT_VX_OBJECT(output = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxAddNode(graph, input1, input2, VX_ENUM_OVERFLOW, output), VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&input2));
    VX_CALL(vxReleaseImage(&input1));
    VX_CALL(vxReleaseImage(&output));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(input1 == 0);
    ASSERT(input2 == 0);
    ASSERT(output == 0);
    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Addition", "U8+U8=S16", 0);
}

TEST(tiovxPerformance, tiovxPerfAdd81616)
{
    vx_context context = context_->vx_context_;
    vx_image input1 = 0, input2 = 0, output;
    vx_float32 alpha = 0.5f;
    vx_scalar alpha_scalar = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(input1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(input1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(input2, &CT()->seed_));

    ASSERT_VX_OBJECT(output = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxAddNode(graph, input1, input2, VX_ENUM_OVERFLOW, output), VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&input2));
    VX_CALL(vxReleaseImage(&input1));
    VX_CALL(vxReleaseImage(&output));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(input1 == 0);
    ASSERT(input2 == 0);
    ASSERT(output == 0);
    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Addition", "U8+S16=S16", 0);
}

TEST(tiovxPerformance, tiovxPerfAdd161616)
{
    vx_context context = context_->vx_context_;
    vx_image input1 = 0, input2 = 0, output;
    vx_float32 alpha = 0.5f;
    vx_scalar alpha_scalar = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(input1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(input1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(input2, &CT()->seed_));

    ASSERT_VX_OBJECT(output = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxAddNode(graph, input1, input2, VX_ENUM_OVERFLOW, output), VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&input2));
    VX_CALL(vxReleaseImage(&input1));
    VX_CALL(vxReleaseImage(&output));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(input1 == 0);
    ASSERT(input2 == 0);
    ASSERT(output == 0);
    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Addition", "S16+S16=S16", 0);
}

TEST(tiovxPerformance, tiovxPerfSubtract888)
{
    vx_context context = context_->vx_context_;
    vx_image input1 = 0, input2 = 0, output;
    vx_float32 alpha = 0.5f;
    vx_scalar alpha_scalar = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(input1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(input1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(input2, &CT()->seed_));

    ASSERT_VX_OBJECT(output = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxSubtractNode(graph, input1, input2, VX_ENUM_OVERFLOW, output), VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&input2));
    VX_CALL(vxReleaseImage(&input1));
    VX_CALL(vxReleaseImage(&output));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(input1 == 0);
    ASSERT(input2 == 0);
    ASSERT(output == 0);
    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Subtraction", "U8-U8=U8", 0);
}

TEST(tiovxPerformance, tiovxPerfSubtract8816)
{
    vx_context context = context_->vx_context_;
    vx_image input1 = 0, input2 = 0, output;
    vx_float32 alpha = 0.5f;
    vx_scalar alpha_scalar = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(input1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(input1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(input2, &CT()->seed_));

    ASSERT_VX_OBJECT(output = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxSubtractNode(graph, input1, input2, VX_ENUM_OVERFLOW, output), VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&input2));
    VX_CALL(vxReleaseImage(&input1));
    VX_CALL(vxReleaseImage(&output));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(input1 == 0);
    ASSERT(input2 == 0);
    ASSERT(output == 0);
    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Subtraction", "U8-U8=S16", 0);
}

TEST(tiovxPerformance, tiovxPerfSubtract81616)
{
    vx_context context = context_->vx_context_;
    vx_image input1 = 0, input2 = 0, output;
    vx_float32 alpha = 0.5f;
    vx_scalar alpha_scalar = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(input1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(input1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(input2, &CT()->seed_));

    ASSERT_VX_OBJECT(output = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxSubtractNode(graph, input1, input2, VX_ENUM_OVERFLOW, output), VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&input2));
    VX_CALL(vxReleaseImage(&input1));
    VX_CALL(vxReleaseImage(&output));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(input1 == 0);
    ASSERT(input2 == 0);
    ASSERT(output == 0);
    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Subtraction", "S16-U8=S16", 0);
}

TEST(tiovxPerformance, tiovxPerfSubtract161616)
{
    vx_context context = context_->vx_context_;
    vx_image input1 = 0, input2 = 0, output;
    vx_float32 alpha = 0.5f;
    vx_scalar alpha_scalar = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(input1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(input1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(input2, &CT()->seed_));

    ASSERT_VX_OBJECT(output = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxSubtractNode(graph, input1, input2, VX_ENUM_OVERFLOW, output), VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&input2));
    VX_CALL(vxReleaseImage(&input1));
    VX_CALL(vxReleaseImage(&output));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(input1 == 0);
    ASSERT(input2 == 0);
    ASSERT(output == 0);
    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Subtraction", "S16-S16=S16", 0);
}

TEST(tiovxPerformance, tiovxPerfNot)
{
    vx_image src, dst;
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_perf_t perf_node, perf_graph;
    vx_node node;

    ASSERT_VX_OBJECT(src = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src, &CT()->seed_));

    // build one-node graph
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node = vxNotNode(graph, src, dst), VX_TYPE_NODE);

    ctSetTileSize(node);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseGraph(&graph));

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Not Operation", "U8", 0);
}

typedef vx_node   (VX_API_CALL *vxBinopFunction) (vx_graph, vx_image, vx_image, vx_image);

TEST(tiovxPerformance, tiovxPerfBinOp8)
{
    vx_image src1, src2, dst;
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node node;
    vxBinopFunction vxFunc[4] = {vxAndNode, vxOrNode, vxXorNode, vxAbsDiffNode};
    char function[4][20] = {"And Operation", "OR Operation",
        "XOR Operation", "Abs Diff"};
    vx_perf_t perf_node, perf_graph;
    vx_uint32 cnt;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    for (cnt = 0; cnt < 4; cnt ++)
    {
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(dst   = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(src1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
        ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));

        ASSERT_VX_OBJECT(node = vxFunc[cnt](graph, src1, src2, dst), VX_TYPE_NODE);

        ctSetTileSize(node);

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
        vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseImage(&src1));
        VX_CALL(vxReleaseImage(&src2));
        VX_CALL(vxReleaseImage(&dst));
        VX_CALL(vxReleaseGraph(&graph));

        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, function[cnt], "U8", 0);
    }
}

TEST(tiovxPerformance, tiovxPerfBinOp16)
{
    vx_image src1, src2, dst;
    vx_graph graph;
    vx_node node;
    CT_Image ref1, ref2, refdst, vxdst;
    vx_context context = context_->vx_context_;

    vx_perf_t perf_node, perf_graph;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));

    // build one-node graph
    ASSERT_VX_OBJECT(node = vxAbsDiffNode(graph, src1, src2, dst), VX_TYPE_NODE);

    ctSetTileSize(node);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseGraph(&graph));

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Abs Diff", "S16", 0);
}


TEST(tiovxPerformance, tiovxPerfBox3x3)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border;

    border.mode = VX_BORDER_UNDEFINED;
    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src_image, &CT()->seed_));

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxBox3x3Node(graph, src_image, dst_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Box", "3x3", 0);
}

typedef struct {
    const char* name;
    const char* filename;
    int32_t grad_size;
    vx_enum norm_type;
    int32_t low_thresh;
    int32_t high_thresh;
} canny_arg;

#define CANNY_ARG(grad, norm, lo, hi, file) ARG(#file "/" #norm " " #grad "x" #grad " thresh=(" #lo ", " #hi ")", #file ".bmp", grad, VX_NORM_##norm, lo, hi)

TEST_WITH_ARG(tiovxPerformance, tiovxPerfCanny, canny_arg,
    CANNY_ARG(3, L1, 100, 120, lena_gray)
)
{
    uint32_t total, count;
    vx_image src, dst;
    vx_threshold hyst;
    vx_graph graph;
    vx_node node;
    CT_Image lena, vxdst, refdst, dist;
    vx_int32 low_thresh  = arg_->low_thresh;
    vx_int32 high_thresh = arg_->high_thresh;
    vx_border_t border = { VX_BORDER_UNDEFINED, {{ 0 }} };
    vx_int32 border_width = arg_->grad_size/2 + 1;
    vx_context context = context_->vx_context_;
    vx_enum thresh_data_type = VX_TYPE_UINT8;
    vx_perf_t perf_node, perf_graph;

    if (low_thresh > 255)
        thresh_data_type = VX_TYPE_INT16;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(lena = ct_read_image(arg_->filename, 1));
    ASSERT_NO_FAILURE(src = ct_image_to_vx_image(lena, context));
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, lena->width, lena->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(hyst = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, thresh_data_type), VX_TYPE_THRESHOLD);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_LOWER, &low_thresh,  sizeof(low_thresh)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_UPPER, &high_thresh, sizeof(high_thresh)));
    /* FALSE_VALUE and TRUE_VALUE of hyst parameter are set to their default values (0, 255) by vxCreateThreshold */
    /* test reference data are computed with assumption that FALSE_VALUE and TRUE_VALUE set to 0 and 255 */

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = vxCannyEdgeDetectorNode(graph, src, hyst, arg_->grad_size, arg_->norm_type, dst), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseNode(&node));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseGraph(&graph));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseThreshold(&hyst));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&src));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&dst));

    PrintPerf(perf_graph, perf_node, lena->width, lena->height, "Canny", "", 0);
}

static CT_Image image_generate_random(int width, int height, vx_df_image format)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, format, &CT()->seed_, 0, 256));

    return image;
}

TEST(tiovxPerformance, tiovxPerfChannelCombineRGB)
{
    vx_context context = context_->vx_context_;
    vx_image src_image[4] = {0, 0, 0, 0};
    vx_image dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    int channels = 0, i;
    CT_Image src[4] = {NULL, NULL, NULL, NULL};
    CT_Image dst = NULL, dst_dummy = NULL;
    vx_enum channel_ref;

    ASSERT_NO_FAILURE(dst_dummy = ct_allocate_image(4, 4, VX_DF_IMAGE_RGB));

    ASSERT_NO_FAILURE(channels = ct_get_num_channels(VX_DF_IMAGE_RGB));
    channel_ref = VX_CHANNEL_R;
    for (i = 0; i < channels; i++)
    {
        int w = WIDTH / ct_image_get_channel_subsampling_x(dst_dummy, channel_ref + i);
        int h = HEIGHT / ct_image_get_channel_subsampling_y(dst_dummy, channel_ref + i);
        ASSERT_NO_FAILURE(src[i] = image_generate_random(w, h, VX_DF_IMAGE_U8));
        ASSERT_VX_OBJECT(src_image[i] = ct_image_to_vx_image(src[i], context), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxChannelCombineNode(graph, src_image[0], src_image[1], src_image[2], src_image[3], dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    ASSERT(dst_image == 0);

    for (i = 0; i < channels; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
        ASSERT(src_image[i] == 0);
    }

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Channel Combine", "3 to 1 (RGB)", 0);
}

TEST(tiovxPerformance, tiovxPerfChannelCombineRGBX)
{
    vx_context context = context_->vx_context_;
    vx_image src_image[4] = {0, 0, 0, 0};
    vx_image dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    int channels = 0, i;
    CT_Image src[4] = {NULL, NULL, NULL, NULL};
    CT_Image dst = NULL, dst_dummy = NULL;
    vx_enum channel_ref;

    ASSERT_NO_FAILURE(dst_dummy = ct_allocate_image(4, 4, VX_DF_IMAGE_RGBX));

    ASSERT_NO_FAILURE(channels = ct_get_num_channels(VX_DF_IMAGE_RGBX));
    channel_ref = VX_CHANNEL_R;
    for (i = 0; i < channels; i++)
    {
        int w = WIDTH / ct_image_get_channel_subsampling_x(dst_dummy, channel_ref + i);
        int h = HEIGHT / ct_image_get_channel_subsampling_y(dst_dummy, channel_ref + i);
        ASSERT_NO_FAILURE(src[i] = image_generate_random(w, h, VX_DF_IMAGE_U8));
        ASSERT_VX_OBJECT(src_image[i] = ct_image_to_vx_image(src[i], context), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_RGBX), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxChannelCombineNode(graph, src_image[0], src_image[1], src_image[2], src_image[3], dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    ASSERT(dst_image == 0);

    for (i = 0; i < channels; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
        ASSERT(src_image[i] == 0);
    }

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Channel Combine", "4 to 1 (RGBX)", 0);
}

TEST(tiovxPerformance, tiovxPerfChannelCombineYUYV)
{
    vx_context context = context_->vx_context_;
    vx_image src_image[4] = {0, 0, 0, 0};
    vx_image dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    int channels = 0, i;
    CT_Image src[4] = {NULL, NULL, NULL, NULL};
    CT_Image dst = NULL, dst_dummy = NULL;
    vx_enum channel_ref;

    ASSERT_NO_FAILURE(dst_dummy = ct_allocate_image(4, 4, VX_DF_IMAGE_YUYV));

    ASSERT_NO_FAILURE(channels = ct_get_num_channels(VX_DF_IMAGE_YUYV));
    channel_ref = VX_CHANNEL_Y;
    for (i = 0; i < channels; i++)
    {
        int w = WIDTH / ct_image_get_channel_subsampling_x(dst_dummy, channel_ref + i);
        int h = HEIGHT / ct_image_get_channel_subsampling_y(dst_dummy, channel_ref + i);
        ASSERT_NO_FAILURE(src[i] = image_generate_random(w, h, VX_DF_IMAGE_U8));
        ASSERT_VX_OBJECT(src_image[i] = ct_image_to_vx_image(src[i], context), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxChannelCombineNode(graph, src_image[0], src_image[1], src_image[2], src_image[3], dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    ASSERT(dst_image == 0);

    for (i = 0; i < channels; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
        ASSERT(src_image[i] == 0);
    }

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Channel Combine", "YUYV", 0);
}

TEST(tiovxPerformance, tiovxPerfChannelCombineNV12)
{
    vx_context context = context_->vx_context_;
    vx_image src_image[4] = {0, 0, 0, 0};
    vx_image dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    int channels = 0, i;
    CT_Image src[4] = {NULL, NULL, NULL, NULL};
    CT_Image dst = NULL, dst_dummy = NULL;
    vx_enum channel_ref;

    ASSERT_NO_FAILURE(dst_dummy = ct_allocate_image(4, 4, VX_DF_IMAGE_NV12));

    ASSERT_NO_FAILURE(channels = ct_get_num_channels(VX_DF_IMAGE_NV12));
    channel_ref = VX_CHANNEL_Y;
    for (i = 0; i < channels; i++)
    {
        int w = WIDTH / ct_image_get_channel_subsampling_x(dst_dummy, channel_ref + i);
        int h = HEIGHT / ct_image_get_channel_subsampling_y(dst_dummy, channel_ref + i);
        ASSERT_NO_FAILURE(src[i] = image_generate_random(w, h, VX_DF_IMAGE_U8));
        ASSERT_VX_OBJECT(src_image[i] = ct_image_to_vx_image(src[i], context), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxChannelCombineNode(graph, src_image[0], src_image[1], src_image[2], src_image[3], dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    ASSERT(dst_image == 0);

    for (i = 0; i < channels; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
        ASSERT(src_image[i] == 0);
    }

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Channel Combine", "2 to 1 (NV12)", 0);
}

TEST(tiovxPerformance, tiovxPerfChannelExtractRGB)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    CT_Image src = NULL, dst = NULL;

    ASSERT_NO_FAILURE(src = image_generate_random(WIDTH, HEIGHT, VX_DF_IMAGE_RGB));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxChannelExtractNode(graph, src_image, VX_CHANNEL_R, dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Channel Extract", "1 of 3 (RGB)", 0);
}

TEST(tiovxPerformance, tiovxPerfChannelExtractRGBX)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    CT_Image src = NULL, dst = NULL;

    ASSERT_NO_FAILURE(src = image_generate_random(WIDTH, HEIGHT, VX_DF_IMAGE_RGBX));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxChannelExtractNode(graph, src_image, VX_CHANNEL_R, dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Channel Extract", "1 of 4 (RGBX)", 0);
}

TEST(tiovxPerformance, tiovxPerfChannelExtractNV12)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    CT_Image src = NULL, dst = NULL;

    ASSERT_NO_FAILURE(src = image_generate_random(WIDTH, HEIGHT, VX_DF_IMAGE_NV12));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, WIDTH/2, HEIGHT/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxChannelExtractNode(graph, src_image, VX_CHANNEL_U, dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Channel Extract", "1 of 2 (NV12)", 0);
}

typedef struct {
    const char* name;
    vx_df_image srcformat;
    vx_df_image dstformat;
    int mode;
    int ythresh;
    int cthresh;
} color_arg;

#define CVT_CASE_(imm, from, to, ythresh, cthresh) \
    {#imm "/" #from "=>" #to, VX_DF_IMAGE_##from, VX_DF_IMAGE_##to, CT_##imm##_MODE, ythresh, cthresh}

#define CVT_CASE(from, to, ythresh, cthresh) \
    CVT_CASE_(Graph, from, to, ythresh, cthresh)

TEST_WITH_ARG(tiovxPerformance, tiovxPerfColorConvert, color_arg,
              CVT_CASE(RGB, RGBX, 0, 0),
              CVT_CASE(RGB, NV12, 1, 1),
              CVT_CASE(RGB, IYUV, 1, 1),
              CVT_CASE(RGB, YUV4, 1, 1),

              CVT_CASE(RGBX, RGB, 0, 0),
              CVT_CASE(RGBX, NV12, 1, 1),
              CVT_CASE(RGBX, IYUV, 1, 1),
              CVT_CASE(RGBX, YUV4, 1, 1),

              CVT_CASE(NV12, RGB, 1, 1),
              CVT_CASE(NV12, RGBX, 1, 1),
              CVT_CASE(NV12, IYUV, 0, 0),
              CVT_CASE(NV12, YUV4, 0, 0),

              CVT_CASE(YUYV, RGB, 1, 1),
              CVT_CASE(YUYV, RGBX, 1, 1),
              CVT_CASE(YUYV, NV12, 0, 0),
              CVT_CASE(YUYV, IYUV, 0, 0),

              CVT_CASE(IYUV, RGB, 1, 1),
              CVT_CASE(IYUV, RGBX, 1, 1),
              CVT_CASE(IYUV, NV12, 0, 0),
              CVT_CASE(IYUV, YUV4, 0, 0),
              )
{
    int srcformat = arg_->srcformat;
    int dstformat = arg_->dstformat;
    int ythresh = arg_->ythresh;
    int cthresh = arg_->cthresh;
    int mode = arg_->mode;
    vx_image src=0, dst=0;
    CT_Image src0, dst0, dst1;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_context context = context_->vx_context_;
    uint64_t rng;
    vx_perf_t perf_node, perf_graph;

    rng = CT()->seed_;

    int width = ct_roundf(ct_log_rng(&rng, 0, 10));
    int height = ct_roundf(ct_log_rng(&rng, 0, 10));
    vx_enum range = VX_CHANNEL_RANGE_FULL;
    vx_enum space = VX_COLOR_SPACE_BT709;

    width = WIDTH;
    height = HEIGHT;

    if( srcformat == VX_DF_IMAGE_RGB || srcformat == VX_DF_IMAGE_RGBX )
    {
        int scn = srcformat == VX_DF_IMAGE_RGB ? 3 : 4;
        ASSERT_NO_FAILURE(src0 = ct_allocate_ct_image_random(width, height, srcformat, &rng, 0, 256));
    }
    else
    {
        ASSERT_NO_FAILURE(src0 = ct_allocate_ct_image_random(width, height, srcformat, &rng, 0, 256));
    }
    ASSERT_NO_FAILURE(src = ct_image_to_vx_image(src0, context));
    ASSERT_VX_OBJECT(src, VX_TYPE_IMAGE);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(src, VX_IMAGE_SPACE, &space, sizeof(space)));

    ASSERT_NO_FAILURE(dst0 = ct_allocate_image(width, height, dstformat));
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, width, height, dstformat), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst, VX_TYPE_IMAGE);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(dst, VX_IMAGE_SPACE, &space, sizeof(space)));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = vxColorConvertNode(graph, src, dst), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    dst1 = ct_image_from_vx_image(dst);

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0 && graph == 0);
    CT_CollectGarbage(CT_GC_IMAGE);

    if ( (srcformat == VX_DF_IMAGE_RGB) && (dstformat == VX_DF_IMAGE_RGBX) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "RGB=>RGBX", 0);
    else if ( (srcformat == VX_DF_IMAGE_RGB) && (dstformat == VX_DF_IMAGE_NV12) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "RGB=>NV12", 0);
    else if ( (srcformat == VX_DF_IMAGE_RGB) && (dstformat == VX_DF_IMAGE_IYUV) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "RGB=>IYUV", 0);
    else if ( (srcformat == VX_DF_IMAGE_RGB) && (dstformat == VX_DF_IMAGE_YUV4) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "RGB=>YUV4", 0);
    else if ( (srcformat == VX_DF_IMAGE_RGBX) && (dstformat == VX_DF_IMAGE_RGB) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "RGBX=>RGB", 0);
    else if ( (srcformat == VX_DF_IMAGE_RGBX) && (dstformat == VX_DF_IMAGE_NV12) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "RGBX=>NV12", 0);
    else if ( (srcformat == VX_DF_IMAGE_RGBX) && (dstformat == VX_DF_IMAGE_IYUV) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "RGBX=>IYUV", 0);
    else if ( (srcformat == VX_DF_IMAGE_RGBX) && (dstformat == VX_DF_IMAGE_YUV4) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "RGBX=>YUV4", 0);
    else if ( (srcformat == VX_DF_IMAGE_NV12) && (dstformat == VX_DF_IMAGE_RGB) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "NVXX=>RGB", 0);
    else if ( (srcformat == VX_DF_IMAGE_NV12) && (dstformat == VX_DF_IMAGE_RGBX) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "NVXX=>RGBX", 0);
    else if ( (srcformat == VX_DF_IMAGE_NV12) && (dstformat == VX_DF_IMAGE_IYUV) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "NVXX=>IYUV", 0);
    else if ( (srcformat == VX_DF_IMAGE_NV12) && (dstformat == VX_DF_IMAGE_YUV4) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "NVXX=>YUV4", 0);
    else if ( (srcformat == VX_DF_IMAGE_YUYV) && (dstformat == VX_DF_IMAGE_RGB) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "YUVX=>RGB", 0);
    else if ( (srcformat == VX_DF_IMAGE_YUYV) && (dstformat == VX_DF_IMAGE_RGBX) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "YUVX=>RGBX", 0);
    else if ( (srcformat == VX_DF_IMAGE_YUYV) && (dstformat == VX_DF_IMAGE_IYUV) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "YUVX=>IYUV", 0);
    else if ( (srcformat == VX_DF_IMAGE_YUYV) && (dstformat == VX_DF_IMAGE_NV12) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "YUVX=>NV12", 0);
    else if ( (srcformat == VX_DF_IMAGE_IYUV) && (dstformat == VX_DF_IMAGE_RGB) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "IYUV=>RGB", 0);
    else if ( (srcformat == VX_DF_IMAGE_IYUV) && (dstformat == VX_DF_IMAGE_RGBX) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "IYUV=>RGBX", 0);
    else if ( (srcformat == VX_DF_IMAGE_IYUV) && (dstformat == VX_DF_IMAGE_NV12) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "IYUV=>NV12", 0);
    else if ( (srcformat == VX_DF_IMAGE_IYUV) && (dstformat == VX_DF_IMAGE_YUV4) )
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Color Convert", "IYUV=>YUV4", 0);
}

typedef struct {
    const char* testName;
    vx_df_image input_format;
    vx_df_image output_format;
} convert_depth_arg;

#define CONVERT_DEPTH_FUZZY_ARGS_(name, input_format, output_format)  \
    ARG(#name "/ConvertDepth(" #input_format "->" #output_format ")",   \
         VX_DF_IMAGE_##input_format, VX_DF_IMAGE_##output_format)

#define CONVERT_DEPTH_FUZZY_ARGS(input_format, output_format)   \
    CONVERT_DEPTH_FUZZY_ARGS_(Graph, input_format, output_format)


TEST_WITH_ARG(tiovxPerformance, tiovxPerfConvertDepth, convert_depth_arg,
              CONVERT_DEPTH_FUZZY_ARGS(U8, S16),
              CONVERT_DEPTH_FUZZY_ARGS(S16, U8))
{
    vx_image src, dst;
    CT_Image ref_src, refdst, vxdst;
    vx_graph graph;
    vx_node node;
    vx_scalar scalar_shift;
    vx_int32 shift = 0;
    vx_int32 tmp = 0;
    vx_context context = context_->vx_context_;
    vx_perf_t perf_node, perf_graph;

    ASSERT_NO_FAILURE(ref_src = image_generate_random(WIDTH, HEIGHT, arg_->input_format));
    src = ct_image_to_vx_image(ref_src, context);

    ASSERT_VX_OBJECT(dst = vxCreateImage(context, WIDTH, HEIGHT, arg_->output_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(scalar_shift = vxCreateScalar(context, VX_TYPE_INT32, &tmp), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = vxConvertDepthNode(graph, src, dst, VX_CONVERT_POLICY_SATURATE, scalar_shift), VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseScalar(&scalar_shift));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    if (arg_->input_format == VX_DF_IMAGE_U8)
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Convert Depth", "U8 to S16", 0);
    else
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Convert Depth", "S16 to U8", 0);
}

static vx_convolution convolution_create(vx_context context, int cols, int rows, vx_int16* data, vx_uint32 scale)
{
    vx_convolution convolution = vxCreateConvolution(context, cols, rows);
    vx_size size = 0;

    ASSERT_VX_OBJECT_(return 0, convolution, VX_TYPE_CONVOLUTION);

    VX_CALL_(return 0, vxQueryConvolution(convolution, VX_CONVOLUTION_SIZE, &size, sizeof(size)));

    VX_CALL_(return 0, vxCopyConvolutionCoefficients(convolution, data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL_(return 0, vxSetConvolutionAttribute(convolution, VX_CONVOLUTION_SCALE, &scale, sizeof(scale)));

    return convolution;
}

static CT_Image convolve_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static CT_Image read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
    return image;
}

static void convolution_data_fill_identity(int cols, int rows, vx_int16* data)
{
    int x = cols / 2, y = rows / 2;
    ct_memset(data, 0, sizeof(vx_int16) * cols * rows);
    data[y * cols + x] = 1;
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int cols, rows;
    vx_uint32 scale;
    void (*convolution_data_generator)(int cols, int rows, vx_int16* data);
    vx_df_image dst_format;
    vx_border_t border;
    int width, height;
} Convolve_Arg;

#define ADD_CONV_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv=3x3", __VA_ARGS__, 3, 3)), \
    CT_EXPAND(nextmacro(testArgName "/conv=9x3", __VA_ARGS__, 9, 3)), \
    CT_EXPAND(nextmacro(testArgName "/conv=3x9", __VA_ARGS__, 3, 9)), \
    CT_EXPAND(nextmacro(testArgName "/conv=5x5", __VA_ARGS__, 5, 5)), \
    CT_EXPAND(nextmacro(testArgName "/conv=7x7", __VA_ARGS__, 7, 7)), \
    CT_EXPAND(nextmacro(testArgName "/conv=9x9", __VA_ARGS__, 9, 9))
#define ADD_CONV_SCALE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv_scale=1", __VA_ARGS__, 1))
#define ADD_CONV_GENERATORS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv_fill=identity", __VA_ARGS__, convolution_data_fill_identity))
#define ADD_CONV_DST_FORMAT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dst8U", __VA_ARGS__, VX_DF_IMAGE_U8)), \
    CT_EXPAND(nextmacro(testArgName "/dst16S", __VA_ARGS__, VX_DF_IMAGE_S16))

TEST_WITH_ARG(tiovxPerformance2, tiovxPerfConvolve, Convolve_Arg,
    CT_GENERATE_PARAMETERS("lena", ADD_CONV_SIZE, ADD_CONV_SCALE, ADD_CONV_GENERATORS, ADD_CONV_DST_FORMAT, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ARG, read_image, "lena.bmp")
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_convolution convolution = 0;
    vx_int16 data[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 };
    vx_size conv_max_dim = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, src->width, src->height, arg_->dst_format), VX_TYPE_IMAGE);

    VX_CALL(vxQueryContext(context, VX_CONTEXT_CONVOLUTION_MAX_DIMENSION, &conv_max_dim, sizeof(conv_max_dim)));

    if ((vx_size)arg_->cols > conv_max_dim || (vx_size)arg_->rows > conv_max_dim)
    {
        printf("%dx%d convolution is not supported. Skip test\n", (int)arg_->cols, (int)arg_->rows);
        return;
    }

    ASSERT_NO_FAILURE(arg_->convolution_data_generator(arg_->cols, arg_->rows, data));
    ASSERT_NO_FAILURE(convolution = convolution_create(context, arg_->cols, arg_->rows, data, arg_->scale));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxConvolveNode(graph, src_image, convolution, dst_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    VX_CALL(vxReleaseConvolution(&convolution));
    ASSERT(convolution == NULL);

    if ( (arg_->dst_format == VX_DF_IMAGE_U8) && (arg_->cols == 3) && (arg_->rows == 3) )
        PrintPerf(perf_graph, perf_node, src->width, src->height, "Convolution", "3x3, O: U8", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_S16) && (arg_->cols == 3) && (arg_->rows == 3) )
        PrintPerf(perf_graph, perf_node, src->width, src->height, "Convolution", "3x3, O: S16", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_U8) && (arg_->cols == 5) && (arg_->rows == 5) )
        PrintPerf(perf_graph, perf_node, src->width, src->height, "Convolution", "5x5, O: U8", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_S16) && (arg_->cols == 5) && (arg_->rows == 5) )
        PrintPerf(perf_graph, perf_node, src->width, src->height, "Convolution", "5x5, O: S16", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_U8) && (arg_->cols == 7) && (arg_->rows == 7) )
        PrintPerf(perf_graph, perf_node, src->width, src->height, "Convolution", "7x7, O: U8", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_S16) && (arg_->cols == 7) && (arg_->rows == 7) )
        PrintPerf(perf_graph, perf_node, src->width, src->height, "Convolution", "7x7, O: S16", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_U8) && (arg_->cols == 9) && (arg_->rows == 9) )
        PrintPerf(perf_graph, perf_node, src->width, src->height, "Convolution", "9x9, O: U8", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_S16) && (arg_->cols == 9) && (arg_->rows == 9) )
        PrintPerf(perf_graph, perf_node, src->width, src->height, "Convolution", "9x9, O: S16", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_U8) && (arg_->cols == 9) && (arg_->rows == 3) )
        PrintPerf(perf_graph, perf_node, src->width, src->height, "Convolution", "9x3, O: U8", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_S16) && (arg_->cols == 9) && (arg_->rows == 3) )
        PrintPerf(perf_graph, perf_node, src->width, src->height, "Convolution", "9x3, O: S16", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_U8) && (arg_->cols == 3) && (arg_->rows == 9) )
        PrintPerf(perf_graph, perf_node, src->width, src->height, "Convolution", "3x9, O: U8", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_S16) && (arg_->cols == 3) && (arg_->rows == 9) )
        PrintPerf(perf_graph, perf_node, src->width, src->height, "Convolution", "3x9, O: S16", 0);
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    vx_border_t border;
    int width, height;
} Filter3x3_Arg;

TEST_WITH_ARG(tiovxPerformance, tiovxPerfDilate3x3, Filter3x3_Arg,
    CT_GENERATE_PARAMETERS("lena", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ARG, read_image, "lena.bmp")
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = ct_create_similar_image(src_image), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxDilate3x3Node(graph, src_image, dst_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    PrintPerf(perf_graph, perf_node, src->width, src->height, "Dilate", "3x3", 0);
}

TEST(tiovxPerformance2, tiovxPerfEqualizeHistogram)
{
    vx_image src, dst;
    vx_node node = 0;
    vx_graph graph = 0;
    vx_context context = context_->vx_context_;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(src = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src, &CT()->seed_));

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxEqualizeHistNode(graph, src, dst);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0 && graph == 0);

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Equalize Histogram", "", 0);
}

TEST_WITH_ARG(tiovxPerformance, tiovxPerfErode3x3, Filter3x3_Arg,
    CT_GENERATE_PARAMETERS("lena", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ARG, read_image, "lena.bmp")
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = ct_create_similar_image(src_image), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxErode3x3Node(graph, src_image, dst_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    PrintPerf(perf_graph, perf_node, src->width, src->height, "Erode", "3x3", 0);
}

typedef struct {
    const char* name;
    const char* imgname;
    int threshold;
    int nonmax;
    int mode;
} fast_arg;

#define MAX_BINS 256

#define FAST_TEST_CASE(imm, imgname, t, nm) \
    {#imm "/" "image=" #imgname "/" "threshold=" #t "/" "nonmax_suppression=" #nm, #imgname ".bmp", t, nm, CT_##imm##_MODE}

TEST_WITH_ARG(tiovxPerformance, tiovxPerfFastCorners, fast_arg,
              FAST_TEST_CASE(Graph, lena, 10, 0),
              )
{
    int mode = arg_->mode;
    const char* imgname = arg_->imgname;
    int threshold = arg_->threshold;
    int nonmax = arg_->nonmax;
    vx_image src;
    vx_node node = 0;
    vx_graph graph = 0;
    CT_Image src0, dst0, mask0, dst1;
    vx_context context = context_->vx_context_;
    vx_scalar sthresh;
    vx_array corners;
    uint32_t width, height;
    vx_float32 threshold_f = (vx_float32)threshold;
    uint32_t ncorners0, ncorners;
    vx_size corners_data_size = 0;
    vx_keypoint_t* corners_data = 0;
    uint32_t i, dst1stride;
    vx_perf_t perf_node, perf_graph;

    ASSERT_NO_FAILURE(src0 = image_generate_random(WIDTH, HEIGHT, VX_DF_IMAGE_U8));
    ASSERT(src0->format == VX_DF_IMAGE_U8);

    width = src0->width;
    height = src0->height;

    ASSERT_NO_FAILURE(dst0 = ct_allocate_image(width, height, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(mask0 = ct_allocate_image(width, height, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(dst1 = ct_allocate_image(width, height, VX_DF_IMAGE_U8));
    dst1stride = ct_stride_bytes(dst1);
    ct_memset(dst1->data.y, 0, (vx_size)dst1stride*height);

    src = ct_image_to_vx_image(src0, context);
    sthresh = vxCreateScalar(context, VX_TYPE_FLOAT32, &threshold_f);
    corners = vxCreateArray(context, VX_TYPE_KEYPOINT, 80000);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    node = vxFastCornersNode(graph, src, sthresh, nonmax ? vx_true_e : vx_false_e, corners, 0);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0 && graph == 0);

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseScalar(&sthresh));
    VX_CALL(vxReleaseArray(&corners));

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Fast Corners", "No NMS", 0);
}


TEST_WITH_ARG(tiovxPerformance2, tiovxPerfGaussian3x3, Filter3x3_Arg,
    CT_GENERATE_PARAMETERS("lena", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ARG, read_image, "lena.bmp")
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxGaussian3x3Node(graph, src_image, dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    PrintPerf(perf_graph, perf_node, src->width, src->height, "Gaussian", "3x3", 0);
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    vx_border_t border;
    int width, height;
    vx_float32 scale;
} GaussianPmd_Arg;

static vx_size gaussian_pyramid_calc_max_levels_count(int width, int height, vx_float32 scale)
{
    vx_size level = 1;
    while ((16 <= width) && (16 <= height) && level < MAX_PMD_LEVELS)
    {
        level++;
        width = (int)((vx_float64)width * scale);
        height = (int)((vx_float64)height * scale);
    }
    return level;
}


#define ADD_VX_SCALE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_SCALE_PYRAMID_ORB", __VA_ARGS__, VX_SCALE_PYRAMID_ORB))

TEST_WITH_ARG(tiovxPerformance2, tiovxPerfGaussianPyramid, GaussianPmd_Arg,
    CT_GENERATE_PARAMETERS("lena", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ADD_VX_SCALE, ARG, read_image, "lena.bmp")
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    CT_Image input = NULL;
    vx_border_t border = arg_->border;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT(arg_->scale < 1.0);

    ASSERT_NO_FAILURE(input = arg_->generator( arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

    levels = gaussian_pyramid_calc_max_levels_count(input->width, input->height, arg_->scale);

    ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, input->width, input->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxGaussianPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);

    if (border.mode != VX_BORDER_UNDEFINED)
        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleasePyramid(&pyr));
    VX_CALL(vxReleaseImage(&input_image));
    ASSERT(pyr == 0);
    ASSERT(input_image == 0);

    PrintPerf(perf_graph, perf_node, input->width, input->height, "Gaussian Pyramid", "", 0);
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int width, height;
    vx_int32 kernel_size;
    vx_border_t border;
} HDG_Arg;


#define ADD_HSG_KERNEL_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/k=5", __VA_ARGS__, 5))

TEST_WITH_ARG(tiovxPerformance2, tiovxPerfHalfScaleGaussian, HDG_Arg,
    CT_GENERATE_PARAMETERS("lena", ADD_SIZE_NONE, ADD_HSG_KERNEL_SIZE, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ARG, read_image, "lena.bmp")
)
{
    vx_context context = context_->vx_context_;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    CT_Image src = NULL, dst = NULL;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_width = (src->width + 1) / 2;
    dst_height = (src->height + 1) / 2;

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxHalfScaleGaussianNode(graph, src_image, dst_image, arg_->kernel_size), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    PrintPerf(perf_graph, perf_node, src->width, src->height, "Half Scale Gaussian Pyramid", "", 0);
}

typedef struct {
    const char* testName;
    const char* filePrefix;
    vx_float32 min_distance;
    vx_float32 sensitivity;
    vx_int32  gradient_size;
    vx_int32  block_size;
} HarrisC_Arg;

typedef struct {
    vx_size         num_corners;
    vx_float32      strength_thresh;
    vx_keypoint_t   *pts;
} HarrisC_TruthData;

static vx_size harris_corner_read_line(const char *data, char *line)
{
    const char* ptr = data;
    int pos_temp = 0;
    while (*ptr && *ptr != '\n')
    {
        line[pos_temp] = *ptr;
        pos_temp++; ptr++;
    }
    line[pos_temp] = 0;
    return (ptr - data);
}

static void harris_corner_read_truth_data(const char *file_path, HarrisC_TruthData *truth_data, float strengthScale)
{
    FILE* f;
    long sz, read_sz;
    void* buf; char* ptr;
    char temp[1024];
    vx_size ln_size = 0;
    vx_size pts_count = 0;
    vx_keypoint_t *pt;

    ASSERT(truth_data && file_path);

    f = fopen(file_path, "rb");
    ASSERT(f);
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if (0 == sz)
    {
        fclose(f);
        return;
    }
    fseek(f, 0, SEEK_SET);

    buf = ct_alloc_mem(sz + 1);
    if (NULL == buf)
    {
        fclose(f);
        return;
    }
    read_sz = fread(buf, 1, sz, f);
    if (read_sz != sz)
    {
        ct_free_mem(buf);
        fclose(f);
        return;
    }

    fclose(f);

    ptr = (char *)buf;
    ptr[sz] = 0;
    ln_size = harris_corner_read_line(ptr, temp);
    ASSERT(ln_size);
    truth_data->num_corners = atoi(temp);
    ASSERT(truth_data->num_corners);
    ptr+= ln_size + 1;

    ASSERT(truth_data->pts = (vx_keypoint_t *)ct_alloc_mem(truth_data->num_corners * sizeof(vx_keypoint_t)));
    pt = truth_data->pts;
    for (;pts_count < truth_data->num_corners; ptr += ln_size + 1, pt++, pts_count++)
    {
        ln_size = harris_corner_read_line(ptr, temp);
        if (0 == ln_size)
            break;
        sscanf(temp, "%d %d %f", &pt->x, &pt->y, &pt->strength);
        pt->strength *= strengthScale;
    }
    ct_free_mem(buf);

    ASSERT(pts_count == truth_data->num_corners);
    truth_data->strength_thresh = truth_data->pts[truth_data->num_corners - 1].strength - FLT_EPSILON;
}

#define ADD_HC_VX_MIN_DISTANCE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/MIN_DISTANCE=30.0", __VA_ARGS__, 30.0f))

#define ADD_HC_VX_SENSITIVITY(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/SENSITIVITY=0.15", __VA_ARGS__, 0.15f))

#define ADD_HC_VX_GRADIENT_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/GRADIENT_SIZE=7", __VA_ARGS__, 7))

#define ADD_HC_VX_BLOCK_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/BLOCK_SIZE=7", __VA_ARGS__, 7))

TEST_WITH_ARG(tiovxPerformance2, tiovxPerfHarrisCorners, HarrisC_Arg,
    CT_GENERATE_PARAMETERS("many_strong_corners", ADD_HC_VX_MIN_DISTANCE, ADD_HC_VX_SENSITIVITY, ADD_HC_VX_GRADIENT_SIZE, ADD_HC_VX_BLOCK_SIZE, ARG, "hc_msc")
)
{
    vx_context context = context_->vx_context_;

    vx_image input_image = 0;
    vx_float32 strength_thresh;
    vx_float32 min_distance = arg_->min_distance + FLT_EPSILON;
    vx_float32 sensitivity = arg_->sensitivity;
    vx_size num_corners;
    vx_graph graph = 0;
    vx_node node = 0;
    size_t sz;
    vx_scalar strength_thresh_scalar, min_distance_scalar, sensitivity_scalar, num_corners_scalar;
    vx_array corners;
    char filepath[MAXPATHLENGTH];
    CT_Image input = NULL;
    HarrisC_TruthData truth_data;
    double scale = 1.0 / ((1 << (arg_->gradient_size - 1)) * arg_->block_size * 255.0);
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    memset(&truth_data, 0, sizeof(truth_data));
    scale = scale * scale * scale * scale;

    sz = snprintf(filepath, MAXPATHLENGTH, "%s/harriscorners/%s_%0.2f_%0.2f_%d_%d.txt", ct_get_test_file_path(), arg_->filePrefix, arg_->min_distance, arg_->sensitivity, arg_->gradient_size, arg_->block_size);
    ASSERT(sz < MAXPATHLENGTH);
    ASSERT_NO_FAILURE(harris_corner_read_truth_data(filepath, &truth_data, (float)scale));

    strength_thresh = truth_data.strength_thresh;

    sprintf(filepath, "harriscorners/%s.bmp", arg_->filePrefix);

    ASSERT_NO_FAILURE(input = ct_read_image(filepath, 1));
    ASSERT(input && (input->format == VX_DF_IMAGE_U8));
    ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

    num_corners = input->width * input->height / 10;

    ASSERT_VX_OBJECT(strength_thresh_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &strength_thresh), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(min_distance_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &min_distance), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(sensitivity_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &sensitivity), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_corners_scalar = vxCreateScalar(context, VX_TYPE_SIZE, &num_corners), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(corners = vxCreateArray(context, VX_TYPE_KEYPOINT, num_corners), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = vxHarrisCornersNode(graph, input_image, strength_thresh_scalar, min_distance_scalar,
                                                sensitivity_scalar, arg_->gradient_size, arg_->block_size, corners,
                                                num_corners_scalar), VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0);
    ASSERT(graph == 0);

    ct_free_mem(truth_data.pts); truth_data.pts = 0;
    VX_CALL(vxReleaseArray(&corners));
    VX_CALL(vxReleaseScalar(&num_corners_scalar));
    VX_CALL(vxReleaseScalar(&sensitivity_scalar));
    VX_CALL(vxReleaseScalar(&min_distance_scalar));
    VX_CALL(vxReleaseScalar(&strength_thresh_scalar));
    VX_CALL(vxReleaseImage(&input_image));

    ASSERT(truth_data.pts == 0);
    ASSERT(corners == 0);
    ASSERT(num_corners_scalar == 0);
    ASSERT(sensitivity_scalar == 0);
    ASSERT(min_distance_scalar == 0);
    ASSERT(strength_thresh_scalar == 0);
    ASSERT(input_image == 0);

    PrintPerf(perf_graph, perf_node, input->width, input->height, "Harris Corners", "", 0);
}

TEST(tiovxPerformance2, tiovxPerfHistogram)
{
    vx_image src;
    CT_Image src0;
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_distribution dist1;
    uint64_t rng;
    vx_perf_t perf_node, perf_graph;
    int val0, val1, offset, nbins, range;

    rng = CT()->seed_;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    val0 = CT_RNG_NEXT_INT(rng, 0, (MAX_HISTOGRAMS_BINS-1)), val1 = CT_RNG_NEXT_INT(rng, 0, (MAX_HISTOGRAMS_BINS-1));
    offset = CT_MIN(val0, val1), range = CT_MAX(val0, val1) - offset + 1;
    nbins = CT_RNG_NEXT_INT(rng, 1, range+1);

    ASSERT_NO_FAILURE(src0 = ct_allocate_ct_image_random(WIDTH, HEIGHT, VX_DF_IMAGE_U8, &rng, 0, 256));

    src = ct_image_to_vx_image(src0, context);
    ASSERT_VX_OBJECT(src, VX_TYPE_IMAGE);

    dist1 = vxCreateDistribution(context, nbins, offset, range);
    ASSERT_VX_OBJECT(dist1, VX_TYPE_DISTRIBUTION);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxHistogramNode(graph, src, dist1);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseDistribution(&dist1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0 && graph == 0);

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Histogram", "U8", 0);
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int width, height;
} Integral_Arg;

TEST_WITH_ARG(tiovxPerformance2, tiovxPerfIntegralImg, Integral_Arg,
    CT_GENERATE_PARAMETERS("lena", ADD_SIZE_NONE, ARG, read_image, "lena.bmp")
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    CT_Image src = NULL, dst = NULL;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, src->width, src->height, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxIntegralImageNode(graph, src_image, dst_image), VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    PrintPerf(perf_graph, perf_node, src->width, src->height, "Integral Image", "", 0);
}

static vx_size own_pyramid_calc_max_levels_count(int width, int height, vx_float32 scale)
{
    vx_size level = 1;

    while ((16 <= width) && (16 <= height) && level < LEVELS_COUNT_MAX)
    {
        level++;
        width  = (int)ceil((vx_float64)width * scale);
        height = (int)ceil((vx_float64)height * scale);
    }

    return level;
}

static void own_laplacian_pyramid_openvx(vx_context context, vx_border_t border, vx_image input, vx_pyramid laplacian, vx_image output, vx_size levels)
{
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxLaplacianPyramidNode(graph, input, laplacian, output), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    if (levels == 6)
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Laplacian Pyramid", "U8; Levels = 6", 0);

    return;
}

static void own_laplacian_reconstruct_openvx(vx_context context, vx_border_t border, vx_pyramid laplacian, vx_image input, vx_image output)
{
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxLaplacianReconstructNode(graph, laplacian, input, output), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Laplacian Reconstruct", "U8", 0);

    return;
}

TEST(tiovxPerformance2, tiovxPerfLaplacianReconstruct)
{
    vx_uint32 i;
    vx_context context = context_->vx_context_;
    vx_size levels = 0;
    vx_image src = 0;
    vx_image ref_lowest_res = 0;
    vx_image ref_dst = 0;
    vx_image tst_dst = 0;
    vx_pyramid ref_pyr = 0;

    CT_Image input = NULL;

    vx_border_t border = { VX_BORDER_UNDEFINED };
    vx_border_t build_border = {VX_BORDER_REPLICATE};

    ASSERT_NO_FAILURE(input = image_generate_random(WIDTH, HEIGHT, VX_DF_IMAGE_U8));
    ASSERT_VX_OBJECT(src = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

    levels = own_pyramid_calc_max_levels_count(input->width, input->height, VX_SCALE_PYRAMID_HALF);

    {
        vx_uint32 lowest_res_width  = input->width;
        vx_uint32 lowest_res_height = input->height;

        for (i = 1; i < levels; i++)
        {
            lowest_res_width  = (vx_uint32)ceilf(lowest_res_width * VX_SCALE_PYRAMID_HALF);
            lowest_res_height = (vx_uint32)ceilf(lowest_res_height * VX_SCALE_PYRAMID_HALF);
        }

        ASSERT_VX_OBJECT(ref_pyr = vxCreatePyramid(context, levels-1, VX_SCALE_PYRAMID_HALF, input->width, input->height, VX_DF_IMAGE_S16), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(ref_lowest_res = vxCreateImage(context, lowest_res_width, lowest_res_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(ref_dst = vxCreateImage(context, input->width, input->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(tst_dst = vxCreateImage(context, input->width, input->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    own_laplacian_pyramid_openvx(context, build_border, src, ref_pyr, ref_lowest_res, levels);
    own_laplacian_reconstruct_openvx(context, border, ref_pyr, ref_lowest_res, tst_dst);

    {
        CT_Image ct_ref_dst = 0;
        CT_Image ct_tst_dst = 0;

        ASSERT_NO_FAILURE(ct_tst_dst = ct_image_from_vx_image(tst_dst));
        EXPECT_CTIMAGE_NEAR(input, ct_tst_dst, 1);
    }

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleasePyramid(&ref_pyr));
    VX_CALL(vxReleaseImage(&ref_lowest_res));
    VX_CALL(vxReleaseImage(&ref_dst));
    VX_CALL(vxReleaseImage(&tst_dst));

    ASSERT(src == 0);
    ASSERT(ref_pyr == 0);
    ASSERT(ref_lowest_res == 0);
    ASSERT(ref_dst == 0);
    ASSERT(tst_dst == 0);
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height, vx_enum data_type);
    const char* fileName;
    void (*lut_generator)(void* data, vx_enum data_type);
    int width, height;
    vx_enum data_type;
} Lut_Arg;

// Generate input to cover these requirements:
// There should be a image with randomly generated pixel intensities.
static CT_Image lut_image_generate_random(const char* fileName, int width, int height, vx_enum data_type)
{
    CT_Image image = 0;

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256);
        break;
    case VX_TYPE_INT16:
        image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_S16, &CT()->seed_, -32768, 32768);
        break;
    }
    ASSERT_(return 0, image != 0);

    return image;
}

static void lut_data_fill_random(void* data, vx_enum data_type)
{
    uint64_t* seed = &CT()->seed_;
    int i;

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        {
            vx_uint8* data8 = (vx_uint8*)data;
            for (i = 0; i < 256; ++i)
                data8[i] = (vx_uint8)CT_RNG_NEXT_INT(*seed, 0, 256);
        }
        break;
    case VX_TYPE_INT16:
        {
            vx_int16* data16 = (vx_int16*)data;
            for (i = 0; i < 65536; ++i)
                data16[i] = (vx_int16)CT_RNG_NEXT_INT(*seed, (uint32_t)-32768, 32768);
        }
        break;
    }
}

static vx_size lut_count(vx_enum data_type)
{
    vx_size count = 0;

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        count = 256;
        break;
    case VX_TYPE_INT16:
        count = 65536;
        break;
    }

    return count;
}

static vx_size lut_size(vx_enum data_type)
{
    vx_size size = 0;

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        size = 256*sizeof(vx_uint8);
        break;
    case VX_TYPE_INT16:
        size = 65536*sizeof(vx_int16);
        break;
    }

    return size;
}

static vx_lut lut_create(vx_context context, void* data, vx_enum data_type)
{
    vx_size count = lut_count(data_type);
    vx_size size = lut_size(data_type);

    vx_lut lut = vxCreateLUT(context, data_type, count);
    void* ptr = NULL;

    ASSERT_VX_OBJECT_(return 0, lut, VX_TYPE_LUT);

    vx_map_id map_id;
    VX_CALL_(return 0, vxMapLUT(lut, &map_id, &ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    ASSERT_(return 0, ptr);
    memcpy(ptr, data, size);
    VX_CALL_(return 0, vxUnmapLUT(lut, map_id));
    return lut;
}

static CT_Image lut_image_read(const char* fileName, int width, int height, vx_enum data_type)
{
    CT_Image image8 = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image8 = ct_read_image(fileName, 1);
    ASSERT_(return 0, image8);
    ASSERT_(return 0, image8->format == VX_DF_IMAGE_U8);

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        return image8;
    case VX_TYPE_INT16:
        {
            vx_int32 offset = 65536/2;
            CT_Image image16 = ct_allocate_image(image8->width, image8->height, VX_DF_IMAGE_S16);
            if (image16)
            {
                CT_FILL_IMAGE_16S(return 0, image16,
                    {
                        vx_uint8 value8 = *CT_IMAGE_DATA_PTR_8U(image8, x, y);
                        vx_uint16 value16 = ((vx_uint16)value8 << 8) | value8;
                        vx_int16 res = (vx_int16)((vx_int32)value16 - offset);
                        *dst_data = res;
                    });
            }
            return image16;
        }
    }

    return NULL;
}

#define ADD_LUT_GENERATOR(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/LutRandom", __VA_ARGS__, lut_data_fill_random))

#define ADD_TYPE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/S16", __VA_ARGS__, VX_TYPE_INT16)), \
    CT_EXPAND(nextmacro(testArgName "/U8", __VA_ARGS__, VX_TYPE_UINT8))

TEST_WITH_ARG(tiovxPerformance2, tiovxPerfLUT, Lut_Arg,
    CT_GENERATE_PARAMETERS("lena", ADD_LUT_GENERATOR, ADD_SIZE_NONE, ADD_TYPE, ARG, lut_image_read, "lena.bmp")
)
{
    vx_enum data_type = arg_->data_type;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    CT_Image src = NULL, dst = NULL;
    void* lut_data;
    vx_lut lut;
    vx_size size;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    size = lut_size(data_type);
    lut_data = ct_alloc_mem(size);
    ASSERT(lut_data != 0);

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height, data_type));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(arg_->lut_generator(lut_data, data_type));
    ASSERT_VX_OBJECT(lut = lut_create(context, lut_data, data_type), VX_TYPE_LUT);

    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxTableLookupNode(graph, src_image, lut, dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseLUT(&lut));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(lut == 0);
    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    ct_free_mem(lut_data);
    if (arg_->data_type == VX_DF_IMAGE_S16)
        PrintPerf(perf_graph, perf_node, src->width, src->height, "LookUpTable", "S16", 0);
    else
        PrintPerf(perf_graph, perf_node, src->width, src->height, "LookUpTable", "U8", 0);
}

TEST(tiovxPerformance2, tiovxPerfMagnitude)
{
    vx_context context = context_->vx_context_;
    vx_image dx = 0, dy = 0, mag;
    vx_float32 alpha = 0.5f;
    vx_scalar alpha_scalar = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    CT_Image dx0, dy0, mag0;
    uint64_t rng;
    vx_perf_t perf_node, perf_graph;

    rng = CT()->seed_;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(dx0 = ct_allocate_ct_image_random(WIDTH, HEIGHT, VX_DF_IMAGE_S16, &rng, -32768, 32768));
    ASSERT_NO_FAILURE(dy0 = ct_allocate_ct_image_random(WIDTH, HEIGHT, VX_DF_IMAGE_S16, &rng, -32768, 32768));

    dx = ct_image_to_vx_image(dx0, context);
    ASSERT_VX_OBJECT(dx, VX_TYPE_IMAGE);
    dy = ct_image_to_vx_image(dy0, context);
    ASSERT_VX_OBJECT(dy, VX_TYPE_IMAGE);

    mag = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(mag, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxMagnitudeNode(graph, dx, dy, mag);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dx));
    VX_CALL(vxReleaseImage(&dy));
    VX_CALL(vxReleaseImage(&mag));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(dx == 0);
    ASSERT(dy == 0);
    ASSERT(mag == 0);
    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Magnitude", "S16", 0);
}

TEST(tiovxPerformance2, tiovxPerfMeanStdDev)
{
    int format = VX_DF_IMAGE_U8;
    vx_image src;
    CT_Image src0;
    vx_node node = 0;
    vx_graph graph = 0;
    vx_scalar mean_s, stddev_s;
    vx_context context = context_->vx_context_;
    uint64_t rng;
    vx_float32 mean0 = 0.f, stddev0 = 0.f, mean = 0.f, stddev = 0.f;
    int a = 0, b = 256;
    vx_perf_t perf_node, perf_graph;

    rng = CT()->seed_;

    mean_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &mean);
    ASSERT_VX_OBJECT(mean_s, VX_TYPE_SCALAR);
    stddev_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &stddev);
    ASSERT_VX_OBJECT(stddev_s, VX_TYPE_SCALAR);

    int width = WIDTH;
    int height = HEIGHT;

    src0 = ct_allocate_ct_image_random(width, height, format, &rng, a, b);
    src = ct_image_to_vx_image(src0, context);
    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    node = vxMeanStdDevNode(graph, src, mean_s, stddev_s);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxCopyScalar(mean_s, &mean, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxCopyScalar(stddev_s, &stddev, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0 && graph == 0);
    CT_CollectGarbage(CT_GC_IMAGE);

    VX_CALL(vxReleaseScalar(&mean_s));
    VX_CALL(vxReleaseScalar(&stddev_s));

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Mean/Standard Deviation", "U8", 0);
}


typedef struct {
    const char* testName;
    vx_df_image format;
} min_max_arg;

#define MIN_MAX_FUZZY_ARGS_(name, format)  \
    ARG(#name "/MinMax(" #format")",   \
         VX_DF_IMAGE_##format)

#define MIN_MAX_FUZZY_ARGS(format)   \
    MIN_MAX_FUZZY_ARGS_(Graph, format)

TEST_WITH_ARG(tiovxPerformance2, tiovxPerfMinMaxLoc, min_max_arg,
     MIN_MAX_FUZZY_ARGS(U8),
     MIN_MAX_FUZZY_ARGS(S16))
{
    const int MAX_CAP = 300;
    vx_image src;
    CT_Image src0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_context context = context_->vx_context_;
    uint64_t rng;
    int a, b;
    int minval0 = 0, maxval0 = 0, minval = 0, maxval = 0;
    uint32_t mincount0 = 0, maxcount0 = 0, mincount = 0, maxcount = 0;
    vx_scalar minval_, maxval_, mincount_, maxcount_;
    vx_array minloc_ = 0, maxloc_ = 0;
    vx_enum sctype = arg_->format == VX_DF_IMAGE_U8 ? VX_TYPE_UINT8 :
                     arg_->format == VX_DF_IMAGE_S16 ? VX_TYPE_INT16 :
                     VX_TYPE_INT32;
    uint32_t pixsize = ct_image_bits_per_pixel(arg_->format)/8;
    vx_size bufbytes = 0, npoints = 0, bufcap = 0;
    vx_perf_t perf_node, perf_graph;

    a = 0, b = 256;

    minval_ = ct_scalar_from_int(context, sctype, 0);
    maxval_ = ct_scalar_from_int(context, sctype, 0);
    mincount_ = ct_scalar_from_int(context, VX_TYPE_UINT32, 0);
    maxcount_ = ct_scalar_from_int(context, VX_TYPE_UINT32, 0);
    minloc_ = vxCreateArray(context, VX_TYPE_COORDINATES2D, MAX_CAP);
    maxloc_ = vxCreateArray(context, VX_TYPE_COORDINATES2D, MAX_CAP);
    ASSERT(vxGetStatus((vx_reference)minloc_) == VX_SUCCESS && vxGetStatus((vx_reference)maxloc_) == VX_SUCCESS);

    rng = CT()->seed_;

    int return_loc = CT_RNG_NEXT_INT(rng, 0, 2);
    int return_count = CT_RNG_NEXT_INT(rng, 0, 2);
    uint32_t stride;
    int width, height;

    width = WIDTH;
    height = HEIGHT;

    src0 = ct_allocate_ct_image_random(width, height, arg_->format, &rng, a, b);
    stride = ct_stride_bytes(src0);
    src = ct_image_to_vx_image(src0, context);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    node = vxMinMaxLocNode(graph, src, minval_, maxval_,
                           return_loc ? minloc_ : 0,
                           return_loc ? maxloc_ : 0,
                           return_count ? mincount_ : 0,
                           return_count ? maxcount_ : 0);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0 && graph == 0);
    CT_CollectGarbage(CT_GC_IMAGE);

    VX_CALL(vxReleaseScalar(&minval_));
    VX_CALL(vxReleaseScalar(&maxval_));
    VX_CALL(vxReleaseScalar(&mincount_));
    VX_CALL(vxReleaseScalar(&maxcount_));
    VX_CALL(vxReleaseArray(&minloc_));
    VX_CALL(vxReleaseArray(&maxloc_));

    if (arg_->format == VX_DF_IMAGE_U8)
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Min Max Loc", "U8", 0);
    else
        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Min Max Loc", "S16", 0);
}

TEST(tiovxPerformance2, tiovxPerfMultiply888)
{
    vx_image src1, src2, dst;
    vx_graph graph;
    vx_node node = 0;
    vx_scalar scale = 0;
    CT_Image ref1, ref2, refdst, refdst_plus_1, vxdst;
    vx_context context = context_->vx_context_;
    vx_perf_t perf_node, perf_graph;
    vx_float32 scale_float = ONE_2_0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scale_float), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));

    // build one-node graph
    ASSERT_VX_OBJECT(node = vxMultiplyNode(graph, src1, src2, scale, VX_CONVERT_POLICY_WRAP, VX_ROUND_POLICY_TO_ZERO, dst), VX_TYPE_NODE);

    ctSetTileSize(node);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseScalar(&scale));
    VX_CALL(vxReleaseGraph(&graph));

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Multiply", "U8 x U8 = U8", 0);
}

TEST(tiovxPerformance2, tiovxPerfMultiply8816)
{
    vx_image src1, src2, dst;
    vx_graph graph;
    vx_node node = 0;
    vx_scalar scale = 0;
    CT_Image ref1, ref2, refdst, refdst_plus_1, vxdst;
    vx_context context = context_->vx_context_;
    vx_perf_t perf_node, perf_graph;
    vx_float32 scale_float = ONE_2_0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scale_float), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));

    // build one-node graph
    ASSERT_VX_OBJECT(node = vxMultiplyNode(graph, src1, src2, scale, VX_CONVERT_POLICY_WRAP, VX_ROUND_POLICY_TO_ZERO, dst), VX_TYPE_NODE);

    ctSetTileSize(node);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseScalar(&scale));
    VX_CALL(vxReleaseGraph(&graph));

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Multiply", "U8 x U8 = S16", 0);
}

TEST(tiovxPerformance2, tiovxPerfMultiply81616)
{
    vx_image src1, src2, dst;
    vx_graph graph;
    vx_node node = 0;
    vx_scalar scale = 0;
    CT_Image ref1, ref2, refdst, refdst_plus_1, vxdst;
    vx_context context = context_->vx_context_;
    vx_perf_t perf_node, perf_graph;
    vx_float32 scale_float = ONE_2_0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scale_float), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));

    // build one-node graph
    ASSERT_VX_OBJECT(node = vxMultiplyNode(graph, src1, src2, scale, VX_CONVERT_POLICY_WRAP, VX_ROUND_POLICY_TO_ZERO, dst), VX_TYPE_NODE);

    ctSetTileSize(node);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseScalar(&scale));
    VX_CALL(vxReleaseGraph(&graph));

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Multiply", "U8 x S16 = S16", 0);
}

TEST(tiovxPerformance2, tiovxPerfMultiply161616)
{
    vx_image src1, src2, dst;
    vx_graph graph;
    vx_node node = 0;
    vx_scalar scale = 0;
    CT_Image ref1, ref2, refdst, refdst_plus_1, vxdst;
    vx_context context = context_->vx_context_;
    vx_perf_t perf_node, perf_graph;
    vx_float32 scale_float = ONE_2_0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scale_float), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));

    // build one-node graph
    ASSERT_VX_OBJECT(node = vxMultiplyNode(graph, src1, src2, scale, VX_CONVERT_POLICY_WRAP, VX_ROUND_POLICY_TO_ZERO, dst), VX_TYPE_NODE);

    ctSetTileSize(node);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseScalar(&scale));
    VX_CALL(vxReleaseGraph(&graph));

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Multiply", "S16 x S16 = S16", 0);
}

TEST_WITH_ARG(tiovxPerformance2, tiovxPerfMedian3x3, Filter3x3_Arg,
    CT_GENERATE_PARAMETERS("lena", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ARG, read_image, "lena.bmp")
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = ct_create_similar_image(src_image), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxMedian3x3Node(graph, src_image, dst_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    PrintPerf(perf_graph, perf_node, src->width, src->height, "Median", "3x3", 0);
}


typedef struct {
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    const char* fileName;
    vx_size mask_size;
    vx_enum function;
    vx_enum pattern;
    vx_border_t border;
    int width, height;
} NLFilter_Arg;


#define NLF_ADD_FUNCTIONS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_NONLINEAR_FILTER_MIN", __VA_ARGS__, VX_NONLINEAR_FILTER_MIN)), \
    CT_EXPAND(nextmacro(testArgName "/VX_NONLINEAR_FILTER_MAX", __VA_ARGS__, VX_NONLINEAR_FILTER_MAX)), \
    CT_EXPAND(nextmacro(testArgName "/VX_NONLINEAR_FILTER_MEDIAN", __VA_ARGS__, VX_NONLINEAR_FILTER_MEDIAN))

#define NLF_NLF_ADD_PATTERNS_BOX_CROSS_DISK(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_PATTERN_DISK", __VA_ARGS__, VX_PATTERN_DISK))

#define NLF_ADD_PATTERNS_BOX_CROSS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_PATTERN_CROSS", __VA_ARGS__, VX_PATTERN_CROSS))

TEST_WITH_ARG(tiovxPerformance2, tiovxPerfNonLinearFilter, NLFilter_Arg,
    CT_GENERATE_PARAMETERS("randomInput/mask=5x5", NLF_ADD_FUNCTIONS, NLF_NLF_ADD_PATTERNS_BOX_CROSS_DISK, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 5)
    )
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_matrix mask = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_enum pattern = 0;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    mask = vxCreateMatrixFromPattern(context, arg_->pattern, arg_->mask_size, arg_->mask_size);
    ASSERT_VX_OBJECT(mask, VX_TYPE_MATRIX);
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_PATTERN, &pattern, sizeof(pattern)));
    ASSERT_EQ_INT(arg_->pattern, pattern);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxNonLinearFilterNode(graph, arg_->function, src_image, mask, dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseMatrix(&mask));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(mask == 0);
    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    if (arg_->function == VX_NONLINEAR_FILTER_MIN)
        PrintPerf(perf_graph, perf_node, src->width, src->height, "NonLinear Filter", "mask=5x5; Erosion", 0);
    else if (arg_->function == VX_NONLINEAR_FILTER_MAX)
        PrintPerf(perf_graph, perf_node, src->width, src->height, "NonLinear Filter", "mask=5x5; Dilation", 0);
    else if (arg_->function == VX_NONLINEAR_FILTER_MEDIAN)
        PrintPerf(perf_graph, perf_node, src->width, src->height, "NonLinear Filter", "mask=5x5; Median", 0);
}

typedef struct {
    const char* testName;
    int dummy;
    vx_enum interpolation;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    void (*dst_size_generator)(int width, int height, int* dst_width, int* dst_height);
    int exact_result;
    int width, height;
    vx_border_t border;
} Scale_Arg;

static void dst_size_generator_1_2(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = width * 2;
    *dst_height = height * 2;
}

#define STR_VX_INTERPOLATION_NEAREST_NEIGHBOR "NN"
#define STR_VX_INTERPOLATION_BILINEAR "BILINEAR"
#define STR_VX_INTERPOLATION_AREA "AREA"

#define SCALE_TEST(interpolation, inputDataGenerator, inputDataFile, scale, exact, nextmacro, ...) \
    CT_EXPAND(nextmacro(STR_##interpolation "/" inputDataFile "/" #scale, __VA_ARGS__, \
            interpolation, inputDataGenerator, inputDataFile, dst_size_generator_ ## scale, exact))

#define SCALE_PARAMETERS \
    /* 1:1 scale */ \
    SCALE_TEST(VX_INTERPOLATION_NEAREST_NEIGHBOR, NULL, "random", 1_2, 1, ADD_SIZE_640x480, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ARG, 0), \
    SCALE_TEST(VX_INTERPOLATION_BILINEAR, NULL, "random", 1_2, 1, ADD_SIZE_640x480, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ARG, 0), \

TEST_WITH_ARG(tiovxPerformance2, tiovxPerfScale, Scale_Arg,
    SCALE_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    CT_Image src = NULL, dst = NULL;

    ASSERT_NO_FAILURE(src = image_generate_random(WIDTH/2, HEIGHT/2, VX_DF_IMAGE_U8));
    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(arg_->dst_size_generator(src->width, src->height, &dst_width, &dst_height));

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxScaleImageNode(graph, src_image, dst_image, arg_->interpolation), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    if (arg_->interpolation == VX_INTERPOLATION_NEAREST_NEIGHBOR)
        PrintPerf(perf_graph, perf_node, dst_width, dst_height, "Scale Image", "1 to 2; Nearest Neighbor", 1);
    else if (arg_->interpolation == VX_INTERPOLATION_BILINEAR)
        PrintPerf(perf_graph, perf_node, dst_width, dst_height, "Scale Image", "1 to 2; Bilinear Interpolation", 1);
}

TEST_WITH_ARG(tiovxPerformance2, tiovxPerfSobel3x3, Filter3x3_Arg,
    CT_GENERATE_PARAMETERS("lena", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ARG, read_image, "lena.bmp")
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_x_image = 0, dst_y_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    CT_Image src = NULL, dst_x = NULL, dst_y = NULL;
    vx_border_t border = arg_->border;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_x_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst_x_image, VX_TYPE_IMAGE);

    dst_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst_y_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxSobel3x3Node(graph, src_image, dst_x_image, dst_y_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_x_image));
    VX_CALL(vxReleaseImage(&dst_y_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_x_image == 0);
    ASSERT(dst_y_image == 0);
    ASSERT(src_image == 0);

    PrintPerf(perf_graph, perf_node, src->width, src->height, "Sobel", "3x3", 0);
}

static vx_array own_create_keypoint_array(vx_context context, vx_size count, vx_keypoint_t* keypoints)
{
    vx_array arr = 0;

    ASSERT_VX_OBJECT_(return 0, arr = vxCreateArray(context, VX_TYPE_KEYPOINT, count), VX_TYPE_ARRAY);

    VX_CALL_(return 0, vxAddArrayItems(arr, count, keypoints, sizeof(vx_keypoint_t)));

    return arr;
}

static CT_Image optflow_pyrlk_read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
    return image;
}

static vx_size own_read_keypoints(const char* fileName, vx_keypoint_t** p_old_points, vx_keypoint_t** p_new_points)
{
    size_t sz = 0, read_sz;
    void* buf = 0;
    char file[MAXPATHLENGTH];

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return 0, (sz < MAXPATHLENGTH));
#if 1
    FILE* f = fopen(file, "rb");
    ASSERT_(return 0, f);
    fseek(f, 0, SEEK_END);

    sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    buf = ct_alloc_mem(sz + 1);
    if (NULL == buf)
    {
        fclose(f);
        return 0;
    }
    read_sz = fread(buf, 1, sz, f);
    if (read_sz != sz)
    {
        ct_free_mem(buf);
        fclose(f);
        return 0;
    }
    fclose(f); f = NULL;
    ((char*)buf)[sz] = 0;
#else
    sz = ...
    buf = ...
#endif

    ASSERT_(return 0, *p_old_points = ct_alloc_mem(sizeof(vx_keypoint_t) * MAX_POINTS));
    ASSERT_(return 0, *p_new_points = ct_alloc_mem(sizeof(vx_keypoint_t) * MAX_POINTS));

    {
        int num = 0;
        char* pos = buf;
        char* next = 0;
        while(pos && (next = strchr(pos, '\n')))
        {
            int id = 0, status = 0;
            float x1, y1, x2, y2;

            int res;

            *next = 0;
            res = sscanf(pos, "%d %d %g %g %g %g", &id, &status, &x1, &y1, &x2, &y2);
            pos = next + 1;
            if (res == 6)
            {
                (*p_old_points)[num].x = (vx_int32)x1;
                (*p_old_points)[num].y = (vx_int32)y1;
                (*p_old_points)[num].strength = 1;
                (*p_old_points)[num].scale = 0;
                (*p_old_points)[num].orientation = 0;
                (*p_old_points)[num].tracking_status = 1;
                (*p_old_points)[num].error = 0;

                (*p_new_points)[num].x = (vx_int32)x2;
                (*p_new_points)[num].y = (vx_int32)y2;
                (*p_new_points)[num].strength = 1;
                (*p_new_points)[num].scale = 0;
                (*p_new_points)[num].orientation = 0;
                (*p_new_points)[num].tracking_status = status;
                (*p_new_points)[num].error = 0;

                num++;
            }
            else
                break;
        }

        ct_free_mem(buf);

        return num;
    }
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* src1_fileName;
    const char* src2_fileName;
    const char* points_fileName;
    vx_size winSize;
    int useReferencePyramid;
} Arg;


#define PARAMETERS \
    ARG("case1/5x5/ReferencePyramid", optflow_pyrlk_read_image, "optflow_00.bmp", "optflow_01.bmp", "optflow_pyrlk_5x5.txt", 5, 1), \

TEST_WITH_ARG(tiovxPerformance2, tiovxPerfOptFlow, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image   src_image[2] = { 0, 0 };
    vx_pyramid src_pyr[2]   = { 0, 0 };
    vx_array old_points_arr = 0;
    vx_array new_points_arr = 0;
    vx_float32 eps_val      = 0.001f;
    vx_uint32  num_iter_val = 100;
    vx_bool   use_estimations_val = vx_true_e;
    vx_scalar eps                 = 0;
    vx_scalar num_iter            = 0;
    vx_scalar use_estimations     = 0;
    vx_size   winSize             = arg_->winSize;
    vx_graph graph = 0;
    vx_node src_pyr_node[2] = { 0, 0 };
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;

    vx_size num_points = 0;
    vx_keypoint_t* old_points = 0;
    vx_keypoint_t* new_points_ref = 0;
    vx_keypoint_t* new_points = 0;
    vx_size new_points_size = 0;

    vx_size max_window_dim = 0;

    CT_Image src_ct_image[2] = {0, 0};

    VX_CALL(vxQueryContext(context, VX_CONTEXT_OPTICAL_FLOW_MAX_WINDOW_DIMENSION, &max_window_dim, sizeof(max_window_dim)));
    if (winSize > max_window_dim)
    {
        printf("%d window dim is not supported. Skip test\n", (int)winSize);
        return;
    }

    ASSERT_NO_FAILURE(src_ct_image[0] = image_generate_random(WIDTH, HEIGHT, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(src_ct_image[1] = image_generate_random(WIDTH, HEIGHT, VX_DF_IMAGE_U8));

    ASSERT_VX_OBJECT(src_image[0] = ct_image_to_vx_image(src_ct_image[0], context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image[1] = ct_image_to_vx_image(src_ct_image[1], context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src_pyr[0] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, src_ct_image[0]->width, src_ct_image[0]->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    ASSERT_VX_OBJECT(src_pyr[1] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, src_ct_image[0]->width, src_ct_image[0]->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_NO_FAILURE(num_points = own_read_keypoints(arg_->points_fileName, &old_points, &new_points_ref));

    ASSERT_VX_OBJECT(old_points_arr = own_create_keypoint_array(context, num_points, old_points), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(new_points_arr = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(eps             = vxCreateScalar(context, VX_TYPE_FLOAT32, &eps_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_iter        = vxCreateScalar(context, VX_TYPE_UINT32, &num_iter_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(use_estimations = vxCreateScalar(context, VX_TYPE_BOOL, &use_estimations_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    VX_CALL(vxuGaussianPyramid(context, src_image[0], src_pyr[0]));
    VX_CALL(vxuGaussianPyramid(context, src_image[1], src_pyr[1]));

    ASSERT_VX_OBJECT(node = vxOpticalFlowPyrLKNode(
        graph,
        src_pyr[0], src_pyr[1],
        old_points_arr, old_points_arr, new_points_arr,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ct_free_mem(new_points);
    ct_free_mem(new_points_ref);
    ct_free_mem(old_points);

    VX_CALL(vxReleaseNode(&node));
    if(src_pyr_node[0])
        VX_CALL(vxReleaseNode(&src_pyr_node[0]));
    if(src_pyr_node[1])
        VX_CALL(vxReleaseNode(&src_pyr_node[1]));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseScalar(&eps));
    VX_CALL(vxReleaseScalar(&num_iter));
    VX_CALL(vxReleaseScalar(&use_estimations));
    VX_CALL(vxReleaseArray(&old_points_arr));
    VX_CALL(vxReleaseArray(&new_points_arr));
    VX_CALL(vxReleasePyramid(&src_pyr[0]));
    VX_CALL(vxReleasePyramid(&src_pyr[1]));
    VX_CALL(vxReleaseImage(&src_image[0]));
    VX_CALL(vxReleaseImage(&src_image[1]));

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Optical Flow", "5x5", 0);
}

TEST(tiovxPerformance2, tiovxPerfPhase)
{
    vx_context context = context_->vx_context_;
    vx_image dx = 0, dy = 0, ph;
    vx_float32 alpha = 0.5f;
    vx_scalar alpha_scalar = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    CT_Image dx0, dy0, mag0;
    uint64_t rng;
    vx_perf_t perf_node, perf_graph;

    rng = CT()->seed_;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(dx0 = ct_allocate_ct_image_random(WIDTH, HEIGHT, VX_DF_IMAGE_S16, &rng, -32768, 32768));
    ASSERT_NO_FAILURE(dy0 = ct_allocate_ct_image_random(WIDTH, HEIGHT, VX_DF_IMAGE_S16, &rng, -32768, 32768));

    dx = ct_image_to_vx_image(dx0, context);
    ASSERT_VX_OBJECT(dx, VX_TYPE_IMAGE);
    dy = ct_image_to_vx_image(dy0, context);
    ASSERT_VX_OBJECT(dy, VX_TYPE_IMAGE);

    ph = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8);
    ASSERT_VX_OBJECT(ph, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxPhaseNode(graph, dx, dy, ph);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dx));
    VX_CALL(vxReleaseImage(&dy));
    VX_CALL(vxReleaseImage(&ph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(dx == 0);
    ASSERT(dy == 0);
    ASSERT(ph == 0);

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Phase", "", 0);
}

static vx_remap remap_generate_map(vx_context context, int src_width, int src_height, int dst_width, int dst_height, int type)
{
    vx_uint32 i;
    vx_uint32 j;
    vx_float32 x;
    vx_float32 y;
    vx_remap map = 0;
    vx_status status;

    map = vxCreateRemap(context, src_width, src_height, dst_width, dst_height);
    if (vxGetStatus((vx_reference)map) == VX_SUCCESS)
    {
        vx_float32 mat[3][2];
        vx_float32 angle, scale_x, scale_y, cos_a, sin_a;
        if (VX_MAP_IDENT == type)
        {
            mat[0][0] = 1.f;
            mat[0][1] = 0.f;

            mat[1][0] = 0.f;
            mat[1][1] = 1.f;

            mat[2][0] = 0.f;
            mat[2][1] = 0.f;
        }
        else if (VX_MAP_SCALE == type)
        {
            scale_x = src_width  / (vx_float32)dst_width;
            scale_y = src_height / (vx_float32)dst_height;

            mat[0][0] = scale_x;
            mat[0][1] = 0.f;

            mat[1][0] = 0.f;
            mat[1][1] = scale_y;

            mat[2][0] = 0.f;
            mat[2][1] = 0.f;
        }
        else if (VX_MAP_SCALE_ROTATE == type)
        {
            angle = M_PIF / RND_FLT(3.f, 6.f);
            scale_x = src_width  / (vx_float32)dst_width;
            scale_y = src_height / (vx_float32)dst_height;
            cos_a = cosf(angle);
            sin_a = sinf(angle);

            mat[0][0] = cos_a * scale_x;
            mat[0][1] = sin_a * scale_y;

            mat[1][0] = -sin_a * scale_x;
            mat[1][1] = cos_a  * scale_y;

            mat[2][0] = 0.f;
            mat[2][1] = 0.f;
        }
        else// if (VX_MATRIX_RANDOM == type)
        {
            angle = M_PIF / RND_FLT(3.f, 6.f);
            scale_x = src_width / (vx_float32)dst_width;
            scale_y = src_height / (vx_float32)dst_height;
            cos_a = cosf(angle);
            sin_a = sinf(angle);

            mat[0][0] = cos_a * RND_FLT(scale_x / 2.f, scale_x);
            mat[0][1] = sin_a * RND_FLT(scale_y / 2.f, scale_y);

            mat[1][0] = -sin_a * RND_FLT(scale_y / 2.f, scale_y);
            mat[1][1] = cos_a  * RND_FLT(scale_x / 2.f, scale_x);

            mat[2][0] = src_width  / 5.f * RND_FLT(-1.f, 1.f);
            mat[2][1] = src_height / 5.f * RND_FLT(-1.f, 1.f);
        }

        for (i = 0; i < (vx_uint32)dst_height; i++)
        {
            for (j = 0; j < (vx_uint32)dst_width; j++)
            {
                x = j * mat[0][0] + i * mat[1][0] + mat[2][0];
                y = j * mat[0][1] + i * mat[1][1] + mat[2][1];
                status = vxSetRemapPoint(map, j, i, x, y);
                if (VX_SUCCESS != status)
                    return 0;
            }
        }
    }

    return map;
}

TEST(tiovxPerformance2, tiovxPerfRemapBilinear)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_remap map = 0;
    vx_perf_t perf_node, perf_graph;

    CT_Image input = NULL, output = NULL;

    vx_border_t border = { VX_BORDER_UNDEFINED };

    ASSERT_NO_FAILURE(input = image_generate_random(WIDTH, HEIGHT, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(output = ct_allocate_image(WIDTH, HEIGHT, VX_DF_IMAGE_U8));

    ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(map = remap_generate_map(context, input->width, input->height, WIDTH, HEIGHT, VX_MAP_RANDOM), VX_TYPE_REMAP);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxRemapNode(graph, input_image, map, VX_INTERPOLATION_BILINEAR, output_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseRemap(&map));
    VX_CALL(vxReleaseImage(&output_image));
    VX_CALL(vxReleaseImage(&input_image));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(map == 0);
    ASSERT(output_image == 0);
    ASSERT(input_image == 0);

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Remap", "Bilinear Interpolation", 1);
}

TEST(tiovxPerformance2, tiovxPerfRemapNearest)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_remap map = 0;
    vx_perf_t perf_node, perf_graph;

    CT_Image input = NULL, output = NULL;

    vx_border_t border = { VX_BORDER_UNDEFINED };

    ASSERT_NO_FAILURE(input = image_generate_random(WIDTH, HEIGHT, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(output = ct_allocate_image(WIDTH, HEIGHT, VX_DF_IMAGE_U8));

    ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(map = remap_generate_map(context, input->width, input->height, WIDTH, HEIGHT, VX_MAP_RANDOM), VX_TYPE_REMAP);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxRemapNode(graph, input_image, map, VX_INTERPOLATION_NEAREST_NEIGHBOR, output_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseRemap(&map));
    VX_CALL(vxReleaseImage(&output_image));
    VX_CALL(vxReleaseImage(&input_image));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(map == 0);
    ASSERT(output_image == 0);
    ASSERT(input_image == 0);

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Remap", "Nearest Neighbor", 1);
}

enum CT_AffineMatrixType {
    VX_MATRIX_IDENT = 0,
    VX_MATRIX_ROTATE_90,
    VX_MATRIX_SCALE,
    VX_MATRIX_SCALE_ROTATE,
    VX_MATRIX_RANDOM
};

typedef struct {
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int src_width, src_height;
    int width, height;
    vx_border_t border;
    vx_enum interp_type;
    int matrix_type;
} WarpAffine_Arg;

#define ADD_VX_BORDERS_WARP_AFFINE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_CONSTANT=255", __VA_ARGS__, { VX_BORDER_CONSTANT, {{ 255 }} }))

#define ADD_VX_INTERP_TYPE_WARP_AFFINE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_INTERPOLATION_NEAREST_NEIGHBOR", __VA_ARGS__, VX_INTERPOLATION_NEAREST_NEIGHBOR)), \
    CT_EXPAND(nextmacro(testArgName "/VX_INTERPOLATION_BILINEAR", __VA_ARGS__, VX_INTERPOLATION_BILINEAR ))

#define ADD_VX_MATRIX_PARAM_WARP_AFFINE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_MATRIX_ROTATE_90", __VA_ARGS__,    VX_MATRIX_ROTATE_90))

#define RND_FLT(low, high)      (vx_float32)CT_RNG_NEXT_REAL(CT()->seed_, low, high);
static void warp_affine_generate_matrix(vx_float32* m, int src_width, int src_height, int dst_width, int dst_height, int type)
{
    vx_float32 mat[3][2];
    vx_float32 angle, scale_x, scale_y, cos_a, sin_a;
    if (VX_MATRIX_IDENT == type)
    {
        mat[0][0] = 1.f;
        mat[0][1] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = 1.f;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
    }
    else if (VX_MATRIX_ROTATE_90 == type)
    {
        mat[0][0] = 0.f;
        mat[0][1] = 1.f;

        mat[1][0] = -1.f;
        mat[1][1] = 0.f;

        mat[2][0] = (vx_float32)src_width;
        mat[2][1] = 0.f;
    }
    else if (VX_MATRIX_SCALE == type)
    {
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;

        mat[0][0] = scale_x;
        mat[0][1] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = scale_y;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
    }
    else if (VX_MATRIX_SCALE_ROTATE == type)
    {
        angle = M_PIF / RND_FLT(3.f, 6.f);
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;
        cos_a = cosf(angle);
        sin_a = sinf(angle);

        mat[0][0] = cos_a * scale_x;
        mat[0][1] = sin_a * scale_y;

        mat[1][0] = -sin_a * scale_x;
        mat[1][1] = cos_a  * scale_y;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
    }
    else// if (VX_MATRIX_RANDOM == type)
    {
        angle = M_PIF / RND_FLT(3.f, 6.f);
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;
        cos_a = cosf(angle);
        sin_a = sinf(angle);

        mat[0][0] = cos_a * RND_FLT(scale_x / 2.f, scale_x);
        mat[0][1] = sin_a * RND_FLT(scale_y / 2.f, scale_y);

        mat[1][0] = -sin_a * RND_FLT(scale_y / 2.f, scale_y);
        mat[1][1] = cos_a  * RND_FLT(scale_x / 2.f, scale_x);

        mat[2][0] = src_width  / 5.f * RND_FLT(-1.f, 1.f);
        mat[2][1] = src_height / 5.f * RND_FLT(-1.f, 1.f);
    }
    memcpy(m, mat, sizeof(mat));
}

static vx_matrix warp_affine_create_matrix(vx_context context, vx_float32 *m)
{
    vx_matrix matrix;
    matrix = vxCreateMatrix(context, VX_TYPE_FLOAT32, 2, 3);
    if (vxGetStatus((vx_reference)matrix) == VX_SUCCESS)
    {
        if (VX_SUCCESS != vxCopyMatrix(matrix, m, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
        {
            VX_CALL_(return 0, vxReleaseMatrix(&matrix));
        }
    }
    return matrix;
}

typedef struct {
    const char* name;
    int mode;
    vx_enum ttype;
} threshold_arg;

#define THRESHOLD_CASE(imm, ttype) { #imm "/" #ttype, CT_##imm##_MODE, VX_THRESHOLD_TYPE_##ttype }

#define CT_THRESHOLD_TRUE_VALUE  255
#define CT_THRESHOLD_FALSE_VALUE 0

TEST_WITH_ARG(tiovxPerformance2, tiovxPerfThreshold, threshold_arg,
              THRESHOLD_CASE(Graph, BINARY),
              )
{
    int format = VX_DF_IMAGE_U8;
    int ttype = arg_->ttype;
    int mode = arg_->mode;
    vx_image src, dst;
    vx_threshold vxt;
    CT_Image src0, dst0, dst1;
    vx_node node = 0;
    vx_graph graph = 0;
    vx_context context = context_->vx_context_;
    int iter, niters = 100;
    uint64_t rng;
    int a = 0, b = 256;
    int true_val = CT_THRESHOLD_TRUE_VALUE;
    int false_val = CT_THRESHOLD_FALSE_VALUE;
    vx_perf_t perf_node, perf_graph;

    rng = CT()->seed_;

    int width, height;

    uint8_t _ta = CT_RNG_NEXT_INT(rng, 0, 256), _tb = CT_RNG_NEXT_INT(rng, 0, 256);
    vx_int32 ta = CT_MIN(_ta, _tb), tb = CT_MAX(_ta, _tb);

    width = WIDTH;
    height = HEIGHT;

    ASSERT_NO_FAILURE(src0 = ct_allocate_ct_image_random(width, height, format, &rng, a, b));

    src = ct_image_to_vx_image(src0, context);
    dst = vxCreateImage(context, width, height, format);
    ASSERT_VX_OBJECT(dst, VX_TYPE_IMAGE);
    vxt = vxCreateThreshold(context, ttype, VX_TYPE_UINT8);
    if( ttype == VX_THRESHOLD_TYPE_BINARY )
    {
        vx_int32 v = 0;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt, VX_THRESHOLD_THRESHOLD_VALUE, &ta, sizeof(ta)));
        VX_CALL(vxQueryThreshold(vxt, VX_THRESHOLD_THRESHOLD_VALUE, &v, sizeof(v)));
        if (v != ta)
        {
            CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_VALUE failed\n");
        }
    }
    else
    {
        vx_int32 v1 = 0;
        vx_int32 v2 = 0;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt, VX_THRESHOLD_THRESHOLD_LOWER, &ta, sizeof(ta)));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt, VX_THRESHOLD_THRESHOLD_UPPER, &tb, sizeof(tb)));

        VX_CALL(vxQueryThreshold(vxt, VX_THRESHOLD_THRESHOLD_LOWER, &v1, sizeof(v1)));
        if (v1 != ta)
        {
            CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_LOWER failed\n");
        }

        VX_CALL(vxQueryThreshold(vxt, VX_THRESHOLD_THRESHOLD_UPPER, &v2, sizeof(v2)));
        if (v2 != tb)
        {
            CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_UPPER failed\n");
        }
    }

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt, VX_THRESHOLD_TRUE_VALUE, &true_val, sizeof(true_val)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt, VX_THRESHOLD_FALSE_VALUE, &false_val, sizeof(false_val)));

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    node = vxThresholdNode(graph, src, vxt, dst);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);
    if (vxIsGraphVerified(graph))
    {
        /* do not expect graph to be in verified state before vxGraphVerify call */
        CT_FAIL("check for vxIsGraphVerified() failed\n");
    }

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    if(!vxIsGraphVerified(graph))
    {
        /* expect graph to be in verified state before vxGraphVerify call */
        /* NB, according to the spec, vxProcessGraph may also do graph verification */
        CT_FAIL("check for vxIsGraphVerified() failed\n");
    }
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    dst1 = ct_image_from_vx_image(dst);

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseThreshold(&vxt));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0 && graph == 0);
    CT_CollectGarbage(CT_GC_IMAGE);

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Threshold", "Binary", 0);
}

TEST_WITH_ARG(tiovxPerformance2, tiovxPerfWarpAffine, WarpAffine_Arg,
    CT_GENERATE_PARAMETERS("lena", ADD_SIZE_256x256, ADD_VX_BORDERS_WARP_AFFINE, ADD_VX_INTERP_TYPE_WARP_AFFINE, ADD_VX_MATRIX_PARAM_WARP_AFFINE, ARG, read_image, "lena.bmp", 0, 0)
)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_matrix matrix = 0;
    vx_float32 m[6];

    CT_Image input = NULL, output = NULL;
    vx_border_t border = arg_->border;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->src_width, arg_->src_height));
    ASSERT_NO_FAILURE(output = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8));

    ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output, context), VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(warp_affine_generate_matrix(m, input->width, input->height, arg_->width, arg_->height, arg_->matrix_type));
    ASSERT_VX_OBJECT(matrix = warp_affine_create_matrix(context, m), VX_TYPE_MATRIX);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxWarpAffineNode(graph, input_image, matrix, arg_->interp_type, output_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseMatrix(&matrix));
    VX_CALL(vxReleaseImage(&output_image));
    VX_CALL(vxReleaseImage(&input_image));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(matrix == 0);
    ASSERT(output_image == 0);
    ASSERT(input_image == 0);

    if (arg_->interp_type == VX_INTERPOLATION_NEAREST_NEIGHBOR)
        PrintPerf(perf_graph, perf_node, input->width, input->height, "Warp Affine", "Nearest Neighbor", 1);
    else if (arg_->interp_type == VX_INTERPOLATION_BILINEAR)
        PrintPerf(perf_graph, perf_node, input->width, input->height, "Warp Affine", "Bilinear Interpolation", 1);
}

typedef struct {
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    const char*      fileName;
    int src_width, src_height;
    int width, height;
    vx_border_t border;
    vx_enum interp_type;
    int matrix_type;
} WarpPerspective_Arg;

#define ADD_VX_BORDERS_WARP_PERSPECTIVE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_CONSTANT=255", __VA_ARGS__, { VX_BORDER_CONSTANT, {{ 255 }} }))

#define ADD_VX_INTERP_TYPE_WARP_PERSPECTIVE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_INTERPOLATION_NEAREST_NEIGHBOR", __VA_ARGS__, VX_INTERPOLATION_NEAREST_NEIGHBOR)), \
    CT_EXPAND(nextmacro(testArgName "/VX_INTERPOLATION_BILINEAR", __VA_ARGS__, VX_INTERPOLATION_BILINEAR ))

#define ADD_VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_INTERPOLATION_NEAREST_NEIGHBOR", __VA_ARGS__, VX_INTERPOLATION_NEAREST_NEIGHBOR))

#define ADD_VX_MATRIX_PARAM_WARP_PERSPECTIVE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_MATRIX_SCALE_ROTATE", __VA_ARGS__, VX_MATRIX_SCALE_ROTATE))

static void warp_perspective_generate_matrix(vx_float32 *m, int src_width, int src_height, int dst_width, int dst_height, int type)
{
    vx_float32 mat[3][3];
    vx_float32 angle, scale_x, scale_y, cos_a, sin_a;
    if (VX_MATRIX_IDENT == type)
    {
        mat[0][0] = 1.f;
        mat[0][1] = 0.f;
        mat[0][2] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = 1.f;
        mat[1][2] = 0.f;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
        mat[2][2] = 1.f;
    }
    else if (VX_MATRIX_SCALE == type)
    {
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;

        mat[0][0] = scale_x;
        mat[0][1] = 0.f;
        mat[0][2] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = scale_y;
        mat[1][2] = 0.f;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
        mat[2][2] = 1.f;
    }
    else if (VX_MATRIX_SCALE_ROTATE == type)
    {
        angle = M_PIF / RND_FLT(3.f, 6.f);
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;
        cos_a = cosf(angle);
        sin_a = sinf(angle);

        mat[0][0] = cos_a * scale_x;
        mat[0][1] = sin_a * scale_y;
        mat[0][2] = 0.f;

        mat[1][0] = -sin_a * scale_x;
        mat[1][1] = cos_a  * scale_y;
        mat[1][2] = 0.f;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
        mat[2][2] = 1.f;
    }
    else// if (VX_MATRIX_RANDOM == type)
    {
        angle = M_PIF / RND_FLT(3.f, 6.f);
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;
        cos_a = cosf(angle);
        sin_a = sinf(angle);

        mat[0][0] = cos_a * RND_FLT(scale_x / 2.f, scale_x);
        mat[0][1] = sin_a * RND_FLT(scale_y / 2.f, scale_y);
        mat[0][2] = RND_FLT(0.f, 0.1f);

        mat[1][0] = -sin_a * RND_FLT(scale_y / 2.f, scale_y);
        mat[1][1] = cos_a  * RND_FLT(scale_x / 2.f, scale_x);
        mat[1][2] = RND_FLT(0.f, 0.1f);

        mat[2][0] = src_width  / 5.f * RND_FLT(-1.f, 1.f);
        mat[2][1] = src_height / 5.f * RND_FLT(-1.f, 1.f);
        mat[2][2] = 1.f;
    }
    memcpy(m, mat, sizeof(mat));
}

static vx_matrix warp_perspective_create_matrix(vx_context context, vx_float32 *m)
{
    vx_matrix matrix;
    matrix = vxCreateMatrix(context, VX_TYPE_FLOAT32, 3, 3);
    if (vxGetStatus((vx_reference)matrix) == VX_SUCCESS)
    {
        if (VX_SUCCESS != vxCopyMatrix(matrix, m, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
        {
            VX_CALL_(return 0, vxReleaseMatrix(&matrix));
        }
    }
    return matrix;
}

TEST_WITH_ARG(tiovxPerformance2, tiovxPerfWarpPerspective, WarpPerspective_Arg,
    CT_GENERATE_PARAMETERS("lena", ADD_SIZE_256x256, ADD_VX_BORDERS_WARP_PERSPECTIVE, ADD_VX_INTERP_TYPE_WARP_PERSPECTIVE, ADD_VX_MATRIX_PARAM_WARP_PERSPECTIVE, ARG, read_image, "lena.bmp", 0, 0)
)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_matrix matrix = 0;
    vx_float32 m[9];

    CT_Image input = NULL, output = NULL;

    vx_border_t border = arg_->border;
    vx_perf_t perf_node, perf_graph;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->src_width, arg_->src_height));
    ASSERT_NO_FAILURE(output = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8));

    ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output, context), VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(warp_perspective_generate_matrix(m, input->width, input->height, arg_->width, arg_->height, arg_->matrix_type));
    ASSERT_VX_OBJECT(matrix = warp_perspective_create_matrix(context, m), VX_TYPE_MATRIX);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxWarpPerspectiveNode(graph, input_image, matrix, arg_->interp_type, output_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ctSetTileSize(node);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseMatrix(&matrix));
    VX_CALL(vxReleaseImage(&output_image));
    VX_CALL(vxReleaseImage(&input_image));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(matrix == 0);
    ASSERT(output_image == 0);
    ASSERT(input_image == 0);

    if (arg_->interp_type == VX_INTERPOLATION_NEAREST_NEIGHBOR)
        PrintPerf(perf_graph, perf_node, input->width, input->height, "Warp Pespective", "Nearest Neighbor", 1);
    else if (arg_->interp_type == VX_INTERPOLATION_BILINEAR)
        PrintPerf(perf_graph, perf_node, input->width, input->height, "Warp Pespective", "Bilinear Interpolation", 1);
}

typedef vx_node   (VX_API_CALL *vxArithmFunction) (vx_graph, vx_image, vx_image, vx_enum, vx_image);

#define SGN_Add "+"
#define SGN_Subtract "-"

typedef struct {
    const char* name;
    enum vx_convert_policy_e policy;
    int width, height;
    vx_df_image arg1_format, arg2_format, result_format;
    vxArithmFunction vxFunc;
} arithm_arg;

#define ARITHM_FUZZY_ARGS_(func, p, w, h, f1, f2, fr)        \
    ARG(#func ": " #p " " #w "x" #h " " #f1 SGN_##func #f2 "=" #fr,   \
        VX_CONVERT_POLICY_##p, w, h, VX_DF_IMAGE_##f1, VX_DF_IMAGE_##f2, VX_DF_IMAGE_##fr, vx##func##Node)

#define ARITHM_FUZZY_ARGS(func)                         \
    ARITHM_FUZZY_ARGS_(func, SATURATE, WIDTH, HEIGHT, U8, U8, U8),      \
    ARITHM_FUZZY_ARGS_(func, SATURATE, WIDTH, HEIGHT, U8, U8, S16),   \
    ARITHM_FUZZY_ARGS_(func, SATURATE, WIDTH, HEIGHT, U8, S16, S16),\
    ARITHM_FUZZY_ARGS_(func, SATURATE, WIDTH, HEIGHT, S16, U8, S16),\
    ARITHM_FUZZY_ARGS_(func, SATURATE, WIDTH, HEIGHT, S16, S16, S16)

#ifdef BUILD_BAM

TEST_WITH_ARG(tiovxSupernodePerformance, tiovxSupernodePerfAdd, arithm_arg, ARITHM_FUZZY_ARGS(Add))
{
    int node_count = 3;
    vx_image src1, src2, src3, src4, dst, virt1, virt2;
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, arg_->result_format), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg2_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg2_format),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4, &CT()->seed_));

    ASSERT_VX_OBJECT(node1 = arg_->vxFunc(graph, src1, src2, arg_->policy, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = arg_->vxFunc(graph, src3, src4, arg_->policy, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = arg_->vxFunc(graph, virt1, virt2, arg_->policy, dst), VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&src4));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));


    if ( (arg_->arg1_format == VX_DF_IMAGE_U8) && (arg_->arg2_format == VX_DF_IMAGE_U8) && (arg_->result_format == VX_DF_IMAGE_U8) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Addition (3)", "U8+U8=U8", 0);
    else if ( (arg_->arg1_format == VX_DF_IMAGE_U8) && (arg_->arg2_format == VX_DF_IMAGE_U8) && (arg_->result_format == VX_DF_IMAGE_S16) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Addition (3)", "U8+U8=S16", 0);
    else if ( (arg_->arg1_format == VX_DF_IMAGE_U8) && (arg_->arg2_format == VX_DF_IMAGE_S16) && (arg_->result_format == VX_DF_IMAGE_S16) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Addition (3)", "U8+S16=S16", 0);
    else if ( (arg_->arg1_format == VX_DF_IMAGE_S16) && (arg_->arg2_format == VX_DF_IMAGE_U8) && (arg_->result_format == VX_DF_IMAGE_S16) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Addition (3)", "S16+U8=S16", 0);
    else if ( (arg_->arg1_format == VX_DF_IMAGE_S16) && (arg_->arg2_format == VX_DF_IMAGE_S16) && (arg_->result_format == VX_DF_IMAGE_S16) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Addition (3)", "S16+S16=S16", 0);
}

TEST_WITH_ARG(tiovxSupernodePerformance, tiovxSupernodePerfSubtract, arithm_arg, ARITHM_FUZZY_ARGS(Subtract))
{
    int node_count = 3;
    vx_image src1, src2, src3, src4, dst, virt1, virt2;
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, arg_->result_format), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg2_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg2_format),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4, &CT()->seed_));

    ASSERT_VX_OBJECT(node1 = arg_->vxFunc(graph, src1, src2, arg_->policy, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = arg_->vxFunc(graph, src3, src4, arg_->policy, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = arg_->vxFunc(graph, virt1, virt2, arg_->policy, dst), VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&src4));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst));

    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    if ( (arg_->arg1_format == VX_DF_IMAGE_U8) && (arg_->arg2_format == VX_DF_IMAGE_U8) && (arg_->result_format == VX_DF_IMAGE_U8) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Subtraction (3)", "U8+U8=U8", 0);
    else if ( (arg_->arg1_format == VX_DF_IMAGE_U8) && (arg_->arg2_format == VX_DF_IMAGE_U8) && (arg_->result_format == VX_DF_IMAGE_S16) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Subtraction (3)", "U8+U8=S16", 0);
    else if ( (arg_->arg1_format == VX_DF_IMAGE_U8) && (arg_->arg2_format == VX_DF_IMAGE_S16) && (arg_->result_format == VX_DF_IMAGE_S16) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Subtraction (3)", "U8+S16=S16", 0);
    else if ( (arg_->arg1_format == VX_DF_IMAGE_S16) && (arg_->arg2_format == VX_DF_IMAGE_U8) && (arg_->result_format == VX_DF_IMAGE_S16) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Subtraction (3)", "S16+U8=S16", 0);
    else if ( (arg_->arg1_format == VX_DF_IMAGE_S16) && (arg_->arg2_format == VX_DF_IMAGE_S16) && (arg_->result_format == VX_DF_IMAGE_S16) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Subtraction (3)", "S16+S16=S16", 0);

}

TEST(tiovxSupernodePerformance, tiovxSupernodePerfNot)
{
    int node_count = 3;
    vx_image src, dst, virt1, virt2;
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    ASSERT_VX_OBJECT(src = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src, &CT()->seed_));

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    // build one-node graph
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node1 = vxNotNode(graph, src, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, virt1, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxNotNode(graph, virt2, dst), VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Not (3)", "U8=>U8", 0);
}

typedef struct {
    const char* name;
    uint32_t width;
    uint32_t height;
    vxBinopFunction   vxFunc;
} binop_arg;

#define BINOP_FUZZY_ARGS_(func,w,h) \
    ARG(#func ": " #w "x" #h, w, h, vx##func##Node)

#define BINOP_FUZZY_ARGS(func)       \
    BINOP_FUZZY_ARGS_(func, WIDTH, HEIGHT)

TEST_WITH_ARG(tiovxSupernodePerformance, tiovxSupernodePerfAnd, binop_arg, BINOP_FUZZY_ARGS(And))
{
    int node_count = 3;
    vx_image src1, src2, src3, src4, dst, virt1, virt2;
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4, &CT()->seed_));

    ASSERT_VX_OBJECT(node1 = arg_->vxFunc(graph, src1, src2, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = arg_->vxFunc(graph, src3, src4, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = arg_->vxFunc(graph, virt1, virt2, dst), VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&src4));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst));

    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "And (3)", "U8+U8=U8", 0);
}

TEST_WITH_ARG(tiovxSupernodePerformance, tiovxSupernodePerfOr, binop_arg, BINOP_FUZZY_ARGS(Or))
{
    int node_count = 3;
    vx_image src1, src2, src3, src4, dst, virt1, virt2;
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4, &CT()->seed_));

    ASSERT_VX_OBJECT(node1 = arg_->vxFunc(graph, src1, src2, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = arg_->vxFunc(graph, src3, src4, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = arg_->vxFunc(graph, virt1, virt2, dst), VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&src4));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Or (3)", "U8+U8=U8", 0);
}

TEST_WITH_ARG(tiovxSupernodePerformance, tiovxSupernodePerfXor, binop_arg, BINOP_FUZZY_ARGS(Xor))
{
    int node_count = 3;
    vx_image src1, src2, src3, src4, dst, virt1, virt2;
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4, &CT()->seed_));

    ASSERT_VX_OBJECT(node1 = arg_->vxFunc(graph, src1, src2, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = arg_->vxFunc(graph, src3, src4, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = arg_->vxFunc(graph, virt1, virt2, dst), VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&src4));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst));

    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Xor (3)", "U8+U8=U8", 0);

}

TEST_WITH_ARG(tiovxSupernodePerformance, tiovxSupernodePerfAbsdiff8, binop_arg, BINOP_FUZZY_ARGS(AbsDiff))
{
    int node_count = 3;
    vx_image src1, src2, src3, src4, dst, virt1, virt2;
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4, &CT()->seed_));

    ASSERT_VX_OBJECT(node1 = arg_->vxFunc(graph, src1, src2, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = arg_->vxFunc(graph, src3, src4, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = arg_->vxFunc(graph, virt1, virt2, dst), VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&src4));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst));

    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "AbsDiff8 (3)", "U8+U8=U8", 0);

}

TEST(tiovxSupernodePerformance, tiovxSupernodePerfAbsdiff16)
{
    int node_count = 3;
    vx_image src1, src2, src3, src4, dst, virt1, virt2;
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16),   VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4, &CT()->seed_));

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    // build one-node graph
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node1 = vxAbsDiffNode(graph, src1, src2, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxAbsDiffNode(graph, src3, src4, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxAbsDiffNode(graph, virt1, virt2, dst), VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&src4));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));

    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Absdiff16 (3)", "S16+S16=S16", 0);

}


TEST(tiovxSupernodePerformance, tiovxSupernodePerfBox3x3)
{
    int node_count = 3;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt1, virt2;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    vx_rectangle_t rect;
    vx_bool valid_rect;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_border_t border;

    border.mode = VX_BORDER_UNDEFINED;
    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src_image, &CT()->seed_));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxBox3x3Node(graph, src_image, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxBox3x3Node(graph, virt1, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxBox3x3Node(graph, virt2, dst_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node3, VX_NODE_BORDER, &border, sizeof(border)));

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));


    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Box3x3 (3)", "U8=>U8", 0);
}

TEST(tiovxSupernodePerformance, tiovxSupernodePerfDilate3x3)
{
    int node_count = 3;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt1, virt2;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    vx_rectangle_t rect;
    vx_bool valid_rect;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_border_t border;

    border.mode = VX_BORDER_UNDEFINED;
    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src_image, &CT()->seed_));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxDilate3x3Node(graph, src_image, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxDilate3x3Node(graph, virt1, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxDilate3x3Node(graph, virt2, dst_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node3, VX_NODE_BORDER, &border, sizeof(border)));

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Dilate3x3 (3)", "U8=>U8", 0);
}

TEST(tiovxSupernodePerformance, tiovxSupernodePerfErode3x3)
{
    int node_count = 3;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt1, virt2;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    vx_rectangle_t rect;
    vx_bool valid_rect;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_border_t border;

    border.mode = VX_BORDER_UNDEFINED;
    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src_image, &CT()->seed_));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxErode3x3Node(graph, src_image, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxErode3x3Node(graph, virt1, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxErode3x3Node(graph, virt2, dst_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node3, VX_NODE_BORDER, &border, sizeof(border)));

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Erode3x3 (3)", "U8=>U8", 0);
}

TEST_WITH_ARG(tiovxSupernodePerformance, tiovxSupernodePerfCanny, canny_arg,
    CANNY_ARG(3, L1, 100, 120, lena_gray)
)
{
    int node_count = 3;
    uint32_t total, count;
    vx_image src, dst, virt1, virt2;
    vx_threshold hyst;
    vx_graph graph;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    CT_Image lena;
    vx_int32 low_thresh  = arg_->low_thresh;
    vx_int32 high_thresh = arg_->high_thresh;
    vx_border_t border = { VX_BORDER_UNDEFINED, {{ 0 }} };
    vx_int32 border_width = arg_->grad_size/2 + 1;
    vx_context context = context_->vx_context_;
    vx_enum thresh_data_type = VX_TYPE_UINT8;

    if (low_thresh > 255)
        thresh_data_type = VX_TYPE_INT16;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(lena = ct_read_image(arg_->filename, 1));
    ASSERT_NO_FAILURE(src = ct_image_to_vx_image(lena, context));
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, lena->width, lena->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(hyst = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, thresh_data_type), VX_TYPE_THRESHOLD);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_LOWER, &low_thresh,  sizeof(low_thresh)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_UPPER, &high_thresh, sizeof(high_thresh)));
    /* FALSE_VALUE and TRUE_VALUE of hyst parameter are set to their default values (0, 255) by vxCreateThreshold */
    /* test reference data are computed with assumption that FALSE_VALUE and TRUE_VALUE set to 0 and 255 */

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node1 = vxBox3x3Node(graph, src, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxDilate3x3Node(graph, virt1, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxCannyEdgeDetectorNode(graph, virt2, hyst, arg_->grad_size, arg_->norm_type, dst), VX_TYPE_NODE);
    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node3, VX_NODE_BORDER, &border, sizeof(border)));

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseThreshold(&hyst));
    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, lena->width, lena->height, "Box-Dilate-Canny", "U8=>U8", 0);

}

typedef struct {
    const char* testName;
    vx_df_image ccomb_dst_format;
    vx_df_image convert_format;
    int width, height;
} channel_comb_arg;

#define CHANNEL_COMBINE_FUZZY_ARGS_(name, ccf1, ccf2, int_kernel, w, h)        \
    ARG(#name "/ChannelCombine(dst=" #ccf1 ")-" #int_kernel "(dst=" #ccf2 ")-" #int_kernel "(dst=" #ccf1 ")",   \
         VX_DF_IMAGE_##ccf1, VX_DF_IMAGE_##ccf2, w, h)

#define CHANNEL_COMBINE_FUZZY_ARGS(ccomb_format, cconvert_format, intermediate_kernel)                         \
    CHANNEL_COMBINE_FUZZY_ARGS_(Graph, ccomb_format, cconvert_format, intermediate_kernel, WIDTH, HEIGHT)

TEST_WITH_ARG(tiovxSupernodePerformance, tiovxSupernodePerfChannelCombine, channel_comb_arg,
              CHANNEL_COMBINE_FUZZY_ARGS(RGB, RGBX, ColorConvert),
              CHANNEL_COMBINE_FUZZY_ARGS(RGBX, RGB, ColorConvert),
              CHANNEL_COMBINE_FUZZY_ARGS(NV12, IYUV, ColorConvert),
              CHANNEL_COMBINE_FUZZY_ARGS(NV12, RGB, ColorConvert),
              CHANNEL_COMBINE_FUZZY_ARGS(UYVY, U8, ChannelExtract),
              CHANNEL_COMBINE_FUZZY_ARGS(YUYV, U8, ChannelExtract),
              CHANNEL_COMBINE_FUZZY_ARGS(IYUV, NV12, ColorConvert),
              CHANNEL_COMBINE_FUZZY_ARGS(YUV4, U8, ChannelExtract)
              )
{
    int node_count = 3;
    vx_context context = context_->vx_context_;
    vx_image src_image[4] = {0, 0, 0, 0};
    vx_image virt1 = 0, virt2 = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    int channels = 0, i;
    CT_Image src1[4] = {NULL, NULL, NULL, NULL};
    CT_Image dst_dummy = NULL;
    vx_enum channel_ref;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(dst_dummy = ct_allocate_image(4, 4, arg_->ccomb_dst_format));

    ASSERT_NO_FAILURE(channels = ct_get_num_channels(arg_->ccomb_dst_format));
    channel_ref = (arg_->ccomb_dst_format==VX_DF_IMAGE_RGB)||(arg_->ccomb_dst_format==VX_DF_IMAGE_RGBX)?VX_CHANNEL_R:VX_CHANNEL_Y;
    for (i = 0; i < channels; i++)
    {
        int w = arg_->width / ct_image_get_channel_subsampling_x(dst_dummy, channel_ref + i);
        int h = arg_->height / ct_image_get_channel_subsampling_y(dst_dummy, channel_ref + i);
        ASSERT_NO_FAILURE(src1[i] = image_generate_random(w, h, VX_DF_IMAGE_U8));
        ASSERT_VX_OBJECT(src_image[i] = ct_image_to_vx_image(src1[i], context), VX_TYPE_IMAGE);
    }

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, arg_->ccomb_dst_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, arg_->convert_format), VX_TYPE_IMAGE);

    if (arg_->convert_format == VX_DF_IMAGE_U8)
    {
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, arg_->width, arg_->height, arg_->convert_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(node1 = vxChannelCombineNode(graph, src_image[0], src_image[1], src_image[2], src_image[3], virt1), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node2 = vxChannelExtractNode(graph, virt1, VX_CHANNEL_Y, virt2), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node3 = vxNotNode(graph, virt2, dst_image), VX_TYPE_NODE);
    }
    else
    {
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, arg_->width, arg_->height, arg_->ccomb_dst_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(node1 = vxChannelCombineNode(graph, src_image[0], src_image[1], src_image[2], src_image[3], virt1), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node2 = vxColorConvertNode(graph, virt1, virt2), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node3 = vxColorConvertNode(graph, virt2, dst_image), VX_TYPE_NODE);
    }

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    for (i = 0; i < channels; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
        ASSERT(src_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst_image));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    if (arg_->ccomb_dst_format == VX_DF_IMAGE_RGB)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "ChannelCombine-ColorConvert-ColorConvert (3)", "RGB=>RGBX=>RGB", 0);
    else if (arg_->ccomb_dst_format == VX_DF_IMAGE_RGBX)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "ChannelCombine-ColorConvert-ColorConvert (3)", "RGBX=>RGB=>RGBX", 0);
    else if (arg_->ccomb_dst_format == VX_DF_IMAGE_NV12)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "ChannelCombine-ColorConvert-ColorConvert (3)", "NV12=>RGB=>NV12", 0);
    else if (arg_->ccomb_dst_format == VX_DF_IMAGE_NV21)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "ChannelCombine-ColorConvert-ColorConvert (3)", "NV21=>RGB=>NV21", 0);
    else if (arg_->ccomb_dst_format == VX_DF_IMAGE_UYVY)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "ChannelCombine-ChannelExtract-Not (3)", "UYVY=>U8=>U8", 0);
    else if (arg_->ccomb_dst_format == VX_DF_IMAGE_YUYV)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "ChannelCombine-ChannelExtract-Not (3)", "YUYV=>U8=>U8", 0);
    else if (arg_->ccomb_dst_format == VX_DF_IMAGE_IYUV)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "ChannelCombine-ColorConvert-ColorConvert (3)", "IYUV=>NV12=>IYUV", 0);
    else if (arg_->ccomb_dst_format == VX_DF_IMAGE_YUV4)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "ChannelCombine-ChannelExtract-Not (3)", "YUV4=>U8=>U8", 0);

}

typedef struct {
    const char* name;
    vx_df_image srcformat;
    vx_df_image int1format;
    vx_df_image int2format;
    vx_df_image dstformat;
} color_conv_arg;

#define COLOR_CONVERT_FUZZY_ARGS_(name, srcfmt, int1fmt, int2fmt, dstfmt, node2name, node3name) \
    ARG(#name "/(" #srcfmt ")=>ColorConvert=>(" #int1fmt ")=>" #node2name "=>(" #int2fmt ")=>" #node3name "=>(" #dstfmt ")",   \
         VX_DF_IMAGE_##srcfmt, VX_DF_IMAGE_##int1fmt, VX_DF_IMAGE_##int2fmt, VX_DF_IMAGE_##dstfmt)

#define COLOR_CONVERT_FUZZY_ARGS(srcfmt, int1fmt, int2fmt, dstfmt, node2name, node3name) \
    COLOR_CONVERT_FUZZY_ARGS_(Graph, srcfmt, int1fmt, int2fmt, dstfmt, node2name, node3name)

TEST_WITH_ARG(tiovxSupernodePerformance, tiovxSupernodePerfColorConvert, color_conv_arg,
              COLOR_CONVERT_FUZZY_ARGS(RGB, YUV4, U8, U8, ChannelExtract, Not),
              COLOR_CONVERT_FUZZY_ARGS(RGB, RGBX, RGB, RGBX, ColorConvert, ColorConvert),
              COLOR_CONVERT_FUZZY_ARGS(RGB, NV12, RGBX, RGB, ColorConvert, ColorConvert),
              COLOR_CONVERT_FUZZY_ARGS(RGB, IYUV, RGBX, IYUV, ColorConvert, ColorConvert),

              COLOR_CONVERT_FUZZY_ARGS(RGBX, RGB, RGBX, YUV4, ColorConvert, ColorConvert),
              COLOR_CONVERT_FUZZY_ARGS(RGBX, NV12, RGB, NV12, ColorConvert, ColorConvert),
              COLOR_CONVERT_FUZZY_ARGS(RGBX, IYUV, RGB, RGBX, ColorConvert, ColorConvert),
              COLOR_CONVERT_FUZZY_ARGS(RGBX, YUV4, U8, U8, ChannelExtract, Not),

              COLOR_CONVERT_FUZZY_ARGS(NV12, YUV4, U8, U8, ChannelExtract, Not),
              COLOR_CONVERT_FUZZY_ARGS(NV12, RGB, NV12, RGB, ColorConvert, ColorConvert),
              COLOR_CONVERT_FUZZY_ARGS(NV12, RGBX, NV12, IYUV, ColorConvert, ColorConvert),
              COLOR_CONVERT_FUZZY_ARGS(NV12, IYUV, NV12, YUV4, ColorConvert, ColorConvert),

              COLOR_CONVERT_FUZZY_ARGS(NV21, RGB, IYUV, YUV4, ColorConvert, ColorConvert),
              COLOR_CONVERT_FUZZY_ARGS(NV21, RGBX, IYUV, NV12, ColorConvert, ColorConvert),
              COLOR_CONVERT_FUZZY_ARGS(NV21, IYUV, YUV4, U8, ColorConvert, ChannelExtract),
              COLOR_CONVERT_FUZZY_ARGS(NV21, YUV4, U8, U8, ChannelExtract, Not),

              COLOR_CONVERT_FUZZY_ARGS(IYUV, YUV4, U8, U8, ChannelExtract, Not),
              COLOR_CONVERT_FUZZY_ARGS(IYUV, RGB, YUV4, U8, ColorConvert, ChannelExtract),
              COLOR_CONVERT_FUZZY_ARGS(IYUV, RGBX, YUV4, U8, ColorConvert, ChannelExtract),
              COLOR_CONVERT_FUZZY_ARGS(IYUV, NV12, IYUV, RGB, ColorConvert, ColorConvert),

              COLOR_CONVERT_FUZZY_ARGS(UYVY, RGB, NV12, RGBX, ColorConvert, ColorConvert),
              COLOR_CONVERT_FUZZY_ARGS(UYVY, RGBX, RGB, IYUV, ColorConvert, ColorConvert),
              COLOR_CONVERT_FUZZY_ARGS(UYVY, NV12, YUV4, U8, ColorConvert, ChannelExtract),
              COLOR_CONVERT_FUZZY_ARGS(UYVY, IYUV, RGB, NV12, ColorConvert, ColorConvert),

              COLOR_CONVERT_FUZZY_ARGS(YUYV, RGB, RGBX, NV12, ColorConvert, ColorConvert),
              COLOR_CONVERT_FUZZY_ARGS(YUYV, RGBX, NV12, RGB, ColorConvert, ColorConvert),
              COLOR_CONVERT_FUZZY_ARGS(YUYV, NV12, RGB, IYUV, ColorConvert, ColorConvert),
              COLOR_CONVERT_FUZZY_ARGS(YUYV, IYUV, RGBX, RGB, ColorConvert, ColorConvert)
              )
{
    int node_count = 3;
    int srcformat = arg_->srcformat;
    int dstformat = arg_->dstformat;
    int int1format = arg_->int1format;
    int int2format = arg_->int2format;
    vx_image src=0, dst=0, virt1=0, virt2=0;
    CT_Image ref_src;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_context context = context_->vx_context_;
    uint64_t rng;
    vx_enum channel_ref;
    char *buffer = malloc(50);

    rng = CT()->seed_;

    int width = WIDTH;
    int height = HEIGHT;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    if( srcformat == VX_DF_IMAGE_RGB || srcformat == VX_DF_IMAGE_RGBX )
    {
        int scn = srcformat == VX_DF_IMAGE_RGB ? 3 : 4;
        ASSERT_NO_FAILURE(ref_src = ct_allocate_ct_image_random(width, height, srcformat, &rng, 0, 256));
    }
    else
    {
        ASSERT_NO_FAILURE(ref_src = ct_allocate_ct_image_random(width, height, srcformat, &rng, 0, 256));
    }

    ASSERT_NO_FAILURE(src = ct_image_to_vx_image(ref_src, context));
    ASSERT_VX_OBJECT(src, VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1 = vxCreateVirtualImage(graph, 0, 0, int1format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2 = vxCreateVirtualImage(graph, 0, 0, int2format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, width, height, dstformat), VX_TYPE_IMAGE);



    ASSERT_VX_OBJECT(node1 = vxColorConvertNode(graph, src, virt1), VX_TYPE_NODE);
    if (int2format == VX_DF_IMAGE_U8)
    {
        channel_ref = (int1format==VX_DF_IMAGE_RGB)||(int1format==VX_DF_IMAGE_RGBX)?VX_CHANNEL_R:VX_CHANNEL_Y;
        ASSERT_VX_OBJECT(node2 = vxChannelExtractNode(graph, virt1, channel_ref, virt2), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node3 = vxNotNode(graph, virt2, dst), VX_TYPE_NODE);
    }
    else
    {
        ASSERT_VX_OBJECT(node2 = vxColorConvertNode(graph, virt1, virt2), VX_TYPE_NODE);
        if (dstformat == VX_DF_IMAGE_U8)
        {
            channel_ref = (int2format==VX_DF_IMAGE_RGB)||(int2format==VX_DF_IMAGE_RGBX)?VX_CHANNEL_R:VX_CHANNEL_Y;
            ASSERT_VX_OBJECT(node3 = vxChannelExtractNode(graph, virt2, channel_ref, dst), VX_TYPE_NODE);
        }
        else
        {
            ASSERT_VX_OBJECT(node3 = vxColorConvertNode(graph, virt2, dst), VX_TYPE_NODE);
        }
    }

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    CT_CollectGarbage(CT_GC_IMAGE);

    if (srcformat == VX_DF_IMAGE_RGB)
        sprintf(buffer,"RGB=>");
    else if (srcformat == VX_DF_IMAGE_RGBX)
        sprintf(buffer,"RGBX=>");
    else if (srcformat == VX_DF_IMAGE_NV12)
        sprintf(buffer,"NV12=>");
    else if (srcformat == VX_DF_IMAGE_NV21)
        sprintf(buffer,"NV21=>");
    else if (srcformat == VX_DF_IMAGE_YUYV)
        sprintf(buffer,"YUYV=>");
    else if (srcformat == VX_DF_IMAGE_UYVY)
        sprintf(buffer,"UYVY=>");
    else if (srcformat == VX_DF_IMAGE_IYUV)
        sprintf(buffer,"IYUV=>");
    else if (srcformat == VX_DF_IMAGE_YUV4)
        sprintf(buffer,"YUV4=>");
    else if (srcformat == VX_DF_IMAGE_U8)
        sprintf(buffer,"U8=>");

    if (int1format == VX_DF_IMAGE_RGB)
    sprintf(buffer+strlen(buffer),"RGB=>");
    else if (int1format == VX_DF_IMAGE_RGBX)
        sprintf(buffer+strlen(buffer),"RGBX=>");
    else if (int1format == VX_DF_IMAGE_NV12)
        sprintf(buffer+strlen(buffer),"NV12=>");
    else if (int1format == VX_DF_IMAGE_NV21)
        sprintf(buffer+strlen(buffer),"NV21=>");
    else if (int1format == VX_DF_IMAGE_YUYV)
        sprintf(buffer+strlen(buffer),"YUYV=>");
    else if (int1format == VX_DF_IMAGE_UYVY)
        sprintf(buffer+strlen(buffer),"UYVY=>");
    else if (int1format == VX_DF_IMAGE_IYUV)
        sprintf(buffer+strlen(buffer),"IYUV=>");
    else if (int1format == VX_DF_IMAGE_YUV4)
        sprintf(buffer+strlen(buffer),"YUV4=>");
    else if (int1format == VX_DF_IMAGE_U8)
        sprintf(buffer+strlen(buffer),"U8=>");

    if (int2format == VX_DF_IMAGE_RGB)
        sprintf(buffer+strlen(buffer),"RGB=>");
    else if (int2format == VX_DF_IMAGE_RGBX)
        sprintf(buffer+strlen(buffer),"RGBX=>");
    else if (int2format == VX_DF_IMAGE_NV12)
        sprintf(buffer+strlen(buffer),"NV12=>");
    else if (int2format == VX_DF_IMAGE_NV21)
        sprintf(buffer+strlen(buffer),"NV21=>");
    else if (int2format == VX_DF_IMAGE_YUYV)
        sprintf(buffer+strlen(buffer),"YUYV=>");
    else if (int2format == VX_DF_IMAGE_UYVY)
        sprintf(buffer+strlen(buffer),"UYVY=>");
    else if (int2format == VX_DF_IMAGE_IYUV)
        sprintf(buffer+strlen(buffer),"IYUV=>");
    else if (int2format == VX_DF_IMAGE_YUV4)
        sprintf(buffer+strlen(buffer),"YUV4=>");
    else if (int2format == VX_DF_IMAGE_U8)
        sprintf(buffer+strlen(buffer),"U8=>");

    if (dstformat == VX_DF_IMAGE_RGB)
        sprintf(buffer+strlen(buffer),"RGB");
    else if (dstformat == VX_DF_IMAGE_RGBX)
        sprintf(buffer+strlen(buffer),"RGBX");
    else if (dstformat == VX_DF_IMAGE_NV12)
        sprintf(buffer+strlen(buffer),"NV12");
    else if (dstformat == VX_DF_IMAGE_NV21)
        sprintf(buffer+strlen(buffer),"NV21");
    else if (dstformat == VX_DF_IMAGE_YUYV)
        sprintf(buffer+strlen(buffer),"YUYV");
    else if (dstformat == VX_DF_IMAGE_UYVY)
        sprintf(buffer+strlen(buffer),"UYVY");
    else if (dstformat == VX_DF_IMAGE_IYUV)
        sprintf(buffer+strlen(buffer),"IYUV");
    else if (dstformat == VX_DF_IMAGE_YUV4)
        sprintf(buffer+strlen(buffer),"YUV4");
    else if (dstformat == VX_DF_IMAGE_U8)
        sprintf(buffer+strlen(buffer),"U8=>");

    if (int2format == VX_DF_IMAGE_U8)
    {
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, width, height, "ColorConvert-ChannelExtract-Not (3)", buffer, 0);
    }
    else
    {
        if (dstformat == VX_DF_IMAGE_U8)
        {
            PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, width, height, "ColorConvert-ColorConvert-ChannelExtract (3)", buffer, 0);
        }
        else
        {
            PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, width, height, "ColorConvert-ColorConvert-ColorConvert (3)", buffer, 0);
        }
    }
}

typedef struct {
    const char* testName;
    vx_df_image format;
    vx_enum channel0;
    vx_enum channel1;
} channel_extract_args;

#define CHANNEL_EXTRACT_FUZZY_ARGS_(name, fmt, channel0, channel1) \
    ARG(#name "/ChannelExtract:" #fmt "->" #channel0 "/" #channel1,   \
         VX_DF_IMAGE_##fmt, VX_CHANNEL_##channel0, VX_CHANNEL_##channel1)

#define CHANNEL_EXTRACT_FUZZY_ARGS(srcfmt, channel0, channel1) \
    CHANNEL_EXTRACT_FUZZY_ARGS_(Graph, srcfmt, channel0, channel1)

TEST_WITH_ARG(tiovxSupernodePerformance, tiovxSupernodePerfChannelExtract, channel_extract_args,
    CHANNEL_EXTRACT_FUZZY_ARGS(RGB,  R, G),
    CHANNEL_EXTRACT_FUZZY_ARGS(RGBX, B, A),
    CHANNEL_EXTRACT_FUZZY_ARGS(YUYV, Y, U),
    CHANNEL_EXTRACT_FUZZY_ARGS(UYVY, Y, V),
    CHANNEL_EXTRACT_FUZZY_ARGS(NV12, Y, U),
    CHANNEL_EXTRACT_FUZZY_ARGS(NV21, Y, V),
    CHANNEL_EXTRACT_FUZZY_ARGS(YUV4, Y, V),
    CHANNEL_EXTRACT_FUZZY_ARGS(IYUV, Y, U)
)
{
    int node_count = 3;
    vx_context context = context_->vx_context_;
    vx_image src_image[4] = {0, 0, 0, 0};
    vx_image virt = 0;
    vx_image dst_image0 = 0, dst_image1 = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    int channels = 0, i;
    CT_Image src1[4] = {NULL, NULL, NULL, NULL};
    CT_Image dst_dummy = NULL, src_dummy = NULL;
    CT_Image dst_ref0 = NULL, dst_ref1 = NULL;
    vx_enum channel_ref;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(dst_dummy = ct_allocate_image(4, 4, arg_->format));
    ASSERT_NO_FAILURE(channels = ct_get_num_channels(arg_->format));

    ASSERT_NO_FAILURE(src_dummy = channel_extract_image_generate_random(WIDTH, HEIGHT, arg_->format));
    ASSERT_NO_FAILURE(dst_ref0 = channel_extract_create_reference_image(src_dummy, arg_->channel0));
    ASSERT_NO_FAILURE(dst_ref1 = channel_extract_create_reference_image(src_dummy, arg_->channel1));

    channel_ref = (arg_->format==VX_DF_IMAGE_RGB)||(arg_->format==VX_DF_IMAGE_RGBX)?VX_CHANNEL_R:VX_CHANNEL_Y;
    for (i = 0; i < channels; i++)
    {
        int w = WIDTH / ct_image_get_channel_subsampling_x(dst_dummy, channel_ref + i);
        int h = HEIGHT / ct_image_get_channel_subsampling_y(dst_dummy, channel_ref + i);
        ASSERT_NO_FAILURE(src1[i] = image_generate_random(w, h, VX_DF_IMAGE_U8));
        ASSERT_VX_OBJECT(src_image[i] = ct_image_to_vx_image(src1[i], context), VX_TYPE_IMAGE);
    }

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, 0, 0, arg_->format), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image0 = vxCreateImage(context, dst_ref0->width, dst_ref0->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image1 = vxCreateImage(context, dst_ref1->width, dst_ref1->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node1 = vxChannelCombineNode(graph, src_image[0], src_image[1], src_image[2], src_image[3], virt), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxChannelExtractNode(graph, virt, arg_->channel0, dst_image0), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxChannelExtractNode(graph, virt, arg_->channel1, dst_image1), VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    for (i = 0; i < channels; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
        ASSERT(src_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseImage(&dst_image0));
    VX_CALL(vxReleaseImage(&dst_image1));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    if (arg_->format == VX_DF_IMAGE_RGB)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "ChannelCombine-ChannelExtract-ChannelExtract (3)", "RGB->U8->U8", 0);
    else if (arg_->format == VX_DF_IMAGE_RGBX)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "ChannelCombine-ChannelExtract-ChannelExtract (3)", "RGBX->U8->U8", 0);
    else if (arg_->format == VX_DF_IMAGE_NV12)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "ChannelCombine-ChannelExtract-ChannelExtract (3)", "YUYV->U8->U8", 0);
    else if (arg_->format == VX_DF_IMAGE_NV21)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "ChannelCombine-ChannelExtract-ChannelExtract (3)", "UYVY->U8->U8", 0);
    else if (arg_->format == VX_DF_IMAGE_UYVY)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "ChannelCombine-ChannelExtract-ChannelExtract (3)", "NV12->U8->U8", 0);
    else if (arg_->format == VX_DF_IMAGE_YUYV)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "ChannelCombine-ChannelExtract-ChannelExtract (3)", "NV21->U8->U8", 0);
    else if (arg_->format == VX_DF_IMAGE_IYUV)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "ChannelCombine-ChannelExtract-ChannelExtract (3)", "YUV4->U8->U8", 0);
    else if (arg_->format == VX_DF_IMAGE_YUV4)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "ChannelCombine-ChannelExtract-ChannelExtract (3)", "IYUV->U8->U8", 0);
}

TEST(tiovxSupernodePerformance, tiovxSupernodePerfConvertDepth)
{
    int node_count = 3;
    vx_image src, dst, virt1, virt2;
    vx_graph graph;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_scalar scalar_shift;
    vx_int32 shift = 2;
    vx_int32 tmp = 0;
    vx_context context = context_->vx_context_;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(src = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src, &CT()->seed_));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(scalar_shift = vxCreateScalar(context, VX_TYPE_INT32, &tmp), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(node1 = vxConvertDepthNode(graph, src, virt1, VX_CONVERT_POLICY_SATURATE, scalar_shift), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxConvertDepthNode(graph, virt1, virt2, VX_CONVERT_POLICY_SATURATE, scalar_shift), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxConvertDepthNode(graph, virt2, dst, VX_CONVERT_POLICY_SATURATE, scalar_shift), VX_TYPE_NODE);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxCopyScalar(scalar_shift, &shift, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));


    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseScalar(&scalar_shift));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
                       WIDTH, HEIGHT, "ConvertDepth-ConvertDepth-ConvertDepth (3)", \
                       "U8=>S16=>U8=>S16", 0);
}

TEST_WITH_ARG(tiovxSupernodePerformance2, tiovxSupernodePerfConvolve, Convolve_Arg,
    CT_GENERATE_PARAMETERS("lena", ADD_CONV_SIZE, ADD_CONV_SCALE, ADD_CONV_GENERATORS, ADD_CONV_DST_FORMAT, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ARG, read_image, "lena.bmp")
)
{
    int node_count = 3;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt1, virt2;
    vx_convolution convolution1 = 0, convolution2 = 0, convolution3 = 0;
    vx_int16 data1[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 }, data2[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 };
    vx_int16 data3[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 };
    vx_size conv_max_dim = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, src->width, src->height, arg_->dst_format), VX_TYPE_IMAGE);

    VX_CALL(vxQueryContext(context, VX_CONTEXT_CONVOLUTION_MAX_DIMENSION, &conv_max_dim, sizeof(conv_max_dim)));

    if ((vx_size)arg_->cols > conv_max_dim || (vx_size)arg_->rows > conv_max_dim)
    {
        printf("%dx%d convolution is not supported. Skip test\n", (int)arg_->cols, (int)arg_->rows);
        return;
    }

    ASSERT_NO_FAILURE(arg_->convolution_data_generator(arg_->cols, arg_->rows, data1));
    ASSERT_NO_FAILURE(convolution1 = convolution_create(context, arg_->cols, arg_->rows, data1, arg_->scale));

    ASSERT_NO_FAILURE(arg_->convolution_data_generator(arg_->cols, arg_->rows, data2));
    ASSERT_NO_FAILURE(convolution2 = convolution_create(context, arg_->cols, arg_->rows, data2, arg_->scale));

    ASSERT_NO_FAILURE(arg_->convolution_data_generator(arg_->cols, arg_->rows, data2));
    ASSERT_NO_FAILURE(convolution3 = convolution_create(context, arg_->cols, arg_->rows, data3, arg_->scale));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxConvolveNode(graph, src_image, convolution1, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxConvolveNode(graph, virt1, convolution2, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxConvolveNode(graph, virt2, convolution3, dst_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node3, VX_NODE_BORDER, &border, sizeof(border)));

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(super_node == 0);
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    VX_CALL(vxReleaseConvolution(&convolution1));
    VX_CALL(vxReleaseConvolution(&convolution2));
    VX_CALL(vxReleaseConvolution(&convolution3));
    ASSERT(convolution1 == NULL);
    ASSERT(convolution2 == NULL);
    ASSERT(convolution3 == NULL);

    if ( (arg_->dst_format == VX_DF_IMAGE_U8) && (arg_->cols == 3) && (arg_->rows == 3) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, src->width, src->height, "Convolve3x3-Convolve3x3-Convolve3x3 (3)", "U8=>U8=>U8=>U8", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_S16) && (arg_->cols == 3) && (arg_->rows == 3) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, src->width, src->height, "Convolve3x3-Convolve3x3-Convolve3x3 (3)", "U8=>U8=>U8=>S16", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_U8) && (arg_->cols == 5) && (arg_->rows == 5) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, src->width, src->height, "Convolve5x5-Convolve5x5-Convolve5x5 (3)", "U8=>U8=>U8=>U8", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_S16) && (arg_->cols == 5) && (arg_->rows == 5) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, src->width, src->height, "Convolve5x5-Convolve5x5-Convolve5x5 (3)", "U8=>U8=>U8=>S16", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_U8) && (arg_->cols == 7) && (arg_->rows == 7) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, src->width, src->height, "Convolve7x7-Convolve7x7-Convolve7x7 (3)", "U8=>U8=>U8=>U8", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_S16) && (arg_->cols == 7) && (arg_->rows == 7) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, src->width, src->height, "Convolve7x7-Convolve7x7-Convolve7x7 (3)", "U8=>U8=>U8=>S16", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_U8) && (arg_->cols == 9) && (arg_->rows == 9) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, src->width, src->height, "Convolve9x9-Convolve9x9-Convolve9x9 (3)", "U8=>U8=>U8=>U8", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_S16) && (arg_->cols == 9) && (arg_->rows == 9) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, src->width, src->height, "Convolve9x9-Convolve9x9-Convolve9x9 (3)", "U8=>U8=>U8=>S16", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_U8) && (arg_->cols == 9) && (arg_->rows == 3) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, src->width, src->height, "Convolve9x3-Convolve9x3-Convolve9x3 (3)", "U8=>U8=>U8=>U8", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_S16) && (arg_->cols == 9) && (arg_->rows == 3) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, src->width, src->height, "Convolve9x3-Convolve9x3-Convolve9x3 (3)", "U8=>U8=>U8=>S16", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_U8) && (arg_->cols == 3) && (arg_->rows == 9) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, src->width, src->height, "Convolve3x9-Convolve3x9-Convolve3x9 (3)", "U8=>U8=>U8=>U8", 0);
    else if ( (arg_->dst_format == VX_DF_IMAGE_S16) && (arg_->cols == 3) && (arg_->rows == 9) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, src->width, src->height, "Convolve3x9-Convolve3x9-Convolve3x9 (3)", "U8=>U8=>U8=>S16", 0);

}

TEST(tiovxSupernodePerformance2, tiovxSupernodePerfGaussian3x3)
{
    int node_count = 3;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt1, virt2;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    vx_rectangle_t rect;
    vx_bool valid_rect;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_border_t border;

    border.mode = VX_BORDER_UNDEFINED;
    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src_image, &CT()->seed_));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxGaussian3x3Node(graph, src_image, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxGaussian3x3Node(graph, virt1, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxGaussian3x3Node(graph, virt2, dst_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node3, VX_NODE_BORDER, &border, sizeof(border)));

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Gaussian3x3 (3)", "U8=>U8", 0);
}

TEST(tiovxSupernodePerformance2, tiovxSupernodePerfHistogram)
{
    vx_image src;
    vx_image virt = 0;
    CT_Image src_ref;
    vx_context context = context_->vx_context_;
    vx_distribution dist1, dist2;
    uint64_t rng;
    int val0, val1, offset, nbins, range;
    int node_count = 3;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    rng = CT()->seed_;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    val0 = CT_RNG_NEXT_INT(rng, 0, (MAX_HISTOGRAMS_BINS-1)), val1 = CT_RNG_NEXT_INT(rng, 0, (MAX_HISTOGRAMS_BINS-1));
    offset = CT_MIN(val0, val1), range = CT_MAX(val0, val1) - offset + 1;
    nbins = CT_RNG_NEXT_INT(rng, 1, range+1);

    ASSERT_NO_FAILURE(src_ref = ct_allocate_ct_image_random(WIDTH, HEIGHT, VX_DF_IMAGE_U8, &rng, 0, 256));

    src = ct_image_to_vx_image(src_ref, context);
    ASSERT_VX_OBJECT(src, VX_TYPE_IMAGE);

    dist1 = vxCreateDistribution(context, nbins, offset, range);
    ASSERT_VX_OBJECT(dist1, VX_TYPE_DISTRIBUTION);
    dist2 = vxCreateDistribution(context, nbins, offset, range);
    ASSERT_VX_OBJECT(dist2, VX_TYPE_DISTRIBUTION);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxNotNode(graph, src, virt), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxHistogramNode(graph, virt, dist1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxHistogramNode(graph, virt, dist2), VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseDistribution(&dist1));
    VX_CALL(vxReleaseDistribution(&dist2));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
                       WIDTH, HEIGHT, "Not-Histogram-Histogram (3)", \
                       "U8=>U8=>U8", 0);
}

TEST(tiovxSupernodePerformance2, tiovxSupernodePerfIntegralImg)
{
    vx_image src, dst1, dst2;
    vx_image virt = 0;
    vx_context context = context_->vx_context_;
    int node_count = 3;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(src = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src, &CT()->seed_));

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node1 = vxNotNode(graph, src, virt), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxIntegralImageNode(graph, virt, dst1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxIntegralImageNode(graph, virt, dst2), VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseImage(&dst1));
    VX_CALL(vxReleaseImage(&dst2));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
                       WIDTH, HEIGHT, "Not-IntegralImg-IntegralImg (3)", \
                       "U8=>U32=>U32", 0);
}

TEST_WITH_ARG(tiovxSupernodePerformance2, tiovxSupernodePerfLUT, Lut_Arg,
    CT_GENERATE_PARAMETERS("lena", ADD_LUT_GENERATOR, ADD_SIZE_NONE, ADD_TYPE, ARG, lut_image_read, "lena.bmp")
)
{
    vx_enum data_type = arg_->data_type;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt1, virt2;
    int node_count = 3;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    CT_Image src = NULL, dst = NULL;
    void* lut1_data;
    void* lut2_data;
    void* lut3_data;
    vx_lut lut1, lut2, lut3;
    vx_size size;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    size = lut_size(data_type);
    lut1_data = ct_alloc_mem(size);
    ASSERT(lut1_data != 0);
    lut2_data = ct_alloc_mem(size);
    ASSERT(lut2_data != 0);
    lut3_data = ct_alloc_mem(size);
    ASSERT(lut3_data != 0);

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height, data_type));
    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(arg_->lut_generator(lut1_data, data_type));
    ASSERT_VX_OBJECT(lut1 = lut_create(context, lut1_data, data_type), VX_TYPE_LUT);

    ASSERT_NO_FAILURE(arg_->lut_generator(lut2_data, data_type));
    ASSERT_VX_OBJECT(lut2 = lut_create(context, lut2_data, data_type), VX_TYPE_LUT);

    ASSERT_NO_FAILURE(arg_->lut_generator(lut3_data, data_type));
    ASSERT_VX_OBJECT(lut3 = lut_create(context, lut3_data, data_type), VX_TYPE_LUT);

    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);
    virt1 = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(virt1, VX_TYPE_IMAGE);
    virt2 = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(virt2, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node1 = vxTableLookupNode(graph, src_image, lut1, virt1);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
    node2 = vxTableLookupNode(graph, virt1, lut2, virt2);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);
    node3 = vxTableLookupNode(graph, virt2, lut3, dst_image);
    ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseLUT(&lut1));
    VX_CALL(vxReleaseLUT(&lut2));
    VX_CALL(vxReleaseLUT(&lut3));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));

    ASSERT(lut1 == 0);
    ASSERT(lut2 == 0);
    ASSERT(lut3 == 0);
    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
    ASSERT(virt1 == 0);
    ASSERT(virt2 == 0);

    ct_free_mem(lut1_data);
    ct_free_mem(lut2_data);
    ct_free_mem(lut3_data);

    if (data_type == VX_TYPE_UINT8)
    {
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
                       WIDTH, HEIGHT, "LookUpTable-LookUpTable-LookUpTable (3)", "U8=>U8=>U8", 0);
    }
    else
    {
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
                       WIDTH, HEIGHT, "LookUpTable-LookUpTable-LookUpTable (3)", "S16=>S16=>S16", 0);
    }
}

TEST(tiovxSupernodePerformance2, tiovxSupernodePerfMeanStdDev)
{
    int format = VX_DF_IMAGE_U8;
    vx_image src, virt;
    CT_Image src0;
    int node_count = 3;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_scalar mean_s0, stddev_s0;
    vx_scalar mean_s1, stddev_s1;
    vx_context context = context_->vx_context_;
    uint64_t rng;
    vx_float32 mean0 = 0.f, stddev0 = 0.f, mean = 0.f, stddev = 0.f;
    int a = 0, b = 256;

    rng = CT()->seed_;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    mean_s0 = vxCreateScalar(context, VX_TYPE_FLOAT32, &mean0);
    ASSERT_VX_OBJECT(mean_s0, VX_TYPE_SCALAR);
    stddev_s0 = vxCreateScalar(context, VX_TYPE_FLOAT32, &stddev0);
    ASSERT_VX_OBJECT(stddev_s0, VX_TYPE_SCALAR);
    mean_s1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &mean);
    ASSERT_VX_OBJECT(mean_s1, VX_TYPE_SCALAR);
    stddev_s1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &stddev);
    ASSERT_VX_OBJECT(stddev_s1, VX_TYPE_SCALAR);

    src0 = ct_allocate_ct_image_random(WIDTH, HEIGHT, format, &rng, a, b);
    src = ct_image_to_vx_image(src0, context);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node1 = vxNotNode(graph, src, virt), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxMeanStdDevNode(graph, virt, mean_s0, stddev_s0), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxMeanStdDevNode(graph, virt, mean_s1, stddev_s1), VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&virt));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    CT_CollectGarbage(CT_GC_IMAGE);

    VX_CALL(vxReleaseScalar(&mean_s0));
    VX_CALL(vxReleaseScalar(&stddev_s0));
    VX_CALL(vxReleaseScalar(&mean_s1));
    VX_CALL(vxReleaseScalar(&stddev_s1));

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
                       WIDTH, HEIGHT, "Not-MeanStandardDev-MeanStandardDev (3)", \
                       "U8", 0);
}

TEST(tiovxSupernodePerformance2, tiovxSupernodePerfMedian3x3)
{
    int node_count = 3;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt1, virt2;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    vx_rectangle_t rect;
    vx_bool valid_rect;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_border_t border;

    border.mode = VX_BORDER_UNDEFINED;
    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src_image, &CT()->seed_));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxMedian3x3Node(graph, src_image, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxMedian3x3Node(graph, virt1, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxMedian3x3Node(graph, virt2, dst_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node3, VX_NODE_BORDER, &border, sizeof(border)));

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
                       WIDTH, HEIGHT, "Median3x3 (3)", "U8=>U8", 0);
}

TEST(tiovxSupernodePerformance2, tiovxSupernodePerfMagnitude)
{
    int node_count = 3;
    int dxformat = VX_DF_IMAGE_S16;
    int srcformat = dxformat;
    int magformat = dxformat;
    vx_image dx_node1=0, dy_node1=0, dx_node2=0, dy_node2=0, mag=0, virt1, virt2;
    CT_Image dx0, dy0, dx1, dy1, mag0, mag1, virt_ctimage1, virt_ctimage2;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_context context = context_->vx_context_;
    uint64_t rng;
    int dxmin = -32768, dxmax = 32768;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    rng = CT()->seed_;

    int width = WIDTH;
    int height = HEIGHT;

    ASSERT_NO_FAILURE(dx0 = ct_allocate_ct_image_random(width, height, dxformat, &rng, dxmin, dxmax));
    ASSERT_NO_FAILURE(dy0 = ct_allocate_ct_image_random(width, height, dxformat, &rng, dxmin, dxmax));
    ASSERT_NO_FAILURE(dx1 = ct_allocate_ct_image_random(width, height, dxformat, &rng, dxmin, dxmax));
    ASSERT_NO_FAILURE(dy1 = ct_allocate_ct_image_random(width, height, dxformat, &rng, dxmin, dxmax));

    dx_node1 = ct_image_to_vx_image(dx0, context);
    ASSERT_VX_OBJECT(dx_node1, VX_TYPE_IMAGE);
    dx_node2 = ct_image_to_vx_image(dx1, context);
    ASSERT_VX_OBJECT(dx_node2, VX_TYPE_IMAGE);
    dy_node1 = ct_image_to_vx_image(dy0, context);
    ASSERT_VX_OBJECT(dy_node1, VX_TYPE_IMAGE);
    dy_node2 = ct_image_to_vx_image(dy1, context);
    ASSERT_VX_OBJECT(dy_node2, VX_TYPE_IMAGE);

    mag = vxCreateImage(context, width, height, magformat);
    ASSERT_VX_OBJECT(mag, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, dxformat), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, dxformat), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    node1 = vxMagnitudeNode(graph, dx_node1, dy_node1, virt1);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
    node2 = vxMagnitudeNode(graph, dx_node2, dy_node2, virt2);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);
    node3 = vxMagnitudeNode(graph, virt1, virt2, mag);
    ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    VX_CALL(vxReleaseImage(&dx_node1));
    VX_CALL(vxReleaseImage(&dx_node2));
    VX_CALL(vxReleaseImage(&dy_node1));
    VX_CALL(vxReleaseImage(&dy_node2));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&mag));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(super_node == 0 && node1 == 0 && node2 == 0 && node3 == 0 && graph == 0);
    CT_CollectGarbage(CT_GC_IMAGE);

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
                       width, height, "Magnitude-Magnitude-Magnitude (3)", "(S16+S16)+(S16+S16)=>S16", 0);
}

typedef struct {
    const char* name;
    vx_df_image format;
} minmax_arg;

#define MINMAXLOC_FUZZY_ARGS(imm, fmt, kernel1, kernel2) \
    {#imm "/" #kernel1 "(" #fmt ")-" #kernel2 "-" #kernel2, VX_DF_IMAGE_##fmt}

TEST_WITH_ARG(tiovxSupernodePerformance2, tiovxSupernodePerfMinMaxLoc, minmax_arg, \
              MINMAXLOC_FUZZY_ARGS(Graph, U8, Phase, MinMaxLoc), \
              MINMAXLOC_FUZZY_ARGS(Graph, S16, Sobel, MinMaxLoc)
)
{
    const int MAX_CAP = 300;
    vx_image src1, src2, virt1, virt2;
    vx_graph graph = 0;
    vx_context context = context_->vx_context_;
    uint64_t rng;
    int a, b;
    vx_scalar minval_1, maxval_1, mincount_1, maxcount_1;
    vx_array minloc_1 = 0, maxloc_1 = 0;
    vx_scalar minval_2, maxval_2, mincount_2, maxcount_2;
    vx_array minloc_2 = 0, maxloc_2 = 0;
    vx_enum sctype = arg_->format == VX_DF_IMAGE_U8 ? VX_TYPE_UINT8 :
                     arg_->format == VX_DF_IMAGE_S16 ? VX_TYPE_INT16 :
                     VX_TYPE_INT32;
    int node_count = 3;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    a = 0, b = 256;

    minval_1 = ct_scalar_from_int(context, sctype, 0);
    maxval_1 = ct_scalar_from_int(context, sctype, 0);
    mincount_1 = ct_scalar_from_int(context, VX_TYPE_UINT32, 0);
    maxcount_1 = ct_scalar_from_int(context, VX_TYPE_UINT32, 0);
    minloc_1 = vxCreateArray(context, VX_TYPE_COORDINATES2D, MAX_CAP);
    maxloc_1 = vxCreateArray(context, VX_TYPE_COORDINATES2D, MAX_CAP);
    ASSERT(vxGetStatus((vx_reference)minloc_1) == VX_SUCCESS &&
           vxGetStatus((vx_reference)maxloc_1) == VX_SUCCESS);

    minval_2 = ct_scalar_from_int(context, sctype, 0);
    maxval_2 = ct_scalar_from_int(context, sctype, 0);
    mincount_2 = ct_scalar_from_int(context, VX_TYPE_UINT32, 0);
    maxcount_2 = ct_scalar_from_int(context, VX_TYPE_UINT32, 0);
    minloc_2 = vxCreateArray(context, VX_TYPE_COORDINATES2D, MAX_CAP);
    maxloc_2 = vxCreateArray(context, VX_TYPE_COORDINATES2D, MAX_CAP);
    ASSERT(vxGetStatus((vx_reference)minloc_2) == VX_SUCCESS &&
           vxGetStatus((vx_reference)maxloc_2) == VX_SUCCESS);

    rng = CT()->seed_;

    int return_loc1 = CT_RNG_NEXT_INT(rng, 0, 2);
    int return_count1 = CT_RNG_NEXT_INT(rng, 0, 2);
    int return_loc2 = CT_RNG_NEXT_INT(rng, 0, 2);
    int return_count2 = CT_RNG_NEXT_INT(rng, 0, 2);
    int width, height;

    width = WIDTH;
    height = HEIGHT;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    if (arg_->format == VX_DF_IMAGE_S16)
    {
        ASSERT_VX_OBJECT(src1 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
        ASSERT_VX_OBJECT(virt1 = vxCreateVirtualImage(graph, 0, 0, arg_->format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(virt2 = vxCreateVirtualImage(graph, 0, 0, arg_->format), VX_TYPE_IMAGE);

        node1 = vxSobel3x3Node(graph, src1, virt1, virt2);
        ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
        node2 = vxMinMaxLocNode(graph, virt1, minval_1, maxval_1,
                               return_loc1 ? minloc_1 : 0,
                               return_loc1 ? maxloc_1 : 0,
                               return_count1 ? mincount_1 : 0,
                               return_count1 ? maxcount_1 : 0);
        ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);
        node3 = vxMinMaxLocNode(graph, virt2, minval_2, maxval_2,
                               return_loc2 ? minloc_2 : 0,
                               return_loc2 ? maxloc_2 : 0,
                               return_count2 ? mincount_2 : 0,
                               return_count2 ? maxcount_2 : 0);
        ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);
    }
    else if (arg_->format == VX_DF_IMAGE_U8)
    {
        ASSERT_VX_OBJECT(src1 = vxCreateImage(context, width, height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src2 = vxCreateImage(context, width, height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
        ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
        ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
        ASSERT_VX_OBJECT(virt1 = vxCreateVirtualImage(graph, 0, 0, arg_->format), VX_TYPE_IMAGE);

        node1 = vxPhaseNode(graph, src1, src2, virt1);
        ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
        node2 = vxMinMaxLocNode(graph, virt1, minval_1, maxval_1,
                               return_loc1 ? minloc_1 : 0,
                               return_loc1 ? maxloc_1 : 0,
                               return_count1 ? mincount_1 : 0,
                               return_count1 ? maxcount_1 : 0);
        ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);
        node3 = vxMinMaxLocNode(graph, virt1, minval_2, maxval_2,
                               return_loc2 ? minloc_2 : 0,
                               return_loc2 ? maxloc_2 : 0,
                               return_count2 ? mincount_2 : 0,
                               return_count2 ? maxcount_2 : 0);
        ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);
    }

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseImage(&src1));
    if (arg_->format == VX_DF_IMAGE_U8)
        VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&virt1));
    if (arg_->format == VX_DF_IMAGE_S16)
        VX_CALL(vxReleaseImage(&virt2));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    CT_CollectGarbage(CT_GC_IMAGE);

    VX_CALL(vxReleaseScalar(&minval_1));
    VX_CALL(vxReleaseScalar(&maxval_1));
    VX_CALL(vxReleaseScalar(&mincount_1));
    VX_CALL(vxReleaseScalar(&maxcount_1));
    VX_CALL(vxReleaseArray(&minloc_1));
    VX_CALL(vxReleaseArray(&maxloc_1));
    VX_CALL(vxReleaseScalar(&minval_2));
    VX_CALL(vxReleaseScalar(&maxval_2));
    VX_CALL(vxReleaseScalar(&mincount_2));
    VX_CALL(vxReleaseScalar(&maxcount_2));
    VX_CALL(vxReleaseArray(&minloc_2));
    VX_CALL(vxReleaseArray(&maxloc_2));

    if (arg_->format == VX_DF_IMAGE_U8)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
            width, height, "Phase-MinMaxLoc-MinMaxLoc (3)", "S16+S16=>U8", 0);
    else if (arg_->format == VX_DF_IMAGE_S16)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
            width, height, "Sobel-MinMaxLoc-MinMaxLoc (3)", "U8=>S16+S16", 0);
}

typedef struct {
    const char* name;
    enum vx_convert_policy_e overflow_policy;
    int width, height;
    vx_df_image arg1_format, arg2_format, result_format;
    enum vx_round_policy_e round_policy;
    vx_float32 scale;
} multiply_fuzzy_arg;

#define MUL_FUZZY_ARGS_(owp, w, h, f1, f2, fr, rp, scale)  \
    ARG(#owp "/" #rp " " #w "x" #h " " #f1 "*" #f2 "*" scale##_STR "=" #fr, \
        VX_CONVERT_POLICY_##owp, w, h, VX_DF_IMAGE_##f1, VX_DF_IMAGE_##f2, \
        VX_DF_IMAGE_##fr, VX_ROUND_POLICY_##rp, scale)

#define MULT_FUZZY_ARGS(owp)  \
    MUL_FUZZY_ARGS_(owp, WIDTH, HEIGHT, U8, U8, U8, TO_ZERO, ONE_2_0),   \
    MUL_FUZZY_ARGS_(owp, WIDTH, HEIGHT, U8, U8, S16, TO_ZERO, ONE_2_0),  \
    MUL_FUZZY_ARGS_(owp, WIDTH, HEIGHT, S16, S16, S16, TO_ZERO, ONE_2_0),\
    MUL_FUZZY_ARGS_(owp, WIDTH, HEIGHT, U8, S16, S16, TO_ZERO, ONE_2_0)

TEST_WITH_ARG(tiovxSupernodePerformance2, tiovxSupernodePerfMultiply,
                multiply_fuzzy_arg, MULT_FUZZY_ARGS(WRAP))
{
    int node_count = 3;
    vx_image src1, src2, src3, src4, dst, virt1, virt2;
    vx_graph graph;
    vx_scalar scale = 0;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_super_node, perf_graph;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateImage(context, arg_->width, arg_->height, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateImage(context, arg_->width, arg_->height, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->scale), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg2_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg2_format),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4, &CT()->seed_));

    // build one-node graph
    ASSERT_VX_OBJECT(node1 = vxMultiplyNode(graph, src1, src2, scale, arg_->overflow_policy, arg_->round_policy, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxMultiplyNode(graph, src3, src4, scale, arg_->overflow_policy, arg_->round_policy, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxMultiplyNode(graph, virt1, virt2, scale, arg_->overflow_policy, arg_->round_policy, dst), VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&src4));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseScalar(&scale));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    if ( (arg_->arg1_format == VX_DF_IMAGE_U8) && (arg_->arg2_format == VX_DF_IMAGE_U8) && (arg_->result_format == VX_DF_IMAGE_U8) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Multiply (3)", "U8+U8=U8", 0);
    else if ( (arg_->arg1_format == VX_DF_IMAGE_U8) && (arg_->arg2_format == VX_DF_IMAGE_U8) && (arg_->result_format == VX_DF_IMAGE_S16) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Multiply (3)", "U8+U8=S16", 0);
    else if ( (arg_->arg1_format == VX_DF_IMAGE_U8) && (arg_->arg2_format == VX_DF_IMAGE_S16) && (arg_->result_format == VX_DF_IMAGE_S16) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Multiply (3)", "U8+S16=S16", 0);
    else if ( (arg_->arg1_format == VX_DF_IMAGE_S16) && (arg_->arg2_format == VX_DF_IMAGE_S16) && (arg_->result_format == VX_DF_IMAGE_S16) )
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, WIDTH, HEIGHT, "Multiply (3)", "S16+S16=S16", 0);
}

typedef struct {
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    const char* fileName;
    vx_size mask_width;
    vx_size mask_height;
    vx_enum function;
    vx_enum pattern;
    vx_border_t border;
    int width, height;
} nonlinear_filter_arg;


#define NONLINEAR_FILTER_FUNCTIONS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_NONLINEAR_FILTER_MIN", __VA_ARGS__, VX_NONLINEAR_FILTER_MIN)), \
    CT_EXPAND(nextmacro(testArgName "/VX_NONLINEAR_FILTER_MAX", __VA_ARGS__, VX_NONLINEAR_FILTER_MAX)), \
    CT_EXPAND(nextmacro(testArgName "/VX_NONLINEAR_FILTER_MEDIAN", __VA_ARGS__, VX_NONLINEAR_FILTER_MEDIAN))

#define NONLINEAR_FILTER_PATTERNS_BOX_CROSS_DISK(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_PATTERN_BOX", __VA_ARGS__, VX_PATTERN_BOX)), \
    CT_EXPAND(nextmacro(testArgName "/VX_PATTERN_CROSS", __VA_ARGS__, VX_PATTERN_CROSS)), \
    CT_EXPAND(nextmacro(testArgName "/VX_PATTERN_DISK", __VA_ARGS__, VX_PATTERN_DISK))

#define NONLINEAR_FILTER_PATTERNS_BOX_CROSS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_PATTERN_BOX", __VA_ARGS__, VX_PATTERN_BOX)), \
    CT_EXPAND(nextmacro(testArgName "/VX_PATTERN_CROSS", __VA_ARGS__, VX_PATTERN_CROSS))

/*
#define NONLINEAR_FILTER_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput/mask=3x5", NONLINEAR_FILTER_FUNCTIONS, NONLINEAR_FILTER_PATTERNS_BOX_CROSS,\
     ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 3, 5), \
    CT_GENERATE_PARAMETERS("randomInput/mask=3x7", NONLINEAR_FILTER_FUNCTIONS, NONLINEAR_FILTER_PATTERNS_BOX_CROSS,\
     ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 3, 7), \
    CT_GENERATE_PARAMETERS("randomInput/mask=3x9", NONLINEAR_FILTER_FUNCTIONS, NONLINEAR_FILTER_PATTERNS_BOX_CROSS,\
     ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 3, 9), \
    CT_GENERATE_PARAMETERS("randomInput/mask=5x3", NONLINEAR_FILTER_FUNCTIONS, NONLINEAR_FILTER_PATTERNS_BOX_CROSS,\
     ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 5, 3), \
    CT_GENERATE_PARAMETERS("randomInput/mask=5x7", NONLINEAR_FILTER_FUNCTIONS, NONLINEAR_FILTER_PATTERNS_BOX_CROSS,\
     ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 5, 7), \
    CT_GENERATE_PARAMETERS("randomInput/mask=5x9", NONLINEAR_FILTER_FUNCTIONS, NONLINEAR_FILTER_PATTERNS_BOX_CROSS,\
     ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 5, 9), \
    CT_GENERATE_PARAMETERS("randomInput/mask=7x3", NONLINEAR_FILTER_FUNCTIONS, NONLINEAR_FILTER_PATTERNS_BOX_CROSS,\
     ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 7, 3), \
    CT_GENERATE_PARAMETERS("randomInput/mask=7x5", NONLINEAR_FILTER_FUNCTIONS, NONLINEAR_FILTER_PATTERNS_BOX_CROSS,\
     ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 7, 5), \
    CT_GENERATE_PARAMETERS("randomInput/mask=7x7", NONLINEAR_FILTER_FUNCTIONS, NONLINEAR_FILTER_PATTERNS_BOX_CROSS,\
     ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 7, 7), \
    CT_GENERATE_PARAMETERS("randomInput/mask=7x9", NONLINEAR_FILTER_FUNCTIONS, NONLINEAR_FILTER_PATTERNS_BOX_CROSS,\
     ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 7, 9), \
    CT_GENERATE_PARAMETERS("randomInput/mask=9x3", NONLINEAR_FILTER_FUNCTIONS, NONLINEAR_FILTER_PATTERNS_BOX_CROSS,\
     ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 9, 3), \
    CT_GENERATE_PARAMETERS("randomInput/mask=9x5", NONLINEAR_FILTER_FUNCTIONS, NONLINEAR_FILTER_PATTERNS_BOX_CROSS,\
     ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 9, 5), \
    CT_GENERATE_PARAMETERS("randomInput/mask=9x7", NONLINEAR_FILTER_FUNCTIONS, NONLINEAR_FILTER_PATTERNS_BOX_CROSS,\
     ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 9, 7), \
    CT_GENERATE_PARAMETERS("randomInput/mask=9x9", NONLINEAR_FILTER_FUNCTIONS, NONLINEAR_FILTER_PATTERNS_BOX_CROSS,\
     ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 9, 9), \
    CT_GENERATE_PARAMETERS("randomInput/mask=5x5", NONLINEAR_FILTER_FUNCTIONS, NONLINEAR_FILTER_PATTERNS_BOX_CROSS_DISK,\
     ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 5, 5)
*/
#define NONLINEAR_FILTER_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput/mask=5x5", NONLINEAR_FILTER_FUNCTIONS, NONLINEAR_FILTER_PATTERNS_BOX_CROSS_DISK,\
     ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, convolve_generate_random, NULL, 5, 5)

TEST_WITH_ARG(tiovxSupernodePerformance2, tiovxSupernodePerfNonLinearFilter, nonlinear_filter_arg,
    NONLINEAR_FILTER_PARAMETERS
    )
{
    int node_count = 3;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt1 = 0, virt2 = 0;
    vx_matrix mask = 0;
    vx_graph graph = 0;
    vx_enum pattern = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    CT_Image src0 = NULL;
    vx_border_t border = arg_->border;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(src0 = arg_->generator(NULL, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src0, context), VX_TYPE_IMAGE);
    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    mask = vxCreateMatrixFromPattern(context, arg_->pattern, arg_->mask_width, arg_->mask_height);
    ASSERT_VX_OBJECT(mask, VX_TYPE_MATRIX);
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_PATTERN, &pattern, sizeof(pattern)));
    ASSERT_EQ_INT(arg_->pattern, pattern);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    node1 = vxNonLinearFilterNode(graph, arg_->function, src_image, mask, virt1);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
    node2 = vxNonLinearFilterNode(graph, arg_->function, virt1, mask, virt2);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);
    node3 = vxNonLinearFilterNode(graph, arg_->function, virt2, mask, dst_image);
    ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node3, VX_NODE_BORDER, &border, sizeof(border)));

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseMatrix(&mask));
    VX_CALL(vxReleaseImage(&src_image));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst_image));

    ASSERT(mask == 0);
    ASSERT(src_image == 0);
    ASSERT(virt1 == 0);
    ASSERT(virt2 == 0);
    ASSERT(dst_image == 0);

    if (arg_->function == VX_NONLINEAR_FILTER_MIN)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
            arg_->width, arg_->height, "NonLinear_Filter (3) - mask=5x5; Erosion", "U8=>U8=>U8=>U8", 0);
    else if (arg_->function == VX_NONLINEAR_FILTER_MAX)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
            arg_->width, arg_->height, "NonLinear_Filter (3) - mask=5x5; Dilation", "U8=>U8=>U8=>U8", 0);
    else if (arg_->function == VX_NONLINEAR_FILTER_MEDIAN)
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
            arg_->width, arg_->height, "NonLinear_Filter (3) - mask=5x5; Median", "U8=>U8=>U8=>U8", 0);
}

TEST(tiovxSupernodePerformance2, tiovxSupernodePerfPhase)
{
    vx_image src1, src2, virt1, virt2, virt3, dst;
    vx_graph graph = 0;
    vx_context context = context_->vx_context_;
    uint64_t rng;
    int a, b;

    int node_count = 3;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    a = 0, b = 256;

    rng = CT()->seed_;

    int width, height;
    width = WIDTH;
    height = HEIGHT;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, width, height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, width, height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
    ASSERT_VX_OBJECT(virt1 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt3 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxPhaseNode(graph, src1, src2, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxSobel3x3Node(graph, virt1, virt2, virt3), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxPhaseNode(graph, virt2, virt3, dst), VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&virt3));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    CT_CollectGarbage(CT_GC_IMAGE);

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
        width, height, "Phase-Sobel3x3-Phase (3)", "S16+S16=>U8", 0);
}

TEST(tiovxSupernodePerformance2, tiovxSupernodePerfSobel3x3)
{
    vx_image src, virt1, virt2, virt3, dst1, dst2;
    vx_graph graph = 0;
    vx_context context = context_->vx_context_;
    uint64_t rng;
    int a, b;

    int node_count = 3;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    a = 0, b = 256;

    rng = CT()->seed_;

    int width, height;
    width = WIDTH;
    height = HEIGHT;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(src = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst1 = vxCreateImage(context, width, height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst2 = vxCreateImage(context, width, height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src, &CT()->seed_));
    ASSERT_VX_OBJECT(virt1 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt3 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxSobel3x3Node(graph, src, virt1, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxPhaseNode(graph, virt1, virt2, virt3), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxSobel3x3Node(graph, virt3, dst1, dst2), VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst1));
    VX_CALL(vxReleaseImage(&dst2));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&virt3));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    CT_CollectGarbage(CT_GC_IMAGE);

    PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
        width, height, "Sobel3x3-Phase-Sobel3x3 (3)", "U8=>S16+S16", 0);
}

TEST_WITH_ARG(tiovxSupernodePerformance2, tiovxSupernodePerfThreshold, threshold_arg,
              THRESHOLD_CASE(Graph, BINARY),
              )
{
    int format = VX_DF_IMAGE_U8;
    int ttype = arg_->ttype;
    vx_image src, dst, virt1, virt2;
    vx_threshold vxt1, vxt2, vxt3;
    CT_Image ref_src;
    int node_count = 3;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_context context = context_->vx_context_;
    uint64_t rng;
    int a = 0, b = 256;
    int true_val = CT_THRESHOLD_TRUE_VALUE;
    int false_val = CT_THRESHOLD_FALSE_VALUE;

    rng = CT()->seed_;

    int width, height;

    vx_int32 ta = 64, tb = 128, tc = 192;

    width = WIDTH;
    height = HEIGHT;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(ref_src = ct_allocate_ct_image_random(width, height, format, &rng, a, b));

    src = ct_image_to_vx_image(ref_src, context);
    dst = vxCreateImage(context, width, height, format);
    ASSERT_VX_OBJECT(dst, VX_TYPE_IMAGE);
    vxt1 = vxCreateThreshold(context, ttype, VX_TYPE_UINT8);
    vxt2 = vxCreateThreshold(context, ttype, VX_TYPE_UINT8);
    vxt3 = vxCreateThreshold(context, ttype, VX_TYPE_UINT8);

    if( ttype == VX_THRESHOLD_TYPE_BINARY )
    {
        vx_int32 v = 0;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt1, VX_THRESHOLD_THRESHOLD_VALUE, &ta, sizeof(ta)));
        VX_CALL(vxQueryThreshold(vxt1, VX_THRESHOLD_THRESHOLD_VALUE, &v, sizeof(v)));
        if (v != ta)
        {
            CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_VALUE failed\n");
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt2, VX_THRESHOLD_THRESHOLD_VALUE, &tb, sizeof(tb)));
        VX_CALL(vxQueryThreshold(vxt2, VX_THRESHOLD_THRESHOLD_VALUE, &v, sizeof(v)));
        if (v != tb)
        {
            CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_VALUE failed\n");
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt3, VX_THRESHOLD_THRESHOLD_VALUE, &tc, sizeof(tc)));
        VX_CALL(vxQueryThreshold(vxt3, VX_THRESHOLD_THRESHOLD_VALUE, &v, sizeof(v)));
        if (v != tc)
        {
            CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_VALUE failed\n");
        }
    }
    else
    {
        vx_int32 v1 = 0;
        vx_int32 v2 = 0;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt1, VX_THRESHOLD_THRESHOLD_LOWER, &ta, sizeof(ta)));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt1, VX_THRESHOLD_THRESHOLD_UPPER, &tc, sizeof(tc)));

        VX_CALL(vxQueryThreshold(vxt1, VX_THRESHOLD_THRESHOLD_LOWER, &v1, sizeof(v1)));
        if (v1 != ta)
        {
            CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_LOWER failed\n");
        }

        VX_CALL(vxQueryThreshold(vxt1, VX_THRESHOLD_THRESHOLD_UPPER, &v2, sizeof(v2)));
        if (v2 != tc)
        {
            CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_UPPER failed\n");
        }

        ta += 20;
        tc -= 20;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt2, VX_THRESHOLD_THRESHOLD_LOWER, &ta, sizeof(ta)));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt2, VX_THRESHOLD_THRESHOLD_UPPER, &tc, sizeof(tc)));

        VX_CALL(vxQueryThreshold(vxt2, VX_THRESHOLD_THRESHOLD_LOWER, &v1, sizeof(v1)));
        if (v1 != ta)
        {
            CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_LOWER failed\n");
        }

        VX_CALL(vxQueryThreshold(vxt2, VX_THRESHOLD_THRESHOLD_UPPER, &v2, sizeof(v2)));
        if (v2 != tc)
        {
            CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_UPPER failed\n");
        }

        ta += 20;
        tc -= 20;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt3, VX_THRESHOLD_THRESHOLD_LOWER, &ta, sizeof(ta)));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt3, VX_THRESHOLD_THRESHOLD_UPPER, &tc, sizeof(tc)));

        VX_CALL(vxQueryThreshold(vxt3, VX_THRESHOLD_THRESHOLD_LOWER, &v1, sizeof(v1)));
        if (v1 != ta)
        {
            CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_LOWER failed\n");
        }

        VX_CALL(vxQueryThreshold(vxt3, VX_THRESHOLD_THRESHOLD_UPPER, &v2, sizeof(v2)));
        if (v2 != tc)
        {
            CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_UPPER failed\n");
        }
    }

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt1, VX_THRESHOLD_TRUE_VALUE, &true_val, sizeof(true_val)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt1, VX_THRESHOLD_FALSE_VALUE, &false_val, sizeof(false_val)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt2, VX_THRESHOLD_TRUE_VALUE, &true_val, sizeof(true_val)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt2, VX_THRESHOLD_FALSE_VALUE, &false_val, sizeof(false_val)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt3, VX_THRESHOLD_TRUE_VALUE, &true_val, sizeof(true_val)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt3, VX_THRESHOLD_FALSE_VALUE, &false_val, sizeof(false_val)));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1 = vxCreateVirtualImage(graph, 0, 0, format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2 = vxCreateVirtualImage(graph, 0, 0, format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node1 = vxThresholdNode(graph, src, vxt1, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxThresholdNode(graph, virt1, vxt2, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxThresholdNode(graph, virt2, vxt3, dst), VX_TYPE_NODE);

    if (ENABLE_SUPERNODE)
    {
        ASSERT_NO_FAILURE(node_list[0] = node1);
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (ENABLE_SUPERNODE)
    {
        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    }
    else
    {
        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    }
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseThreshold(&vxt1));
    VX_CALL(vxReleaseThreshold(&vxt2));
    VX_CALL(vxReleaseThreshold(&vxt3));
    if (ENABLE_SUPERNODE)
        VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    CT_CollectGarbage(CT_GC_IMAGE);

    if ( ttype == VX_THRESHOLD_TYPE_BINARY )
    {
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
            width, height, "Threshold (3) - Binary", "U8=>U8=>U8=>U8", 0);
    }
    else
    {
        PrintSupernodePerf(perf_graph, perf_super_node, perf_node1, perf_node2, perf_node3, \
            width, height, "Threshold (3) - Range", "U8=>U8=>U8=>U8", 0);
    }
}

#endif

TESTCASE_TESTS(tiovxPerformance,
    tiovxPerfOpenFile,
    tiovxPerfAccumulate,
    tiovxPerfAccumulateSquare,
    tiovxPerfAccumulateWeighted,
    tiovxPerfAdd888,
    tiovxPerfAdd8816,
    tiovxPerfAdd81616,
    tiovxPerfAdd161616,
    tiovxPerfSubtract888,
    tiovxPerfSubtract8816,
    tiovxPerfSubtract81616,
    tiovxPerfSubtract161616,
    tiovxPerfNot,
    tiovxPerfBinOp8,
    tiovxPerfBinOp16,
    tiovxPerfBox3x3,
    tiovxPerfDilate3x3,
    tiovxPerfErode3x3,
    tiovxPerfFastCorners,
    tiovxPerfCanny,
    tiovxPerfChannelCombineRGB,
    tiovxPerfChannelCombineRGBX,
    tiovxPerfChannelCombineYUYV,
    tiovxPerfChannelCombineNV12,
    tiovxPerfChannelExtractRGB,
    tiovxPerfChannelExtractRGBX,
    tiovxPerfColorConvert,
    tiovxPerfConvertDepth)

TESTCASE_TESTS(tiovxPerformance2,
    tiovxPerfConvolve,
    tiovxPerfEqualizeHistogram,
    tiovxPerfGaussian3x3,
    tiovxPerfGaussianPyramid,
    tiovxPerfHalfScaleGaussian,
    tiovxPerfHarrisCorners,
    tiovxPerfHistogram,
    tiovxPerfIntegralImg,
    tiovxPerfLaplacianReconstruct,
    tiovxPerfLUT,
    tiovxPerfMagnitude,
    tiovxPerfMeanStdDev,
    tiovxPerfMedian3x3,
    tiovxPerfMinMaxLoc,
    tiovxPerfMultiply888,
    tiovxPerfMultiply8816,
    tiovxPerfMultiply81616,
    tiovxPerfMultiply161616,
    tiovxPerfNonLinearFilter,
    tiovxPerfOptFlow,
    tiovxPerfPhase,
    tiovxPerfRemapBilinear,
    tiovxPerfRemapNearest,
    tiovxPerfScale,
    tiovxPerfSobel3x3,
    tiovxPerfThreshold,
    tiovxPerfWarpAffine,
    tiovxPerfWarpPerspective,
    tiovxPerfCloseFile)

#ifdef BUILD_BAM

TESTCASE_TESTS(tiovxSupernodePerformance,
    tiovxSupernodePerfOpenFile,
    tiovxSupernodePerfAdd,
    tiovxSupernodePerfSubtract,
    tiovxSupernodePerfNot,
    tiovxSupernodePerfAnd,
    tiovxSupernodePerfOr,
    tiovxSupernodePerfXor,
    tiovxSupernodePerfAbsdiff8,
    tiovxSupernodePerfAbsdiff16,
    tiovxSupernodePerfBox3x3,
    tiovxSupernodePerfDilate3x3,
    tiovxSupernodePerfErode3x3,
    tiovxSupernodePerfCanny,
    tiovxSupernodePerfChannelCombine,
    tiovxSupernodePerfChannelExtract,
    tiovxSupernodePerfColorConvert,
    tiovxSupernodePerfConvertDepth)

TESTCASE_TESTS(tiovxSupernodePerformance2,
    tiovxSupernodePerfConvolve,
    /*tiovxSupernodePerfEqualizeHistogram,*/
    tiovxSupernodePerfGaussian3x3,
    /*tiovxSupernodePerfHalfScaleGaussian,*/
    /*tiovxSupernodePerfHarrisCorners,*/
    tiovxSupernodePerfHistogram,
    tiovxSupernodePerfIntegralImg,
    tiovxSupernodePerfLUT,
    tiovxSupernodePerfMeanStdDev,
    tiovxSupernodePerfMedian3x3,
    tiovxSupernodePerfMagnitude,
    tiovxSupernodePerfMinMaxLoc,
    tiovxSupernodePerfMultiply,
    tiovxSupernodePerfNonLinearFilter,
    tiovxSupernodePerfPhase,
    tiovxSupernodePerfSobel3x3,
    tiovxSupernodePerfThreshold,
    tiovxSupernodePerfCloseFile)

#endif
