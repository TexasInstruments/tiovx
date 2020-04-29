/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
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
#if !defined(_TIVX_UTILS_H_)
#define _TIVX_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <VX/vx.h>
#include <TI/tivx_debug.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TIOVX_UTILS_MAXPATHLENGTH   (512u)

/**
 * \brief Returns the absolute path defined by the environment variable VX_TEST_DATA_PATH.
 *
 * \return A string if VX_TEST_DATA_PATH is defined. NULL, otherwise.
 */
const char *tivx_utils_get_test_file_dir();

/**
 * \brief Conditionally returns the absolute path of the 'inFilePath'.
 *
 * \param inFilePath [in] inFilePath
 *
 * \param outFilePath [out] This shall contain the path to the inFilePath as follows:
 *                      - If the 'inFilePath' contains reference to environment
 *                        variable then it shall be expanded and an absolute
 *                        file path is returned. If the environment variable
 *                        is not defined then VX_FAILURE is returned. The
 *                        content of 'outFilePath' will be undefined.
 *                      - Otherwise, the 'inFilePath' is copied to 'outFilePath' as
 *                        is.
 *                        ex:-
 *                        1)- ${ENV_NAME} maps to /home/someone/test_dir
 *                            inFilePath = "${ENV_NAME}/colors.bmp"
 *                            outFilePath = "/home/someone/test_dir/colors.bmp"
 *                            return VX_SUCCESS
 *                        2)- ${ENV_NAME} not defined
 *                            inFilePath = "${ENV_NAME}/colors.bmp"
 *                            outFilePath = not defined
 *                            return VX_FAILURE
 *                        2)- inFilePath = "/some/dir/name/colors.bmp"
 *                            outFilePath = "/some/dir/name/colors.bmp"
 *                            return VX_SUCCESS
 *                        NOTE: The length of 'outFilePath' will be truncated to
 *                              TIOVX_UTILS_MAXPATHLENGTH-1;
 *
 * \return  - VX_SUCCESS and the content of 'outFilePath' will be valid
 *          - VX_FAILURE and the content of 'outFilePath' will be undefined
 */
vx_status tivx_utils_expand_file_path(const char *inFilePath, char *outFilePath);

#ifdef __cplusplus
}
#endif

#endif /* _TIVX_UTILS_H_ */

