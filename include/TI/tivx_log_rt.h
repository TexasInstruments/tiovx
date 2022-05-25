/*
*
* Copyright (c) 2017 - 2019 Texas Instruments Incorporated
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




#ifndef TIVX_LOG_RT_H_
#define TIVX_LOG_RT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to TI run-time logging APIs
 */


/*!
 * \brief Enable run-time logging of graph trace
 *
 *  - By default run-time logging is disabled.
 *  - This API MUST be called after vxVerifyGraph and before starting the graph execution
 *
 * \param [in] graph Graph reference
 *
 * \ingroup group_tivx_log_rt_trace_host
 */
vx_status VX_API_CALL tivxLogRtTraceEnable(vx_graph graph);

/*!
 * \brief Sisable run-time logging of graph trace
 *
 *  - By default run-time logging is disabled.
 *  - This API MUST be called after graph executions have stopped and before calling vxReleaseGraph
 *
 * \param [in] graph Graph reference
 *
 * \ingroup group_tivx_log_rt_trace_host
 */
vx_status VX_API_CALL tivxLogRtTraceDisable(vx_graph graph);

/*!
 * \brief Saved current log events to file and clear the event log memory
 *
 * - This API can be called while graphs are executing
 * - Each time API is called, current log events are saved to the file 'filename'
 *   and existing log is cleared.
 * - Calling this API periodically avoids the event log from getting full.
 * - Thus this API allows to continously store data to a file on the filesystem, limited only by the
 *   size of free space on the filesystem.
 * - The data stored in the file can be visualized by doing below
 * - Copy the saved file(s) to PC
 * - Convert the binary data to 'VCD - Value Change Dump' format
 * - Use a opensource tool like gtkwave to visualize the data
 *
 * \param [in] filename Filename to save the log data into. When filename is NULL, data is not saved, only the event log is cleared.
 *
 * \pre \ref tivxLogRtTraceEnable
 *
 * \ingroup group_tivx_log_rt_trace_host
 */
vx_status VX_API_CALL tivxLogRtTraceExportToFile(char *filename);


#ifdef __cplusplus
}
#endif

#endif

