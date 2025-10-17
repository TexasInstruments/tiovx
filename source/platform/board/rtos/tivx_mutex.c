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

vx_status tivxMutexCreate(tivx_mutex *mutex)
{
    vx_status status = (vx_status)VX_FAILURE;
    app_rtos_semaphore_handle_t handle;
    app_rtos_semaphore_params_t semParams;

    if (NULL != mutex)
    {
        /* Default parameter initialization */
        appRtosSemaphoreParamsInit(&semParams);

        semParams.mode = APP_RTOS_SEMAPHORE_MODE_BINARY;
        semParams.initValue = 1U;

        handle = appRtosSemaphoreCreate(semParams);

        if (NULL == handle)
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "Semaphore create returned NULL\n");
            VX_PRINT(VX_ZONE_ERROR, "  Check for memory leak, or may need to increase\n");
            #ifdef FREERTOS
            VX_PRINT(VX_ZONE_ERROR, "  the value of OSAL_FREERTOS_MAX_SEMAPHOREP_PER_SOC\n");
            #elif defined (SAFERTOS)
            VX_PRINT(VX_ZONE_ERROR, "  the value of OSAL_SAFERTOS_MAX_SEMAPHOREP_PER_SOC\n");
            #endif
            VX_PRINT(VX_ZONE_ERROR, "  in pdk/packages/ti/osal/soc/<>/osal_soc.h\n");
        }
        else
        {
            *mutex = (tivx_mutex)handle;
            status = (vx_status)VX_SUCCESS;
            ownLogResourceAlloc("TIVX_MUTEX_MAX_OBJECTS", 1);
        }
    }

    return (status);
}

vx_status tivxMutexDelete(tivx_mutex *mutex)
{
    vx_status status = (vx_status)VX_FAILURE;
    app_rtos_status_t retVal;
    app_rtos_semaphore_handle_t handle;

    if ((NULL != mutex) && (*mutex != NULL))
    {
        handle = (tivx_mutex)*mutex;
        retVal = appRtosSemaphoreDelete(&handle);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_RTOS_MUTEX_UM001
<justification end> */
        if (APP_RTOS_STATUS_SUCCESS != retVal)
        {
            VX_PRINT(VX_ZONE_ERROR, "Semaphore delete returned an error\n");
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_RTOS_MUTEX_UM001
<justification end> */
        else
/* LDRA_JUSTIFY_END */
        {
            *mutex = NULL;
            status = (vx_status)VX_SUCCESS;
            ownLogResourceFree("TIVX_MUTEX_MAX_OBJECTS", 1);
        }
    }

    return (status);
}

vx_status tivxMutexLock(tivx_mutex mutex)
{
    vx_status status = (vx_status)VX_SUCCESS;
    app_rtos_status_t retVal;

    if (NULL != mutex)
    {
        retVal = appRtosSemaphorePend((app_rtos_semaphore_handle_t)mutex,
            APP_RTOS_SEMAPHORE_WAIT_FOREVER);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_RTOS_MUTEX_UM002
<justification end> */
        if (APP_RTOS_STATUS_SUCCESS != retVal)
        {
            VX_PRINT(VX_ZONE_ERROR, "Semaphore wait failed\n");
            status = (vx_status)VX_FAILURE;
        }
/* LDRA_JUSTIFY_END */
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Mutex is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return (status);
}

vx_status tivxMutexUnlock(tivx_mutex mutex)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != mutex)
    {
        (void)appRtosSemaphorePost((app_rtos_semaphore_handle_t)mutex);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Mutex is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return (status);
}
