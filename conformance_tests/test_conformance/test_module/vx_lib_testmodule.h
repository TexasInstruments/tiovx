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
