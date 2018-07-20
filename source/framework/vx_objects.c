/*
*
* Copyright (c) 2018 Texas Instruments Incorporated
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




#include <vx_internal.h>
#include <tivx_objects.h>

static tivx_object_t g_tivx_objects;
static tivx_mutex g_tivx_objects_lock;


static vx_status ownCheckUseFlag(vx_bool inUse[], uint32_t num_ele);
static void ownInitUseFlag(vx_bool inUse[], uint32_t num_ele);
static vx_status ownFreeObject(
    const uint8_t *obj_ptr, const uint8_t *obj_start_ptr, vx_bool inUse[],
    uint32_t max_objects, uint32_t size, char *resource_name);
static uint8_t *ownAllocObject(
    uint8_t *obj_start_ptr, vx_bool inUse[], uint32_t max_objects,
    uint32_t size, char *resource_name);


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
        ownInitUseFlag(g_tivx_objects.isTensorUse,
            TIVX_TENSOR_MAX_OBJECTS);
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
        ownInitUseFlag(g_tivx_objects.isObjArrUse,
            TIVX_OBJ_ARRAY_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isParameterUse,
            TIVX_PARAMETER_MAX_OBJECTS);
        ownInitUseFlag(g_tivx_objects.isDataRefQUse,
            TIVX_DATA_REF_Q_MAX_OBJECTS);
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
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is meta format use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isContextUse,
            TIVX_CONTEXT_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is context use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isGraphUse,
            TIVX_GRAPH_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is graph use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isNodeUse,
            TIVX_NODE_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is node use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isKernelUse,
            TIVX_KERNEL_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is kernel use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isArrayUse,
            TIVX_ARRAY_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is array use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isConvolutionUse,
            TIVX_CONVOLUTION_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is convolution use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isDelayUse,
            TIVX_DELAY_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is delay use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isDistributionUse,
            TIVX_DISTRIBUTION_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is distribution use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isImageUse,
            TIVX_IMAGE_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is image use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isTensorUse,
            TIVX_TENSOR_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is tensor use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isLutUse,
            TIVX_LUT_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is LUT use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isMatrixUse,
            TIVX_MATRIX_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is matrix use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isPyramidUse,
            TIVX_PYRAMID_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is pyramid use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isRemapUse,
            TIVX_REMAP_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is remap use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isScalarUse,
            TIVX_SCALAR_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is scalar use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isThresholdUse,
            TIVX_THRESHOLD_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is threshold use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isErrorUse,
            TIVX_ERROR_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is error use failed\n");
        }
        status = ownCheckUseFlag(g_tivx_objects.isDataRefQUse,
            TIVX_DATA_REF_Q_MAX_OBJECTS);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjectDeInit: Is data ref q use failed\n");
        }
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
                    TIVX_META_FORMAT_MAX_OBJECTS, sizeof(tivx_meta_format_t),
                    "TIVX_META_FORMAT_MAX_OBJECTS");
                break;
            case VX_TYPE_CONTEXT:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.context, g_tivx_objects.isContextUse,
                    TIVX_CONTEXT_MAX_OBJECTS, sizeof(tivx_context_t),
                    "TIVX_CONTEXT_MAX_OBJECTS");
                break;
            case VX_TYPE_GRAPH:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.graph, g_tivx_objects.isGraphUse,
                    TIVX_GRAPH_MAX_OBJECTS, sizeof(tivx_graph_t),
                    "TIVX_GRAPH_MAX_OBJECTS");
                break;
            case VX_TYPE_NODE:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.node, g_tivx_objects.isNodeUse,
                    TIVX_NODE_MAX_OBJECTS, sizeof(tivx_node_t),
                    "TIVX_NODE_MAX_OBJECTS");
                break;
            case VX_TYPE_KERNEL:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.kernel, g_tivx_objects.isKernelUse,
                    TIVX_KERNEL_MAX_OBJECTS, sizeof(tivx_kernel_t),
                    "TIVX_KERNEL_MAX_OBJECTS");
                break;
            case VX_TYPE_ARRAY:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.array, g_tivx_objects.isArrayUse,
                    TIVX_ARRAY_MAX_OBJECTS, sizeof(tivx_array_t),
                    "TIVX_ARRAY_MAX_OBJECTS");
                break;
            case VX_TYPE_CONVOLUTION:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.convolution,
                    g_tivx_objects.isConvolutionUse,
                    TIVX_CONVOLUTION_MAX_OBJECTS, sizeof(tivx_convolution_t),
                    "TIVX_CONVOLUTION_MAX_OBJECTS");
                break;
            case VX_TYPE_DELAY:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.delay, g_tivx_objects.isDelayUse,
                    TIVX_DELAY_MAX_OBJECTS, sizeof(tivx_delay_t),
                    "TIVX_DELAY_MAX_OBJECTS");
                break;
            case VX_TYPE_DISTRIBUTION:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.distribution,
                    g_tivx_objects.isDistributionUse,
                    TIVX_DISTRIBUTION_MAX_OBJECTS, sizeof(tivx_distribution_t),
                    "TIVX_DISTRIBUTION_MAX_OBJECTS");
                break;
            case VX_TYPE_IMAGE:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.image, g_tivx_objects.isImageUse,
                    TIVX_IMAGE_MAX_OBJECTS, sizeof(tivx_image_t),
                    "TIVX_IMAGE_MAX_OBJECTS");
                break;
            case VX_TYPE_TENSOR:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.tensor, g_tivx_objects.isTensorUse,
                    TIVX_TENSOR_MAX_OBJECTS, sizeof(tivx_tensor_t),
                    "TIVX_TENSOR_MAX_OBJECTS");
                break;
            case VX_TYPE_LUT:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.lut, g_tivx_objects.isLutUse,
                    TIVX_LUT_MAX_OBJECTS, sizeof(tivx_lut_t),
                    "TIVX_LUT_MAX_OBJECTS");
                break;
            case VX_TYPE_MATRIX:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.matrix, g_tivx_objects.isMatrixUse,
                    TIVX_MATRIX_MAX_OBJECTS, sizeof(tivx_matrix_t),
                    "TIVX_MATRIX_MAX_OBJECTS");
                break;
            case VX_TYPE_PYRAMID:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.pyramid, g_tivx_objects.isPyramidUse,
                    TIVX_PYRAMID_MAX_OBJECTS, sizeof(tivx_pyramid_t),
                    "TIVX_PYRAMID_MAX_OBJECTS");
                break;
            case VX_TYPE_REMAP:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.remap, g_tivx_objects.isRemapUse,
                    TIVX_REMAP_MAX_OBJECTS, sizeof(tivx_remap_t),
                    "TIVX_REMAP_MAX_OBJECTS");
                break;
            case VX_TYPE_SCALAR:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.scalar, g_tivx_objects.isScalarUse,
                    TIVX_SCALAR_MAX_OBJECTS, sizeof(tivx_scalar_t),
                    "TIVX_SCALAR_MAX_OBJECTS");
                break;
            case VX_TYPE_THRESHOLD:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.threshold, g_tivx_objects.isThresholdUse,
                    TIVX_THRESHOLD_MAX_OBJECTS, sizeof(tivx_threshold_t),
                    "TIVX_THRESHOLD_MAX_OBJECTS");
                break;
            case VX_TYPE_ERROR:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.error, g_tivx_objects.isErrorUse,
                    TIVX_ERROR_MAX_OBJECTS, sizeof(tivx_error_t),
                    "TIVX_ERROR_MAX_OBJECTS");
                break;
            case VX_TYPE_OBJECT_ARRAY:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.obj_array, g_tivx_objects.isObjArrUse,
                    TIVX_OBJ_ARRAY_MAX_OBJECTS, sizeof(tivx_objarray_t),
                    "TIVX_OBJ_ARRAY_MAX_OBJECTS");
                break;
            case VX_TYPE_PARAMETER:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.parameter, g_tivx_objects.isParameterUse,
                    TIVX_PARAMETER_MAX_OBJECTS, sizeof(tivx_parameter_t),
                    "TIVX_PARAMETER_MAX_OBJECTS");
                break;
            case TIVX_TYPE_DATA_REF_Q:
                ref = (vx_reference)ownAllocObject(
                    (uint8_t *)g_tivx_objects.data_ref_q, g_tivx_objects.isDataRefQUse,
                    TIVX_DATA_REF_Q_MAX_OBJECTS, sizeof(tivx_data_ref_queue_t),
                    "TIVX_DATA_REF_Q_MAX_OBJECTS");
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
                case VX_TYPE_PARAMETER:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.parameter,
                        g_tivx_objects.isParameterUse,
                        TIVX_PARAMETER_MAX_OBJECTS,
                        sizeof(tivx_parameter_t),
                        "TIVX_PARAMETER_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free parameter object failed\n");
                    }
                    break;
                case VX_TYPE_OBJECT_ARRAY:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.obj_array,
                        g_tivx_objects.isObjArrUse,
                        TIVX_OBJ_ARRAY_MAX_OBJECTS,
                        sizeof(tivx_objarray_t),
                        "TIVX_OBJ_ARRAY_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free object array object failed\n");
                    }
                    break;
                case VX_TYPE_ERROR:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.error,
                        g_tivx_objects.isErrorUse,
                        TIVX_ERROR_MAX_OBJECTS,
                        sizeof(tivx_error_t),
                        "TIVX_ERROR_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free error object failed\n");
                    }
                    break;
                case VX_TYPE_META_FORMAT:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.meta_format,
                        g_tivx_objects.isMfUse,
                        TIVX_META_FORMAT_MAX_OBJECTS,
                        sizeof(tivx_meta_format_t),
                        "TIVX_META_FORMAT_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free meta format object failed\n");
                    }
                    break;
                case VX_TYPE_CONTEXT:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.context,
                        g_tivx_objects.isContextUse,
                        TIVX_CONTEXT_MAX_OBJECTS, sizeof(tivx_context_t),
                        "TIVX_CONTEXT_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free context object failed\n");
                    }
                    break;
                case VX_TYPE_GRAPH:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.graph, g_tivx_objects.isGraphUse,
                        TIVX_GRAPH_MAX_OBJECTS, sizeof(tivx_graph_t),
                        "TIVX_GRAPH_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free graph object failed\n");
                    }
                    break;
                case VX_TYPE_NODE:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.node, g_tivx_objects.isNodeUse,
                        TIVX_NODE_MAX_OBJECTS, sizeof(tivx_node_t),
                        "TIVX_NODE_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free node object failed\n");
                    }
                    break;
                case VX_TYPE_KERNEL:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.kernel,
                        g_tivx_objects.isKernelUse,
                        TIVX_KERNEL_MAX_OBJECTS, sizeof(tivx_kernel_t),
                        "TIVX_KERNEL_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free kernel object failed\n");
                    }
                    break;
                case VX_TYPE_ARRAY:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.array, g_tivx_objects.isArrayUse,
                        TIVX_ARRAY_MAX_OBJECTS, sizeof(tivx_array_t),
                        "TIVX_ARRAY_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free array object failed\n");
                    }
                    break;
                case VX_TYPE_CONVOLUTION:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.convolution,
                        g_tivx_objects.isConvolutionUse,
                        TIVX_CONVOLUTION_MAX_OBJECTS,
                        sizeof(tivx_convolution_t),
                        "TIVX_CONVOLUTION_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free convolution object failed\n");
                    }
                    break;
                case VX_TYPE_DELAY:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.delay, g_tivx_objects.isDelayUse,
                        TIVX_DELAY_MAX_OBJECTS, sizeof(tivx_delay_t),
                        "TIVX_DELAY_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free delay object failed\n");
                    }
                    break;
                case VX_TYPE_DISTRIBUTION:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.distribution,
                        g_tivx_objects.isDistributionUse,
                        TIVX_DISTRIBUTION_MAX_OBJECTS,
                        sizeof(tivx_distribution_t),
                        "TIVX_DISTRIBUTION_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free distribution object failed\n");
                    }
                    break;
                case VX_TYPE_IMAGE:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.image, g_tivx_objects.isImageUse,
                        TIVX_IMAGE_MAX_OBJECTS, sizeof(tivx_image_t),
                        "TIVX_IMAGE_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free image object failed\n");
                    }
                    break;
                case VX_TYPE_TENSOR:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.tensor, g_tivx_objects.isTensorUse,
                        TIVX_TENSOR_MAX_OBJECTS, sizeof(tivx_tensor_t),
                        "TIVX_TENSOR_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free tensor object failed\n");
                    }
                    break;
                case VX_TYPE_LUT:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.lut, g_tivx_objects.isLutUse,
                        TIVX_LUT_MAX_OBJECTS, sizeof(tivx_lut_t),
                        "TIVX_LUT_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free LUT object failed\n");
                    }
                    break;
                case VX_TYPE_MATRIX:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.matrix,
                        g_tivx_objects.isMatrixUse,
                        TIVX_MATRIX_MAX_OBJECTS, sizeof(tivx_matrix_t),
                        "TIVX_MATRIX_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free matrix object failed\n");
                    }
                    break;
                case VX_TYPE_PYRAMID:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.pyramid,
                        g_tivx_objects.isPyramidUse,
                        TIVX_PYRAMID_MAX_OBJECTS, sizeof(tivx_pyramid_t),
                        "TIVX_PYRAMID_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free pyramid object failed\n");
                    }
                    break;
                case VX_TYPE_REMAP:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.remap, g_tivx_objects.isRemapUse,
                        TIVX_REMAP_MAX_OBJECTS, sizeof(tivx_remap_t),
                        "TIVX_REMAP_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free remap object failed\n");
                    }
                    break;
                case VX_TYPE_SCALAR:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.scalar,
                        g_tivx_objects.isScalarUse,
                        TIVX_SCALAR_MAX_OBJECTS, sizeof(tivx_scalar_t),
                        "TIVX_SCALAR_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free scalar object failed\n");
                    }
                    break;
                case VX_TYPE_THRESHOLD:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.threshold,
                        g_tivx_objects.isThresholdUse,
                        TIVX_THRESHOLD_MAX_OBJECTS, sizeof(tivx_threshold_t),
                        "TIVX_THRESHOLD_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free threshold object failed\n");
                    }
                    break;
                case TIVX_TYPE_DATA_REF_Q:
                    status = ownFreeObject((uint8_t *)ref,
                        (uint8_t *)g_tivx_objects.data_ref_q, g_tivx_objects.isDataRefQUse,
                        TIVX_DATA_REF_Q_MAX_OBJECTS, sizeof(tivx_data_ref_queue_t),
                        "TIVX_DATA_REF_Q_MAX_OBJECTS");
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Free data ref queue object failed\n");
                    }
                    break;
                default:
                    VX_PRINT(VX_ZONE_ERROR,"tivxObjectFree: Invalid reference type\n");
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
    uint32_t size, char *resource_name)
{
    uint32_t i;
    uint8_t *obj_ptr = NULL;

    for (i = 0;i < max_objects; i ++)
    {
        if (vx_false_e == inUse[i])
        {
            obj_ptr = obj_start_ptr;
            inUse[i] = vx_true_e;
            tivxLogResourceAlloc(resource_name, 1);
            break;
        }
        obj_start_ptr += size;
    }

    if (obj_ptr == NULL)
    {
        VX_PRINT(VX_ZONE_WARNING, "ownAllocObject: May need to increase the value of %s in tiovx/include/tivx_config.h\n", resource_name);
    }

    return (obj_ptr);
}

static vx_status ownFreeObject(
    const uint8_t *obj_ptr, const uint8_t *obj_start_ptr, vx_bool inUse[],
    uint32_t max_objects, uint32_t size, char *resource_name)
{
    uint32_t i;
    vx_status status = VX_FAILURE;

    for (i = 0;i < max_objects; i ++)
    {
        if ((obj_ptr == obj_start_ptr) && (vx_true_e == inUse[i]))
        {
            inUse[i] = vx_false_e;
            status = VX_SUCCESS;
            tivxLogResourceFree(resource_name, 1);
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

