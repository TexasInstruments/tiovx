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
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <vx_internal.h>
#include <tivx_objects.h>

static tivx_object_t g_tivx_objects;
static tivx_mutex g_tivx_objects_lock;


static vx_status ownCheckUseFlag(vx_bool inUse[], uint32_t num_ele);
static void ownInitUseFlag(vx_bool inUse[], uint32_t num_ele);
static vx_status ownFreeObject(
    uint8_t *obj_ptr, uint8_t *obj_start_ptr, vx_bool inUse[],
    uint32_t max_objects, uint32_t size);
static uint8_t *ownAllocObject(
    uint8_t *obj_start_ptr, vx_bool inUse[], uint32_t max_objects,
    uint32_t size);


vx_status tivxObjectInit(void)
{
    vx_status status;

    status = tivxMutexCreate(&g_tivx_objects_lock);

    if (VX_SUCCESS == status)
    {
        ownInitUseFlag(g_tivx_objects.isMfUse,
            TIVX_META_FORMAT_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isContextUse,
            TIVX_CONTEXT_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isGraphUse,
            TIVX_GRAPH_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isNodeUse,
            TIVX_NODE_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isKernelUse,
            TIVX_KERNEL_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isArrayUse,
            TIVX_ARRAY_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isConvolutionUse,
            TIVX_CONVOLUTION_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isDelayUse,
            TIVX_DELAY_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isDistributionUse,
            TIVX_DISTRIBUTION_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isImageUse,
            TIVX_IMAGE_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isLutUse,
            TIVX_LUT_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isMatrixUse,
            TIVX_MATRIX_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isPyramidUse,
            TIVX_PYRAMID_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isRemapUse,
            TIVX_REMAP_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isScalarUse,
            TIVX_SCALAR_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isThresholdUse,
            TIVX_THRESHOLD_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isErrorUse,
            TIVX_ERROR_MAX_OBJECTS);
    }

    return (status);
}

vx_status tivxObjectDeInit(void)
{
    vx_status status;

    status = tivxMutexDelete(&g_tivx_objects_lock);

    if (VX_SUCCESS == status)
    {
        status = ownCheckUseFlag(g_tivx_objects.isMfUse,
            TIVX_META_FORMAT_MAX_OBJECTS);
        status = ownCheckUseFlag(g_tivx_objects.isContextUse,
            TIVX_CONTEXT_MAX_OBJECTS);
        status = ownCheckUseFlag(g_tivx_objects.isGraphUse,
            TIVX_GRAPH_MAX_OBJECTS);
        status = ownCheckUseFlag(g_tivx_objects.isNodeUse,
            TIVX_NODE_MAX_OBJECTS);
        status = ownCheckUseFlag(g_tivx_objects.isKernelUse,
            TIVX_KERNEL_MAX_OBJECTS);
        status = ownCheckUseFlag(g_tivx_objects.isArrayUse,
            TIVX_ARRAY_MAX_OBJECTS);
        status = ownCheckUseFlag(g_tivx_objects.isConvolutionUse,
            TIVX_CONVOLUTION_MAX_OBJECTS);
        status = ownCheckUseFlag(g_tivx_objects.isDelayUse,
            TIVX_DELAY_MAX_OBJECTS);
        status = ownCheckUseFlag(g_tivx_objects.isDistributionUse,
            TIVX_DISTRIBUTION_MAX_OBJECTS);
        status = ownCheckUseFlag(g_tivx_objects.isImageUse,
            TIVX_IMAGE_MAX_OBJECTS);
        status = ownCheckUseFlag(g_tivx_objects.isLutUse,
            TIVX_LUT_MAX_OBJECTS);
        status = ownCheckUseFlag(g_tivx_objects.isMatrixUse,
            TIVX_MATRIX_MAX_OBJECTS);
        status = ownCheckUseFlag(g_tivx_objects.isPyramidUse,
            TIVX_PYRAMID_MAX_OBJECTS);
        status = ownCheckUseFlag(g_tivx_objects.isRemapUse,
            TIVX_REMAP_MAX_OBJECTS);
        status = ownCheckUseFlag(g_tivx_objects.isScalarUse,
            TIVX_SCALAR_MAX_OBJECTS);
        status = ownCheckUseFlag(g_tivx_objects.isThresholdUse,
            TIVX_THRESHOLD_MAX_OBJECTS);
        status = ownCheckUseFlag(g_tivx_objects.isErrorUse,
            TIVX_ERROR_MAX_OBJECTS);
    }

    return (status);
}



