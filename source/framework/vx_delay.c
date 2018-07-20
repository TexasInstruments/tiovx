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

#define tivxIsValidDelay(d) ((NULL != d) && (ownIsValidSpecificReference((vx_reference)(d), VX_TYPE_DELAY) == vx_true_e))
#define tivxIsValidGraph(g) ((NULL != g) && (ownIsValidSpecificReference((vx_reference)(g), VX_TYPE_GRAPH) == vx_true_e))

static vx_bool ownIsValidObject(vx_enum type);
static void ownResetDelayPrmPool(vx_delay delay);
static tivx_delay_param_t *ownAllocDelayPrm(vx_delay delay);
static void ownFreeDelayPrm(vx_delay delay, tivx_delay_param_t *prm);
static vx_status ownAddRefToDelay(vx_context context, vx_delay delay, vx_reference ref, uint32_t i);
static void ownReleaseRefFromDelay(vx_delay delay, uint32_t num_items);
static vx_status ownDestructDelay(vx_reference ref);
static vx_status ownAllocDelayBuffer(vx_reference delay_ref);
static void ownDelayInit(vx_delay delay, vx_size count, vx_enum type);


static vx_bool ownIsValidObject(vx_enum type)
{
    vx_bool status = vx_false_e;

    if ((VX_TYPE_IMAGE == type) ||
        (VX_TYPE_ARRAY == type) ||
        (VX_TYPE_SCALAR == type) ||
        (VX_TYPE_DISTRIBUTION == type) ||
        (VX_TYPE_THRESHOLD == type) ||
        (VX_TYPE_PYRAMID == type) ||
        (VX_TYPE_MATRIX == type) ||
        (VX_TYPE_REMAP == type)  ||
        (VX_TYPE_LUT == type) ||
        (VX_TYPE_CONVOLUTION == type)
        )
    {
        status = vx_true_e;
    }

    return (status);
}

static void ownResetDelayPrmPool(vx_delay delay)
{
    vx_uint32 i;
    for(i=0; i<TIVX_DELAY_MAX_PRM_OBJECT; i++)
    {
        delay->prm_pool[i].next = NULL;
        delay->prm_pool[i].node = NULL;
        delay->prm_pool[i].index = 0;
    }
}

static tivx_delay_param_t *ownAllocDelayPrm(vx_delay delay)
{
    tivx_delay_param_t *prm = NULL;
    vx_uint32 i;

    for(i=0; i<TIVX_DELAY_MAX_PRM_OBJECT; i++)
    {
        if(delay->prm_pool[i].node==NULL)
        {
            prm = &delay->prm_pool[i];
            prm->next = NULL;
            prm->index = 0;
            tivxLogSetResourceUsedValue("TIVX_DELAY_MAX_PRM_OBJECT", i+1);
            break;
        }
    }

    if (prm == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "ownAllocDelayPrm: May need to increase the value of TIVX_DELAY_MAX_PRM_OBJECT in tiovx/include/tivx_config.h\n");
    }

    return prm;
}

static void ownFreeDelayPrm(vx_delay delay, tivx_delay_param_t *prm)
{
    prm->node = NULL;
    prm->next = NULL;
    prm->index = 0;
}

vx_bool ownAddAssociationToDelay(vx_reference value,
                                vx_node n, vx_uint32 i)
{
    vx_delay delay = value->delay;
    vx_int32 delay_index = value->delay_slot_index;
    vx_bool status = vx_true_e;

    vx_int32 index = (delay->index + abs(delay_index)) % (vx_int32)delay->count;


    if (delay->set[index].node == NULL) /* head is empty */
    {
        delay->set[index].node = n;
        delay->set[index].index = i;
    }
    else
    {
        tivx_delay_param_t **ptr = &delay->set[index].next;
        do
        {
            if (*ptr == NULL)
            {
                *ptr = ownAllocDelayPrm(delay);
                if (*ptr != NULL)
                {
                    (*ptr)->node = n;
                    (*ptr)->index = i;
                }
                else
                {
                    status = vx_false_e;
                }
                break;
            }
            else
            {
                ptr = &((*ptr)->next);
            }
        } while (1);
    }

    if(status == vx_true_e)
    {
        /* Increment a reference to the delay */
        ownIncrementReference((vx_reference)delay, VX_INTERNAL);
    }

    return status;
}


vx_bool ownRemoveAssociationToDelay(vx_reference value,
                                   vx_node n, vx_uint32 i)
{
    vx_delay delay = value->delay;
    vx_int32 delay_index = value->delay_slot_index;
    vx_bool do_break;

    vx_int32 index = delay_index;
    vx_bool status = vx_true_e;

    if ( (delay->set[index].node == n) && (delay->set[index].index == i) ) /* head is a match */
    {
        delay->set[index].node = NULL;
        delay->set[index].index = 0;
    }
    else
    {
        tivx_delay_param_t **ptr = &delay->set[index].next;
        tivx_delay_param_t *next = NULL;
        do_break = vx_false_e;
        do
        {
            if (*ptr != NULL)
            {
                if ( ((*ptr)->node == n) && ((*ptr)->index == i) )
                {
                    next = (*ptr)->next;
                    ownFreeDelayPrm(delay, *ptr);
                    *ptr = next;
                    do_break = vx_true_e;
                }
                else
                {
                   ptr = &((*ptr)->next);
                }
            }
            else
            {
                status = vx_false_e;
                do_break = vx_true_e;
            }

            if (vx_true_e == do_break)
            {
                break;
            }
        } while (1);
    }


    if (status == vx_true_e) /* Release the delay */
    {
        vx_reference ref=(vx_reference)delay;
        ownReleaseReferenceInt(&ref, VX_TYPE_DELAY, VX_INTERNAL, NULL);
    }
    return status;
}

