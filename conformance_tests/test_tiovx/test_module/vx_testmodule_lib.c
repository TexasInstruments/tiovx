/*
 * Copyright (c) 2013-2016 The Khronos Group Inc.
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

#include <VX/vx.h>

#include "vx_lib_testmodule.h"

#define dimof(x) (sizeof(x)/sizeof(x[0]))

//! [node]
vx_node vxTestModuleNode(vx_graph graph, vx_image input, vx_uint32 value, vx_image output, vx_array temp)
{
    vx_uint32 i;
    vx_node node = 0;
    vx_context context = vxGetContext((vx_reference)graph);
    vx_status status = vxLoadKernels(context, "test");
    if (status == VX_SUCCESS)
    {
        //! [test node]
        vx_kernel kernel = vxGetKernelByName(context, VX_KERNEL_NAME_KHR_TESTMODULE);
        if (kernel)
        {
            node = vxCreateGenericNode(graph, kernel);
            if (vxGetStatus((vx_reference)node) == VX_SUCCESS)
            {
                vx_status statuses[4];
                vx_scalar scalar = vxCreateScalar(context, VX_TYPE_INT32, &value);
                statuses[0] = vxSetParameterByIndex(node, 0, (vx_reference)input);
                statuses[1] = vxSetParameterByIndex(node, 1, (vx_reference)scalar);
                statuses[2] = vxSetParameterByIndex(node, 2, (vx_reference)output);
                statuses[3] = vxSetParameterByIndex(node, 3, (vx_reference)temp);
                vxReleaseScalar(&scalar);
                for (i = 0; i < dimof(statuses); i++)
                {
                    if (statuses[i] != VX_SUCCESS)
                    {
                        status = VX_ERROR_INVALID_PARAMETERS;
                        vxReleaseNode(&node);
                        vxReleaseKernel(&kernel);
                        node = 0;
                        kernel = 0;
                        break;
                    }
                }
            }
            else
            {
                vxReleaseKernel(&kernel);
            }
        }
        else
        {
            vxUnloadKernels(context, "test");
        }
        //! [test node]
    }
    return node;
}
//! [node]

//! [vxu]
vx_status vxuTestModule(vx_context context, vx_image input, vx_uint32 value, vx_image output, vx_array temp)
{
    vx_status status = VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == VX_SUCCESS)
    {
        vx_node node = vxTestModuleNode(graph, input, value, output, temp);
        if (node)
        {
            status = vxVerifyGraph(graph);
            if (status == VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}
//! [vxu]
