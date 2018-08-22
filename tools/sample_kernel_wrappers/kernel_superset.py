'''
* Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''
'''
* The below PyTIOVX script creates a dummy kernel and contains a superset of 
* potential data objects added to the kernel. It also shows how to set
* relationships between parameters of the kernel in order to perform parameter
* checking.
'''

from tiovx import *

code = KernelExportCode("superset_module", Core.C66, "CUSTOM_APPLICATION_PATH")

kernel = Kernel("kernel_superset")

# Setting input parameters
kernel.setParameter(Type.ENUM,          Direction.INPUT, ParamState.OPTIONAL, "IN_ENUM")
kernel.setParameter(Type.IMAGE,         Direction.INPUT, ParamState.REQUIRED, "IN_IMAGE",         ['VX_DF_IMAGE_U8', 'VX_DF_IMAGE_U16'])
kernel.setParameter(Type.SCALAR,        Direction.INPUT, ParamState.REQUIRED, "IN_SCALAR")
kernel.setParameter(Type.CHAR,          Direction.INPUT, ParamState.REQUIRED, "IN_CHAR",          ['VX_TYPE_CHAR'])
kernel.setParameter(Type.INT8,          Direction.INPUT, ParamState.REQUIRED, "IN_INT8",          ['VX_TYPE_INT8'])
kernel.setParameter(Type.UINT8,         Direction.INPUT, ParamState.REQUIRED, "IN_UINT8",         ['VX_TYPE_UINT8'])
kernel.setParameter(Type.INT16,         Direction.INPUT, ParamState.REQUIRED, "IN_INT16",         ['VX_TYPE_INT16'])
kernel.setParameter(Type.UINT16,        Direction.INPUT, ParamState.REQUIRED, "IN_UINT16",        ['VX_TYPE_UINT16'])
kernel.setParameter(Type.INT32,         Direction.INPUT, ParamState.REQUIRED, "IN_INT32",         ['VX_TYPE_INT32'])
kernel.setParameter(Type.UINT32,        Direction.INPUT, ParamState.REQUIRED, "IN_UINT32",        ['VX_TYPE_UINT32'])
kernel.setParameter(Type.INT64,         Direction.INPUT, ParamState.REQUIRED, "IN_INT64",         ['VX_TYPE_INT64'])
kernel.setParameter(Type.UINT64,        Direction.INPUT, ParamState.REQUIRED, "IN_UINT64",        ['VX_TYPE_UINT64'])
kernel.setParameter(Type.FLOAT32,       Direction.INPUT, ParamState.REQUIRED, "IN_FLOAT32",       ['VX_TYPE_FLOAT32'])
kernel.setParameter(Type.FLOAT64,       Direction.INPUT, ParamState.REQUIRED, "IN_FLOAT64",       ['VX_TYPE_FLOAT64'])
kernel.setParameter(Type.SIZE,          Direction.INPUT, ParamState.REQUIRED, "IN_SIZE",          ['VX_TYPE_SIZE'])
kernel.setParameter(Type.DF_IMAGE,      Direction.INPUT, ParamState.REQUIRED, "IN_DF_IMAGE",      ['VX_TYPE_DF_IMAGE'])
kernel.setParameter(Type.BOOL,          Direction.INPUT, ParamState.REQUIRED, "IN_BOOL",          ['VX_TYPE_BOOL'])
kernel.setParameter(Type.ARRAY,         Direction.INPUT, ParamState.REQUIRED, "IN_ARRAY",         ['tivx_array_params_t'])
kernel.setParameter(Type.PYRAMID,       Direction.INPUT, ParamState.REQUIRED, "IN_PYRAMID",       ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.MATRIX,        Direction.INPUT, ParamState.REQUIRED, "IN_MATRIX",        ['VX_DF_IMAGE_S16'])
kernel.setParameter(Type.CONVOLUTION,   Direction.INPUT, ParamState.REQUIRED, "IN_CONVOLUTION",   ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.DISTRIBUTION,  Direction.INPUT, ParamState.REQUIRED, "IN_DISTRIBUTION",  ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.REMAP,         Direction.INPUT, ParamState.REQUIRED, "IN_REMAP",         ['VX_DF_IMAGE_U16'])
kernel.setParameter(Type.LUT,           Direction.INPUT, ParamState.REQUIRED, "IN_LUT",           ['VX_DF_IMAGE_U16'])
kernel.setParameter(Type.THRESHOLD,     Direction.INPUT, ParamState.REQUIRED, "IN_THRESHOLD",     ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.OBJECT_ARRAY,  Direction.INPUT, ParamState.REQUIRED, "IN_OBJECT_ARRAY",  ['VX_DF_IMAGE_S32'])

# Setting output parameters
kernel.setParameter(Type.IMAGE,         Direction.OUTPUT, ParamState.REQUIRED, "OUT_IMAGE",         ['VX_DF_IMAGE_U8', 'VX_DF_IMAGE_U16'])
kernel.setParameter(Type.SCALAR,        Direction.OUTPUT, ParamState.REQUIRED, "OUT_SCALAR")
kernel.setParameter(Type.CHAR,          Direction.OUTPUT, ParamState.REQUIRED, "OUT_CHAR",          ['VX_TYPE_CHAR'])
kernel.setParameter(Type.INT8,          Direction.OUTPUT, ParamState.REQUIRED, "OUT_INT8",          ['VX_TYPE_INT8'])
kernel.setParameter(Type.UINT8,         Direction.OUTPUT, ParamState.REQUIRED, "OUT_UINT8",         ['VX_TYPE_UINT8'])
kernel.setParameter(Type.INT16,         Direction.OUTPUT, ParamState.REQUIRED, "OUT_INT16",         ['VX_TYPE_INT16'])
kernel.setParameter(Type.UINT16,        Direction.OUTPUT, ParamState.REQUIRED, "OUT_UINT16",        ['VX_TYPE_UINT16'])
kernel.setParameter(Type.INT32,         Direction.OUTPUT, ParamState.REQUIRED, "OUT_INT32",         ['VX_TYPE_INT32'])
kernel.setParameter(Type.UINT32,        Direction.OUTPUT, ParamState.REQUIRED, "OUT_UINT32",        ['VX_TYPE_UINT32'])
kernel.setParameter(Type.INT64,         Direction.OUTPUT, ParamState.REQUIRED, "OUT_INT64",         ['VX_TYPE_INT64'])
kernel.setParameter(Type.UINT64,        Direction.OUTPUT, ParamState.REQUIRED, "OUT_UINT64",        ['VX_TYPE_UINT64'])
kernel.setParameter(Type.FLOAT32,       Direction.OUTPUT, ParamState.REQUIRED, "OUT_FLOAT32",       ['VX_TYPE_FLOAT32'])
kernel.setParameter(Type.FLOAT64,       Direction.OUTPUT, ParamState.REQUIRED, "OUT_FLOAT64",       ['VX_TYPE_FLOAT64'])
kernel.setParameter(Type.ENUM,          Direction.OUTPUT, ParamState.REQUIRED, "OUT_ENUM",          ['VX_TYPE_ENUM'])
kernel.setParameter(Type.SIZE,          Direction.OUTPUT, ParamState.REQUIRED, "OUT_SIZE",          ['VX_TYPE_SIZE'])
kernel.setParameter(Type.DF_IMAGE,      Direction.OUTPUT, ParamState.REQUIRED, "OUT_DF_IMAGE",      ['VX_TYPE_DF_IMAGE'])
kernel.setParameter(Type.BOOL,          Direction.OUTPUT, ParamState.REQUIRED, "OUT_BOOL",          ['VX_TYPE_BOOL'])
kernel.setParameter(Type.ARRAY,         Direction.OUTPUT, ParamState.REQUIRED, "OUT_ARRAY",         ['tivx_array_params_t'])
kernel.setParameter(Type.PYRAMID,       Direction.OUTPUT, ParamState.REQUIRED, "OUT_PYRAMID",       ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.MATRIX,        Direction.OUTPUT, ParamState.REQUIRED, "OUT_MATRIX",        ['VX_DF_IMAGE_S16'])
kernel.setParameter(Type.CONVOLUTION,   Direction.OUTPUT, ParamState.REQUIRED, "OUT_CONVOLUTION",   ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.DISTRIBUTION,  Direction.OUTPUT, ParamState.REQUIRED, "OUT_DISTRIBUTION",  ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.REMAP,         Direction.OUTPUT, ParamState.REQUIRED, "OUT_REMAP",         ['VX_DF_IMAGE_U16'])
kernel.setParameter(Type.LUT,           Direction.OUTPUT, ParamState.REQUIRED, "OUT_LUT",           ['VX_DF_IMAGE_U16'])
kernel.setParameter(Type.THRESHOLD,     Direction.OUTPUT, ParamState.REQUIRED, "OUT_THRESHOLD",     ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.OBJECT_ARRAY,  Direction.OUTPUT, ParamState.REQUIRED, "OUT_OBJECT_ARRAY",  ['VX_DF_IMAGE_S32'])

# Setting parameter relationships
kernel.setParameterRelationship(["IN_IMAGE",         "OUT_IMAGE"],        [Attribute.Image.WIDTH, Attribute.Image.HEIGHT, \
    Attribute.Image.FORMAT, Attribute.Image.PLANES, Attribute.Image.SPACE, Attribute.Image.RANGE, Attribute.Image.SIZE, Attribute.Image.MEMORY_TYPE])
kernel.setParameterRelationship(["IN_ARRAY",         "OUT_ARRAY"],        [Attribute.Array.ITEMTYPE, Attribute.Array.NUMITEMS, \
    Attribute.Array.CAPACITY, Attribute.Array.ITEMSIZE])
kernel.setParameterRelationship(["IN_PYRAMID",       "OUT_PYRAMID"],      [Attribute.Pyramid.LEVELS, Attribute.Pyramid.SCALE, \
    Attribute.Pyramid.WIDTH, Attribute.Pyramid.HEIGHT, Attribute.Pyramid.FORMAT])
kernel.setParameterRelationship(["IN_MATRIX",        "OUT_MATRIX"],       [Attribute.Matrix.TYPE, Attribute.Matrix.ROWS, Attribute.Matrix.COLUMNS, \
    Attribute.Matrix.SIZE, Attribute.Matrix.ORIGIN, Attribute.Matrix.PATTERN])
kernel.setParameterRelationship(["IN_CONVOLUTION",   "OUT_CONVOLUTION"],  [Attribute.Convolution.ROWS, Attribute.Convolution.COLUMNS, \
    Attribute.Convolution.SCALE, Attribute.Convolution.SIZE])
kernel.setParameterRelationship(["IN_DISTRIBUTION",  "OUT_DISTRIBUTION"], [Attribute.Distribution.DIMENSIONS, Attribute.Distribution.OFFSET, \
    Attribute.Distribution.RANGE, Attribute.Distribution.BINS, Attribute.Distribution.WINDOW, Attribute.Distribution.SIZE])
kernel.setParameterRelationship(["IN_REMAP",         "OUT_REMAP"],        [Attribute.Remap.SOURCE_WIDTH, Attribute.Remap.SOURCE_HEIGHT, \
    Attribute.Remap.DESTINATION_WIDTH, Attribute.Remap.DESTINATION_HEIGHT])
kernel.setParameterRelationship(["IN_LUT",           "OUT_LUT"],          [Attribute.Lut.TYPE, Attribute.Lut.COUNT, Attribute.Lut.SIZE, \
    Attribute.Lut.OFFSET])
kernel.setParameterRelationship(["IN_THRESHOLD",     "OUT_THRESHOLD"],    [Attribute.Threshold.TYPE, Attribute.Threshold.THRESHOLD_VALUE, \
    Attribute.Threshold.THRESHOLD_LOWER, Attribute.Threshold.THRESHOLD_UPPER, Attribute.Threshold.TRUE_VALUE, Attribute.Threshold.FALSE_VALUE, \
    Attribute.Threshold.DATA_TYPE])
kernel.setParameterRelationship(["IN_OBJECT_ARRAY",  "OUT_OBJECT_ARRAY"], [Attribute.ObjectArray.NUMITEMS, Attribute.ObjectArray.ITEMTYPE])

# Setting target cores
kernel.setTarget(Target.IPU1_0)
kernel.setTarget(Target.IPU1_1)
kernel.setTarget(Target.IPU2)
kernel.setTarget(Target.A15_0)
kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)
kernel.setTarget(Target.EVE1)
kernel.setTarget(Target.EVE2)
kernel.setTarget(Target.EVE3)
kernel.setTarget(Target.EVE4)

code.export(kernel)
code.exportDiagram(kernel)
