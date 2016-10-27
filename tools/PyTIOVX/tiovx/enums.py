'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from enum import Enum
from . import *

class Type(Enum):
    INVALID = 1
    CHAR    = 2
    INT8    = 3
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

    def get_vx_name(type) :
        return "vx_" + type.name.lower()

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

    def get_vx_name(df_format) :
        return VX_DF_IMAGE_ + df_format.name

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

class Policy(Enum) :
    WRAP      = 1
    SATURATE  = 2

class Interpolation(Enum) :
    NEAREST   = 1
    BILINEAR  = 2
    AREA      = 3

class NonlinearFilter(Enum) :
    MEDIAN   = 1
    MIN      = 2
    MAX      = 3

class Pattern(Enum) :
    BOX      = 1
    CROSS    = 2
    DISK     = 3
    OTHER    = 4
