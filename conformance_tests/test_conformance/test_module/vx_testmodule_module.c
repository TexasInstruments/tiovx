/*

 * Copyright (c) 2013-2017 The Khronos Group Inc.
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

#include <VX/vx.h>

#include "vx_lib_testmodule.h"

/*! An internal definition of the order of the parameters to the function.
 * This list must match the parameter list in the function and in the
 * publish kernel list.
 * \ingroup group_testmodule_kernel
 */
typedef enum _test_params_e {
    TESTMODULE_PARAM_INPUT = 0,
    TESTMODULE_PARAM_VALUE,
    TESTMODULE_PARAM_OUTPUT,
    TESTMODULE_PARAM_TEMP
} test_params_e;

/*! \brief An example parameter validator.
 * \param [in] node The handle to the node.
 * \param [in] parameters The array of parameters to be validated.
 * \param [in] num Number of parameters to be validated.
 * \param [out] metas The array of metadata used to check output parameters only.
 * \return A \ref vx_status_e enumeration.
 * \ingroup group_testmodule_kernel
 */
vx_status VX_CALLBACK TestModuleValidator(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    vx_uint32 i;

    if (num != (TESTMODULE_PARAM_TEMP + 1))
        return VX_ERROR_INVALID_PARAMETERS;

    for (i = 0u; i < num; i++)
    {
        if (!parameters[i])
            return VX_ERROR_INVALID_REFERENCE;

        switch (i)
        {
            case TESTMODULE_PARAM_INPUT:
            {
                vx_df_image df_image = 0;

                if (vxQueryImage((vx_image)parameters[i], VX_IMAGE_FORMAT, &df_image, sizeof(df_image)) != VX_SUCCESS)
                    return VX_ERROR_INVALID_PARAMETERS;

                if (df_image != VX_DF_IMAGE_U8)
                    return VX_ERROR_INVALID_VALUE;

            }   break;
            case TESTMODULE_PARAM_VALUE:
            {
                vx_enum type = 0;
                vx_int32 value = 0;

                if (vxQueryScalar((vx_scalar)parameters[i], VX_SCALAR_TYPE, &type, sizeof(type)) != VX_SUCCESS)
                    return VX_ERROR_INVALID_PARAMETERS;

                if (type != VX_TYPE_INT32 ||
                    vxCopyScalar((vx_scalar)parameters[i], &value, VX_READ_ONLY, VX_MEMORY_TYPE_HOST) != VX_SUCCESS)
                    return VX_ERROR_INVALID_PARAMETERS;

                if (TESTMODULE_VALUE_MIN >= value || value >= TESTMODULE_VALUE_MAX)
                    return VX_ERROR_INVALID_VALUE;

            }   break;
            case TESTMODULE_PARAM_OUTPUT:
            {
                vx_image input = (vx_image)parameters[TESTMODULE_PARAM_INPUT];
                vx_uint32 width = 0, height = 0;
                vx_df_image format = VX_DF_IMAGE_VIRT;

                if (!metas[i])
                    return VX_ERROR_INVALID_REFERENCE;

                if (vxQueryImage(input, VX_IMAGE_FORMAT, &format, sizeof(format)) != VX_SUCCESS ||
                    vxQueryImage(input, VX_IMAGE_WIDTH, &width, sizeof(width)) != VX_SUCCESS ||
                    vxQueryImage(input, VX_IMAGE_HEIGHT, &height, sizeof(height)) != VX_SUCCESS)
                    return VX_ERROR_INVALID_PARAMETERS;

                if (vxSetMetaFormatAttribute(metas[i], VX_IMAGE_WIDTH, &width, sizeof(width)) != VX_SUCCESS ||
                    vxSetMetaFormatAttribute(metas[i], VX_IMAGE_HEIGHT, &height, sizeof(height)) != VX_SUCCESS ||
                    vxSetMetaFormatAttribute(metas[i], VX_IMAGE_FORMAT, &format, sizeof(format)))
                    return VX_ERROR_INVALID_VALUE;

            }   break;
            case TESTMODULE_PARAM_TEMP:
            {
                vx_size num_items = 0;

                if (vxQueryArray((vx_array)parameters[i], VX_ARRAY_NUMITEMS, &num_items, sizeof(num_items)) != VX_SUCCESS)
                    return VX_ERROR_INVALID_PARAMETERS;

                if (num_items < TESTMODULE_TEMP_NUMITEMS)
                    return VX_ERROR_INVALID_DIMENSION;

            }   break;
            default:
                return VX_ERROR_INVALID_PARAMETERS;
        }
    }

    return VX_SUCCESS;
}

/*!
 * \brief The private kernel function for TestModule.
 * \note This is not called directly by users.
 * \param [in] node The handle to the node this kernel is instanced into.
 * \param [in] parameters The array of \ref vx_reference references.
 * \param [in] num The number of parameters in the array.
 * functions.
 * \return A \ref vx_status_e enumeration.
 * \retval VX_SUCCESS Successful return.
 * \retval VX_ERROR_INVALID_PARAMETER The input or output image were
 * of the incorrect dimensions.
 * \ingroup group_testmodule_kernel
 */
