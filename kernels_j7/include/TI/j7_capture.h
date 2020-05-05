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

#ifndef J7_CAPTURE_H_
#define J7_CAPTURE_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The Capture kernels in this kernel extension.
 */

/*! \brief capture kernel name
 *  \ingroup group_vision_function_capture
 */
#define TIVX_KERNEL_CAPTURE_NAME          "com.ti.capture"


/*********************************
 *      Capture Control Commands
 *********************************/

/*! \brief Control Command to print capture statistics
 *         No argument is needed to query statistics.
 *
 *  \ingroup group_vision_function_capture
 */
#define TIVX_CAPTURE_PRINT_STATISTICS                    (0x40000000u)

/*! \brief Control Command to return capture statistics to application
 *
 *  \ingroup group_vision_function_capture
 *  This control command returns the status of the capture node.
 *  Please refer to #tivx_capture_statistics_t structure.
 */
#define TIVX_CAPTURE_GET_STATISTICS                      (0x40000001u)

/*! \brief Control Command to create additional frame object descriptors
 *         from a provided frame reference.  The data buffers for
 *         each new descriptor will point to the same data buffer as
 *         the provided frame reference.
 *
 *  \ingroup group_vision_function_capture
 */
#define TIVX_CAPTURE_GENERATE_FRAMES                     (0x40000002u)

/* None */

/*********************************
 *      Capture Defines
 *********************************/

/*! \brief Maximum number of channels supported in the capture node.
 *
 *  \ingroup group_vision_function_capture
 */
#define TIVX_CAPTURE_MAX_CH                                 (16U)

/*! \brief Maximum number of instances supported in the capture node.
 *
 *  \ingroup group_vision_function_capture
 */
#define TIVX_CAPTURE_MAX_INST                               (2U)

/*! \brief Maximum number of streams supported in a capture module.
 *
 *  \ingroup group_vision_function_capture
 */
#define TIVX_CAPTURE_MAX_STRM                               (4U)


/*********************************
 *      Capture STRUCTURES
 *********************************/
/*!
 * \brief The CSIRX DRV instance configuration data structure used by the TIVX_KERNEL_CAPTURE kernel.
 *
 * \ingroup group_vision_function_capture
 */
typedef struct
{
    uint32_t enableCsiv2p0Support;  /*!< Flag indicating CSIV2P0 support */
    uint32_t numDataLanes;          /*!< Number of CSIRX data lanes */
    uint32_t dataLanesMap[4];       /*!< Data Lanes map array; note: size from CSIRX_CAPT_DATA_LANES_MAX */
} tivx_capture_inst_params_t;

/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_CAPTURE kernel.
 *
 * \ingroup group_vision_function_capture
 */
typedef struct
{
    tivx_capture_inst_params_t instCfg[TIVX_CAPTURE_MAX_INST]; /*!< CSI2Rx Instance configuration */
    uint32_t instId[TIVX_CAPTURE_MAX_INST]; /*!< CSI2Rx Instance Id, 0:CSIRx0 1:CSIRx0 */
    uint32_t numInst; /*!< Number of instances in current capture node */
    uint32_t numCh; /*!< Number of channels to be processed on current instance of Node */
    uint32_t chVcNum[TIVX_CAPTURE_MAX_CH]; /*!< Virtual Channel Number for each channel */
    uint32_t chInstMap[TIVX_CAPTURE_MAX_CH]; /*!< Instance ID for each channel */
    uint32_t timeout; /*!< Total timeout (in ms) for all cameras.  If this timeout is exceeded, cameras that have not been received are dead */
    uint32_t timeoutInitial; /*!< Initial failure timeout (in ms) for first time the capture node fails.  Once this
                                  timeout has been exceeded, the "timeout" parameter will then be used. */
} tivx_capture_params_t;

/*!
 * \brief Capture status structure used to get the current status.
 *
 * \ingroup group_vision_function_capture
 */
