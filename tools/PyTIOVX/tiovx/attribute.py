#
# Copyright (c) 2017 Texas Instruments Incorporated
#
# All rights reserved not granted herein.
#
# Limited License.
#
# Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
# license under copyrights and patents it now or hereafter owns or controls to make,
# have made, use, import, offer to sell and sell ("Utilize") this software subject to the
# terms herein.  With respect to the foregoing patent license, such license is granted
# solely to the extent that any such patent is necessary to Utilize the software alone.
# The patent license shall not apply to any combinations which include this software,
# other than combinations with devices manufactured by or for TI ("TI Devices").
# No hardware patent is licensed hereunder.
#
# Redistributions must preserve existing copyright notices and reproduce this license
# (including the above copyright notice and the disclaimer and (if applicable) source
# code license limitations below) in the documentation and/or other materials provided
# with the distribution
#
# Redistribution and use in binary form, without modification, are permitted provided
# that the following conditions are met:
#
#       No reverse engineering, decompilation, or disassembly of this software is
# permitted with respect to any software provided in binary form.
#
#       any redistribution and use are licensed by TI for use only with TI Devices.
#
#       Nothing shall obligate TI to provide you with source code for the software
# licensed and provided to you in object code.
#
# If software source code is provided to you, modification and redistribution of the
# source code are permitted provided that the following conditions are met:
#
#       any redistribution and use of the source code, including any resulting derivative
# works, are licensed by TI for use only with TI Devices.
#
#       any redistribution and use of any object code compiled from the source code
# and any resulting derivative works, are licensed by TI for use only with TI Devices.
#
# Neither the name of Texas Instruments Incorporated nor the names of its suppliers
#
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# DISCLAIMER.
#
# THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
#

from enum import Enum
from . import *

## Object/Data type (OpenVX equivalent = \ref vx_image_attribute_e)
#
# \par Example usage:
# \code
# ImageAttribute.WIDTH
# ImageAttribute.HEIGHT
# \endcode
# \ingroup ATTRIBUTES
#
class ImageAttribute(Enum) :
    ## OpenVX equivalent = \ref VX_IMAGE_WIDTH
    WIDTH       = ('w', 'vx_uint32')
    ## OpenVX equivalent = \ref VX_IMAGE_HEIGHT
    HEIGHT      = ('h', 'vx_uint32')
    ## OpenVX equivalent = \ref VX_IMAGE_FORMAT
    FORMAT      = ('fmt', 'vx_df_image')
    ## OpenVX equivalent = \ref VX_IMAGE_PLANES
    PLANES      = ('planes', 'vx_size')
    ## OpenVX equivalent = \ref VX_IMAGE_SPACE
    SPACE       = ('space', 'vx_enum')
    ## OpenVX equivalent = \ref VX_IMAGE_RANGE
    RANGE       = ('range', 'vx_enum')
    ## OpenVX equivalent = \ref VX_IMAGE_SIZE
    SIZE        = ('size', 'vx_size')
    ## OpenVX equivalent = \ref VX_IMAGE_MEMORY_TYPE
    MEMORY_TYPE = ('memory_type', 'vx_enum')

    def vx_enum_name(attr) :
        return "VX_IMAGE_" + attr.name

    def object_type() :
        return Type.IMAGE

## Object/Data type (OpenVX equivalent = \ref vx_pyramid_attribute_e)
#
# \par Example usage:
# \code
# PyramidAttribute.WIDTH
# PyramidAttribute.HEIGHT
# \endcode
# \ingroup ATTRIBUTES
#
class PyramidAttribute(Enum) :
    ## OpenVX equivalent = \ref VX_PYRAMID_LEVELS
    LEVELS      = ('levels', 'vx_size')
    ## OpenVX equivalent = \ref VX_PYRAMID_SCALE
    SCALE       = ('scale', 'vx_float32')
    ## OpenVX equivalent = \ref VX_PYRAMID_WIDTH
    WIDTH       = ('w', 'vx_uint32')
    ## OpenVX equivalent = \ref VX_PYRAMID_HEIGHT
    HEIGHT      = ('h', 'vx_uint32')
    ## OpenVX equivalent = \ref VX_PYRAMID_FORMAT
    FORMAT      = ('fmt', 'vx_df_image')

    def vx_enum_name(attr) :
        return "VX_PYRAMID_" + attr.name

    def object_type() :
        return Type.PYRAMID

