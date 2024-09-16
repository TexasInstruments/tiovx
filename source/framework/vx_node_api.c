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
 * \brief The Graph Mode Interface for all Base Kernels.
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <vx_internal.h>

static vx_node vxCreateNodeByStructure(vx_graph graph,
                                vx_kernel kernel,
                                vx_enum kernelenum,
                                vx_reference params[],
                                vx_uint32 num);

/* Note: with the current sample implementation structure, we have no other choice than
returning 0 in case of errors not due to vxCreateGenericNode, because vxGetErrorObject
is internal to another library and is not exported. This is not an issue since vxGetStatus
correctly manages a ref == 0 */
static vx_node vxCreateNodeByStructure(vx_graph graph,
                                vx_kernel kernel,
                                vx_enum kernelenum,
                                vx_reference params[],
                                vx_uint32 num)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_node node = NULL;
    vx_uint32 release_kernel = 0;
    vx_context context = vxGetContext(vxCastRefFromGraph(graph));

    if(kernel==NULL)
    {
        kernel = vxGetKernelByEnum(context, kernelenum);
        release_kernel = 1;
    }
    if (kernel != NULL)
    {
        node = vxCreateGenericNode(graph, kernel);
        if (vxGetStatus(vxCastRefFromNode(node)) == (vx_status)VX_SUCCESS)
        {
            vx_uint32 p = 0;
            for (p = 0; p < num; p++)
            {
                status = vxSetParameterByIndex(node, p, params[p]);
                if (status != (vx_status)VX_SUCCESS)
                {
                    vxAddLogEntry(vxCastRefFromGraph(graph), status, "Kernel %d Parameter %u is invalid.\n", kernelenum, p);
                    (void)vxReleaseNode(&node);
                    node = NULL;
                    break;
                }
            }
        }
        else
        {
            vxAddLogEntry(vxCastRefFromGraph(graph), (vx_status)VX_ERROR_INVALID_PARAMETERS, "Failed to create node with kernel enum %d\n", kernelenum);
            VX_PRINT(VX_ZONE_ERROR, "Failed to create node with kernel enum %d\n", kernelenum);
        }
        if (release_kernel != 0U)
        {
            (void)vxReleaseKernel(&kernel);
        }
    }
    else
    {
        vxAddLogEntry(vxCastRefFromGraph(graph), (vx_status)VX_ERROR_INVALID_PARAMETERS, "failed to retrieve kernel enum %d\n", kernelenum);
        VX_PRINT(VX_ZONE_ERROR, "failed to retrieve kernel enum %d\n", kernelenum);
    }
    return node;
}

vx_node tivxCreateNodeByKernelEnum(vx_graph graph,
                                vx_enum kernelenum,
                                vx_reference params[],
                                vx_uint32 num)
{
    return vxCreateNodeByStructure(graph, NULL, kernelenum, params, num);
}

vx_node tivxCreateNodeByKernelRef(vx_graph graph,
                                vx_kernel kernel,
                                vx_reference params[],
                                vx_uint32 num)
{
    return vxCreateNodeByStructure(graph, kernel, 0, params, num);
}

