/*
 * Copyright (c) 2012-2025 The Khronos Group Inc.
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
/*
 * Copyright (c) 2025 Texas Instruments Incorporated
 */



#include <vx_internal.h>

static vx_bool ownIsValidObjArrayType(vx_enum type);
static vx_status ownDestructObjArray(vx_reference ref);
static vx_status ownInitObjArrayFromObject(vx_context context,
            vx_object_array objarr, vx_reference exemplar);
static vx_status ownAllocObjArrayBuffer(vx_reference ref);
static vx_status ownAddRefToObjArray(vx_context context,
            vx_object_array objarr, vx_reference ref, uint32_t i);
static vx_status ownReleaseRefFromObjArray(
            vx_object_array objarr, uint32_t num_items);
static vx_object_array ownCreateObjArrayInt(vx_context context, vx_reference* src, 
            vx_size count, vx_bool is_virtual, vx_bool is_from_list, vx_graph graph);
static vx_status ownInitObjArrayFromList(vx_context context, 
            vx_object_array objArray, vx_reference list[]);

static vx_status VX_CALLBACK objectArrayKernelCallback(vx_enum kernel_enum, vx_bool validate_only, const vx_reference input, const vx_reference output)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 i, num_items = ((tivx_obj_desc_object_array_t *)input->obj_desc)->num_items;

    if ((vx_bool)vx_true_e == validate_only)
    {
        if ((vx_bool)vx_true_e == tivxIsReferenceMetaFormatEqual(input, output))
        {
            status = (vx_status)VX_SUCCESS;
        }
        else
        {
            status = (vx_status)VX_ERROR_NOT_COMPATIBLE;
        }
        for (i = 0U; (i < num_items) && ((vx_status)VX_SUCCESS == status); ++i)
        {
            vx_reference p2[2] = {vxCastRefAsObjectArray(input, &status)->ref[i], vxCastRefAsObjectArray(output, &status)->ref[i]};
            if (tivxIsReferenceMetaFormatEqual(p2[0], p2[1]) != (vx_bool)vx_true_e)
            {
                VX_PRINT(VX_ZONE_ERROR, "Corresponding children of object array from list have mismatched meta-data, no copy/swap possible.\n");
                status = (vx_status)VX_ERROR_NOT_COMPATIBLE;
                break;
            }
        }
    }
    else    /* dispatch to each sub-object in turn */
    {
        vx_uint32 item;
        for (item = 0U; (item < num_items) && ((vx_status)VX_SUCCESS == status); ++item)
        {
            vx_reference p2[2] = {vxCastRefAsObjectArray(input, &status)->ref[item], vxCastRefAsObjectArray(output, &status)->ref[item]};
            vx_kernel_callback_f kf = p2[0]->kernel_callback;
            if ((kf != NULL) && /* TIOVX-1896- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR010 */
                ((vx_status)VX_SUCCESS == status)) /* TIOVX-1896- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR011 */
            {
                status = (*kf)(kernel_enum, (vx_bool)vx_false_e, p2[0], p2[1]);
                /* Not all the condition combinations of the below block are testable since, per definition,  we cannot have a callback 
                   if the input has no supplementary */
                if (((vx_status)VX_SUCCESS == status) &&
                    (NULL != p2[0]->supplementary_data) &&
                    (NULL != p2[1]->supplementary_data) &&
                    (NULL != p2[0]->supplementary_data->base.kernel_callback))
                {
                    vx_reference supp_params[2] = {&p2[0]->supplementary_data->base, &p2[1]->supplementary_data->base};
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_OBJDESC_UM007
<justification end> */
                    if ((vx_status)VX_SUCCESS == p2[0]->supplementary_data->base.kernel_callback(kernel_enum, (vx_bool)vx_true_e, supp_params[0], supp_params[1]))
                    {
                        status = p2[0]->supplementary_data->base.kernel_callback(kernel_enum, (vx_bool)vx_false_e, supp_params[0], supp_params[1]);
                    }
/* LDRA_JUSTIFY_END */
                }
                else
                {
                    VX_PRINT(VX_ZONE_WARNING, "No Supplementary data available, no copy/swap possible.\n");
                }
            }
            else /* TIOVX_CODE_COVERAGE_OBJARRAY_UM006 */
            {
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
            }
        }
    }
    return status;
}

