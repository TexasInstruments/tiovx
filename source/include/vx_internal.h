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
#include <vx_tensor.h>
#include <vx_delay.h>
#include <vx_module.h>
#include <vx_meta_format.h>
#include <tivx_objects.h>
#include <tivx_log_rt_trace.h>
#include <tivx_log_resource.h>


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
#define TIVX_ALIGN(value, align)      ((((value)+((align)-1))/(align))*(align))

/*! \brief Macro to floor a 'value' to 'align' units
 * \ingroup group_vx_utils
 */
#define TIVX_FLOOR(value, align)      (((value)/(align))*(align))

/*! \brief Macro to specify default alignment to use for stride in Y-direction
 * \ingroup group_vx_utils
 */
#define TIVX_DEFAULT_STRIDE_Y_ALIGN   (16U)

/*! \brief Used to determine if a type is a scalar.
 * \ingroup group_vx_utils
 */
#define TIVX_TYPE_IS_SCALAR(type) ((VX_TYPE_INVALID < (type)) && ((type) < VX_TYPE_SCALAR_MAX))

/*! \brief Used to determine if a type is a struct.
 * \ingroup group_vx_utils
 */
#define TIVX_TYPE_IS_STRUCT(type) (((type) >= VX_TYPE_RECTANGLE) && ((type) < VX_TYPE_KHRONOS_STRUCT_MAX))

/*! \brief Used to determine if a type is an Khronos defined object.
 * \ingroup group_vx_utils
 */
#define TIVX_TYPE_IS_OBJECT(type) (((type) >= VX_TYPE_REFERENCE) && ((type) < VX_TYPE_KHRONOS_OBJECT_END))

/*! \brief Used to determine if a type is TI defined object.
 * \ingroup group_vx_utils
 */
#define TIVX_TYPE_IS_TI_OBJECT(type) (((type) >= VX_TYPE_VENDOR_OBJECT_START) && ((type) < VX_TYPE_VENDOR_OBJECT_END))

/*! \brief A magic value to look for and set in references.
 * \ingroup group_vx_utils
 */
#define TIVX_MAGIC            (0xFACEC0DEU)


/*! \brief A magic value to look for and set in references. Used to indicate a free'ed reference
 * \ingroup group_vx_utils
 */
#define TIVX_BAD_MAGIC        (42)

/*! \brief TIVX defined reference types
 * \ingroup group_vx_utils
 */
enum tivx_type_e {

    TIVX_TYPE_DATA_REF_Q = VX_TYPE_VENDOR_OBJECT_START, /*! \brief Data reference queue type */
};

/*! \brief A parameter checker for size and alignment.
 * \ingroup group_vx_utils
 */
#define VX_CHECK_PARAM(ptr, size, type, align) ((size == sizeof(type)) && (((vx_size)ptr & align) == 0))

static inline vx_bool tivxFlagIsBitSet(uint32_t flag_var, uint32_t flag_val);
static inline void tivxFlagBitSet(uint32_t *flag_var, uint32_t flag_val);
static inline void tivxFlagBitClear(uint32_t *flag_var, uint32_t flag_val);
static inline void tivx_uint32_to_uint64(uint64_t *val, uint32_t h, uint32_t l);
static inline void tivx_uint64_to_uint32(uint64_t val, uint32_t *h, uint32_t *l);

/*! \brief Macro to check if flag is set, flag MUST be of bit type
 * \ingroup group_vx_utils
 */
static inline vx_bool tivxFlagIsBitSet(uint32_t flag_var, uint32_t flag_val)
{
    return (vx_bool)((flag_var & flag_val) == flag_val);
}

/*! \brief Macro to set flag value, flag MUST be of bit type
 * \ingroup group_vx_utils
 */
static inline void tivxFlagBitSet(uint32_t *flag_var, uint32_t flag_val)
{
    *flag_var |= flag_val;
}

/*! \brief Macro to clear flag value, flag MUST be of bit type
 * \ingroup group_vx_utils
 */
static inline void tivxFlagBitClear(uint32_t *flag_var, uint32_t flag_val)
{
    uint32_t value = *flag_var;

    value = value & ~flag_val;

    *flag_var = value;
}

/*! \brief Macro to convert 2x uint32 to uint64
 * \ingroup group_vx_utils
 */
static inline void tivx_uint32_to_uint64(uint64_t *val, uint32_t h, uint32_t l)
{
    *val = ((uint64_t)h<<32) | (uint64_t)l;
}

/*! \brief Macro to convert uint64 to 2x uint32
 * \ingroup group_vx_utils
 */
static inline void tivx_uint64_to_uint32(uint64_t val, uint32_t *h, uint32_t *l)
{
    *h = (uint32_t)(val >> 32);
    *l = (uint32_t)(val >>  0);
}


#ifdef __cplusplus
}
#endif

#endif

/*!
 * \defgroup group_tivx_api 1: TIOVX External APIs
 */

/*!
 * \defgroup group_tivx_ext_common a: TIOVX Common APIs
 * \brief APIs accessible on both host and target
 * \ingroup group_tivx_api
 */

/*!
 * \defgroup group_vx_framework_config TIOVX Configuration Parameters
 * \brief Parameters used for static configuration of data structures
 * \ingroup group_tivx_ext_common
 */

/*!
 * \defgroup group_tivx_obj_desc_priv Object Descriptor APIs
 * \brief APIs for object descriptor manipulation
 * \ingroup group_tivx_ext_common
 */

/*!
 * \defgroup group_tivx_ext_host_kernel Kernel Helper APIs
 * \brief Helper APIs used used for host side kernel validation
 * \ingroup group_tivx_ext_host
 */

