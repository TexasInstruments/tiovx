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

#ifndef J7_VPAC_VISS_H_
#define J7_VPAC_VISS_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#include <TI/j7_vpac_viss_fcp.h>

#ifdef VPAC3
#include <TI/j7_vpac_viss3.h>
#elif defined (VPAC3L)
#include <TI/j7_vpac_viss3l.h>
#else
#include <TI/j7_vpac_viss1.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The VISS kernels in this kernel extension.
 */

/*! \brief vpac_viss kernel name
 *  \ingroup group_vision_function_vpac_viss
 */
#define TIVX_KERNEL_VPAC_VISS_NAME     "com.ti.hwa.vpac_viss"

/*********************************
 *      VPAC_VISS Control Commands
 *********************************/

/*! \brief Control Command to set the DCC (Dynamic Camera Configuration)
 *         information to the given VISS Node.
 *
 *         Viss node gets the pointer to DCC buffer containing
 *         VISS configuration. It uses DCC parser to parse and
 *         map DCC parameters into VISS configuration and sets it
 *         in the driver.
 *
 *         Note that if the module is bypassed in tivx_vpac_viss_params_t
 *         during node creating, the parameter will not be set for
 *         this module, even through DCC profile has config for
 *         the same module.
 *
 *         User data object containing DCC buffer is passed
 *         as argument with this control command.
 *
 *  \ingroup group_vision_function_vpac_viss
 */
#define TIVX_VPAC_VISS_CMD_SET_DCC_PARAMS                   (0x30000000u)

/*********************************
 *      VPAC_VISS Defines
 *********************************/

/*! \brief Maximum H3A number of bytes in statistics data array
 *
 *   Allocating 24KB buffer which is large enough for 32x32 windows
 *
 *  \ingroup group_vision_function_vpac_viss
 */
#define TIVX_VPAC_VISS_MAX_H3A_STAT_NUMBYTES            (24576U)

/*! \brief The H3A output memory address alignment
 *
 *   The start address of the tivx_h3a_data_t::data buffer must be aligned
 *   to this value, so the tivx_h3a_data_t::resv field is used
 *   along with this value to properly pad the structure to enable this
 *   alignment.  \see tivx_h3a_data_t::resv
 *
 *  \ingroup group_vision_function_vpac_viss
 */
#define TIVX_VPAC_VISS_H3A_OUT_BUFF_ALIGN  (64U)

/*!
 * \defgroup group_vision_function_vpac_viss_h3a Enumerations
 * \brief Enumerations for configuration parameters in VPAC VISS structures
 * \ingroup group_vision_function_vpac_viss
 * @{*/

/*! \see tivx_vpac_viss_params_t::h3a_in and tivx_ae_awb_params_t::h3a_source_data and tivx_h3a_data_t::h3a_source_data */
#define TIVX_VPAC_VISS_H3A_IN_RAW0     (0U)
/*! \see tivx_vpac_viss_params_t::h3a_in and tivx_ae_awb_params_t::h3a_source_data and tivx_h3a_data_t::h3a_source_data */
#define TIVX_VPAC_VISS_H3A_IN_RAW1     (1U)
/*! \see tivx_vpac_viss_params_t::h3a_in and tivx_ae_awb_params_t::h3a_source_data and tivx_h3a_data_t::h3a_source_data */
#define TIVX_VPAC_VISS_H3A_IN_RAW2     (2U)
/*! \see tivx_vpac_viss_params_t::h3a_in and tivx_ae_awb_params_t::h3a_source_data and tivx_h3a_data_t::h3a_source_data */
#define TIVX_VPAC_VISS_H3A_IN_LSC      (3U)
#ifdef VPAC3L
/*! \see tivx_vpac_viss_params_t::h3a_in and tivx_ae_awb_params_t::h3a_source_data and tivx_h3a_data_t::h3a_source_data */
#define TIVX_VPAC_VISS_H3A_IN_PCID     (4U)
#endif

/*! \see tivx_vpac_viss_params_t::h3a_mode and tivx_h3a_data_t::aew_af_mode */
#define TIVX_VPAC_VISS_H3A_MODE_AEWB   (0U)
/*! \see tivx_vpac_viss_params_t::h3a_mode and tivx_h3a_data_t::aew_af_mode */
#define TIVX_VPAC_VISS_H3A_MODE_AF     (1U)

/*@}*/


/*********************************
 *      VPAC_VISS STRUCTURES
 *********************************/

/*!
 * \brief Data corresponding to results of 2A algorithm
 *
 *  \details This is the data structure used for the \a ae_awb_result parameter of
 *  \ref tivxVpacVissNode.  This is used by the VISS to index into the
 *  appropriate photospace in the DCC database.
 *
 * \ingroup group_vision_function_vpac_viss
 */
typedef struct {
    /*! Indicates the source data corresponding to this data:
     *  |          Enum                       |   Description                    |
     *  |:------------------------------------|:---------------------------------|
     *  | \ref TIVX_VPAC_VISS_H3A_IN_RAW0     | H3A Input is from RAW0           |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_RAW1     | H3A Input is from RAW1           |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_RAW2     | H3A Input is from RAW2           |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_LSC      | H3A Input is from LSC            |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_PCID     | H3A Input is from PCID           |
     *  \ref TIVX_VPAC_VISS_H3A_IN_PCID is applicable for only VPAC3L
     */
    uint32_t                    h3a_source_data;
    /*! Measured in micro seconds (us) */
    uint32_t                    exposure_time;
    /*! Analog Gain */
    uint32_t                    analog_gain;
    /*! Is AE output valid? */
    uint32_t                    ae_valid;
    /*! Is AE converged? */
    uint32_t                    ae_converged;
    /*! Digital Gain */
    uint32_t                    digital_gain;
    /*! White Balance Gains */
    uint32_t                    wb_gains[4];
    /*! White Balance Offsets */
    int32_t                     wb_offsets[4];
    /*! Color Temperature (K) */
    uint32_t                    color_temperature;
    /*! Is AWB output valid? */
    uint32_t                    awb_valid;
    /*! Is AWB converged? */
    uint32_t                    awb_converged;
} tivx_ae_awb_params_t;

