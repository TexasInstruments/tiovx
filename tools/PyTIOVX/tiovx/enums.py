#
# Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
# ALL RIGHTS RESERVED
#

from enum import Enum
from . import *


## Object/Data type (OpenVX equivalent = vx_type_e)
#
# \ingroup ENUMS
#
class Type(Enum):
    ## OpenVX equivalent = VX_TYPE_INVALID
    INVALID = 1
    ## OpenVX equivalent = VX_TYPE_CHAR
    CHAR    = 2
    ## OpenVX equivalent = VX_TYPE_INT8
    INT8    = 3
    ## OpenVX equivalent = VX_TYPE_UINT8
    UINT8   = 4
    INT16   = 5
    UINT16  = 6
    INT32   = 7
    UINT32  = 8
    INT64   = 9
    UINT64  = 10
    FLOAT32 = 11
    FLOAT64 = 12
    ENUM    = 13
    SIZE    = 14
    DF_IMAGE= 15
    BOOL    = 16
    REFERENCE    = 17
    CONTEXT      = 18
    GRAPH        = 19
    NODE         = 20
    KERNEL       = 21
    PARAMETER    = 22
    DELAY        = 23
    LUT          = 24
    DISTRIBUTION = 25
    PYRAMID      = 26
    THRESHOLD    = 27
    MATRIX       = 28
    CONVOLUTION  = 29
    SCALAR       = 30
    ARRAY        = 31
    IMAGE        = 32
    REMAP        = 33
    ERROR        = 34
    META_FORMAT  = 35
    OBJECT_ARRAY = 36

    def __lt__(self, other):
        if self.__class__ is other.__class__:
            return self.value < other.value
        return NotImplemented

    def __gt__(self, other):
        if self.__class__ is other.__class__:
            return self.value > other.value
        return NotImplemented

    def __eq__(self, other):
        if self.__class__ is other.__class__:
            return self.value == other.value
        return NotImplemented

    def get_vx_enum_name(type) :
        return "VX_TYPE_" + type.name

    def get_vx_name(type) :
        return "vx_" + type.name.lower()

    def is_scalar_type(type) :
        if type > Type.INVALID and type < Type.REFERENCE :
            return True
        if type == Type.SCALAR :
            return True
        return False

    def get_obj_desc_name(type):
        if Type.is_scalar_type(type) :
            type = Type.SCALAR
        return "tivx_obj_desc_" + type.name.lower() +  "_t"

    def get_scalar_obj_desc_data_name(type) :
        if Type.is_scalar_type(type) :
            if type is Type.CHAR :
                return "chr"
            if type is Type.INT8 :
                return "s8"
            if type is Type.UINT8 :
                return "u8"
            if type is Type.INT16:
                return "s16"
            if type is Type.UINT16:
                return "u16"
            if type is Type.INT32:
                return "s32"
            if type is Type.UINT32:
                return "u32"
            if type is Type.INT64:
                return "s64"
            if type is Type.UINT64:
                return "u64"
            if type is Type.FLOAT32:
                return "f32"
            if type is Type.FLOAT64:
                return "f64"
            if type is Type.ENUM :
                return "enm"
            if type is Type.SIZE :
                return "size"
            if type is Type.DF_IMAGE:
                return "fcc"
            if type is Type.BOOL :
                return "boolean"
            return "invalid"
        return "invalid"

class DfImage(Enum) :
    INVALID = 0
    VIRT = 1
    RGB  = 2
    RGBX = 3
    NV12 = 4
    NV21 = 5
    UYVY = 6
    YUYV = 7
    IYUV = 8
    YUV4 = 9
    U8   = 10
    U16  = 11
    S16  = 12
    U32  = 13
    S32  = 14

    def get_vx_enum_name(df_format) :
        return "VX_DF_IMAGE_" + df_format.name

    def get_vx_name(df_format) :
        return "vx_df_image_e"

class Channel(Enum) :
    C0 = 1
    C1 = 2
    C2 = 3
    C3 = 4
    R  = 5
    G  = 6
    B  = 7
    A  = 8
    Y  = 9
    U  = 10
    V  = 11

    def get_vx_enum_name(type) :
        if type == Channel.C0 :
            return "VX_CHANNEL_0"
        if type == Channel.C1 :
            return "VX_CHANNEL_1"
        if type == Channel.C2 :
            return "VX_CHANNEL_2"
        if type == Channel.C3 :
            return "VX_CHANNEL_3"
        return "VX_CHANNEL_" + type.name

    def get_vx_name(type) :
        return "vx_channel_e"

