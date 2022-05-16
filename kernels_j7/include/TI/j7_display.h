/*
 *
 * Copyright (c) 2019-2021 Texas Instruments Incorporated
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

#ifndef J7_DISPLAY_H_
#define J7_DISPLAY_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The Display kernels in this kernel extension.
 */

/*! \brief Display Kernel Name
 *  \ingroup group_vision_function_display
 */
#define TIVX_KERNEL_DISPLAY_NAME       "com.ti.hwa.display"



/*********************************
 *      Display Control Commands
 *********************************/


/*! \brief Control Command to select the channel to be displayed.
 *
 *          The display node can only display one of the input channels.
 *          In case, input is obeject array, there could be frames from
 *          multiple channels. In this case, this control command is used
 *          to select the channels to be displayed.
 *          This cmd can be called at runtime, provided all channels are
 *          exactly same in format, size, storage format etc..
 *
 *         This control command uses pointer to structure
 *         tivx_display_select_channel_params_t as an input argument.
 *
 *  \ingroup group_vision_function_display
 */
#define TIVX_DISPLAY_SELECT_CHANNEL         (0x30000000u)

/*! \brief Control Command to set the Crop parameters.
 *
 *          The display node supports cropping the input
 *          image and then using scalar, zoom out/in effect could be
 *          achieved. Typically for zoom effect, output
 *          resolution ie scalar output resolution remains constant.
 *          Only input crop parameters are changed.
 *
 *         This control command uses pointer to structure
 *         tivx_display_crop_params_t as an input argument.
 *
 *  \ingroup group_vision_function_display
 */
#define TIVX_DISPLAY_SET_CROP_PARAMS        (0x30000001u)

/*********************************
 *      Display Defines
 *********************************/

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


/*********************************
 *      Capture STRUCTURES
 *********************************/

/*!
 * \brief The configuration data structure for setting crop parameters.
 *
 * \ingroup group_vision_function_display
 */
typedef struct
{
    uint32_t  startX; /*!< Horizontal start for cropping */
    uint32_t  startY; /*!< Vertical start for cropping */
    uint32_t  width;  /*!< Width of crop window */
    uint32_t  height; /*!< Height of crop window */
} tivx_display_crop_params_t;

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
    uint32_t  enableCropping; /*!< Flag to enable cropping */
    tivx_display_crop_params_t cropPrms; /*!< Crop parameters */
} tivx_display_params_t;

/*! \brief Channels parameters for selecting channel to be displayed.
 *         Passed as an argument to #TIVX_DISPLAY_SELECT_CHANNEL
 *         control command.
 *
 *  \ingroup group_vision_function_display
 */
typedef struct
{
    /*! Id of the active channel to be displayed */
    uint32_t                   active_channel_id;
} tivx_display_select_channel_params_t;


/*********************************
 *      Function Prototypes
 *********************************/

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


/*! \brief [Graph] Creates a DSS Display Node.
 *
 * \param [in] graph         The reference to the graph.
 * \param [in] configuration The input user data object of a single display params structure of type <tt>\ref tivx_display_params_t</tt>.
 * \param [in] image         The input image in one of the below formats:
 *                           <tt>\ref VX_DF_IMAGE_RGB</tt>,
 *                           <tt>\ref VX_DF_IMAGE_RGBX</tt>,
 *                           <tt>\ref VX_DF_IMAGE_BGRX</tt>,
 *                           <tt>\ref VX_DF_IMAGE_UYVY</tt>,
 *                           <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *                           <tt>\ref VX_DF_IMAGE_U16</tt>,
 *                           <tt>\ref VX_DF_IMAGE_U8</tt> or
 *                           <tt>\ref TIVX_DF_IMAGE_RGB565</tt>.
 *
 * \see <tt>TIVX_KERNEL_DISPLAY_NAME</tt>
 * \ingroup group_vision_function_display
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxDisplayNode(
                                            vx_graph graph,
                                            vx_user_data_object configuration,
                                            vx_image image);



#ifdef __cplusplus
}
#endif

#endif /* J7_DISPLAY_H_ */

