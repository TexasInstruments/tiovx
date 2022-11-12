/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <vx_internal.h>

VX_API_ENTRY void VX_API_CALL vxRegisterLogCallback(vx_context cntxt, vx_log_callback_f callback, vx_bool reentrant)
{
    if (ownIsValidContext(cntxt) == (vx_bool)vx_true_e)
    {
        ownContextLock(cntxt);
        if ((cntxt->log_callback == NULL) && (callback != NULL))
        {
            cntxt->log_enabled = (vx_bool)vx_true_e;

            /* reentrant is ignored, lock is always taken */
        }
        if ((cntxt->log_callback != NULL) && (callback == NULL))
        {
            cntxt->log_enabled = (vx_bool)vx_false_e;
        }
        cntxt->log_callback = callback;
        ownContextUnlock(cntxt);
    }
}

VX_API_ENTRY void VX_API_CALL vxAddLogEntry(vx_reference ref, vx_status status, const char *message, ...)
{
    va_list ap;
    vx_context context = NULL;
    vx_status ret = (vx_status)VX_SUCCESS;
    /* this is non-reentrant hence need to take lock */
    static vx_char string[VX_MAX_LOG_MESSAGE_LEN];

    if (ownIsValidReference(ref) == (vx_bool)vx_false_e)
    {
        if (ownIsValidContext((vx_context)ref) == (vx_bool)vx_false_e)
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid reference!\n");
            ret = (vx_status)VX_ERROR_INVALID_REFERENCE;
        }
    }

    if(ret==(vx_status)VX_SUCCESS)
    {
        if (status == (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_WARNING, "Invalid status code; VX_SUCCESS status is not logged!\n");
            ret = (vx_status)VX_ERROR_INVALID_VALUE;
        }
    }

    if(ret==(vx_status)VX_SUCCESS)
    {
        if (message == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid message!\n");
            ret = (vx_status)VX_ERROR_INVALID_VALUE;
        }
    }

    if(ret==(vx_status)VX_SUCCESS)
    {
        if (ref->type == (vx_enum)VX_TYPE_CONTEXT)
        {
            context = (vx_context)ref;
        }
        else
        {
            context = ref->context;
        }

        if (context->log_callback == NULL)
        {
            ret = (vx_status)VX_ERROR_INVALID_VALUE;
        }
    }
    if(ret==(vx_status)VX_SUCCESS)
    {
        if (context->log_enabled == (vx_bool)vx_false_e)
        {

        }
        else
        {
            tivxMutexLock(context->log_lock);

            va_start(ap, message);
            vsnprintf(string, VX_MAX_LOG_MESSAGE_LEN, message, ap);
            string[VX_MAX_LOG_MESSAGE_LEN-1] = '\0'; /* for MSVC which is not C99 compliant */
            va_end(ap);

            context->log_callback(context, ref, status, string);

            tivxMutexUnlock(context->log_lock);
        }
    }

    return;
}