vx_node tivxCreateNodeByKernelName(vx_graph graph,
                                const char *kernel_name,
                                vx_reference params[],
                                vx_uint32 num)
{
    vx_node node = NULL;
    vx_kernel kernel;
    vx_context context = vxGetContext(vxCastRefFromGraph(graph));
    kernel = vxGetKernelByName(context, kernel_name);
    if(kernel!=NULL)
    {
        /* kernel is released inside vxCreateNodeByStructure */
        node =  vxCreateNodeByStructure(graph, kernel, 0, params, num);
        (void)vxReleaseKernel(&kernel);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Call to vxGetKernelByName failed; kernel may not be registered\n");
    }
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxColorConvertNode(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromImage(output),
    };
    return tivxCreateNodeByKernelEnum(graph, (vx_enum)VX_KERNEL_COLOR_CONVERT, params, dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxChannelExtractNode(vx_graph graph,
                             vx_image input,
                             vx_enum channelNum,
                             vx_image output)
{
    vx_context context = vxGetContext(vxCastRefFromGraph(graph));
    vx_scalar scalar = vxCreateScalar(context, (vx_enum)VX_TYPE_ENUM, &channelNum);
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromScalar(scalar),
        vxCastRefFromImage(output),
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           (vx_enum)VX_KERNEL_CHANNEL_EXTRACT,
                                           params,
                                           dimof(params));
    (void)vxReleaseScalar(&scalar); /* node hold reference */

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxChannelCombineNode(vx_graph graph,
                             vx_image plane0,
                             vx_image plane1,
                             vx_image plane2,
                             vx_image plane3,
                             vx_image output)
{
    vx_reference params[] = {
       vxCastRefFromImage(plane0),
       vxCastRefFromImage(plane1),
       vxCastRefFromImage(plane2),
       vxCastRefFromImage(plane3),
       vxCastRefFromImage(output),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_CHANNEL_COMBINE,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxSobel3x3Node(vx_graph graph, vx_image input, vx_image output_x, vx_image output_y)
{
    vx_reference params[] = {
       vxCastRefFromImage(input),
       vxCastRefFromImage(output_x),
       vxCastRefFromImage(output_y),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_SOBEL_3x3,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxMagnitudeNode(vx_graph graph, vx_image grad_x, vx_image grad_y, vx_image mag)
{
    vx_reference params[] = {
       vxCastRefFromImage(grad_x),
       vxCastRefFromImage(grad_y),
       vxCastRefFromImage(mag),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_MAGNITUDE,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxPhaseNode(vx_graph graph, vx_image grad_x, vx_image grad_y, vx_image orientation)
{
    vx_reference params[] = {
       vxCastRefFromImage(grad_x),
       vxCastRefFromImage(grad_y),
       vxCastRefFromImage(orientation),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_PHASE,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxScaleImageNode(vx_graph graph, vx_image src, vx_image dst, vx_enum type)
{
    vx_context context = vxGetContext(vxCastRefFromGraph(graph));
    vx_scalar stype = vxCreateScalar(context, (vx_enum)VX_TYPE_ENUM, &type);
    vx_reference params[] = {
        vxCastRefFromImage(src),
        vxCastRefFromImage(dst),
        vxCastRefFromScalar(stype),
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           (vx_enum)VX_KERNEL_SCALE_IMAGE,
                                           params,
                                           dimof(params));
    (void)vxReleaseScalar(&stype);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTableLookupNode(vx_graph graph, vx_image input, vx_lut lut, vx_image output)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromLUT(lut),
        vxCastRefFromImage(output),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_TABLE_LOOKUP,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxHistogramNode(vx_graph graph, vx_image input, vx_distribution distribution)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromDistribution(distribution),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_HISTOGRAM,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxEqualizeHistNode(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromImage(output),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_EQUALIZE_HISTOGRAM,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxAbsDiffNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    vx_reference params[] = {
       vxCastRefFromImage(in1),
       vxCastRefFromImage(in2),
       vxCastRefFromImage(out),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_ABSDIFF,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxMeanStdDevNode(vx_graph graph, vx_image input, vx_scalar mean, vx_scalar stddev)
{
    vx_reference params[] = {
       vxCastRefFromImage(input),
       vxCastRefFromScalar(mean),
       vxCastRefFromScalar(stddev),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_MEAN_STDDEV,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxThresholdNode(vx_graph graph, vx_image input, vx_threshold thesh, vx_image output)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromThreshold(thesh),
        vxCastRefFromImage(output),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_THRESHOLD,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxIntegralImageNode(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromImage(output),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_INTEGRAL_IMAGE,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxErode3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromImage(output),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_ERODE_3x3,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxDilate3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromImage(output),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_DILATE_3x3,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxMedian3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromImage(output),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_MEDIAN_3x3,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxBox3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromImage(output),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_BOX_3x3,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxGaussian3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromImage(output),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_GAUSSIAN_3x3,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxNonLinearFilterNode(vx_graph graph, vx_enum function, vx_image input, vx_matrix mask, vx_image output)
{
    vx_scalar func = vxCreateScalar(vxGetContext(vxCastRefFromGraph(graph)), (vx_enum)VX_TYPE_ENUM, &function);
    vx_reference params[] = {
        vxCastRefFromScalar(func),
        vxCastRefFromImage(input),
        vxCastRefFromMatrix(mask),
        vxCastRefFromImage(output),
    };

    vx_node node = tivxCreateNodeByKernelEnum(graph,
        (vx_enum)VX_KERNEL_NON_LINEAR_FILTER,
        params,
        dimof(params));

    (void)vxReleaseScalar(&func);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxConvolveNode(vx_graph graph, vx_image input, vx_convolution conv, vx_image output)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromConvolution(conv),
        vxCastRefFromImage(output),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_CUSTOM_CONVOLUTION,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxGaussianPyramidNode(vx_graph graph, vx_image input, vx_pyramid gaussian)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromPyramid(gaussian),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_GAUSSIAN_PYRAMID,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxLaplacianPyramidNode(vx_graph graph, vx_image input, vx_pyramid laplacian, vx_image output)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromPyramid(laplacian),
        vxCastRefFromImage(output),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_LAPLACIAN_PYRAMID,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxLaplacianReconstructNode(vx_graph graph, vx_pyramid laplacian, vx_image input,
                                       vx_image output)
{
    vx_reference params[] = {
        vxCastRefFromPyramid(laplacian),
        vxCastRefFromImage(input),
        vxCastRefFromImage(output),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_LAPLACIAN_RECONSTRUCT,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxAccumulateImageNode(vx_graph graph, vx_image input, vx_image accum)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromImage(accum),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_ACCUMULATE,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxAccumulateWeightedImageNode(vx_graph graph, vx_image input, vx_scalar alpha, vx_image accum)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromScalar(alpha),
        vxCastRefFromImage(accum),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_ACCUMULATE_WEIGHTED,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxAccumulateWeightedImageNodeX(vx_graph graph, vx_image input, vx_float32 alpha, vx_image accum)
{
    vx_scalar salpha = vxCreateScalar(vxGetContext(vxCastRefFromGraph(graph)), (vx_enum)VX_TYPE_FLOAT32, &alpha);
    vx_node node = vxAccumulateWeightedImageNode(graph, input, salpha, accum);

    (void)vxReleaseScalar(&salpha);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxAccumulateSquareImageNode(vx_graph graph, vx_image input, vx_scalar scalar, vx_image accum)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromScalar(scalar),
        vxCastRefFromImage(accum),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_ACCUMULATE_SQUARE,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxAccumulateSquareImageNodeX(vx_graph graph, vx_image input, vx_uint32 shift, vx_image accum)
{
    vx_scalar scalar = vxCreateScalar(vxGetContext(vxCastRefFromGraph(graph)), (vx_enum)VX_TYPE_UINT32, &shift);
    vx_node node = vxAccumulateSquareImageNode(graph, input, scalar, accum);

    (void)vxReleaseScalar(&scalar);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxMinMaxLocNode(vx_graph graph,
                        vx_image input,
                        vx_scalar minVal, vx_scalar maxVal,
                        vx_array minLoc, vx_array maxLoc,
                        vx_scalar minCount, vx_scalar maxCount)
{
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromScalar(minVal),
        vxCastRefFromScalar(maxVal),
        vxCastRefFromArray(minLoc),
        vxCastRefFromArray(maxLoc),
        vxCastRefFromScalar(minCount),
        vxCastRefFromScalar(maxCount),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_MINMAXLOC,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxConvertDepthNode(vx_graph graph, vx_image input, vx_image output, vx_enum policy, vx_scalar shift)
{
    vx_scalar pol = vxCreateScalar(vxGetContext(vxCastRefFromGraph(graph)), (vx_enum)VX_TYPE_ENUM, &policy);
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromImage(output),
        vxCastRefFromScalar(pol),
        vxCastRefFromScalar(shift),
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_CONVERTDEPTH,
                                   params,
                                   dimof(params));
    (void)vxReleaseScalar(&pol);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxCannyEdgeDetectorNode(vx_graph graph, vx_image input, vx_threshold hyst,
                                vx_int32 gradient_size, vx_enum norm_type,
                                vx_image output)
{
    vx_scalar gs = vxCreateScalar(vxGetContext(vxCastRefFromGraph(graph)), (vx_enum)VX_TYPE_INT32, &gradient_size);
    vx_scalar nt = vxCreateScalar(vxGetContext(vxCastRefFromGraph(graph)), (vx_enum)VX_TYPE_ENUM, &norm_type);
    vx_reference params[] = {
        vxCastRefFromImage(input),
        vxCastRefFromThreshold(hyst),
        vxCastRefFromScalar(gs),
        vxCastRefFromScalar(nt),
        vxCastRefFromImage(output),
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           (vx_enum)VX_KERNEL_CANNY_EDGE_DETECTOR,
                                           params,
                                           dimof(params));
    (void)vxReleaseScalar(&gs);

    (void)vxReleaseScalar(&nt);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxAndNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    vx_reference params[] = {
       vxCastRefFromImage(in1),
       vxCastRefFromImage(in2),
       vxCastRefFromImage(out),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_AND,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxOrNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    vx_reference params[] = {
       vxCastRefFromImage(in1),
       vxCastRefFromImage(in2),
       vxCastRefFromImage(out),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_OR,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxXorNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    vx_reference params[] = {
       vxCastRefFromImage(in1),
       vxCastRefFromImage(in2),
       vxCastRefFromImage(out),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_XOR,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxNotNode(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference params[] = {
       vxCastRefFromImage(input),
       vxCastRefFromImage(output),
    };
    return tivxCreateNodeByKernelEnum(graph,
                                   (vx_enum)VX_KERNEL_NOT,
                                   params,
                                   dimof(params));
}

VX_API_ENTRY vx_node VX_API_CALL vxMultiplyNode(vx_graph graph, vx_image in1, vx_image in2, vx_scalar scale, vx_enum overflow_policy, vx_enum rounding_policy, vx_image out)
{
    vx_context context = vxGetContext(vxCastRefFromGraph(graph));
    vx_scalar spolicy = vxCreateScalar(context, (vx_enum)VX_TYPE_ENUM, &overflow_policy);
    vx_scalar rpolicy = vxCreateScalar(context, (vx_enum)VX_TYPE_ENUM, &rounding_policy);
    vx_reference params[] = {
       vxCastRefFromImage(in1),
       vxCastRefFromImage(in2),
       vxCastRefFromScalar(scale),
       vxCastRefFromScalar(spolicy),
       vxCastRefFromScalar(rpolicy),
       vxCastRefFromImage(out),
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           (vx_enum)VX_KERNEL_MULTIPLY,
                                           params,
                                           dimof(params));
    (void)vxReleaseScalar(&spolicy);

    (void)vxReleaseScalar(&rpolicy);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxAddNode(vx_graph graph, vx_image in1, vx_image in2, vx_enum policy, vx_image out)
{
    vx_context context = vxGetContext(vxCastRefFromGraph(graph));
    vx_scalar spolicy = vxCreateScalar(context, (vx_enum)VX_TYPE_ENUM, &policy);
    vx_reference params[] = {
       vxCastRefFromImage(in1),
       vxCastRefFromImage(in2),
       vxCastRefFromScalar(spolicy),
       vxCastRefFromImage(out)
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           (vx_enum)VX_KERNEL_ADD,
                                           params,
                                           dimof(params));
    (void)vxReleaseScalar(&spolicy);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxSubtractNode(vx_graph graph, vx_image in1, vx_image in2, vx_enum policy, vx_image out)
{
    vx_context context = vxGetContext(vxCastRefFromGraph(graph));
    vx_scalar spolicy = vxCreateScalar(context, (vx_enum)VX_TYPE_ENUM, &policy);
    vx_reference params[] = {
       vxCastRefFromImage(in1),
       vxCastRefFromImage(in2),
       vxCastRefFromScalar(spolicy),
       vxCastRefFromImage(out)
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           (vx_enum)VX_KERNEL_SUBTRACT,
                                           params,
                                           dimof(params));
    (void)vxReleaseScalar(&spolicy);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxWarpAffineNode(vx_graph graph, vx_image input, vx_matrix matrix, vx_enum type, vx_image output)
{
    vx_context context = vxGetContext(vxCastRefFromGraph(graph));
    vx_scalar stype = vxCreateScalar(context, (vx_enum)VX_TYPE_ENUM, &type);
    vx_reference params[] = {
            vxCastRefFromImage(input),
            vxCastRefFromMatrix(matrix),
            vxCastRefFromScalar(stype),
            vxCastRefFromImage(output),
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           (vx_enum)VX_KERNEL_WARP_AFFINE,
                                           params,
                                           dimof(params));
    (void)vxReleaseScalar(&stype);

    if (vxGetStatus(vxCastRefFromNode(node)) == (vx_status)VX_SUCCESS)
    {
        /* default value for Warp node */
        /* change node attribute as kernel attributes alreay copied to node */
        /* in tivxCreateNodeByKernelEnum() */
        (void)ownSetNodeAttributeValidRectReset(node, (vx_bool)vx_true_e);
    }

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxWarpPerspectiveNode(vx_graph graph, vx_image input, vx_matrix matrix, vx_enum type, vx_image output)
{
    vx_context context = vxGetContext(vxCastRefFromGraph(graph));
    vx_scalar stype = vxCreateScalar(context, (vx_enum)VX_TYPE_ENUM, &type);
    vx_reference params[] = {
            vxCastRefFromImage(input),
            vxCastRefFromMatrix(matrix),
            vxCastRefFromScalar(stype),
            vxCastRefFromImage(output),
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           (vx_enum)VX_KERNEL_WARP_PERSPECTIVE,
                                           params,
                                           dimof(params));
    (void)vxReleaseScalar(&stype);

    if (vxGetStatus(vxCastRefFromNode(node)) == (vx_status)VX_SUCCESS)
    {
        /* default value for Warp node */
        /* change node attribute as kernel attributes alreay copied to node */
        /* in tivxCreateNodeByKernelEnum() */
        (void)ownSetNodeAttributeValidRectReset(node, (vx_bool)vx_true_e);
    }

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxHarrisCornersNode(vx_graph graph,
                            vx_image input,
                            vx_scalar strength_thresh,
                            vx_scalar min_distance,
                            vx_scalar sensitivity,
                            vx_int32 gradient_size,
                            vx_int32 block_size,
                            vx_array corners,
                            vx_scalar num_corners)
{
    vx_scalar win = vxCreateScalar(vxGetContext(vxCastRefFromGraph(graph)), (vx_enum)VX_TYPE_INT32, &gradient_size);
    vx_scalar blk = vxCreateScalar(vxGetContext(vxCastRefFromGraph(graph)), (vx_enum)VX_TYPE_INT32, &block_size);
    vx_reference params[] = {
            vxCastRefFromImage(input),
            vxCastRefFromScalar(strength_thresh),
            vxCastRefFromScalar(min_distance),
            vxCastRefFromScalar(sensitivity),
            vxCastRefFromScalar(win),
            vxCastRefFromScalar(blk),
            vxCastRefFromArray(corners),
            vxCastRefFromScalar(num_corners),
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           (vx_enum)VX_KERNEL_HARRIS_CORNERS,
                                           params,
                                           dimof(params));
    (void)vxReleaseScalar(&win);

    (void)vxReleaseScalar(&blk);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxFastCornersNode(vx_graph graph, vx_image input, vx_scalar strength_thresh, vx_bool nonmax_suppression, vx_array corners, vx_scalar num_corners)
{
    vx_scalar nonmax = vxCreateScalar(vxGetContext(vxCastRefFromGraph(graph)),(vx_enum)VX_TYPE_BOOL, &nonmax_suppression);
    vx_reference params[] = {
            vxCastRefFromImage(input),
            vxCastRefFromScalar(strength_thresh),
            vxCastRefFromScalar(nonmax),
            vxCastRefFromArray(corners),
            vxCastRefFromScalar(num_corners)
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           (vx_enum)VX_KERNEL_FAST_CORNERS,
                                           params,
                                           dimof(params));
    (void)vxReleaseScalar(&nonmax);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxOpticalFlowPyrLKNode(vx_graph graph,
                               vx_pyramid old_images,
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
    vx_scalar term = vxCreateScalar(vxGetContext(vxCastRefFromGraph(graph)), (vx_enum)VX_TYPE_ENUM, &termination);
    vx_scalar winsize = vxCreateScalar(vxGetContext(vxCastRefFromGraph(graph)), (vx_enum)VX_TYPE_SIZE, &window_dimension);
    vx_reference params[] = {
            vxCastRefFromPyramid(old_images),
            vxCastRefFromPyramid(new_images),
            vxCastRefFromArray(old_points),
            vxCastRefFromArray(new_points_estimates),
            vxCastRefFromArray(new_points),
            vxCastRefFromScalar(term),
            vxCastRefFromScalar(epsilon),
            vxCastRefFromScalar(num_iterations),
            vxCastRefFromScalar(use_initial_estimate),
            vxCastRefFromScalar(winsize),
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           (vx_enum)VX_KERNEL_OPTICAL_FLOW_PYR_LK,
                                           params,
                                           dimof(params));
    (void)vxReleaseScalar(&term);

    (void)vxReleaseScalar(&winsize);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxRemapNode(vx_graph graph,
                    vx_image input,
                    vx_remap table,
                    vx_enum policy,
                    vx_image output)
{
    vx_scalar spolicy = vxCreateScalar(vxGetContext(vxCastRefFromGraph(graph)), (vx_enum)VX_TYPE_ENUM, &policy);
    vx_reference params[] = {
            vxCastRefFromImage(input),
            vxCastRefFromRemap(table),
            vxCastRefFromScalar(spolicy),
            vxCastRefFromImage(output),
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           (vx_enum)VX_KERNEL_REMAP,
                                           params,
                                           dimof(params));
    (void)vxReleaseScalar(&spolicy);

    if (vxGetStatus(vxCastRefFromNode(node)) == (vx_status)VX_SUCCESS)
    {
        /* default value for Remap node */
        /* change node attribute as kernel attributes alreay copied to node */
        /* in tivxCreateNodeByKernelEnum() */
        (void)ownSetNodeAttributeValidRectReset(node, (vx_bool)vx_true_e);
    }

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxHalfScaleGaussianNode(vx_graph graph, vx_image input, vx_image output, vx_int32 kernel_size)
{
    vx_scalar ksize = vxCreateScalar(vxGetContext(vxCastRefFromGraph(graph)), (vx_enum)VX_TYPE_INT32, &kernel_size);
    vx_reference params[] = {
            vxCastRefFromImage(input),
            vxCastRefFromImage(output),
            vxCastRefFromScalar(ksize),
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           (vx_enum)VX_KERNEL_HALFSCALE_GAUSSIAN,
                                           params,
                                           dimof(params));
    (void)vxReleaseScalar(&ksize);

    return node;
}
