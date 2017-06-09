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

static vx_status VX_API_CALL ownReleaseErrorInt(vx_reference *ref);
static tivx_error_t *ownAllocateError(vx_context context, vx_status status);

static vx_status VX_API_CALL ownReleaseErrorInt(vx_reference *ref)
{
    return ownReleaseReferenceInt(ref, VX_TYPE_ERROR, VX_INTERNAL, NULL);
}

static tivx_error_t *ownAllocateError(vx_context context, vx_status status)
{
    /* PROBLEM: ownCreateReference needs error object to be created already */
    tivx_error_t *error = (tivx_error_t *)ownCreateReference(context, VX_TYPE_ERROR, VX_INTERNAL, &context->base);
    if (error)
    {
        error->base.release_callback = &ownReleaseErrorInt;
        error->status = status;
    }
    return error;
}

vx_bool ownCreateConstErrors(vx_context context)
{
    vx_bool ret = vx_true_e;
    vx_enum e = 0;
    /* create an error object for each status enumeration */
    for (e = VX_STATUS_MIN; (e < VX_SUCCESS) && (ret == vx_true_e); e++)
    {
        if (ownAllocateError(context, e) == NULL)
        {
            ret = vx_false_e;
        }
    }
    return ret;
}

vx_reference ownGetErrorObject(vx_context context, vx_status status)
{
    tivx_error_t *error = NULL;
    vx_size i = 0ul;

    ownContextLock(context);

    for (i = 0ul; i < dimof(context->reftable); i++)
    {
        if (context->reftable[i] != NULL)
        {
            if (context->reftable[i]->type == VX_TYPE_ERROR)
            {
                error = (tivx_error_t *)context->reftable[i];
                if (error->status == status)
                {
                    break;
                }
                error = NULL;
            }
        }
    }

    ownContextUnlock(context);

    return (vx_reference)error;
}

