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

static vx_status ownDestructMatrix(vx_reference ref);
static vx_status ownAllocMatrixBuffer(vx_reference ref);

vx_matrix VX_API_CALL vxCreateMatrix(
    vx_context context, vx_enum data_type, vx_size columns, vx_size rows)
{
    vx_matrix matrix = NULL;
    vx_size dim = 0U;
    tivx_obj_desc_matrix_t *obj_desc = NULL;

    if(ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        if ((data_type == (vx_enum)VX_TYPE_INT8) || (data_type == (vx_enum)VX_TYPE_UINT8))
        {
            dim = sizeof(vx_uint8);
        }
        else if ((data_type == (vx_enum)VX_TYPE_INT16) ||
                 (data_type == (vx_enum)VX_TYPE_UINT16))
        {
            dim = sizeof(vx_uint16);
        }
        else if ((data_type == (vx_enum)VX_TYPE_INT32) ||
                 (data_type == (vx_enum)VX_TYPE_UINT32) ||
                 (data_type == (vx_enum)VX_TYPE_FLOAT32))
        {
            dim = sizeof(vx_uint32);
        }
        else if ((data_type == (vx_enum)VX_TYPE_INT64) ||
                 (data_type == (vx_enum)VX_TYPE_UINT64) ||
                 (data_type == (vx_enum)VX_TYPE_FLOAT64))
        {
            dim = sizeof(vx_uint64);
        }
        else
        {
            dim = 0U;
        }

        if ((rows != 0U) && (columns != 0U) && (dim != 0UL))
        {
            matrix = (vx_matrix)ownCreateReference(context, (vx_enum)VX_TYPE_MATRIX,
                (vx_enum)VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)matrix) == (vx_status)VX_SUCCESS) &&
                (matrix->base.type == (vx_enum)VX_TYPE_MATRIX))
            {
                /* assign refernce type specific callback's */
                matrix->base.destructor_callback = &ownDestructMatrix;
                matrix->base.mem_alloc_callback = &ownAllocMatrixBuffer;
                matrix->base.release_callback =
                    (tivx_reference_release_callback_f)&vxReleaseMatrix;

                obj_desc = (tivx_obj_desc_matrix_t*)ownObjDescAlloc(
                    (vx_enum)TIVX_OBJ_DESC_MATRIX, (vx_reference)matrix);
                if(obj_desc==NULL)
                {
                    vxReleaseMatrix(&matrix);

                    vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                        "Could not allocate matrix object descriptor\n");
                    matrix = (vx_matrix)ownGetErrorObject(
                        context, (vx_status)VX_ERROR_NO_RESOURCES);
                    VX_PRINT(VX_ZONE_ERROR, "Could not allocate matrix object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                    VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in source/platform/psdk_j7/common/soc/tivx_platform_psdk_<soc>.h\n");
                }
                else
                {
                    obj_desc->data_type = data_type;
                    obj_desc->columns = (uint32_t)columns;
                    obj_desc->rows = (uint32_t)rows;
                    obj_desc->origin_x = (uint32_t)columns/2U;
                    obj_desc->origin_y = (uint32_t)rows/2U;
                    obj_desc->pattern = (vx_enum)VX_PATTERN_OTHER;
                    obj_desc->mem_size = (uint32_t)columns*(uint32_t)rows*(uint32_t)dim;
                    obj_desc->mem_ptr.host_ptr = (uint64_t)(uintptr_t)NULL;
                    obj_desc->mem_ptr.shared_ptr = (uint64_t)(uintptr_t)NULL;
                    obj_desc->mem_ptr.mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;
                    matrix->base.obj_desc = (tivx_obj_desc_t *)obj_desc;
                }
            }
        }
    }

    return (matrix);
}

