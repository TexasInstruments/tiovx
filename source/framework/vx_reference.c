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

/* Constant internal to this file. */
#define TIOVX_REF_MAX_NUM_MEM_ELEM  (TIVX_PYRAMID_MAX_LEVEL_OBJECTS)

typedef struct _vx_enum_type_size {
    vx_enum item_type;
    vx_size item_size;
} vx_enum_type_size_t;

static vx_enum_type_size_t g_reference_enum_type_sizes[] = {
    {(vx_enum)VX_TYPE_INVALID,       0},
    /* Scalar Types */
    {(vx_enum)VX_TYPE_CHAR,          sizeof(vx_char)},
    {(vx_enum)VX_TYPE_INT8,          sizeof(vx_int8)},
    {(vx_enum)VX_TYPE_INT16,         sizeof(vx_int16)},
    {(vx_enum)VX_TYPE_INT32,         sizeof(vx_int32)},
    {(vx_enum)VX_TYPE_INT64,         sizeof(vx_int64)},
    {(vx_enum)VX_TYPE_UINT8,         sizeof(vx_uint8)},
    {(vx_enum)VX_TYPE_UINT16,        sizeof(vx_uint16)},
    {(vx_enum)VX_TYPE_UINT32,        sizeof(vx_uint32)},
    {(vx_enum)VX_TYPE_UINT64,        sizeof(vx_uint64)},
    {(vx_enum)VX_TYPE_FLOAT32,       sizeof(vx_float32)},
    {(vx_enum)VX_TYPE_FLOAT64,       sizeof(vx_float64)},
    {(vx_enum)VX_TYPE_ENUM,          sizeof(vx_enum)},
    {(vx_enum)VX_TYPE_BOOL,          sizeof(vx_bool)},
    {(vx_enum)VX_TYPE_SIZE,          sizeof(vx_size)},
    {(vx_enum)VX_TYPE_DF_IMAGE,      sizeof(vx_df_image)},
    {(vx_enum)TIVX_TYPE_UINTPTR,     sizeof(uintptr_t)},
    /* Structures */
    {(vx_enum)VX_TYPE_RECTANGLE,     sizeof(vx_rectangle_t)},
    {(vx_enum)VX_TYPE_COORDINATES2D, sizeof(vx_coordinates2d_t)},
    {(vx_enum)VX_TYPE_COORDINATES3D, sizeof(vx_coordinates3d_t)},
    {(vx_enum)VX_TYPE_KEYPOINT,      sizeof(vx_keypoint_t)},
    {(vx_enum)TIVX_TYPE_EVENT,       sizeof(tivx_event)},
    /* Pseudo Objects */
    {(vx_enum)VX_TYPE_META_FORMAT,   sizeof(tivx_meta_format_t)},
    /* Framework Objects */
    {(vx_enum)VX_TYPE_REFERENCE,     sizeof(tivx_reference_t)},
    {(vx_enum)VX_TYPE_CONTEXT,       sizeof(tivx_context_t)},
    {(vx_enum)VX_TYPE_GRAPH,         sizeof(tivx_graph_t)},
    {(vx_enum)VX_TYPE_NODE,          sizeof(tivx_node_t)},
    {(vx_enum)VX_TYPE_PARAMETER,     sizeof(tivx_parameter_t)},
    {(vx_enum)VX_TYPE_KERNEL,        sizeof(tivx_kernel_t)},
    {(vx_enum)TIVX_TYPE_SUPER_NODE,  sizeof(tivx_super_node_t)},
    /* data objects */
    {(vx_enum)VX_TYPE_ARRAY,         sizeof(tivx_array_t)},
    {(vx_enum)VX_TYPE_USER_DATA_OBJECT, sizeof(tivx_user_data_object_t)},
    {(vx_enum)TIVX_TYPE_RAW_IMAGE, sizeof(tivx_raw_image_t)},
    {(vx_enum)VX_TYPE_CONVOLUTION,   sizeof(tivx_convolution_t)},
    {(vx_enum)VX_TYPE_DELAY,         sizeof(tivx_delay_t)},
    {(vx_enum)VX_TYPE_DISTRIBUTION,  sizeof(tivx_distribution_t)},
    {(vx_enum)VX_TYPE_IMAGE,         sizeof(tivx_image_t)},
    {(vx_enum)VX_TYPE_TENSOR,        sizeof(tivx_tensor_t)},
    {(vx_enum)VX_TYPE_LUT,           sizeof(tivx_lut_t)},
    {(vx_enum)VX_TYPE_MATRIX,        sizeof(tivx_matrix_t)},
    {(vx_enum)VX_TYPE_PYRAMID,       sizeof(tivx_pyramid_t)},
    {(vx_enum)VX_TYPE_REMAP,         sizeof(tivx_remap_t)},
    {(vx_enum)VX_TYPE_SCALAR,        sizeof(tivx_scalar_t)},
    {(vx_enum)VX_TYPE_THRESHOLD,     sizeof(tivx_threshold_t)},
    {(vx_enum)TIVX_TYPE_DATA_REF_Q,  sizeof(tivx_data_ref_queue_t)},
    {(vx_enum)TIVX_TYPE_ARRAY_MAP_INFO,  sizeof(tivx_array_map_info_t)},
    {(vx_enum)TIVX_TYPE_IMAGE_MAP_INFO,  sizeof(tivx_image_map_info_t)},
    {(vx_enum)TIVX_TYPE_RAW_IMAGE_MAP_INFO,  sizeof(tivx_raw_image_map_info_t)},
    {(vx_enum)TIVX_TYPE_TENSOR_MAP_INFO,  sizeof(tivx_tensor_map_info_t)},
    {(vx_enum)TIVX_TYPE_USER_DATA_OBJECT_MAP_INFO,  sizeof(tivx_user_data_object_map_info_t)},
    {(vx_enum)TIVX_TYPE_EVENT_QUEUE_ELEMENT,  sizeof(tivx_event_queue_elem_t)},
    {(vx_enum)TIVX_TYPE_TARGET,  sizeof(tivx_target_t)},
    {(vx_enum)TIVX_TYPE_TARGET_KERNEL_INSTANCE,  sizeof(tivx_target_kernel_instance_t)},
    {(vx_enum)TIVX_TYPE_TARGET_KERNEL,  sizeof(tivx_target_kernel_t)},
    {(vx_enum)TIVX_TYPE_DELAY_PARAM,  sizeof(tivx_delay_param_t)},
    {(vx_enum)TIVX_TYPE_CONTEXT_USER_STRUCTS,  sizeof(tivx_user_structs_t)},
    {(vx_enum)TIVX_TYPE_GRAPH_PARAMETERS,  sizeof(tivx_parameters_t)},
    {(vx_enum)TIVX_TYPE_DATA_REF_QUEUE_LIST,  sizeof(tivx_data_ref_q_list_t)},
    {(vx_enum)TIVX_TYPE_DELAY_DATA_REF_QUEUE_LIST,  sizeof(tivx_delay_data_ref_q_list_t)}
};

static vx_bool ownIsGenericAllocReferenceType(vx_enum ref_type);
static vx_bool ownIsGenericReferenceType(vx_enum ref_type);
static vx_status ownReferenceGetMemAttrsFromObjDesc(vx_reference ref, tivx_shared_mem_ptr_t **mem_ptr, uint32_t *mem_size);

vx_size ownSizeOfEnumType(vx_enum item_type)
{
    vx_uint32 i = 0;
    vx_size size = 0U;

    for (i = 0; i < dimof(g_reference_enum_type_sizes); i++) {
        if (item_type == g_reference_enum_type_sizes[i].item_type) {
            size = g_reference_enum_type_sizes[i].item_size;
            break;
        }
    }

    return (size);
}

vx_bool ownIsValidReference(vx_reference ref)
{
    vx_bool ret = (vx_bool)vx_false_e;
    if (ref != NULL)
    {
        if ( (ref->magic == TIVX_MAGIC) &&
             (ownIsValidType(ref->type) == (vx_bool)vx_true_e) &&
             (( (ref->type != (vx_enum)VX_TYPE_CONTEXT) && (ownIsValidContext(ref->context) == (vx_bool)vx_true_e) ) ||
              ( (ref->type == (vx_enum)VX_TYPE_CONTEXT) && (ref->context == NULL) )) )
        {
            ret = (vx_bool)vx_true_e;
        }
        else if (ref->magic == TIVX_BAD_MAGIC)
        {
            VX_PRINT(VX_ZONE_INFO, "Reference has already been released and garbage collected!\n");
        }
        else if (ref->type != (vx_enum)VX_TYPE_CONTEXT)
        {
            VX_PRINT(VX_ZONE_INFO, "Not a valid reference!\n");
        }
        else
        {
            /* do nothing as ret is already initialized */
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_INFO, "Reference was NULL\n");
    }
    return ret;
}

/* Generic allocated references here are categorized as references
 * which are a simple allocation of a given size
 */
static vx_bool ownIsGenericAllocReferenceType(vx_enum ref_type)
{
    vx_bool ret = (vx_bool)vx_false_e;

    if ( (ref_type == (vx_enum)VX_TYPE_ARRAY) ||
         (ref_type == (vx_enum)VX_TYPE_CONVOLUTION) ||
         (ref_type == (vx_enum)VX_TYPE_DISTRIBUTION) ||
         (ref_type == (vx_enum)VX_TYPE_LUT) ||
         (ref_type == (vx_enum)VX_TYPE_MATRIX) ||
         (ref_type == (vx_enum)VX_TYPE_REMAP) ||
         (ref_type == (vx_enum)VX_TYPE_TENSOR) ||
         (ref_type == (vx_enum)VX_TYPE_USER_DATA_OBJECT)
       )
    {
        ret = (vx_bool)vx_true_e;
    }

    return ret;
}

