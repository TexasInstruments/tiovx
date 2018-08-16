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
#include <TI/tivx_obj_desc.h>
#include <TI/tivx_config.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include <string.h>
#include <math.h>

#include "shared_functions.h"

#define MAX_POINTS 100

TESTCASE(tivxBoundary, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(tivxNegativeBoundary, CT_VXContext, ct_setup_vx_context, 0)

TESTCASE(tivxBoundary2, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(tivxNegativeBoundary2, CT_VXContext, ct_setup_vx_context, 0)

/* TIVX_IMAGE_MAX_OBJECTS */
TEST(tivxBoundary, testImageBoundary)
{
    vx_context context = context_->vx_context_;
    vx_image   src_image[TIVX_IMAGE_MAX_OBJECTS];
    int i;

    for (i = 0; i < TIVX_IMAGE_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_image[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT(vx_false_e == tivxIsReferenceVirtual((vx_reference)src_image[i]) );
    }

    for (i = 0; i < TIVX_IMAGE_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
    }
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_IMAGE_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_IMAGE_MAX_OBJECTS);
}

/* TIVX_IMAGE_MAX_SUBIMAGES */
TEST(tivxBoundary2, testSubImageBoundary)
{
    vx_context context = context_->vx_context_;
    vx_image   src_image, src_image2;
    vx_image   subimage[TIVX_IMAGE_MAX_SUBIMAGES], subimage2[TIVX_IMAGE_MAX_SUBIMAGES];
    int fullimage_size = 640;
    int subimage_size = sqrt((fullimage_size*fullimage_size)/TIVX_IMAGE_MAX_SUBIMAGES);
    int startx = 0;
    int starty = 0;
    int endx   = subimage_size;
    int endy   = subimage_size;
    int i;

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, fullimage_size, fullimage_size, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src_image2 = vxCreateImage(context, fullimage_size, fullimage_size, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    for (i = 0; i < TIVX_IMAGE_MAX_SUBIMAGES; i++)
    {
        if (i == 0)
        {
            startx = 0;
            starty = 0;
            endx   = subimage_size;
            endy   = subimage_size;
        }
        else if (i < (TIVX_IMAGE_MAX_SUBIMAGES / 2) )
        {
            if ( (i % 2) == 0 )
            {
                startx += subimage_size;
                endx   += subimage_size;
                starty -= subimage_size;
                endy   -= subimage_size;
            }
            else
            {
                starty += subimage_size;
                endy   += subimage_size;
            }
        }
        else
        {
            if (i == (TIVX_IMAGE_MAX_SUBIMAGES / 2) )
            {
                startx = 0;
                starty = 2*subimage_size;
                endx   = subimage_size;
                endy   = 2*subimage_size + subimage_size;
            }
            else if ( (i % 2) == 0 )
            {
                startx += subimage_size;
                endx   += subimage_size;
                starty -= subimage_size;
                endy   -= subimage_size;
            }
            else
            {
                starty += subimage_size;
                endy   += subimage_size;
            }
        }
        vx_rectangle_t rect = {startx, starty, endx, endy};
        ASSERT_VX_OBJECT(subimage[i] = vxCreateImageFromROI(src_image, &rect), VX_TYPE_IMAGE);
    }

    for (i = 0; i < TIVX_IMAGE_MAX_SUBIMAGES; i++)
    {
        if (i == 0)
        {
            startx = 0;
            starty = 0;
            endx   = subimage_size;
            endy   = subimage_size;
        }
        else if (i < (TIVX_IMAGE_MAX_SUBIMAGES / 2) )
        {
            if ( (i % 2) == 0 )
            {
                startx += subimage_size;
                endx   += subimage_size;
                starty -= subimage_size;
                endy   -= subimage_size;
            }
            else
            {
                starty += subimage_size;
                endy   += subimage_size;
            }
        }
        else
        {
            if (i == (TIVX_IMAGE_MAX_SUBIMAGES / 2) )
            {
                startx = 0;
                starty = 2*subimage_size;
                endx   = subimage_size;
                endy   = 2*subimage_size + subimage_size;
            }
            else if ( (i % 2) == 0 )
            {
                startx += subimage_size;
                endx   += subimage_size;
                starty -= subimage_size;
                endy   -= subimage_size;
            }
            else
            {
                starty += subimage_size;
                endy   += subimage_size;
            }
        }
        vx_rectangle_t rect = {startx, starty, endx, endy};
        ASSERT_VX_OBJECT(subimage2[i] = vxCreateImageFromROI(src_image2, &rect), VX_TYPE_IMAGE);
    }

    for (i = 0; i < TIVX_IMAGE_MAX_SUBIMAGES; i++)
    {
        VX_CALL(vxReleaseImage(&subimage2[i]));
        VX_CALL(vxReleaseImage(&subimage[i]));
    }
    VX_CALL(vxReleaseImage(&src_image2));
    VX_CALL(vxReleaseImage(&src_image));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_IMAGE_MAX_SUBIMAGES", &stats));
    ASSERT(stats.max_used_value == TIVX_IMAGE_MAX_SUBIMAGES);
}

/* TIVX_IMAGE_MAX_OBJECTS */
TEST(tivxBoundary, testVirtualImageBoundary)
{
    vx_context context = context_->vx_context_;
    vx_image   src_image[TIVX_IMAGE_MAX_OBJECTS];
    int i;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for (i = 0; i < TIVX_IMAGE_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_image[i] = vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT(vx_true_e == tivxIsReferenceVirtual((vx_reference)src_image[i]) );
    }

    for (i = 0; i < TIVX_IMAGE_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
    }

    VX_CALL(vxReleaseGraph(&graph));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_IMAGE_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_IMAGE_MAX_OBJECTS);
}

/* TIVX_PYRAMID_MAX_OBJECTS */
TEST(tivxBoundary, testPyramidBoundary)
{
    vx_context context = context_->vx_context_;
    vx_pyramid   src_pyr[TIVX_PYRAMID_MAX_OBJECTS];
    int i;

    for (i = 0; i < TIVX_PYRAMID_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_pyr[i] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    }

    for (i = 0; i < TIVX_PYRAMID_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleasePyramid(&src_pyr[i]));
    }
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_PYRAMID_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_PYRAMID_MAX_OBJECTS);
}

/* TIVX_PYRAMID_MAX_OBJECTS */
TEST(tivxBoundary, testVirtualPyramidBoundary)
{
    vx_context context = context_->vx_context_;
    vx_pyramid   src_pyr[TIVX_PYRAMID_MAX_OBJECTS];
    int i;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for (i = 0; i < TIVX_PYRAMID_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_pyr[i] = vxCreateVirtualPyramid(graph, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    }

    for (i = 0; i < TIVX_PYRAMID_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleasePyramid(&src_pyr[i]));
    }

    VX_CALL(vxReleaseGraph(&graph));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_PYRAMID_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_PYRAMID_MAX_OBJECTS);
}

/* TIVX_PYRAMID_MAX_LEVEL_OBJECTS */
TEST(tivxBoundary, testPyramidLevelBoundary)
{
    vx_context context = context_->vx_context_;
    vx_pyramid   src_pyr;

    ASSERT_VX_OBJECT(src_pyr = vxCreatePyramid(context, TIVX_PYRAMID_MAX_LEVEL_OBJECTS, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    VX_CALL(vxReleasePyramid(&src_pyr));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_PYRAMID_MAX_LEVEL_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_PYRAMID_MAX_LEVEL_OBJECTS);
}

/* TIVX_PYRAMID_MAX_LEVELS_ORB */
TEST(tivxBoundary2, testOrbPyramidLevelBoundary)
{
    vx_context context = context_->vx_context_;
    vx_pyramid   src_pyr;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(src_pyr = vxCreatePyramid(context, TIVX_PYRAMID_MAX_LEVELS_ORB, VX_SCALE_PYRAMID_ORB, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    VX_CALL(vxReleasePyramid(&src_pyr));

    ASSERT_VX_OBJECT(src_pyr = vxCreateVirtualPyramid(graph, TIVX_PYRAMID_MAX_LEVELS_ORB, VX_SCALE_PYRAMID_ORB, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    VX_CALL(vxReleasePyramid(&src_pyr));

    VX_CALL(vxReleaseGraph(&graph));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_PYRAMID_MAX_LEVELS_ORB", &stats));
    ASSERT(stats.max_used_value == TIVX_PYRAMID_MAX_LEVELS_ORB);
}

/* TIVX_PYRAMID_MAX_LEVEL_OBJECTS */
TEST(tivxBoundary, testVirtualPyramidLevelBoundary)
{
    vx_context context = context_->vx_context_;
    vx_pyramid   src_pyr;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(src_pyr = vxCreateVirtualPyramid(graph, TIVX_PYRAMID_MAX_LEVEL_OBJECTS, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    VX_CALL(vxReleasePyramid(&src_pyr));

    VX_CALL(vxReleaseGraph(&graph));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_PYRAMID_MAX_LEVEL_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_PYRAMID_MAX_LEVEL_OBJECTS);
}

/* TIVX_ARRAY_MAX_OBJECTS */
TEST(tivxBoundary, testArrayBoundary)
{
    vx_context context = context_->vx_context_;
    vx_array   src_array[TIVX_ARRAY_MAX_OBJECTS];
    int i;

    for (i = 0; i < TIVX_ARRAY_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_array[i] = vxCreateArray(context, VX_TYPE_KEYPOINT, 4), VX_TYPE_ARRAY);
    }

    for (i = 0; i < TIVX_ARRAY_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseArray(&src_array[i]));
    }
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_ARRAY_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_ARRAY_MAX_OBJECTS);
}

/* TIVX_ARRAY_MAX_OBJECTS */
TEST(tivxBoundary, testVirtualArrayBoundary)
{
    vx_context context = context_->vx_context_;
    vx_array   src_array[TIVX_ARRAY_MAX_OBJECTS];
    int i;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for (i = 0; i < TIVX_ARRAY_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_array[i] = vxCreateVirtualArray(graph, VX_TYPE_KEYPOINT, 4), VX_TYPE_ARRAY);
    }

    for (i = 0; i < TIVX_ARRAY_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseArray(&src_array[i]));
    }

    VX_CALL(vxReleaseGraph(&graph));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_ARRAY_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_ARRAY_MAX_OBJECTS);
}

/* TIVX_CONVOLUTION_MAX_OBJECTS */
TEST(tivxBoundary, testConvolutionBoundary)
{
    vx_context context = context_->vx_context_;
    vx_convolution   src_conv[TIVX_CONVOLUTION_MAX_OBJECTS];
    int i;

    for (i = 0; i < TIVX_CONVOLUTION_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_conv[i] = vxCreateConvolution(context, 3, 3), VX_TYPE_CONVOLUTION);
    }

    for (i = 0; i < TIVX_CONVOLUTION_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseConvolution(&src_conv[i]));
    }
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_CONVOLUTION_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_CONVOLUTION_MAX_OBJECTS);
}

/* TIVX_DISTRIBUTION_MAX_OBJECTS */
TEST(tivxBoundary, testDistributionBoundary)
{
    vx_context context = context_->vx_context_;
    vx_distribution   src_dist[TIVX_DISTRIBUTION_MAX_OBJECTS];
    int i;

    for (i = 0; i < TIVX_DISTRIBUTION_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_dist[i] = vxCreateDistribution(context, 100, 5, 200), VX_TYPE_DISTRIBUTION);
    }

    for (i = 0; i < TIVX_DISTRIBUTION_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseDistribution(&src_dist[i]));
    }
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_DISTRIBUTION_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_DISTRIBUTION_MAX_OBJECTS);
}

/* TIVX_LUT_MAX_OBJECTS */
TEST(tivxBoundary, testLUTBoundary)
{
    vx_context context = context_->vx_context_;
    vx_lut   src_lut[TIVX_LUT_MAX_OBJECTS];
    int i;

    for (i = 0; i < TIVX_LUT_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_lut[i] = vxCreateLUT(context, VX_TYPE_UINT8, 256), VX_TYPE_LUT);
    }

    for (i = 0; i < TIVX_LUT_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseLUT(&src_lut[i]));
    }
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_LUT_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_LUT_MAX_OBJECTS);
}

/* TIVX_DELAY_MAX_OBJECTS */
TEST(tivxBoundary, testDelayBoundary)
{
    vx_context context = context_->vx_context_;
    vx_delay   src_delay[TIVX_DELAY_MAX_OBJECTS];
    vx_image   image;
    int i;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    for (i = 0; i < TIVX_DELAY_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_delay[i] = vxCreateDelay(context, (vx_reference)image, 4), VX_TYPE_DELAY);
    }

    for (i = 0; i < TIVX_DELAY_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseDelay(&src_delay[i]));
    }

    VX_CALL(vxReleaseImage(&image));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_DELAY_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_DELAY_MAX_OBJECTS);
}