typedef struct
{
    /*! Counter to keep track of how many requests are queued to the
        driver.
        Note: This counter will be reset at the time of driver init. */
    uint32_t queueCount[TIVX_CAPTURE_MAX_INST][TIVX_CAPTURE_MAX_CH];
    /*! Counter to keep track of how many requests are dequeued from the
        driver.
        Note: This counter will be reset at the time of driver init. */
    uint32_t dequeueCount[TIVX_CAPTURE_MAX_INST][TIVX_CAPTURE_MAX_CH];
    /*! Counter to keep track of how many frames are dropped from the
        driver when no buffers are queued by the application.
        Note: This counter will be reset at the time of driver init. */
    uint32_t dropCount[TIVX_CAPTURE_MAX_INST][TIVX_CAPTURE_MAX_CH];
    /*! Counter to keep track of the occurrence of overflow error.
        Note: This counter will be reset at the time of driver create and
        during driver start. */
    uint32_t overflowCount[TIVX_CAPTURE_MAX_INST];
    /*! Counter to keep track of the occurrences of spurious UDMA interrupts.
        Note: This counter will be reset at the time of driver create and
        during driver start. */
    uint32_t spuriousUdmaIntrCount[TIVX_CAPTURE_MAX_INST];
    /*! Counter to keep track of the occurrences of Front FIFO Overflow.
        Note: This counter will be reset at the time of driver create and
        during driver start. */
    uint32_t frontFIFOOvflCount[TIVX_CAPTURE_MAX_INST];
    /*! Counter to keep track of the occurrences of CRC errors.
        Note: This counter will be reset at the time of driver create and
        during driver start. */
    uint32_t crcCount[TIVX_CAPTURE_MAX_INST];
    /*! Counter to keep track of the occurrences of un-corrected ECC errors.
        Note: This counter will be reset at the time of driver create and
        during driver start. */
    uint32_t eccCount[TIVX_CAPTURE_MAX_INST];
    /*! Counter to keep track of the occurrences of corrected ECC errors.
        Note: This counter will be reset at the time of driver create and
        during driver start. */
    uint32_t correctedEccCount[TIVX_CAPTURE_MAX_INST];
    /*! Counter to keep track of the occurrences of Data ID errors.
        Note: This counter will be reset at the time of driver create and
        during driver start. */
    uint32_t dataIdErrorCount[TIVX_CAPTURE_MAX_INST];
    /*! Counter to keep track of the occurrences of Invalid accesses.
        Note: This counter will be reset at the time of driver create and
        during driver start. */
    uint32_t invalidAccessCount[TIVX_CAPTURE_MAX_INST];
    /*! Counter to keep track of the occurrences of reception of
        invalid short packet.
        Note: This counter will be reset at the time of driver create and
        during driver start. */
    uint32_t invalidSpCount[TIVX_CAPTURE_MAX_INST];
    /*! Counter to keep track of the occurrences of Front FIFO Overflow.
        Note: This counter will be reset at the time of driver create and
        during driver start. */
    uint32_t strmFIFOOvflCount[TIVX_CAPTURE_MAX_INST][TIVX_CAPTURE_MAX_STRM];
    /*! Indicates active channels.  Channel 0 is bit 0; channel N is bit N.
        If active, the associated bit will be set to 1.  If inactive, the bit
        is set to 0. */
    uint8_t activeChannelMask;
} tivx_capture_statistics_t;


/*********************************
 *      Function Prototypes
 *********************************/

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


/*! \brief [Graph] Creates a camera capture node.
 * \details The capture node takes in a user data object of type <tt>\ref tivx_capture_params_t</tt> to
            configure one or more capture channels. The output is of type \ref vx_object_array, which contain an array of
            either \ref vx_image or \ref tivx_raw_image data types.  \ref vx_image is used if the sensor outputs processed
            image formats, while \ref tivx_raw_image is used for raw sensor outputs.

            The number of items in the output object array corresponds to the number of syncronized, homogeneous
            camera sensors assigned to the capture node.  Each item in an object array must have the same
            dimensions and formats.  If multiple sensors are needed tot be configured at different rates, formats, or
            resolutions,  then separate capture nodes should be used and syncronized by the application code.

            \note The capture node does not program or control the sensors, but relies on the application (or custom nodes)
            to call into the sensor drivers.
 * \param [in] graph The reference to the graph.
 * \param [in] input The input user data object of a single capture params structure of type <tt>\ref tivx_capture_params_t</tt>.
 * \param [out] output Object array output which has been created from an exemplar of either \ref vx_image or \ref tivx_raw_image.
 *              If using \ref vx_image, the image type MUST be one of the following formats:
                <tt>\ref VX_DF_IMAGE_RGBX</tt>, <tt>\ref VX_DF_IMAGE_U16</tt>,
                <tt>\ref VX_DF_IMAGE_UYVY</tt>, <tt>\ref VX_DF_IMAGE_YUYV</tt>.
 * \see <tt>TIVX_KERNEL_CAPTURE_NAME</tt>
 * \ingroup group_vision_function_capture
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxCaptureNode(vx_graph graph,
                                      vx_user_data_object  input,
                                      vx_object_array      output);

/*!
 * \brief Function to initialize Capture Parameters
 *
 * \param prms  [in] Pointer to capture params configuration structure
 *
 * \ingroup group_vision_function_capture
 */
void tivx_capture_params_init(tivx_capture_params_t *prms);

/*!
 * \brief Function to send error frame to capture node
 *
 *        Provided ref must be of the same type and have same properties as
 *        an element of the capture node output object array
 *
 *        This API must be called after the vxVerifyGraph call and before the
 *        processing of the OpenVX graph
 *
 * \param node  [in] Capture node reference
 * \param ref   [in] Error frame to be replicated
 *
 * \ingroup group_vision_function_capture
 */
vx_status tivxCaptureAllocErrorFrames(vx_node node, vx_reference ref);

#ifdef __cplusplus
}
#endif

#endif /* J7_CAPTURE_H_ */

