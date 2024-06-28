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

vx_status tivxMutexCreate(tivx_mutex *mutex)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t temp_status;
    pthread_mutexattr_t mutex_attr;
    tivx_mutex tmp_mutex;

    tmp_mutex = (tivx_mutex)ownPosixObjectAlloc((vx_enum)TIVX_POSIX_TYPE_MUTEX);
    if(tmp_mutex==NULL)
    {
        *mutex = NULL;
        VX_PRINT(VX_ZONE_ERROR, "Mutex memory allocation failed\n");
        status = (vx_status)VX_ERROR_NO_MEMORY;
    }
    else
    {
        temp_status = (uint32_t)status | (uint32_t)pthread_mutexattr_init(&mutex_attr);
        status = (vx_status)temp_status;
        temp_status = (uint32_t)status | (uint32_t)pthread_mutex_init(&tmp_mutex->lock, &mutex_attr);
        status = (vx_status)temp_status;
       

        if(status!=0)
        {
            (void)pthread_mutex_destroy(&tmp_mutex->lock);
            status = ownPosixObjectFree((uint8_t *)tmp_mutex, (vx_enum)TIVX_POSIX_TYPE_MUTEX);
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Mutex free failed\n");
            }
            *mutex = NULL;
            VX_PRINT(VX_ZONE_ERROR, "Mutex initialization failed\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
        else
        {
            *mutex = tmp_mutex;
        }
        (void)pthread_mutexattr_destroy(&mutex_attr);
    }

    return (status);
}

vx_status tivxMutexDelete(tivx_mutex *mutex)
{
    vx_status status = (vx_status)VX_FAILURE;

    if(*mutex != NULL)
    {
        (void)pthread_mutex_destroy(&(*mutex)->lock);
        (void)ownPosixObjectFree((uint8_t *)(*mutex), (vx_enum)TIVX_POSIX_TYPE_MUTEX);
        *mutex = NULL;
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}

vx_status tivxMutexLock(tivx_mutex mutex)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;

    if(mutex != NULL)
    {
        status = pthread_mutex_lock(&mutex->lock);
        if(status != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "Mutex lock failed\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    return (status);
}

vx_status tivxMutexUnlock(tivx_mutex mutex)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;

    if(mutex != NULL)
    {
        status = pthread_mutex_unlock(&mutex->lock);
        if(status != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "Mutex unlock failed\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    return (status);
}