/* TIVX_GRAPH_MAX_DELAYS */
TEST(tivxBoundary2, testGraphDelayBoundary)
{
    vx_context context = context_->vx_context_;
    vx_delay   src_delay[TIVX_GRAPH_MAX_DELAYS];
    vx_graph   graph = 0;
    vx_image   image, in_image[TIVX_GRAPH_MAX_DELAYS], out_image[TIVX_GRAPH_MAX_DELAYS];
    vx_node    box_nodes[TIVX_GRAPH_MAX_DELAYS], med_nodes[TIVX_GRAPH_MAX_DELAYS];
    int i;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    for (i = 0; i < TIVX_GRAPH_MAX_DELAYS; i++)
    {
        ASSERT_VX_OBJECT(in_image[i]  = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out_image[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src_delay[i] = vxCreateDelay(context, (vx_reference)image, 2), VX_TYPE_DELAY);
    }

    for (i = 0; i < TIVX_GRAPH_MAX_DELAYS; i++)
    {
        ASSERT_VX_OBJECT(box_nodes[i] = vxBox3x3Node(graph, in_image[i], (vx_image)vxGetReferenceFromDelay(src_delay[i], 0)), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(med_nodes[i] = vxMedian3x3Node(graph, (vx_image)vxGetReferenceFromDelay(src_delay[i], -1), out_image[i]), VX_TYPE_NODE);
    }

    for (i = 0; i < TIVX_GRAPH_MAX_DELAYS; i++)
    {
        VX_CALL(vxRegisterAutoAging(graph, src_delay[i]));
    }

    for (i = 0; i < TIVX_GRAPH_MAX_DELAYS; i++)
    {
        VX_CALL(vxReleaseDelay(&src_delay[i]));
        VX_CALL(vxReleaseImage(&in_image[i]));
        VX_CALL(vxReleaseImage(&out_image[i]));
    }

    VX_CALL(vxReleaseImage(&image));

    for (i = 0; i < TIVX_GRAPH_MAX_DELAYS; i++)
    {
        VX_CALL(vxReleaseNode(&box_nodes[i]));
        VX_CALL(vxReleaseNode(&med_nodes[i]));
    }

    VX_CALL(vxReleaseGraph(&graph));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_GRAPH_MAX_DELAYS", &stats));
    ASSERT(stats.max_used_value == TIVX_GRAPH_MAX_DELAYS);
}

/* TIVX_DELAY_MAX_PRM_OBJECT */
/* Note: TIVX_NODE_MAX_IN_NODES and TIVX_NODE_MAX_OUT_NODES restrict this from being fully tested
 * Therefore, asserting that it must be >= to these values */
TEST(tivxBoundary2, testDelayMaxPrmBoundary)
{
    if ( (TIVX_DELAY_MAX_PRM_OBJECT == TIVX_NODE_MAX_IN_NODES) &&
         (TIVX_DELAY_MAX_PRM_OBJECT == TIVX_NODE_MAX_OUT_NODES) )
    {

        vx_context context = context_->vx_context_;
        vx_delay   src_delay;
        vx_graph   graph = 0;
        vx_image   image, in_image, out_image, out_image2[TIVX_DELAY_MAX_PRM_OBJECT];
        vx_node    box_nodes, med_nodes, med_nodes2[TIVX_DELAY_MAX_PRM_OBJECT];
        int i;

        ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        vx_kernel kernels[] = {
            vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3),
            vxGetKernelByEnum(context, VX_KERNEL_MEDIAN_3x3)
        };

        ASSERT_VX_OBJECT(in_image  = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out_image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(src_delay = vxCreateDelay(context, (vx_reference)image, 2), VX_TYPE_DELAY);

        for (i = 0; i < TIVX_DELAY_MAX_PRM_OBJECT; i++)
        {
            ASSERT_VX_OBJECT(out_image2[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }

        box_nodes = vxCreateGenericNode(graph, kernels[0]);
        med_nodes = vxCreateGenericNode(graph, kernels[1]);

        VX_CALL(vxSetParameterByIndex(box_nodes, 0, (vx_reference)in_image));
        VX_CALL(vxSetParameterByIndex(box_nodes, 1, (vx_reference)vxGetReferenceFromDelay(src_delay, 0)));
        VX_CALL(vxSetParameterByIndex(med_nodes, 0, (vx_reference)vxGetReferenceFromDelay(src_delay, -1)));
        VX_CALL(vxSetParameterByIndex(med_nodes, 1, (vx_reference)out_image));

        for (i = 0; i < TIVX_DELAY_MAX_PRM_OBJECT; i++)
        {
            med_nodes2[i] = vxCreateGenericNode(graph, kernels[1]);
            VX_CALL(vxSetParameterByIndex(med_nodes2[i], 0, (vx_reference)vxGetReferenceFromDelay(src_delay, -1)));
            VX_CALL(vxSetParameterByIndex(med_nodes2[i], 1, (vx_reference)out_image2[i]));
        }

        VX_CALL(vxRegisterAutoAging(graph, src_delay));

        VX_CALL(vxReleaseImage(&in_image));
        VX_CALL(vxReleaseImage(&out_image));

        for (i = 0; i < TIVX_DELAY_MAX_PRM_OBJECT; i++)
        {
            VX_CALL(vxReleaseImage(&out_image2[i]));
        }

        VX_CALL(vxReleaseDelay(&src_delay));

        for (i = 0; i < dimof(kernels); i++)
        {
            VX_CALL(vxReleaseKernel(&kernels[i]));
        }

        VX_CALL(vxReleaseNode(&box_nodes));
        VX_CALL(vxReleaseNode(&med_nodes));

        for (i = 0; i < TIVX_DELAY_MAX_PRM_OBJECT; i++)
        {
            VX_CALL(vxReleaseNode(&med_nodes2[i]));
        }

        VX_CALL(vxReleaseImage(&image));

        VX_CALL(vxReleaseGraph(&graph));

    }
    else
    {
        ASSERT(TIVX_DELAY_MAX_PRM_OBJECT >= TIVX_NODE_MAX_IN_NODES);
        ASSERT(TIVX_DELAY_MAX_PRM_OBJECT >= TIVX_NODE_MAX_OUT_NODES);
        printf("To fully test the TIVX_DELAY_MAX_PRM_OBJECT value, set it to %d in include/tivx_config.h and re-run only this test case\n", TIVX_NODE_MAX_IN_NODES);
    }

}

#define VX_KERNEL_CONFORMANCE_TEST_OWN_USER (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 2)
#define VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME "org.khronos.openvx.test.own_user"

typedef enum _own_params_e
{
    OWN_PARAM_INPUT0 = 0,
    OWN_PARAM_INPUT1,
    OWN_PARAM_INPUT2,
    OWN_PARAM_INPUT3,
    OWN_PARAM_INPUT4,
    OWN_PARAM_INPUT5,
    OWN_PARAM_INPUT6,
    OWN_PARAM_INPUT7,
    OWN_PARAM_OUTPUT0,
    OWN_PARAM_OUTPUT1,
    OWN_PARAM_OUTPUT2,
    OWN_PARAM_OUTPUT3,
    OWN_PARAM_OUTPUT4,
    OWN_PARAM_OUTPUT5,
    OWN_PARAM_OUTPUT6,
    OWN_PARAM_OUTPUT7
} own_params_e;

static enum vx_type_e type = VX_TYPE_IMAGE;
static enum vx_type_e objarray_itemtype = VX_TYPE_INVALID;

static vx_bool is_kernel_alloc = vx_false_e;
static vx_size local_size_auto_alloc = 10;
static vx_size local_size_kernel_alloc = 0;

static vx_status set_local_size_status_init = VX_SUCCESS;
static vx_status set_local_ptr_status_init = VX_SUCCESS;

static vx_status query_local_size_status_deinit = VX_SUCCESS;
static vx_status query_local_ptr_status_deinit = VX_SUCCESS;
static vx_status set_local_size_status_deinit = VX_SUCCESS;
static vx_status set_local_ptr_status_deinit = VX_SUCCESS;

static vx_status VX_CALLBACK own_set_image_valid_rect(
    vx_node node,
    vx_uint32 index,
    const vx_rectangle_t* const input_valid[],
    vx_rectangle_t* const output_valid[])
{
    vx_status status = VX_FAILURE;

    if (index == OWN_PARAM_OUTPUT0)
    {
        output_valid[0]->start_x = input_valid[0]->start_x + 2;
        output_valid[0]->start_y = input_valid[0]->start_y + 2;
        output_valid[0]->end_x   = input_valid[0]->end_x - 2;
        output_valid[0]->end_y   = input_valid[0]->end_y - 2;

        status = VX_SUCCESS;
    }

    return status;
}

static vx_bool is_validator_called = vx_false_e;
static vx_status VX_CALLBACK own_ValidatorMetaFromRef(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    is_validator_called = vx_true_e;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);

    vx_reference input = parameters[OWN_PARAM_INPUT0];
    ASSERT_VX_OBJECT_(return VX_FAILURE, input, type);
    vx_reference output = parameters[OWN_PARAM_OUTPUT0];
    ASSERT_VX_OBJECT_(return VX_FAILURE, output, type);

    vx_meta_format meta = metas[OWN_PARAM_OUTPUT0];

    vx_enum in_ref_type = VX_TYPE_INVALID;
    VX_CALL_(return VX_ERROR_INVALID_PARAMETERS, vxQueryReference(input, VX_REFERENCE_TYPE, &in_ref_type, sizeof(vx_enum)));
    vx_enum out_ref_type = VX_TYPE_INVALID;
    VX_CALL_(return VX_ERROR_INVALID_PARAMETERS, vxQueryReference(output, VX_REFERENCE_TYPE, &out_ref_type, sizeof(vx_enum)));

    if (in_ref_type == out_ref_type)
    {
        vx_enum format = VX_DF_IMAGE_U8;
        vx_uint32 src_width = 128, src_height = 128;
        vx_uint32 dst_width = 256, dst_height = 256;
        vx_enum item_type = (type == VX_TYPE_OBJECT_ARRAY) ? objarray_itemtype : VX_TYPE_UINT8;
        vx_size capacity = 20;
        vx_size bins = 36;
        vx_int32 offset = 0;
        vx_uint32 range = 360;
        vx_enum thresh_type = VX_THRESHOLD_TYPE_BINARY;
        vx_size num_items = 100;
        vx_size m = 5, n = 5;

        vx_enum actual_format = VX_TYPE_INVALID;
        vx_uint32 actual_src_width = 128, actual_src_height = 128;
        vx_uint32 actual_dst_width = 256, actual_dst_height = 256;
        vx_enum actual_item_type = VX_TYPE_INVALID;
        vx_size actual_capacity = 0;
        vx_size actual_levels = 0;
        vx_float32 actual_scale = 0;
        vx_size actual_bins = 0;
        vx_int32 actual_offset = -1;
        vx_uint32 actual_range = 0;
        vx_enum actual_thresh_type = VX_TYPE_INVALID;
        vx_size actual_num_items = 0;
        vx_size actual_m = 0, actual_n = 0;
        switch (type)
        {
        case VX_TYPE_IMAGE:
            VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_FORMAT, &actual_format, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_WIDTH, &actual_src_width, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_HEIGHT, &actual_src_height, sizeof(vx_uint32)));

            if (format == actual_format && src_width == actual_src_width && src_height == actual_src_height)
            {
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatFromReference(meta, input));
                vx_kernel_image_valid_rectangle_f callback = &own_set_image_valid_rect;
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_VALID_RECT_CALLBACK, &callback, sizeof(callback)));
            }
            else
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        default:
            return VX_ERROR_INVALID_PARAMETERS;
            break;
        }

    }

    return VX_SUCCESS;
}

static vx_status VX_CALLBACK own_ValidatorMetaFromAttr(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    is_validator_called = vx_true_e;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);

    vx_reference input = parameters[OWN_PARAM_INPUT0];

    vx_meta_format meta = metas[OWN_PARAM_OUTPUT0];

    vx_enum format = VX_DF_IMAGE_U8;
    vx_uint32 src_width = 128, src_height = 128;
    vx_uint32 dst_width = 256, dst_height = 256;
    vx_enum item_type = (type == VX_TYPE_OBJECT_ARRAY) ? objarray_itemtype : VX_TYPE_UINT8;
    vx_size capacity = 20;
    vx_size levels = 8;
    vx_float32 scale = 0.5f;
    vx_size bins = 36;
    vx_int32 offset = 0;
    vx_uint32 range = 360;
    vx_enum thresh_type = VX_THRESHOLD_TYPE_BINARY;
    vx_size num_items = 100;
    vx_size m = 5, n = 5;

    vx_enum actual_format = VX_TYPE_INVALID;
    vx_uint32 actual_src_width = 128, actual_src_height = 128;
    vx_uint32 actual_dst_width = 256, actual_dst_height = 256;
    vx_enum actual_item_type = VX_TYPE_INVALID;
    vx_size actual_capacity = 0;
    vx_size actual_levels = 0;
    vx_float32 actual_scale = 0;
    vx_size actual_bins = 0;
    vx_int32 actual_offset = -1;
    vx_uint32 actual_range = 0;
    vx_enum actual_thresh_type = VX_TYPE_INVALID;
    vx_size actual_num_items = 0;
    vx_size actual_m = 0, actual_n = 0;
    switch (type)
    {
    case VX_TYPE_IMAGE:
        VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_FORMAT, &actual_format, sizeof(vx_enum)));
        VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_WIDTH, &actual_src_width, sizeof(vx_uint32)));
        VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_HEIGHT, &actual_src_height, sizeof(vx_uint32)));

        if (format == actual_format && src_width == actual_src_width && src_height == actual_src_height)
        {
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_IMAGE_FORMAT, &format, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_IMAGE_WIDTH, &src_width, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_IMAGE_HEIGHT, &src_height, sizeof(vx_uint32)));
            vx_kernel_image_valid_rectangle_f callback = &own_set_image_valid_rect;
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_VALID_RECT_CALLBACK, &callback, sizeof(callback)));
        }
        else
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
        break;
    default:
        return VX_ERROR_INVALID_PARAMETERS;
        break;
    }

    return VX_SUCCESS;
}

static vx_bool is_kernel_called = vx_false_e;
static vx_status VX_CALLBACK own_Kernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    is_kernel_called = vx_true_e;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);
    EXPECT(parameters != NULL);
    EXPECT(num == 2);
    if (parameters != NULL && num == 2)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        EXPECT_VX_OBJECT(parameters[1], type);
    }

    return VX_SUCCESS;
}

