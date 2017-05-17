/*
 *******************************************************************************
 *
 * Copyright (C) 2016-2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_TARGET_KERNELS_UTILS_
#define TIVX_TARGET_KERNELS_UTILS_

#include <tivx_kernel_utils.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief A utility API to initialize VXLIB buf parameters based
 *        on the provided valid rectangle and given object descriptor.
 *
 *        This API takes valid rectangle and object descriptor as an argument
 *        uses them to initialize VXLIB buf descriptor. It uses valid
 *        rectangle to initialize dimensions of the frame and object
 *        descriptor to initialize stride and data type.
 *        While initializing frame dimensions, it also takes into account
 *        the padding requirement of the calling kernel. If the kernel
 *        requires few pixels/lines on all sides of the kernels, this api
 *        increases the valid rectangle and then initializes vxlib buf
 *        descriptor.
 *
 *        If the valid rectangle is not provided, this API uses valid
 *        rectangle from the object descriptor.
 *
 * \param prms [in] Valid Rectangle Parameters
 */
void ownInitBufParams(
    tivx_obj_desc_image_t *obj_desc,
    vx_rectangle_t *rect,
    VXLIB_bufParams2D_t buf_params[],
    uint8_t *addr[],
    uint32_t lpad, uint32_t tpad, uint32_t rpad, uint32_t bpad);

/*!
 * \brief Reserve L2MEM within C66x for usage with BAM framework
 *
 */
void ownReserveC66xL2MEM(void);

#ifdef __cplusplus
}
#endif

#endif
