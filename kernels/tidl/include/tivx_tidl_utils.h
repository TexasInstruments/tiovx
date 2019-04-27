/*
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
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

#include <TI/tivx.h>
#include <stdint.h>
#include <assert.h>

#define VX_TIDL_UTILS_NO_ZERO_COEFF_PERCENT       (100)
#define VX_TIDL_UTILS_RANDOM_INPUT                (0)

/**
 * \file tivx_tidl_utils.h. Utility APIs used to read network files for TI-DL.
 */

/**
 *******************************************************************************
 *
 * \brief Function vx_tutorial_tidl_readNetwork() read the network model from a file
 * \return  user data object corresponding to the network
 *
 *******************************************************************************
 */
vx_user_data_object vx_tidl_utils_readNetwork(vx_context context, char *network_file);

/**
 *******************************************************************************
 *
 * \brief Function vx_tidl_utils_countLayersGroup() returns the number of groups of layers in the network
 *                 and fill the array layersGroupCount with the number of layers in each group.
 *                 layersGrouCount[0] always contains 0.
 *                 layersGrouCount[1] contains the number of layers associated to group 1
 *                 layersGrouCount[2] contains the number of layers associated to group 2
 *                 Each group of layers is processed by a different core.
 * \return  number of groups of layers.
 *
 *******************************************************************************
 */
int32_t vx_tidl_utils_countLayersGroup(vx_user_data_object  network, int32_t layersGroupCount[TIVX_CPU_ID_MAX]);

/**
 *******************************************************************************
 *
 * \brief Function vx_tidl_utils_updateLayersGroup() change each layer's group id so it runs on the cpu specified by target_cpu
 * \return  status
 *
 *******************************************************************************
 */
vx_status vx_tidl_utils_updateLayersGroup(vx_user_data_object  network, vx_enum target_cpu);

/**
 *******************************************************************************
 *
 * \brief Function vx_tidl_utils_getConfig() extracts the input/output buffer configuration from the network
 * \return  user data object corresponding to the I/O buffer configuration
 *
 *******************************************************************************
 */
vx_user_data_object vx_tidl_utils_getConfig(vx_context context, vx_user_data_object  network, uint32_t *num_input_tensors, uint32_t *num_output_tensors, vx_enum target_cpu);


/**
 *******************************************************************************
 *
 * \brief Function vx_tidl_utils_readParams() fills each layer's parameters buffer by reading the values from a file.
 * \return  vx_status
 *
 *******************************************************************************
 */
vx_status vx_tidl_utils_readParams(vx_user_data_object  network, char *params_file);

/**
 *******************************************************************************
 *
 * \brief Function vx_tidl_utils_setCreateParams() creates, intializes, returns a user data object that maps to the
 *        underlying structure TIDL_CreateParams . This data object will be one of the 5 input parameter to the function
 *        tivxTIDLNode().
 *        vx_tidl_utils_setCreateParams() function accepts 3 input parameters quantHistoryBoot,
 *        quantHistory, quantMargin that are used to initialize the members quantHistoryParam1, quantHistoryParam2, quantMargin of the structure
 *        TIDL_CreateParams.
 *        TIDL maintains range statistics for previously processed frames. It quantizes the current inference activations
 *        using range statistics from history for processes (weighted average range).
 *        Below is the parameters controls quantization.
 *        - quantMargin is margin added to the average in percentage.
 *        - quantHistoryBoot weights used for previously processed inference during application boot time for initial few frames
 *        - quantHistory weights used for previously processed inference during application execution (After initial few frames)
 *
 *        Below settings are adequate for running on videos sequences.
 *          quantHistoryBoot= 20;
 *          quantHistory= 5;
 *          quantMargin= 0;
 *        For still images, set all settings to 0.
 *
 * \return  user data object corresponding to the TIDL_CreateParams object.
 *
 *******************************************************************************
 */
vx_user_data_object vx_tidl_utils_setCreateParams(vx_context context, int32_t quantHistoryBoot, int32_t quantHistory, int32_t quantMargin);

/**
 *******************************************************************************
 *
 * \brief Function vx_tidl_utils_setInArgs() creates, intializes, returns a user data object that maps to the
 *        underlying structure TIDL_InArgs . This data object will be one of the 5 input parameter to the function
 *        tivxTIDLNode().
 *        The current implementation of vx_tidl_utils_setInArgs() does not initialize any useful members if TIDL_InArgs
 *        beyond some book-keeping parameters. Future versions might extend to initializing more useful parameters.
 *
 *******************************************************************************
 */
vx_user_data_object vx_tidl_utils_setInArgs(vx_context context);

/**
 *******************************************************************************
 *
 * \brief Function vx_tidl_utils_setOutArgs() creates, intializes, returns a user data object that maps to the
 *        underlying structure TIDL_outArgs . This data object will be one of the 5 input parameter to the function
 *        tivxTIDLNode().
 *        The current implementation of vx_tidl_utils_setOutArgs() does not initialize any useful members if TIDL_InArgs
 *        beyond some book-keeping parameters. Future versions might extend to initializing more useful parameters.
 *
 *******************************************************************************
 */
vx_user_data_object vx_tidl_utils_setOutArgs(vx_context context);