static vx_bool is_initialize_called = vx_false_e;
static vx_status VX_CALLBACK own_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_size size = 0;
    void* ptr = NULL;
    is_initialize_called = vx_true_e;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);
    EXPECT(parameters != NULL);
    EXPECT(num == 2);
    if (parameters != NULL && num == 2)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        EXPECT_VX_OBJECT(parameters[1], type);
    }
    if (local_size_kernel_alloc > 0)
    {
        size = local_size_kernel_alloc;
        ptr = ct_calloc(1, local_size_kernel_alloc);
    }
    return VX_SUCCESS;
}

static vx_bool is_deinitialize_called = vx_false_e;
static vx_status VX_CALLBACK own_Deinitialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_size size = 0;
    void* ptr = NULL;
    is_deinitialize_called = vx_true_e;
    EXPECT(node != 0);
    EXPECT(parameters != NULL);
    EXPECT(num == 2);
    if (parameters != NULL && num == 2)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        EXPECT_VX_OBJECT(parameters[1], type);
    }
    query_local_size_status_deinit = vxQueryNode(node, VX_NODE_LOCAL_DATA_SIZE, &size, sizeof(size));
    query_local_ptr_status_deinit = vxQueryNode(node, VX_NODE_LOCAL_DATA_PTR, &ptr, sizeof(ptr));
    if (local_size_kernel_alloc > 0)
    {
        size = 0;
        if (ptr != NULL)
        {
            ct_free_mem(ptr);
            ptr = NULL;
        }
    }
    return VX_SUCCESS;
}

static void own_register_kernel(vx_context context, int numParams)
{
    vx_kernel kernel = 0;
    vx_size size = local_size_auto_alloc;

    ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
        context,
        VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME,
        VX_KERNEL_CONFORMANCE_TEST_OWN_USER,
        own_Kernel,
        numParams,
        own_ValidatorMetaFromRef,
        own_Initialize,
        own_Deinitialize), VX_TYPE_KERNEL);

    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_INPUT0, VX_INPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_INPUT0), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_INPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_INPUT1, VX_INPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_INPUT1), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_INPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_INPUT2, VX_INPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_INPUT2), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_INPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_INPUT3, VX_INPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_INPUT3), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_INPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_INPUT4, VX_INPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_INPUT4), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_INPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_INPUT5, VX_INPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_INPUT5), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_INPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_INPUT6, VX_INPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_INPUT6), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_INPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_INPUT7, VX_INPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_INPUT7), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_INPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_OUTPUT0, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_OUTPUT0), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_OUTPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_OUTPUT1, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_OUTPUT1), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_OUTPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_OUTPUT2, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_OUTPUT2), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_OUTPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_OUTPUT3, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_OUTPUT3), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_OUTPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_OUTPUT4, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_OUTPUT4), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_OUTPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_OUTPUT5, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_OUTPUT5), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_OUTPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_OUTPUT6, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_OUTPUT6), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_OUTPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_OUTPUT7, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_OUTPUT7), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_OUTPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxSetKernelAttribute(kernel, VX_KERNEL_LOCAL_DATA_SIZE, &size, sizeof(size)));
    VX_CALL(vxFinalizeKernel(kernel));
    VX_CALL(vxReleaseKernel(&kernel));
}

/* TIVX_KERNEL_MAX_PARAMS */
TEST(tivxBoundary2, testKernelParamsBoundary)
{
    vx_context context = context_->vx_context_;
    vx_kernel user_kernel = 0;

    ASSERT_NO_FAILURE(own_register_kernel(context, TIVX_KERNEL_MAX_PARAMS));
    ASSERT_VX_OBJECT(user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME), VX_TYPE_KERNEL);
    VX_CALL(vxRemoveKernel(user_kernel));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_KERNEL_MAX_PARAMS", &stats));
    ASSERT(stats.max_used_value == TIVX_KERNEL_MAX_PARAMS);
#ifdef DEBUG_PRINT_RESOURCES
    tivxPrintAllResourceStats();
    tivxExportAllResourceMaxUsedValueToFile();
#endif
}

/* TIVX_GRAPH_MAX_DATA_REF */
/* Note: Due to additional max defines, I reduced the value of TIVX_GRAPH_MAX_DATA_REF to 64 */
#define OPT_NODE 7
/* Since there are 5 scalars per node and 48 is max scalars */
#define MAX_SCALE_NODES 6
TEST(tivxBoundary2, testGraphDataRefBoundary)
{
    vx_context context = context_->vx_context_;
    if (TIVX_GRAPH_MAX_DATA_REF == 64)
    {
    vx_pyramid src_pyr1[OPT_NODE], src_pyr2[OPT_NODE];
    vx_array old_points_arr[OPT_NODE];
    vx_array new_points_arr[OPT_NODE];
    vx_array final_new_points_arr = 0;
    vx_uint32  num_iter_val = 100;
    vx_float32 eps_val      = 0.001f;
    vx_bool   use_estimations_val = vx_true_e;
    vx_scalar eps[MAX_SCALE_NODES];
    vx_scalar num_iter[MAX_SCALE_NODES];
    vx_scalar use_estimations[MAX_SCALE_NODES];
    vx_size   winSize = 9;
    vx_graph graph = 0;
    vx_node node[OPT_NODE], node_add;
    int width=640, height=480, i;
    vx_size num_points = 100;
    vx_image src1_add, src2_add, dst_add;

    ASSERT_VX_OBJECT(src1_add = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2_add = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_add  = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    for (i = 0; i < MAX_SCALE_NODES; i++)
    {
        ASSERT_VX_OBJECT(eps[i]             = vxCreateScalar(context, VX_TYPE_FLOAT32, &eps_val), VX_TYPE_SCALAR);
        ASSERT_VX_OBJECT(num_iter[i]        = vxCreateScalar(context, VX_TYPE_UINT32, &num_iter_val), VX_TYPE_SCALAR);
        ASSERT_VX_OBJECT(use_estimations[i] = vxCreateScalar(context, VX_TYPE_BOOL, &use_estimations_val), VX_TYPE_SCALAR);
    }

    /* 7 total data references -- not sure if multiple levels of pyramid count though */
    for (i = 0; i < OPT_NODE; i++)
    {
        ASSERT_VX_OBJECT(src_pyr1[i] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(src_pyr2[i] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(old_points_arr[i] = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);
        ASSERT_VX_OBJECT(new_points_arr[i] = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);
    }

    ASSERT_VX_OBJECT(final_new_points_arr = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for (i = 0; i < OPT_NODE-1; i++)
    {
        if (i < MAX_SCALE_NODES)
        {
            ASSERT_VX_OBJECT(node[i] = vxOpticalFlowPyrLKNode(
                graph,
                src_pyr1[i], src_pyr2[i],
                old_points_arr[i], old_points_arr[i], new_points_arr[i],
                VX_TERM_CRITERIA_BOTH, eps[i], num_iter[i], use_estimations[i], winSize), VX_TYPE_NODE);
        }
        else
        {
            ASSERT_VX_OBJECT(node[i] = vxOpticalFlowPyrLKNode(
                graph,
                src_pyr1[i], src_pyr2[i],
                old_points_arr[i], old_points_arr[i], new_points_arr[i],
                VX_TERM_CRITERIA_BOTH, eps[MAX_SCALE_NODES-1], num_iter[MAX_SCALE_NODES-1], use_estimations[MAX_SCALE_NODES-1], winSize), VX_TYPE_NODE);
        }
    }

    ASSERT_VX_OBJECT(node[OPT_NODE-1] = vxOpticalFlowPyrLKNode(
        graph,
        src_pyr1[OPT_NODE-1], src_pyr2[OPT_NODE-1],
        old_points_arr[OPT_NODE-1], old_points_arr[OPT_NODE-1], final_new_points_arr,
        VX_TERM_CRITERIA_BOTH, eps[MAX_SCALE_NODES-1], num_iter[MAX_SCALE_NODES-1], use_estimations[MAX_SCALE_NODES-1], winSize), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node_add = vxAddNode(graph, src1_add, src2_add, VX_CONVERT_POLICY_WRAP, dst_add), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseImage(&src1_add));
    VX_CALL(vxReleaseImage(&src2_add));
    VX_CALL(vxReleaseImage(&dst_add));

    VX_CALL(vxReleaseArray(&final_new_points_arr));

    for (i = 0; i < MAX_SCALE_NODES; i++)
    {
        VX_CALL(vxReleaseScalar(&eps[i]));
        VX_CALL(vxReleaseScalar(&num_iter[i]));
        VX_CALL(vxReleaseScalar(&use_estimations[i]));
    }

    for (i = 0; i < OPT_NODE; i++)
    {
        VX_CALL(vxReleaseArray(&old_points_arr[i]));
        VX_CALL(vxReleaseArray(&new_points_arr[i]));
        VX_CALL(vxReleasePyramid(&src_pyr1[i]));
        VX_CALL(vxReleasePyramid(&src_pyr2[i]));
        VX_CALL(vxReleaseNode(&node[i]));
    }
    VX_CALL(vxReleaseGraph(&graph));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_GRAPH_MAX_DATA_REF", &stats));
    ASSERT(stats.max_used_value == TIVX_GRAPH_MAX_DATA_REF);
    }
    else
    {
        printf("To fully test the TIVX_GRAPH_MAX_DATA_REF value, set it to 64 in include/tivx_config.h and re-run this test case\n");
    }

}

/* TIVX_DELAY_MAX_OBJECT */
TEST(tivxBoundary2, testDelayMaxObjectBoundary)
{
    vx_context context = context_->vx_context_;
    vx_delay   src_delay;
    vx_image   image;
    int i;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src_delay = vxCreateDelay(context, (vx_reference)image, TIVX_DELAY_MAX_OBJECT), VX_TYPE_DELAY);

    VX_CALL(vxReleaseDelay(&src_delay));

    VX_CALL(vxReleaseImage(&image));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_DELAY_MAX_OBJECT", &stats));
    ASSERT(stats.max_used_value == TIVX_DELAY_MAX_OBJECT);

}

/* TIVX_MATRIX_MAX_OBJECTS */
TEST(tivxBoundary, testMatrixBoundary)
{
    vx_context context = context_->vx_context_;
    vx_matrix   src_matrix[TIVX_MATRIX_MAX_OBJECTS];
    int i;

    for (i = 0; i < TIVX_MATRIX_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_matrix[i] = vxCreateMatrix(context, VX_TYPE_FLOAT32, 3, 3), VX_TYPE_MATRIX);
    }

    for (i = 0; i < TIVX_MATRIX_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseMatrix(&src_matrix[i]));
    }
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_MATRIX_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_MATRIX_MAX_OBJECTS);
}

/* TIVX_REMAP_MAX_OBJECTS */
TEST(tivxBoundary, testRemapBoundary)
{
    vx_context context = context_->vx_context_;
    vx_remap   src_remap[TIVX_REMAP_MAX_OBJECTS];
    int i;

    for (i = 0; i < TIVX_REMAP_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_remap[i] = vxCreateRemap(context, 32, 24, 16, 12), VX_TYPE_REMAP);
    }

    for (i = 0; i < TIVX_REMAP_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseRemap(&src_remap[i]));
    }
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_REMAP_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_REMAP_MAX_OBJECTS);

}

/* TIVX_SCALAR_MAX_OBJECTS */
TEST(tivxBoundary, testScalarBoundary)
{
    vx_context context = context_->vx_context_;
    vx_scalar   src_scalar[TIVX_SCALAR_MAX_OBJECTS];
    vx_int32 tmp;
    int i;

    for (i = 0; i < TIVX_SCALAR_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_scalar[i] = vxCreateScalar(context, VX_TYPE_INT32, &tmp), VX_TYPE_SCALAR);
    }

    for (i = 0; i < TIVX_SCALAR_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseScalar(&src_scalar[i]));
    }
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_SCALAR_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_SCALAR_MAX_OBJECTS);

}

/* TIVX_THRESHOLD_MAX_OBJECTS */
TEST(tivxBoundary, testThresholdBoundary)
{
    vx_context context = context_->vx_context_;
    vx_threshold   src_threshold[TIVX_THRESHOLD_MAX_OBJECTS];
    int i;

    for (i = 0; i < TIVX_THRESHOLD_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_threshold[i] = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8), VX_TYPE_THRESHOLD);
    }

    for (i = 0; i < TIVX_THRESHOLD_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseThreshold(&src_threshold[i]));
    }
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_THRESHOLD_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_THRESHOLD_MAX_OBJECTS);

}

/* TIVX_GRAPH_MAX_NODES */
/* TIVX_NODE_MAX_OBJECTS */
TEST(tivxBoundary, testNodeBoundary)
{
    vx_context context = context_->vx_context_;
    vx_kernel   src_kernel;
    vx_node   src_node[TIVX_GRAPH_MAX_NODES];
    int i;
    vx_graph graph = 0;

    ASSERT(TIVX_GRAPH_MAX_NODES == TIVX_NODE_MAX_OBJECTS);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(src_kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);

    for (i = 0; i < TIVX_GRAPH_MAX_NODES; i++)
    {
        ASSERT_VX_OBJECT(src_node[i] = vxCreateGenericNode(graph, src_kernel), VX_TYPE_NODE);
    }

    for (i = 0; i < TIVX_GRAPH_MAX_NODES; i++)
    {
        VX_CALL(vxReleaseNode(&src_node[i]));
    }

    VX_CALL(vxReleaseKernel(&src_kernel));
    VX_CALL(vxReleaseGraph(&graph));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_GRAPH_MAX_NODES", &stats));
    ASSERT(stats.max_used_value == TIVX_GRAPH_MAX_NODES);

}

