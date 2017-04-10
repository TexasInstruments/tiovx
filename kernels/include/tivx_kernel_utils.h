/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_KERNEL_UTILS_
#define TIVX_KERNEL_UTILS_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for utility functions for the Kernel
 */

static inline vx_uint32 ownComputePatchOffset(
    vx_uint32 x, vx_uint32 y, const vx_imagepatch_addressing_t *addr);

static inline void* ownFormatImagePatchAddress2d(
    void *ptr, vx_uint32 x, vx_uint32 y,
    const vx_imagepatch_addressing_t *addr);

static inline vx_uint32 ownComputePatchOffset(
    vx_uint32 x, vx_uint32 y, const vx_imagepatch_addressing_t *addr)
{
    return (addr->stride_y * (y / addr->step_y)) +
           (addr->stride_x * (x / addr->step_x));
}

static inline void* ownFormatImagePatchAddress2d(
    void *ptr, vx_uint32 x, vx_uint32 y,
    const vx_imagepatch_addressing_t *addr)
{
    vx_uint8 *new_ptr = NULL;
    if (ptr && (x < addr->dim_x) && (y < addr->dim_y))
    {
        vx_uint32 offset = ownComputePatchOffset(x, y, addr);
        new_ptr = (vx_uint8 *)ptr;
        new_ptr = &new_ptr[offset];
    }
    return new_ptr;
}


/*!
 * \brief Check number of parameters and NULL pointers
 *
 *          First checks that the num_params is equal to the max_params
 *          defined by the OpenVX kernel.
 *
 *          Also checks that each of the obj_desc pointers are not NULL.
 *
 *          This function can be called if ALL of the parameters are
 *          mandatory.  If there are any optional parameters, then
 *          custom code should be used to check the parameters.
 */
static inline vx_status ownCheckNullParams(
    tivx_obj_desc_t *obj_desc[], uint16_t num_params,
    uint16_t max_params)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;

    if (num_params != max_params)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < max_params; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }
    return status;
}

#ifdef __cplusplus
}
#endif

#endif
