/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
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

#ifndef TDA4X_H_
#define TDA4X_H_

#include <TI/tivx.h>
#include <TI/tda4x_kernels.h>
#include <TI/tda4x_nodes.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to TI extension APIs
 */

/*! \brief Target name for VPAC NF
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_VPAC_NF      "RESV00"
/*! \brief Target name for VPAC LDC1
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_VPAC_LDC1    "RESV01"
/*! \brief Target name for VPAC LDC2
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_VPAC_LDC2    "RESV02"
/*! \brief Target name for VPAC MSC1
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_VPAC_MSC1    "RESV03"
/*! \brief Target name for VPAC MSC2
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_VPAC_MSC2    "RESV04"
/*! \brief Target name for VPAC SDE
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_DMPAC_SDE    "RESV05"
/*! \brief Target name for VPAC DOF
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_DMPAC_DOF    "RESV06"
/*! \brief Target name for VPAC VISS1
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_VPAC_VISS1   "RESV07"

/*! \brief Based on the VX_DF_IMAGE definition.
 * \note Use <tt>\ref vx_df_image</tt> to contain these values.
 * \ingroup group_tivx_ext
 */
enum tivx_df_image_e {
    /*! \brief A single plane of packed 12-bit data.
     */
    TIVX_DF_IMAGE_P12 = VX_DF_IMAGE('P','0','1','2'),
};

#ifdef __cplusplus
}
#endif

#endif

/*!
 * \defgroup group_tivx_api 1: TIOVX Interface Modules
 */

/*!
 * \defgroup group_tivx_ext TI Extention APIs
 * \ingroup group_tivx_api
 */

/*!
 * \defgroup group_vision_function_hwa TI HWA Kernel APIs
 * \ingroup group_tivx_api
 */

/*!
 * \defgroup group_vision_function_vpac_nf TI VPAC NF (Noise filter) Kernel APIs
 * \ingroup group_vision_function_hwa
 */

/*!
 * \defgroup group_vision_function_vpac_msc TI VPAC MSC (Multi-scalar) Kernel APIs
 * \ingroup group_vision_function_hwa
 */

/*!
 * \defgroup group_vision_function_vpac_viss TI VPAC VISS (Vision Imaging Sub System) Kernel APIs
 * \ingroup group_vision_function_hwa
 */

/*!
 * \defgroup group_vision_function_vpac_ldc TI VPAC LDC (Lens Distortion Correction) Kernel APIs
 * \ingroup group_vision_function_hwa
 */

/*!
 * \defgroup group_vision_function_dmpac_dof TI DMPAC DOF (Dense Optical Flow) Kernel APIs
 * \ingroup group_vision_function_hwa
 */

/*!
 * \defgroup group_vision_function_dmpac_sde TI DMPAC SDE (Stereo Disparity Engine) Kernel APIs
 * \ingroup group_vision_function_hwa
 */