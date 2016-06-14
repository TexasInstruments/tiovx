/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */
/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _VX_REMAP_H_
#define _VX_REMAP_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Remap object
 */

/*!
 * \brief Remap point in remap table
 *
 * \ingroup group_vx_remap
 */
typedef struct _tivx_remap_point {

    vx_float32 src_x;
    vx_float32 src_y;

} tivx_remap_point_t;

/*!
 * \brief Remap object descriptor as placed in shared memory
 *
 * \ingroup group_vx_remap
 */
typedef struct _tivx_obj_desc_remap
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;
    /*! \brief The source width */
    uint32_t src_width;
    /*! \brief The source height */
    uint32_t src_height;
    /*! \brief The destination width */
    uint32_t dst_width;
    /*! \brief The destination height */
    uint32_t dst_height;
    /*! \brief size of buffer pointed to by mem_ptr */
    uint32_t mem_size;
    /*! \brief buffer address */
    tivx_shared_mem_ptr_t mem_ptr;
} tivx_obj_desc_remap_t;

/*!
 * \brief Remap object internal state
 *
 * \ingroup group_vx_remap
 */
typedef struct _vx_remap
{
    /*! \brief reference object */
    tivx_reference_t base;

    /*! \brief object descriptor */
    tivx_obj_desc_remap_t *obj_desc;

} tivx_remap_t;

#ifdef __cplusplus
}
#endif

#endif
