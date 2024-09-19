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

static vx_bool ownIsValidObject(vx_enum type);
static vx_status ownDestructObjArray(vx_reference ref);
static vx_status ownInitObjArrayFromObject(
    vx_context context, vx_object_array objarr, vx_reference exemplar);
static vx_status ownAllocObjectArrayBuffer(vx_reference ref);
static vx_status ownAddRefToObjArray(vx_context context,
            vx_object_array objarr, vx_reference ref, uint32_t i);
static vx_status ownReleaseRefFromObjArray(
            vx_object_array objarr, uint32_t num_items);

static vx_bool ownIsValidObject(vx_enum type)
{
    vx_bool status = (vx_bool)vx_false_e;

    if (((vx_enum)VX_TYPE_IMAGE == type) ||
        ((vx_enum)VX_TYPE_TENSOR == type) || /* TIOVX-1896- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR001 */
        ((vx_enum)VX_TYPE_ARRAY == type) ||
        (VX_TYPE_USER_DATA_OBJECT == type) || /* TIOVX-1896- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR002 */
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
    vx_status status = (vx_status)VX_SUCCESS;

    if ((ownIsValidContext(context) == (vx_bool)vx_true_e) &&
        (NULL != exemplar))
    {
        if (((vx_bool)vx_true_e == ownIsValidObject(exemplar->type)) &&
            (count <= TIVX_OBJECT_ARRAY_MAX_ITEMS))
        {
            vx_reference ref = ownCreateReference(context, (vx_enum)VX_TYPE_OBJECT_ARRAY, (vx_enum)VX_EXTERNAL, &context->base);
            if ((vxGetStatus(ref) == (vx_status)VX_SUCCESS) &&
                (ref->type == (vx_enum)VX_TYPE_OBJECT_ARRAY))
            {
                /* status set to NULL due to preceding type check */
                objarr = vxCastRefAsObjectArray(ref,NULL);
                /* assign reference type specific callback's */
                objarr->base.destructor_callback = &ownDestructObjArray;
                objarr->base.mem_alloc_callback = &ownAllocObjectArrayBuffer;
                objarr->base.release_callback = &ownReleaseReferenceBufferGeneric;

                objarr->base.obj_desc = ownObjDescAlloc(
                    (vx_enum)TIVX_OBJ_DESC_OBJARRAY, vxCastRefFromObjectArray(objarr));
                if(objarr->base.obj_desc==NULL)
                {
                    (void)vxReleaseObjectArray(&objarr);

                    vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                        "Could not allocate objarr object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "Could not allocate objarr object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                    VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in source/platform/psdk_j7/common/soc/tivx_platform_psdk_<soc>.h\n");
                    objarr = (vx_object_array)ownGetErrorObject(
                        context, (vx_status)VX_ERROR_NO_RESOURCES);
                }
                else
                {
                    tivx_obj_desc_object_array_t *obj_desc =
                        (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;

                    obj_desc->item_type = exemplar->type;
                    obj_desc->num_items = (uint32_t)count;

                    ownLogSetResourceUsedValue("TIVX_OBJECT_ARRAY_MAX_ITEMS", (uint16_t)obj_desc->num_items);

                    status = ownInitObjArrayFromObject(context, objarr, exemplar);

                    if(status != (vx_status)VX_SUCCESS)
                    {
                        (void)vxReleaseObjectArray(&objarr);

                        vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                            "Could not allocate objarr object descriptor\n");
                        VX_PRINT(VX_ZONE_ERROR, "Could not allocate objarr object descriptor\n");
                        VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                        VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in source/platform/psdk_j7/common/soc/tivx_platform_psdk_<soc>.h\n");
                        objarr = (vx_object_array)ownGetErrorObject(
                            context, (vx_status)VX_ERROR_NO_RESOURCES);
                    }
                }
            }
        }
    }

    return (objarr);
}

VX_API_ENTRY vx_object_array VX_API_CALL vxCreateVirtualObjectArray(
    vx_graph graph, vx_reference exemplar, vx_size count)
{
    vx_object_array objarr = NULL;
    vx_reference ref = NULL;
    vx_context context;
    vx_status status = (vx_status)VX_SUCCESS;

    if ((ownIsValidSpecificReference(vxCastRefFromGraph(graph), (vx_enum)VX_TYPE_GRAPH) ==
                (vx_bool)vx_true_e) &&
        (NULL != exemplar))
    {
        context = graph->base.context;

        if (((vx_bool)vx_true_e == ownIsValidObject(exemplar->type)) &&
            (count <= TIVX_OBJECT_ARRAY_MAX_ITEMS))
        {
            ref = ownCreateReference(
                context, (vx_enum)VX_TYPE_OBJECT_ARRAY, (vx_enum)VX_EXTERNAL, &context->base);

            if ((vxGetStatus(ref) == (vx_status)VX_SUCCESS) &&
                (ref->type == (vx_enum)VX_TYPE_OBJECT_ARRAY))
            {
                /* status set to NULL due to preceding type check */
                objarr = vxCastRefAsObjectArray(ref,NULL);
                /* assign refernce type specific callback's */
                objarr->base.destructor_callback = &ownDestructObjArray;
                objarr->base.mem_alloc_callback = &ownAllocObjectArrayBuffer;
                objarr->base.release_callback = &ownReleaseReferenceBufferGeneric;

                objarr->base.obj_desc = ownObjDescAlloc(
                    (vx_enum)TIVX_OBJ_DESC_OBJARRAY, vxCastRefFromObjectArray(objarr));
                if(objarr->base.obj_desc==NULL)
                {
                    (void)vxReleaseObjectArray(&objarr);

                    vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                        "Could not allocate objarr object descriptor\n");
                    objarr = (vx_object_array)ownGetErrorObject(
                        context, (vx_status)VX_ERROR_NO_RESOURCES);
                    VX_PRINT(VX_ZONE_ERROR, "Could not allocate object array object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                    VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in source/platform/psdk_j7/common/soc/tivx_platform_psdk_<soc>.h\n");
                }
                else
                {
                    tivx_obj_desc_object_array_t *obj_desc =
                        (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;

                    obj_desc->item_type = exemplar->type;
                    obj_desc->num_items = (uint32_t)count;

                    ownLogSetResourceUsedValue("TIVX_OBJECT_ARRAY_MAX_ITEMS", (uint16_t)obj_desc->num_items);

                    status = ownInitObjArrayFromObject(context, objarr, exemplar);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1859- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_OBJARRAY_UTJT003 */
                    if(status != (vx_status)VX_SUCCESS)
                    {
                        (void)vxReleaseObjectArray(&objarr);

                        vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                            "Could not allocate objarr object descriptor\n");
                        VX_PRINT(VX_ZONE_ERROR, "Could not allocate objarr object descriptor\n");
                        VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                        VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in source/platform/psdk_j7/common/soc/tivx_platform_psdk_<soc>.h\n");
                        objarr = (vx_object_array)ownGetErrorObject(
                            context, (vx_status)VX_ERROR_NO_RESOURCES);
                    }
                    else
/* END: TIOVX_CODE_COVERAGE_OBJARRAY_UTJT003 */
#endif
                    {
                        objarr->base.is_virtual = (vx_bool)vx_true_e;
                        ownReferenceSetScope(&objarr->base, &graph->base);
                    }
                }
            }
        }
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
                (vx_bool)vx_true_e) && /* TIOVX-1896- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR003 */
                (obj_desc != NULL) && /* TIOVX-1896- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR004 */
            (index < obj_desc->num_items) &&
            (objarr->base.is_virtual == (vx_bool)vx_false_e))
        {
            ref = ownReferenceGetHandleFromObjDescId(obj_desc->obj_desc_id[index]);

            if (NULL != ref) /* TIOVX-1896- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR005 */
            {
                /* Setting it as void since the return value 'count' is not used further */
                (void)ownIncrementReference(ref, (vx_enum)VX_EXTERNAL);
                /* set is_array_element flag */
                ref->is_array_element = (vx_bool)vx_true_e;
            }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1859- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_OBJARRAY_UTJT004 */
            else
            {
                vxAddLogEntry(&objarr->base.context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                    "Object array item is invalid\n");
                ref = ownGetErrorObject(objarr->base.context, (vx_status)VX_ERROR_NO_RESOURCES);
                VX_PRINT(VX_ZONE_ERROR, "Object array item is invalid\n");
            }
/* END: TIOVX_CODE_COVERAGE_OBJARRAY_UTJT004 */
#endif
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
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid query attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

static vx_status ownInitObjArrayFromObject(
    vx_context context, vx_object_array objarr, vx_reference exemplar)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_reference ref;
    tivx_obj_desc_object_array_t *obj_desc =
        (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;
    vx_uint32 num_items, i;

    num_items = obj_desc->num_items;
    for (i = 0; i < num_items; i ++)
    {
        ref = tivxCreateReferenceFromExemplar(context, exemplar);

        status = vxGetStatus(ref);

        if(status == (vx_status)VX_SUCCESS)
        {
            status = ownAddRefToObjArray(context, objarr, ref, i);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxCreateReferenceFromExemplar Failed\n");
            break;
        }
    }

    if ((vx_status)VX_SUCCESS != status)
    {
/*LDRA_NOANALYSIS*/
/* TIOVX-1859- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_OBJARRAY_UTJT005 */
        for (; i > 0u; i--)
        {
            status = ownReleaseRefFromObjArray(objarr, i);
            if(status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,"Releasing reference from object array failed\n");
                break;
            }
        }
/* END: TIOVX_CODE_COVERAGE_OBJARRAY_UTJT005 */
/*LDRA_ANALYSIS*/
    }

    return (status);
}

