/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */
/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef OPENVX_INT_DEBUG_H_
#define OPENVX_INT_DEBUG_H_

/*!
 * \file
 * \brief The Internal Debugging API
 */

/*! \brief These are the bit flags for debugging.
 * \ingroup group_vx_debug
 */
enum tivx_debug_zone_e {
    VX_ZONE_ERROR       = 0,    /*!< Used for most errors */
    VX_ZONE_WARNING     = 1,    /*!< Used to warning developers of possible issues */
    VX_ZONE_API         = 2,    /*!< Used to trace API calls and return values */
    VX_ZONE_INFO        = 3,    /*!< Used to show run-time processing debug */

    VX_ZONE_PERF        = 4,    /*!< Used to show performance information */
    VX_ZONE_CONTEXT     = 5,
    VX_ZONE_OSAL        = 6,
    VX_ZONE_REFERENCE   = 7,

    VX_ZONE_ARRAY       = 8,
    VX_ZONE_IMAGE       = 9,
    VX_ZONE_SCALAR      = 10,
    VX_ZONE_KERNEL      = 11,

    VX_ZONE_GRAPH       = 12,
    VX_ZONE_NODE        = 13,
    VX_ZONE_PARAMETER   = 14,
    VX_ZONE_DELAY       = 15,

    VX_ZONE_TARGET      = 16,
    VX_ZONE_LOG         = 17,

    VX_ZONE_INIT        = 18,

    VX_ZONE_MAX         = 32
};

#define VX_PRINT(zone, message, ...) do { tivx_print((zone), "[%s:%u] "message, __FUNCTION__, __LINE__, ## __VA_ARGS__); } while (0)

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
void tivx_print(vx_enum zone, char *format, ...);

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

