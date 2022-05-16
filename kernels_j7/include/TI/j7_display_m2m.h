/*
 *
 * Copyright (c) 2021 Texas Instruments Incorporated
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

#ifndef J7_DISPLAY_M2M_H_
#define J7_DISPLAY_M2M_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The Display M2M kernels in this kernel extension.
 */

/*! \brief Display M2M kernel name
 *  \ingroup group_vision_function_display_m2m
 */
#define TIVX_KERNEL_DISPLAY_M2M_NAME          "com.ti.hwa.displaym2m"

/*********************************
 *      Display M2M Control Commands
 *********************************/
/*! \brief Control Command to return Display M2M statistics to application
 *
 *  \ingroup group_vision_function_display_m2m
 *  This control command returns the status of the Display M2M node.
 *  Please refer to #tivx_display_m2m_statistics_t structure.
 */
#define TIVX_DISPLAY_M2M_GET_STATISTICS                      (0x40000001u)

/*********************************
 *      Display M2M Defines
 *********************************/
/*! \brief Maximum number of display pipe-line supported in the DSS M2M node.
 *         Multiple number of pipe-lines are needed to support blending.
 *
 *  \ingroup group_vision_function_display_m2m
 */
#define TIVX_DISPLAY_M2M_MAX_PIPE                     (1U)

/*********************************
 *      Display M2M STRUCTURES
 *********************************/
/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_DISPLAY_M2M kernel.
 *
 * \ingroup group_vision_function_display_m2m
 */
typedef struct {
    /*! Write-back pipeline to use. IDs=> 0: Write-back pipe-line1 */
    uint32_t instId;
    /*! Number of pipe-lines to be used,
        should be set to '1' as blending is not supported currently */
    uint32_t numPipe;
    /*! IDs of pipe-lines to be used,
        IDs=> 0:VID1, 1:VIDL1, 2:VID2 */
    uint32_t pipeId[TIVX_DISPLAY_M2M_MAX_PIPE];
    /*! Overlay to be used. IDs=> 0:Overlay1 1:Overlay2 2:Overlay3 3:Overlay4 */
    uint32_t overlayId;
} tivx_display_m2m_params_t;

/*!
 * \brief Display M2M status structure used to get the current status.
 *
 * \ingroup group_vision_function_display_m2m
 */
typedef struct
{
    /*! Counter to keep track of how many requests are queued to the
        driver.
        Note: This counter will be reset at the time of driver init. */
    uint32_t queueCount;
    /*! Counter to keep track of how many requests are dequeued from the
        driver.
        Note: This counter will be reset at the time of driver init. */
    uint32_t dequeueCount;
    /*! Counter to keep track of how many requests are wrote back by the
        driver.
        Note: This counter will be reset at the time of driver init. */
    uint32_t wbFrmCount;
    /*! Counter to keep track of the occurrence of underflow error.
        Note: This counter will be reset at the time of driver create and
        during driver start. */
    uint32_t underflowCount;
} tivx_display_m2m_statistics_t;

/*********************************
 *      Function Prototypes
 *********************************/

/*!
 * \brief Function to register HWA Kernels on the display_m2m Target
 * \ingroup group_tivx_ext
 */
void tivxRegisterHwaTargetDisplayM2MKernels(void);

/*!
 * \brief Function to un-register HWA Kernels on the display_m2m Target
 * \ingroup group_tivx_ext
 */
void tivxUnRegisterHwaTargetDisplayM2MKernels(void);

/*! \brief [Graph] Creates a DISPLAY_M2M Node.
 * This node can be used to do color space conversion like YUV422 to YUV420 OR
 * RGB888 to YUV420 and vice versa. This node operates in Memory to Memory mode.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration
 * \param [in] input
 * \param [out] output
 * \see <tt>TIVX_KERNEL_DISPLAY_M2M_NAME</tt>
 * \ingroup group_vision_function_display_m2m
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxDisplayM2MNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input,
                                      vx_image             output);

/*!
 * \brief Function to initialize Display M2M Parameters
 *
 * \param prms  [in] Pointer to display M2M params configuration structure
 *
 * \ingroup group_vision_function_display_m2m
 */
void tivx_display_m2m_params_init(tivx_display_m2m_params_t *prms);


#ifdef __cplusplus
}
#endif

#endif /* J7_DISPLAY_M2M_H_ */