/* Generic references here are categorized as references which
 * utilize a common alloc/destruct logic, i.e., they do not
 * require a unique allocation of memory data buffers
 */
static vx_bool ownIsGenericReferenceType(vx_enum ref_type)
{
    vx_bool ret = (vx_bool)vx_false_e;

    if ( (ownIsGenericAllocReferenceType(ref_type) == (vx_bool)vx_true_e) ||
         (ref_type == (vx_enum)VX_TYPE_SCALAR) ||
         (ref_type == (vx_enum)VX_TYPE_THRESHOLD)
       )
    {
        ret = (vx_bool)vx_true_e;
    }

    return ret;
}

static vx_status ownReferenceGetMemAttrsFromObjDesc(vx_reference ref, tivx_shared_mem_ptr_t **mem_ptr, uint32_t *mem_size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* Note: the obj_desc is not checked for NULL here because
     * it is checked in the previous logic */
    switch (ref->type)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_REFERENCE_UBR001
<justification end> */
    {
/* LDRA_JUSTIFY_END */
        case (vx_enum)VX_TYPE_ARRAY:
        {
            tivx_obj_desc_array_t *obj_desc = NULL;
            obj_desc = (tivx_obj_desc_array_t *)ref->obj_desc;

            *mem_ptr    = &obj_desc->mem_ptr;
            *mem_size   = obj_desc->mem_size;
            break;
        }
        case (vx_enum)VX_TYPE_CONVOLUTION:
        {
            tivx_obj_desc_convolution_t *obj_desc = NULL;
            obj_desc = (tivx_obj_desc_convolution_t *)ref->obj_desc;

            *mem_ptr    = &obj_desc->mem_ptr;
            *mem_size   = obj_desc->mem_size;
            break;
        }
        case (vx_enum)VX_TYPE_DISTRIBUTION:
        {
            tivx_obj_desc_distribution_t *obj_desc = NULL;
            obj_desc = (tivx_obj_desc_distribution_t *)ref->obj_desc;

            *mem_ptr    = &obj_desc->mem_ptr;
            *mem_size   = obj_desc->mem_size;
            break;
        }
        case (vx_enum)VX_TYPE_LUT:
        {
            tivx_obj_desc_lut_t *obj_desc = NULL;
            obj_desc = (tivx_obj_desc_lut_t *)ref->obj_desc;

            *mem_ptr    = &obj_desc->mem_ptr;
            *mem_size   = obj_desc->mem_size;
            break;
        }
        case (vx_enum)VX_TYPE_MATRIX:
        {
            tivx_obj_desc_matrix_t *obj_desc = NULL;
            obj_desc = (tivx_obj_desc_matrix_t *)ref->obj_desc;

            *mem_ptr    = &obj_desc->mem_ptr;
            *mem_size   = obj_desc->mem_size;
            break;
        }
        case (vx_enum)VX_TYPE_REMAP:
        {
            tivx_obj_desc_remap_t *obj_desc = NULL;
            obj_desc = (tivx_obj_desc_remap_t *)ref->obj_desc;

            *mem_ptr    = &obj_desc->mem_ptr;
            *mem_size   = obj_desc->mem_size;
            break;
        }
        case (vx_enum)VX_TYPE_TENSOR:
        {
            tivx_obj_desc_tensor_t *obj_desc = NULL;
            obj_desc = (tivx_obj_desc_tensor_t *)ref->obj_desc;

            *mem_ptr    = &obj_desc->mem_ptr;
            *mem_size   = obj_desc->mem_size;
            break;
        }
        case (vx_enum)VX_TYPE_USER_DATA_OBJECT:
        {
            tivx_obj_desc_user_data_object_t *obj_desc = NULL;
            obj_desc = (tivx_obj_desc_user_data_object_t *)ref->obj_desc;

            *mem_ptr    = &obj_desc->mem_ptr;
            *mem_size   = obj_desc->mem_size;
            break;
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_REFERENCE_UM001
<justification end> */
        default:
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            break;
        }
/* LDRA_JUSTIFY_END */
    }

    return status;
}

vx_status ownAllocReferenceBufferGeneric(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_t *base_obj_desc = NULL;

    if((vx_bool)vx_true_e == ownIsGenericReferenceType(ref->type))
    {
        base_obj_desc = (tivx_obj_desc_t *)ref->obj_desc;

        if(base_obj_desc == NULL)
        {
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
            VX_PRINT(VX_ZONE_ERROR, "Object descriptor is NULL!\n");
        }
    }
    else
    {
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference provided to destructor!\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if((vx_bool)vx_true_e == ownIsGenericAllocReferenceType(ref->type))
        {
            tivx_shared_mem_ptr_t  *mem_ptr = NULL;
            uint32_t       mem_size = 0;
            /* added void here as this status check of
             * ownReferenceGetMemAttrsFromObjDesc will always be true if
             * the previous condition is true
             */
            (void)ownReferenceGetMemAttrsFromObjDesc(ref, &mem_ptr, &mem_size);

            /* memory is not allocated, so allocate it */
            if(mem_ptr->host_ptr == (uint64_t)0)
            {
                status = tivxMemBufferAlloc(
                    mem_ptr, mem_size,
                    (vx_enum)TIVX_MEM_EXTERNAL);

                if ((vx_status)VX_SUCCESS == status)
                {
                    mem_ptr->shared_ptr = tivxMemHost2SharedPtr(
                        mem_ptr->host_ptr, (vx_enum)TIVX_MEM_EXTERNAL);

                    tivxCheckStatus(&status, tivxMemBufferMap((void *)(uintptr_t)mem_ptr->host_ptr, (uint32_t)mem_size,
                        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));

                    if(ref->type == VX_TYPE_USER_DATA_OBJECT)
                    {
                        (void)memset((vx_uint8 *)(uintptr_t)mem_ptr->host_ptr, 0, mem_size);

                        tivxCheckStatus(&status, tivxMemBufferUnmap((void *)(uintptr_t)mem_ptr->host_ptr, (uint32_t)mem_size,
                            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));
                    }

                    ref->is_allocated = (vx_bool)vx_true_e;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Memory allocation failed\n");
                }
            }
        }
    }

    return status;
}

