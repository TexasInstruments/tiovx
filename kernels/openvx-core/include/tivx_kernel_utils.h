/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KERNEL_UTILS_
#define _TIVX_KERNEL_UTILS_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for utility functions for the Kernel
 */

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
    if (ptr && x < addr->dim_x && y < addr->dim_y)
    {
        vx_uint32 offset = ownComputePatchOffset(x, y, addr);
        new_ptr = (vx_uint8 *)ptr;
        new_ptr = &new_ptr[offset];
    }
    return new_ptr;
}



#ifdef __cplusplus
}
#endif

#endif
