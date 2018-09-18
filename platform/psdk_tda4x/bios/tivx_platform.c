/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

#include <xdc/runtime/Types.h>
#include <xdc/runtime/Timestamp.h>

#include <utils/console_io/include/app_log.h>

uint64_t tivxPlatformGetTimeInUsecs(void)
{
    Types_Timestamp64 bios_timestamp64;
    Types_FreqHz bios_freq;
    uint64_t cur_ts, freq;

    Timestamp_get64(&bios_timestamp64);
    Timestamp_getFreq(&bios_freq);

    cur_ts = ((uint64_t) bios_timestamp64.hi << 32) | bios_timestamp64.lo;
    freq = ((uint64_t) bios_freq.hi << 32) | bios_freq.lo;

    return (cur_ts*1000000u)/freq;
}

void tivxPlatformPrintf(const char *format)
{
    appLogPrintf(format);
}

void tivxPlatformActivate()
{

}

void tivxPlatformDeactivate()
{

}