vx_reference tivxObjectAlloc(vx_enum type)
{
    vx_status status;
    vx_reference ref = NULL;

    status = tivxMutexLock(g_tivx_objects_lock);

    if (VX_SUCCESS == status)
    {
        switch(type)
        {
            case VX_TYPE_META_FORMAT:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.meta_format, g_tivx_objects.isMfUse,
                    TIVX_META_FORMAT_MAX_OBJECTS, sizeof(tivx_meta_format_t));
                break;
            case VX_TYPE_CONTEXT:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.context, g_tivx_objects.isContextUse,
                    TIVX_CONTEXT_MAX_OBJECTS, sizeof(tivx_context_t));
                break;
            case VX_TYPE_GRAPH:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.graph, g_tivx_objects.isGraphUse,
                    TIVX_GRAPH_MAX_OBJECTS, sizeof(tivx_graph_t));
                break;
            case VX_TYPE_NODE:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.node, g_tivx_objects.isNodeUse,
                    TIVX_NODE_MAX_OBJECTS, sizeof(tivx_node_t));
                break;
            case VX_TYPE_KERNEL:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.kernel, g_tivx_objects.isKernelUse,
                    TIVX_KERNEL_MAX_OBJECTS, sizeof(tivx_kernel_t));
                break;
            case VX_TYPE_ARRAY:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.array, g_tivx_objects.isArrayUse,
                    TIVX_ARRAY_MAX_OBJECTS, sizeof(tivx_array_t));
                break;
            case VX_TYPE_CONVOLUTION:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.convolution,
                    g_tivx_objects.isConvolutionUse,
                    TIVX_CONVOLUTION_MAX_OBJECTS, sizeof(tivx_convolution_t));
                break;
            case VX_TYPE_DELAY:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.delay, g_tivx_objects.isDelayUse,
                    TIVX_DELAY_MAX_OBJECTS, sizeof(tivx_delay_t));
                break;
            case VX_TYPE_DISTRIBUTION:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.distribution,
                    g_tivx_objects.isDistributionUse,
                    TIVX_DISTRIBUTION_MAX_OBJECTS, sizeof(tivx_distribution_t));
                break;
            case VX_TYPE_IMAGE:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.image, g_tivx_objects.isImageUse,
                    TIVX_IMAGE_MAX_OBJECTS, sizeof(tivx_image_t));
                break;
            case VX_TYPE_LUT:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.lut, g_tivx_objects.isLutUse,
                    TIVX_LUT_MAX_OBJECTS, sizeof(tivx_lut_t));
                break;
            case VX_TYPE_MATRIX:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.matrix, g_tivx_objects.isMatrixUse,
                    TIVX_MATRIX_MAX_OBJECTS, sizeof(tivx_matrix_t));
                break;
            case VX_TYPE_PYRAMID:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.pyramid, g_tivx_objects.isPyramidUse,
                    TIVX_PYRAMID_MAX_OBJECTS, sizeof(tivx_pyramid_t));
                break;
            case VX_TYPE_REMAP:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.remap, g_tivx_objects.isRemapUse,
                    TIVX_REMAP_MAX_OBJECTS, sizeof(tivx_remap_t));
                break;
            case VX_TYPE_SCALAR:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.scalar, g_tivx_objects.isScalarUse,
                    TIVX_SCALAR_MAX_OBJECTS, sizeof(tivx_scalar_t));
                break;
            case VX_TYPE_THRESHOLD:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.threshold, g_tivx_objects.isThresholdUse,
                    TIVX_THRESHOLD_MAX_OBJECTS, sizeof(tivx_threshold_t));
                break;
            case VX_TYPE_ERROR:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.error, g_tivx_objects.isErrorUse,
                    TIVX_ERROR_MAX_OBJECTS, sizeof(tivx_error_t));
                break;
            default:
                ref = NULL;
                break;
        }

        tivxMutexUnlock(g_tivx_objects_lock);
    }

    return (ref);
}

