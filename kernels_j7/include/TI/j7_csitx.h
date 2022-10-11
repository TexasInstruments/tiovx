/*
 *
 * Copyright (c) 2020-2021 Texas Instruments Incorporated
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

#ifndef J7_CSITX_H_
#define J7_CSITX_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The CSITX kernels in this kernel extension.
 */

/*! \brief csitx kernel name
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_KERNEL_CSITX_NAME          "com.ti.csitx"



/*********************************
 *      CSITX Control Commands
 *********************************/

/*! \brief Control Command to print csitx statistics
 *         No argument is needed to query statistics.
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_PRINT_STATISTICS                    (0x40000000u)

/*! \brief Control Command to return csitx statistics to application
 *
 *  \ingroup group_vision_function_csitx
 *  This control command returns the status of the csitx node.
 *  Please refer to #tivx_csitx_statistics_t structure.
 */
#define TIVX_CSITX_GET_STATISTICS                      (0x40000001u)


/*********************************
 *      CSITX Defines
 *********************************/

/*! \brief Maximum number of channels supported in the csitx node.
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_MAX_CH                                 (4U)

/*! \brief Maximum number of instances supported in the csitx node.
 *
 *  \ingroup group_vision_function_csitx
 */
#if defined (SOC_J721S2) || defined (SOC_J784S4)
#define TIVX_CSITX_MAX_INST                               (2U)
#else
#define TIVX_CSITX_MAX_INST                               (1U)
#endif

/*! \brief Lane Band Speed: 80 Mbps to 100 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_80_TO_100_MBPS              ((uint32_t) 0x00U)
/*! \brief Lane Band Speed: 100 Mbps to 120 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_100_TO_120_MBPS             ((uint32_t) 0x01U)
/*! \brief Lane Band Speed: 120 Mbps to 160 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_120_TO_160_MBPS             ((uint32_t) 0x02U)
/*! \brief Lane Band Speed: 160 Mbps to 200 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_160_TO_200_MBPS             ((uint32_t) 0x03U)
/*! \brief Lane Band Speed: 200 Mbps to 240 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_200_TO_240_MBPS             ((uint32_t) 0x04U)
/*! \brief Lane Band Speed: 240 Mbps to 320 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_240_TO_320_MBPS             ((uint32_t) 0x05U)
/*! \brief Lane Band Speed: 320 Mbps to 390 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_320_TO_390_MBPS             ((uint32_t) 0x06U)
/*! \brief Lane Band Speed: 390 Mbps to 450 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_390_TO_450_MBPS             ((uint32_t) 0x07U)
/*! \brief Lane Band Speed: 450 Mbps to 510 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_450_TO_510_MBPS             ((uint32_t) 0x08U)
/*! \brief Lane Band Speed: 510 Mbps to 560 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_510_TO_560_MBPS             ((uint32_t) 0x09U)
/*! \brief Lane Band Speed: 560 Mbps to 640 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_560_TO_640_MBPS             ((uint32_t) 0x0AU)
/*! \brief Lane Band Speed: 640 Mbps to 690 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_640_TO_690_MBPS             ((uint32_t) 0x0BU)
/*! \brief Lane Band Speed: 690 Mbps to 770 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_690_TO_770_MBPS             ((uint32_t) 0x0CU)
/*! \brief Lane Band Speed: 770 Mbps to 870 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_770_TO_870_MBPS             ((uint32_t) 0x0DU)
/*! \brief Lane Band Speed: 870 Mbps to 950 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_870_TO_950_MBPS             ((uint32_t) 0x0EU)
/*! \brief Lane Band Speed: 950 Mbps to 1000 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_950_TO_1000_MBPS            ((uint32_t) 0x0FU)
/*! \brief Lane Band Speed: 1000 Mbps to 1200 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_1000_TO_1200_MBPS           ((uint32_t) 0x10U)
/*! \brief Lane Band Speed: 1200 Mbps to 1400 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_1200_TO_1400_MBPS           ((uint32_t) 0x11U)
/*! \brief Lane Band Speed: 1400 Mbps to 1600 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_1400_TO_1600_MBPS           ((uint32_t) 0x12U)
/*! \brief Lane Band Speed: 1600 Mbps to 1800 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_1600_TO_1800_MBPS           ((uint32_t) 0x13U)
/*! \brief Lane Band Speed: 1800 Mbps to 2000 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_1800_TO_2000_MBPS           ((uint32_t) 0x14U)
/*! \brief Lane Band Speed: 2000 Mbps to 2200 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_2000_TO_2200_MBPS           ((uint32_t) 0x15U)
/*! \brief Lane Band Speed: 2200 Mbps to 2500 Mbps
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_2200_TO_2500_MBPS           ((uint32_t) 0x16U)

/*! \brief Lane Band Speed: Reserved
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_BAND_SPEED_RESERVED                    ((uint32_t) 0x17U)

/*! \brief Lane Speed: Reserved
 *
 *  \ingroup group_vision_function_csitx
 */
#define TIVX_CSITX_LANE_SPEED_MBPS_RESERVED                    ((uint32_t) 0xFFFFFFFFU)

/*********************************
 *      CSITX STRUCTURES
 *********************************/
