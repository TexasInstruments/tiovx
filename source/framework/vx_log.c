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


#include <vx_internal.h>

VX_API_ENTRY void VX_API_CALL vxRegisterLogCallback(vx_context cntxt, vx_log_callback_f callback, vx_bool reentrant)
{
    if (ownIsValidContext(cntxt) == vx_true_e)
    {
        ownContextLock(cntxt);
        if ((cntxt->log_callback == NULL) && (callback != NULL))
        {
            cntxt->log_enabled = vx_true_e;

            /* reentrant is ignored, lock is always taken */
        }
        if ((cntxt->log_callback != NULL) && (callback == NULL))
        {
            cntxt->log_enabled = vx_false_e;
        }
        cntxt->log_callback = callback;
        ownContextUnlock(cntxt);
    }
}

VX_API_ENTRY void VX_API_CALL vxAddLogEntry(vx_reference ref, vx_status status, const char *message, ...)
{
    va_list ap;
    vx_context context = NULL;
    vx_status ret = VX_SUCCESS;
    /* this is non-reentrant hence need to take lock */
    static vx_char string[VX_MAX_LOG_MESSAGE_LEN];

    if (ownIsValidReference(ref) == vx_false_e)
    {
        if (ownIsValidContext((vx_context)ref) == vx_false_e)
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid reference!\n");
            ret = VX_ERROR_INVALID_REFERENCE;
        }
    }

    if(ret==VX_SUCCESS)
    {
        if (status == VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid status code!\n");
            ret = VX_ERROR_INVALID_VALUE;
        }
    }

    if(ret==VX_SUCCESS)
    {
        if (message == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid message!\n");
            ret = VX_ERROR_INVALID_VALUE;
        }
    }

    if(ret==VX_SUCCESS)
    {
        if (ref->type == VX_TYPE_CONTEXT)
        {
            context = (vx_context)ref;
        }
        else
        {
            context = ref->context;
        }

        if (context->log_callback == NULL)
        {
            ret = VX_ERROR_INVALID_VALUE;
        }
    }
    if(ret==VX_SUCCESS)
    {
        if (context->log_enabled == vx_false_e)
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