vx_status tivxObjectFree(vx_reference ref)
{
    vx_status status = VX_FAILURE;

    if (NULL != ref)
    {
        status = tivxMutexLock(g_tivx_objects_lock);

        if (VX_SUCCESS == status)
        {
            switch(ref->type)
            {
                case VX_TYPE_ERROR:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.error,
                        g_tivx_objects.isErrorUse,
                        TIVX_ERROR_MAX_OBJECTS,
                        sizeof(tivx_error_t));
                    break;
                case VX_TYPE_META_FORMAT:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.meta_format,
                        g_tivx_objects.isMfUse,
                        TIVX_META_FORMAT_MAX_OBJECTS,
                        sizeof(tivx_meta_format_t));
                    break;
                case VX_TYPE_CONTEXT:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.context,
                        g_tivx_objects.isContextUse,
                        TIVX_CONTEXT_MAX_OBJECTS, sizeof(tivx_context_t));
                    break;
                case VX_TYPE_GRAPH:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.graph, g_tivx_objects.isGraphUse,
                        TIVX_GRAPH_MAX_OBJECTS, sizeof(tivx_graph_t));
                    break;
                case VX_TYPE_NODE:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.node, g_tivx_objects.isNodeUse,
                        TIVX_NODE_MAX_OBJECTS, sizeof(tivx_node_t));
                    break;
                case VX_TYPE_KERNEL:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.kernel,
                        g_tivx_objects.isKernelUse,
                        TIVX_KERNEL_MAX_OBJECTS, sizeof(tivx_kernel_t));
                    break;
                case VX_TYPE_ARRAY:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.array, g_tivx_objects.isArrayUse,
                        TIVX_ARRAY_MAX_OBJECTS, sizeof(tivx_array_t));
                    break;
                case VX_TYPE_CONVOLUTION:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.convolution,
                        g_tivx_objects.isConvolutionUse,
                        TIVX_CONVOLUTION_MAX_OBJECTS,
                        sizeof(tivx_convolution_t));
                    break;
                case VX_TYPE_DELAY:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.delay, g_tivx_objects.isDelayUse,
                        TIVX_DELAY_MAX_OBJECTS, sizeof(tivx_delay_t));
                    break;
                case VX_TYPE_DISTRIBUTION:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.distribution,
                        g_tivx_objects.isDistributionUse,
                        TIVX_DISTRIBUTION_MAX_OBJECTS,
                        sizeof(tivx_distribution_t));
                    break;
                case VX_TYPE_IMAGE:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.image, g_tivx_objects.isImageUse,
                        TIVX_IMAGE_MAX_OBJECTS, sizeof(tivx_image_t));
                    break;
                case VX_TYPE_LUT:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.lut, g_tivx_objects.isLutUse,
                        TIVX_LUT_MAX_OBJECTS, sizeof(tivx_lut_t));
                    break;
                case VX_TYPE_MATRIX:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.matrix,
                        g_tivx_objects.isMatrixUse,
                        TIVX_MATRIX_MAX_OBJECTS, sizeof(tivx_matrix_t));
                    break;
                case VX_TYPE_PYRAMID:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.pyramid,
                        g_tivx_objects.isPyramidUse,
                        TIVX_PYRAMID_MAX_OBJECTS, sizeof(tivx_pyramid_t));
                    break;
                case VX_TYPE_REMAP:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.remap, g_tivx_objects.isRemapUse,
                        TIVX_REMAP_MAX_OBJECTS, sizeof(tivx_remap_t));
                    break;
                case VX_TYPE_SCALAR:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.scalar,
                        g_tivx_objects.isScalarUse,
                        TIVX_SCALAR_MAX_OBJECTS, sizeof(tivx_scalar_t));
                    break;
                case VX_TYPE_THRESHOLD:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.threshold,
                        g_tivx_objects.isThresholdUse,
                        TIVX_THRESHOLD_MAX_OBJECTS, sizeof(tivx_threshold_t));
                    break;
                default:
                    status = VX_ERROR_INVALID_REFERENCE;
                    break;
            }

            tivxMutexUnlock(g_tivx_objects_lock);
        }
    }

    return (status);
}

static uint8_t *ownAllocObject(
    uint8_t *obj_start_ptr, vx_bool inUse[], uint32_t max_objects,
    uint32_t size)
{
    uint32_t i;
    uint8_t *obj_ptr = NULL;

    for (i = 0;i < max_objects; i ++)
    {
        if (vx_false_e == inUse[i])
        {
            obj_ptr = obj_start_ptr;
            inUse[i] = vx_true_e;

            break;
        }
        obj_start_ptr += size;
    }

    return (obj_ptr);
}

static vx_status ownFreeObject(
    uint8_t *obj_ptr, uint8_t *obj_start_ptr, vx_bool inUse[],
    uint32_t max_objects, uint32_t size)
{
    uint32_t i;
    vx_status status = VX_FAILURE;

    for (i = 0;i < max_objects; i ++)
    {
        if ((obj_ptr == obj_start_ptr) && (vx_true_e == inUse[i]))
        {
            inUse[i] = vx_false_e;
            status = VX_SUCCESS;
            break;
        }
        obj_start_ptr += size;
    }

    return (status);
}

static void ownInitUseFlag(vx_bool inUse[], uint32_t num_ele)
{
    uint32_t i;

    for (i = 0; i < num_ele; i ++)
    {
        inUse[i] = vx_false_e;
    }
}

static vx_status ownCheckUseFlag(vx_bool inUse[], uint32_t num_ele)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;

    for (i = 0; i < num_ele; i ++)
    {
        if (vx_true_e == inUse[i])
        {
            status = VX_FAILURE;
        }
    }

    return (status);
}

