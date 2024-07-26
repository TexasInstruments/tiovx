/*
 *******************************************************************************
 *
 * Copyright (C) 2018-2023 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <vx_internal.h>

#include <utils/rtos/include/app_rtos.h>

vx_status tivxEventCreate(tivx_event *event)
{
    vx_status status = (vx_status)VX_FAILURE;
    app_rtos_semaphore_handle_t handle;
    app_rtos_semaphore_params_t semParams;

    if (NULL != event)
    {
        /* Default parameter initialization */
        appRtosSemaphoreParamsInit(&semParams);

        semParams.mode = APP_RTOS_SEMAPHORE_MODE_BINARY;
        semParams.initValue = 0U;

        handle = appRtosSemaphoreCreate(semParams);

        if (NULL == handle)
        {
            VX_PRINT(VX_ZONE_ERROR, "Semaphore creation failed\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            *event = (tivx_event)handle;
            status = (vx_status)VX_SUCCESS;
        }
    }

    return (status);
}

vx_status tivxEventDelete(tivx_event *event)
{
    vx_status status = (vx_status)VX_SUCCESS;
    app_rtos_status_t ret_status = (app_rtos_status_t)APP_RTOS_STATUS_SUCCESS;
    app_rtos_semaphore_handle_t handle;

    if ((NULL != event) && (*event != NULL))
    {
        handle = (app_rtos_semaphore_handle_t)*event;
        ret_status = appRtosSemaphoreDelete(&handle);
        if ((app_rtos_status_t)APP_RTOS_STATUS_SUCCESS == ret_status)
        {
            *event = NULL;
        }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1760- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_RTOS_EVENT_UM001 */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Semaphore delete returned an error\n");
            status = (vx_status)VX_FAILURE;
        }
#endif
    }
    else
    {
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

vx_status tivxEventPost(tivx_event event)
{
    vx_status status = (vx_status)VX_SUCCESS;
    app_rtos_status_t ret_status = APP_RTOS_STATUS_SUCCESS;
    if (NULL != event)
    {
        ret_status = appRtosSemaphorePost((app_rtos_semaphore_handle_t)event);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1760- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_RTOS_EVENT_UM002 */
        if(APP_RTOS_STATUS_SUCCESS != ret_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "app IPC failed to unlock\n");
            status = (vx_status)VX_FAILURE;
        }
#endif
    }
    else
    {
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

vx_status tivxEventWait(tivx_event event, uint32_t timeout)
{
    vx_status status = (vx_status)VX_SUCCESS;
    app_rtos_status_t retVal;
    uint32_t bsp_timeout;

    if (NULL != event)
    {
        if (TIVX_EVENT_TIMEOUT_WAIT_FOREVER == timeout)
        {
            bsp_timeout = APP_RTOS_SEMAPHORE_WAIT_FOREVER;
        }
        else if (TIVX_EVENT_TIMEOUT_NO_WAIT == timeout)
        {
            bsp_timeout = APP_RTOS_SEMAPHORE_NO_WAIT;
        }
        else
        {
            bsp_timeout = timeout;
        }

        retVal = appRtosSemaphorePend((app_rtos_semaphore_handle_t)event, bsp_timeout);

        if (APP_RTOS_STATUS_TIMEOUT == retVal)
        {
            VX_PRINT(VX_ZONE_INFO, "Semaphore wait timed out\n");
            status = (app_rtos_status_t)TIVX_ERROR_EVENT_TIMEOUT;
        }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1760- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_RTOS_EVENT_UM003 */
        else if (APP_RTOS_STATUS_FAILURE == retVal)
        {
            VX_PRINT(VX_ZONE_ERROR, "Semaphore wait returned an error\n");
            status = (vx_status)VX_FAILURE;
        }
#endif
        else
        {
            /*Do Nothing*/
        }

    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Event was NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

vx_status tivxEventClear(tivx_event event)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != event)
    {
        (void)appRtosSemaphoreReset((app_rtos_semaphore_handle_t)event);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Event was NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    return status;
}
