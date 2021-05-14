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



#ifndef TIVX_DEBUG_H
#define TIVX_DEBUG_H

/*!
 * \file
 * \brief The Internal Debugging API
 */

/*! \brief These are the bit flags for debugging.
 * \ingroup group_vx_debug
 */
enum tivx_debug_zone_e {
    VX_ZONE_ERROR        = 0,    /*!< Used for most errors */
    VX_ZONE_WARNING      = 1,    /*!< Used to warning developers of possible issues */
    VX_ZONE_API          = 2,    /*!< Used to trace API calls and return values */
    VX_ZONE_INFO         = 3,    /*!< Used to show run-time processing debug */

    VX_ZONE_PERF         = 4,    /*!< Used to show performance information */
    VX_ZONE_CONTEXT      = 5,    /*!< Used to show information related to context objects */
    VX_ZONE_OSAL         = 6,    /*!< Used to show information related to the OS abstraction layer */
    VX_ZONE_REFERENCE    = 7,    /*!< Used to show information related to reference objects */

    VX_ZONE_ARRAY        = 8,    /*!< Used to show information related to array objects */
    VX_ZONE_IMAGE        = 9,    /*!< Used to show information related to image objects */
    VX_ZONE_SCALAR       = 10,   /*!< Used to show information related to scalar objects */
    VX_ZONE_KERNEL       = 11,   /*!< Used to show information related to kernel objects */

    VX_ZONE_GRAPH        = 12,   /*!< Used to show information related to graph objects */
    VX_ZONE_NODE         = 13,   /*!< Used to show information related to node objects */
    VX_ZONE_PARAMETER    = 14,   /*!< Used to show information related to parameter objects */
    VX_ZONE_DELAY        = 15,   /*!< Used to show information related to delay objects */

    VX_ZONE_TARGET       = 16,   /*!< Used to show information related to OpenVX targets */
    VX_ZONE_LOG          = 17,   /*!< Used to show generic logging information */

    VX_ZONE_INIT         = 18,   /*!< Used to show initialization logs */

    VX_ZONE_OPTIMIZATION = 19,   /*!< Used to provide optimization tips */

    VX_ZONE_MAX          = 32
};

#define VX_PRINT(zone, message, ...) do { tivx_print(((vx_enum)zone), "[%s:%u] " message, __FUNCTION__, __LINE__, ## __VA_ARGS__); } while (1 == 0)

/*! \def VX_PRINT
 * \brief The OpenVX Debugging Facility.
 * \ingroup group_vx_debug
 */


#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Internal Printing Function.
 * \param [in] zone The debug zone from \ref tivx_debug_zone_e.
 * \param [in] format The format string to print.
 * \param [in] ... The variable list of arguments.
 * \ingroup group_vx_debug
 */
void tivx_print(vx_enum zone, const char *format, ...);

/*! \brief Sets a zone bit in the debug mask
 * \param [in] zone The debug zone from \ref tivx_debug_zone_e.
 * \ingroup group_vx_debug
 */
void tivx_set_debug_zone(vx_enum zone);

/*! \brief Clears the zone bit in the mask.
 * \param [in] zone The debug zone from \ref tivx_debug_zone_e.
 * \ingroup group_vx_debug
 */
void tivx_clr_debug_zone(vx_enum zone);

/*! \brief Returns true or false if the zone bit is set or cleared.
 * \param [in] zone The debug zone from \ref tivx_debug_zone_e.
 * \ingroup group_vx_debug
 */
vx_bool tivx_get_debug_zone(vx_enum zone);

#ifdef __cplusplus
}
#endif

#endif

