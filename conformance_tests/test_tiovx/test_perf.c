/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */



#include "test_engine/test.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <VX/vx.h>
#include <VX/vxu.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

#define WIDTH           (640)
#define HEIGHT          (480)
#define OUTPUT_FILE     "tiovx_performance.html"
#define MAX_CONV_SIZE   (15)
#define MAX_PMD_LEVELS  (7)
#define MAX_HISTOGRAMS_BINS        (256)
#ifndef M_PI
#define M_PIF   3.14159265358979323846f
#else
#define M_PIF   (vx_float32)M_PI
#endif


static FILE *perf_file = NULL;
static uint32_t gTiovxKernIdx;

TESTCASE(tiovxPerformance, CT_VXContext, ct_setup_vx_context, 0)

TEST(tiovxPerformance, tiovxPerfOpenFile)
{
    char filepath[MAXPATHLENGTH];
    size_t sz;

    if (!perf_file)
    {
        sz = snprintf(filepath, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(),
            OUTPUT_FILE);
        ASSERT(sz < MAXPATHLENGTH);

        printf ("File %s\n", filepath);
        perf_file = fopen(filepath, "wb");
        if (!perf_file)
        {
            printf("Cannot open file %s\n", OUTPUT_FILE);
            exit(0);
        }
        fprintf(perf_file, "%s\n", "<html>");
        fprintf(perf_file, "%s\n", "<head>");
        fprintf(perf_file, "%s\n", "  <meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">");
        fprintf(perf_file, "%s\n", "  <title>TIOVX Kernel Performance</title>");
        fprintf(perf_file, "%s\n", "</head>");

        fprintf(perf_file, "%s\n", "<body>");
        fprintf(perf_file, "%s\n", "  <table width=\"100%\" border=\"1\" bgcolor=\"#FFFFDD\">");
        fprintf(perf_file, "%s\n", "    <tr bgcolor=\"#FFFFEE\">");
        fprintf(perf_file, "%s\n", "<td width=\"5\" align=\"center\">Index</td>");
        fprintf(perf_file, "%s\n", "<td width=\"25%\" align=\"center\">Kernel</td>");
        fprintf(perf_file, "%s\n", "<td width=\"20%\" align=\"center\">Frame Size (Pixels)</td>");
        fprintf(perf_file, "%s\n", "<td width=\"25%\" align=\"center\">Graph Performance (msec)</td>");
        fprintf(perf_file, "%s\n", "<td width=\"25%\" align=\"center\">Node Performance</td>");
        fprintf(perf_file, "%s\n", "</tr>");

        gTiovxKernIdx = 0;
    }
}

TEST(tiovxPerformance, tiovxPerfCloseFile)
{
    if (perf_file)
    {
        fprintf(perf_file, "%s\n", "</table>");
        fprintf(perf_file, "%s\n", "</body>");
        fprintf(perf_file, "%s\n", "</html>");

        fclose(perf_file);
        perf_file = NULL;
    }
}

void PrintPerf(vx_perf_t graph, vx_perf_t node, uint32_t width,  uint32_t height, const char* testName)
{

    gTiovxKernIdx ++;
    fprintf(perf_file, "%s\n", " <tr align=\"center\">");
    fprintf(perf_file, "%s%d%s\n", "<td>", gTiovxKernIdx, "</td>");
    fprintf(perf_file, "%s%s%s\n", "<td>", testName, "</td>");
    fprintf(perf_file, "%s%dx%d  (%d)%s\n", "<td>", width, height, width*height, "</td>");
    fprintf(perf_file, "%s%4d.%-6d%s\n", "<td>", (uint32_t)(graph.max/1000000), (uint32_t)(graph.max%1000000), "</td>");
    fprintf(perf_file, "%s%4d.%-6d%s\n", "<td>", (uint32_t)(node.max/1000000), (uint32_t)(node.max%1000000), "</td>");
    fprintf(perf_file, "%s\n", " </tr>");
}

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

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Accumulate");
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

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "AccumulateSquare");
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
    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "AccumulateWeighted");
}