/*!
 * \brief H3A AEW configuration data structure used by the TIVX_KERNEL_VISS kernel.
 *
 * \details This is included in the \ref tivx_h3a_data_t H3A output if the
 * AEWB mode of H3A was enabled.  This is included in case
 * the 2A algorithm needs to know the configuration that the VISS used for the
 * H3A AEWB engine.
 *
 * \ingroup group_vision_function_vpac_viss
 */
typedef struct
{
    /*! AEW Window Height */
    uint16_t                    aewwin1_WINH;
    /*! AEW Window Width */
    uint16_t                    aewwin1_WINW;
    /*! AEW Vertical Count */
    uint16_t                    aewwin1_WINVC;
    /*! AEW Horizontal Count */
    uint16_t                    aewwin1_WINHC;
    /*! AEW Subwindow Vertical Increment Value */
    uint16_t                    aewsubwin_AEWINCV;
    /*! AEW Subwindow Horizontal Increment Value */
    uint16_t                    aewsubwin_AEWINCH;
} tivx_h3a_aew_config;

/*!
 * \brief The \a h3a_output data structure used by the TIVX_KERNEL_VISS kernel.
 *
 * \details This is used for the \a h3a_output parameter of \ref tivxVpacVissNode
 *
 * \note Must be allocated at 64 byte boundary.
 *
 * \ingroup group_vision_function_vpac_viss
 */
typedef struct {
    /*! Indicates the contents of this buffer:
     *  |          Enum                       |   Description                    |
     *  |:------------------------------------|:---------------------------------|
     *  | \ref TIVX_VPAC_VISS_H3A_MODE_AEWB   | AEWB                             |
     *  | \ref TIVX_VPAC_VISS_H3A_MODE_AF     | AF                               |
     */
    uint32_t                    aew_af_mode;
    /*! Indicates the source data corresponding to this data:
     *  |          Enum                       |   Description                    |
     *  |:------------------------------------|:---------------------------------|
     *  | \ref TIVX_VPAC_VISS_H3A_IN_RAW0     | H3A Input is from RAW0           |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_RAW1     | H3A Input is from RAW1           |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_RAW2     | H3A Input is from RAW2           |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_LSC      | H3A Input is from LSC            |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_PCID     | H3A Input is from PCID           |
     *  \ref TIVX_VPAC_VISS_H3A_IN_PCID is applicable for only VPAC3L
     */
    uint32_t                    h3a_source_data;
    /*! If aew_af_mode == TIVX_VPAC_VISS_H3A_MODE_AEWB, this is the aew HW configuration used */
    tivx_h3a_aew_config         aew_config;
    /*! Identifier for cpu ID that the VISS node is running on.
     *  Currently used only for notifying AEWB node which cpu to send the update to based on these results
     *  when ae_awb_result from the graph is NULL. */
    uint32_t                    cpu_id;
    /*! Identifier for camera channel ID.
     *  Currently used only for notifying AEWB node which channel to update based on these results
     *  when ae_awb_result from the graph is NULL. */
    uint32_t                    channel_id;
    /*! Total used size of the data buffer in bytes */
    uint32_t                    size;
    /*! Reserved dummy field to make data to be 64 byte aligned */
    uint32_t                    resv[(TIVX_VPAC_VISS_H3A_OUT_BUFF_ALIGN-(sizeof(tivx_h3a_aew_config)+16U))/4U ];
    /*! Payload of the AEW or AF data */
    uint8_t                     data[TIVX_VPAC_VISS_MAX_H3A_STAT_NUMBYTES];
} tivx_h3a_data_t;

/*********************************
 *      Function Prototypes
 *********************************/

/*!
 * \brief Function to register HWA Kernels on the vpac_viss Target
 * \ingroup group_vision_function_vpac_viss
 */
void tivxRegisterHwaTargetVpacVissKernels(void);

/*!
 * \brief Function to un-register HWA Kernels on the vpac_viss Target
 * \ingroup group_vision_function_vpac_viss
 */
void tivxUnRegisterHwaTargetVpacVissKernels(void);

/*!
 * \brief Function to initialize VISS Parameters
 *
 * \param prms  [in] Pointer to VISS parameters
 *
 * \ingroup group_vision_function_vpac_viss
 */
void tivx_vpac_viss_params_init(tivx_vpac_viss_params_t *prms);

/*!
 * \brief Function to initialize H3A data Parameters
 *
 * \param prms  [in] Pointer to H3A data parameters
 *
 * \ingroup group_vision_function_vpac_viss
 */
void tivx_h3a_data_init(tivx_h3a_data_t *prms);

/*!
 * \brief Function to initialize AEWB Output Parameters
 *        These parameters come from the AEWB algorithm.
 *
 * \param prms  [in] Pointer to AEWB output parameters
 *
 * \ingroup group_vision_function_vpac_viss
 */
void tivx_ae_awb_params_init(tivx_ae_awb_params_t *prms);

/*!
 * \brief Function to initialize H3A aew Config
 *
 * \param prms  [in] Pointer to H3A aew config
 *
 * \ingroup group_vision_function_vpac_viss
 */
void tivx_h3a_aew_config_init(tivx_h3a_aew_config *prms);


#ifdef __cplusplus
}
#endif

#endif /* J7_VPAC_VISS_H_ */