## Object/Data type (OpenVX equivalent = \ref vx_array_attribute_e)
#
# \par Example usage:
# \code
# ArrayAttribute.ITEMSIZE
# ArrayAttribute.ITEMTYPE
# \endcode
# \ingroup ATTRIBUTES
#
class ArrayAttribute(Enum) :
    ## OpenVX equivalent = \ref VX_ARRAY_ITEMTYPE
    ITEMTYPE      = ('item_type', 'vx_enum')
    ## OpenVX equivalent = \ref VX_ARRAY_NUMITEMS
    NUMITEMS      = ('num_items', 'vx_size')
    ## OpenVX equivalent = \ref VX_ARRAY_CAPACITY
    CAPACITY      = ('capacity', 'vx_size')
    ## OpenVX equivalent = \ref VX_ARRAY_ITEMSIZE
    ITEMSIZE      = ('item_size', 'vx_size')

    def vx_enum_name(attr) :
        return "VX_ARRAY_" + attr.name

    def object_type() :
        return Type.ARRAY

## Object/Data type (OpenVX equivalent = \ref vx_scalar_attribute_e)
#
# \par Example usage:
# \code
# ScalarAttribute.TYPE
# \endcode
# \ingroup ATTRIBUTES
#
class ScalarAttribute(Enum) :
    ## OpenVX equivalent = \ref VX_SCALAR_TYPE
    TYPE      = ('scalar_type', 'vx_enum')

    def vx_enum_name(attr) :
        return "VX_SCALAR_" + attr.name

    def object_type() :
        return Type.SCALAR

## Object/Data type (OpenVX equivalent = \ref vx_convolution_attribute_e)
#
# \par Example usage:
# \code
# ConvolutionAttribute.ROWS
# ConvolutionAttribute.COLUMNS
# \endcode
# \ingroup ATTRIBUTES
#
class ConvolutionAttribute(Enum) :
    ## OpenVX equivalent = \ref VX_CONVOLUTION_ROWS
    ROWS      = ('row', 'vx_size')
    ## OpenVX equivalent = \ref VX_CONVOLUTION_COLUMNS
    COLUMNS   = ('col', 'vx_size')
    ## OpenVX equivalent = \ref VX_CONVOLUTION_SCALE
    SCALE     = ('scale', 'vx_uint32')
    ## OpenVX equivalent = \ref VX_CONVOLUTION_SIZE
    SIZE      = ('size', 'vx_size')

    def vx_enum_name(attr) :
        return "VX_CONVOLUTION_" + attr.name

    def object_type() :
        return Type.CONVOLUTION

## Object/Data type (OpenVX equivalent = \ref vx_matrix_attribute_e)
#
# \par Example usage:
# \code
# MatrixAttribute.ROWS
# MatrixAttribute.COLUMNS
# \endcode
# \ingroup ATTRIBUTES
#
class MatrixAttribute(Enum) :
    ## OpenVX equivalent = \ref VX_MATRIX_TYPE
    TYPE      = ('type', 'vx_enum')
    ## OpenVX equivalent = \ref VX_MATRIX_ROWS
    ROWS      = ('h', 'vx_size')
    ## OpenVX equivalent = \ref VX_MATRIX_COLUMNS
    COLUMNS   = ('w', 'vx_size')
    ## OpenVX equivalent = \ref VX_MATRIX_SIZE
    SIZE      = ('size', 'vx_size')
    ## OpenVX equivalent = \ref VX_MATRIX_ORIGIN
    ORIGIN    = ('origin', 'vx_coordinates2d_t')
    ## OpenVX equivalent = \ref VX_MATRIX_PATTERN
    PATTERN   = ('pattern', 'vx_enum')

    def vx_enum_name(attr) :
        return "VX_MATRIX_" + attr.name

    def object_type() :
        return Type.MATRIX

