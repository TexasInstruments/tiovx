/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
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
#include <pthread.h>

typedef struct _tivx_event_t {

    uint16_t is_set;
    pthread_mutex_t lock;
    pthread_cond_t  cond;

} tivx_linux_event_t;

vx_status tivxEventCreate(tivx_event *event)
{
    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;
    tivx_event tmp_event;
    vx_status status = VX_SUCCESS;

    tmp_event = (tivx_event)malloc(sizeof(tivx_linux_event_t));
    if(tmp_event==NULL)
    {
        *event = NULL;
        VX_PRINT(VX_ZONE_ERROR, "tivxEventCreate: Memory allocation failed\n");
        status = VX_ERROR_NO_MEMORY;
    }
    else
    {
        status |= pthread_mutexattr_init(&mutex_attr);
        status |= pthread_condattr_init(&cond_attr);

        status |= pthread_mutex_init(&tmp_event->lock, &mutex_attr);
        status |= pthread_cond_init(&tmp_event->cond, &cond_attr);

        tmp_event->is_set = 0;

        if(status!=0)
        {
            free(tmp_event);
            *event = NULL;
            VX_PRINT(VX_ZONE_ERROR, "tivxEventCreate: Mutex initialization failed\n");
            status = VX_ERROR_NO_MEMORY;
        }
        else
        {
            *event = tmp_event;
        }

        pthread_condattr_destroy(&cond_attr);
        pthread_mutexattr_destroy(&mutex_attr);
    }

    return (status);
}

vx_status tivxEventDelete(tivx_event *event)
{
    vx_status status = VX_FAILURE;

    if(*event)
    {
        pthread_cond_destroy(&(*event)->cond);
        pthread_mutex_destroy(&(*event)->lock);
        free(*event);
        *event = NULL;
        status = VX_SUCCESS;
    }

    return (status);
}

vx_status tivxEventPost(tivx_event event)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if(event)
    {
        status = pthread_mutex_lock(&event->lock);
        if(status == 0)
        {
            event->is_set = 1;

            status |= pthread_cond_signal(&event->cond);
            status |= pthread_mutex_unlock(&event->lock);
        }
        if(status != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxEventPost: Mutex post failed\n");
            status = VX_FAILURE;
        }
    }

    return (status);
}

vx_status tivxEventWait(tivx_event event, uint32_t timeout)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if(event)
    {
        status = pthread_mutex_lock(&event->lock);
        if(status == 0)
        {
            vx_bool done = vx_false_e;

            while(!done)
            {
                if(event->is_set==1)
                {
                    /* clear event */
                    event->is_set = 0;
                    status = VX_SUCCESS;
                    done = vx_true_e;
                }
                else
                if(timeout==TIVX_EVENT_TIMEOUT_NO_WAIT)
                {
                    VX_PRINT(VX_ZONE_ERROR, "tivxEventWait: Timeout set to TIVX_EVENT_TIMEOUT_NO_WAIT\n");
                    status = VX_FAILURE;
                    done = vx_true_e;
                }
                else
                {
                    pthread_cond_wait(&event->cond, &event->lock);
                }
            }
            status |= pthread_mutex_unlock(&event->lock);
        }
        if(status != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxEventWait: Mutex unlock failed\n");
            status = VX_FAILURE;
        }
    }

    return (status);
}

vx_status tivxEventClear(tivx_event event)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if(event)
    {
        status = pthread_mutex_lock(&event->lock);
        if(status == 0)
        {
            event->is_set = 0;
            status = pthread_mutex_unlock(&event->lock);
        }
        if(status != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxEventClear: Mutex lock failed\n");
            status = VX_FAILURE;
        }
    }

    return status;
}