vx_matrix VX_API_CALL vxCreateMatrixFromPattern(
    vx_context context, vx_enum pattern, vx_size columns, vx_size rows)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_matrix matrix = NULL;
    vx_size dim = 0U, i, j;
    vx_uint8 *pTempDataPtr;
    tivx_obj_desc_matrix_t *obj_desc = NULL;

    /* Check for errors */
    if(ownIsValidContext(context) != (vx_bool)vx_true_e)
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid context\n");
        status = (vx_status)VX_FAILURE;
    }

    if (rows == 0U)
    {
        VX_PRINT(VX_ZONE_ERROR, "rows value is equal to zero\n");
        status = (vx_status)VX_FAILURE;
    }

    if (columns == 0U)
    {
        VX_PRINT(VX_ZONE_ERROR, "columns value is equal to zero\n");
        status = (vx_status)VX_FAILURE;
    }

    if (((vx_enum)VX_PATTERN_BOX != pattern) && ((vx_enum)VX_PATTERN_CROSS != pattern) &&
             ((vx_enum)VX_PATTERN_OTHER != pattern) && ((vx_enum)VX_PATTERN_DISK != pattern))
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid pattern value\n");
        status = (vx_status)VX_FAILURE;
    }

    /* For Cross pattern, rows and columns must be odd */
    if (((vx_enum)VX_PATTERN_CROSS == pattern) && (((rows%2U) == 0U) || ((columns%2U) == 0U)))
    {
        VX_PRINT(VX_ZONE_ERROR, "cross pattern rows and columns are not odd\n");
        status = (vx_status)VX_FAILURE;
    }

    /* For Disk pattern, rows and columns must be equal */
    if (((vx_enum)VX_PATTERN_DISK == pattern) && ( rows != columns ))
    {
        VX_PRINT(VX_ZONE_ERROR, "disk pattern rows and columns are not equal\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        dim = sizeof(vx_uint8);

        matrix = (vx_matrix)ownCreateReference(context, (vx_enum)VX_TYPE_MATRIX,
            (vx_enum)VX_EXTERNAL, &context->base);

        if ((vxGetStatus((vx_reference)matrix) == (vx_status)VX_SUCCESS) &&
            (matrix->base.type == (vx_enum)VX_TYPE_MATRIX))
        {
            /* assign refernce type specific callback's */
            matrix->base.destructor_callback = &ownDestructMatrix;
            matrix->base.mem_alloc_callback = &ownAllocMatrixBuffer;
            matrix->base.release_callback =
                (tivx_reference_release_callback_f)&vxReleaseMatrix;

            obj_desc = (tivx_obj_desc_matrix_t*)ownObjDescAlloc(
                (vx_enum)TIVX_OBJ_DESC_MATRIX, (vx_reference)matrix);
            if(obj_desc==NULL)
            {
                vxReleaseMatrix(&matrix);

                vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                    "Could not allocate matrix object descriptor\n");
                matrix = (vx_matrix)ownGetErrorObject(
                    context, (vx_status)VX_ERROR_NO_RESOURCES);
                VX_PRINT(VX_ZONE_ERROR, "Could not allocate matrix object descriptor\n");
                VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in source/platform/psdk_j7/common/soc/tivx_platform_psdk_<soc>.h\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                /* Initialize descriptor object */
                obj_desc->data_type = (vx_enum)VX_TYPE_UINT8;
                obj_desc->columns = (uint32_t)columns;
                obj_desc->rows = (uint32_t)rows;
                obj_desc->origin_x = (uint32_t)columns/2U;
                obj_desc->origin_y = (uint32_t)rows/2U;
                obj_desc->pattern = pattern;
                obj_desc->mem_size = (uint32_t)columns*(uint32_t)rows*(uint32_t)dim;
                obj_desc->mem_ptr.mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;
                matrix->base.obj_desc = (tivx_obj_desc_t *)obj_desc;

                obj_desc->mem_ptr.host_ptr = (uint64_t)(uintptr_t)NULL;
                obj_desc->mem_ptr.shared_ptr = (uint64_t)(uintptr_t)NULL;

                /* Allocate memory for matrix since matrix need to be
                   filled up with a pattern  */
                status = ownAllocMatrixBuffer(&matrix->base);

                if ((vx_status)VX_SUCCESS != status)
                {
                    /* Free up memory allocated for matrix */
                    ownDestructMatrix(&matrix->base);

                    /* Release matrix */
                    vxReleaseMatrix(&matrix);

                    vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                        "Could not allocate matrix object descriptor\n");
                    matrix = (vx_matrix)ownGetErrorObject(
                        context, (vx_status)VX_ERROR_NO_RESOURCES);
                    VX_PRINT(VX_ZONE_ERROR, "Could not allocate matrix object descriptor\n");
                }
                else
                {
                    obj_desc->mem_ptr.shared_ptr =
                        tivxMemHost2SharedPtr(
                            obj_desc->mem_ptr.host_ptr,
                            (vx_enum)TIVX_MEM_EXTERNAL);
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid matrix reference\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if (((vx_status)VX_SUCCESS == status) && ((uint64_t)(uintptr_t)NULL != obj_desc->mem_ptr.host_ptr))
    {
        tivxCheckStatus(&status, tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr,
            obj_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_WRITE_ONLY));

        pTempDataPtr = (vx_uint8 *)(uintptr_t)obj_desc->mem_ptr.host_ptr;
        if ((vx_enum)VX_PATTERN_BOX == pattern)
        {
            for (i = 0U; i < rows; i ++)
            {
                for (j = 0U; j < columns; j ++)
                {
                    *pTempDataPtr = 255U;
                    pTempDataPtr ++;
                }
            }
        }
        else if ((vx_enum)VX_PATTERN_CROSS == pattern)
        {
            for (i = 0U; i < rows; i ++)
            {
                for (j = 0U; j < columns; j ++)
                {
                    *pTempDataPtr = 0;
                    pTempDataPtr ++;
                }
            }
            /* Set data values in the centre row and column to 255  */
            pTempDataPtr = (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr;
            pTempDataPtr = pTempDataPtr + (((rows/2U))*columns);
            for (i = 0U; i < columns; i ++)
            {
                *pTempDataPtr = 255;
                pTempDataPtr ++;
            }
            pTempDataPtr = (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr;
            pTempDataPtr = pTempDataPtr + ((columns/2U));
            for (i = 0U; i < rows; i ++)
            {
                *pTempDataPtr = 255;
                pTempDataPtr += columns;
            }
        }
        else if ((vx_enum)VX_PATTERN_DISK == pattern)
        {
            vx_uint8* mask = (vx_uint8*)(uintptr_t)obj_desc->mem_ptr.host_ptr;
            vx_int16 ref;

            for (i = 0U; i < rows; i ++)
            {
                for (j = 0U; j < columns; j ++)
                {
                    ref = (((((((vx_float32)i - ((vx_float32)rows / 2.0f)) + 0.5f) * (((vx_float32)i - ((vx_float32)rows / 2.0f) ) + 0.5f)) / (((vx_float32)rows / 2.0f) * ((vx_float32)rows / 2.0f))) +
                        (((((vx_float32)j - ((vx_float32)columns / 2.0f)) + 0.5f) * (((vx_float32)j - ((vx_float32)columns / 2.0f)) + 0.5f)) / (((vx_float32)columns / 2.0f) * ((vx_float32)columns / 2.0f))))
                        <= 1.0f) ? 255 : 0;

                    mask[j + (i * columns)] = (vx_uint8)ref;
                }
            }
        }
        else /* VS_PATTERN_OTHER */
        {
            for (i = 0U; i < rows; i ++)
            {
                for (j = 0U; j < columns; j ++)
                {
                    *pTempDataPtr = 0;
                    pTempDataPtr ++;
                }
            }
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr,
            obj_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_WRITE_ONLY));
    }

    return (matrix);
}

vx_status VX_API_CALL vxQueryMatrix(
    vx_matrix matrix, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_matrix_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference((vx_reference)matrix, (vx_enum)VX_TYPE_MATRIX) == (vx_bool)vx_false_e)
        ||
        (matrix->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid matrix reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_matrix_t *)matrix->base.obj_desc;
        switch (attribute)
        {
            case (vx_enum)VX_MATRIX_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = obj_desc->data_type;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query matrix type failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_MATRIX_COLUMNS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->columns;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query matrix columns failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)(vx_enum)VX_MATRIX_ROWS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->rows;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query matrix rows failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_MATRIX_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr =
                        obj_desc->mem_size;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query matrix size failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_MATRIX_ORIGIN:
                if (VX_CHECK_PARAM(ptr, size, vx_coordinates2d_t, 0x3U))
                {
                    vx_coordinates2d_t *rect = (vx_coordinates2d_t *)ptr;

                    rect->x = obj_desc->origin_x;
                    rect->y = obj_desc->origin_y;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query matrix origin failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_MATRIX_PATTERN:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = obj_desc->pattern;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query matrix pattern failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid matrix query attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseMatrix(vx_matrix *matrix)
{
    return (ownReleaseReferenceInt(
        (vx_reference*)matrix, (vx_enum)VX_TYPE_MATRIX, (vx_enum)VX_EXTERNAL, NULL));
}

vx_status VX_API_CALL vxCopyMatrix(
    vx_matrix matrix, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 size;
    tivx_obj_desc_matrix_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference((vx_reference)matrix, (vx_enum)VX_TYPE_MATRIX) == (vx_bool)vx_false_e)
        ||
        (matrix->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid matrix reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_matrix_t *)matrix->base.obj_desc;
        if ((vx_enum)VX_MEMORY_TYPE_HOST != user_mem_type)
        {
            VX_PRINT(VX_ZONE_ERROR, "user mem type is not equal to VX_MEMORY_TYPE_HOST\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        /* Memory still not allocated */
        if (((vx_enum)VX_READ_ONLY == usage) &&
            ((uint64_t)(uintptr_t)NULL == obj_desc->mem_ptr.host_ptr))
        {
            VX_PRINT(VX_ZONE_ERROR, "Memory is not allocated\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        if (NULL == user_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "User pointer is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        size = obj_desc->mem_size;

        /* Copy from matrix object to user memory */
        if ((vx_enum)VX_READ_ONLY == usage)
        {
            tivxCheckStatus(&status, tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            memcpy(user_ptr, (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size);

            tivxCheckStatus(&status, tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }
        else /* Copy from user memory to matrix object */
        {
            status = ownAllocMatrixBuffer(&matrix->base);

            if ((vx_status)VX_SUCCESS == status)
            {
                tivxCheckStatus(&status, tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

                memcpy((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, user_ptr, size);

                tivxCheckStatus(&status, tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
            }
        }
    }

    return (status);
}

static vx_status ownAllocMatrixBuffer(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_matrix_t *obj_desc = NULL;

    if(ref->type == (vx_enum)VX_TYPE_MATRIX)
    {
        obj_desc = (tivx_obj_desc_matrix_t *)ref->obj_desc;
        if(obj_desc != NULL)
        {
            /* memory is not allocated, so allocate it */
            if(obj_desc->mem_ptr.host_ptr == (uint64_t)(uintptr_t)NULL)
            {
                status = tivxMemBufferAlloc(
                    &obj_desc->mem_ptr, obj_desc->mem_size,
                    (vx_enum)TIVX_MEM_EXTERNAL);

                if (((vx_status)VX_SUCCESS != status) ||
                    (obj_desc->mem_ptr.host_ptr == (uint64_t)(uintptr_t)NULL))
                {
                    /* could not allocate memory */
                    VX_PRINT(VX_ZONE_ERROR, "Memory could not be allocated\n");
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Object descriptor is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid matrix reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownDestructMatrix(vx_reference ref)
{
    tivx_obj_desc_matrix_t *obj_desc = NULL;

    if(ref->type == (vx_enum)VX_TYPE_MATRIX)
    {
        obj_desc = (tivx_obj_desc_matrix_t *)ref->obj_desc;
        if(obj_desc!=NULL)
        {
            if(obj_desc->mem_ptr.host_ptr!=(uint64_t)(uintptr_t)NULL)
            {
                tivxMemBufferFree(
                    &obj_desc->mem_ptr, obj_desc->mem_size);
            }

            ownObjDescFree((tivx_obj_desc_t**)&obj_desc);
        }
    }
    return (vx_status)VX_SUCCESS;
}