/*!
 * \defgroup group_vx_context_cfg Context Configuration
 * \brief Context static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_graph_cfg Graph Configuration
 * \brief Graph static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_tivx_target_cfg Target Configuration
 * \brief Target static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_tivx_target_kernel_cfg Target Kernel Configuration
 * \brief Target kernel static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_tivx_target_kernel_instance_cfg Target Kernel Instance Configuration
 * \brief Target kernel instance static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_module_cfg Module Configuration Configuration
 * \brief Module static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_tivx_obj_desc_cfg Object Descriptor Configuration
 * \brief Object Descriptor static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_image_cfg Data Object: Image Configuration
 * \brief Image static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_array_cfg Data Object: Array Configuration
 * \brief Array static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_delay_cfg Data Object: Delay Configuration
 * \brief Delay static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_pyramid_cfg Data Object: Pyramid Configuration
 * \brief Pyramid static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_tivx_obj_cfg Object Configuration
 * \brief Object static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_tivx_ext_host b: TIOVX Host APIs
 * \brief APIs accessible only on host
 * \ingroup group_tivx_api
 */

/*!
 * \defgroup group_tivx_ext_target c: TIOVX Target APIs
 * \brief APIs accessible only on target
 * \ingroup group_tivx_api
 */

/*!
 * \defgroup group_tivx_mem Memory APIs
 * \brief APIs for memory mapping on host and target
 * \ingroup group_tivx_ext_common
 */

/*!
 * \defgroup group_tivx_target_kernel Target Kernel APIs
 * \brief APIs for kernel operations on the target
 * \ingroup group_tivx_ext_target
 */

/*!
 * \defgroup group_tivx_queue Queue APIs
 * \brief APIs for queue operations on the host
 * \ingroup group_tivx_ext_host
 */

/*!
 * \defgroup group_tivx_mutex Mutex APIs
 * \brief APIs for mutex operations on the host
 * \ingroup group_tivx_ext_host
 */

/*!
 * \defgroup group_tivx_event Event APIs
 * \brief APIs for event operations on the host
 * \ingroup group_tivx_ext_host
 */

/*!
 * \defgroup group_tivx_task Task APIs
 * \brief APIs for task operations on the host
 * \ingroup group_tivx_ext_host
 */

/*!
 * \defgroup group_vx_debug Debug APIs
 * \brief APIs for debug operations on the host
 * \ingroup group_tivx_ext_host
 */

/*!
 * \defgroup group_vx_internal 2: TIOVX Internal APIs
 */

/*!
 * \defgroup group_vx_framework a: TIOVX Implementation Modules
 * \brief Internal APIs for framework implementation
 * \ingroup group_vx_internal
 */

/*!
 * \defgroup group_tivx_int_common_kernel Kernel Helper APIs
 * \brief Internal APIs for openVX standard 1.1 kernels
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_vx_framework_object Framework Objects
 * \brief Internal APIs for framework object operations
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_vx_framework_data_object Data Objects
 * \brief Internal APIs for framework object operations
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_vx_context Context APIs
 * \brief Internal APIs for context operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_vx_graph Graph APIs
 * \brief Internal APIs for graph operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_vx_node Node APIs
 * \brief Internal APIs for node operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_vx_kernel Kernel APIs
 * \brief Internal APIs for kernel operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_vx_parameter Parameter APIs
 * \brief Internal APIs for parameter operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_vx_reference Reference APIs
 * \brief Internal APIs for reference operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_tivx_target Target APIs
 * \brief Internal APIs for target operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_tivx_target_kernel_priv Target Kernel APIs
 * \brief Internal APIs for target kernel operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_tivx_target_kernel_instance Target Kernel Instance APIs
 * \brief Internal APIs for target kernel instance operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_tivx_obj Object APIs
 * \brief Internal APIs for object operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_vx_module Module APIs
 * \brief Internal APIs for module operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_vx_image Image APIs
 * \brief Internal APIs for image operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_scalar Scalar APIs
 * \brief Internal APIs for scalar operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_remap Remap APIs
 * \brief Internal APIs for remap operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_delay Delay APIs
 * \brief Internal APIs for delay operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_objarray Object Array APIs
 * \brief Internal APIs for object array operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_array Array APIs
 * \brief Internal APIs for array operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_convolution Convolution APIs
 * \brief Internal APIs for convolution operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_distribution Distribution APIs
 * \brief Internal APIs for distribution operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_lut LUT APIs
 * \brief Internal APIs for lut operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_matrix Matrix APIs
 * \brief Internal APIs for matrix operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_pyramid Pyramid APIs
 * \brief Internal APIs for pyramid operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_threshold Threshold APIs
 * \brief Internal APIs for threshold operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_framework_utils Utility and Debug Modules
 * \brief Internal APIs for utility and debug operations
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_vx_utils Utility APIs
 * \brief Internal APIs for utility operations
 * \ingroup group_vx_framework_utils
 */

/*!
 * \defgroup group_vx_error Error APIs
 * \brief Internal APIs for error operations
 * \ingroup group_vx_framework_utils
 */

/*!
 * \defgroup group_vx_platform b: TIOVX Platform Modules
 * \brief Internal APIs for platform-specific operations
 * \ingroup group_vx_internal
 */


/*!
 * \defgroup group_tivx_ipc Inter-Processor Communication (IPC) APIs
 * \brief Internal APIs for IPC operations
 * \ingroup group_vx_platform
 */

/*!
 * \defgroup group_tivx_platform Platform APIs
 * \brief Internal APIs for platform operations
 * \ingroup group_vx_platform
 */

