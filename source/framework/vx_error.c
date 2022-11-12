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

static vx_status VX_API_CALL ownReleaseErrorInt(vx_reference *ref);
static tivx_error_t *ownAllocateError(vx_context context, vx_status status);

static vx_status VX_API_CALL ownReleaseErrorInt(vx_reference *ref)
{
    return ownReleaseReferenceInt(ref, (vx_enum)VX_TYPE_ERROR, (vx_enum)VX_INTERNAL, NULL);
}

static tivx_error_t *ownAllocateError(vx_context context, vx_status status)
{
    /* PROBLEM: ownCreateReference needs error object to be created already */
    tivx_error_t *error = (tivx_error_t *)ownCreateReference(context, (vx_enum)VX_TYPE_ERROR, (vx_enum)VX_INTERNAL, &context->base);
    if (error != NULL)
    {
        error->base.release_callback = &ownReleaseErrorInt;
        error->status = status;
    }
    return error;
}

vx_bool ownCreateConstErrors(vx_context context)
{
    vx_bool ret = (vx_bool)vx_true_e;
    vx_enum e = 0;
    /* create an error object for each status enumeration */
    for (e = (vx_status)VX_STATUS_MIN; (e < (vx_status)VX_SUCCESS) && (ret == (vx_bool)vx_true_e); e++)
    {
        if (ownAllocateError(context, e) == NULL)
        {
            ret = (vx_bool)vx_false_e;
        }
    }
    return ret;
}

vx_reference ownGetErrorObject(vx_context context, vx_status status)
{
    tivx_error_t *error = NULL;
    vx_size i = 0U;

    ownContextLock(context);

    for (i = 0U; i < dimof(context->reftable); i++)
    {
        if (context->reftable[i] != NULL)
        {
            if (context->reftable[i]->type == (vx_enum)VX_TYPE_ERROR)
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

