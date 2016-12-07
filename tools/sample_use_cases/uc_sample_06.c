/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <uc_sample_06.h>

vx_status uc_sample_06_create(uc_sample_06 usecase)
{
    vx_status status = VX_SUCCESS;
    
    if (status == VX_SUCCESS)
    {
        usecase->context = vxCreateContext();
        if (usecase->context == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        status = uc_sample_06_data_create(usecase);
    }
    
    return status;
}

vx_status uc_sample_06_verify(uc_sample_06 usecase)
{
    vx_status status = VX_SUCCESS;
    
    
    return status;
}

vx_status uc_sample_06_run(uc_sample_06 usecase)
{
    vx_status status = VX_SUCCESS;
    
    
    return status;
}

vx_status uc_sample_06_delete(uc_sample_06 usecase)
{
    vx_status status = VX_SUCCESS;
    
    if (status == VX_SUCCESS)
    {
        status = uc_sample_06_data_delete(usecase);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseContext(&usecase->context);
    }
    
    return status;
}

vx_status uc_sample_06_data_create(uc_sample_06 usecase)
{
    vx_status status = VX_SUCCESS;
    
    vx_context context = usecase->context;
    
    if (status == VX_SUCCESS)
    {
        vx_char value = 'c';
        
        usecase->scalar_0 = vxCreateScalar(context, VX_TYPE_CHAR, &value);
        if (usecase->scalar_0 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_int8 value = -18;
        
        usecase->scalar_1 = vxCreateScalar(context, VX_TYPE_INT8, &value);
        if (usecase->scalar_1 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_uint8 value = 18;
        
        usecase->scalar_2 = vxCreateScalar(context, VX_TYPE_UINT8, &value);
        if (usecase->scalar_2 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_int16 value = -4660;
        
        usecase->scalar_3 = vxCreateScalar(context, VX_TYPE_INT16, &value);
        if (usecase->scalar_3 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_uint16 value = 4660;
        
        usecase->scalar_4 = vxCreateScalar(context, VX_TYPE_UINT16, &value);
        if (usecase->scalar_4 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_int32 value = -305419896;
        
        usecase->scalar_5 = vxCreateScalar(context, VX_TYPE_INT32, &value);
        if (usecase->scalar_5 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_uint32 value = 305419896;
        
        usecase->scalar_6 = vxCreateScalar(context, VX_TYPE_UINT32, &value);
        if (usecase->scalar_6 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_int64 value = -78187493520;
        
        usecase->scalar_7 = vxCreateScalar(context, VX_TYPE_INT64, &value);
        if (usecase->scalar_7 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_uint64 value = 78187493520;
        
        usecase->scalar_8 = vxCreateScalar(context, VX_TYPE_UINT64, &value);
        if (usecase->scalar_8 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_float32 value = -1234.1234;
        
        usecase->scalar_9 = vxCreateScalar(context, VX_TYPE_FLOAT32, &value);
        if (usecase->scalar_9 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_float64 value = -1234567890.1234567;
        
        usecase->scalar_10 = vxCreateScalar(context, VX_TYPE_FLOAT64, &value);
        if (usecase->scalar_10 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_TYPE_FLOAT32;
        
        usecase->scalar_11 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_11 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_size value = 1234;
        
        usecase->scalar_12 = vxCreateScalar(context, VX_TYPE_SIZE, &value);
        if (usecase->scalar_12 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_df_image value = VX_DF_IMAGE_NV12;
        
        usecase->scalar_13 = vxCreateScalar(context, VX_TYPE_DF_IMAGE, &value);
        if (usecase->scalar_13 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_bool value = vx_true_e;
        
        usecase->scalar_14 = vxCreateScalar(context, VX_TYPE_BOOL, &value);
        if (usecase->scalar_14 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    
    return status;
}

vx_status uc_sample_06_data_delete(uc_sample_06 usecase)
{
    vx_status status = VX_SUCCESS;
    
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_0);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_1);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_2);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_3);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_4);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_5);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_6);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_7);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_8);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_9);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_10);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_11);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_12);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_13);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_14);
    }
    
    return status;
}


