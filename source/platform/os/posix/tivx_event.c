/*
*
* Copyright (c) 2017-2021 Texas Instruments Incorporated
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

#include <tivx_platform_posix.h>
#include <errno.h>
#include <sys/time.h>

vx_status tivxEventCreate(tivx_event *event)
{
    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;
    tivx_event tmp_event;
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t temp_status;

    if (NULL != event)
    {
        tmp_event = (tivx_event)ownPosixObjectAlloc((vx_enum)TIVX_POSIX_TYPE_EVENT);
        if(tmp_event==NULL)
        {
            *event = NULL;
            VX_PRINT(VX_ZONE_ERROR, "Memory allocation failed\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
        else
        {
        
            temp_status = (uint32_t)status | (uint32_t)pthread_mutexattr_init(&mutex_attr);
            status = (vx_status)temp_status;
            temp_status = (uint32_t)status | (uint32_t)pthread_condattr_init(&cond_attr);
            status = (vx_status)temp_status;


            temp_status = (uint32_t)status | (uint32_t)pthread_mutex_init(&tmp_event->lock, &mutex_attr);
            status = (vx_status)temp_status;
            temp_status = (uint32_t)status | (uint32_t)pthread_cond_init(&tmp_event->cond, &cond_attr);
            status = (vx_status)temp_status;

            tmp_event->is_set = (uint16_t)0;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_EVENT_UM001 <justification end> */
/* TIOVX-1731- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_EVENT_UM001 */
            if(status!=0)
            {
                (void)pthread_cond_destroy(&tmp_event->cond);
                (void)pthread_mutex_destroy(&tmp_event->lock);
                status = ownPosixObjectFree((uint8_t *)tmp_event, (vx_enum)TIVX_POSIX_TYPE_EVENT);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Event free failed\n");
                }

                *event = NULL;
                VX_PRINT(VX_ZONE_ERROR, "Mutex initialization failed\n");
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_EVENT_UM001 <justification end> */
            else
/* LDRA_JUSTIFY_END */
            {
                *event = tmp_event;
            }

            (void)pthread_condattr_destroy(&cond_attr);
            (void)pthread_mutexattr_destroy(&mutex_attr);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Event is NULL\n");
        status = (vx_status)VX_FAILURE;
    }
    return (status);
}

vx_status tivxEventDelete(tivx_event *event)
{
    vx_status status = (vx_status)VX_FAILURE;

    if((NULL != event) && (*event != NULL))
    {
        (void)pthread_cond_destroy(&(*event)->cond);
        (void)pthread_mutex_destroy(&(*event)->lock);
        (void)ownPosixObjectFree((uint8_t *)(*event), (vx_enum)TIVX_POSIX_TYPE_EVENT);
        *event = NULL;
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}

vx_status tivxEventPost(tivx_event event)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    uint32_t temp_status;

    if(event != NULL)
    {
        status = pthread_mutex_lock(&event->lock);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_EVENT_UBR001
<justification end> */

        if(status == 0) /* TIOVX-1944- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_EVENT_UBR001 */
        {
            event->is_set = 1;

            temp_status = (uint32_t)status | (uint32_t)pthread_cond_signal(&event->cond);
            status = (vx_status)temp_status;
            temp_status = (uint32_t)status | (uint32_t)pthread_mutex_unlock(&event->lock);
            status = (vx_status)temp_status;
           
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_EVENT_UM002
<justification end> */
/* TIOVX-1731- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_EVENT_UM002 */
        if(status != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "Mutex post failed\n");
            status = (vx_status)VX_FAILURE;
        }
/* LDRA_JUSTIFY_END */
    }

    return (status);
}

