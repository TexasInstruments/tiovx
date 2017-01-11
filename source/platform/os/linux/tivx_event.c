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
/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <vx_internal.h>
#include <pthread.h>

typedef struct _tivx_event_t {

    uint16_t is_set;
    pthread_mutex_t lock;
    pthread_cond_t  cond;

} tivx_event_t;

vx_status tivxEventCreate(tivx_event *event)
{
    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;
    tivx_event tmp_event;
    vx_status status = VX_SUCCESS;

    tmp_event = (tivx_event)malloc(sizeof(tivx_event_t));
    if(tmp_event==NULL)
    {
        *event = NULL;
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
            status = VX_FAILURE;
        }
    }

    return status;
}
