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

typedef struct _tivx_mutex_t {

  pthread_mutex_t lock;

} tivx_mutex_t;

vx_status tivxMutexCreate(tivx_mutex *mutex)
{
    vx_status status = VX_SUCCESS;
    pthread_mutexattr_t mutex_attr;
    tivx_mutex tmp_mutex;

    tmp_mutex = (tivx_mutex)malloc(sizeof(tivx_mutex_t));
    if(tmp_mutex==NULL)
    {
        *mutex = NULL;
        status = VX_ERROR_NO_MEMORY;
    }
    else
    {
        status |= pthread_mutexattr_init(&mutex_attr);
        status |= pthread_mutex_init(&tmp_mutex->lock, &mutex_attr);

        if(status!=0)
        {
            free(tmp_mutex);
            *mutex = NULL;
            status = VX_ERROR_NO_MEMORY;
        }
        else
        {
            *mutex = tmp_mutex;
        }
        pthread_mutexattr_destroy(&mutex_attr);
    }

    return (status);
}

vx_status tivxMutexDelete(tivx_mutex *mutex)
{
    vx_status status = VX_FAILURE;

    if(*mutex)
    {
        pthread_mutex_destroy(&(*mutex)->lock);
        free(*mutex);
        *mutex = NULL;
        status = VX_SUCCESS;
    }

    return (status);
}

vx_status tivxMutexLock(tivx_mutex mutex)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if(mutex)
    {
        status = pthread_mutex_lock(&mutex->lock);
        if(status != 0)
        {
            status = VX_FAILURE;
        }
    }

    return (status);
}

vx_status tivxMutexUnlock(tivx_mutex mutex)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if(mutex)
    {
        status = pthread_mutex_unlock(&mutex->lock);
        if(status != 0)
        {
            status = VX_FAILURE;
        }
    }

    return (status);
}



