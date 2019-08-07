/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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

#ifndef J7_KERNELS_H_
#define J7_KERNELS_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The list of supported kernels in this kernel extension.
 */

/*! \brief Name for OpenVX Extension kernel module: hwa
 * \ingroup group_tivx_ext_top
 */
#define TIVX_MODULE_NAME_HWA    "hwa"

/*! \brief Name for OpenVX Extension kernel module: tidl
 * \ingroup group_tivx_ext_top
 */
#define TIVX_MODULE_NAME_TIDL    "tidl"

/*! \brief dof_visualize kernel name
 *  \ingroup group_vision_function_dmpac_dof
 */
#define TIVX_KERNEL_DOF_VISUALIZE_NAME     "com.ti.hwa.dof_visualize"

/*! \brief Display Kernel Name
 *  \ingroup group_vision_function_display
 */
#define TIVX_KERNEL_DISPLAY_NAME       "com.ti.hwa.display"

/*! \brief capture kernel name
 *  \ingroup group_vision_function_capture
 */
#define TIVX_KERNEL_CAPTURE_NAME          "com.ti.capture"

/*! \brief tidl kernel name
 *  \ingroup group_vision_function_tidl
 */
#define TIVX_KERNEL_TIDL_NAME          "com.ti.tidl"

/**
 *  \anchor Display_opMode
 *  \name   Display Kernel 's Operation Mode
 *  \brief  Display kernel can be run in two modes: it can be used to display
 *          only one buffer repeatedly or application provides a new buffer
 *          every VSYNC. In case of one buffer, kernel needs to maintain a local
 *          copy of the buffer.
 *
 *  \ingroup group_vision_function_display
 *  @{
 */
/** \brief Display Kernel does not need to maintain buffer copy i.e. application
 *   gives new frame at every VSYNC */
#define TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE            ((uint32_t) 0U)
/** \brief Display Kernel needs to maintain buffer copy */
#define TIVX_KERNEL_DISPLAY_BUFFER_COPY_MODE                 ((uint32_t) 1U)
/* @} */

/*! End of group_vision_function_hwa */



/*********************************
 *      CAPTURE STRUCTURES
 *********************************/
/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_CAPTURE kernel.
 *
 * \ingroup group_vision_function_capture
 */
typedef struct
{
    uint32_t enableCsiv2p0Support;  /*!< Flag indicating CSIV2P0 support */
    uint32_t numDataLanes;          /*!< Number of CSIRX data lanes */
    uint32_t dataLanesMap[4];       /*!< Data Lanes map array; note: size from CSIRX_CAPT_DATA_LANES_MAX */
} tivx_capture_params_t;

/*********************************
 *      DISPLAY STRUCTURES
 *********************************/
/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_DISPLAY kernel.
 *
 * \ingroup group_vision_function_display
 */
typedef struct {
    uint32_t  opMode;      /*!< Operation Mode of display kernel. Refer \ref Display_opMode for values */
    uint32_t  pipeId;      /*!< 0:VID1, 1:VIDL1, 2:VID2 and 3:VIDL2 */
    uint32_t  outWidth;    /*!< Horizontal Size of picture at display output */
    uint32_t  outHeight;   /*!< Vertical Size of picture at display output */
    uint32_t  posX;        /*!< X position of the video buffer */
    uint32_t  posY;        /*!< Y position of the video buffer */
} tivx_display_params_t;

/*********************************
 *      Function Prototypes
 *********************************/

/*!
 * \brief Used for the Application to load the hwa kernels into the context.
 * 
 * This includes Capture, Display, VPAC, and DMPAC kernels
 * 
 * \ingroup group_vision_function_hwa
 */
void tivxHwaLoadKernels(vx_context context);

/*!
 * \brief Used for the Application to unload the hwa kernels from the context.
 * 
 * This includes Capture, Display, VPAC, and DMPAC kernels
 * 
 * \ingroup group_vision_function_hwa
 */
void tivxHwaUnLoadKernels(vx_context context);

/*!
 * \brief Used for the Application to load the tidl kernels into the context.
 * \ingroup group_vision_function_hwa
 */
void tivxTIDLLoadKernels(vx_context context);

/*!
 * \brief Used for the Application to unload the tidl kernels from the context.
 * \ingroup group_vision_function_hwa
 */
void tivxTIDLUnLoadKernels(vx_context context);

/*!
 * \brief Function to register HWA Kernels on the display Target
 * \ingroup group_vision_function_display
 */
void tivxRegisterHwaTargetDisplayKernels(void);

/*!
 * \brief Function to un-register HWA Kernels on the display Target
 * \ingroup group_vision_function_display
 */
void tivxUnRegisterHwaTargetDisplayKernels(void);

/*!
 * \brief Function to register HWA Kernels on the capture Target
 * \ingroup group_vision_function_capture
 */
void tivxRegisterHwaTargetCaptureKernels(void);

/*!
 * \brief Function to un-register HWA Kernels on the capture Target
 * \ingroup group_vision_function_capture
 */
void tivxUnRegisterHwaTargetCaptureKernels(void);


/*!
 * \brief Function to register TIDL Kernels on the TIDL Target
 * \ingroup group_vision_function_tidl
 */
void tivxRegisterTIDLTargetKernels(void);

/*!
 * \brief Function to un-register TIDL Kernels on the TIDL Target
 * \ingroup group_vision_function_tidl
 */
void tivxUnRegisterTIDLTargetKernels(void);

#ifdef __cplusplus
}
#endif

#endif /* J7_KERNELS_H_ */