vx_status tivxEventWait(tivx_event event, uint32_t timeout)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    int32_t   status1;
    int32_t   retVal;

    if(event != NULL)
    {
        status = pthread_mutex_lock(&event->lock);
        if(status == 0) /* TIOVX-1944- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_EVENT_UBR002 */
        {
            bool done = (bool)vx_false_e;

            while(!done)
            {
                if(event->is_set==1U)
                {
                    /* clear event */
                    event->is_set = 0;
                    status = (vx_status)VX_SUCCESS;
                    done = (bool)vx_true_e;
                }
                else
                if(timeout==TIVX_EVENT_TIMEOUT_NO_WAIT)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                             "Timeout set to TIVX_EVENT_TIMEOUT_NO_WAIT\n");
                    status = (vx_status)TIVX_ERROR_EVENT_TIMEOUT;
                    done = (bool)vx_true_e;
                }
                else
                if(timeout!=TIVX_EVENT_TIMEOUT_WAIT_FOREVER)
                {
                    /* A valid and finite timeout has been specified. */
                    struct timespec ts;
                    struct timeval  tv;

                    retVal = gettimeofday(&tv, NULL);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_EVENT_UBR003
<justification end> */
                    if (retVal == 0) /* TIOVX-1944- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_EVENT_UBR003 */
/* LDRA_JUSTIFY_END */
                    {
                        uint32_t        sec;
                        unsigned long   micro;

                        /* timeout is expected to be in milli-sec. */
                        micro = (uint64_t)tv.tv_usec + ((uint64_t)timeout * 1000U);
                        sec   = (uint32_t)tv.tv_sec;

                        if (micro >= 1000000LLU)
                        {
                            sec   += (uint32_t)micro/(uint32_t)1000000LLU;
                            micro %= (uint32_t)1000000LLU;
                        }

                        ts.tv_nsec = (int64_t)micro * 1000;
                        ts.tv_sec  = (int64_t)sec;

                        retVal = pthread_cond_timedwait(&event->cond,
                                                        &event->lock,
                                                        &ts);

                        if (retVal == ETIMEDOUT)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Event timed-out.\n");
                            status = (vx_status)TIVX_ERROR_EVENT_TIMEOUT;
                            done = (bool)vx_true_e;
                        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_EVENT_UM006
<justification end> */
/* TIOVX-1731- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_EVENT_UM006 */
                        else if ((int32_t)0 != retVal)
                        {
                            /* Error other than ETIMEDOUT. */
                            VX_PRINT(VX_ZONE_ERROR, "Event wait failed.\n");
                            status = (vx_status)VX_FAILURE;
                            done = (bool)vx_true_e;
                        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_EVENT_UM006
<justification end> */
                        else
/* LDRA_JUSTIFY_END */
                        {
                            /*Do Nothing*/
                        }
                    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_EVENT_UM005
<justification end> */
/* TIOVX-1731- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_EVENT_UM005 */
                    else
                    {
                        /* gettimeofday() failed. */
                        VX_PRINT(VX_ZONE_ERROR, "gettimeofday() failed.\n");
                        status = (vx_status)VX_FAILURE;
                        done = (bool)vx_true_e;
                    }
/* LDRA_JUSTIFY_END */
                }
                else
                {
                    /* timeout == TIVX_EVENT_TIMEOUT_WAIT_FOREVER */
                    retVal = pthread_cond_wait(&event->cond, &event->lock);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_EVENT_UM007
<justification end> */
/* TIOVX-1731- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_EVENT_UM007 */
                    if ((int32_t)0 != retVal)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Event wait failed.\n");
                        status = (vx_status)VX_FAILURE;
                        done = (bool)vx_true_e;
                    }
/* LDRA_JUSTIFY_END */
                }
            }

            status1 = pthread_mutex_unlock(&event->lock);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_EVENT_UM003
<justification end> */
/* TIOVX-1731- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_EVENT_UM003 */
            if(status1 != 0)
            {
                VX_PRINT(VX_ZONE_ERROR, "Mutex unlock failed\n");
                status = (vx_status)VX_FAILURE;
            }
/* LDRA_JUSTIFY_END */
        }
    }

    return (status);
}

vx_status tivxEventClear(tivx_event event)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;

    if(event != NULL)
    {
        status = pthread_mutex_lock(&event->lock);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_EVENT_UBR004
<justification end> */
        if(status == 0) /* TIOVX-1944- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_EVENT_UBR004 */
        {
            event->is_set = 0;
            status = pthread_mutex_unlock(&event->lock);
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_EVENT_UM004
<justification end> */
/* TIOVX-1731- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_EVENT_UM004 */
        if(status != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "Mutex lock failed\n");
            status = (vx_status)VX_FAILURE;
        }
/* LDRA_JUSTIFY_END */
    }

    return status;
}
