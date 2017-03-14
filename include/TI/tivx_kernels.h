/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_KERNELS_H_
#define TIVX_KERNELS_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The list of supported kernels in the TIOVX.
 */

/*!
 * \brief The list of kernels supported in ivision.
 *
 * Each kernel listed here can be used with the <tt>\ref vxGetKernelByEnum</tt> call.
 * When programming the parameters, use
 * \arg <tt>\ref VX_INPUT</tt> for [in]
 * \arg <tt>\ref VX_OUTPUT</tt> for [out]
 * \arg <tt>\ref VX_BIDIRECTIONAL</tt> for [in,out]
 *
 * When programming the parameters, use
 * \arg <tt>\ref VX_TYPE_IMAGE</tt> for a <tt>\ref vx_image</tt> in the size field of <tt>\ref vxGetParameterByIndex</tt> or <tt>\ref vxSetParameterByIndex</tt>  * \arg <tt>\ref VX_TYPE_ARRAY</tt> for a <tt>\ref vx_array</tt> in the size field of <tt>\ref vxGetParameterByIndex</tt> or <tt>\ref vxSetParameterByIndex</tt>  * \arg or other appropriate types in \ref vx_type_e.
 * \ingroup group_kernel
 */
enum tivx_kernel_ivision_e {
    /*! \brief The Harris Corners Kernel.
     * \see group_vision_function_harris
     */
    TIVX_KERNEL_IVISION_HARRIS_CORNERS = VX_KERNEL_BASE(VX_ID_TI, VX_LIBRARY_IVISION_BASE) + 0x0,

    /* insert new kernels here */
    TIVX_KERNEL_IVISION_MAX_1_0, /*!< \internal Used for bounds checking in the conformance test. */
};



#ifdef __cplusplus
}
#endif

#endif
