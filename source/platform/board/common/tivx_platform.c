/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <tivx_target_config.h>
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>

const char *tivxPlatformGetEnv(const char *env_var);

uint64_t tivxPlatformGetTimeInUsecs(void)
{
    return appLogGetTimeInUsec();
}

void ownPlatformPrintf(const char *format)
{
    appLogPrintf(format);
}

void ownPlatformActivate(void)
{

}

void ownPlatformDeactivate(void)
{

}

#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_HOST_ONLY_PLATFORM_UM001
<justification end> */
/* TIOVX-1766- Host only Id: TIOVX_CODE_COVERAGE_HOST_ONLY_PLATFORM_UM001 */
#endif
const char *tivxPlatformGetEnv(const char *env_var)
{
    const char *value=" ";

    if(strcmp(env_var, "VX_TEST_DATA_PATH")==0)
    {
        value="/test_data/";
    }
    return (value);
}
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_END */
#endif