TEST(tivxBoundary2, testReplicateBoundary)
{
    vx_context context = context_->vx_context_;
    vx_node   src_node[TIVX_GRAPH_MAX_NODES];
    vx_image  src_image[TIVX_GRAPH_MAX_NODES], dst_image[TIVX_GRAPH_MAX_NODES];
    int i;
    vx_graph graph = 0;
    /* Splitting up into pyramid and object array due to limitations on each */
    vx_pyramid   src_pyr[TIVX_GRAPH_MAX_NODES/2], dst_pyr[TIVX_GRAPH_MAX_NODES/2];
    vx_object_array src_object_array[TIVX_GRAPH_MAX_NODES/2], dst_object_array[TIVX_GRAPH_MAX_NODES/2];
    vx_image src_obj_arr_image = 0;
    vx_image dst_obj_arr_image = 0;

    ASSERT_VX_OBJECT(src_obj_arr_image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_obj_arr_image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    for (i = 0; i < TIVX_GRAPH_MAX_NODES/2; i++)
    {
        ASSERT_VX_OBJECT(src_object_array[i] = vxCreateObjectArray(context, (vx_reference)src_obj_arr_image, 2), VX_TYPE_OBJECT_ARRAY);
        ASSERT_VX_OBJECT(dst_object_array[i] = vxCreateObjectArray(context, (vx_reference)dst_obj_arr_image, 2), VX_TYPE_OBJECT_ARRAY);
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);


    for (i = 0; i < TIVX_GRAPH_MAX_NODES/2; i++)
    {
        ASSERT_VX_OBJECT(src_pyr[i] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(dst_pyr[i] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    }

    /* Replicating nodes */
    for (i = 0; i < TIVX_GRAPH_MAX_NODES/2; i++)
    {
        ASSERT_VX_OBJECT(src_image[i] = vxGetPyramidLevel((vx_pyramid)src_pyr[i], 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image[i] = vxGetPyramidLevel((vx_pyramid)dst_pyr[i], 0), VX_TYPE_IMAGE);
        vx_bool replicate[] = { vx_true_e, vx_true_e };
        ASSERT_VX_OBJECT(src_node[i] = vxBox3x3Node(graph, src_image[i], dst_image[i]), VX_TYPE_NODE);
        VX_CALL(vxReplicateNode(graph, src_node[i], replicate, 2));
    }

    for (i = 0; i < TIVX_GRAPH_MAX_NODES/2; i++)
    {
        ASSERT_VX_OBJECT(src_image[i+TIVX_GRAPH_MAX_NODES/2] = (vx_image)vxGetObjectArrayItem((vx_object_array)src_object_array[i], 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image[i+TIVX_GRAPH_MAX_NODES/2] = (vx_image)vxGetObjectArrayItem((vx_object_array)dst_object_array[i], 0), VX_TYPE_IMAGE);
        vx_bool replicate[] = { vx_true_e, vx_true_e };
        ASSERT_VX_OBJECT(src_node[i+TIVX_GRAPH_MAX_NODES/2] = vxBox3x3Node(graph, src_image[i+TIVX_GRAPH_MAX_NODES/2], dst_image[i+TIVX_GRAPH_MAX_NODES/2]), VX_TYPE_NODE);
        VX_CALL(vxReplicateNode(graph, src_node[i+TIVX_GRAPH_MAX_NODES/2], replicate, 2));
    }

    /* Releasing objects */
    for (i = 0; i < TIVX_GRAPH_MAX_NODES/2; i++)
    {
        VX_CALL(vxReleaseObjectArray(&src_object_array[i]));
        VX_CALL(vxReleaseObjectArray(&dst_object_array[i]));
        VX_CALL(vxReleasePyramid(&src_pyr[i]));
        VX_CALL(vxReleasePyramid(&dst_pyr[i]));
    }

    VX_CALL(vxReleaseImage(&src_obj_arr_image));
    VX_CALL(vxReleaseImage(&dst_obj_arr_image));

    for (i = 0; i < TIVX_GRAPH_MAX_NODES; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
        VX_CALL(vxReleaseImage(&dst_image[i]));
        VX_CALL(vxReleaseNode(&src_node[i]));
    }

    VX_CALL(vxReleaseGraph(&graph));
}

/* TIVX_NODE_MAX_REPLICATE */
TEST(tivxBoundary2, testReplicateNodeBoundary)
{
    vx_context context = context_->vx_context_;
    vx_node   src_node;
    vx_image  src_image, dst_image;
    int i;
    vx_graph graph = 0;
    /* Splitting up into pyramid and object array due to limitations on each */
    vx_pyramid   src_pyr, dst_pyr;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);


    ASSERT_VX_OBJECT(src_pyr = vxCreatePyramid(context, TIVX_NODE_MAX_REPLICATE, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    ASSERT_VX_OBJECT(dst_pyr = vxCreatePyramid(context, TIVX_NODE_MAX_REPLICATE, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    /* Replicating nodes */
    ASSERT_VX_OBJECT(src_image = vxGetPyramidLevel((vx_pyramid)src_pyr, 0), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxGetPyramidLevel((vx_pyramid)dst_pyr, 0), VX_TYPE_IMAGE);
    vx_bool replicate[] = { vx_true_e, vx_true_e };
    ASSERT_VX_OBJECT(src_node = vxBox3x3Node(graph, src_image, dst_image), VX_TYPE_NODE);
    VX_CALL(vxReplicateNode(graph, src_node, replicate, 2));

    /* Releasing objects */
    VX_CALL(vxReleasePyramid(&src_pyr));
    VX_CALL(vxReleasePyramid(&dst_pyr));

    VX_CALL(vxReleaseImage(&src_image));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseNode(&src_node));

    VX_CALL(vxReleaseGraph(&graph));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_NODE_MAX_REPLICATE", &stats));
    ASSERT(stats.max_used_value == TIVX_NODE_MAX_REPLICATE);

}

/* TIVX_GRAPH_MAX_PARAMS */
TEST(tivxBoundary2, testGraphParamBoundary)
{
    vx_context context = context_->vx_context_;
    vx_graph   graph;
    vx_uint32 i;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    vx_kernel kernels[] = {
        vxGetKernelByEnum(context, VX_KERNEL_CHANNEL_EXTRACT),
        vxGetKernelByEnum(context, VX_KERNEL_MEDIAN_3x3),
        vxGetKernelByEnum(context, VX_KERNEL_HARRIS_CORNERS)
    };

    vx_node nodes[dimof(kernels)] = {
        vxCreateGenericNode(graph, kernels[0]),
        vxCreateGenericNode(graph, kernels[1]),
        vxCreateGenericNode(graph, kernels[2])
    };

    vx_parameter parameters[] = {
        vxGetParameterByIndex(nodes[0], 0),
        vxGetParameterByIndex(nodes[0], 1),
        vxGetParameterByIndex(nodes[0], 2),
        vxGetParameterByIndex(nodes[1], 0),
        vxGetParameterByIndex(nodes[1], 1),
        vxGetParameterByIndex(nodes[2], 0),
        vxGetParameterByIndex(nodes[2], 1),
        vxGetParameterByIndex(nodes[2], 2),
        vxGetParameterByIndex(nodes[2], 3),
        vxGetParameterByIndex(nodes[2], 4),
        vxGetParameterByIndex(nodes[2], 5)
    };

    for (i = 0; i < TIVX_GRAPH_MAX_PARAMS; i++)
    {
        VX_CALL(vxAddParameterToGraph(graph, parameters[i]));
    }

    for (i = 0; i < dimof(kernels); i++)
    {
        VX_CALL(vxReleaseKernel(&kernels[i]));
        ASSERT(kernels[i] == NULL);
    }
    for (i = 0; i < dimof(nodes); i++)
    {
        VX_CALL(vxReleaseNode(&nodes[i]));
        ASSERT(nodes[i] == NULL);
    }

    for (i = 0; i < dimof(parameters); i++)
    {
        VX_CALL(vxReleaseParameter(&parameters[i]));
        ASSERT(parameters[i] == NULL);
    }
    VX_CALL(vxReleaseGraph(&graph));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_GRAPH_MAX_PARAMS", &stats));
    ASSERT(stats.max_used_value == TIVX_GRAPH_MAX_PARAMS);

}

/* TIVX_PARAMETER_MAX_OBJECTS */
TEST(tivxBoundary, testParameterBoundary)
{
    vx_context context = context_->vx_context_;
    vx_kernel   src_kernel;
    vx_node   src_node[32];
    vx_parameter src_parameter[TIVX_PARAMETER_MAX_OBJECTS];
    int i;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(src_kernel = vxGetKernelByEnum(context, VX_KERNEL_CHANNEL_EXTRACT), VX_TYPE_KERNEL);

    for (i = 0; i < 32; i++)
    {
        ASSERT_VX_OBJECT(src_node[i] = vxCreateGenericNode(graph, src_kernel), VX_TYPE_NODE);
    }

    for (i = 0; i < TIVX_PARAMETER_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_parameter[i] = vxGetParameterByIndex(src_node[0], 0), VX_TYPE_PARAMETER);
    }

    for (i = 0; i < TIVX_PARAMETER_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseParameter(&src_parameter[i]));
    }

    for (i = 0; i < 32; i++)
    {
        VX_CALL(vxReleaseNode(&src_node[i]));
    }

    VX_CALL(vxReleaseKernel(&src_kernel));
    VX_CALL(vxReleaseGraph(&graph));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_PARAMETER_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_PARAMETER_MAX_OBJECTS);

}

/* TIVX_GRAPH_MAX_OBJECTS */
TEST(tivxBoundary, testGraphBoundary)
{
    vx_context context = context_->vx_context_;
    vx_graph   src_graph[TIVX_GRAPH_MAX_OBJECTS];
    int i;

    for (i = 0; i < TIVX_GRAPH_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_graph[i] = vxCreateGraph(context), VX_TYPE_GRAPH);
    }

    for (i = 0; i < TIVX_GRAPH_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseGraph(&src_graph[i]));
    }
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_GRAPH_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_GRAPH_MAX_OBJECTS);

}

/* TIVX_OBJ_ARRAY_MAX_OBJECTS */
TEST(tivxBoundary, testObjectArray)
{
    vx_context context = context_->vx_context_;
    vx_object_array src_object_array[TIVX_OBJ_ARRAY_MAX_OBJECTS];
    int i;
    vx_image image = 0;
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    for (i = 0; i < TIVX_OBJ_ARRAY_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_object_array[i] = vxCreateObjectArray(context, (vx_reference)image, 2), VX_TYPE_OBJECT_ARRAY);
    }

    for (i = 0; i < TIVX_OBJ_ARRAY_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseObjectArray(&src_object_array[i]));
    }

    VX_CALL(vxReleaseImage(&image));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_OBJ_ARRAY_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_OBJ_ARRAY_MAX_OBJECTS);

}

/* TIVX_OBJ_ARRAY_MAX_OBJECTS */
TEST(tivxBoundary, testVirtualObjectArray)
{
    vx_context context = context_->vx_context_;
    vx_object_array src_object_array[TIVX_OBJ_ARRAY_MAX_OBJECTS];
    int i;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    vx_image image = 0;
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    for (i = 0; i < TIVX_OBJ_ARRAY_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_object_array[i] = vxCreateVirtualObjectArray(graph, (vx_reference)image, 2), VX_TYPE_OBJECT_ARRAY);
    }

    for (i = 0; i < TIVX_OBJ_ARRAY_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseObjectArray(&src_object_array[i]));
    }

    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseGraph(&graph));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_OBJ_ARRAY_MAX_OBJECTS", &stats));
    ASSERT(stats.max_used_value == TIVX_OBJ_ARRAY_MAX_OBJECTS);

}

/* TIVX_OBJECT_ARRAY_MAX_ITEMS */
TEST(tivxBoundary, testObjectArrayItems)
{
    vx_context context = context_->vx_context_;
    vx_object_array src_object_array;
    vx_image image = 0;
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src_object_array = vxCreateObjectArray(context, (vx_reference)image, TIVX_OBJECT_ARRAY_MAX_ITEMS), VX_TYPE_OBJECT_ARRAY);

    VX_CALL(vxReleaseObjectArray(&src_object_array));

    VX_CALL(vxReleaseImage(&image));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_OBJECT_ARRAY_MAX_ITEMS", &stats));
    ASSERT(stats.max_used_value == TIVX_OBJECT_ARRAY_MAX_ITEMS);

}

/* TIVX_OBJECT_ARRAY_MAX_ITEMS */
TEST(tivxBoundary, testVirtualObjectArrayItems)
{
    vx_context context = context_->vx_context_;
    vx_object_array src_object_array;
    vx_image image = 0;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src_object_array = vxCreateVirtualObjectArray(graph, (vx_reference)image, TIVX_OBJECT_ARRAY_MAX_ITEMS), VX_TYPE_OBJECT_ARRAY);

    VX_CALL(vxReleaseObjectArray(&src_object_array));

    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseGraph(&graph));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_OBJECT_ARRAY_MAX_ITEMS", &stats));
    ASSERT(stats.max_used_value == TIVX_OBJECT_ARRAY_MAX_ITEMS);

}

/* TIVX_CONTEXT_MAX_OBJECTS */
TEST(tivxBoundary, testContext)
{
    vx_context context = context_->vx_context_;

    vx_context context2 = vxCreateContext();

    ASSERT_EQ_PTR(context, context2);

    VX_CALL(vxReleaseContext(&context2));
}