vx_status ownDestructReferenceGeneric(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_t *base_obj_desc = NULL;

    base_obj_desc = (tivx_obj_desc_t *)ref->obj_desc;

    if(base_obj_desc == NULL)
    {
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        VX_PRINT(VX_ZONE_ERROR, "Object descriptor is NULL!\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if((vx_bool)vx_true_e == ownIsGenericAllocReferenceType(ref->type))
        {
            tivx_shared_mem_ptr_t  *mem_ptr = NULL;
            uint32_t       mem_size = 0;
            /* void is added as the status check for the ownReferenceGetMemAttrsFromObjDesc
            * will be always true as the previous condition is true
            */
            (void)ownReferenceGetMemAttrsFromObjDesc(ref, &mem_ptr, &mem_size);

            if(mem_ptr->host_ptr!=(uint64_t)0)
            {
                status = tivxMemBufferFree(
                    mem_ptr, mem_size);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_REFERENCE_UM002
<justification end> */
                if((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Buffer free failed!\n");
                }
/* LDRA_JUSTIFY_END */
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = ownObjDescFree((tivx_obj_desc_t**)&base_obj_desc);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_REFERENCE_UM003
<justification end> */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Object descriptor free failed!\n");
        }
/* LDRA_JUSTIFY_END */
    }

    return status;
}

vx_status ownCopyReferenceGeneric(vx_reference input, vx_reference output)
{
    tivx_shared_mem_ptr_t  *ip_mem_ptr = NULL;
    uint32_t                ip_mem_size = 0;
    tivx_shared_mem_ptr_t  *op_mem_ptr = NULL;
    uint32_t                op_mem_size = 0;
    vx_status status;
     /* added void here as this status check of
    * ownReferenceGetMemAttrsFromObjDesc will always be true if
    * the previous condition is true
    */
    (void)ownReferenceGetMemAttrsFromObjDesc(input, &ip_mem_ptr, &ip_mem_size);
    (void)ownReferenceGetMemAttrsFromObjDesc(output, &op_mem_ptr, &op_mem_size);
    status = ownReferenceLock(output);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_REFERENCE_UBR007
<justification end> */
    if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
    {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_REFERENCE_UM009
<justification end> */
        if (ip_mem_size <= op_mem_size)
/* LDRA_JUSTIFY_END */
        {
            tivxCheckStatus(&status, tivxMemBufferMap((void *)(uintptr_t)ip_mem_ptr->host_ptr, ip_mem_size,
                                                      (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_REFERENCE_UBR008
<justification end> */
            if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
            {
                tivxCheckStatus(&status, tivxMemBufferMap((void *)(uintptr_t)op_mem_ptr->host_ptr, op_mem_size,
                                                          (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_REFERENCE_UBR009
<justification end> */
                if ((vx_status)VX_SUCCESS == status)
                {
                    (void)memcpy((void *)(uintptr_t)op_mem_ptr->host_ptr, (void *)(uintptr_t)ip_mem_ptr->host_ptr, ip_mem_size);
                    /* copy specific fields*/
                    if ((vx_enum)VX_TYPE_ARRAY == input->type)
                    {
                        tivx_obj_desc_array_t *ip_obj_desc = (tivx_obj_desc_array_t *)input->obj_desc;
                        tivx_obj_desc_array_t *op_obj_desc = (tivx_obj_desc_array_t *)output->obj_desc;
                        op_obj_desc->num_items = ip_obj_desc->num_items;
                    }
                    else if ((vx_enum)VX_TYPE_USER_DATA_OBJECT == input->type)
                    {
                        tivx_obj_desc_user_data_object_t *ip_obj_desc = (tivx_obj_desc_user_data_object_t *)input->obj_desc;
                        tivx_obj_desc_user_data_object_t *op_obj_desc = (tivx_obj_desc_user_data_object_t *)output->obj_desc;
                        op_obj_desc->valid_mem_size = ip_obj_desc->valid_mem_size;
                    }
                    else
                    {
                        /* Do nothing, required by MISRA-C */
                    }
                    tivxCheckStatus(&status, tivxMemBufferUnmap((void *)(uintptr_t)op_mem_ptr->host_ptr, op_mem_size,
                                                                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
                }
/* LDRA_JUSTIFY_END */
                tivxCheckStatus(&status, tivxMemBufferUnmap((void *)(uintptr_t)ip_mem_ptr->host_ptr, ip_mem_size,
                                                            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            }
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_REFERENCE_UM010
<justification end> */
        else
        {
            status = (vx_status)VX_ERROR_NOT_COMPATIBLE;
        }
/* LDRA_JUSTIFY_END */
        (void)ownReferenceUnlock(output);
    }

    return status;
}

vx_status ownSwapReferenceGeneric(vx_reference input, vx_reference output)
{
    vx_status status = (vx_status)VX_FAILURE;
    tivx_shared_mem_ptr_t  *ip_mem_ptr = NULL;
    uint32_t                ip_mem_size = 0;
    tivx_shared_mem_ptr_t  *op_mem_ptr = NULL;
    uint32_t                op_mem_size = 0;

    /* added void here as this status check of
    * ownReferenceGetMemAttrsFromObjDesc will always be true if
    * the previous condition is true
    */
    (void)ownReferenceGetMemAttrsFromObjDesc(input, &ip_mem_ptr, &ip_mem_size);
    (void)ownReferenceGetMemAttrsFromObjDesc(output, &op_mem_ptr, &op_mem_size);

    /*lock only one reference as this is locking the global vx context*/
    status = ownReferenceLock(output);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_REFERENCE_UBR010
<justification end> */
    if ((vx_status)VX_SUCCESS == status)
    {
        tivx_shared_mem_ptr_t mem_ptr;
        tivx_obj_desc_memcpy(&mem_ptr, op_mem_ptr, (uint32_t)sizeof(tivx_shared_mem_ptr_t));
        tivx_obj_desc_memcpy(op_mem_ptr, ip_mem_ptr, (uint32_t)sizeof(tivx_shared_mem_ptr_t));
        tivx_obj_desc_memcpy(ip_mem_ptr, &mem_ptr, (uint32_t)sizeof(tivx_shared_mem_ptr_t));

        if ((vx_enum)VX_TYPE_ARRAY == input->type)
        {
            tivx_obj_desc_array_t *ip_obj_desc = (tivx_obj_desc_array_t *)input->obj_desc;
            tivx_obj_desc_array_t *op_obj_desc = (tivx_obj_desc_array_t *)output->obj_desc;
            uint32_t num_items = op_obj_desc->num_items;
            op_obj_desc->num_items = ip_obj_desc->num_items;
            ip_obj_desc->num_items = num_items;
        }
        (void)ownReferenceUnlock(output);
    }
/* LDRA_JUSTIFY_END */
    return status;
}

vx_status VX_CALLBACK ownKernelCallbackGeneric(vx_enum kernel_enum, vx_bool validate_only, const vx_reference input, const vx_reference output)
{
    vx_status res= (vx_status)VX_ERROR_NOT_SUPPORTED;

    if ((vx_bool)vx_true_e == validate_only)
    {
        if ((vx_bool)vx_true_e == tivxIsReferenceMetaFormatEqual(input, output))
        {
            res = (vx_status)VX_SUCCESS;
        }
        else
        {
            res = (vx_status)VX_ERROR_NOT_COMPATIBLE;
        }
    }
    else
    {
        switch (kernel_enum)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_REFERENCE_UBR011
<justification end> */
        {
/* LDRA_JUSTIFY_END */
            case (vx_enum)VX_KERNEL_COPY:
                res = ownCopyReferenceGeneric(input, output);
                break;
            case (vx_enum)VX_KERNEL_SWAP:    /* Swap and move do exactly the same */
            case (vx_enum)VX_KERNEL_MOVE:
                res = ownSwapReferenceGeneric(input, output);
                break;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_REFERENCE_UM011
<justification end> */
            default:
                res = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
/* LDRA_JUSTIFY_END */
        }
    }
    return (res);
}

vx_status ownCreateReferenceLock(vx_reference ref)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;

    if (ref != NULL)
    {
        status = (vx_status)VX_SUCCESS;

        if((ref->type==(vx_enum)VX_TYPE_CONTEXT) || (ref->type==(vx_enum)VX_TYPE_GRAPH))
        {
            /* create reference only for context and graph
             * for others use the context lock
             */
            status = tivxMutexCreate(&ref->lock);
            if (status != 0)
            {
                VX_PRINT(VX_ZONE_ERROR, "Cannot create Semaphore\n");
            }
        }
    }

    return status;
}

vx_status ownDeleteReferenceLock(vx_reference ref)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;

    if (ref != NULL)
    {
        status = (vx_status)VX_SUCCESS;

        if((ref->type==(vx_enum)VX_TYPE_CONTEXT) || (ref->type==(vx_enum)VX_TYPE_GRAPH))
        {
            /* delete reference only from context and graph
             * for others use the context lock
             */
            status = tivxMutexDelete(&ref->lock);
            if (status != 0)
            {
                VX_PRINT(VX_ZONE_ERROR, "Cannot delete mutex\n");
            }
        }
    }

    return status;
}

vx_status ownInitReference(vx_reference ref, vx_context context, vx_enum ref_type, vx_reference scope)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;

    if (ref != NULL)
    {
        ref->magic = TIVX_MAGIC;
        ref->type = ref_type;
        ref->context = context;
        ref->internal_count = 0;
        ref->external_count = 0;
        ref->mem_alloc_callback = NULL;
        ref->destructor_callback = NULL;
        ref->delay = NULL;
        ref->delay_slot_index = 0;
        ref->is_virtual = (vx_bool)vx_false_e;
        ref->obj_desc = NULL;
        ref->lock = NULL;
        ref->is_accessible = (vx_bool)vx_false_e;
        ref->is_array_element = (vx_bool)vx_false_e;
        ref->is_allocated = (vx_bool)vx_false_e;
        ref->supplementary_data = NULL;

        ownReferenceSetScope(ref, scope);

        status = ownCreateReferenceLock(ref);
    }

    return status;
}

vx_uint32 ownDecrementReference(vx_reference ref, vx_enum reftype)
{
    vx_uint32 result = (uint32_t)UINT32_MAX;
    vx_status status = (vx_status)VX_SUCCESS;
    if (ref != NULL)
    {
        status = ownReferenceLock(ref);
        if((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to lock reference \n");
        }
        else
        {
            if ((reftype == (vx_enum)VX_INTERNAL) || (reftype == (vx_enum)VX_BOTH))
            {
                if (ref->internal_count == 0U)
                {
                    VX_PRINT(VX_ZONE_WARNING, "#### INTERNAL REF COUNT IS ALREADY ZERO!!! "VX_FMT_REF" type:%08x #####\n", ref, ref->type);
                }
                else
                {
                    ref->internal_count--;
                }
            }
            if ((reftype == (vx_enum)VX_EXTERNAL) || (reftype == (vx_enum)VX_BOTH))
            {
                if (ref->external_count == 0U)
                {
                    VX_PRINT(VX_ZONE_WARNING, "#### EXTERNAL REF COUNT IS ALREADY ZERO!!! "VX_FMT_REF" type:%08x #####\n", ref, ref->type);
                }
                else
                {
                    ref->external_count--;
                }
            }

            result = ref->internal_count + ref->external_count;
            (void)ownReferenceUnlock(ref);
        }
    }
    return result;
}

vx_uint32 ownIncrementReference(vx_reference ref, vx_enum reftype)
{
    vx_uint32 count = 0u;
    vx_status status = (vx_status)VX_SUCCESS;
    if (ref != NULL)
    {
        status = ownReferenceLock(ref);
        if((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to lock reference \n");
        }
        else
        {
            if ((reftype == (vx_enum)VX_EXTERNAL) || (reftype == (vx_enum)VX_BOTH))
            {
                ref->external_count++;
            }
            if ((reftype == (vx_enum)VX_INTERNAL) || (reftype == (vx_enum)VX_BOTH))
            {
                ref->internal_count++;
            }
            count = ref->internal_count + ref->external_count;
            (void)ownReferenceUnlock(ref);
        }
    }
    return count;
}

vx_status ownReleaseReferenceInt(vx_reference *pref,
                        vx_enum ref_type,
                        vx_enum reftype,
                        tivx_reference_callback_f special_destructor)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_reference ref;
    vx_bool is_removed = (vx_bool)vx_true_e;

    if (pref != NULL)
    {
        ref = *pref;
    }
    else
    {
        ref = NULL;
    }

    if (ownIsValidSpecificReference(ref, ref_type) == (vx_bool)vx_true_e)
    {
        if (ownDecrementReference(ref, reftype) == 0U)
        {
            tivx_reference_callback_f destructor = special_destructor;

            if ((NULL != ref->supplementary_data) && (NULL != (ref->supplementary_data->base.release_callback)))
            {
                (void)ref->supplementary_data->base.release_callback((vx_reference *)&ref->supplementary_data);
            }

            is_removed = ownRemoveReferenceFromContext(ref->context, ref);

            if (is_removed  == (vx_bool)vx_false_e)
            {
                VX_PRINT(VX_ZONE_ERROR,"Invalid reference\n");
                status = (vx_status)VX_ERROR_INVALID_REFERENCE;
            }
            else
            {
                /* find the destructor method */
                if (destructor==NULL)
                {
                    destructor = ref->destructor_callback;
                }

                /* if there is a destructor, call it. */
                if (destructor != NULL)
                {
                    status = destructor(ref);

                    if (status != (vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"destructor() returned error.\n");
                    }
                }

                (void)ownDeleteReferenceLock(ref);
                ref->magic = TIVX_BAD_MAGIC; /* make sure no existing copies of refs can use ref again */
                if((vx_status)VX_SUCCESS != ownObjectFree(ref))
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to free memory of reference \n");
                }
            }
        }
        *pref = NULL;
    } else {
        VX_PRINT(VX_ZONE_ERROR,"Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

vx_reference ownCreateReference(vx_context context, vx_enum ref_type, vx_enum reftype, vx_reference scope)
{
    vx_reference ref = (vx_reference)ownObjectAlloc(ref_type);
    vx_status status = (vx_status)VX_SUCCESS;
    vx_bool is_add = (vx_bool)vx_true_e;

    if (ref != NULL)
    {
        status = ownInitReference(ref, context, ref_type, scope);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_REFERENCE_UBR002
<justification end> */
        if(status==(vx_status)VX_SUCCESS)
        {
            /* Setting it as void since return value 'ref count' is not used further */
            (void)ownIncrementReference(ref, reftype);

            is_add = ownAddReferenceToContext(context, ref);
            if(is_add == (vx_bool)vx_false_e)
            {
                VX_PRINT(VX_ZONE_ERROR, "Add reference to context failed\n");
                status = (vx_status)VX_ERROR_NO_RESOURCES;
            }
        }
/* LDRA_JUSTIFY_END */

        if(status!=(vx_status)VX_SUCCESS)
        {
            (void)ownObjectFree(ref);
            vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES, "Failed to add to resources table\n");
            VX_PRINT(VX_ZONE_ERROR, "Failed to add to resources table\n");
            ref = (vx_reference)ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
        }
    }
    else
    {
        vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES, "Failed to allocate reference object\n");
        VX_PRINT(VX_ZONE_ERROR, "Failed to allocate reference object\n");
        ref = (vx_reference)ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
    }
    return ref;
}



vx_bool ownIsValidSpecificReference(vx_reference ref, vx_enum ref_type)
{
    vx_bool ret = (vx_bool)vx_false_e;
    if (ref != NULL)
    {
        if ((ref->magic == TIVX_MAGIC) &&
            (ref->type == ref_type) &&
            (ownIsValidContext(ref->context) == (vx_bool)vx_true_e))
        {
            ret = (vx_bool)vx_true_e;
        }
    }
    return ret;
}

vx_bool ownIsValidType(vx_enum ref_type)
{
    vx_bool ret = (vx_bool)vx_false_e;
    if (ref_type <= (vx_enum)VX_TYPE_INVALID)
    {
        ret = (vx_bool)vx_false_e;
    }
    else if (TIVX_TYPE_IS_SCALAR(ref_type)) /* some scalar */
    {
        ret = (vx_bool)vx_true_e;
    }
    else if (TIVX_TYPE_IS_STRUCT(ref_type)) /* some struct */
    {
        ret = (vx_bool)vx_true_e;
    }
    else if (TIVX_TYPE_IS_OBJECT(ref_type)) /* some object */
    {
        ret = (vx_bool)vx_true_e;
    }
    else if (TIVX_TYPE_IS_TI_OBJECT(ref_type)) /* some object */
    {
        ret = (vx_bool)vx_true_e;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Type 0x%08x is invalid!\n");
    }
    return ret; /* otherwise, not a valid type */
}

vx_status ownReferenceLock(vx_reference ref)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;

    if(ref != NULL)
    {
        if(ref->lock != NULL)
        {
            status = tivxMutexLock(ref->lock);
        }
        else
        {
            if(ref->context != NULL)
            {
                status = tivxMutexLock(ref->context->base.lock);
            }
        }
    }

    return status;
}

vx_status ownReferenceUnlock(vx_reference ref)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;

    if(ref != NULL)
    {
        if(ref->lock != NULL)
        {
            status = tivxMutexUnlock(ref->lock);
        }
        else
        {
            if(ref->context != NULL)
            {
                status = tivxMutexUnlock(ref->context->base.lock);
            }
        }
    }

    return status;
}

void ownInitReferenceForDelay(vx_reference ref, vx_delay d, vx_int32 idx) {
    ref->delay=d;
    ref->delay_slot_index=idx;
}

vx_status ownReferenceAllocMem(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidReference(ref) == (vx_bool)vx_true_e)
    {
        if(ref->mem_alloc_callback != NULL)
        {
            status = ref->mem_alloc_callback(ref);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

void ownReferenceSetScope(vx_reference ref, vx_reference scope)
{
    if(ref != NULL)
    {
        ref->scope = scope;
        if(NULL != ref->obj_desc)
        {
            ref->obj_desc->scope_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
            if((NULL != scope) && (NULL != scope->obj_desc))
            {
                if ((ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
                        ||
                    (ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
                    )
                {
                    /* set scope_obj_desc_id in obj_desc only for composite objects like pyramid and object array */
                    ref->obj_desc->scope_obj_desc_id = scope->obj_desc->obj_desc_id;
                }
            }
        }
    }
}

uint16_t ownReferenceGetObjDescId(vx_reference ref)
{
    uint16_t obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;

    if (NULL != ref)
    {
        obj_desc_id = ref->obj_desc->obj_desc_id;
    }

    return (obj_desc_id);
}

vx_reference ownReferenceGetHandleFromObjDescId(uint16_t obj_desc_id)
{
    tivx_obj_desc_t *obj_desc = ownObjDescGet(obj_desc_id);
    vx_reference ref = NULL;

    if(obj_desc!=NULL)
    {
        ref = (vx_reference)(uintptr_t)obj_desc->host_ref;
    }

    return ref;
}

VX_API_ENTRY vx_status VX_API_CALL tivxSetReferenceAttribute(vx_reference ref, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((ownIsValidReference(ref) == (vx_bool)vx_false_e) &&
        (ownIsValidContext((vx_context)ref) == (vx_bool)vx_false_e))
    {

        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case (vx_enum)TIVX_REFERENCE_TIMESTAMP:
                if ((VX_CHECK_PARAM(ptr, size, vx_uint64, 0x3U) && (NULL != ref->obj_desc)))
                {
                    ref->obj_desc->timestamp = (vx_uint64)*(const vx_uint64 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "error setting reference timestamp\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_REFERENCE_INVALID:
                if ((VX_CHECK_PARAM(ptr, size, vx_bool, 0x3U) && (NULL != ref->obj_desc)))
                {
                    vx_bool is_ref_invalid;

                    is_ref_invalid = *(const vx_bool *)ptr;

                    if ((vx_bool)vx_true_e == is_ref_invalid)
                    {
                        tivxFlagBitSet(&ref->obj_desc->flags, TIVX_REF_FLAG_IS_INVALID);
                    }
                    else
                    {
                        tivxFlagBitClear(&ref->obj_desc->flags, TIVX_REF_FLAG_IS_INVALID);
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "error setting reference validity\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryReference(vx_reference ref, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* if it is not a reference and not a context */
    if ((ownIsValidReference(ref) == (vx_bool)vx_false_e) &&
        (ownIsValidContext((vx_context)ref) == (vx_bool)vx_false_e)) {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case (vx_enum)VX_REFERENCE_COUNT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = ref->external_count;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query reference count failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_REFERENCE_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = ref->type;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query reference type failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_REFERENCE_NAME:
                if (VX_CHECK_PARAM(ptr, size, vx_char*, 0x3U))
                {
                    *(vx_char**)ptr = &ref->name[0];
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query reference name failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_REFERENCE_ENQUEUE_COUNT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = ref->obj_desc->num_enqueues;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query reference enqueue count failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_REFERENCE_TIMESTAMP:
                if ((VX_CHECK_PARAM(ptr, size, vx_uint64, 0x3U) && (NULL != ref->obj_desc)))
                {
                    *(vx_uint64 *)ptr = ref->obj_desc->timestamp;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query reference timestamp failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_REFERENCE_INVALID:
                if (VX_CHECK_PARAM(ptr, size, vx_bool, 0x3U))
                {
                    vx_bool is_ref_invalid;

                    if (tivxFlagIsBitSet(ref->obj_desc->flags, TIVX_REF_FLAG_IS_INVALID) != (vx_bool)0U)
                    {
                        is_ref_invalid = (vx_bool)vx_true_e;
                    }
                    else
                    {
                        is_ref_invalid = (vx_bool)vx_false_e;
                    }
                    *(vx_bool*)ptr = is_ref_invalid;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query reference invalid flag failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_REFERENCE_BUFFER_IS_ALLOCATED:
                if (VX_CHECK_PARAM(ptr, size, vx_bool, 0x3U))
                {
                    *(vx_bool*)ptr = (vx_bool)ref->is_allocated;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query reference buffer allocated failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetReferenceName(vx_reference ref, const vx_char *name)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    if (ownIsValidReference(ref) != 0)
    {
        (void)snprintf(ref->name, VX_MAX_REFERENCE_NAME, "%s", name);
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseReference(vx_reference* ref_ptr)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    vx_reference ref;

    if (ref_ptr != NULL)
    {
        ref = *ref_ptr;
    }
    else
    {
        ref = NULL;
    }

    if (ownIsValidReference(ref) == (vx_bool)vx_true_e)
    {
        if(ref->release_callback != NULL)
        {
            status = ref->release_callback(ref_ptr);
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxRetainReference(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidReference(ref) == (vx_bool)vx_true_e)
    {
        /* Setting it as void since return value 'ref count' is not used further */
        (void)ownIncrementReference(ref, (vx_enum)VX_EXTERNAL);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxGetStatus(vx_reference ref)
{
    vx_status status = (vx_status)VX_FAILURE;

    if (ref == NULL)
    {
        /*! \internal probably ran out of handles or memory */
        VX_PRINT(VX_ZONE_ERROR, "Reference is NULL\n");
        status = (vx_status)VX_ERROR_NO_RESOURCES;
    }
    else if (ownIsValidContext((vx_context)ref) == (vx_bool)vx_true_e)
    {
        status = (vx_status)VX_SUCCESS;
    }
    else if (ownIsValidReference(ref) == (vx_bool)vx_true_e)
    {
        if (ref->type == (vx_enum)VX_TYPE_ERROR)
        {
            tivx_error_t *error = (tivx_error_t *)ref;
            status = error->status;
        }
        else
        {
            status = (vx_status)VX_SUCCESS;
        }
    }
    else
    {
        /* do nothing and return failure */
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxHint(vx_reference reference, vx_enum hint, const void* data, vx_size data_size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* reference param should be a valid OpenVX reference*/
    if ((ownIsValidContext((vx_context)reference) == (vx_bool)vx_false_e) && (ownIsValidReference(reference) == (vx_bool)vx_false_e))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Hint not supported\n");
        status = (vx_status)VX_ERROR_NOT_SUPPORTED;
    }

    return status;
}

VX_API_ENTRY vx_context VX_API_CALL vxGetContext(vx_reference reference)
{
    vx_context context = NULL;
    if (ownIsValidContext((vx_context)reference) == (vx_bool)vx_true_e)
    {
        context = (vx_context)reference;
    }
    else if (ownIsValidReference(reference) == (vx_bool)vx_true_e)
    {
        context = reference->context;
    }
    else
    {
        /* Do nothing as context is already initialized with null */
    }
    return context;
}

VX_API_ENTRY vx_bool VX_API_CALL tivxIsReferenceMetaFormatEqual(vx_reference ref1, vx_reference ref2)
{
    vx_meta_format  mf1;
    vx_meta_format  mf2;
    vx_status       status;
    vx_bool         boolStatus;

    mf1        = NULL;
    mf2        = NULL;
    status     = (vx_status)VX_SUCCESS;
    boolStatus = (vx_bool)vx_false_e;

    if ((ref1 == NULL) ||
        (ownIsValidReference(ref1) == (vx_bool)vx_false_e))
    {
        VX_PRINT(VX_ZONE_ERROR, "Reference1 is Invalid.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if ((ref2 == NULL) ||
             (ownIsValidReference(ref2) == (vx_bool)vx_false_e))
    {
        VX_PRINT(VX_ZONE_ERROR, "Reference2 is Invalid.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (ref1->type != ref2->type)
    {
        VX_PRINT(VX_ZONE_ERROR, "The type of the references do not match.\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        /* do nothing as references are already validated */
    }
    /* Create a meta format object. */
    if (status == (vx_status)VX_SUCCESS)
    {
        mf1 = ownCreateMetaFormat(ref1->context);

        if (mf1 == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to create meta format object.\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    /* Create a meta format object. */
    if (status == (vx_status)VX_SUCCESS)
    {
        mf2 = ownCreateMetaFormat(ref2->context);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_REFERENCE_UM004
<justification end> */
        if (mf2 == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to create meta format object.\n");
            status = (vx_status)VX_FAILURE;
        }
/* LDRA_JUSTIFY_END */
    }

    /* Set the ref1 in mf1. */
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vxSetMetaFormatFromReference(mf1, ref1);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_REFERENCE_UM005
<justification end> */
        if (status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "vxSetMetaFormatFromReference(ref1) failed.\n");
            status = (vx_status)VX_FAILURE;
        }
/* LDRA_JUSTIFY_END */
    }

    /* Set the ref2 in mf2. */
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vxSetMetaFormatFromReference(mf2, ref2);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_REFERENCE_UM006
<justification end> */
        if (status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "vxSetMetaFormatFromReference(ref2) failed.\n");
            status = (vx_status)VX_FAILURE;
        }
/* LDRA_JUSTIFY_END */
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        boolStatus = ownIsMetaFormatEqual(mf1, mf2, mf1->type);
    }

    /* Release the meta objects. */
    if (mf1 != NULL)
    {
        status = ownReleaseMetaFormat(&mf1);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_REFERENCE_UM007
<justification end> */
        if (status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to release a meta-format object.\n");
        }
/* LDRA_JUSTIFY_END */
    }

    if (mf2 != NULL)
    {
        status =  ownReleaseMetaFormat(&mf2);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_REFERENCE_UM008
<justification end> */
        if (status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to release a meta-format object.\n");
        }
/* LDRA_JUSTIFY_END */
    }

    return boolStatus;
}

vx_bool tivxIsReferenceVirtual(vx_reference ref)
{
    vx_bool ret = (vx_bool)vx_false_e;

    if (NULL != ref)
    {
        ret = ref->is_virtual;
    }

    return (ret);
}

vx_reference tivxGetReferenceParent(vx_reference child_ref)
{
    vx_reference ref = NULL;

    if (ownIsValidReference(child_ref) == (vx_bool)vx_true_e)
    {
        tivx_obj_desc_t *obj_desc =
            (tivx_obj_desc_t *)child_ref->obj_desc;

        if (obj_desc != NULL)
        {
            if ((vx_bool)vx_true_e == child_ref->is_array_element)
            {
                ref = ownReferenceGetHandleFromObjDescId(obj_desc->scope_obj_desc_id);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Provided child_ref->obj_desc is NULL.\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Provided child_ref is invalid.\n");
    }

    return ref;
}

vx_status tivxReferenceImportHandle(vx_reference ref, const void *addr[], const uint32_t size[], uint32_t num_entries)
{
    tivx_shared_mem_ptr_t  *mem_ptr;
    volatile uint32_t      *mem_size;
    uint64_t                shared_ptr[TIOVX_REF_MAX_NUM_MEM_ELEM] = {0};
    uint32_t                numMemElem, numPlanes, total_size = 0;
    uint32_t                numNulls;
    vx_status               status;
    vx_uint32               num_levels = 0;
    tivx_shared_mem_ptr_t  *mem_ptr_arr[TIOVX_REF_MAX_NUM_MEM_ELEM];
    volatile uint32_t      *mem_size_arr[TIOVX_REF_MAX_NUM_MEM_ELEM];
    volatile uint32_t       num_planes[TIOVX_REF_MAX_NUM_MEM_ELEM];
    uint32_t                i;
    uint32_t                j;

    status = (vx_status)VX_SUCCESS;

    /* Validate the arguments. */
    if ((ref == NULL) || (ownIsValidReference(ref) == (vx_bool)vx_false_e))
    {
        VX_PRINT(VX_ZONE_ERROR, "Reference is Invalid.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (num_entries == 0U)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "The parameter 'num_entries' must be non-zero.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (addr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'addr' is NULL.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (size == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'size' is NULL.\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
       /* do nothing as the arguments are already validated */;
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        numMemElem = 1;
        numPlanes  = 1;
        mem_ptr    = NULL;
        mem_size   = NULL;

        if (ref->type == (vx_enum)VX_TYPE_IMAGE)
        {
            /* status set to NULL due to preceding type check */
            vx_image image = vxCastRefAsImage(ref,NULL);

            if (NULL == image->parent)
            {
                tivx_obj_desc_image_t *obj_desc;

                obj_desc = (tivx_obj_desc_image_t *)ref->obj_desc;

                if (obj_desc == NULL)
                {
                    VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                    status = (vx_status)VX_FAILURE;
                }
                else
                {
                    numPlanes = obj_desc->planes;
                    for (i = 0; i < numPlanes; i++)
                    {
                        total_size += obj_desc->mem_size[i];
                    }
                    mem_ptr    = obj_desc->mem_ptr;
                    mem_size   = obj_desc->mem_size;
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Import to a subimage is not supported.\n");
                status = (vx_status)VX_FAILURE;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_TENSOR)
        {
            tivx_obj_desc_tensor_t *obj_desc;

            obj_desc = (tivx_obj_desc_tensor_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_USER_DATA_OBJECT)
        {
            tivx_obj_desc_user_data_object_t *obj_desc;

            obj_desc = (tivx_obj_desc_user_data_object_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_ARRAY)
        {
            tivx_obj_desc_array_t *obj_desc;

            obj_desc = (tivx_obj_desc_array_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_CONVOLUTION)
        {
            tivx_obj_desc_convolution_t *obj_desc;

            obj_desc = (tivx_obj_desc_convolution_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_MATRIX)
        {
            tivx_obj_desc_matrix_t *obj_desc;

            obj_desc = (tivx_obj_desc_matrix_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_DISTRIBUTION)
        {
            tivx_obj_desc_distribution_t *obj_desc;

            obj_desc = (tivx_obj_desc_distribution_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)TIVX_TYPE_RAW_IMAGE)
        {
            tivx_obj_desc_raw_image_t *obj_desc;

            obj_desc = (tivx_obj_desc_raw_image_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                numMemElem = obj_desc->params.num_exposures;
                mem_ptr    = obj_desc->mem_ptr;
                mem_size   = obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_PYRAMID)
        {
            tivx_obj_desc_pyramid_t    *obj_desc;
            tivx_obj_desc_image_t      *img_obj_desc;
            vx_pyramid                  pyramid;
            vx_reference                img_ref;

            obj_desc = (tivx_obj_desc_pyramid_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                num_levels = obj_desc->num_levels;
                /* status set to NULL due to preceding type check */
                pyramid = vxCastRefAsPyramid(ref,NULL);
                numMemElem = 0;

                for (i = 0; i < num_levels; i++)
                {
                    img_ref = vxCastRefFromImage(pyramid->img[i]);
                    img_obj_desc = (tivx_obj_desc_image_t *)img_ref->obj_desc;

                    if (img_obj_desc == NULL)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "'img_obj_desc' is NULL.\n");
                        status = (vx_status)VX_FAILURE;
                        break;
                    }
                    numPlanes = img_obj_desc->planes;

                    numMemElem++;
                    mem_ptr_arr[i]   = img_obj_desc->mem_ptr;
                    mem_size_arr[i]  = img_obj_desc->mem_size;
                    num_planes[i]    = img_obj_desc->planes;
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Unsupported type [%d].\n", ref->type);
            status = (vx_status)VX_FAILURE;
        }

        if ((status == (vx_status)VX_SUCCESS) &&
            (numMemElem > num_entries))
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "num_entries [%d] less than num handles expected [%d].\n",
                     num_entries, numMemElem);

            status = (vx_status)VX_FAILURE;
        }

        /* Validate the addr entries. */
        numNulls = 0;

        if (status == (vx_status)VX_SUCCESS)
        {
            for (i = 0; i < numMemElem; i++)
            {
                if (addr[i] != NULL)
                {
                    /* Check if the memory allocated can be translated. */
                    shared_ptr[i] =
                        tivxMemHost2SharedPtr((uint64_t)(uintptr_t)addr[i],
                                              (vx_enum)TIVX_MEM_EXTERNAL);

                    if (shared_ptr[i] == 0U)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "addr[%d] is INVALID.\n", i);
                        status = (vx_status)VX_FAILURE;
                        break;
                    }
                }
                else
                {
                    numNulls++;
                }
            }

            if ((status == (vx_status)VX_SUCCESS) &&
                (numNulls != 0U) &&
                (numNulls != numMemElem))
            {
                VX_PRINT(VX_ZONE_ERROR,
                         "addr[] has a mix of NULL and non-NULL entries.\n");
                status = (vx_status)VX_FAILURE;
            }
        }

        /* Validate the sizes of the handles. Do this only if we are not
         * importing NULLs.
         */
        if ((status == (vx_status)VX_SUCCESS) && (numNulls == 0U))
        {
            if (ref->type == (vx_enum)VX_TYPE_PYRAMID)
            {
                for (i = 0; i < num_levels; i++)
                {
                    total_size = 0;
                    mem_size = mem_size_arr[i];

                    for (j = 0; j < num_planes[i]; j++)
                    {
                        total_size += mem_size[j];
                    }

                    if (total_size != size[i])
                    {
                        VX_PRINT(VX_ZONE_ERROR,
                                 "[Entry %d] Memory size mis-match: Expecting [%d] "
                                 "but given [%d]\n",
                                 i, total_size, size[i]);

                        status = (vx_status)VX_FAILURE;
                    }
                }
            }
            else if (ref->type == (vx_enum)VX_TYPE_IMAGE)
            {
                if (total_size != size[0])
                {
                    VX_PRINT(VX_ZONE_ERROR,
                             "[Entry %d] Memory size mis-match: Expecting [%d] "
                             "but given [%d]\n",
                             i, total_size, size[0]);

                    status = (vx_status)VX_FAILURE;
                }
            }
            else
            {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_REFERENCE_UBR003
<justification end> */
                if (mem_size != NULL)
                {
                    for (i = 0; i < numMemElem; i++)
                    {
                        if (mem_size[i] != size[i])
                        {
                            VX_PRINT(VX_ZONE_ERROR,
                                    "[Entry %d] Memory size mis-match: Expecting [%d] "
                                    "but given [%d]\n",
                                    i, mem_size[i], size[i]);

                            status = (vx_status)VX_FAILURE;
                        }
                    }
                }
/* LDRA_JUSTIFY_END */
            }
        }

        if (status == (vx_status)VX_SUCCESS)
        {
            /* Issue an error if the number of handles passed is more than
             * what is needed.
             */
            if (numMemElem < num_entries)
            {
                VX_PRINT(VX_ZONE_ERROR,
                         "The value 'num_entries' exceeds the number of "
                         "handles needed.\n");
                status = (vx_status)VX_FAILURE;
            }

        }

        if (status == (vx_status)VX_SUCCESS)
        {
            /* Update the object. */
            if (ref->type == (vx_enum)VX_TYPE_PYRAMID)
            {
                for (i = 0; i < num_levels; i++)
                {
                    mem_ptr  = mem_ptr_arr[i];
                    mem_size = mem_size_arr[i];

                    if (mem_ptr[0].host_ptr != (uint64_t)0)
                    {
                        VX_PRINT(VX_ZONE_INFO,
                                 "Non-NULL handle detected. Overwriting.\n");
                    }

                    if (NULL != addr[i])
                    {
                        ref->is_allocated = (vx_bool)vx_true_e;
                    }
                    else
                    {
                        ref->is_allocated = (vx_bool)vx_false_e;
                    }

                    mem_ptr[0].host_ptr        = (uint64_t)(uintptr_t)addr[i];
                    mem_ptr[0].shared_ptr      = shared_ptr[i];
                    mem_ptr[0].mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;

                    for (j = 1; j < num_planes[i]; j++)
                    {
                        if (mem_ptr[0].host_ptr != (uint64_t)0)
                        {
                            mem_ptr[j].host_ptr        = mem_ptr[(int32_t)j-1].host_ptr + mem_size[(int32_t)j-1];
                            mem_ptr[j].shared_ptr      = tivxMemHost2SharedPtr(
                                    mem_ptr[j].host_ptr,
                                    (vx_enum)TIVX_MEM_EXTERNAL);
                        }
                        else
                        {
                            mem_ptr[j].host_ptr        = (uint64_t)0;
                            mem_ptr[j].shared_ptr      = (uint64_t)0;
                        }

                        mem_ptr[j].mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;
                    } /* for (j = 1; j < num_planes[i]; j++) */

                } /* for (i = 0; i < num_levels; i++) */
            }
            else
            {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_REFERENCE_UBR004
<justification end> */
                if (mem_ptr != NULL)
                {
                    for (i = 0; i < numMemElem; i++)
                    {
                        mem_ptr[i].mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;

                        if (mem_ptr[i].host_ptr != (uint64_t)0)
                        {
                            VX_PRINT(VX_ZONE_INFO,
                                    "Non-NULL handle detected. Overwriting.\n");
                        }

                        if (NULL != addr[i])
                        {
                            ref->is_allocated = (vx_bool)vx_true_e;
                        }
                        else
                        {
                            ref->is_allocated = (vx_bool)vx_false_e;
                        }

                        mem_ptr[i].host_ptr   = (uint64_t)(uintptr_t)addr[i];
                        mem_ptr[i].shared_ptr = shared_ptr[i];
                        if (ref->type == (vx_enum)VX_TYPE_IMAGE)
                        {
                            for (j = 1; j < numPlanes; j++)
                            {
                                if (mem_ptr[0].host_ptr != (uint64_t)0)
                                {
                                    mem_ptr[j].host_ptr        = mem_ptr[(int32_t)j-1].host_ptr + mem_size[(int32_t)j-1];
                                    mem_ptr[j].shared_ptr      = tivxMemHost2SharedPtr(
                                            mem_ptr[j].host_ptr,
                                            (vx_enum)TIVX_MEM_EXTERNAL);
                                }
                                else
                                {
                                    mem_ptr[j].host_ptr        = (uint64_t)0;
                                    mem_ptr[j].shared_ptr      = (uint64_t)0;
                                }
                            } /* for (j = 1; j < numPlanes; j++) */
                        } /* for (i = 0; i < numMemElem; i++) */
                    }
                }
/* LDRA_JUSTIFY_END */
            }
        }

        if((status == (vx_status)VX_SUCCESS) &&
           (ref->type == (vx_enum)TIVX_TYPE_RAW_IMAGE))
        {
            status = ownDeriveRawImageBufferPointers(ref);
        }
    }

    return status;
}

vx_status tivxReferenceExportHandle(const vx_reference ref, void *addr[], uint32_t size[], uint32_t max_num_entries, uint32_t *num_entries)
{
    tivx_shared_mem_ptr_t  *mem_ptr;
    volatile uint32_t      *mem_size = NULL;
    uint32_t                numMemElem, total_size = 0;
    uint32_t                i,j;
    vx_status               status;

    status = (vx_status)VX_SUCCESS;

    /* Validate the arguments. */
    if ((ref == NULL) || (ownIsValidReference(ref) == (vx_bool)vx_false_e))
    {
        VX_PRINT(VX_ZONE_ERROR, "Reference is Invalid.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (addr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'addr' is NULL.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (size == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'size' is NULL.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (max_num_entries == 0U)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'max_num_entries' is 0.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (num_entries == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'num_entries' is NULL.\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        /* do nothing as the arguments are already validated */;
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        numMemElem = 1;
        mem_ptr    = NULL;

        if (ref->type == (vx_enum)VX_TYPE_IMAGE)
        {
            /* status set to NULL due to preceding type check */
            vx_image image = vxCastRefAsImage(ref,NULL);

            if (NULL == image->parent)
            {
                tivx_obj_desc_image_t *obj_desc;

                obj_desc = (tivx_obj_desc_image_t *)ref->obj_desc;

                if (obj_desc == NULL)
                {
                    VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                    status = (vx_status)VX_FAILURE;
                }
                else
                {
                    for (i = 0; i < obj_desc->planes; i++)
                    {
                        total_size += obj_desc->mem_size[i];
                    }
                    mem_ptr    = obj_desc->mem_ptr;
                    mem_size   = &total_size;
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Export from a subimage is not supported.\n");
                status = (vx_status)VX_FAILURE;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_TENSOR)
        {
            tivx_obj_desc_tensor_t *obj_desc;

            obj_desc = (tivx_obj_desc_tensor_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_USER_DATA_OBJECT)
        {
            tivx_obj_desc_user_data_object_t *obj_desc;

            obj_desc = (tivx_obj_desc_user_data_object_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_ARRAY)
        {
            tivx_obj_desc_array_t *obj_desc;

            obj_desc = (tivx_obj_desc_array_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_CONVOLUTION)
        {
            tivx_obj_desc_convolution_t *obj_desc;

            obj_desc = (tivx_obj_desc_convolution_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_MATRIX)
        {
            tivx_obj_desc_matrix_t *obj_desc;

            obj_desc = (tivx_obj_desc_matrix_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_DISTRIBUTION)
        {
            tivx_obj_desc_distribution_t *obj_desc;

            obj_desc = (tivx_obj_desc_distribution_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)TIVX_TYPE_RAW_IMAGE)
        {
            tivx_obj_desc_raw_image_t *obj_desc;

            obj_desc = (tivx_obj_desc_raw_image_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                numMemElem = obj_desc->params.num_exposures;
                mem_ptr    = obj_desc->mem_ptr;
                mem_size   = obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_PYRAMID)
        {
            tivx_obj_desc_pyramid_t    *obj_desc;
            tivx_obj_desc_image_t      *img_obj_desc;
            vx_uint32                   num_levels;
            vx_pyramid                  pyramid;
            vx_reference                img_ref;

            obj_desc = (tivx_obj_desc_pyramid_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                num_levels = obj_desc->num_levels;
                /* status set to NULL due to preceding type check */
                pyramid = vxCastRefAsPyramid(ref,NULL);

                if (num_levels > max_num_entries)
                {
                    /* Having more handles than needed is OK but not the
                     * other way.
                     */
                    VX_PRINT(VX_ZONE_ERROR,
                             "max_num_entries [%d] less than num "
                             "levels of pyramid [%d].\n",
                              max_num_entries, num_levels);
                    status = (vx_status)VX_FAILURE;
                }
                else
                {
                    for (i = 0; i < num_levels; i++)
                    {
                        img_ref = vxCastRefFromImage(pyramid->img[i]);
                        img_obj_desc = (tivx_obj_desc_image_t *)img_ref->obj_desc;
                        total_size   = 0;

                        if (img_obj_desc == NULL)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "'img_obj_desc' is NULL.\n");
                            status = (vx_status)VX_FAILURE;
                            break;
                        }
                        for (j = 0; j < img_obj_desc->planes; j++)
                        {
                            total_size += img_obj_desc->mem_size[j];
                        }

                        addr[i] = (void *)
                            (uintptr_t)img_obj_desc->mem_ptr[0].host_ptr;
                        size[i] = total_size;

                    } /* for (i = 0; i < num_levels; i++) */

                    /* Update the entry count. */
                    *num_entries = num_levels;
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Unsupported type [%d].\n", ref->type);
            status = (vx_status)VX_FAILURE;
        }

        if ((status == (vx_status)VX_SUCCESS) &&
            (ref->type != (vx_enum)VX_TYPE_PYRAMID))
        {
            if ((mem_ptr != NULL) /* TIOVX-1926- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_REFERENCE_UBR005 */
            && (mem_size != NULL)) /* TIOVX-1926- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_REFERENCE_UBR006 */
            {
                for (i = 0; i < numMemElem; i++)
                {
                    addr[i] = (void *)(uintptr_t)mem_ptr[i].host_ptr;
                    size[i] = mem_size[i];
                }
            }

            /* Update the entry count. */
            *num_entries = numMemElem;

        } /* if (status == (vx_status)VX_SUCCESS) */

    } /* if (status == (vx_status)VX_SUCCESS) */

    return status;
}
  
VX_API_ENTRY vx_user_data_object vxGetSupplementaryUserDataObject(vx_reference ref, const vx_char * type_name, vx_status *status)
{
    vx_user_data_object retref = NULL;
    vx_status local_status;
    vx_status *statusptr = (NULL != status) ? status : &local_status;

    *statusptr = (vx_status)VX_SUCCESS;
    if ( (vx_bool)vx_false_e == ownIsValidReference(ref))
    {
        *statusptr = (vx_status)VX_ERROR_INVALID_REFERENCE;
        retref = (vx_user_data_object)ownGetErrorObject(ownGetContext(), (vx_status)VX_ERROR_INVALID_REFERENCE);
    }
    else if (((vx_bool)vx_true_e == ref->is_virtual) &&
            ((vx_bool)vx_false_e == ref->is_accessible))
    {
        /* Can't get supplementary data from an inaccessible object*/
        VX_PRINT(VX_ZONE_WARNING, "Attempt to get supplementary data from virtual object\n");
        *statusptr = (vx_status)VX_ERROR_OPTIMIZED_AWAY;
        retref = (vx_user_data_object)ownGetErrorObject(ref->context, (vx_status)VX_ERROR_OPTIMIZED_AWAY);
    }
    else if ((vx_enum)VX_TYPE_CONTEXT == ref->type)
    {
        /* Specifically avoid issues later */
        *statusptr = (vx_status)VX_ERROR_NOT_SUPPORTED;
        retref = (vx_user_data_object)ownGetErrorObject((vx_context)ref, (vx_status)VX_ERROR_NOT_SUPPORTED);
    }
    else if (NULL == ref->obj_desc)
    {
        /* objects without an object descriptor are not supported */
        *statusptr = (vx_status)VX_ERROR_NOT_SUPPORTED;
        retref = (vx_user_data_object)ownGetErrorObject(ref->context, (vx_status)VX_ERROR_NOT_SUPPORTED);
    }
    else
    {
        vx_reference new_ref = ref;
        /* Look for parent data if this not in the kernel function or it is an input */
        if (((vx_bool)vx_false_e == tivxFlagIsBitSet(ref->obj_desc->flags, TIVX_REF_FLAG_IS_IN_KERNEL)) ||
            ((vx_bool)vx_true_e == tivxFlagIsBitSet(ref->obj_desc->flags, TIVX_REF_FLAG_IS_INPUT)))
        {
            while (NULL == new_ref->supplementary_data)
            {
                vx_reference parent = NULL;
                if ((vx_enum)VX_TYPE_IMAGE == new_ref->type)
                {
                    parent = (vx_reference)((vx_image)new_ref)->parent;
                }
                else if ((vx_enum)VX_TYPE_TENSOR == new_ref->type)
                {
                    parent = (vx_reference)((vx_tensor)new_ref)->parent;
                }
                else
                {
                    /* Empty else as required for MISRA-C */
                }

                if (NULL != parent)
                {
                    new_ref = parent;
                }
                else
                {
                    break;
                }
            }
        }
        if (NULL == new_ref->supplementary_data)
        {
            *statusptr = (vx_status)VX_FAILURE;
            retref = (vx_user_data_object)ownGetErrorObject(ref->context, (vx_status)VX_FAILURE);
        }
        else if ((NULL != type_name) &&
                 (tivx_obj_desc_strncmp(((tivx_obj_desc_user_data_object_t *)new_ref->supplementary_data->base.obj_desc)->type_name, (volatile void *)type_name, VX_MAX_REFERENCE_NAME) != 0)
                )
        {
            char suppl_data_type_name[VX_MAX_REFERENCE_NAME] = {0};
            tivx_obj_desc_strncpy(suppl_data_type_name, ((tivx_obj_desc_user_data_object_t *)new_ref->supplementary_data->base.obj_desc)->type_name, VX_MAX_REFERENCE_NAME);
            VX_PRINT(VX_ZONE_ERROR, "Type mismatch for supplementary data. Expected user type \"%s\"; got \"%s\"\n", type_name, suppl_data_type_name);
            *statusptr = (vx_status)VX_ERROR_INVALID_TYPE;
            retref = (vx_user_data_object)ownGetErrorObject(ref->context, (vx_status)VX_ERROR_INVALID_TYPE);
        }
        else if ((ref == new_ref) ||
                ((vx_bool)vx_true_e == tivxFlagIsBitSet(ref->obj_desc->flags, TIVX_REF_FLAG_IS_IN_KERNEL))
                )
        {
            retref = new_ref->supplementary_data;
            *statusptr = vxRetainReference(&retref->base);
        }
        else
        {
            retref = ownCreateReadOnlyUserDataObject(new_ref->supplementary_data);

            if (vxGetStatus((vx_reference)retref) == (vx_status)VX_SUCCESS)
            {
                retref->owner = ref;
            }
            else
            {
                *statusptr = (vx_status)VX_ERROR_NO_RESOURCES;
            }
        }
    }
    return retref;
}

VX_API_ENTRY vx_status vxSetSupplementaryUserDataObject(vx_reference destination, const vx_user_data_object source)
{
    vx_status status;
    if ((vx_bool)vx_true_e == ownIsValidSpecificReference(&source->base, (vx_enum)VX_TYPE_USER_DATA_OBJECT))
    {
        vx_uint32 size = ((tivx_obj_desc_user_data_object_t *)source->base.obj_desc)->valid_mem_size;
        status = vxExtendSupplementaryUserDataObject(destination, source, NULL, size, size);
    }
    else
    {
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status vxExtendSupplementaryUserDataObject(vx_reference destination, const vx_user_data_object source, const void *user_data, vx_uint32 source_bytes, vx_uint32 user_bytes)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 l_source_bytes, l_user_bytes;
    l_source_bytes = source_bytes;
    l_user_bytes = user_bytes;

    if (((vx_bool)vx_false_e == ownIsValidReference(destination)) ||
        ((NULL == destination->supplementary_data) &&
        ((vx_bool)vx_false_e == ownIsValidSpecificReference(&source->base, (vx_enum)VX_TYPE_USER_DATA_OBJECT))))
    {
        /* Both source and destination must be valid */
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else if ((NULL != source) &&
             (NULL != source->base.supplementary_data))
    {
        /* Supplementary data cannot have supplementary data */
        status = (vx_status)VX_ERROR_NOT_SUPPORTED;
    }
    else if ((vx_enum)VX_TYPE_CONTEXT == destination->type)
    {
        /* Specifically avoid issues later */
        status = (vx_status)VX_ERROR_NOT_SUPPORTED;
    }
    else if (NULL == destination->obj_desc)
    {
        /* Objects without object descriptors are not supported */
        status = (vx_status)VX_ERROR_NOT_SUPPORTED;
    }
    else if (((vx_enum)VX_TYPE_USER_DATA_OBJECT == destination->type) &&
            (NULL != vxCastRefAsUserDataObject(destination, &status)->owner))
    {
        /* Destination cannot be supplementary data of another object */
        status = (vx_status)VX_ERROR_NOT_SUPPORTED;
    }
    else if ((((vx_bool)vx_true_e == destination->is_virtual) &&
             ((vx_bool)vx_false_e == destination->is_accessible)) ||
             ((NULL != source) &&
             ((vx_bool)vx_true_e == source->base.is_virtual) &&
             ((vx_bool)vx_false_e == source->base.is_accessible))) /* TIOVX_CODE_COVERAGE_REFERENCE_UM012 */
    {
        /* Cannot access virtual objects outside of kernel functions */
        status = (vx_status)VX_ERROR_OPTIMIZED_AWAY;
    }
    else if (source == user_data)
    {
        /* Both user_data and source may not be NULL */
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        if (NULL == destination->supplementary_data)
        {
            /* We can't create supplementary data for an object associated with a verified graph. This means
               any object connected to a node in a verified graph, or a possible parameter to a verified graph.
               We assume that this is set by the verify function somehow.
            */
            if (((vx_bool)vx_true_e == tivxFlagIsBitSet(destination->obj_desc->flags, TIVX_REF_FLAG_IS_IN_GRAPH)) || (NULL == source))
            {
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                /* We must create a new supplementary data object and initialise it with the data from source*/
                static vx_enum supported_type_table[] =
                {
                    (vx_enum)VX_TYPE_LUT,
                    (vx_enum)VX_TYPE_DISTRIBUTION,
                    (vx_enum)VX_TYPE_PYRAMID,
                    (vx_enum)VX_TYPE_THRESHOLD,
                    (vx_enum)VX_TYPE_MATRIX,
                    (vx_enum)VX_TYPE_CONVOLUTION,
                    (vx_enum)VX_TYPE_SCALAR,
                    (vx_enum)VX_TYPE_ARRAY,
                    (vx_enum)VX_TYPE_IMAGE,
                    (vx_enum)VX_TYPE_REMAP,
                    (vx_enum)VX_TYPE_OBJECT_ARRAY,
                    (vx_enum)VX_TYPE_TENSOR,
                    (vx_enum)VX_TYPE_USER_DATA_OBJECT,
                    (vx_enum)TIVX_TYPE_RAW_IMAGE
                };
                vx_uint32 i;
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                for (i = 0;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_REFERENCE_UM013
<justification end> */
                    i < dimof(supported_type_table);
/* LDRA_JUSTIFY_END */
                    ++i)
                {
                    if (destination->type == supported_type_table[i])
                    {
                        char type_name[VX_MAX_REFERENCE_NAME] = {0};
                        tivx_obj_desc_strncpy(type_name, ((tivx_obj_desc_user_data_object_t *)source->base.obj_desc)->type_name, VX_MAX_REFERENCE_NAME);
                        vx_size size = ((tivx_obj_desc_user_data_object_t *)source->base.obj_desc)->mem_size;
                        if ((l_source_bytes > size) ||
                            (l_user_bytes > size))
                        {
                            status = (vx_status)VX_ERROR_INVALID_VALUE;
                        }
                        else
                        {
                            vx_user_data_object supp = ownCreateUserDataObject(destination->context, type_name, size, NULL);
                            status = vxGetStatus(&supp->base);
                            if ((vx_status)VX_SUCCESS == status)
                            {
                                destination->supplementary_data = supp;
                                destination->obj_desc->supp_data_id = supp->base.obj_desc->obj_desc_id;
                                supp->owner = destination;
                            }
                        }
                        break;
                    }
                }
            }
        }
        /* Make sure both source and destination supplementary data have memory allocated */
        if((vx_status)VX_SUCCESS == status)
        {
            if(NULL != source)
            {
                status = ownAllocReferenceBufferGeneric(&source->base);
            }
        }
        if ((vx_status)VX_SUCCESS == status)
        {
            status = ownAllocReferenceBufferGeneric(&destination->supplementary_data->base);
        }        
        if ((vx_status)VX_SUCCESS == status)
        {
            /* Copy data from source & user data if we can */
            tivx_obj_desc_user_data_object_t * src_obj_desc = NULL;
            if (NULL != source)
            {
                src_obj_desc = (tivx_obj_desc_user_data_object_t *)source->base.obj_desc;
            }
            tivx_obj_desc_user_data_object_t * dst_obj_desc = (tivx_obj_desc_user_data_object_t *)destination->supplementary_data->base.obj_desc;
            if ((l_source_bytes > dst_obj_desc->mem_size) ||
                (l_user_bytes > dst_obj_desc->mem_size))
            {
                status = (vx_status)VX_ERROR_INVALID_VALUE;
            }
            else if ((NULL != src_obj_desc) &&
                ((dst_obj_desc->mem_size != src_obj_desc->mem_size) ||
                (tivx_obj_desc_strncmp(&dst_obj_desc->type_name[0], &src_obj_desc->type_name[0], (uint32_t)sizeof(src_obj_desc->type_name)) != 0)))
            {
                status = (vx_status)VX_ERROR_INVALID_TYPE;
            }
            else
            {
                vx_uint32 dest_bytes    = 0u;
                vx_uint32 source_offset = 0u;
                vx_uint32 user_offset   = 0u;
                if (l_source_bytes > l_user_bytes)
                {
                    dest_bytes = l_source_bytes;
                    l_source_bytes = dest_bytes - l_user_bytes;
                    source_offset = l_user_bytes;
                    user_offset = 0;
                }
                else
                {
                    dest_bytes = l_user_bytes;
                    l_user_bytes = dest_bytes - l_source_bytes;
                    user_offset = l_source_bytes;
                    if (NULL != src_obj_desc)
                    {
                        if (src_obj_desc->valid_mem_size < l_source_bytes)
                        {
                            l_source_bytes = src_obj_desc->valid_mem_size;
                        }
                    }
                    source_offset = 0;
                }

                status = ownReferenceLock(destination);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_REFERENCE_UM014
<justification end> */
                if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
                {
                    tivxCheckStatus(&status, tivxMemBufferMap((void *)(uintptr_t)dst_obj_desc->mem_ptr.host_ptr, dest_bytes, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_REFERENCE_UM015
<justification end> */
                    if ((vx_status)VX_SUCCESS == status)
                    {
/* LDRA_JUSTIFY_END */
                        if ((NULL != src_obj_desc) && (0U != l_source_bytes))
                        {
                            tivxCheckStatus(&status, tivxMemBufferMap((void *)(uintptr_t)(src_obj_desc->mem_ptr.host_ptr + source_offset), l_source_bytes, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_REFERENCE_UM015
<justification end> */
                            if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
                            {
                                tivx_obj_desc_memcpy((void *)(uintptr_t)(dst_obj_desc->mem_ptr.host_ptr + source_offset), (void *)(uintptr_t)(src_obj_desc->mem_ptr.host_ptr + source_offset), l_source_bytes);
                                tivxCheckStatus(&status, tivxMemBufferUnmap((void *)(uintptr_t)(src_obj_desc->mem_ptr.host_ptr + source_offset), l_source_bytes, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
                            }
                        }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_REFERENCE_UM015
<justification end> */
                        if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
                        {
                            if ((NULL != user_data) && (0U != l_user_bytes))
                            {
                                tivx_obj_desc_memcpy((void *)(uintptr_t)(dst_obj_desc->mem_ptr.host_ptr + user_offset), (void *)((uintptr_t)user_data + user_offset), l_user_bytes);
                            }
                            if (dest_bytes > dst_obj_desc->valid_mem_size)
                            {
                                dst_obj_desc->valid_mem_size = dest_bytes;
                            }
                        }
                        tivxCheckStatus(&status, tivxMemBufferUnmap((void *)(uintptr_t)dst_obj_desc->mem_ptr.host_ptr, dest_bytes, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
                    }
                    (void)ownReferenceUnlock(destination);
                }
            }
        }
    }
    return status;
}