TEST(tiovxPerformance, tiovxPerfAdd)
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
    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Addition");
}

TEST(tiovxPerformance, tiovxPerfSubtract)
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
    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Sutraction");
}

typedef vx_node   (VX_API_CALL *vxBinopFunction) (vx_graph, vx_image, vx_image, vx_image);

TEST(tiovxPerformance, tiovxPerfBinOp)
{
    vx_image src1, src2, dst;
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node node;
    vxBinopFunction vxFunc[3] = {vxAndNode, vxOrNode, vxXorNode};
    char function[3][20] = {"And Opearation", "OR Opearation",
        "XOR Opearation"};
    vx_perf_t perf_node, perf_graph;
    vx_uint32 cnt;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    for (cnt = 0; cnt < 3; cnt ++)
    {
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(dst   = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(src1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
        ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));

        ASSERT_VX_OBJECT(node = vxFunc[cnt](graph, src1, src2, dst), VX_TYPE_NODE);

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
        vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseImage(&src1));
        VX_CALL(vxReleaseImage(&src2));
        VX_CALL(vxReleaseImage(&dst));
        VX_CALL(vxReleaseGraph(&graph));

        PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, function[cnt]);
    }
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

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Box3x3");
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

    VX_CALL(vxVerifyGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseNode(&node));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseGraph(&graph));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseThreshold(&hyst));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&src));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&dst));

    PrintPerf(perf_graph, perf_node, lena->width, lena->height, "Canny");
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
    CT_EXPAND(nextmacro(testArgName "/conv=3x3", __VA_ARGS__, 3, 3))
#define ADD_CONV_SCALE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv_scale=1", __VA_ARGS__, 1))
#define ADD_CONV_GENERATORS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv_fill=identity", __VA_ARGS__, convolution_data_fill_identity))
#define ADD_CONV_DST_FORMAT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dst8U", __VA_ARGS__, VX_DF_IMAGE_U8))

TEST_WITH_ARG(tiovxPerformance, tiovxPerfConvolve, Convolve_Arg,
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

    PrintPerf(perf_graph, perf_node, src->width, src->height, "Convolution");
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

    PrintPerf(perf_graph, perf_node, src->width, src->height, "Dilate 3x3");
}

TEST(tiovxPerformance, tiovxPerfEqualizeHistogram)
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

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0 && graph == 0);

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Equalize Histogram");
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

    PrintPerf(perf_graph, perf_node, src->width, src->height, "Erode (3x3)");
}