## Object/Data type (OpenVX equivalent = \ref vx_lut_attribute_e)
#
# \par Example usage:
# \code
# LutAttribute.COUNT
# LutAttribute.TYPE
# \endcode
# \ingroup ATTRIBUTES
#
class LutAttribute(Enum) :
    ## OpenVX equivalent = \ref VX_LUT_TYPE
    TYPE      = ('type', 'vx_enum')
    ## OpenVX equivalent = \ref VX_LUT_COUNT
    COUNT     = ('count', 'vx_size')
    ## OpenVX equivalent = \ref VX_LUT_SIZE
    SIZE      = ('size', 'vx_size')
    ## OpenVX equivalent = \ref VX_LUT_OFFSET
    OFFSET    = ('offset', 'vx_uint32')

    def vx_enum_name(attr) :
        return "VX_LUT_" + attr.name

    def object_type() :
        return Type.LUT

## Object/Data type (OpenVX equivalent = \ref vx_distribution_attribute_e)
#
# \par Example usage:
# \code
# DistributionAttribute.BINS
# DistributionAttribute.RANGE
# \endcode
# \ingroup ATTRIBUTES
#
class DistributionAttribute(Enum) :
    ## OpenVX equivalent = \ref VX_DISTRIBUTION_DIMENSIONS
    DIMENSIONS  = ('dims', 'vx_size')
    ## OpenVX equivalent = \ref VX_DISTRIBUTION_OFFSET
    OFFSET      = ('offset', 'vx_int32')
    ## OpenVX equivalent = \ref VX_DISTRIBUTION_RANGE
    RANGE       = ('range', 'vx_uint32')
    ## OpenVX equivalent = \ref VX_DISTRIBUTION_BINS
    BINS        = ('bins', 'vx_size')
    ## OpenVX equivalent = \ref VX_DISTRIBUTION_WINDOW
    WINDOW      = ('win', 'vx_uint32')
    ## OpenVX equivalent = \ref VX_DISTRIBUTION_SIZE
    SIZE        = ('size', 'vx_size')

    def vx_enum_name(attr) :
        return "VX_DISTRIBUTION_" + attr.name

    def object_type() :
        return Type.DISTRIBUTION

## Object/Data type (OpenVX equivalent = \ref vx_threshold_attribute_e)
#
# \par Example usage:
# \code
# ThresholdAttribute.TYPE
# ThresholdAttribute.DATA_TYPE
# \endcode
# \ingroup ATTRIBUTES
#
class ThresholdAttribute(Enum) :
    ## OpenVX equivalent = \ref VX_THRESHOLD_TYPE
    TYPE             = ('threshold_type', 'vx_enum')
    ## OpenVX equivalent = \ref VX_THRESHOLD_THRESHOLD_VALUE
    THRESHOLD_VALUE  = ('value', 'vx_int32')
    ## OpenVX equivalent = \ref VX_THRESHOLD_THRESHOLD_LOWER
    THRESHOLD_LOWER  = ('lower', 'vx_int32')
    ## OpenVX equivalent = \ref VX_THRESHOLD_THRESHOLD_UPPER
    THRESHOLD_UPPER  = ('upper', 'vx_int32')
    ## OpenVX equivalent = \ref VX_THRESHOLD_TRUE_VALUE
    TRUE_VALUE       = ('true_value', 'vx_int32')
    ## OpenVX equivalent = \ref VX_THRESHOLD_FALSE_VALUE
    FALSE_VALUE      = ('false_value', 'vx_int32')
    ## OpenVX equivalent = \ref VX_THRESHOLD_FALSE_VALUE
    DATA_TYPE        = ('threshold_data_type', 'vx_enum')

    def vx_enum_name(attr) :
        return "VX_THRESHOLD_" + attr.name

    def object_type() :
        return Type.THRESHOLD

