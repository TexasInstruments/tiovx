/*
*
* Copyright (c) 2025 Texas Instruments Incorporated
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
            ownLogResourceAlloc("TIVX_EVENT_MAX_OBJECTS", 1);
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
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_RTOS_TIVX_EVENT_UBR001
<justification end> */
        if ((app_rtos_status_t)APP_RTOS_STATUS_SUCCESS == ret_status)
/* LDRA_JUSTIFY_END */
        {
            *event = NULL;
            ownLogResourceFree("TIVX_EVENT_MAX_OBJECTS", 1);
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_RTOS_EVENT_UM001
<justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Semaphore delete returned an error\n");
            status = (vx_status)VX_FAILURE;
        }
/* LDRA_JUSTIFY_END */
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

    if (NULL != event)
    {
        (void)appRtosSemaphorePost((app_rtos_semaphore_handle_t)event);
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
        if (VX_TIMEOUT_WAIT_FOREVER == timeout)
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
            status = (app_rtos_status_t)VX_ERROR_TIMEOUT;
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_RTOS_EVENT_UM002
<justification end> */
        else if (APP_RTOS_STATUS_FAILURE == retVal)
        {
            VX_PRINT(VX_ZONE_ERROR, "Semaphore wait returned an error\n");
            status = (vx_status)VX_FAILURE;
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_RTOS_EVENT_UM002
<justification end> */
        else
/* LDRA_JUSTIFY_END */
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