/* TIVX_IMAGE_MAX_MAPS */
TEST(tivxBoundary, testMapImage)
{
    int i, w = 128, h = 128;
    vx_df_image f = VX_DF_IMAGE_U8;
    vx_context context = context_->vx_context_;
    vx_image image, image2;
    vx_imagepatch_addressing_t addr;
    vx_uint8 *pdata = 0;
    vx_rectangle_t rect = {0, 0, 1, 1};
    vx_map_id map_id;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(image  = vxCreateImage(context, w, h, f), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, w, h, f), VX_TYPE_IMAGE);

    /* image[0] gets 1 */

    // Verifying that it is not restricted to max image maps as long as it frees memory in vxUnmapImagePatch
    for (i = 0; i < TIVX_IMAGE_MAX_MAPS+1; i++)
    {
        pdata = NULL;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(image2, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
        *pdata = 1;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(image, map_id));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(image2, map_id));
    }

    for (i = 0; i < TIVX_IMAGE_MAX_MAPS; i++)
    {
        pdata = NULL;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(image2, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    }

    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseImage(&image2));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_IMAGE_MAX_MAPS", &stats));
    ASSERT(stats.max_used_value == TIVX_IMAGE_MAX_MAPS);

}

static
void* own_alloc_init_data_items(vx_enum item_type, vx_size num_items)
{
    vx_size i;
    vx_size item_size;
    void* p = 0;

    switch (item_type)
    {
    case VX_TYPE_KEYPOINT:          item_size = sizeof(vx_keypoint_t); break;

    default:
        break;
    }

    p = ct_alloc_mem(num_items * item_size);
    if (NULL == p)
        return p;

    for (i = 0; i < num_items; i++)
    {
        switch (item_type)
        {

        case VX_TYPE_KEYPOINT:
            {
                vx_keypoint_t kp = { (vx_int32)i, (vx_int32)(num_items - i), (1.0f / i), (1.0f / i), (1.0f / i), (vx_int32)i, (1.0f / i) };
                ((vx_keypoint_t*)p)[i] = kp;
            }
            break;

        default:
            break;
        }
    }

    return p;
}

