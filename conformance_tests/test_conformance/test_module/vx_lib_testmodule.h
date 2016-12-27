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

#pragma once
#ifndef _OPENVX_EXT_TESTMODULE_H_
#define _OPENVX_EXT_TESTMODULE_H_

/*!
 * \file
 * \brief Module with test kernel for use in tests.
 *
 * \defgroup group_testmodule_lib The module with test kernel for use in tests
 *
 */

#include <VX/vx.h>

/*!
 * \file vx_lib_testmodule.h
 * \brief The header for test kernel module
 */

/*! \brief The TestModule Data area in bytes
 * \ingroup group_testmodule_lib
 */
#define TESTMODULE_DATA_AREA (1024)

/*! \brief The required number of items in the temp array
 * \ingroup group_testmodule_lib
 */
#define TESTMODULE_TEMP_NUMITEMS (374)

/*! \brief The minimum value of the scalar for the TestModule Kernel.
 * \ingroup group_testmodule_lib
 */
#define TESTMODULE_VALUE_MIN   (-10)

/*! \brief The maximum value of the scalar for the TestModule Kernel.
 * \ingroup group_testmodule_lib
 */
#define TESTMODULE_VALUE_MAX  (10)

//! [KERNEL ENUM]
#define VX_KERNEL_NAME_KHR_TESTMODULE "org.khronos.test.testmodule"
/*! \brief The TESTMODULE Example Library Set
 * \ingroup group_testmodule_lib
 */
#define VX_LIBRARY_TESTMODULE (0x4) // assigned from Khronos, vendors control their own

/*! \brief The list of TestModule Kernels.
 * \ingroup group_testmodule_lib
 */
enum vx_kernel_testmodule_ext_e {
    /*! \brief The Example User Defined Kernel */
    VX_KERNEL_KHR_TESTMODULE = VX_KERNEL_BASE(VX_ID_DEFAULT, VX_LIBRARY_TESTMODULE) + 0x0,
    // up to 0xFFF kernel enums can be created.
};
//! [KERNEL ENUM]

#ifdef __cplusplus
extern "C" {
#endif

//! [node]
/*! \brief [Graph] This is an example ISV or OEM provided node which executes
 * in the Graph to call the TestModule Kernel.
 * \param [in] graph The handle to the graph in which to instantiate the node.
 * \param [in] input The input image.
 * \param [in] value The input scalar value
 * \param [out] output The output image.
 * \param [in,out] temp A temp array for some data which is needed for
 * every iteration.
 * \ingroup group_example_kernel
 */
vx_node vxTestModuleNode(vx_graph graph, vx_image input, vx_uint32 value, vx_image output, vx_array temp);
//! [node]

//! [vxu]
/*! \brief [Immediate] This is an example of an immediate mode version of the TESTMODULE node.
 * \param [in] context The overall context of the implementation.
 * \param [in] input The input image.
 * \param [in] value The input scalar value
 * \param [out] output The output image.
 * \param [in,out] temp A temp array for some data which is needed for
 * every iteration.
 * \ingroup group_example_kernel
 */
vx_status vxuTestModule(vx_context context, vx_image input, vx_uint32 value, vx_image output, vx_array temp);
//! [vxu]

#ifdef __cplusplus
}
#endif

#endif