static vx_bool ownIsValidObjArrayType(vx_enum type)
{
    vx_bool status = (vx_bool)vx_false_e;

    if (((vx_enum)VX_TYPE_IMAGE == type) ||
        ((vx_enum)VX_TYPE_TENSOR == type) ||
        ((vx_enum)VX_TYPE_ARRAY == type) ||
        (VX_TYPE_USER_DATA_OBJECT == type) ||
        (TIVX_TYPE_RAW_IMAGE == type) ||
        ((vx_enum)VX_TYPE_SCALAR == type) ||
        ((vx_enum)VX_TYPE_DISTRIBUTION == type) ||
        ((vx_enum)VX_TYPE_THRESHOLD == type) ||
        ((vx_enum)VX_TYPE_PYRAMID == type) ||
        ((vx_enum)VX_TYPE_MATRIX == type) ||
        ((vx_enum)VX_TYPE_REMAP == type)  ||
        ((vx_enum)VX_TYPE_LUT == type))
    {
        status = (vx_bool)vx_true_e;
    }

    return (status);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseObjectArray(vx_object_array *arr)
{
    return (ownReleaseReferenceInt(
        vxCastRefFromObjectArrayP(arr), (vx_enum)VX_TYPE_OBJECT_ARRAY, (vx_enum)VX_EXTERNAL, NULL));
}

VX_API_ENTRY vx_object_array VX_API_CALL vxCreateObjectArray(
    vx_context context, vx_reference exemplar, vx_size count)
{
    vx_object_array objarr = NULL;
    if (ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        objarr = ownCreateObjArrayInt(context, &exemplar, count, (vx_bool)vx_false_e, (vx_bool)vx_false_e, NULL);
    }
    return (objarr);
}

VX_API_ENTRY vx_object_array VX_API_CALL vxCreateVirtualObjectArray(
    vx_graph graph, vx_reference exemplar, vx_size count)
{
    vx_object_array objarr = NULL;
    vx_context context;

    if ((ownIsValidSpecificReference(vxCastRefFromGraph(graph), (vx_enum)VX_TYPE_GRAPH) ==
                (vx_bool)vx_true_e))
    {
        context = graph->base.context;
        objarr = ownCreateObjArrayInt(context, &exemplar, count, (vx_bool)vx_true_e, (vx_bool)vx_false_e, graph);
    }

    return (objarr);
}

VX_API_ENTRY vx_object_array VX_API_CALL tivxCreateObjectArrayFromList(
    vx_context context, vx_reference list[], vx_size count)
{
    vx_object_array objarr = NULL;

    if (ownIsValidContext(context) == (vx_bool)vx_true_e)
    { 
        objarr = ownCreateObjArrayInt(context, list, count, (vx_bool)vx_false_e, (vx_bool)vx_true_e, NULL);
    }

    return (objarr);
}

VX_API_ENTRY vx_object_array VX_API_CALL tivxCreateVirtualObjectArrayFromList(
    vx_graph graph, vx_reference list[], vx_size count)
{
    vx_object_array objarr = NULL;
    vx_context context;

    if ((ownIsValidSpecificReference(vxCastRefFromGraph(graph), (vx_enum)VX_TYPE_GRAPH) ==
                (vx_bool)vx_true_e))
    {
        context = graph->base.context;
        objarr = ownCreateObjArrayInt(context, list, count, (vx_bool)vx_true_e, (vx_bool)vx_true_e, graph);
    }

    return (objarr);
}

VX_API_ENTRY vx_reference VX_API_CALL vxGetObjectArrayItem(
    vx_object_array objarr, vx_uint32 index)
{
    vx_reference ref = NULL;

    if (ownIsValidSpecificReference(vxCastRefFromObjectArray(objarr), (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_false_e)
    {
        vx_context context = ownGetContext();
        if (ownIsValidContext(context) == (vx_bool)vx_true_e)
        {
            vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                "Provided reference was not an object array, returning an error reference\n");
            ref = ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
            VX_PRINT(VX_ZONE_ERROR, "Provided reference was not an object array, returning an error reference\n");
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid object array reference\n");
            VX_PRINT(VX_ZONE_ERROR, "Context has not yet been created, returning NULL for vx_reference\n");
        }
    }
    else
    {
        tivx_obj_desc_object_array_t *obj_desc =
            (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;

        if ((ownIsValidSpecificReference(vxCastRefFromObjectArray(objarr), (vx_enum)VX_TYPE_OBJECT_ARRAY) ==
                (vx_bool)vx_true_e) && /* TIOVX-1896- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR001 */
                (obj_desc != NULL) &&
            (index < obj_desc->num_items))
        {
            ref = ownReferenceGetHandleFromObjDescId(obj_desc->obj_desc_id[index]);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR002
<justification end> */
            if (NULL != ref)
/* LDRA_JUSTIFY_END */
            {
                /* Setting it as void since the return value 'count' is not used further */
                (void)ownIncrementReference(ref, (vx_enum)VX_EXTERNAL);
                /* set is_array_element flag */
                ref->is_array_element = (vx_bool)vx_true_e;
            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_OBJARRAY_UTJT004
<justification end> */
            else
            {
                vxAddLogEntry(&objarr->base.context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                    "Object array item is invalid\n");
                ref = ownGetErrorObject(objarr->base.context, (vx_status)VX_ERROR_NO_RESOURCES);
                VX_PRINT(VX_ZONE_ERROR, "Object array item is invalid\n");
            }
/* LDRA_JUSTIFY_END */
        }
    }

    return (ref);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryObjectArray(
    vx_object_array objarr, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((ownIsValidSpecificReference(vxCastRefFromObjectArray(objarr), (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_false_e)
        ||
        (objarr->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid object array reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case (vx_enum)VX_OBJECT_ARRAY_ITEMTYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    tivx_obj_desc_object_array_t *obj_desc =
                        (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;

                    *(vx_enum *)ptr = obj_desc->item_type;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query object array item type failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_OBJECT_ARRAY_NUMITEMS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    tivx_obj_desc_object_array_t *obj_desc =
                        (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;

                    *(vx_size *)ptr = obj_desc->num_items;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query object array num items failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_OBJECT_ARRAY_IS_FROM_LIST:
                if (VX_CHECK_PARAM(ptr, size, vx_bool, 0x3U))
                    {
                        tivx_obj_desc_object_array_t *obj_desc =
                            (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;

                        *(vx_bool *)ptr = obj_desc->is_from_list;
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Query object array is from list failed\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                    break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid query attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

static vx_object_array ownCreateObjArrayInt(vx_context context, vx_reference* src, vx_size count, vx_bool is_virtual, vx_bool is_from_list, vx_graph graph)
{
    /* Case 1: src = &Exemplar, is_from_list = vx_false_e
     *     src needs to be used as *src dereferenced
     * Case 2: src = list, is_from_list = vx_false_e
     *     src is list[], *src = list[0]
     */
    vx_object_array objarr = NULL;
    vx_status status = (vx_status)VX_SUCCESS;

    if ((src == NULL) ||
        (ownIsValidReference(*src) != (vx_bool)vx_true_e))
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "Invalid source reference passed to object array creation\n");
    }
    else if ((count > TIVX_OBJECT_ARRAY_MAX_ITEMS) || (count <= 0U))
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "Invalid count parameter passed to object array creation\n");
    }
    else if (ownIsValidObjArrayType((*src)->type) != (vx_bool)vx_true_e)
    {
        status = (vx_status)VX_ERROR_INVALID_TYPE;
        VX_PRINT(VX_ZONE_ERROR, "Source reference passed to object array creation is of invalid type\n");
    }
    else
    {
        /* do nothing */
    }

    if (status != (vx_status)VX_SUCCESS)
    {
        objarr = (vx_object_array)ownGetErrorObject(
                        context, (vx_status)status);
    }
    else
    {
        /* ownCreateReference has its own internal error logging */
        vx_reference ref = ownCreateReference(context, (vx_enum)VX_TYPE_OBJECT_ARRAY, (vx_enum)VX_EXTERNAL, &context->base);
        if ((vxGetStatus(ref) == (vx_status)VX_SUCCESS) &&
            (ref->type == (vx_enum)VX_TYPE_OBJECT_ARRAY))
        {
            /* status set to NULL due to preceding type check */
            objarr = vxCastRefAsObjectArray(ref,NULL);
            /* assign reference type specific callback's */
            objarr->base.destructor_callback = &ownDestructObjArray;
            objarr->base.mem_alloc_callback = &ownAllocObjArrayBuffer;
            objarr->base.release_callback = &ownReleaseReferenceBufferGeneric;
            objarr->base.kernel_callback = &objectArrayKernelCallback;
            objarr->base.obj_desc = ownObjDescAlloc(
                (vx_enum)TIVX_OBJ_DESC_OBJARRAY, vxCastRefFromObjectArray(objarr));
            if (objarr->base.obj_desc==NULL)
            {
                (void)vxReleaseObjectArray(&objarr);

                vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                    "Could not allocate objarr object descriptor\n");
                VX_PRINT(VX_ZONE_ERROR, "Could not allocate objarr object descriptor\n");
                VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available.\n");
                VX_PRINT_BOUND_ERROR("TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST");

                objarr = (vx_object_array)ownGetErrorObject(
                    context, (vx_status)VX_ERROR_NO_RESOURCES);
            }
            else
            {
                tivx_obj_desc_object_array_t *obj_desc =
                    (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;
                obj_desc->item_type = (*src)->type;
                obj_desc->num_items = (uint32_t)count;
                obj_desc->is_from_list = is_from_list;
                if (is_virtual == (vx_bool)vx_true_e)
                {
                    objarr->base.is_virtual = is_virtual;
                    ownReferenceSetScope(&objarr->base, &graph->base);
                }

                ownLogSetResourceUsedValue("TIVX_OBJECT_ARRAY_MAX_ITEMS", (uint16_t)obj_desc->num_items);

                if (is_from_list == (vx_bool)vx_true_e)
                {
                    status = ownInitObjArrayFromList(context, objarr, src);
                }
                else
                {
                    status = ownInitObjArrayFromObject(context, objarr, *src);
                }

                if (status != (vx_status)VX_SUCCESS)
                {
                    (void)vxReleaseObjectArray(&objarr);
                    if (is_from_list == (vx_bool)vx_true_e)
                    {
                        vxAddLogEntry(&context->base, (vx_status)status,
                            "ownInitObjArrayFromList FAILED!\n");
                        VX_PRINT(VX_ZONE_ERROR, "Could not instantiate object array from given list\n");
                        VX_PRINT(VX_ZONE_ERROR, "Please check vx_status error code for more details\n");
                    }
                    else
                    {
                        status = (vx_status)VX_ERROR_NO_RESOURCES;
                        vxAddLogEntry(&context->base, (vx_status)status,
                            "Could not allocate object descriptors for objarr items\n");
                        VX_PRINT(VX_ZONE_ERROR, "Could not allocate object descriptors for objarr items\n");
                        VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available.\n");
                        VX_PRINT_BOUND_ERROR("TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST");
                    }
                    objarr = (vx_object_array)ownGetErrorObject(
                            context, (vx_status)status);
                }
            }
        }
    }
    return objarr;
}

static vx_status ownInitObjArrayFromObject(
    vx_context context, vx_object_array objarr, vx_reference exemplar)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_reference ref;
    tivx_obj_desc_object_array_t *obj_desc =
        (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;
    vx_uint32 index, k;

    for (index = 0; index < obj_desc->num_items; index++)
    {
        ref = tivxCreateReferenceFromExemplar(context, exemplar);

        status = vxGetStatus(ref);

        if (status == (vx_status)VX_SUCCESS)
        {
            status = ownAddRefToObjArray(context, objarr, ref, index);
            ref->is_virtual = objarr->base.is_virtual;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxCreateReferenceFromExemplar Failed\n");
            break;
        }
    }

    if ((vx_status)VX_SUCCESS != status)
    {
        vx_status release_status;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_OBJARRAY_UTJT005
<justification end> */
        /* Clean up partially initialized ObjArray after an error occurs, releasing all references up to the last successful index */
        for ( k = 0; k < index; k++)
        {
            release_status = ownReleaseRefFromObjArray(objarr, k);
            if (release_status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,"Releasing reference from object array failed\n");
            }
        }
/* LDRA_JUSTIFY_END */
    }

    return (status);
}

static vx_status ownInitObjArrayFromList(
    vx_context context, vx_object_array objarr, vx_reference list[])
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_reference ref;
    tivx_obj_desc_object_array_t *obj_desc =
        (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;
    vx_uint32 i, k;

    for (i = 0; i < obj_desc->num_items; i++)
    {
        ref = (vx_reference)list[i];

        status = vxGetStatus(ref);

        if (status == (vx_status)VX_SUCCESS)
        {
            if (ref->type != ((tivx_obj_desc_object_array_t *)(objarr->base.obj_desc))->item_type)
            {
                VX_PRINT(VX_ZONE_ERROR, "Creation of object array from list failed; reference %d provided in references[] is the wrong type\n", i);
                status = (vx_status)VX_ERROR_INVALID_TYPE;
            }
            else if (ref->is_virtual != objarr->base.is_virtual)
            {
                VX_PRINT(VX_ZONE_ERROR, "Creation of object array from list failed; reference %d provided in references[] of a different virtuality\n", i);
                status = (vx_status)VX_ERROR_NOT_COMPATIBLE;
            }
            /* Mixed virtual objects also have different scope, so this check comes after */
            else if (ref->scope != objarr->base.scope)
            {
                VX_PRINT(VX_ZONE_ERROR, "Creation of object array from list failed; reference %d provided in references[] has an invalid scope\n", i);
                status = (vx_status)VX_ERROR_INVALID_SCOPE;
            }
            else
            {
                status = ownAddRefToObjArray(context, objarr, ref, i);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Creation of object array failed, addition of reference %d provided in references[] FAILED\n", i);
            break;
        }
    }
    /* Clean up in case of failures */
    if ((vx_status)VX_SUCCESS != status)
    {
        /* Use a temp var to propogate the main failure status */
        vx_status release_status;
        for ( k = 0; k < i; k++)
        {
            release_status = ownReleaseRefFromObjArray(objarr, k);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_OBJARRAY_UTJT006
<justification end> */
            if (release_status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,"Releasing reference from object array failed\n");
            }
/* LDRA_JUSTIFY_END */
        }
    }
    return (status);
}

static vx_status ownAddRefToObjArray(vx_context context, vx_object_array objarr, vx_reference ref, uint32_t i)
{
    vx_status status = (vx_status)VX_SUCCESS;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR003
<justification end>*/
    if (vxGetStatus(ref) == (vx_status)VX_SUCCESS)
/* LDRA_JUSTIFY_END */
    {
        tivx_obj_desc_object_array_t *obj_desc =
            (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;

        objarr->ref[i] = ref;
        obj_desc->obj_desc_id[i] =
            ref->obj_desc->obj_desc_id;

        /* Setting the element index so that we can index into object array */
        ref->obj_desc->element_idx = i;

        if (obj_desc->is_from_list == (vx_bool)vx_false_e)
        {
            /* increment the internal counter on the image, not the
            * external one. Setting it as void since the return value
            * 'count' is not used further.
            */
            (void)ownIncrementReference(ref, (vx_enum)VX_INTERNAL);
        }
        else
        {
            /* increment both internal and external counters on the image.
            * When creating from an exemplar, the exemplar already carries an external count of 1
            * Hence, all object array children created from it will correctly have an external count of 1.
            *
            * Since the list is an array of objects already with external count 1,
            * when they are added as children, they will have an external count of 2:
            * 1. the existing reference in list[]
            * 2. and the new objectarray from list.
            */
            (void)ownIncrementReference(ref, (vx_enum)VX_BOTH);
        }

        ownReferenceSetScope(ref, &objarr->base);
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_OBJARRAY_UM001
<justification end>*/
    else
    {
        status = (vx_status)VX_FAILURE;
        VX_PRINT(VX_ZONE_ERROR, "Ref passed to ownAddRefToObjArray has NOT been allocated an object descriptor\n");
        VX_PRINT_BOUND_ERROR("TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST");
        vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
           "Could not allocate image object descriptor\n");
    }
/* LDRA_JUSTIFY_END */

   return status;
}

static vx_status ownReleaseRefFromObjArray(vx_object_array objarr, uint32_t i)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != objarr->ref[i])
    {
        /* decrement the internal counter on the object, not the
         * external one. Setting it as void since the return value
         * 'count' is not used further.
         */
        (void)ownDecrementReference(objarr->ref[i], (vx_enum)VX_INTERNAL);

        status = vxReleaseReference(&objarr->ref[i]);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_OBJARRAY_UM002
<justification end> */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Release object array element %d failed!\n", i);
        }
/* LDRA_JUSTIFY_END */
    }
    return status;
}

static vx_status ownDestructObjArray(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_object_array objarr = NULL;
    uint32_t i;

    /* status check set to NULL due to guaranteed type in internal function */
    objarr = vxCastRefAsObjectArray(ref,NULL);
    if (objarr->base.obj_desc!=NULL)
    {
        tivx_obj_desc_object_array_t *obj_desc =
            (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;

        for (i = 0; i < obj_desc->num_items; i++)
        {
            status = ownReleaseRefFromObjArray(objarr, i);
        }

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR005
<justification end> */
        if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
        {
            status = ownObjDescFree(&objarr->base.obj_desc);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_OBJARRAY_UM005
<justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Object array object descriptor release failed!\n");
            }
/* LDRA_JUSTIFY_END */
        }
    }
    else
    {
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        VX_PRINT(VX_ZONE_ERROR, "Object descriptor is NULL!\n");
    }
    return status;
}

static vx_status ownAllocObjArrayBuffer(vx_reference objarr_ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 i=0;
    vx_object_array objarr = NULL;
    tivx_obj_desc_object_array_t *obj_desc = NULL;
    vx_reference ref;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR006
<justification end>*/
    if (objarr_ref->type == (vx_enum)VX_TYPE_OBJECT_ARRAY)
/* LDRA_JUSTIFY_END */
    {
        /* status set to NULL due to preceding type check */
        objarr = vxCastRefAsObjectArray(objarr_ref,NULL);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR007
<justification end>*/
        if (objarr->base.obj_desc != NULL)
/* LDRA_JUSTIFY_END */
        {
            obj_desc = (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;
            for (i = 0U; i < obj_desc->num_items; i++)
            {
                ref = objarr->ref[i];
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR008
<justification end>*/
                if (ref != NULL)
/* LDRA_JUSTIFY_END */
                {
                    status = (vx_status)VX_SUCCESS;
                    if (ref->mem_alloc_callback != NULL)
                    {
                        status = ref->mem_alloc_callback(ref);
                    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_OBJARRAY_UTJT001
<justification end> */
                    if ((vx_status)VX_SUCCESS != status)
                    {
                        break;
                    }
/* LDRA_JUSTIFY_END */
                }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_OBJARRAY_UTJT002
<justification end>*/
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Object array reference is NULL for %d num_item\n", i);
                    status = (vx_status)VX_ERROR_INVALID_VALUE;
                }
/* LDRA_JUSTIFY_END */
            }

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR009
<justification end> */
            if ((vx_status)VX_SUCCESS==status)
            {
                objarr_ref->is_allocated = (vx_bool)vx_true_e;
            }
/* LDRA_JUSTIFY_END */
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_OBJARRAY_UM003
<justification end>*/
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Object array object descriptor is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_VALUE;
        }
/* LDRA_JUSTIFY_END */
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_OBJARRAY_UM004
<justification end>*/
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Data type is not Object Array\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
/* LDRA_JUSTIFY_END */
    return status;
}