/* TIVX_ARRAY_MAX_MAPS */
TEST(tivxBoundary, testMapArray)
{
    int i;
    vx_context context = context_->vx_context_;
    vx_array array, array2;
    vx_enum item_type = VX_TYPE_KEYPOINT;
    vx_size num_items = 10;
    vx_size item_size = 0;
    void* array_items = 0;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(array = vxCreateArray(context, item_type, 10), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(array2 = vxCreateArray(context, item_type, 10), VX_TYPE_ARRAY);

    /* 3. check if array's actual item_size corresponds to requested item_type size */
    VX_CALL(vxQueryArray(array, VX_ARRAY_ITEMSIZE, &item_size, sizeof(item_size)));

    array_items = own_alloc_init_data_items(item_type, num_items);
    ASSERT(NULL != array_items);

    VX_CALL(vxAddArrayItems(array, num_items, array_items, item_size));
    VX_CALL(vxAddArrayItems(array2, num_items, array_items, item_size));

    // Verifying that it is not restricted to max array maps as long as it frees memory in vxUnmapArrayRange
    for (i = 0; i < TIVX_ARRAY_MAX_MAPS+1; i++)
    {
        vx_size stride = 0;
        void* ptr = 0;
        vx_map_id map_id;
        VX_CALL(vxMapArrayRange(array, 0, num_items, &map_id, &stride, &ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
        VX_CALL(vxMapArrayRange(array2, 0, num_items, &map_id, &stride, &ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));

        VX_CALL(vxUnmapArrayRange(array, map_id));
        VX_CALL(vxUnmapArrayRange(array2, map_id));
    }

    for (i = 0; i < TIVX_ARRAY_MAX_MAPS; i++)
    {
        vx_size stride = 0;
        void* ptr = 0;
        vx_map_id map_id;
        VX_CALL(vxMapArrayRange(array, 0, num_items, &map_id, &stride, &ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
        VX_CALL(vxMapArrayRange(array2, 0, num_items, &map_id, &stride, &ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    }

    VX_CALL(vxReleaseArray(&array));
    VX_CALL(vxReleaseArray(&array2));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_ARRAY_MAX_MAPS", &stats));
    ASSERT(stats.max_used_value == TIVX_ARRAY_MAX_MAPS);

}

/* TIVX_GRAPH_MAX_HEAD_NODES */
/* TIVX_GRAPH_MAX_LEAF_NODES */
TEST(tivxBoundary, testHeadLeafNodes)
{
    int i;
    vx_image src_image[TIVX_GRAPH_MAX_HEAD_NODES], dst_image[TIVX_GRAPH_MAX_LEAF_NODES];
    vx_node node[TIVX_GRAPH_MAX_HEAD_NODES];
    vx_context context = context_->vx_context_;
    vx_graph graph;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for (i = 0; i < TIVX_GRAPH_MAX_HEAD_NODES; i++)
    {
        ASSERT_VX_OBJECT(src_image[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(node[i]      = vxNotNode(graph, src_image[i], dst_image[i]), VX_TYPE_NODE);
    }

    VX_CALL(vxVerifyGraph(graph));

    for (i = 0; i < TIVX_GRAPH_MAX_HEAD_NODES; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
        VX_CALL(vxReleaseImage(&dst_image[i]));
        VX_CALL(vxReleaseNode(&node[i]));
    }

    VX_CALL(vxReleaseGraph(&graph));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_GRAPH_MAX_HEAD_NODES", &stats));
    ASSERT(stats.max_used_value == TIVX_GRAPH_MAX_HEAD_NODES);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_GRAPH_MAX_LEAF_NODES", &stats));
    ASSERT(stats.max_used_value == TIVX_GRAPH_MAX_LEAF_NODES);

}

/* TIVX_CONTEXT_MAX_CONVOLUTION_DIM */
TEST(tivxBoundary, testConvolutionDimBoundary)
{
    vx_context context = context_->vx_context_;
    vx_size conv_max_dim = 0;

    VX_CALL(vxQueryContext(context, VX_CONTEXT_CONVOLUTION_MAX_DIMENSION, &conv_max_dim, sizeof(conv_max_dim)));

    ASSERT(conv_max_dim == 9);
}

/* TIVX_CONTEXT_MAX_OPTICALFLOWPYRLK_DIM */
TEST(tivxBoundary, testOpticalFlowDimBoundary)
{
    vx_context context = context_->vx_context_;
    vx_size opt_flow_max_dim = 0;

    VX_CALL(vxQueryContext(context, VX_CONTEXT_OPTICAL_FLOW_MAX_WINDOW_DIMENSION, &opt_flow_max_dim, sizeof(opt_flow_max_dim)));

    ASSERT(opt_flow_max_dim == 9);
}

/* TIVX_CONTEXT_MAX_NONLINEAR_DIM */
TEST(tivxBoundary, testNonlinearDimBoundary)
{
    vx_context context = context_->vx_context_;
    vx_size nonlinear_max_dim = 0;

    VX_CALL(vxQueryContext(context, VX_CONTEXT_NONLINEAR_MAX_DIMENSION, &nonlinear_max_dim, sizeof(nonlinear_max_dim)));

    ASSERT(nonlinear_max_dim == 9);
}

/* TIVX_CONTEXT_MAX_REFERENCES = 512 */
#define TIVX_INTERNAL_REFS 69
TEST(tivxBoundary, testReferenceBoundary)
{
    vx_context context = context_->vx_context_;
    vx_image     src_image[TIVX_IMAGE_MAX_OBJECTS];
    int i;

    if (TIVX_CONTEXT_MAX_REFERENCES == (TIVX_IMAGE_MAX_OBJECTS+TIVX_INTERNAL_REFS))
    {
        for (i = 0; i < TIVX_IMAGE_MAX_OBJECTS; i++)
        {
            ASSERT_VX_OBJECT(src_image[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }

        for (i = 0; i < TIVX_IMAGE_MAX_OBJECTS; i++)
        {
            VX_CALL(vxReleaseImage(&src_image[i]));
        }
        tivx_resource_stats_t stats;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_CONTEXT_MAX_REFERENCES", &stats));
        ASSERT(stats.max_used_value == TIVX_CONTEXT_MAX_REFERENCES);
    }
    else
    {
        printf("To fully test the TIVX_CONTEXT_MAX_REFERENCES value, set it to %d in include/tivx_config.h and re-run only this test case\n", TIVX_IMAGE_MAX_OBJECTS+TIVX_INTERNAL_REFS);
    }

}

/* TIVX_CONTEXT_MAX_USER_STRUCTS */
typedef struct _own_struct
{
    vx_uint32  some_uint;
    vx_float64 some_double;
} own_struct;

/* TIVX_CONTEXT_MAX_USER_STRUCTS */
TEST(tivxBoundary2, testUserStructBoundary)
{
    vx_context context = context_->vx_context_;
    int i;
    vx_enum item_type = VX_TYPE_INVALID;

    for (i = 0; i < TIVX_CONTEXT_MAX_USER_STRUCTS; i++)
    {
        item_type = vxRegisterUserStruct(context, sizeof(own_struct));
        ASSERT(item_type != VX_TYPE_INVALID);
    }

    item_type = vxRegisterUserStruct(context, sizeof(own_struct));
    ASSERT(item_type == VX_TYPE_INVALID);
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_CONTEXT_MAX_USER_STRUCTS", &stats));
    ASSERT(stats.max_used_value == TIVX_CONTEXT_MAX_USER_STRUCTS);

}

/* TIVX_IMAGE_MAX_MAPS */
TEST(tivxNegativeBoundary, negativeTestMapImage)
{
    int i, w = 128, h = 128;
    vx_df_image f = VX_DF_IMAGE_U8;
    vx_context context = context_->vx_context_;
    vx_image image;
    vx_imagepatch_addressing_t addr;
    vx_uint8 *pdata = 0;
    vx_rectangle_t rect = {0, 0, 1, 1};
    vx_map_id map_id;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(image = vxCreateImage(context, w, h, f), VX_TYPE_IMAGE);

    /* image[0] gets 1 */

    for (i = 0; i < TIVX_IMAGE_MAX_MAPS; i++)
    {
        pdata = NULL;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    }

    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxMapImagePatch(image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));

    VX_CALL(vxReleaseImage(&image));
}

/* TIVX_ARRAY_MAX_MAPS */
TEST(tivxNegativeBoundary, negativeTestMapArray)
{
    int i;
    vx_context context = context_->vx_context_;
    vx_array array;
    vx_enum item_type = VX_TYPE_KEYPOINT;
    vx_size num_items = 10;
    vx_size item_size = 0;
    void* array_items = 0;
    vx_size stride = 0;
    void* ptr = 0;
    vx_map_id map_id;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(array = vxCreateArray(context, item_type, 10), VX_TYPE_ARRAY);

    /* 3. check if array's actual item_size corresponds to requested item_type size */
    VX_CALL(vxQueryArray(array, VX_ARRAY_ITEMSIZE, &item_size, sizeof(item_size)));

    array_items = own_alloc_init_data_items(item_type, num_items);
    ASSERT(NULL != array_items);

    VX_CALL(vxAddArrayItems(array, num_items, array_items, item_size));

    for (i = 0; i < TIVX_ARRAY_MAX_MAPS; i++)
    {
        VX_CALL(vxMapArrayRange(array, 0, num_items, &map_id, &stride, &ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    }

    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxMapArrayRange(array, 0, num_items, &map_id, &stride, &ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));

    VX_CALL(vxReleaseArray(&array));
}

/* TIVX_OBJECT_ARRAY_MAX_ITEMS */
TEST(tivxNegativeBoundary, negativeTestObjectArrayItems)
{
    vx_context context = context_->vx_context_;
    vx_object_array src_object_array;
    vx_lut lut = 0;
    ASSERT_VX_OBJECT(lut = vxCreateLUT(context, VX_TYPE_UINT8, 256), VX_TYPE_LUT);

    EXPECT_VX_ERROR(src_object_array = vxCreateObjectArray(context, (vx_reference)lut, TIVX_OBJECT_ARRAY_MAX_ITEMS+1), VX_ERROR_NO_RESOURCES);

    VX_CALL(vxReleaseLUT(&lut));
}

/* TIVX_OBJECT_ARRAY_MAX_ITEMS */
TEST(tivxNegativeBoundary, negativeTestVirtualObjectArrayItems)
{
    vx_context context = context_->vx_context_;
    vx_object_array src_object_array;
    vx_lut lut = 0;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(lut = vxCreateLUT(context, VX_TYPE_UINT8, 256), VX_TYPE_LUT);

    EXPECT_VX_ERROR(src_object_array = vxCreateVirtualObjectArray(graph, (vx_reference)lut, TIVX_OBJECT_ARRAY_MAX_ITEMS+1), VX_ERROR_NO_RESOURCES);

    VX_CALL(vxReleaseLUT(&lut));
    VX_CALL(vxReleaseGraph(&graph));
}

/* TIVX_OBJ_ARRAY_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestObjectArray)
{
    vx_context context = context_->vx_context_;
    vx_object_array src_object_array[TIVX_OBJ_ARRAY_MAX_OBJECTS+1];
    int i;
    vx_image image = 0;
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    for (i = 0; i < TIVX_OBJ_ARRAY_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_object_array[i] = vxCreateObjectArray(context, (vx_reference)image, 1), VX_TYPE_OBJECT_ARRAY);
    }

    EXPECT_VX_ERROR(src_object_array[TIVX_OBJ_ARRAY_MAX_OBJECTS] = vxCreateObjectArray(context, (vx_reference)image, 1), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_OBJ_ARRAY_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseObjectArray(&src_object_array[i]));
    }

    VX_CALL(vxReleaseImage(&image));
}

/* TIVX_OBJ_ARRAY_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestVirtualObjectArray)
{
    vx_context context = context_->vx_context_;
    vx_object_array src_object_array[TIVX_OBJ_ARRAY_MAX_OBJECTS+1];
    int i;
    vx_image image = 0;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    for (i = 0; i < TIVX_OBJ_ARRAY_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_object_array[i] = vxCreateVirtualObjectArray(graph, (vx_reference)image, 1), VX_TYPE_OBJECT_ARRAY);
    }

    EXPECT_VX_ERROR(src_object_array[TIVX_OBJ_ARRAY_MAX_OBJECTS] = vxCreateVirtualObjectArray(graph, (vx_reference)image, 1), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_OBJ_ARRAY_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseObjectArray(&src_object_array[i]));
    }

    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseGraph(&graph));
}

/* TIVX_PARAMETER_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestParameterBoundary)
{
    vx_context context = context_->vx_context_;
    vx_kernel   src_kernel;
    vx_node   src_node[32];
    vx_parameter src_parameter[TIVX_PARAMETER_MAX_OBJECTS+1];
    int i;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(src_kernel = vxGetKernelByEnum(context, VX_KERNEL_CHANNEL_EXTRACT), VX_TYPE_KERNEL);

    for (i = 0; i < 32; i++)
    {
        ASSERT_VX_OBJECT(src_node[i] = vxCreateGenericNode(graph, src_kernel), VX_TYPE_NODE);
    }

    for (i = 0; i < TIVX_PARAMETER_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_parameter[i] = vxGetParameterByIndex(src_node[0], 0), VX_TYPE_PARAMETER);
    }

    EXPECT_VX_ERROR(src_parameter[TIVX_PARAMETER_MAX_OBJECTS] = vxGetParameterByIndex(src_node[0], 0), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_PARAMETER_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseParameter(&src_parameter[i]));
    }

    for (i = 0; i < 32; i++)
    {
        VX_CALL(vxReleaseNode(&src_node[i]));
    }

    VX_CALL(vxReleaseKernel(&src_kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

/* TIVX_GRAPH_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestGraphBoundary)
{
    vx_context context = context_->vx_context_;
    vx_graph   src_graph[TIVX_GRAPH_MAX_OBJECTS+1];
    int i;

    for (i = 0; i < TIVX_GRAPH_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_graph[i] = vxCreateGraph(context), VX_TYPE_GRAPH);
    }

    EXPECT_VX_ERROR(src_graph[TIVX_GRAPH_MAX_OBJECTS] = vxCreateGraph(context), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_GRAPH_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseGraph(&src_graph[i]));
    }
}

/* TIVX_GRAPH_MAX_NODES */
TEST(tivxNegativeBoundary, negativeTestNodeBoundary)
{
    vx_context context = context_->vx_context_;
    vx_kernel   src_kernel;
    vx_node   src_node[TIVX_GRAPH_MAX_NODES+1];
    int i;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(src_kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);

    for (i = 0; i < TIVX_GRAPH_MAX_NODES; i++)
    {
        ASSERT_VX_OBJECT(src_node[i] = vxCreateGenericNode(graph, src_kernel), VX_TYPE_NODE);
    }

    EXPECT_VX_ERROR(src_node[TIVX_GRAPH_MAX_NODES] = vxCreateGenericNode(graph, src_kernel), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_GRAPH_MAX_NODES; i++)
    {
        VX_CALL(vxReleaseNode(&src_node[i]));
    }

    VX_CALL(vxReleaseKernel(&src_kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

/* TIVX_DELAY_MAX_PRM_OBJECT */
/* Note: TIVX_NODE_MAX_IN_NODES and TIVX_NODE_MAX_OUT_NODES restrict this from being fully tested
 * Therefore, asserting that it must be >= to these values */
TEST(tivxNegativeBoundary2, negativeTestDelayMaxPrmBoundary)
{
    if ( (TIVX_DELAY_MAX_PRM_OBJECT == TIVX_NODE_MAX_IN_NODES) &&
         (TIVX_DELAY_MAX_PRM_OBJECT == TIVX_NODE_MAX_OUT_NODES) )
    {

        vx_context context = context_->vx_context_;
        vx_delay   src_delay;
        vx_graph   graph = 0;
        vx_image   image, in_image, out_image, out_image2[TIVX_DELAY_MAX_PRM_OBJECT+1];
        vx_node    box_nodes, med_nodes, med_nodes2[TIVX_DELAY_MAX_PRM_OBJECT+1];
        int i;

        ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        vx_kernel kernels[] = {
            vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3),
            vxGetKernelByEnum(context, VX_KERNEL_MEDIAN_3x3)
        };

        ASSERT_VX_OBJECT(in_image  = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out_image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(src_delay = vxCreateDelay(context, (vx_reference)image, 2), VX_TYPE_DELAY);

        for (i = 0; i < TIVX_DELAY_MAX_PRM_OBJECT+1; i++)
        {
            ASSERT_VX_OBJECT(out_image2[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }

        box_nodes = vxCreateGenericNode(graph, kernels[0]);
        med_nodes = vxCreateGenericNode(graph, kernels[1]);

        VX_CALL(vxSetParameterByIndex(box_nodes, 0, (vx_reference)in_image));
        VX_CALL(vxSetParameterByIndex(box_nodes, 1, (vx_reference)vxGetReferenceFromDelay(src_delay, 0)));
        VX_CALL(vxSetParameterByIndex(med_nodes, 0, (vx_reference)vxGetReferenceFromDelay(src_delay, -1)));
        VX_CALL(vxSetParameterByIndex(med_nodes, 1, (vx_reference)out_image));

        for (i = 0; i < TIVX_DELAY_MAX_PRM_OBJECT; i++)
        {
            med_nodes2[i] = vxCreateGenericNode(graph, kernels[1]);
            VX_CALL(vxSetParameterByIndex(med_nodes2[i], 0, (vx_reference)vxGetReferenceFromDelay(src_delay, -1)));
            VX_CALL(vxSetParameterByIndex(med_nodes2[i], 1, (vx_reference)out_image2[i]));
        }

        med_nodes2[TIVX_DELAY_MAX_PRM_OBJECT] = vxCreateGenericNode(graph, kernels[1]);
        EXPECT_NE_VX_STATUS(VX_SUCCESS, vxSetParameterByIndex(med_nodes2[TIVX_DELAY_MAX_PRM_OBJECT], 0, (vx_reference)vxGetReferenceFromDelay(src_delay, -1)));
        //VX_CALL(vxSetParameterByIndex(med_nodes2[TIVX_DELAY_MAX_PRM_OBJECT], 1, (vx_reference)out_image2[i]));

        VX_CALL(vxRegisterAutoAging(graph, src_delay));

        VX_CALL(vxReleaseImage(&in_image));
        VX_CALL(vxReleaseImage(&out_image));

        for (i = 0; i < TIVX_DELAY_MAX_PRM_OBJECT+1; i++)
        {
            VX_CALL(vxReleaseImage(&out_image2[i]));
        }

        VX_CALL(vxReleaseDelay(&src_delay));

        for (i = 0; i < dimof(kernels); i++)
        {
            VX_CALL(vxReleaseKernel(&kernels[i]));
        }

        VX_CALL(vxReleaseNode(&box_nodes));
        VX_CALL(vxReleaseNode(&med_nodes));

        for (i = 0; i < TIVX_DELAY_MAX_PRM_OBJECT+1; i++)
        {
            VX_CALL(vxReleaseNode(&med_nodes2[i]));
        }

        VX_CALL(vxReleaseImage(&image));

        VX_CALL(vxReleaseGraph(&graph));

    }
    else
    {
        ASSERT(TIVX_DELAY_MAX_PRM_OBJECT >= TIVX_NODE_MAX_IN_NODES);
        ASSERT(TIVX_DELAY_MAX_PRM_OBJECT >= TIVX_NODE_MAX_OUT_NODES); 
        printf("To fully test the TIVX_DELAY_MAX_PRM_OBJECT value, set it to %d in include/tivx_config.h and re-run only this test case\n", TIVX_NODE_MAX_IN_NODES);
    }
}

/* TIVX_NODE_MAX_REPLICATE */
TEST(tivxNegativeBoundary2, negativeTestReplicateBoundary)
{
    /* Asserting that max replicate nodes is greater than max allowed nodes
     * In this case, a negative test is impossible due to node restriction */
    ASSERT(TIVX_NODE_MAX_REPLICATE >= TIVX_GRAPH_MAX_NODES);
}

/* TIVX_GRAPH_MAX_PARAMS */
TEST(tivxNegativeBoundary2, negativeTestGraphParamBoundary)
{
    vx_context context = context_->vx_context_;
    vx_graph   graph;
    vx_uint32 i;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    vx_kernel kernels[] = {
        vxGetKernelByEnum(context, VX_KERNEL_CHANNEL_EXTRACT),
        vxGetKernelByEnum(context, VX_KERNEL_MEDIAN_3x3),
        vxGetKernelByEnum(context, VX_KERNEL_HARRIS_CORNERS)
    };

    vx_node nodes[dimof(kernels)] = {
        vxCreateGenericNode(graph, kernels[0]),
        vxCreateGenericNode(graph, kernels[1]),
        vxCreateGenericNode(graph, kernels[2])
    };

    vx_parameter parameters[] = {
        vxGetParameterByIndex(nodes[0], 0),
        vxGetParameterByIndex(nodes[0], 1),
        vxGetParameterByIndex(nodes[0], 2),
        vxGetParameterByIndex(nodes[1], 0),
        vxGetParameterByIndex(nodes[1], 1),
        vxGetParameterByIndex(nodes[2], 0),
        vxGetParameterByIndex(nodes[2], 1),
        vxGetParameterByIndex(nodes[2], 2),
        vxGetParameterByIndex(nodes[2], 3),
        vxGetParameterByIndex(nodes[2], 4),
        vxGetParameterByIndex(nodes[2], 5)
    };

    for (i = 0; i < TIVX_GRAPH_MAX_PARAMS; i++)
    {
        VX_CALL(vxAddParameterToGraph(graph, parameters[i]));
    }

    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxAddParameterToGraph(graph, parameters[i]));

    for (i = 0; i < dimof(kernels); i++)
    {
        VX_CALL(vxReleaseKernel(&kernels[i]));
        ASSERT(kernels[i] == NULL);
    }
    for (i = 0; i < dimof(nodes); i++)
    {
        VX_CALL(vxReleaseNode(&nodes[i]));
        ASSERT(nodes[i] == NULL);
    }

    for (i = 0; i < dimof(parameters); i++)
    {
        VX_CALL(vxReleaseParameter(&parameters[i]));
        ASSERT(parameters[i] == NULL);
    }
    VX_CALL(vxReleaseGraph(&graph));
}

/* TIVX_THRESHOLD_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestThresholdBoundary)
{
    vx_context context = context_->vx_context_;
    vx_threshold   src_threshold[TIVX_THRESHOLD_MAX_OBJECTS+1];
    int i;

    for (i = 0; i < TIVX_THRESHOLD_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_threshold[i] = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8), VX_TYPE_THRESHOLD);
    }

    EXPECT_VX_ERROR(src_threshold[TIVX_THRESHOLD_MAX_OBJECTS] = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_THRESHOLD_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseThreshold(&src_threshold[i]));
    }
}

/* TIVX_SCALAR_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestScalarBoundary)
{
    vx_context context = context_->vx_context_;
    vx_scalar   src_scalar[TIVX_SCALAR_MAX_OBJECTS+1];
    vx_int32 tmp;
    int i;

    for (i = 0; i < TIVX_SCALAR_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_scalar[i] = vxCreateScalar(context, VX_TYPE_INT32, &tmp), VX_TYPE_SCALAR);
    }

    EXPECT_VX_ERROR(src_scalar[TIVX_SCALAR_MAX_OBJECTS] = vxCreateScalar(context, VX_TYPE_INT32, &tmp), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_SCALAR_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseScalar(&src_scalar[i]));
    }
}

/* TIVX_REMAP_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestRemapBoundary)
{
    vx_context context = context_->vx_context_;
    vx_remap   src_remap[TIVX_REMAP_MAX_OBJECTS+1];
    int i;

    for (i = 0; i < TIVX_REMAP_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_remap[i] = vxCreateRemap(context, 32, 24, 16, 12), VX_TYPE_REMAP);
    }

    EXPECT_VX_ERROR(src_remap[TIVX_REMAP_MAX_OBJECTS] = vxCreateRemap(context, 32, 24, 16, 12), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_REMAP_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseRemap(&src_remap[i]));
    }
}

/* TIVX_MATRIX_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestMatrixBoundary)
{
    vx_context context = context_->vx_context_;
    vx_matrix   src_matrix[TIVX_MATRIX_MAX_OBJECTS+1];
    int i;

    for (i = 0; i < TIVX_MATRIX_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_matrix[i] = vxCreateMatrix(context, VX_TYPE_FLOAT32, 3, 3), VX_TYPE_MATRIX);
    }

    EXPECT_VX_ERROR(src_matrix[TIVX_MATRIX_MAX_OBJECTS] = vxCreateMatrix(context, VX_TYPE_FLOAT32, 3, 3), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_MATRIX_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseMatrix(&src_matrix[i]));
    }
}

/* TIVX_DELAY_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestDelayBoundary)
{
    vx_context context = context_->vx_context_;
    vx_delay   src_delay[TIVX_DELAY_MAX_OBJECTS+1];
    vx_image   image;
    int i;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    for (i = 0; i < TIVX_DELAY_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_delay[i] = vxCreateDelay(context, (vx_reference)image, 2), VX_TYPE_DELAY);
    }

    EXPECT_VX_ERROR(src_delay[TIVX_DELAY_MAX_OBJECTS] = vxCreateDelay(context, (vx_reference)image, 2), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_DELAY_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseDelay(&src_delay[i]));
    }

    VX_CALL(vxReleaseImage(&image));
}

/* TIVX_GRAPH_MAX_DELAYS */
TEST(tivxNegativeBoundary2, negativeTestGraphDelayBoundary)
{
    vx_context context = context_->vx_context_;
    vx_delay   src_delay[TIVX_GRAPH_MAX_DELAYS+1];
    vx_graph   graph = 0;
    vx_image   image, in_image[TIVX_GRAPH_MAX_DELAYS+1], out_image[TIVX_GRAPH_MAX_DELAYS+1];
    vx_node    box_nodes[TIVX_GRAPH_MAX_DELAYS+1], med_nodes[TIVX_GRAPH_MAX_DELAYS+1];
    int i;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    for (i = 0; i < TIVX_GRAPH_MAX_DELAYS+1; i++)
    {
        ASSERT_VX_OBJECT(in_image[i]  = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out_image[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src_delay[i] = vxCreateDelay(context, (vx_reference)image, 2), VX_TYPE_DELAY);
    }

    for (i = 0; i < TIVX_GRAPH_MAX_DELAYS+1; i++)
    {
    ASSERT_VX_OBJECT(box_nodes[i] = vxBox3x3Node(graph, in_image[i], (vx_image)vxGetReferenceFromDelay(src_delay[i], 0)), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(med_nodes[i] = vxMedian3x3Node(graph, (vx_image)vxGetReferenceFromDelay(src_delay[i], -1), out_image[i]), VX_TYPE_NODE);
    }

    for (i = 0; i < TIVX_GRAPH_MAX_DELAYS; i++)
    {
        VX_CALL(vxRegisterAutoAging(graph, src_delay[i]));
    }

    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxRegisterAutoAging(graph, src_delay[TIVX_GRAPH_MAX_DELAYS]));

    for (i = 0; i < TIVX_GRAPH_MAX_DELAYS+1; i++)
    {
        VX_CALL(vxReleaseDelay(&src_delay[i]));
        VX_CALL(vxReleaseImage(&in_image[i]));
        VX_CALL(vxReleaseImage(&out_image[i]));
    }

    VX_CALL(vxReleaseImage(&image));

    for (i = 0; i < TIVX_GRAPH_MAX_DELAYS+1; i++)
    {
        VX_CALL(vxReleaseNode(&box_nodes[i]));
        VX_CALL(vxReleaseNode(&med_nodes[i]));
    }

    VX_CALL(vxReleaseGraph(&graph));
}

/* TIVX_DELAY_MAX_OBJECT */
TEST(tivxNegativeBoundary2, negativeTestDelayMaxObjectBoundary)
{
    vx_context context = context_->vx_context_;
    vx_delay   src_delay;
    vx_image   image;
    int i;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    EXPECT_VX_ERROR(src_delay = vxCreateDelay(context, (vx_reference)image, TIVX_DELAY_MAX_OBJECT+1), VX_ERROR_INVALID_REFERENCE);

    VX_CALL(vxReleaseImage(&image));
}

/* TIVX_KERNEL_MAX_PARAMS */
TEST(tivxNegativeBoundary2, negativeTestKernelParamsBoundary)
{
    vx_context context = context_->vx_context_;
    int numParams = TIVX_KERNEL_MAX_PARAMS+1;

    vx_kernel kernel = 0;
    vx_size size = local_size_auto_alloc;

    EXPECT_VX_ERROR(kernel = vxAddUserKernel(
        context,
        VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME,
        VX_KERNEL_CONFORMANCE_TEST_OWN_USER,
        own_Kernel,
        numParams,
        own_ValidatorMetaFromRef,
        own_Initialize,
        own_Deinitialize), VX_ERROR_INVALID_PARAMETERS);
}

/* TIVX_GRAPH_MAX_DATA_REF */
/* Note: Due to additional max defines, I reduced the value of TIVX_GRAPH_MAX_DATA_REF to 64 */
#define OPT_NODE 7
/* Since there are 5 scalars per node and 48 is max scalars */
#define MAX_SCALE_NODES 6
TEST(tivxNegativeBoundary2, negativeTestGraphDataRefBoundary)
{
    vx_context context = context_->vx_context_;

    if (TIVX_GRAPH_MAX_DATA_REF == 64)
    {
        vx_pyramid src_pyr1[OPT_NODE], src_pyr2[OPT_NODE];
        vx_array old_points_arr[OPT_NODE];
        vx_array new_points_arr[OPT_NODE];
        vx_array final_new_points_arr = 0;
        vx_uint32  num_iter_val = 100;
        vx_float32 eps_val      = 0.001f;
        vx_bool   use_estimations_val = vx_true_e;
        vx_scalar eps[MAX_SCALE_NODES];
        vx_scalar num_iter[MAX_SCALE_NODES];
        vx_scalar use_estimations[MAX_SCALE_NODES];
        vx_size   winSize = 9;
        vx_graph graph = 0;
        vx_node node[OPT_NODE], node_ch;
        int width=640, height=480, i;
        vx_size num_points = 100;
        vx_image src1_ch, src2_ch, src3_ch, src4_ch, dst_ch;

        ASSERT_VX_OBJECT(src1_ch = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src2_ch = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src3_ch = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src4_ch = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_ch  = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        for (i = 0; i < MAX_SCALE_NODES; i++)
        {
            ASSERT_VX_OBJECT(eps[i]             = vxCreateScalar(context, VX_TYPE_FLOAT32, &eps_val), VX_TYPE_SCALAR);
            ASSERT_VX_OBJECT(num_iter[i]        = vxCreateScalar(context, VX_TYPE_UINT32, &num_iter_val), VX_TYPE_SCALAR);
            ASSERT_VX_OBJECT(use_estimations[i] = vxCreateScalar(context, VX_TYPE_BOOL, &use_estimations_val), VX_TYPE_SCALAR);
        }

        /* 7 total data references -- not sure if multiple levels of pyramid count though */
        for (i = 0; i < OPT_NODE; i++)
        {
            ASSERT_VX_OBJECT(src_pyr1[i] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
            ASSERT_VX_OBJECT(src_pyr2[i] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

            ASSERT_VX_OBJECT(old_points_arr[i] = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);
            ASSERT_VX_OBJECT(new_points_arr[i] = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);
        }

        ASSERT_VX_OBJECT(final_new_points_arr = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        for (i = 0; i < OPT_NODE-1; i++)
        {
            if (i < MAX_SCALE_NODES)
            {
                ASSERT_VX_OBJECT(node[i] = vxOpticalFlowPyrLKNode(
                    graph,
                    src_pyr1[i], src_pyr2[i],
                    old_points_arr[i], old_points_arr[i], new_points_arr[i],
                    VX_TERM_CRITERIA_BOTH, eps[i], num_iter[i], use_estimations[i], winSize), VX_TYPE_NODE);
            }
            else
            {
                ASSERT_VX_OBJECT(node[i] = vxOpticalFlowPyrLKNode(
                    graph,
                    src_pyr1[i], src_pyr2[i],
                    old_points_arr[i], old_points_arr[i], new_points_arr[i],
                    VX_TERM_CRITERIA_BOTH, eps[MAX_SCALE_NODES-1], num_iter[MAX_SCALE_NODES-1], use_estimations[MAX_SCALE_NODES-1], winSize), VX_TYPE_NODE);
            }
        }

        ASSERT_VX_OBJECT(node[OPT_NODE-1] = vxOpticalFlowPyrLKNode(
            graph,
            src_pyr1[OPT_NODE-1], src_pyr2[OPT_NODE-1],
            old_points_arr[OPT_NODE-1], old_points_arr[OPT_NODE-1], final_new_points_arr,
            VX_TERM_CRITERIA_BOTH, eps[MAX_SCALE_NODES-1], num_iter[MAX_SCALE_NODES-1], use_estimations[MAX_SCALE_NODES-1], winSize), VX_TYPE_NODE);

        ASSERT_VX_OBJECT(node_ch = vxChannelCombineNode(graph, src1_ch, src2_ch, src3_ch, src4_ch, dst_ch), VX_TYPE_NODE);

        EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

        VX_CALL(vxReleaseImage(&src1_ch));
        VX_CALL(vxReleaseImage(&src2_ch));
        VX_CALL(vxReleaseImage(&src3_ch));
        VX_CALL(vxReleaseImage(&src4_ch));
        VX_CALL(vxReleaseImage(&dst_ch));

        VX_CALL(vxReleaseArray(&final_new_points_arr));

        for (i = 0; i < MAX_SCALE_NODES; i++)
        {
            VX_CALL(vxReleaseScalar(&eps[i]));
            VX_CALL(vxReleaseScalar(&num_iter[i]));
            VX_CALL(vxReleaseScalar(&use_estimations[i]));
        }

        for (i = 0; i < OPT_NODE; i++)
        {
            VX_CALL(vxReleaseArray(&old_points_arr[i]));
            VX_CALL(vxReleaseArray(&new_points_arr[i]));
            VX_CALL(vxReleasePyramid(&src_pyr1[i]));
            VX_CALL(vxReleasePyramid(&src_pyr2[i]));
            VX_CALL(vxReleaseNode(&node[i]));
        }
        VX_CALL(vxReleaseGraph(&graph));
    }
    else
    {
        printf("To fully test the TIVX_GRAPH_MAX_DATA_REF value, set it to 64 in include/tivx_config.h and re-run this test case\n");
    }
}

/* TIVX_LUT_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestLUTBoundary)
{
    vx_context context = context_->vx_context_;
    vx_lut   src_lut[TIVX_LUT_MAX_OBJECTS+1];
    int i;

    for (i = 0; i < TIVX_LUT_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_lut[i] = vxCreateLUT(context, VX_TYPE_UINT8, 256), VX_TYPE_LUT);
    }

    EXPECT_VX_ERROR(src_lut[TIVX_LUT_MAX_OBJECTS] = vxCreateLUT(context, VX_TYPE_UINT8, 256), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_LUT_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseLUT(&src_lut[i]));
    }
}

/* TIVX_DISTRIBUTION_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestDistributionBoundary)
{
    vx_context context = context_->vx_context_;
    vx_distribution   src_dist[TIVX_DISTRIBUTION_MAX_OBJECTS+1];
    int i;

    for (i = 0; i < TIVX_DISTRIBUTION_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_dist[i] = vxCreateDistribution(context, 100, 5, 200), VX_TYPE_DISTRIBUTION);
    }

    EXPECT_VX_ERROR(src_dist[TIVX_DISTRIBUTION_MAX_OBJECTS] = vxCreateDistribution(context, 100, 5, 200), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_DISTRIBUTION_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseDistribution(&src_dist[i]));
    }
}

/* TIVX_ARRAY_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestArrayBoundary)
{
    vx_context context = context_->vx_context_;
    vx_array   src_array[TIVX_ARRAY_MAX_OBJECTS+1];
    int i;

    for (i = 0; i < TIVX_ARRAY_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_array[i] = vxCreateArray(context, VX_TYPE_KEYPOINT, 4), VX_TYPE_ARRAY);
    }

    EXPECT_VX_ERROR(src_array[TIVX_ARRAY_MAX_OBJECTS] = vxCreateArray(context, VX_TYPE_KEYPOINT, 4), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_ARRAY_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseArray(&src_array[i]));
    }
}

/* TIVX_ARRAY_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestVirtualArrayBoundary)
{
    vx_context context = context_->vx_context_;
    vx_array   src_array[TIVX_ARRAY_MAX_OBJECTS+1];
    int i;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for (i = 0; i < TIVX_ARRAY_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_array[i] = vxCreateVirtualArray(graph, VX_TYPE_KEYPOINT, 4), VX_TYPE_ARRAY);
    }

    EXPECT_VX_ERROR(src_array[TIVX_ARRAY_MAX_OBJECTS] = vxCreateVirtualArray(graph, VX_TYPE_KEYPOINT, 4), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_ARRAY_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseArray(&src_array[i]));
    }

    VX_CALL(vxReleaseGraph(&graph));
}

/* TIVX_CONVOLUTION_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestConvolutionBoundary)
{
    vx_context context = context_->vx_context_;
    vx_convolution   src_conv[TIVX_CONVOLUTION_MAX_OBJECTS+1];
    int i;

    for (i = 0; i < TIVX_CONVOLUTION_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_conv[i] = vxCreateConvolution(context, 3, 3), VX_TYPE_CONVOLUTION);
    }

    EXPECT_VX_ERROR(src_conv[TIVX_CONVOLUTION_MAX_OBJECTS] = vxCreateConvolution(context, 3, 3), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_CONVOLUTION_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseConvolution(&src_conv[i]));
    }
}

/* TIVX_PYRAMID_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestPyramidBoundary)
{
    vx_context context = context_->vx_context_;
    vx_pyramid   src_pyr[TIVX_PYRAMID_MAX_OBJECTS+1];
    int i;

    for (i = 0; i < TIVX_PYRAMID_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_pyr[i] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    }

    EXPECT_VX_ERROR(src_pyr[TIVX_PYRAMID_MAX_OBJECTS] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_PYRAMID_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleasePyramid(&src_pyr[i]));
    }
}

/* TIVX_PYRAMID_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestVirtualPyramidBoundary)
{
    vx_context context = context_->vx_context_;
    vx_pyramid   src_pyr[TIVX_PYRAMID_MAX_OBJECTS+1];
    int i;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for (i = 0; i < TIVX_PYRAMID_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_pyr[i] = vxCreateVirtualPyramid(graph, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    }

    EXPECT_VX_ERROR(src_pyr[TIVX_PYRAMID_MAX_OBJECTS] = vxCreateVirtualPyramid(graph, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_PYRAMID_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleasePyramid(&src_pyr[i]));
    }

    VX_CALL(vxReleaseGraph(&graph));
}

/* TIVX_IMAGE_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestImageBoundary)
{
    vx_context context = context_->vx_context_;
    vx_image   src_image[TIVX_IMAGE_MAX_OBJECTS+1];
    int i;

    for (i = 0; i < TIVX_IMAGE_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_image[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    EXPECT_VX_ERROR(src_image[TIVX_IMAGE_MAX_OBJECTS] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_IMAGE_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
    }
}

/* TIVX_IMAGE_MAX_SUBIMAGES */
TEST(tivxNegativeBoundary, negativeTestSubImageBoundary)
{
    vx_context context = context_->vx_context_;
    vx_image   src_image;
    vx_image   subimage[TIVX_IMAGE_MAX_SUBIMAGES+1];
    int fullimage_size = 680;
    int subimage_size = sqrt((640*640)/TIVX_IMAGE_MAX_SUBIMAGES);
    int startx = 0;
    int starty = 0;
    int endx   = subimage_size;
    int endy   = subimage_size;
    int i;

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, fullimage_size, fullimage_size, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    for (i = 0; i < TIVX_IMAGE_MAX_SUBIMAGES; i++)
    {
        if (i == 0)
        {
            startx = 0;
            starty = 0;
            endx   = subimage_size;
            endy   = subimage_size;
        }
        else if (i < (TIVX_IMAGE_MAX_SUBIMAGES / 2) )
        {
            if ( (i % 2) == 0 )
            {
                startx += subimage_size;
                endx   += subimage_size;
                starty -= subimage_size;
                endy   -= subimage_size;
            }
            else
            {
                starty += subimage_size;
                endy   += subimage_size;
            }
        }
        else
        {
            if (i == (TIVX_IMAGE_MAX_SUBIMAGES / 2) )
            {
                startx = 0;
                starty = 2*subimage_size;
                endx   = subimage_size;
                endy   = 2*subimage_size + subimage_size;
            }
            else if ( (i % 2) == 0 )
            {
                startx += subimage_size;
                endx   += subimage_size;
                starty -= subimage_size;
                endy   -= subimage_size;
            }
            else
            {
                starty += subimage_size;
                endy   += subimage_size;
            }
        }
        vx_rectangle_t rect = {startx, starty, endx, endy};
        ASSERT_VX_OBJECT(subimage[i] = vxCreateImageFromROI(src_image, &rect), VX_TYPE_IMAGE);
    }

    vx_rectangle_t rect = {640, 640, fullimage_size, fullimage_size};
    EXPECT_VX_ERROR(subimage[TIVX_IMAGE_MAX_SUBIMAGES] = vxCreateImageFromROI(src_image, &rect), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_IMAGE_MAX_SUBIMAGES; i++)
    {
        VX_CALL(vxReleaseImage(&subimage[i]));
    }
    VX_CALL(vxReleaseImage(&src_image));
    tivx_resource_stats_t stats;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueryResourceStats("TIVX_IMAGE_MAX_SUBIMAGES", &stats));
    ASSERT(stats.max_used_value == TIVX_IMAGE_MAX_SUBIMAGES);

}

/* TIVX_IMAGE_MAX_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestVirtualImageBoundary)
{
    vx_context context = context_->vx_context_;
    vx_image   src_image[TIVX_IMAGE_MAX_OBJECTS+1];
    int i;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for (i = 0; i < TIVX_IMAGE_MAX_OBJECTS; i++)
    {
        ASSERT_VX_OBJECT(src_image[i] = vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    EXPECT_VX_ERROR(src_image[TIVX_IMAGE_MAX_OBJECTS] = vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_U8), VX_ERROR_NO_RESOURCES);

    for (i = 0; i < TIVX_IMAGE_MAX_OBJECTS; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
    }

    VX_CALL(vxReleaseGraph(&graph));
}

/* TIVX_PYRAMID_MAX_LEVEL_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestPyramidLevelBoundary)
{
    vx_context context = context_->vx_context_;
    vx_pyramid   src_pyr;

    EXPECT_VX_ERROR(src_pyr = vxCreatePyramid(context, TIVX_PYRAMID_MAX_LEVEL_OBJECTS+1, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_ERROR_NO_RESOURCES);
}

/* TIVX_PYRAMID_MAX_LEVELS_ORB */
TEST(tivxNegativeBoundary, negativeTestOrbPyramidLevelBoundary)
{
    vx_context context = context_->vx_context_;
    vx_pyramid   src_pyr;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    EXPECT_VX_ERROR(src_pyr = vxCreatePyramid(context, TIVX_PYRAMID_MAX_LEVELS_ORB+1, VX_SCALE_PYRAMID_ORB, 16, 16, VX_DF_IMAGE_U8), VX_ERROR_NO_RESOURCES);

    EXPECT_VX_ERROR(src_pyr = vxCreateVirtualPyramid(graph, TIVX_PYRAMID_MAX_LEVELS_ORB+1, VX_SCALE_PYRAMID_ORB, 16, 16, VX_DF_IMAGE_U8), VX_ERROR_NO_RESOURCES);

    VX_CALL(vxReleaseGraph(&graph));
}

/* TIVX_PYRAMID_MAX_LEVEL_OBJECTS */
TEST(tivxNegativeBoundary, negativeTestVirtualPyramidLevelBoundary)
{
    vx_context context = context_->vx_context_;
    vx_pyramid   src_pyr;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    EXPECT_VX_ERROR(src_pyr = vxCreateVirtualPyramid(graph, TIVX_PYRAMID_MAX_LEVEL_OBJECTS+1, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_ERROR_NO_RESOURCES);

    VX_CALL(vxReleaseGraph(&graph));
}

/* TIVX_GRAPH_MAX_HEAD_NODES */
TEST(tivxNegativeBoundary, negativeTestHeadNodes)
{
    int i;
    vx_image src_image[TIVX_GRAPH_MAX_HEAD_NODES+1], dst_image[TIVX_GRAPH_MAX_HEAD_NODES+1];
    vx_node node[TIVX_GRAPH_MAX_HEAD_NODES+1];
    vx_context context = context_->vx_context_;
    vx_graph graph;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for (i = 0; i < TIVX_GRAPH_MAX_HEAD_NODES+1; i++)
    {
        ASSERT_VX_OBJECT(src_image[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(node[i]      = vxNotNode(graph, src_image[i], dst_image[i]), VX_TYPE_NODE);
    }

    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    for (i = 0; i < TIVX_GRAPH_MAX_HEAD_NODES+1; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
        VX_CALL(vxReleaseImage(&dst_image[i]));
        VX_CALL(vxReleaseNode(&node[i]));
    }

    VX_CALL(vxReleaseGraph(&graph));
}

/* TIVX_GRAPH_MAX_LEAF_NODES */
TEST(tivxNegativeBoundary, negativeTestLeafNodes)
{
    int i;
    vx_image src_image[TIVX_GRAPH_MAX_LEAF_NODES], dst_image[TIVX_GRAPH_MAX_LEAF_NODES+2], sobel_x, sobel_y;
    vx_scalar scalar_shift;
    vx_node node[TIVX_GRAPH_MAX_LEAF_NODES+2];
    vx_int32 tmp = 0;
    vx_context context = context_->vx_context_;
    vx_graph graph;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_shift = vxCreateScalar(context, VX_TYPE_INT32, &tmp), VX_TYPE_SCALAR);

    for (i = 0; i < 7; i++)
    {
        ASSERT_VX_OBJECT(src_image[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(node[i]      = vxNotNode(graph, src_image[i], dst_image[i]), VX_TYPE_NODE);
    }
    ASSERT_VX_OBJECT(src_image[TIVX_GRAPH_MAX_LEAF_NODES-1] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(sobel_x      = vxCreateImage(context, 16, 16, VX_DF_IMAGE_S16),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(sobel_y      = vxCreateImage(context, 16, 16, VX_DF_IMAGE_S16),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image[TIVX_GRAPH_MAX_LEAF_NODES-1] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image[TIVX_GRAPH_MAX_LEAF_NODES] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node[TIVX_GRAPH_MAX_LEAF_NODES-1] = vxSobel3x3Node(graph, src_image[TIVX_GRAPH_MAX_LEAF_NODES-1], sobel_x, sobel_y), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node[TIVX_GRAPH_MAX_LEAF_NODES] = vxConvertDepthNode(graph, sobel_x, dst_image[TIVX_GRAPH_MAX_LEAF_NODES-1], VX_CONVERT_POLICY_WRAP, scalar_shift), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node[TIVX_GRAPH_MAX_LEAF_NODES+1] = vxConvertDepthNode(graph, sobel_y, dst_image[TIVX_GRAPH_MAX_LEAF_NODES], VX_CONVERT_POLICY_WRAP, scalar_shift), VX_TYPE_NODE);

    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseScalar(&scalar_shift));
    for (i = 0; i < TIVX_GRAPH_MAX_LEAF_NODES-1; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
        VX_CALL(vxReleaseImage(&dst_image[i]));
        VX_CALL(vxReleaseNode(&node[i]));
    }
    VX_CALL(vxReleaseImage(&src_image[TIVX_GRAPH_MAX_LEAF_NODES-1]));
    VX_CALL(vxReleaseImage(&sobel_x));
    VX_CALL(vxReleaseImage(&sobel_y));
    VX_CALL(vxReleaseImage(&dst_image[TIVX_GRAPH_MAX_LEAF_NODES-1]));
    VX_CALL(vxReleaseImage(&dst_image[TIVX_GRAPH_MAX_LEAF_NODES]));
    VX_CALL(vxReleaseNode(&node[TIVX_GRAPH_MAX_LEAF_NODES-1]));
    VX_CALL(vxReleaseNode(&node[TIVX_GRAPH_MAX_LEAF_NODES]));
    VX_CALL(vxReleaseNode(&node[TIVX_GRAPH_MAX_LEAF_NODES+1]));

    VX_CALL(vxReleaseGraph(&graph));
}

/* TIVX_CONTEXT_MAX_REFERENCES = 512 */
TEST(tivxNegativeBoundary, negativeTestReferenceBoundary)
{
    vx_context context = context_->vx_context_;
    vx_image   src_image[TIVX_IMAGE_MAX_OBJECTS+1];
    int i;

    if (TIVX_CONTEXT_MAX_REFERENCES == (TIVX_IMAGE_MAX_OBJECTS+TIVX_INTERNAL_REFS))
    {
        for (i = 0; i < TIVX_IMAGE_MAX_OBJECTS; i++)
        {
            ASSERT_VX_OBJECT(src_image[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }

        EXPECT_VX_ERROR(src_image[TIVX_IMAGE_MAX_OBJECTS] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_ERROR_NO_RESOURCES);

        for (i = 0; i < TIVX_IMAGE_MAX_OBJECTS; i++)
        {
            VX_CALL(vxReleaseImage(&src_image[i]));
        }
    }
    else
    {
        printf("To fully test the TIVX_CONTEXT_MAX_REFERENCES value, set it to %d in include/tivx_config.h and re-run only this test case\n", TIVX_IMAGE_MAX_OBJECTS+TIVX_INTERNAL_REFS);
    }
}

TESTCASE_TESTS(tivxBoundary,
        testImageBoundary,
        testVirtualImageBoundary,
        testPyramidBoundary,
        testVirtualPyramidBoundary,
        testPyramidLevelBoundary,
        testVirtualPyramidLevelBoundary,
        testArrayBoundary,
        testVirtualArrayBoundary,
        testConvolutionBoundary,
        testDistributionBoundary,
        testLUTBoundary,
        testDelayBoundary,
        testMatrixBoundary,
        testRemapBoundary,
        testScalarBoundary,
        testThresholdBoundary,
        testNodeBoundary,
        testParameterBoundary,
        testGraphBoundary,
        testObjectArray,
        testVirtualObjectArray,
        testObjectArrayItems,
        testVirtualObjectArrayItems,
        testContext,
        testMapImage,
        testMapArray,
        testHeadLeafNodes,
        testConvolutionDimBoundary,
        testOpticalFlowDimBoundary,
        testNonlinearDimBoundary,
        testReferenceBoundary
        )

TESTCASE_TESTS(tivxBoundary2,
        testUserStructBoundary,
        testOrbPyramidLevelBoundary,
        testSubImageBoundary,
        testReplicateBoundary,
        testReplicateNodeBoundary,
        testGraphParamBoundary,
        testGraphDelayBoundary,
        testDelayMaxObjectBoundary,
        testDelayMaxPrmBoundary,
        testGraphDataRefBoundary,
        testKernelParamsBoundary
        )


TESTCASE_TESTS(tivxNegativeBoundary,
        negativeTestObjectArrayItems,
        negativeTestVirtualObjectArrayItems,
        negativeTestObjectArray,
        negativeTestVirtualObjectArray,
        negativeTestParameterBoundary,
        negativeTestGraphBoundary,
        negativeTestNodeBoundary,
        negativeTestPyramidLevelBoundary,
        negativeTestVirtualPyramidLevelBoundary,
        negativeTestThresholdBoundary,
        negativeTestScalarBoundary,
        negativeTestRemapBoundary,
        negativeTestMatrixBoundary,
        negativeTestDelayBoundary,
        negativeTestLUTBoundary,
        negativeTestDistributionBoundary,
        negativeTestConvolutionBoundary,
        negativeTestArrayBoundary,
        negativeTestVirtualArrayBoundary,
        negativeTestPyramidBoundary,
        negativeTestImageBoundary,
        negativeTestVirtualPyramidBoundary,
        negativeTestVirtualImageBoundary,
        negativeTestHeadNodes,
        negativeTestLeafNodes,
        negativeTestReferenceBoundary,
        negativeTestOrbPyramidLevelBoundary,
        negativeTestSubImageBoundary,
        negativeTestMapImage,
        negativeTestMapArray
        )

TESTCASE_TESTS(tivxNegativeBoundary2,
        negativeTestReplicateBoundary,
        negativeTestGraphParamBoundary,
        negativeTestGraphDelayBoundary,
        negativeTestDelayMaxObjectBoundary,
        negativeTestGraphDataRefBoundary,
        negativeTestKernelParamsBoundary,
        negativeTestDelayMaxPrmBoundary
        )

