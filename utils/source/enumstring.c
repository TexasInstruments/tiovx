/*
 * Copyright (c) 2024 The Khronos Group Inc.
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