/*!
 * \brief The CSITX DRV instance configuration data structure used by the TIVX_KERNEL_CSITX kernel.
 *
 * \ingroup group_vision_function_csitx
 * Note: Application should at least provide valid value for 'laneBandSpeed' OR 'laneSpeedMbps'.
 *       If 'laneBandSpeed' is provided, then 'laneSpeedMbps' is optional and driver assign this to default speed from that band.
 *       If 'laneSpeedMbps' is provided, then 'laneBandSpeed' is ignored and driver will calculate this accordingly.
 */
typedef struct
{
    uint32_t rxCompEnable;          /*!< Flag indicating RX compatibility mode */
    uint32_t rxv1p3MapEnable;       /*!< Flag indicating RX V1P3 mapping */
    uint32_t numDataLanes;          /*!< Number of CSITX data lanes */
    uint32_t lanePolarityCtrl[5];   /*!< Data Lanes invert control array; note: size from (CSITX_TX_DATA_LANES_MAX + CSITX_TX_CLK_LANES_MAX) */
    uint32_t vBlank;                /*!< Vertical blanking in terms of number of line. */
    uint32_t hBlank;                /*!< Horizontal blanking in terms of number of pixels */
    uint32_t startDelayPeriod;      /*!< Delay in terms of micro-seconds before sending first line after enabling. Note: This is only applicable if chType is CSITX_CH_TYPE_COLORBAR. */
    uint32_t laneBandSpeed;         /*!< Data rates for lane band control. This parameter is ignored if 'laneSpeedMbps' is provided. */
    uint32_t laneSpeedMbps;         /*!< Exact data-rate for lane in Mbps. This parameter is optional if 'laneBandSpeed' is provided. */
} tivx_csitx_inst_params_t;

/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_CSITX kernel.
 *
 * \ingroup group_vision_function_csitx
 */
typedef struct
{
    tivx_csitx_inst_params_t instCfg[TIVX_CSITX_MAX_INST]; /*!< CSI2Tx Instance configuration */
    uint32_t instId[TIVX_CSITX_MAX_INST]; /*!< CSI2Tx Instance Id, 0:CSITx0 */
    uint32_t numInst; /*!< Number of instances in current csitx node */
    uint32_t numCh; /*!< Number of channels to be processed on current instance of Node */
    uint32_t chVcNum[TIVX_CSITX_MAX_CH]; /*!< Virtual Channel Number for each channel */
    uint32_t chInstMap[TIVX_CSITX_MAX_CH]; /*!< Instance ID for each channel */
} tivx_csitx_params_t;

/*!
 * \brief Csitx status structure used to get the current status.
 *
 * \ingroup group_vision_function_csitx
 */
typedef struct
{
    /*! Counter to keep track of how many requests are queued to the
        driver.
        Note: This counter will be reset at the time of driver init. */
    uint32_t queueCount[TIVX_CSITX_MAX_INST][TIVX_CSITX_MAX_CH];
    /*! Counter to keep track of how many requests are dequeued from the
        driver.
        Note: This counter will be reset at the time of driver init. */
    uint32_t dequeueCount[TIVX_CSITX_MAX_INST][TIVX_CSITX_MAX_CH];
    /*! Counter to keep track of how many frames are repeated from the
        driver when no buffers are queued by the application.
        Note: This counter will be reset at the time of driver init. */
    uint32_t frmRepeatCount[TIVX_CSITX_MAX_INST][TIVX_CSITX_MAX_CH];
    /*! Counter to keep track of the occurrence of overflow error.
        Note: This counter will be reset at the time of driver create and
        during driver start. */
    uint32_t overflowCount[TIVX_CSITX_MAX_INST];
} tivx_csitx_statistics_t;

/*********************************
 *      Function Prototypes
 *********************************/

/*!
 * \brief Function to register HWA Kernels on the csitx Target
 * \ingroup group_tivx_ext
 */
void tivxRegisterHwaTargetCsitxKernels(void);

/*!
 * \brief Function to un-register HWA Kernels on the csitx Target
 * \ingroup group_tivx_ext
 */
void tivxUnRegisterHwaTargetCsitxKernels(void);



/*! \brief [Graph] Creates a CSI Tx node.
 * \details The csitx node takes in a user data object of type <tt>\ref tivx_csitx_params_t</tt> to
            configure one or more csitx channels.The input must be of type \ref vx_object_array, which contain an array of
            either \ref vx_image or \ref tivx_raw_image data types.  \ref vx_image is used for transmitting processed
            image formats, while \ref tivx_raw_image is used for raw sensor outputs.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration The input user data object of a single csitx params structure of type <tt>\ref tivx_csitx_params_t</tt>.
 * \param [in] input Object array input which has been created from an exemplar of either \ref vx_image or \ref tivx_raw_image.
 *              If using \ref vx_image, the image type MUST be one of the following formats:
                <tt>\ref VX_DF_IMAGE_RGBX</tt>, <tt>\ref VX_DF_IMAGE_U16</tt>,
                <tt>\ref VX_DF_IMAGE_UYVY</tt> or <tt>\ref VX_DF_IMAGE_YUYV</tt>.
 * \see <tt>TIVX_KERNEL_CSITX_NAME</tt>
 * \ingroup group_vision_function_csitx
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxCsitxNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_object_array             input);


/*!
 * \brief Function to initialize Csitx Parameters
 *
 * \param prms  [in] Pointer to csitx params configuration structure
 *
 * \ingroup group_vision_function_csitx
 */
void tivx_csitx_params_init(tivx_csitx_params_t *prms);

#ifdef __cplusplus
}
#endif

#endif /* J7_CSITX_H_ */

