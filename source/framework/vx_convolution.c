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

static vx_bool vxIsPowerOfTwo(vx_uint32 a);
static int8_t isodd(size_t a);

VX_API_ENTRY vx_convolution VX_API_CALL vxCreateConvolution(
    vx_context context, vx_size columns, vx_size rows)
{
    vx_convolution cnvl = NULL;
    vx_reference ref = NULL;
    tivx_obj_desc_convolution_t *obj_desc = NULL;

    if(ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        if ((isodd(columns) == 1) && (columns >= 3U) &&
            ((vx_enum)columns < (vx_enum)VX_CONTEXT_CONVOLUTION_MAX_DIMENSION) &&
            (isodd(rows) == 1) && (rows >= 3U) &&
            ((vx_enum)rows < (vx_enum)VX_CONTEXT_CONVOLUTION_MAX_DIMENSION))
        {
            ref = ownCreateReference(context,
                (vx_enum)VX_TYPE_CONVOLUTION, (vx_enum)VX_EXTERNAL, &context->base);

            if ((vxGetStatus(ref) == (vx_status)VX_SUCCESS) &&
                (ref->type == (vx_enum)VX_TYPE_CONVOLUTION))
            {
                /* status set to NULL due to preceding type check */
                cnvl = vxCastRefAsConvolution(ref, NULL);
                /* assign refernce type specific callback's */
                cnvl->base.destructor_callback = &ownDestructReferenceGeneric;
                cnvl->base.mem_alloc_callback = &ownAllocReferenceBufferGeneric;
                cnvl->base.release_callback =
                    &ownReleaseReferenceBufferGeneric;

                obj_desc = (tivx_obj_desc_convolution_t*)ownObjDescAlloc(
                    (vx_enum)TIVX_OBJ_DESC_CONVOLUTION, vxCastRefFromConvolution(cnvl));
                if(obj_desc==NULL)
                {
                    (void)vxReleaseConvolution(&cnvl);

                    vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                        "Could not allocate cnvl object descriptor\n");
                    cnvl = (vx_convolution)ownGetErrorObject(
                        context, (vx_status)VX_ERROR_NO_RESOURCES);
                    VX_PRINT(VX_ZONE_ERROR, "Could not allocate cnvl object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                    VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in source/platform/psdk_j7/common/soc/tivx_platform_psdk_<soc>.h\n");
                }
                else
                {
                    obj_desc->columns = (uint32_t)columns;
                    obj_desc->rows = (uint32_t)rows;
                    obj_desc->scale = 1;
                    obj_desc->mem_size = (uint32_t)columns*(uint32_t)rows*(uint32_t)sizeof(vx_int16);
                    obj_desc->mem_ptr.host_ptr = (uint64_t)0;
                    obj_desc->mem_ptr.shared_ptr = (uint64_t)0;
                    obj_desc->mem_ptr.mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;
                    cnvl->base.obj_desc = (tivx_obj_desc_t *)obj_desc;
                }
            }
        }
    }

    return (cnvl);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseConvolution(vx_convolution *cnvl)
{
    return (ownReleaseReferenceInt(
        vxCastRefFromConvolutionP(cnvl), (vx_enum)VX_TYPE_CONVOLUTION, (vx_enum)VX_EXTERNAL, NULL));
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryConvolution(
    vx_convolution cnvl, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_convolution_t *obj_desc = NULL;

    if (0 != ownIsValidSpecificReference(vxCastRefFromConvolution(cnvl), (vx_enum)VX_TYPE_CONVOLUTION))
    {
        obj_desc = (tivx_obj_desc_convolution_t *)cnvl->base.obj_desc;
    }

    if (obj_desc == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Object Descriptor! \n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = ownAllocReferenceBufferGeneric(&cnvl->base);

        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Could not allocate memory! \n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        switch (attribute)
        {
            case (vx_enum)VX_CONVOLUTION_SCALE:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->scale;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "scale query failed \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONVOLUTION_COLUMNS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->columns;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "columns query failed \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONVOLUTION_ROWS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->rows;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "rows query failed \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONVOLUTION_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr =
                        obj_desc->mem_size;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "size query failed \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "invalid attribute \n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetConvolutionAttribute(
    vx_convolution cnvl, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_convolution_t *obj_desc = NULL;

    if (0 != ownIsValidSpecificReference(vxCastRefFromConvolution(cnvl), (vx_enum)VX_TYPE_CONVOLUTION))
    {
        obj_desc = (tivx_obj_desc_convolution_t *)cnvl->base.obj_desc;
    }

    if (obj_desc == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Object Descriptor! \n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = ownAllocReferenceBufferGeneric(&cnvl->base);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        switch (attribute)
        {
            case (vx_enum)VX_CONVOLUTION_SCALE:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    vx_uint32 scale = *(const vx_uint32 *)ptr;
                    if (vxIsPowerOfTwo(scale) == (vx_bool)vx_true_e)
                    {
                        obj_desc->scale = scale;
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "scale value is not a power of two \n");
                        status = (vx_status)VX_ERROR_INVALID_VALUE;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "set convolution scale failed \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "invalid attribute \n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                break;
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxCopyConvolutionCoefficients(
    vx_convolution cnvl, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 size;
    tivx_obj_desc_convolution_t *obj_desc = NULL;

    if (0 != ownIsValidSpecificReference(vxCastRefFromConvolution(cnvl), (vx_enum)VX_TYPE_CONVOLUTION))
    {
        obj_desc = (tivx_obj_desc_convolution_t *)cnvl->base.obj_desc;
    }

    if (obj_desc == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Object Descriptor! \n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if ((vx_enum)VX_MEMORY_TYPE_HOST != user_mem_type)
        {
            VX_PRINT(VX_ZONE_ERROR, "user_mem_type is not VX_MEMORY_TYPE_HOST\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        /* Memory still not allocated */
        if (((vx_enum)VX_READ_ONLY == usage) &&
            ((uint64_t)0 == obj_desc->mem_ptr.host_ptr))
        {
            VX_PRINT(VX_ZONE_ERROR, "Memory still not allocated\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        if (NULL == user_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "user pointer is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        size = obj_desc->mem_size;

        /* Copy from cnvl object to user memory */
        if ((vx_enum)VX_READ_ONLY == usage)
        {
            tivxCheckStatus(&status, tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            (void)memcpy(user_ptr, (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size);

            tivxCheckStatus(&status, tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }
        else /* Copy from user memory to cnvl object */
        {
            status = ownAllocReferenceBufferGeneric(&cnvl->base);

            if ((vx_status)VX_SUCCESS == status)
            {
                tivxCheckStatus(&status, tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

                (void)memcpy((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, user_ptr, size);

                tivxCheckStatus(&status, tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Convolution allocation failed\n");
            }
        }
    }

    return (status);
}

static vx_bool vxIsPowerOfTwo(vx_uint32 a)
{
    vx_bool isPowTwo;
    if (a == 0U)
    {
        isPowTwo =(vx_bool)vx_false_e;
    }
    else if ((a & ((a) - 1U)) == 0U)
    {
        isPowTwo = (vx_bool)vx_true_e;
    }
    else
    {
        isPowTwo = (vx_bool)vx_false_e;
    }

    return (isPowTwo);
}

static int8_t isodd(size_t a)
{
    int8_t result = (((a & 1ULL) != 0ULL)? 1 : 0);
    return result;
}

