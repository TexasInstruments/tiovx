/*
*
* Copyright (c) 2017-2025 Texas Instruments Incorporated
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

#include <stdbool.h>
#include <VX/vx.h>

/*! \brief Macros for build time check
 * \ingroup group_tivx_platform
 */
#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)
#define BUILD_ASSERT(e) \
     enum { ASSERT_CONCAT(assert_line_, __LINE__) = (1U/(e)) }

/*!
 * \file
 * \brief The Internal Debugging API
 */

/*! \brief These are the bit flags for debugging.
 * \ingroup group_vx_debug
 */
enum tivx_debug_zone_e {
    VX_ZONE_ERROR        = 0,    /*!< Used for error prints */
    VX_ZONE_WARNING      = 1,    /*!< Used for prints that report possible issues */
    VX_ZONE_INFO         = 2,    /*!< Used for debug processing/tracking prints */
    VX_ZONE_MAX          = 3     /*!< Used to indicate the upper bound of zone enumerations */
};

/*! \def VX_PRINT
 * \brief Utility macro to print debug information if specified zone is globally set
 * \ingroup group_vx_debug
 */
#define VX_PRINT(zone, message, ...) do { tivx_print_global(((vx_enum)zone), "[%s:%u] " \
    message, __FUNCTION__, __LINE__, ## __VA_ARGS__); } while (1 == 0)

/*! \def VX_PRINT_GRAPH
 * \brief Utility macro to print debug information if specified zone is set on a given graph object
 * \ingroup group_vx_debug
 */
#define VX_PRINT_GRAPH(zone, graph, message, ...) do { tivx_print_object(((vx_enum)zone), \
    tivxGetGraphDebugZonemask(graph), "[ %s ] " message, tivxGetGraphName(graph), ## __VA_ARGS__); } while (1 == 0)

/*! \def VX_PRINT_NODE
 * \brief Utility macro to print debug information if specified zone is set on a given node object
 * \ingroup group_vx_debug
 */
#define VX_PRINT_NODE(zone, node, message, ...) do { tivx_print_object(((vx_enum)zone), \
    tivxGetNodeDebugZonemask(node), "[ %s ] " message, tivxGetNodeName(node), ## __VA_ARGS__); } while (1 == 0)

/*! \def VX_PRINT_BOUND_ERROR
 * \brief Utility macro to print the standard boundary error for a specified configuration resource
 * \ingroup group_vx_debug
 */
#define VX_PRINT_BOUND_ERROR(resource) tivx_bound_error(__FUNCTION__, __LINE__, resource)

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Returns the name of an enumerated debug zone
 * \param [in] zone The debug zone from \ref tivx_debug_zone_e.
 * \ingroup group_vx_debug
 */
vx_char *tivx_find_zone_name(vx_enum zone);

/*! \brief Internal printing function for the global debug zone bitmask
 * \param [in] zone The debug zone from \ref tivx_debug_zone_e required to print the given message.
 * \param [in] format The format string to print.
 * \param [in] ... The variable list of arguments.
 * \ingroup group_vx_debug
 */
void tivx_print_global(vx_enum zone, const char *format, ...);

/*! \brief Internal printing function for a framework object with a set debug zone bitmask
 * \param [in] zone The debug zone from \ref tivx_debug_zone_e required to print the given message.
 * \param [in] debug_zonemask The debug zone bitmask of a framework object
 * \param [in] format The format string to print.
 * \param [in] ... The variable list of arguments.
 * \ingroup group_vx_debug
 */
void tivx_print_object(vx_enum zone, vx_uint32 debug_zonemask, const char *format, ...);

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
vx_bool tivx_is_zone_enabled(vx_enum zone);

/*! \brief Returns the debug zonemask value of a node
 * \param [in] node A given vx_node object.
 * \ingroup group_vx_debug
 */
vx_uint32 tivxGetNodeDebugZonemask(vx_node node);

/*! \brief Returns the debug zonemask value of a graph
 * \param [in] graph A given vx_graph object.
 * \ingroup group_vx_debug
 */
vx_uint32 tivxGetGraphDebugZonemask(vx_graph graph);

/*!
 * \brief Sets or clears a given debug zone for a graph
 *
 * \param [in] graph Graph reference
 * \param [in] debug_zone Given debug zone enumeration
 * \param [in] enable Flag to indicate if zone should be enabled or disabled
 * \ingroup group_vx_debug
 */
vx_status tivxSetGraphDebugZone(vx_graph graph, vx_uint32 debug_zone, vx_bool enable);

/*!
 * \brief Sets or clears a given debug zone for a node
 *
 * \param [in] node Node reference
 * \param [in] debug_zone Given debug zone enumeration
 * \param [in] enable Flag to indicate if zone should be enabled or disabled
 * \ingroup group_vx_debug
 */
vx_status tivxSetNodeDebugZone(vx_node node, vx_uint32 debug_zone, vx_bool enable);

/*! \brief Returns the name of a node object
 * \param [in] node A given vx_node object.
 * \ingroup group_vx_debug
 */
const char * tivxGetNodeName(vx_node node);

/*! \brief Returns the name of a graph object
 * \param [in] graph A given vx_graph object.
 * \ingroup group_vx_debug
 */
const char * tivxGetGraphName(vx_graph graph);

/*! \brief Internal function - prints the common boundary error statement for a given config resource
 *
 * \param [in] func Calling function name
 * \param [in] line Line number of calling function
 * \param [in] resource Name of specific configuration resource
 * \ingroup group_tivx_common_prints
 */
void tivx_bound_error(const char *func, uint32_t line, const char *resource);

#ifdef __cplusplus
}
#endif

#endif