static vx_status ownAddRefToObjArray(vx_context context, vx_object_array objarr, vx_reference ref, uint32_t i)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (vxGetStatus(ref) == (vx_status)VX_SUCCESS) /* TIOVX-1896- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR006 */
    {
        tivx_obj_desc_object_array_t *obj_desc =
            (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;

        objarr->ref[i] = ref;
        obj_desc->obj_desc_id[i] =
            ref->obj_desc->obj_desc_id;

        /* Setting the element index so that we can index into object array */
        ref->obj_desc->element_idx = i;

        /* increment the internal counter on the image, not the
         * external one. Setting it as void since the return value
         * 'count' is not used further.
         */
        (void)ownIncrementReference(ref, (vx_enum)VX_INTERNAL);

        ownReferenceSetScope(ref, &objarr->base);
    }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1706- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_OBJARRAY_UM001 */
    else
    {
        status = (vx_status)VX_FAILURE;
        VX_PRINT(VX_ZONE_ERROR, "Could not allocate image object descriptor\n");
        vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
           "Could not allocate image object descriptor\n");
    }
#endif

   return status;
}

static vx_status ownReleaseRefFromObjArray(vx_object_array objarr, uint32_t num_items)
{
    uint32_t i;
    vx_status status = (vx_status)VX_SUCCESS;

    for (i = 0; i < num_items; i ++)
    {
        if (NULL != objarr->ref[i])
        {
            /* decrement the internal counter on the object, not the
             * external one. Setting it as void since the return value
             * 'count' is not used further.
             */
            (void)ownDecrementReference(objarr->ref[i], (vx_enum)VX_INTERNAL);

            status = vxReleaseReference(&objarr->ref[i]);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1706- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_OBJARRAY_UM002 */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Release object array element %d failed!\n", i);
                break;
            }
#endif
        }
    }
    return status;
}

