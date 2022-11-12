/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
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

/*!
 * \file
 * \brief The sample implementation of the immediate mode calls.
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vxu.h>
#include <vx_internal.h>

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
static vx_status setNodeTarget(vx_node node);
static vx_status vx_isBorderModeSupported(vx_enum mode_to_check, const vx_enum supported_modes[], vx_size num_modes);
static vx_status vx_useImmediateBorderMode(vx_context context, vx_node node, const vx_enum supported_modes[], vx_size num_modes);

static vx_status setNodeTarget(vx_node node)
{
    return ownSetNodeImmTarget(node);
}

VX_API_ENTRY vx_status VX_API_CALL vxuColorConvert(vx_context context, vx_image src, vx_image dst)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxColorConvertNode(graph, src, dst);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuChannelExtract(vx_context context, vx_image src, vx_enum channel, vx_image dst)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxChannelExtractNode(graph, src, channel, dst);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuChannelCombine(vx_context context,
                            vx_image plane0,
                            vx_image plane1,
                            vx_image plane2,
                            vx_image plane3,
                            vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxChannelCombineNode(graph, plane0, plane1, plane2, plane3, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

static const vx_enum border_modes_2[] =
{
    (vx_enum)VX_BORDER_UNDEFINED,
    (vx_enum)VX_BORDER_CONSTANT
};

static const vx_enum border_modes_3[] =
{
    (vx_enum)VX_BORDER_UNDEFINED,
    (vx_enum)VX_BORDER_CONSTANT,
    (vx_enum)VX_BORDER_REPLICATE
};

static vx_status vx_isBorderModeSupported(vx_enum mode_to_check, const vx_enum supported_modes[], vx_size num_modes)
{
    vx_status status = (vx_status)VX_ERROR_NOT_SUPPORTED;
    vx_uint32 m = 0;
    for ( m = 0; m < num_modes; m++)
    {
        if (mode_to_check == supported_modes[m])
        {
            status = (vx_status)VX_SUCCESS;
            break;
        }
    }
    return status;
}

static vx_status vx_useImmediateBorderMode(vx_context context, vx_node node, const vx_enum supported_modes[], vx_size num_modes)
{
    vx_border_t border;
    vx_status status = vxQueryContext(context, (vx_enum)VX_CONTEXT_IMMEDIATE_BORDER, &border, sizeof(border));
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vx_isBorderModeSupported(border.mode, supported_modes, num_modes);
        if (status == (vx_status)VX_ERROR_NOT_SUPPORTED)
        {
            vx_enum policy;
            status = vxQueryContext(context, (vx_enum)VX_CONTEXT_IMMEDIATE_BORDER_POLICY, &policy, sizeof(policy));
            if (status == (vx_status)VX_SUCCESS)
            {
                switch (policy)
                {
                    case (vx_enum)VX_BORDER_POLICY_DEFAULT_TO_UNDEFINED:
                        border.mode = (vx_enum)VX_BORDER_UNDEFINED;
                        status = (vx_status)VX_SUCCESS;
                        break;
                    case (vx_enum)VX_BORDER_POLICY_RETURN_ERROR:
                        status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                        break;
                    default:
                        status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                        break;
                }
            }
        }
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vxSetNodeAttribute(node, (vx_enum)VX_NODE_BORDER, &border, sizeof(border));
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuSobel3x3(vx_context context, vx_image src, vx_image output_x, vx_image output_y)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxSobel3x3Node(graph, src, output_x, output_y);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, border_modes_3, dimof(border_modes_3));
            if (status == (vx_status)VX_SUCCESS)
            {
                status = setNodeTarget(node);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuMagnitude(vx_context context, vx_image grad_x, vx_image grad_y, vx_image dst)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxMagnitudeNode(graph, grad_x, grad_y, dst);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuPhase(vx_context context, vx_image grad_x, vx_image grad_y, vx_image dst)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxPhaseNode(graph, grad_x, grad_y, dst);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuScaleImage(vx_context context, vx_image src, vx_image dst, vx_enum type)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxScaleImageNode(graph, src, dst, type);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, border_modes_3, dimof(border_modes_3));
            if (status == (vx_status)VX_SUCCESS)
            {
                status = setNodeTarget(node);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuTableLookup(vx_context context, vx_image input, vx_lut lut, vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxTableLookupNode(graph, input, lut, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuHistogram(vx_context context, vx_image input, vx_distribution distribution)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxHistogramNode(graph, input, distribution);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuEqualizeHist(vx_context context, vx_image input, vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxEqualizeHistNode(graph, input, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuAbsDiff(vx_context context, vx_image in1, vx_image in2, vx_image out)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxAbsDiffNode(graph, in1, in2, out);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuMeanStdDev(vx_context context, vx_image input, vx_float32 *mean, vx_float32 *stddev)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_scalar s_mean = vxCreateScalar(context, (vx_enum)VX_TYPE_FLOAT32, NULL);
        vx_scalar s_stddev = vxCreateScalar(context, (vx_enum)VX_TYPE_FLOAT32, NULL);
        vx_node node = vxMeanStdDevNode(graph, input, s_mean, s_stddev);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
                vxCopyScalar(s_mean, mean, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST);
                vxCopyScalar(s_stddev, stddev, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST);
            }
            vxReleaseNode(&node);
        }
        vxReleaseScalar(&s_mean);
        vxReleaseScalar(&s_stddev);
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuThreshold(vx_context context, vx_image input, vx_threshold thresh, vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxThresholdNode(graph, input, thresh, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuIntegralImage(vx_context context, vx_image input, vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxIntegralImageNode(graph, input, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuErode3x3(vx_context context, vx_image input, vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxErode3x3Node(graph, input, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, border_modes_3, dimof(border_modes_3));
            if (status == (vx_status)VX_SUCCESS)
            {
                status = setNodeTarget(node);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuDilate3x3(vx_context context, vx_image input, vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxDilate3x3Node(graph, input, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, border_modes_3, dimof(border_modes_3));
            if (status == (vx_status)VX_SUCCESS)
            {
                status = setNodeTarget(node);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuMedian3x3(vx_context context, vx_image input, vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxMedian3x3Node(graph, input, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, border_modes_3, dimof(border_modes_3));
            if (status == (vx_status)VX_SUCCESS)
            {
                status = setNodeTarget(node);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuBox3x3(vx_context context, vx_image input, vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxBox3x3Node(graph, input, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, border_modes_3, dimof(border_modes_3));
            if (status == (vx_status)VX_SUCCESS)
            {
                status = setNodeTarget(node);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuGaussian3x3(vx_context context, vx_image input, vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxGaussian3x3Node(graph, input, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, border_modes_3, dimof(border_modes_3));
            if (status == (vx_status)VX_SUCCESS)
            {
                status = setNodeTarget(node);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuNonLinearFilter(vx_context context, vx_enum function, vx_image input, vx_matrix mask, vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxNonLinearFilterNode(graph, function, input, mask, output);
        if (vxGetStatus((vx_reference)node) == (vx_status)VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, border_modes_3, dimof(border_modes_3));
            if (status == (vx_status)VX_SUCCESS)
            {
                status = setNodeTarget(node);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuConvolve(vx_context context, vx_image input, vx_convolution conv, vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxConvolveNode(graph, input, conv, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, border_modes_3, dimof(border_modes_3));
            if (status == (vx_status)VX_SUCCESS)
            {
                status = setNodeTarget(node);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuGaussianPyramid(vx_context context, vx_image input, vx_pyramid gaussian)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxGaussianPyramidNode(graph, input, gaussian);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, border_modes_2, dimof(border_modes_2));
            if (status == (vx_status)VX_SUCCESS)
            {
                status = setNodeTarget(node);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuLaplacianPyramid(vx_context context, vx_image input, vx_pyramid laplacian, vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxLaplacianPyramidNode(graph, input, laplacian, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, border_modes_2, dimof(border_modes_2));
            if (status == (vx_status)VX_SUCCESS)
            {
                status = setNodeTarget(node);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuLaplacianReconstruct(vx_context context, vx_pyramid laplacian, vx_image input, vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxLaplacianReconstructNode(graph, laplacian, input, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, border_modes_2, dimof(border_modes_2));
            if (status == (vx_status)VX_SUCCESS)
            {
                status = setNodeTarget(node);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuAccumulateImage(vx_context context, vx_image input, vx_image accum)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxAccumulateImageNode(graph, input, accum);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuAccumulateWeightedImage(vx_context context, vx_image input, vx_scalar scale, vx_image accum)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxAccumulateWeightedImageNode(graph, input, scale, accum);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuAccumulateSquareImage(vx_context context, vx_image input, vx_scalar scale, vx_image accum)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxAccumulateSquareImageNode(graph, input, scale, accum);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuMinMaxLoc(vx_context context, vx_image input,
                        vx_scalar minVal, vx_scalar maxVal,
                        vx_array minLoc, vx_array maxLoc,
                        vx_scalar minCount, vx_scalar maxCount)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxMinMaxLocNode(graph, input, minVal, maxVal, minLoc, maxLoc, minCount, maxCount);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuConvertDepth(vx_context context, vx_image input, vx_image output, vx_enum policy, vx_int32 shift_val)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);

    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_scalar shift = vxCreateScalar(context, (vx_enum)VX_TYPE_INT32, &shift_val);
        vx_node node = vxConvertDepthNode(graph, input, output, policy, shift);
        if ( ((vx_status)VX_SUCCESS == vxGetStatus((vx_reference)node)) &&
             ((vx_status)VX_SUCCESS == vxGetStatus((vx_reference)shift)) )
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
        vxReleaseScalar(&shift);
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuCannyEdgeDetector(vx_context context, vx_image input, vx_threshold hyst,
                               vx_int32 gradient_size, vx_enum norm_type,
                               vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxCannyEdgeDetectorNode(graph, input, hyst, gradient_size, norm_type, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuHalfScaleGaussian(vx_context context, vx_image input, vx_image output, vx_int32 kernel_size)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxHalfScaleGaussianNode(graph, input, output, kernel_size);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, border_modes_2, dimof(border_modes_2));
            if (status == (vx_status)VX_SUCCESS)
            {
                status = setNodeTarget(node);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuAnd(vx_context context, vx_image in1, vx_image in2, vx_image out)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxAndNode(graph, in1, in2, out);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuOr(vx_context context, vx_image in1, vx_image in2, vx_image out)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxOrNode(graph, in1, in2, out);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuXor(vx_context context, vx_image in1, vx_image in2, vx_image out)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxXorNode(graph, in1, in2, out);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuNot(vx_context context, vx_image input, vx_image out)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxNotNode(graph, input, out);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuMultiply(vx_context context, vx_image in1, vx_image in2, vx_float32 scale_val, vx_enum overflow_policy, vx_enum rounding_policy, vx_image out)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);

    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_scalar scale = vxCreateScalar(context, (vx_enum)VX_TYPE_FLOAT32, &scale_val);
        vx_node node = vxMultiplyNode(graph, in1, in2, scale, overflow_policy, rounding_policy, out);
        if ( ((vx_status)VX_SUCCESS == vxGetStatus((vx_reference)node)) &&
             ((vx_status)VX_SUCCESS == vxGetStatus((vx_reference)scale)) )
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
        vxReleaseScalar(&scale);
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuAdd(vx_context context, vx_image in1, vx_image in2, vx_enum policy, vx_image out)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxAddNode(graph, in1, in2, policy, out);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuSubtract(vx_context context, vx_image in1, vx_image in2, vx_enum policy, vx_image out)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxSubtractNode(graph, in1, in2, policy, out);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuWarpAffine(vx_context context, vx_image input, vx_matrix matrix, vx_enum type, vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxWarpAffineNode(graph, input, matrix, type, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, border_modes_2, dimof(border_modes_2));
            if (status == (vx_status)VX_SUCCESS)
            {
                status = setNodeTarget(node);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuWarpPerspective(vx_context context, vx_image input, vx_matrix matrix, vx_enum type, vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxWarpPerspectiveNode(graph, input, matrix, type, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, border_modes_2, dimof(border_modes_2));
            if (status == (vx_status)VX_SUCCESS)
            {
                status = setNodeTarget(node);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuHarrisCorners(vx_context context, vx_image input,
        vx_scalar strength_thresh,
        vx_scalar min_distance,
        vx_scalar sensitivity,
        vx_int32 gradient_size,
        vx_int32 block_size,
        vx_array corners,
        vx_scalar num_corners)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxHarrisCornersNode(graph, input, strength_thresh, min_distance, sensitivity, gradient_size, block_size, corners, num_corners);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuFastCorners(vx_context context, vx_image input, vx_scalar strength_thresh, vx_bool nonmax_suppression, vx_array corners, vx_scalar num_corners)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxFastCornersNode(graph, input, strength_thresh, nonmax_suppression, corners, num_corners);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuOpticalFlowPyrLK(vx_context context, vx_pyramid old_images,
                              vx_pyramid new_images,
                              vx_array old_points,
                              vx_array new_points_estimates,
                              vx_array new_points,
                              vx_enum termination,
                              vx_scalar epsilon,
                              vx_scalar num_iterations,
                              vx_scalar use_initial_estimate,
                              vx_size window_dimension)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxOpticalFlowPyrLKNode(graph, old_images, new_images, old_points,new_points_estimates, new_points,
                termination,epsilon,num_iterations,use_initial_estimate,window_dimension);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = setNodeTarget(node);
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuRemap(vx_context context, vx_image input, vx_remap table, vx_enum policy, vx_image output)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS)
    {
        vx_node node = vxRemapNode(graph, input, table, policy, output);
        if (vxGetStatus((vx_reference)node)==(vx_status)VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, border_modes_2, dimof(border_modes_2));
            if (status == (vx_status)VX_SUCCESS)
            {
                status = setNodeTarget(node);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxVerifyGraph(graph);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}
