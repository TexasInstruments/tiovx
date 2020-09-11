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



#ifndef VX_INTERNAL_H_
#define VX_INTERNAL_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <VX/vx.h>
#include <VX/vx_khr_pipelining.h>
#include <TI/tivx.h>
#include <TI/tivx_mem.h>
#include <TI/tivx_obj_desc.h>
#include <TI/tivx_target_kernel.h>
#include <TI/tivx_debug.h>
#include <tivx_platform.h>
#include <tivx_ipc.h>
#include <TI/tivx_mutex.h>
#include <TI/tivx_event.h>
#include <TI/tivx_task.h>
#include <TI/tivx_queue.h>
#include <TI/tivx_config.h>
#include <tivx_obj_desc_priv.h>
#include <tivx_target.h>
#include <tivx_target_kernel_priv.h>
#include <tivx_target_kernel_instance.h>
#include <tivx_obj_desc_queue.h>
#include <tivx_event_queue.h>

#include <vx_reference.h>
#include <tivx_data_ref_queue.h>
#include <vx_context.h>
#include <vx_error.h>
#include <vx_graph.h>
#include <vx_kernel.h>
#include <vx_node.h>
#include <vx_parameter.h>
#include <vx_remap.h>
#include <vx_scalar.h>
#include <vx_image.h>
#include <vx_matrix.h>
#include <vx_lut.h>
#include <vx_convolution.h>
#include <vx_distribution.h>
#include <vx_threshold.h>
#include <vx_pyramid.h>
#include <vx_objarray.h>
#include <vx_array.h>
#include <vx_user_data_object.h>
#include <tivx_raw_image.h>
#include <tivx_super_node.h>
#include <vx_tensor.h>
#include <vx_delay.h>
#include <vx_module.h>
#include <vx_meta_format.h>
#include <tivx_objects.h>
#include <tivx_log_rt_trace.h>
#include <tivx_log_resource.h>
#include <tivx_kernels_host_utils.h>


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The top level TI OpenVX implementation header.
 */

/*! \brief Macro to align a 'value' to 'align' units
 * \ingroup group_vx_utils
 */
#define TIVX_ALIGN(value, align)      ((((value)+((align)-1U))/(align))*(align))

/*! \brief Macro to floor a 'value' to 'align' units
 * \ingroup group_vx_utils
 */
#define TIVX_FLOOR(value, align)      (((value)/(align))*(align))

/*! \brief Macro to specify default alignment to use for stride in Y-direction
 *
 *         Video codecs require 64 byte alignment
 *
 *         Remaining HWA nodes require 16 byte alignment
 *
 * \ingroup group_vx_utils
 */
#define TIVX_DEFAULT_STRIDE_Y_ALIGN   (64U)

/*! \brief Used to determine if a type is a scalar.
 * \ingroup group_vx_utils
 */
#define TIVX_TYPE_IS_SCALAR(type) (((vx_enum)VX_TYPE_INVALID < (type)) && ((type) < (vx_enum)VX_TYPE_SCALAR_MAX))

/*! \brief Used to determine if a type is a struct.
 * \ingroup group_vx_utils
 */
#define TIVX_TYPE_IS_STRUCT(type) (((type) >= (vx_enum)VX_TYPE_RECTANGLE) && ((type) < (vx_enum)VX_TYPE_KHRONOS_STRUCT_MAX))

/*! \brief Used to determine if a type is an Khronos defined object.
 * \ingroup group_vx_utils
 */
#define TIVX_TYPE_IS_OBJECT(type) (((type) >= (vx_enum)VX_TYPE_REFERENCE) && ((type) < (vx_enum)VX_TYPE_KHRONOS_OBJECT_END))

/*! \brief Used to determine if a type is TI defined object.
 * \ingroup group_vx_utils
 */
#define TIVX_TYPE_IS_TI_OBJECT(type) (((type) >= (vx_enum)VX_TYPE_VENDOR_OBJECT_START) && ((type) < (vx_enum)VX_TYPE_VENDOR_OBJECT_END))

/*! \brief A magic value to look for and set in references.
 * \ingroup group_vx_utils
 */
#define TIVX_MAGIC            (0xFACEC0DEU)


/*! \brief A magic value to look for and set in references. Used to indicate a free'ed reference
 * \ingroup group_vx_utils
 */
#define TIVX_BAD_MAGIC        (42U)

/*! \brief TIVX defined reference types
 * \ingroup group_vx_utils
 */
enum tivx_type_e {

    TIVX_TYPE_DATA_REF_Q = VX_TYPE_VENDOR_OBJECT_START, /*! \brief Data reference queue type */
};

/*! \brief A parameter checker for size and alignment.
 * \ingroup group_vx_utils
 */
#define VX_CHECK_PARAM(ptr, size, type, align) ((size == sizeof(type)) && (((vx_size)ptr & align) == 0U))

static inline void tivx_uint32_to_uint64(volatile uint64_t *val, uint32_t h, uint32_t l);
static inline void tivx_uint64_to_uint32(uint64_t val, volatile uint32_t *h, volatile uint32_t *l);

/*! \brief Macro to convert 2x uint32 to uint64
 * \ingroup group_vx_utils
 */
static inline void tivx_uint32_to_uint64(volatile uint64_t *val, uint32_t h, uint32_t l)
{
    *val = ((uint64_t)h<<32) | (uint64_t)l;
}

/*! \brief Macro to convert uint64 to 2x uint32
 * \ingroup group_vx_utils
 */
static inline void tivx_uint64_to_uint32(uint64_t val, volatile uint32_t *h, volatile uint32_t *l)
{
    *h = (uint32_t)(val >> 32);
    *l = (uint32_t)(val >>  0);
}


#ifdef __cplusplus
}
#endif

#endif