## Object/Data type (OpenVX equivalent = \ref vx_remap_attribute_e)
#
# \par Example usage:
# \code
# RemapAttribute.SOURCE_WIDTH
# RemapAttribute.SOURCE_HEIGHT
# \endcode
# \ingroup ATTRIBUTES
#
class RemapAttribute(Enum) :
    ## OpenVX equivalent = \ref VX_REMAP_SOURCE_WIDTH
    SOURCE_WIDTH         = ('src_w', 'vx_uint32')
    ## OpenVX equivalent = \ref VX_REMAP_SOURCE_HEIGHT
    SOURCE_HEIGHT        = ('src_h', 'vx_uint32')
    ## OpenVX equivalent = \ref VX_REMAP_DESTINATION_WIDTH
    DESTINATION_WIDTH    = ('dst_w', 'vx_uint32')
    ## OpenVX equivalent = \ref VX_REMAP_DESTINATION_HEIGHT
    DESTINATION_HEIGHT   = ('dst_h', 'vx_uint32')

    def vx_enum_name(attr) :
        return "VX_REMAP_" + attr.name

    def object_type() :
        return Type.REMAP

## Object/Data type (OpenVX equivalent = \ref vx_object_array_attribute_e)
#
# \par Example usage:
# \code
# ObjectArrayAttribute.ITEMTYPE
# ObjectArrayAttribute.NUMITEMS
# \endcode
# \ingroup ATTRIBUTES
#
class ObjectArrayAttribute(Enum) :
    ## OpenVX equivalent = \ref VX_OBJECT_ARRAY_ITEMTYPE
    ITEMTYPE      = ('type', 'vx_enum')
    ## OpenVX equivalent = \ref VX_OBJECT_ARRAY_NUMITEMS
    NUMITEMS      = ('num_items', 'vx_size')

    def vx_enum_name(attr) :
        return "VX_OBJECT_ARRAY_" + attr.name

    def object_type() :
        return Type.OBJECT_ARRAY

## Object/Data type (OpenVX equivalent = \ref vx_user_data_object_attribute_e)
#
# \par Example usage:
# \code
# UserDataObjectAttribute.NAME
# UserDataObjectAttribute.SIZE
# \endcode
# \ingroup ATTRIBUTES
#
class UserDataObjectAttribute(Enum) :
    ## OpenVX equivalent = \ref VX_USER_DATA_OBJECT_NAME
    NAME        = ('name', 'vx_char')
    ## OpenVX equivalent = \ref VX_USER_DATA_OBJECT_SIZE
    SIZE        = ('size', 'vx_size')

    def vx_enum_name(attr) :
        return "VX_USER_DATA_OBJECT_" + attr.name

    def object_type() :
        return Type.USER_DATA_OBJECT

## Object/Data type (OpenVX equivalent = \ref tivx_raw_image_attribute_e)
#
# \par Example usage:
# \code
# RawImageAttribute.WIDTH
# RawImageAttribute.HEIGHT
# \endcode
# \ingroup ATTRIBUTES
#
class RawImageAttribute(Enum) :
    ## OpenVX equivalent = \ref TIVX_RAW_IMAGE_WIDTH
    WIDTH              = ('w', 'vx_uint32')
    ## OpenVX equivalent = \ref TIVX_RAW_IMAGE_HEIGHT
    HEIGHT             = ('h', 'vx_uint32')
    ## OpenVX equivalent = \ref TIVX_RAW_IMAGE_NUM_EXPOSURES
    MUM_EXPOSURES      = ('num_exposures', 'vx_uint32')
    ## OpenVX equivalent = \ref TIVX_RAW_IMAGE_LINE_INTERLEAVED
    LINE_INTERLEAVED   = ('line_interleaved', 'vx_bool')
    ## OpenVX equivalent = \ref TIVX_RAW_IMAGE_FORMAT
    FORMAT             = ('format', 'tivx_raw_image_format_t')
    ## OpenVX equivalent = \ref TIVX_RAW_IMAGE_META_HEIGHT_BEFORE
    META_HEIGHT_BEFORE = ('meta_height_before', 'vx_uint32')
    ## OpenVX equivalent = \ref TIVX_RAW_IMAGE_META_HEIGHT_AFTER
    META_HEIGHT_AFTER  = ('meta_height_after', 'vx_uint32')

    def vx_enum_name(attr) :
        return "TIVX_RAW_IMAGE_" + attr.name

    def object_type() :
        return Type.RAW_IMAGE