static vx_status ownAddRefToDelay(vx_context context, vx_delay delay, vx_reference ref, uint32_t i)
{
    vx_status status = VX_SUCCESS;

    delay->refs[i] = ref;

    /* set the object as a delay element */
    ownInitReferenceForDelay(ref, delay, (vx_int32)i);

    /* increment the internal counter on the image, not the
       external one */
    ownIncrementReference(ref, VX_INTERNAL);

    return status;
}

static void ownReleaseRefFromDelay(vx_delay delay, uint32_t num_items)
{
    uint32_t i;

    if (tivxIsValidDelay(delay) && (delay->type == VX_TYPE_PYRAMID) && (delay->pyr_num_levels > 0) )
    {
        /* release pyramid delays */
        for (i = 0; i < delay->pyr_num_levels; i++)
        {
            ownReleaseReferenceInt((vx_reference *)&(delay->pyr_delay[i]), VX_TYPE_DELAY, VX_INTERNAL, NULL);
        }
        delay->pyr_num_levels = 0;
    }

    for (i = 0; i < num_items; i ++)
    {
        if (NULL != delay->refs[i])
        {
            /* decrement the internal counter on the image, not the
               external one */
            ownDecrementReference(delay->refs[i], VX_INTERNAL);

            vxReleaseReference(&delay->refs[i]);
        }
    }
}

static vx_status ownDestructDelay(vx_reference ref)
{
    vx_delay delay = (vx_delay)ref;

    if(delay->base.type == VX_TYPE_DELAY)
    {
        ownReleaseRefFromDelay(delay, delay->count);
    }
    return VX_SUCCESS;
}

