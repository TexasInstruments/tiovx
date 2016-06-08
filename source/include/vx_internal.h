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


#ifndef _VX_INTERNAL_H_
#define _VX_INTERNAL_H_

#include <VX/vx.h>

#include <vx_reference.h>
#include <vx_context.h>

#include <tivx_obj_desc.h>
#include <tivx_mem.h>

#include <vx_graph.h>
#include <vx_node.h>
#include <vx_remap.h>
#include <vx_scalar.h>
#include <vx_image.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The top level TI OpenVX implementation header.
 */

/*! \brief A parameter checker for size and alignment.
 * \ingroup group_vx_utils
 */
#define VX_CHECK_PARAM(ptr, size, type, align) (size == sizeof(type) && ((vx_size)ptr & align) == 0)


/*! \brief Macro to align a 'value' to 'align' units
 * \ingroup group_vx_utils
 */
#define TIVX_ALIGN(value, align)      ((((value)+(align-1))/(align))*(align))

/*! \brief Macro to floor a 'value' to 'align' units
 * \ingroup group_vx_utils
 */
#define TIVX_FLOOR(value, align)      (((value)/(align))*(align))

/*! \brief Macro to specify default alignment to use for stride in Y-direction
 * \ingroup group_vx_utils
 */
#define TIVX_DEFAULT_STRIDE_Y_ALIGN   (32U)

#ifdef __cplusplus
}
#endif

#endif

/*!
 *
 * \mainpage OpenVX - TI Implementation - API Guide
 *
 * \par Introduction
 *
 *      TI's implementation of OpenVX
 *
 * \par IMPORTANT NOTE
 *
 *      See also \ref TI_DISCLAIMER.
 */

/*!
 *
 * \page  TI_DISCLAIMER  TI Disclaimer
 *
 * \htmlinclude ti_disclaim.htm
 *
 */

/*!
 * \defgroup group_vx_framework OpenVX Implementation
 */

/*!
 * \defgroup group_vx_context Object: Context APIs
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_vx_graph Object: Graph APIs
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_vx_node Object: Node APIs
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_vx_reference Object: Reference APIs
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_vx_image Object: Image APIs
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_vx_scalar Object: Scalar APIs
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_vx_remap Object: Remap APIs
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_vx_utils Utility: Common Utility APIs
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_vx_platform Platform Specific APIs
 */

/*!
 * \defgroup group_tivx_mem Platform: Memory APIs
 * \ingroup group_vx_platform
 */

/*!
 * \defgroup group_tivx_obj_desc Platform: Object Descriptor APIs
 * \ingroup group_vx_platform
 */
