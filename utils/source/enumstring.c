/*
 * Copyright (c) 2022 ETAS Ltd.
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
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */
#include <stdio.h>
#include <stdlib.h>
#include "enumstring.h"

#define sconv( x ) case x: return #x ;
const char * enumToString(vx_enum enum_c)
{
    switch (enum_c)
    {
        sconv(VX_TYPE_UINT8)
        sconv(VX_TYPE_UINT16)
        sconv(VX_TYPE_UINT32)
        sconv(VX_TYPE_UINT64)
        sconv(VX_TYPE_INT16)
        sconv(VX_TYPE_INT32)
        sconv(VX_TYPE_INT64)
        sconv(VX_DF_IMAGE_U8)
        sconv(VX_DF_IMAGE_U16)
        sconv(VX_DF_IMAGE_U32)
        sconv(VX_DF_IMAGE_S16)
        sconv(VX_DF_IMAGE_S32)
        sconv(VX_DF_IMAGE_YUV4)
        sconv(VX_DF_IMAGE_YUYV)
        sconv(VX_DF_IMAGE_IYUV)
        sconv(VX_DF_IMAGE_UYVY)
        sconv(VX_DF_IMAGE_RGB)
        sconv(VX_DF_IMAGE_RGBX)
        sconv(VX_DF_IMAGE_VIRT)
        sconv(VX_CHANNEL_0)
        sconv(VX_CHANNEL_1)
        sconv(VX_CHANNEL_2)
        sconv(VX_CHANNEL_3)
        sconv(VX_CHANNEL_A)
        sconv(VX_CHANNEL_B)
        sconv(VX_CHANNEL_G)
        sconv(VX_CHANNEL_R)
        sconv(VX_CHANNEL_RANGE_FULL)
        sconv(VX_CHANNEL_RANGE_RESTRICTED)
        sconv(VX_CHANNEL_U)
        sconv(VX_CHANNEL_V)
        sconv(VX_TYPE_ARRAY)
        sconv(VX_TYPE_CONTEXT)
        sconv(VX_TYPE_CONVOLUTION)
        sconv(VX_TYPE_DELAY)
        sconv(VX_TYPE_DISTRIBUTION)
        sconv(VX_TYPE_ERROR)
        sconv(VX_TYPE_GRAPH)
        sconv(VX_TYPE_IMAGE)
        sconv(VX_TYPE_KERNEL)
        sconv(VX_TYPE_LUT)
        sconv(VX_TYPE_MATRIX)
        sconv(VX_TYPE_META_FORMAT)
        sconv(VX_TYPE_NODE)
        sconv(VX_TYPE_OBJECT_ARRAY)
        sconv(VX_TYPE_PARAMETER)
        sconv(VX_TYPE_PYRAMID)
        sconv(TIVX_TYPE_RAW_IMAGE)
        sconv(VX_TYPE_REMAP)
        sconv(VX_TYPE_SCALAR)
        sconv(VX_TYPE_TENSOR)
        sconv(VX_TYPE_THRESHOLD)
        sconv(VX_TYPE_USER_DATA_OBJECT)
        sconv(vx_true_e)
        sconv(vx_false_e)
        default: return "<enumeration constant>";
    }
}