vx_status VX_CALLBACK TestModuleKernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (num == 4)
    {
        vx_image input  = (vx_image)parameters[0];
        vx_scalar scalar = (vx_scalar)parameters[1];
        vx_image output = (vx_image)parameters[2];
        vx_array temp  = (vx_array)parameters[3];
        void *buf, *in = NULL, *out = NULL;
        vx_uint32 y, x;
        vx_int32 value = 0;
        vx_imagepatch_addressing_t addr1, addr2;
        vx_rectangle_t rect;
        vx_enum item_type = VX_TYPE_INVALID;
        vx_size num_items = 0, capacity = 0;
        vx_size stride = 0;
        vx_map_id map_id_input, map_id_output, map_id_array;

        status = VX_SUCCESS;

        status |= vxCopyScalar(scalar, &value, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
        status |= vxGetValidRegionImage(input, &rect);
        status |= vxMapImagePatch(input, &rect, 0, &map_id_input, &addr1, &in, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);
        status |= vxMapImagePatch(output, &rect, 0, &map_id_output, &addr2, &out, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);
        status |= vxQueryArray(temp, VX_ARRAY_ITEMTYPE, &item_type, sizeof(item_type));
        status |= vxQueryArray(temp, VX_ARRAY_NUMITEMS, &num_items, sizeof(num_items));
        status |= vxQueryArray(temp, VX_ARRAY_CAPACITY, &capacity, sizeof(capacity));
        status |= vxMapArrayRange(temp, 0, num_items, &map_id_array, &stride, &buf, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0);
        for (y = 0; y < addr1.dim_y; y+=addr1.step_y)
        {
            for (x = 0; x < addr1.dim_x; x+=addr1.step_x)
            {
                // do some operation...
            }
        }
        status |= vxUnmapArrayRange(temp, map_id_array);
        status |= vxUnmapImagePatch(output, map_id_output);
        status |= vxUnmapImagePatch(input, map_id_input);
    }
    return status;
}

/*! \brief An initializer function.
 * \param [in] node The handle to the node this kernel is instanced into.
 * \param [in] parameters The array of \ref vx_reference references.
 * \param [in] num The number of parameters in the array.
 * functions.
 * \return A \ref vx_status_e enumeration.
 * \retval VX_SUCCESS Successful return.
 * \retval VX_ERROR_INVALID_PARAMETER The input or output image were
 * of the incorrect dimensions.
 * \ingroup group_testmodule_kernel
 */
vx_status VX_CALLBACK TestModuleInitialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    /* TestModule requires no initialization of memory or resources */
    return VX_SUCCESS;
}

/*! \brief A deinitializer function.
 * \param [in] node The handle to the node this kernel is instanced into.
 * \param [in] parameters The array of \ref vx_reference references.
 * \param [in] num The number of parameters in the array.
 * functions.
 * \return A \ref vx_status_e enumeration.
 * \retval VX_SUCCESS Successful return.
 * \retval VX_ERROR_INVALID_PARAMETER The input or output image were
 * of the incorrect dimensions.
 * \ingroup group_testmodule_kernel
 */
vx_status VX_CALLBACK TestModuleDeinitialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    /* TestModule requires no de-initialization of memory or resources */
    return VX_SUCCESS;
}

//**********************************************************************
//  PUBLIC FUNCTION
//**********************************************************************

/*! \brief The entry point into this module to add the extensions to OpenVX.
 * \param [in] context The handle to the implementation context.
 * \return A \ref vx_status_e enumeration. Returns errors if some or all kernels were not added
 * correctly.
 * \note This follows the function pointer definition of a \ref vx_publish_kernels_f
 * and uses the predefined name for the entry point, "vxPublishKernels".
 * \ingroup group_testmodule_kernel
 */
/*VX_API_ENTRY*/ vx_status VX_API_CALL vxPublishKernels(vx_context context)
{
    vx_status status = VX_SUCCESS;
    vx_kernel kernel = vxAddUserKernel(context,
                                    "org.khronos.test.testmodule",
                                    VX_KERNEL_KHR_TESTMODULE,
                                    TestModuleKernel,
                                    4,
                                    TestModuleValidator,
                                    TestModuleInitialize,
                                    TestModuleDeinitialize);
    if (kernel)
    {
        vx_size size = TESTMODULE_DATA_AREA;
        status = vxAddParameterToKernel(kernel, 0, VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED);
        if (status != VX_SUCCESS) goto exit;
        status = vxAddParameterToKernel(kernel, 1, VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED);
        if (status != VX_SUCCESS) goto exit;
        status = vxAddParameterToKernel(kernel, 2, VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED);
        if (status != VX_SUCCESS) goto exit;
        status = vxAddParameterToKernel(kernel, 3, VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED);
        if (status != VX_SUCCESS) goto exit;
        status = vxSetKernelAttribute(kernel, VX_KERNEL_LOCAL_DATA_SIZE, &size, sizeof(size));
        if (status != VX_SUCCESS) goto exit;
        status = vxFinalizeKernel(kernel);
        if (status != VX_SUCCESS) goto exit;
    }
exit:
    if (status != VX_SUCCESS) {
        vxRemoveKernel(kernel);
    }
    return status;
}

/*! \brief The destructor to remove a user loaded module from OpenVX.
 * \param [in] context The handle to the implementation context.
 * \return A \ref vx_status_e enumeration. Returns errors if some or all kernels were not added
 * correctly.
 * \note This follows the function pointer definition of a \ref vx_unpublish_kernels_f
 * and uses the predefined name for the entry point, "vxUnpublishKernels".
 * \ingroup group_testmodule_kernel
 */
/*VX_API_ENTRY*/ vx_status VX_API_CALL vxUnpublishKernels(vx_context context)
{
    vx_status status = VX_SUCCESS;
    vx_kernel kernel = vxGetKernelByName(context, "org.khronos.test.testmodule");
    vx_kernel kernelcpy = kernel;

    if (kernel)
    {
        status = vxReleaseKernel(&kernelcpy);
        if (status == VX_SUCCESS)
        {
            status = vxRemoveKernel(kernel);
        }
    }
    return status;
}

#include <TI/tivx.h>

void TestModuleRegister()
{
    tivxRegisterModule("test-testmodule", vxPublishKernels, vxUnpublishKernels);
}

void TestModuleUnRegister()
{
    tivxUnRegisterModule("test-testmodule");
}
