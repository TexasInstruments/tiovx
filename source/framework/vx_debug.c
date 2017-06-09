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


#include <vx_internal.h>

static vx_char *find_zone_name(vx_enum zone);

static vx_uint32 g_debug_zonemask = 0;

#ifdef ZONE_BIT
#undef  ZONE_BIT
#endif

#define ZONE_BIT(zone)  (1 << (zone))

#define _STR2(x) {#x, x}

struct vx_string_and_enum_e {
    vx_char name[20];
    vx_enum value;
};

static struct vx_string_and_enum_e g_debug_enumnames[] = {
    _STR2(VX_ZONE_ERROR),
    _STR2(VX_ZONE_WARNING),
    _STR2(VX_ZONE_API),
    _STR2(VX_ZONE_INFO),
    _STR2(VX_ZONE_PERF),
    _STR2(VX_ZONE_CONTEXT),
    _STR2(VX_ZONE_OSAL),
    _STR2(VX_ZONE_REFERENCE),
    _STR2(VX_ZONE_ARRAY),
    _STR2(VX_ZONE_IMAGE),
    _STR2(VX_ZONE_SCALAR),
    _STR2(VX_ZONE_KERNEL),
    _STR2(VX_ZONE_GRAPH),
    _STR2(VX_ZONE_NODE),
    _STR2(VX_ZONE_PARAMETER),
    _STR2(VX_ZONE_DELAY),
    _STR2(VX_ZONE_TARGET),
    _STR2(VX_ZONE_LOG),
    _STR2(VX_ZONE_INIT),
    {"UNKNOWN", -1} /* if the zone is not found, this will be returned. */
};

static vx_char *find_zone_name(vx_enum zone)
{
    vx_uint32 i;

    for (i = 0; i < (dimof(g_debug_enumnames) - 1u); i++)
    {
        if (g_debug_enumnames[i].value == zone)
        {
            break;
        }
    }

    return g_debug_enumnames[i].name;
}

void tivx_set_debug_zone(vx_enum zone)
{
    if ( (0 <= zone) && (zone < VX_ZONE_MAX) ) {
        g_debug_zonemask |= ZONE_BIT(zone);
        tivx_print(zone, "Enabled\n");
    }
}

void tivx_clr_debug_zone(vx_enum zone)
{
    if ( (0 <= zone) && (zone < VX_ZONE_MAX) ) {
        tivx_print(zone, "Disabled\n");
        g_debug_zonemask &= ~(ZONE_BIT(zone));
    }
}

vx_bool tivx_get_debug_zone(vx_enum zone)
{
    vx_bool zone_enabled;

    if ( (0 <= zone) && (zone < VX_ZONE_MAX) )
    {
        zone_enabled = ((g_debug_zonemask & zone)?vx_true_e:vx_false_e);
    }
    else
    {
        zone_enabled = vx_false_e;
    }
    return zone_enabled;
}

void tivx_print(vx_enum zone, char *format, ...)
{
    if (g_debug_zonemask & ZONE_BIT(zone))
    {
        uint32_t size;
        char string[1024];
        va_list ap;

        va_start(ap, format);

        snprintf(string, sizeof(string), " %s:", find_zone_name(zone));
        size = strlen(string);
        vsnprintf(&string[size], sizeof(string)-size, format, ap);
        tivxPlatformPrintf(string);
        va_end(ap);
    }
}