## Object/Data type (OpenVX equivalent = \ref vx_tensor_attribute_e)
#
# \par Example usage:
# \code
# TensorAttribute.NUMBER_OF_DIMS
# TensorAttribute.DIMS
# TensorAttribute.DATA_TYPE
# TensorAttribute.FIXED_POINT_POSITION
# TensorAttribute.SCALING_DIVISOR
# TensorAttribute.SCALING_DIVISOR_FIXED_POINT_POSITION
# \endcode
# \ingroup ATTRIBUTES
#
class TensorAttribute(Enum) :
    ## OpenVX equivalent = \ref VX_TENSOR_NUMBER_OF_DIMS
    NUMBER_OF_DIMS         = ('number_of_dims', 'vx_size')
    ## OpenVX equivalent = \ref VX_TENSOR_DIMS
    DIMS                   = ('dims', 'vx_size')
    ## OpenVX equivalent = \ref VX_TENSOR_DATA_TYPE
    DATA_TYPE              = ('data_type', 'vx_enum')
    ## OpenVX equivalent = \ref VX_TENSOR_FIXED_POINT_POSITION
    FIXED_POINT_POSITION   = ('fixed_point_position', 'vx_int8')
    ## OpenVX equivalent = \ref TIVX_TENSOR_SCALING_DIVISOR
    SCALING_DIVISOR        = ('scaling_divisor', 'vx_int8')
    ## OpenVX equivalent = \ref TIVX_TENSOR_SCALING_DIVISOR_FIXED_POINT_POSITION
    SCALING_DIVISOR_FIXED_POINT_POSITION   = ('scaling_divisor_fixed_point_position', 'vx_int8')

    def vx_enum_name(attr) :
        return "VX_TENSOR_" + attr.name

    def object_type() :
        return Type.TENSOR

## Object/Data type (OpenVX equivalent = none ... type aggregator for all attributes)
#
# \par Example usage:
# \code
# Attribute.Image.WIDTH
# Attribute.Array.ITEMSIZE
# \endcode
# \ingroup ATTRIBUTES
#
class Attribute :
    Image = ImageAttribute
    Pyramid = PyramidAttribute
    Array = ArrayAttribute
    Scalar = ScalarAttribute
    Convolution = ConvolutionAttribute
    Matrix = MatrixAttribute
    Lut = LutAttribute
    Distribution = DistributionAttribute
    Threshold = ThresholdAttribute
    Remap = RemapAttribute
    ObjectArray = ObjectArrayAttribute
    UserDataObject = UserDataObjectAttribute
    RawImage = RawImageAttribute
    Tensor = TensorAttribute

    def from_type(type) :
        if type == Type.IMAGE :
            return Attribute.Image
        if type == Type.PYRAMID :
            return Attribute.Pyramid
        if type == Type.ARRAY :
            return Attribute.Array
        if type == Type.SCALAR :
            return Attribute.Scalar
        if type == Type.CONVOLUTION :
            return Attribute.Convolution
        if type == Type.MATRIX :
            return Attribute.Matrix
        if type == Type.LUT :
            return Attribute.Lut
        if type == Type.DISTRIBUTION :
            return Attribute.Distribution
        if type == Type.THRESHOLD :
            return Attribute.Threshold
        if type == Type.REMAP :
            return Attribute.Remap
        if type == Type.OBJECT_ARRAY :
            return Attribute.ObjectArray
        if type == Type.USER_DATA_OBJECT :
            return Attribute.UserDataObject
        if type == Type.RAW_IMAGE :
            return Attribute.RawImage
        if type == Type.TENSOR :
            return Attribute.Tensor
        return "INVALID"