static vx_status ownAllocDelayBuffer(vx_reference delay_ref)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i=0;
    vx_delay delay = (vx_delay)delay_ref;
    vx_reference ref;

    if(delay->base.type == VX_TYPE_DELAY)
    {
        for (i = 0u; i < delay->count; i++)
        {
            ref = delay->refs[i];

            if (ref)
            {
                status = VX_SUCCESS;
                if(ref->mem_alloc_callback)
                {
                    status = ref->mem_alloc_callback(ref);
                }

                if (VX_SUCCESS != status)
                {
                    break;
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "ownAllocDelayBuffer: delay reference %d is null\n", i);
                status = VX_ERROR_INVALID_VALUE;
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "ownAllocDelayBuffer: reference type is not delay\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static void ownDelayInit(vx_delay delay, vx_size count, vx_enum type)
{
    vx_size i;

    delay->type = type;
    delay->count = count;
    delay->pyr_num_levels = 0;
    delay->index = 0;
    ownResetDelayPrmPool(delay);
    for(i=0; i<count; i++)
    {
        delay->set[i].next = NULL;
        delay->set[i].node = NULL;
        delay->set[i].index = 0;
    }

    /* assign refernce type specific callback's */
    delay->base.destructor_callback = &ownDestructDelay;
    delay->base.mem_alloc_callback = &ownAllocDelayBuffer;
    delay->base.release_callback =
        (tivx_reference_release_callback_f)&vxReleaseDelay;
}

VX_API_ENTRY vx_delay VX_API_CALL vxCreateDelay(vx_context context,
                              vx_reference exemplar,
                              vx_size count)
{
    vx_status status = VX_SUCCESS;
    vx_delay delay = NULL;
    vx_reference ref;
    vx_uint32 i;

    if ( ownIsValidContext(context) )
    {
        if ((ownIsValidReference(exemplar)) && (ownIsValidObject(exemplar->type)) && (count <= TIVX_DELAY_MAX_OBJECT) )
        {
            delay = (vx_delay)ownCreateReference(
                                    context, VX_TYPE_DELAY, VX_EXTERNAL, &context->base);
            if ( (vxGetStatus((vx_reference)delay) == VX_SUCCESS) && (delay->base.type == VX_TYPE_DELAY) )
            {
                ownDelayInit(delay, count, exemplar->type);

                for(i=0; i<count; i++)
                {
                    ref = ownCreateReferenceFromExemplar(context, exemplar);
                    status = VX_SUCCESS;
                    if(ownIsValidReference(ref)==vx_false_e)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "vxCreateDelay: invalid reference type\n");
                        status = VX_ERROR_INVALID_REFERENCE;
                    }
                    if(status == VX_SUCCESS)
                    {
                        status = ownAddRefToDelay(context, delay, ref, i);
                    }
                    if(status!=VX_SUCCESS)
                    {
                        break;
                    }
                }
                if(status == VX_SUCCESS)
                {
                    tivxLogSetResourceUsedValue("TIVX_DELAY_MAX_OBJECT", count);
                }
                if ( (status == VX_SUCCESS) && (exemplar->type == VX_TYPE_PYRAMID) )
                {
                    vx_size levels, level_idx;
                    vx_delay pyrdelay;

                    status = vxQueryPyramid((vx_pyramid)exemplar, VX_PYRAMID_LEVELS, &levels, sizeof(levels));
                    delay->pyr_num_levels = 0;
                    if(status == VX_SUCCESS)
                    {
                        for (level_idx = 0; level_idx < levels; level_idx++)
                        {
                            pyrdelay = (vx_delay)ownCreateReference(context, VX_TYPE_DELAY, VX_INTERNAL, (vx_reference)delay);
                            delay->pyr_delay[level_idx] = pyrdelay;
                            if ( (vxGetStatus((vx_reference)pyrdelay) == VX_SUCCESS) && (pyrdelay->base.type == VX_TYPE_DELAY) )
                            {
                                ownDelayInit(pyrdelay, count, VX_TYPE_IMAGE);
                                for (i = 0; i < count; i++)
                                {
                                    ref = (vx_reference)vxGetPyramidLevel((vx_pyramid)delay->refs[i], (vx_uint32)level_idx);

                                    status = ownAddRefToDelay(context, pyrdelay, ref, i);
                                    if(status!=VX_SUCCESS)
                                    {
                                        VX_PRINT(VX_ZONE_ERROR, "vxCreateDelay: reference was not added to delay\n");
                                        break;
                                    }
                                }
                                delay->pyr_num_levels++;
                            }
                        }
                    }
                }
                if(status!=VX_SUCCESS)
                {
                    ownReleaseRefFromDelay(delay, i);
                    vxReleaseDelay(&delay);

                    VX_PRINT(VX_ZONE_ERROR, "vxCreateDelay: Could not allocate delay object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "vxCreateDelay: May need to increase the value of TIVX_DELAY_MAX_OBJECT in tiovx/include/tivx_config.h\n");
                    vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                        "Could not allocate delay object descriptor\n");
                    delay = (vx_delay)ownGetErrorObject(
                        context, VX_ERROR_NO_RESOURCES);
                }
            }
        }
        else
        {
            delay = (vx_delay)ownGetErrorObject(context, VX_ERROR_INVALID_REFERENCE);
        }
    }

    return delay;
}

VX_API_ENTRY vx_reference VX_API_CALL vxGetReferenceFromDelay(
    vx_delay delay, vx_int32 index)
{
    vx_reference ref = NULL;
    if (tivxIsValidDelay(delay) == vx_true_e)
    {
        if ((vx_uint32)abs(index) < delay->count)
        {
            vx_int32 i = (delay->index + abs(index)) % (vx_int32)delay->count;
            ref = delay->refs[i];
        }
        else
        {
            vxAddLogEntry(&delay->base, VX_ERROR_INVALID_PARAMETERS, "Failed to retrieve reference from delay by index %d\n", index);
            ref = (vx_reference)ownGetErrorObject(delay->base.context, VX_ERROR_INVALID_PARAMETERS);
        }
    }
    return ref;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryDelay(vx_delay delay,
    vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;

    if (tivxIsValidDelay(delay) == vx_true_e)
    {
        switch (attribute)
        {
            case VX_DELAY_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_enum *)ptr = delay->type;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryDelay: delay type query failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_DELAY_SLOTS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = (vx_size)delay->count;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryDelay: delay slots query failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "vxQueryDelay: invalid attribute\n");
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryDelay: invalid delay\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseDelay(vx_delay *d)
{
    return ownReleaseReferenceInt((vx_reference *)d, VX_TYPE_DELAY, VX_EXTERNAL, NULL);
}

VX_API_ENTRY vx_status VX_API_CALL vxAgeDelay(vx_delay delay)
{
    vx_status status = VX_SUCCESS;

    if (tivxIsValidDelay(delay) == vx_true_e)
    {
        vx_int32 i,j;

        /* increment the index */
        delay->index = (delay->index + 1) % (vx_uint32)delay->count;

        /* then reassign the parameters */
        for (i = 0; i < (vx_int32)delay->count; i++)
        {
            tivx_delay_param_t *param = NULL;

            j = (delay->index + i) % (vx_int32)delay->count;
            param = &delay->set[i];
            do
            {
                if (param->node != 0)
                {
                    ownNodeSetParameter(param->node,
                                       param->index,
                                       delay->refs[j]);

                    ownInitReferenceForDelay(delay->refs[j], delay, i);
                }
                param = param->next;
            } while (param != NULL);
        }

        if ( (delay->type == VX_TYPE_PYRAMID) && (delay->pyr_num_levels > 0) )
        {
            /* age pyramid levels */
            vx_int32 numLevels = (vx_int32)delay->pyr_num_levels;
            for (i = 0; i < numLevels; ++i)
            {
                vxAgeDelay(delay->pyr_delay[i]);
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryDelay: invalid delay\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