TEST_WITH_ARG(tiovxPerformance, tiovxPerfGaussian3x3, Filter3x3_Arg,
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

    PrintPerf(perf_graph, perf_node, src->width, src->height, "Gaussian (3x3)");
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

TEST_WITH_ARG(tiovxPerformance, tiovxPerfGaussianPyramid, GaussianPmd_Arg,
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

    PrintPerf(perf_graph, perf_node, input->width, input->height, "Gaussian Pyramid");
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

TEST_WITH_ARG(tiovxPerformance, tiovxPerfHalfScaleGaussian, HDG_Arg,
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

    PrintPerf(perf_graph, perf_node, src->width, src->height, "Half Scale Gaussian Pyramid");
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
    long sz;
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
    ASSERT(sz);
    fseek(f, 0, SEEK_SET);

    ASSERT(buf = ct_alloc_mem(sz + 1));
    ASSERT(sz == fread(buf, 1, sz, f));

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

TEST_WITH_ARG(tiovxPerformance, tiovxPerfHarrisCorners, HarrisC_Arg,
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

    PrintPerf(perf_graph, perf_node, input->width, input->height, "Harris Corners");
}

TEST(tiovxPerformance, tiovxPerfHistogram)
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

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseDistribution(&dist1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0 && graph == 0);

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Histogram");
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int width, height;
} Integral_Arg;

TEST_WITH_ARG(tiovxPerformance, tiovxPerfIntegralImg, Integral_Arg,
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

    PrintPerf(perf_graph, perf_node, src->width, src->height, "Integral Image");
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
    CT_EXPAND(nextmacro(testArgName "/S16", __VA_ARGS__, VX_TYPE_INT16))

TEST_WITH_ARG(tiovxPerformance, tiovxPerfLUT, Lut_Arg,
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
    PrintPerf(perf_graph, perf_node, src->width, src->height, "LookUpTable");
}

TEST(tiovxPerformance, tiovxPerfMagnitude)
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
    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Magnitude");
}

TEST_WITH_ARG(tiovxPerformance, tiovxPerfMedian3x3, Filter3x3_Arg,
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

    PrintPerf(perf_graph, perf_node, src->width, src->height, "Median (3x3)");
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
    CT_EXPAND(nextmacro(testArgName "/VX_NONLINEAR_FILTER_MEDIAN", __VA_ARGS__, VX_NONLINEAR_FILTER_MEDIAN))

#define NLF_NLF_ADD_PATTERNS_BOX_CROSS_DISK(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_PATTERN_DISK", __VA_ARGS__, VX_PATTERN_DISK))

#define NLF_ADD_PATTERNS_BOX_CROSS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_PATTERN_CROSS", __VA_ARGS__, VX_PATTERN_CROSS))

TEST_WITH_ARG(tiovxPerformance, tiovxPerfNonLinearFilter, NLFilter_Arg,
    CT_GENERATE_PARAMETERS("randomInput/mask=5x5", NLF_ADD_FUNCTIONS, NLF_NLF_ADD_PATTERNS_BOX_CROSS_DISK, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_256x256, ARG, convolve_generate_random, NULL, 5)
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

    PrintPerf(perf_graph, perf_node, src->width, src->height, "NonLinear Filter");
}

TEST_WITH_ARG(tiovxPerformance, tiovxPerfSobel3x3, Filter3x3_Arg,
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

    PrintPerf(perf_graph, perf_node, src->width, src->height, "Sobel (3x3)");
}

TEST(tiovxPerformance, tiovxPerfPhase)
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

    PrintPerf(perf_graph, perf_node, WIDTH, HEIGHT, "Phase");
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

TEST_WITH_ARG(tiovxPerformance, tiovxPerfWarpAffine, WarpAffine_Arg,
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

    PrintPerf(perf_graph, perf_node, input->width, input->height, "Warp Affine");
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

TEST_WITH_ARG(tiovxPerformance, tiovxPerfWarpPerspective, WarpPerspective_Arg,
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

    PrintPerf(perf_graph, perf_node, input->width, input->height, "Warp Pespective");
}

TESTCASE_TESTS(tiovxPerformance,
    tiovxPerfOpenFile,
    tiovxPerfAccumulate,
    tiovxPerfAccumulateSquare,
    tiovxPerfAccumulateWeighted,
    tiovxPerfAdd,
    tiovxPerfSubtract,
    tiovxPerfBinOp,
    tiovxPerfBox3x3,
    tiovxPerfMedian3x3,
    tiovxPerfDilate3x3,
    tiovxPerfErode3x3,
    tiovxPerfSobel3x3,
    tiovxPerfCanny,
    tiovxPerfConvolve,
    tiovxPerfEqualizeHistogram,
    tiovxPerfGaussian3x3,
    tiovxPerfGaussianPyramid,
    tiovxPerfHarrisCorners,
    tiovxPerfHistogram,
    tiovxPerfIntegralImg,
    tiovxPerfLUT,
    tiovxPerfMagnitude,
    tiovxPerfNonLinearFilter,
    tiovxPerfPhase,
    tiovxPerfWarpAffine,
    tiovxPerfWarpPerspective,
    tiovxPerfCloseFile)
