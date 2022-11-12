/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <vx_internal.h>

static vx_char *find_zone_name(vx_enum zone);

static vx_uint32 g_debug_zonemask = 0;

#ifdef ZONE_BIT
#undef  ZONE_BIT
#endif

#define ZONE_BIT(zone)  ((vx_uint32)1U << (zone))

#define _STR2(x) {#x, (vx_enum)x}

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
    if ( (0 <= zone) && (zone < (vx_enum)VX_ZONE_MAX) ) {
        g_debug_zonemask |= ZONE_BIT((vx_uint32)zone);
        tivx_print(zone, "Enabled\n");
    }
}

void tivx_clr_debug_zone(vx_enum zone)
{
    if ( (0 <= zone) && (zone < (vx_enum)VX_ZONE_MAX) ) {
        tivx_print(zone, "Disabled\n");
        g_debug_zonemask &= ~(ZONE_BIT((vx_uint32)zone));
    }
}

vx_bool tivx_get_debug_zone(vx_enum zone)
{
    vx_bool zone_enabled;

    if ( (0 <= zone) && (zone < (vx_enum)VX_ZONE_MAX) )
    {
        zone_enabled = (((vx_uint32)g_debug_zonemask & (vx_uint32)zone) != (vx_uint32)vx_false_e) ? (vx_bool)vx_true_e : (vx_bool)vx_false_e;
    }
    else
    {
        zone_enabled = (vx_bool)vx_false_e;
    }
    return zone_enabled;
}

void tivx_print(vx_enum zone, const char *format, ...)
{
    if ((g_debug_zonemask & ZONE_BIT((vx_uint32)zone)) != 0U)
    {
        uint32_t size;
        char string[1024];
        va_list ap;

        va_start(ap, format);

        snprintf(string, sizeof(string), " %s:", find_zone_name(zone));
        size = (uint32_t)strlen(string);
        vsnprintf(&string[size], sizeof(string)-size, format, ap);
        ownPlatformPrintf(string);
        va_end(ap);
    }
}




