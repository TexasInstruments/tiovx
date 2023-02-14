/*
*
* Copyright (c) 2018 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


#include <vx_internal.h>

/*==============================================================================
User Data Object HELPER FUNCTIONS
=============================================================================*/

static vx_status ownDestructUserDataObject(vx_reference ref);
static vx_status ownAllocUserDataObjectBuffer(vx_reference ref);
static vx_status ownInitUserDataObjectObject(
    vx_user_data_object user_data_object, const vx_char* type_name, vx_size size);

static vx_status ownAllocUserDataObjectBuffer(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *obj_desc = NULL;

    if(ref->type == VX_TYPE_USER_DATA_OBJECT)
    {
        obj_desc = (tivx_obj_desc_user_data_object_t *)ref->obj_desc;

        if(obj_desc != NULL)
        {
            /* memory is not allocated, so allocate it */
            if(obj_desc->mem_ptr.host_ptr == (uint64_t)(uintptr_t)NULL)
            {
                tivxMemBufferAlloc(
                    &obj_desc->mem_ptr, obj_desc->mem_size,
                    (vx_enum)TIVX_MEM_EXTERNAL);

                if(obj_desc->mem_ptr.host_ptr==(uint64_t)(uintptr_t)NULL)
                {
                    /* could not allocate memory */
                    VX_PRINT(VX_ZONE_ERROR,"Could not allocate user data object memory\n");
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
                else
                {
                    obj_desc->mem_ptr.shared_ptr = tivxMemHost2SharedPtr(
                        obj_desc->mem_ptr.host_ptr, (vx_enum)TIVX_MEM_EXTERNAL);
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"User Data Object object descriptor is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Reference is not a user data object type\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownDestructUserDataObject(vx_reference ref)
{
    tivx_obj_desc_user_data_object_t *obj_desc = NULL;

    if(ref->type == VX_TYPE_USER_DATA_OBJECT)
    {
        obj_desc = (tivx_obj_desc_user_data_object_t *)ref->obj_desc;
        if(obj_desc != NULL)
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


static vx_status ownInitUserDataObjectObject(
    vx_user_data_object user_data_object, const vx_char* type_name, vx_size size)
{
    vx_uint32 i;
    tivx_obj_desc_user_data_object_t *obj_desc = NULL;
    vx_status status = (vx_status)VX_SUCCESS;

    obj_desc = (tivx_obj_desc_user_data_object_t *)user_data_object->base.obj_desc;

    obj_desc->mem_size = (vx_uint32)size;
    obj_desc->valid_mem_size = (vx_uint32)size;

    /* Initialize string with zeros, which safely fills with null terminators */
    obj_desc->type_name[0] = (char)0;

    if (type_name != NULL)
    {
        tivx_obj_desc_strncpy(obj_desc->type_name, (volatile void*)type_name, VX_MAX_REFERENCE_NAME);
    }

    obj_desc->mem_ptr.host_ptr = (uint64_t)(uintptr_t)NULL;
    obj_desc->mem_ptr.shared_ptr = (uint64_t)(uintptr_t)NULL;
    obj_desc->mem_ptr.mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;

    for (i = 0; i < TIVX_USER_DATA_OBJECT_MAX_MAPS; i ++)
    {
        user_data_object->maps[i].map_addr = NULL;
        user_data_object->maps[i].map_size = 0;
    }

    return status;
}


/*==============================================================================
   User Data Object API FUNCTIONS
=============================================================================*/

VX_API_ENTRY vx_user_data_object VX_API_CALL vxCreateUserDataObject(
    vx_context context,
    const vx_char *type_name,
    vx_size size,
    const void *ptr)
{
    vx_user_data_object user_data_object = NULL;

    if(ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        if (size < 1U)
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid size for the user data object.\n");
            user_data_object = (vx_user_data_object)ownGetErrorObject((vx_context)context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
        }

        if( NULL == user_data_object )
        {
            user_data_object = (vx_user_data_object)ownCreateReference(context, VX_TYPE_USER_DATA_OBJECT, (vx_enum)VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)user_data_object) == (vx_status)VX_SUCCESS) &&
                (user_data_object->base.type == VX_TYPE_USER_DATA_OBJECT))
            {
                /* assign reference type specific callback's */
                user_data_object->base.destructor_callback = &ownDestructUserDataObject;
                user_data_object->base.mem_alloc_callback = &ownAllocUserDataObjectBuffer;
                user_data_object->base.release_callback =
                    (tivx_reference_release_callback_f)&vxReleaseUserDataObject;

                user_data_object->base.obj_desc = (tivx_obj_desc_t *)ownObjDescAlloc(
                    (vx_enum)TIVX_OBJ_DESC_USER_DATA_OBJECT, (vx_reference)user_data_object);
                if(user_data_object->base.obj_desc==NULL)
                {
                    vxReleaseUserDataObject(&user_data_object);

                    vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                        "Could not allocate user data object descriptor\n");
                    user_data_object = (vx_user_data_object)ownGetErrorObject(
                        context, (vx_status)VX_ERROR_NO_RESOURCES);
                    VX_PRINT(VX_ZONE_ERROR, "Could not allocate user data object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                    VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in source/platform/psdk_j7/common/soc/tivx_platform_psdk_<soc>.h\n");
                }
                else
                {
                    vx_status status;
                    status = ownInitUserDataObjectObject(user_data_object, type_name, size);

                    if(status == (vx_status)VX_SUCCESS)
                    {
                        if (NULL != ptr)
                        {
                            status = vxCopyUserDataObject(user_data_object, 0, size, (void*)ptr, (vx_enum)VX_WRITE_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST);
                        }
                        else
                        {
                            status = ownAllocUserDataObjectBuffer(&user_data_object->base);

                            if (status == (vx_status)VX_SUCCESS)
                            {
                                vx_uint8 *start_ptr;
                                tivx_obj_desc_user_data_object_t *obj_desc = NULL;

                                obj_desc = (tivx_obj_desc_user_data_object_t *)user_data_object->base.obj_desc;
                                start_ptr = (vx_uint8 *)(uintptr_t)obj_desc->mem_ptr.host_ptr;

                                tivxCheckStatus(&status, tivxMemBufferMap(start_ptr, (uint32_t)size,
                                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

                                memset(start_ptr, 0, size);

                                tivxCheckStatus(&status, tivxMemBufferUnmap(start_ptr, (uint32_t)size,
                                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
                            }
                        }
                    }

                    if(status != (vx_status)VX_SUCCESS)
                    {
                        user_data_object = (vx_user_data_object)ownGetErrorObject(
                            context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
                    }
                }
            }
        }
    }

    return (user_data_object);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseUserDataObject(vx_user_data_object *user_data_object)
{
    return (ownReleaseReferenceInt(
        (vx_reference*)user_data_object, VX_TYPE_USER_DATA_OBJECT, (vx_enum)VX_EXTERNAL, NULL));
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryUserDataObject (
    vx_user_data_object user_data_object, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference((vx_reference)user_data_object, VX_TYPE_USER_DATA_OBJECT) == (vx_bool)vx_false_e)
            || (user_data_object->base.obj_desc == NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"vxQueryUserDataObject failed\n");
        VX_PRINT(VX_ZONE_ERROR,"Reference is invalid or object descriptor is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_user_data_object_t *)user_data_object->base.obj_desc;
        switch (attribute)
        {
            case (vx_enum)VX_USER_DATA_OBJECT_NAME:
                if ((ptr != NULL) && ((vx_enum)size >= VX_MAX_REFERENCE_NAME))
                {
                    tivx_obj_desc_strncpy(ptr, obj_desc->type_name, VX_MAX_REFERENCE_NAME);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "VX_USER_DATA_OBJECT_NAME failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_USER_DATA_OBJECT_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->mem_size;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "VX_USER_DATA_OBJECT_SIZE failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_USER_DATA_OBJECT_VALID_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->valid_mem_size;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "VX_USER_DATA_OBJECT_VALID_SIZE failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR,"query user data object option not supported\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxSetUserDataObjectAttribute(
    vx_user_data_object user_data_object, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference((vx_reference)user_data_object, VX_TYPE_USER_DATA_OBJECT) == (vx_bool)vx_false_e)
            || (user_data_object->base.obj_desc == NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"vxSetUserDataObjectAttribute failed\n");
        VX_PRINT(VX_ZONE_ERROR,"Reference is invalid or object descriptor is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_user_data_object_t *)user_data_object->base.obj_desc;
        switch (attribute)
        {
            case (vx_enum)TIVX_USER_DATA_OBJECT_VALID_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    obj_desc->valid_mem_size = (vx_uint32)*(const vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "VX_USER_DATA_OBJECT_VALID_SIZE failed\n");
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

VX_API_ENTRY vx_status VX_API_CALL vxCopyUserDataObject(vx_user_data_object user_data_object,
        vx_size offset, vx_size size, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *obj_desc = NULL;
    vx_uint8 *start_ptr;

    if ((ownIsValidSpecificReference((vx_reference)user_data_object, VX_TYPE_USER_DATA_OBJECT) == (vx_bool)vx_false_e) ||
        (user_data_object->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid user data object reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_user_data_object_t *)user_data_object->base.obj_desc;

        if ((vx_enum)VX_MEMORY_TYPE_HOST != user_mem_type)
        {
            VX_PRINT(VX_ZONE_ERROR, "User mem type is not equal to VX_MEMORY_TYPE_HOST\n");
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
            VX_PRINT(VX_ZONE_ERROR, "Invalid NULL pointer\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        if ((size < 1U) || ((offset + size) > obj_desc->mem_size))
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid offset or size parameter\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = ownAllocUserDataObjectBuffer(&user_data_object->base);
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        /* Get the offset to the free memory */
        start_ptr = (vx_uint8 *)(uintptr_t)obj_desc->mem_ptr.host_ptr + offset;

        /* Copy from internal object to user memory */
        if ((vx_enum)VX_READ_ONLY == usage)
        {
            tivxCheckStatus(&status, tivxMemBufferMap(start_ptr, (uint32_t)size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            memcpy(user_ptr, start_ptr, size);

            tivxCheckStatus(&status, tivxMemBufferUnmap(start_ptr, (uint32_t)size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }
        else /* Copy from user memory to internal object */
        {
            tivxCheckStatus(&status, tivxMemBufferMap(start_ptr, (uint32_t)size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

            memcpy(start_ptr, user_ptr, size);

            tivxCheckStatus(&status, tivxMemBufferUnmap(start_ptr, (uint32_t)size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
        }
    }

    return (status);
}

VX_API_ENTRY vx_status VX_API_CALL vxMapUserDataObject(
    vx_user_data_object user_data_object,
    vx_size offset,
    vx_size size,
    vx_map_id *map_id,
    void **ptr,
    vx_enum usage,
    vx_enum mem_type,
    vx_uint32 flags)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *obj_desc = NULL;
    vx_uint32 i;

    if ((ownIsValidSpecificReference((vx_reference)user_data_object, VX_TYPE_USER_DATA_OBJECT) == (vx_bool)vx_false_e) ||
        (user_data_object->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid user data object reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        if (ptr == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "User pointer is null\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        if (map_id == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "Map ID is null\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        obj_desc = (tivx_obj_desc_user_data_object_t *)user_data_object->base.obj_desc;

        if (((offset + size) > obj_desc->mem_size))
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid offset or size parameter\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = ownAllocUserDataObjectBuffer(&user_data_object->base);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        for (i = 0; i < TIVX_USER_DATA_OBJECT_MAX_MAPS; i ++)
        {
            if (user_data_object->maps[i].map_addr == NULL)
            {
                break;
            }
        }

        if (i < TIVX_USER_DATA_OBJECT_MAX_MAPS)
        {
            vx_uint8* map_addr = NULL;
            uint32_t map_size;

            map_addr = (vx_uint8*)(uintptr_t)(obj_desc->mem_ptr.host_ptr + offset);
            map_size = (size > 0U) ? (uint32_t)size : ((uint32_t)obj_desc->mem_size - (uint32_t)offset);

            user_data_object->maps[i].map_addr = map_addr;
            user_data_object->maps[i].map_size = map_size;
            user_data_object->maps[i].mem_type = mem_type;
            user_data_object->maps[i].usage = usage;

            tivxCheckStatus(&status, tivxMemBufferMap(map_addr, map_size,
                mem_type, usage));

            *ptr = (vx_uint8 *)map_addr;

            *map_id = i;

            ownLogSetResourceUsedValue("TIVX_USER_DATA_OBJECT_MAX_MAPS", (uint16_t)i+1U);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "No available user data object maps\n");
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_USER_DATA_OBJECT_MAX_MAPS in tiovx/include/TI/tivx_config.h\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }

    return (status);
}

VX_API_ENTRY vx_status VX_API_CALL vxUnmapUserDataObject(vx_user_data_object user_data_object, vx_map_id map_id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((ownIsValidSpecificReference((vx_reference)user_data_object, VX_TYPE_USER_DATA_OBJECT) == (vx_bool)vx_false_e) ||
        (user_data_object->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid user data object reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        if(map_id >= TIVX_USER_DATA_OBJECT_MAX_MAPS)
        {
            VX_PRINT(VX_ZONE_ERROR, "map ID is greater than the maximum user data object maps\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        if( (user_data_object->maps[map_id].map_addr!=NULL)
            &&
            (user_data_object->maps[map_id].map_size!=0U)
            )
        {
            vx_uint8* map_addr = NULL, *end_addr = NULL;
            uint32_t map_size = 0;

            map_addr = user_data_object->maps[map_id].map_addr;
            map_size = (uint32_t)user_data_object->maps[map_id].map_size;

            end_addr = map_addr + map_size;
            map_addr = (vx_uint8*)TIVX_FLOOR((uintptr_t)map_addr, 128U);
            end_addr = (vx_uint8*)TIVX_ALIGN((uintptr_t)end_addr, 128U);
            uintptr_t temp_map_size = (uintptr_t)end_addr - (uintptr_t)map_addr;
            map_size = (uint32_t)temp_map_size;

            tivxCheckStatus(&status, tivxMemBufferUnmap(
                map_addr, map_size,
                user_data_object->maps[map_id].mem_type,
                user_data_object->maps[map_id].usage));

            user_data_object->maps[map_id].map_addr = NULL;
        }
        else
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            if(user_data_object->maps[map_id].map_addr==NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "map address is null\n");
            }
            if(user_data_object->maps[map_id].map_size==0U)
            {
                VX_PRINT(VX_ZONE_ERROR, "map size is equal to 0\n");
            }
        }
    }

    return status;
}