class Cpu(Enum) :
    INVALID = 1
    DSP1    = 2
    DSP2    = 3
    EVE1    = 4
    EVE2    = 5
    EVE3    = 6
    EVE4    = 7
    MPU_0   = 8
    MPU_1   = 9
    IPU1_0  = 10
    IPU1_1  = 11
    IPU2_0  = 12
    IPU2_1  = 13

    def get_vx_enum_name(type) :
        return "TIVX_CPU_ID_" + type.name

class Target(Enum) :
    INVALID = 1
    DSP1    = 2
    DSP2    = 3
    EVE1    = 4
    EVE2    = 5
    EVE3    = 6
    EVE4    = 7
    MPU_0   = 8
    MPU_1   = 9
    IPU1_0  = 10
    IPU1_1  = 11
    IPU2_0  = 12
    IPU2_1  = 13
    DEFAULT = DSP1

    def get_vx_enum_name(type) :
        return "TIVX_TARGET_" + type.name

    def get_cpu(target) :
        if target == Target.DSP1 :
            return Cpu.DSP1
        if target == Target.DSP2 :
            return Cpu.DSP2
        if target == Target.EVE1 :
            return Cpu.EVE1
        if target == Target.EVE2 :
            return Cpu.EVE2
        if target == Target.EVE3 :
            return Cpu.EVE3
        if target == Target.EVE4 :
            return Cpu.EVE4
        if target == Target.MPU_0 :
            return Cpu.MPU_0
        if target == Target.MPU_1 :
            return Cpu.MPU_1
        if target == Target.IPU1_0 :
            return Cpu.IPU1_0
        if target == Target.IPU1_1 :
            return Cpu.IPU1_1
        if target == Target.IPU2_0 :
            return Cpu.IPU2_0
        if target == Target.IPU2_1 :
            return Cpu.IPU2_1

        return Cpu.INVALID

class Policy(Enum) :
    WRAP      = 1
    SATURATE  = 2

    def get_vx_enum_name(type) :
        return "VX_CONVERT_POLICY_" + type.name

    def get_vx_name(type) :
        return "vx_convert_policy_e"

class NonLinearFilter(Enum) :
    MEDIAN   = 1
    MIN      = 2
    MAX      = 3

    def get_vx_enum_name(type) :
        return "VX_NONLINEAR_FILTER_" + type.name

    def get_vx_name(type) :
        return "vx_non_linear_filter_e"

class Pattern(Enum) :
    BOX      = 1
    CROSS    = 2
    DISK     = 3
    OTHER    = 4

    def get_vx_enum_name(type) :
        return "VX_PATTERN_" + type.name

    def get_vx_name(type) :
        return "vx_pattern_e"

class InterpolationType(Enum) :
    NEAREST_NEIGHBOR    = 1
    BILINEAR            = 2
    AREA                = 3

    def get_vx_enum_name(type) :
        return "VX_INTERPOLATION_" + type.name

    def get_vx_name(type) :
        return "vx_interpolation_type_e"

class Bool(Enum) :
    FALSE           = 0
    TRUE            = 1

    def get_vx_enum_name(type) :
        return "vx_" + type.name.lower() + "_e"

    def get_vx_name(type) :
        return "vx_bool"

class Norm(Enum):
    L1 = 1
    L2 = 2

    def get_vx_enum_name(type) :
        return "VX_NORM_" + type.name

    def get_vx_name(type) :
        return "vx_norm_e"

class Direction(Enum):
    INPUT = 1
    OUTPUT = 2
    BIDIRECTIONAL = 3

    def get_vx_enum_name(type) :
        return "VX_" + type.name

    def get_vx_name(type) :
        return "vx_direction_e"

    def get_access_type(type) :
        if type == Direction.INPUT:
            return "VX_READ_ONLY"
        if type == Direction.OUTPUT:
            return "VX_WRITE_ONLY"
        if type == Direction.BIDIRECTIONAL:
            return "VX_READ_WRITE"
        return "INVALID"

class ParamState(Enum):
    REQUIRED = 1
    OPTIONAL = 2

    def get_vx_enum_name(type) :
        return "VX_PARAMETER_STATE_" + type.name

    def get_vx_name(type) :
        return "vx_parameter_state_e"