static vx_status ownDestructObjArray(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_object_array objarr = NULL;

    if(ref->type == (vx_enum)VX_TYPE_OBJECT_ARRAY) /* TIOVX-1896- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR007 */
    {
        /* status set to NULL due to preceding type check */
        objarr = vxCastRefAsObjectArray(ref,NULL);
        if(objarr->base.obj_desc!=NULL)
        {
            tivx_obj_desc_object_array_t *obj_desc =
                (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;

            status = ownReleaseRefFromObjArray(objarr, obj_desc->num_items);

            if ((vx_status)VX_SUCCESS == status) /* TIOVX-1896- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR008 */
            {
                status = ownObjDescFree(&objarr->base.obj_desc);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1706- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_OBJARRAY_UM005 */
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Object array object descriptor release failed!\n");
                }
#endif
            }
        }
        else
        {
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
            VX_PRINT(VX_ZONE_ERROR, "Object descriptor is NULL!\n");
        }
    }
    return status;
}

static vx_status ownAllocObjectArrayBuffer(vx_reference objarr_ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 i=0;
    vx_object_array objarr = NULL;
    tivx_obj_desc_object_array_t *obj_desc = NULL;
    vx_reference ref;

    if(objarr_ref->type == (vx_enum)VX_TYPE_OBJECT_ARRAY) /* TIOVX-1896- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR009 */
    {
        /* status set to NULL due to preceding type check */
        objarr = vxCastRefAsObjectArray(objarr_ref,NULL);
        if(objarr->base.obj_desc != NULL) /* TIOVX-1896- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR010 */
        {
            obj_desc = (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;
            for (i = 0u; i < obj_desc->num_items; i++)
            {
                ref = objarr->ref[i];

                if (ref != NULL) /* TIOVX-1896- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR011 */
                {
                    status = (vx_status)VX_SUCCESS;
                    if(ref->mem_alloc_callback != NULL)
                    {
                        status = ref->mem_alloc_callback(ref);
                    }
/*LDRA_NOANALYSIS*/
/* TIOVX-1859- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_OBJARRAY_UTJT001 */
                    if ((vx_status)VX_SUCCESS != status)
                    {
                        break;
                    }
/* END: TIOVX_CODE_COVERAGE_OBJARRAY_UTJT001 */
/*LDRA_ANALYSIS*/
                }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1859- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_OBJARRAY_UTJT002 */
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Object array reference is NULL for %d num_item\n", i);
                    status = (vx_status)VX_ERROR_INVALID_VALUE;
                }
/* END: TIOVX_CODE_COVERAGE_OBJARRAY_UTJT002 */
#endif
            }

            if ((vx_status)VX_SUCCESS==status) /* TIOVX-1896- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_OBJARRAY_UBR012 */
            {
                objarr_ref->is_allocated = (vx_bool)vx_true_e;
            }
        }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1706- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_OBJARRAY_UM003 */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Object array object descriptor is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_VALUE;
        }
#endif
    }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1706- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_OBJARRAY_UM004 */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Data type is not Object Array\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
#endif

    return status;
}
