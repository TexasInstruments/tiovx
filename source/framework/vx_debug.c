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

static void tivx_print(vx_enum zone, vx_uint32 debug_zonemask, const char *format, va_list ap);

static vx_uint32 g_debug_zonemask = 0;

#define STR2(x) {#x, (vx_enum)x}

struct vx_string_and_enum_e {
    vx_char name[20];
    vx_enum value;
};

static struct vx_string_and_enum_e g_debug_enumnames[] = {
    STR2(VX_ZONE_ERROR),
    STR2(VX_ZONE_WARNING),
    STR2(VX_ZONE_INFO),
    {"UNKNOWN", -1} /* if the zone is not found, this will be returned. */
};

vx_char *tivx_find_zone_name(vx_enum zone)
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
    if ( (0 <= zone) && (zone < (vx_enum)VX_ZONE_MAX) )
    {

        g_debug_zonemask |= ZONE_BIT((vx_uint32)zone);
        tivx_print_object((vx_enum)VX_ZONE_INFO, ZONE_BIT((vx_uint32)VX_ZONE_INFO), "Globally Enabled %s\n", tivx_find_zone_name(zone));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid debug zone specified\n");
    }
}

void tivx_clr_debug_zone(vx_enum zone)
{
    if ( (0 <= zone) && (zone < (vx_enum)VX_ZONE_MAX) )
    {
        g_debug_zonemask &= ~(ZONE_BIT((vx_uint32)zone));
        tivx_print_object((vx_enum)VX_ZONE_INFO, ZONE_BIT((vx_uint32)VX_ZONE_INFO), "Globally Disabled %s\n", tivx_find_zone_name(zone));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid debug zone specified\n");
    }
}

vx_uint32 ownGetGlobalZonemask(void)
{
    return g_debug_zonemask;
}

vx_bool tivx_is_zone_enabled(vx_enum zone)
{
    vx_bool zone_enabled;

    if ( (0 <= zone) && (zone < (vx_enum)VX_ZONE_MAX) )
    {
        zone_enabled = (((vx_uint32)g_debug_zonemask & ZONE_BIT((vx_uint32)zone)) != (vx_uint32)vx_false_e) ? (vx_bool)vx_true_e : (vx_bool)vx_false_e;
    }
    else
    {
        zone_enabled = (vx_bool)vx_false_e;
    }
    return zone_enabled;
}

static void tivx_print(vx_enum zone, vx_uint32 debug_zonemask, const char *format, va_list ap)
{
    if ((debug_zonemask & ZONE_BIT((vx_uint32)zone)) != 0U)
    {
        uint32_t size;
        char string[256];

        (void)snprintf(string, sizeof(string), "%s: ", tivx_find_zone_name(zone));
        size = (uint32_t)strlen(string);

        (void)vsnprintf(&string[size], sizeof(string)-size, format, ap);
        ownPlatformPrintf(string);
    }
}

void tivx_print_global(vx_enum zone, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    tivx_print(zone, g_debug_zonemask, format, args);
    va_end(args);
}

void tivx_print_object(vx_enum zone, vx_uint32 debug_zonemask, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    tivx_print(zone, debug_zonemask, format, args);
    va_end(args);
}
